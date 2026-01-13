/*
 * lepton_capture.cpp
 *
 *  Copyright (C) Daniel Kampert, 2026
 *  Website: www.kampis-elektroecke.de
 *  File info: Lepton frame capture implementation.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * Errors and commissions should be reported to DanielKampert@kampis-elektroecke.de
 */

#include <esp_log.h>
#include <esp_timer.h>
#include <esp_task_wdt.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include "lepton.h"
#include "vospi.h"

#include <sdkconfig.h>

#ifndef CONFIG_LEPTON_CAPTURE_TASK_STACK
#define CONFIG_LEPTON_CAPTURE_TASK_STACK                4096
#endif

#ifndef CONFIG_LEPTON_CAPTURE_TASK_PRIORITY
#define CONFIG_LEPTON_CAPTURE_TASK_PRIORITY             16
#endif

#ifndef CONFIG_LEPTON_CAPTURE_TASK_CORE_AFFINITY
#ifndef CONFIG_LEPTON_CAPTURE_TASK_CORE
#define CONFIG_LEPTON_CAPTURE_TASK_CORE                 1
#endif
#endif

static const char *TAG = "Lepton-Capture";

#ifdef CONFIG_LEPTON_GPIO_USE_VSYNC
/** @brief          VSync interrupt handler.
 *  @param p_Args   Pointer to task arguments
 */
#ifdef CONFIG_LEPTON_VSYNC_PLACE_IRAM
static void IRAM_ATTR Lepton_VSync_ISR_Handler(void *p_Args)
#else
static void Lepton_VSync_ISR_Handler(void *p_Args)
#endif
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xTaskNotifyFromISR((TaskHandle_t)p_Args, 0, eNoAction, &xHigherPriorityTaskWoken);

    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}
#endif

/** @brief          Camera image capture task.
 *  @param p_Args   Pointer to task arguments
 */
static void Lepton_CaptureTask(void *p_Args)
{
    Lepton_t *Device = static_cast<Lepton_t *>(p_Args);

    int ConsecutiveErrors = 0;

    esp_task_wdt_add(NULL);

    ESP_LOGD(TAG, "Capture task started");

    while (Device->Internal.VoSPI.isCapturing) {
        int Error;
        uint8_t BufferIndex = 0;

        esp_task_wdt_reset();

#ifdef CONFIG_LEPTON_GPIO_USE_VSYNC
        uint32_t ulNotificationValue;
        if (xTaskNotifyWait(0x00, 0xFFFFFFFF, &ulNotificationValue, portMAX_DELAY) != pdTRUE) {
            ESP_LOGW(TAG, "VSync wait failed!");
            continue;
        }
#endif

        /* Capture the frame */
        Error = VoSPI_CaptureImage(&Device->Internal.VoSPI, &BufferIndex);
        if (Error == LEPTON_ERR_OK) {
            ConsecutiveErrors = 0;

            if (Device->Internal.VoSPI.FrameCounter % 100 == 0) {
                if (Device->Internal.FrameQueue != NULL) {
                    ESP_LOGD(TAG, "Captured %u frames, queue space: %d", static_cast<unsigned int>(Device->Internal.VoSPI.FrameCounter),
                             uxQueueSpacesAvailable(Device->Internal.FrameQueue));
                } else {
                    ESP_LOGD(TAG, "Captured %u frames", static_cast<unsigned int>(Device->Internal.VoSPI.FrameCounter));
                }
            }

            /* Frame successfully captured - send pointer to queue if available */
            if (Device->Internal.FrameQueue != NULL) {
                Lepton_FrameBuffer_t FrameBuffer;
                FrameBuffer.Height = Device->Internal.VoSPI.ImageHeight;
                FrameBuffer.Width = Device->Internal.VoSPI.ImageWidth;
                FrameBuffer.BytesPerPixel = Device->Internal.VoSPI.BytesPerPixel;

                /* Use the buffer index returned by VoSPI_CaptureImage */
                FrameBuffer.Image_Buffer = Device->Internal.VoSPI.Image_Buffer[BufferIndex];

                if (Device->Internal.VoSPI.useTelemetry) {
                    FrameBuffer.Telemetry_Buffer = Device->Internal.VoSPI.Telemetry_Buffer[BufferIndex];
                } else {
                    FrameBuffer.Telemetry_Buffer = NULL;
                }

                /* Use overwrite to always update with latest frame */
                xQueueOverwrite(Device->Internal.FrameQueue, &FrameBuffer);
            } else {
                ESP_LOGE(TAG, "Frame queue is NULL!");
            }
        } else if (Error == LEPTON_ERR_FAIL) {
            /* Sync error - already handled in VoSPI_CaptureImage */
            Device->Internal.VoSPI.SyncErrors++;
            ConsecutiveErrors++;

            if (ConsecutiveErrors % 10 == 1) {
                ESP_LOGW(TAG, "Sync error occurred (consecutive: %d, total: %" PRIu32 ")", ConsecutiveErrors,
                         Device->Internal.VoSPI.SyncErrors);
            }

            /* Longer delay on errors to reduce CPU load during problematic conditions */
            vTaskDelay(10 / portTICK_PERIOD_MS);
        } else if (Error == LEPTON_ERR_NOT_FINISHED) {
            ESP_LOGD(TAG, "Resyncing...");
            /* Short delay during resyncing */
            vTaskDelay(1 / portTICK_PERIOD_MS);
        } else {
            ESP_LOGW(TAG, "Unexpected result: %d", Error);
            vTaskDelay(5 / portTICK_PERIOD_MS);
        }
    }
}

Lepton_Error_t Lepton_StartCapture(Lepton_t *p_Device, QueueHandle_t p_Queue)
{
    Lepton_Error_t Error;

    if ((p_Device == NULL) || (p_Queue == NULL)) {
        return LEPTON_ERR_INVALID_ARG;
    } else if (p_Device->Internal.isInitialized == false) {
        return LEPTON_ERR_NOT_INITIALIZED;
    } else if (p_Device->Internal.CapHandle != NULL) {
        return LEPTON_ERR_BUSY;
    }

    Error = LEPTON_ERR_OK;

    p_Device->Internal.FrameQueue = p_Queue;
    p_Device->Internal.VoSPI.CurrentBuffer = 0;
    p_Device->Internal.VoSPI.isCapturing = true;
    p_Device->Internal.MinSmooth = 0;
    p_Device->Internal.MaxSmooth = 16383;

    ESP_LOGD(TAG, "Creating capture task...");

#ifdef CONFIG_LEPTON_CAPTURE_TASK_CORE_AFFINITY
    xTaskCreatePinnedToCore(&Lepton_CaptureTask, "Lepton_Capture", CONFIG_LEPTON_CAPTURE_TASK_STACK, p_Device,
                            CONFIG_LEPTON_CAPTURE_TASK_PRIORITY, &p_Device->Internal.CapHandle, CONFIG_LEPTON_CAPTURE_TASK_CORE);
#else
    xTaskCreate(&Lepton_CaptureTask, "Lepton_Capture", CONFIG_LEPTON_CAPTURE_TASK_STACK, p_Device,
                CONFIG_LEPTON_CAPTURE_TASK_PRIORITY, &p_Device->Internal.CapHandle);
#endif

    if (p_Device->Internal.CapHandle == NULL) {
        ESP_LOGE(TAG, "Failed to create capture task!");
        p_Device->Internal.VoSPI.isCapturing = false;
        p_Device->Internal.FrameQueue = NULL;
        Error = LEPTON_ERR_NO_MEM;
        goto Lepton_StartCapture_Error_1;
    }

    ESP_LOGD(TAG, "Capture task created successfully");

    /* V-Sync is a high-level signal. So we need to add a positive edge interrupt */
#ifdef CONFIG_LEPTON_GPIO_USE_VSYNC
    gpio_set_direction(p_Device->Internal.VSync, GPIO_MODE_INPUT);
    gpio_set_pull_mode(p_Device->Internal.VSync, GPIO_PULLDOWN_ONLY);
    gpio_set_intr_type(p_Device->Internal.VSync, GPIO_INTR_POSEDGE);

#ifdef CONFIG_LEPTON_VSYNC_PLACE_IRAM
    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
#else
    gpio_install_isr_service(0);
#endif

    /* Hook ISR handler for specific GPIO */
    gpio_isr_handler_add(p_Device->Internal.VSync, Lepton_VSync_ISR_Handler, p_Device->Internal.CapHandle);
#endif

    ESP_LOGD(TAG, "Capture started successfully");

    return LEPTON_ERR_OK;

Lepton_StartCapture_Error_1:
    ESP_LOGE(TAG, "Initialization error: 0x%X!", static_cast<unsigned int>(Error));

    return Error;
}

Lepton_Error_t Lepton_StopCapture(Lepton_t *p_Device)
{
    if (p_Device == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    } else if (p_Device->Internal.isInitialized == false) {
        return LEPTON_ERR_NOT_INITIALIZED;
    } else if (p_Device->Internal.CapHandle == NULL) {
        return LEPTON_ERR_OK;
    }

    p_Device->Internal.VoSPI.isCapturing = false;

    vTaskDelete(p_Device->Internal.CapHandle);
    esp_task_wdt_delete(p_Device->Internal.CapHandle);

    return LEPTON_ERR_OK;
}

bool Lepton_Raw14ToRGB(Lepton_t *p_Device, uint16_t *p_Input, uint8_t *p_Output, int16_t *p_Min, int16_t *p_Max, uint16_t Width,
                       uint16_t Height)
{
    uint16_t min = INT16_MAX;
    uint16_t max = 0;
    uint32_t range;

    if ((p_Device == NULL) || (p_Device->Internal.isInitialized == false) || ((p_Input == NULL) || (p_Output == NULL))) {
        return false;
    }

    /* Find min and max values for normalization */
    for (uint32_t i = 0; i < (Width * Height); i++) {
        if (p_Input[i] < min) {
            min = p_Input[i];
        }
        if (p_Input[i] > max) {
            max = p_Input[i];
        }

        /* Reset watchdog periodically during min/max search */
        if ((i & 0x3FF) == 0) { /* Every 1024 pixels */
            esp_task_wdt_reset();
        }
    }

    /* Smooth min/max to reduce flicker */
    if ((p_Device->Internal.MinSmooth == 0) && (p_Device->Internal.MaxSmooth == 16383)) {
        /* First frame - initialize */
        p_Device->Internal.MinSmooth = min;
        p_Device->Internal.MaxSmooth = max;
    } else {
        /* Exponential moving average */
        p_Device->Internal.MinSmooth = static_cast<uint16_t>(p_Device->Internal.SmoothFactor * p_Device->Internal.MinSmooth + (1.0f - p_Device->Internal.SmoothFactor) * min);
        p_Device->Internal.MaxSmooth = static_cast<uint16_t>(p_Device->Internal.SmoothFactor * p_Device->Internal.MaxSmooth + (1.0f - p_Device->Internal.SmoothFactor) * max);
    }

    range = p_Device->Internal.MaxSmooth - p_Device->Internal.MinSmooth;
    /* Avoid division by zero */
    if (range < 10) {
        range = 10;
    }

    /* Apply iron palette */
    for (uint32_t i = 0; i < (Width * Height); i++) {
        /* Normalize to 0-255 range using smoothed min/max */
        uint32_t normalized = ((p_Input[i] - p_Device->Internal.MinSmooth) * 255) / range;
        if (normalized > 255) {
            normalized = 255;
        }

        p_Output[i * 3 + 0] = Lepton_Palette_Iron[normalized][0];
        p_Output[i * 3 + 1] = Lepton_Palette_Iron[normalized][1];
        p_Output[i * 3 + 2] = Lepton_Palette_Iron[normalized][2];

        /* Reset watchdog periodically during conversion */
        if ((i & 0x3FF) == 0) { /* Every 1024 pixels */
            esp_task_wdt_reset();
        }
    }

    if (p_Min != NULL) {
        *p_Min = static_cast<int16_t>(min);
    }

    if (p_Max != NULL) {
        *p_Max = static_cast<int16_t>(max);
    }

    return true;
}