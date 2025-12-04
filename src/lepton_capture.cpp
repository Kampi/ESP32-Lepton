 /*
 * lepton_capture.cpp
 *
 *  Copyright (C) Daniel Kampert, 2025
 *	Website: www.kampis-elektroecke.de
 *  File info: FLIR Lepton thermal imaging sensor driver for ESP32.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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

#ifndef CONFIG_LEPTON_TASK_STACK
    #define CONFIG_LEPTON_TASK_STACK                2048
#endif

#ifndef CONFIG_LEPTON_TASK_PRIORITY
    #define CONFIG_LEPTON_TASK_PRIORITY             12
#endif

#ifndef CONFIG_LEPTON_TASK_CORE_AFFINITY
    #ifndef CONFIG_LEPTON_TASK_CORE
        #define CONFIG_LEPTON_TASK_CORE             1
    #endif
#endif

static const char* TAG = "Lepton-Capture";


#if CONFIG_LEPTON_GPIO_VSYNC_PIN >= 0
/** @brief			
 *  @param p_Args	
 */
#ifdef CONFIG_LEPTON_VSYNC_PLACE_IRAM
    static void IRAM_ATTR Lepton_VSync_ISR_Handler(void* p_Args)
#else
    static void Lepton_VSync_ISR_Handler(void* p_Args)
#endif
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xTaskNotifyFromISR((TaskHandle_t)p_Args, 0, eNoAction, &xHigherPriorityTaskWoken);

    if(xHigherPriorityTaskWoken)
    {
        portYIELD_FROM_ISR();
    }
}
#endif

/** @brief			Camera image capture task.
 *  @param p_Args	Pointer to task arguments
 */
static void Lepton_CaptureTask(void* p_Args)
{
    Lepton_t* Device = static_cast<Lepton_t*>(p_Args);
    
    int consecutiveErrors = 0;

    ESP_LOGI(TAG, "Capture task started");

    Device->Internal.VoSPI.isCapturing = true;

    while(true)
    {
        int Error;
        uint8_t BufferIndex = 0;

        esp_task_wdt_reset();

        /* Capture the frame */
        Error = VoSPI_CaptureImage(&Device->Internal.VoSPI, &BufferIndex);

        if(Error == ESP_OK)
        {
            consecutiveErrors = 0;
            
            if(Device->Internal.VoSPI.FrameCounter % 100 == 0)
            {
                ESP_LOGI(TAG, "Captured %u frames, queue space: %d", static_cast<unsigned int>(Device->Internal.VoSPI.FrameCounter), uxQueueSpacesAvailable(Device->Internal.FrameQueue));
            }

            /* Frame successfully captured - send pointer to queue if available */
            if(Device->Internal.FrameQueue != NULL)
            {
                Lepton_FrameBuffer_t FrameBuffer;
                FrameBuffer.Height = Device->Internal.VoSPI.ImageHeight;
                FrameBuffer.Width = Device->Internal.VoSPI.ImageWidth;
                FrameBuffer.BytesPerPixel = Device->Internal.VoSPI.BytesPerPixel;

                /* Use the buffer index returned by VoSPI_CaptureImage */
                FrameBuffer.Image_Buffer = Device->Internal.VoSPI.Image_Buffer[BufferIndex];

                /* Use overwrite to always update with latest frame */
                xQueueOverwrite(Device->Internal.FrameQueue, &FrameBuffer);
            }
            else
            {
                ESP_LOGW(TAG, "Frame queue is NULL!");
            }
        }
        else if(Error == ESP_FAIL)
        {
            /* Sync error - already handled in VoSPI_CaptureImage */
            Device->Internal.VoSPI.SyncErrors++;
            consecutiveErrors++;

            if(consecutiveErrors % 10 == 1)
            {
                ESP_LOGW(TAG, "Sync error occurred (consecutive: %d, total: %" PRIu32 ")", consecutiveErrors, Device->Internal.VoSPI.SyncErrors);
            }
        }
        else if(Error == ESP_ERR_NOT_FINISHED)
        {
            ESP_LOGD(TAG, "Resyncing...");
        }
        else
        {
            ESP_LOGW(TAG, "Unexpected result: %d", Error);
        }

        /* Small delay to prevent CPU hogging during resync or errors */
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

Lepton_Error_t Lepton_StartCapture(Lepton_t* p_Device, QueueHandle_t p_Queue)
{
    Lepton_Error_t Error;

    if(p_Device == NULL)
    {
        ESP_LOGE(TAG, "Device is NULL");
        return LEPTON_ERR_INVALID_ARG;
    }
    else if(p_Device->Internal.isInitialized == false)
    {
        ESP_LOGE(TAG, "Device not initialized");
        return LEPTON_ERR_NOT_INITIALIZED;
    }
    else if(p_Device->Internal.CapHandle != NULL)
    {
        ESP_LOGE(TAG, "Capture already running");
        return LEPTON_ERR_BUSY;
    }

    Error = LEPTON_ERR_OK;

    p_Device->Internal.FrameQueue = p_Queue;
    p_Device->Internal.VoSPI.CurrentBuffer = 0;

    ESP_LOGI(TAG, "Creating capture task...");

    #ifdef CONFIG_LEPTON_TASK_CORE_AFFINITY
        xTaskCreatePinnedToCore(&Lepton_CaptureTask, "Lepton_Capture", CONFIG_LEPTON_CAPTURE_TASK_STACK, p_Device, CONFIG_LEPTON_CAPTURE_TASK_PRIORITY, &p_Device->Internal.CapHandle, CONFIG_LEPTON_CAPTURE_TASK_CORE);
    #else
        xTaskCreate(&Lepton_CaptureTask, "Lepton_Capture", CONFIG_LEPTON_CAPTURE_TASK_STACK, p_Device, CONFIG_LEPTON_CAPTURE_TASK_PRIORITY, &p_Device->Internal.CapHandle);
    #endif

    if(p_Device->Internal.CapHandle == NULL)
    {
        Error = LEPTON_ERR_NO_MEM;
        ESP_LOGE(TAG, "Failed to create capture task!");
        goto Lepton_StartCapture_Error_1;
    }

    ESP_LOGI(TAG, "Capture task created successfully");

    esp_task_wdt_add(p_Device->Internal.CapHandle);

    /* V-Sync is a high-level signal. So we need to add a positive edge interrupt */
    #if CONFIG_LEPTON_GPIO_VSYNC_PIN >= 0
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
        #error "Untested"
    #endif

    ESP_LOGI(TAG, "Capture started successfully");

    return LEPTON_ERR_OK;

Lepton_StartCapture_Error_1:
    ESP_LOGE(TAG, "Initialization error: 0x%X", static_cast<unsigned int>(Error));

    return Error;
}

Lepton_Error_t Lepton_StopCapture(Lepton_t* p_Device)
{
    if(p_Device == NULL)
    {
        return LEPTON_ERR_INVALID_ARG;
    }
    else if(p_Device->Internal.isInitialized == false)
    {
        return LEPTON_ERR_NOT_INITIALIZED;
    }
    else if(p_Device->Internal.CapHandle == NULL)
    {
        return LEPTON_ERR_OK;
    }

    vTaskDelete(p_Device->Internal.CapHandle);
    esp_task_wdt_delete(p_Device->Internal.CapHandle);

    return LEPTON_ERR_OK;
}

bool Lepton_Raw14ToRGB(uint16_t* p_Input, uint8_t* p_Output, uint16_t Width, uint16_t Height)
{
    static uint16_t min_smooth = 0;
    static uint16_t max_smooth = 16383;
    static const float SMOOTH_FACTOR = 0.95f; // Temporal smoothing
    
    uint16_t min = 0xFFFF;
    uint16_t max = 0;
    uint32_t range;

    if((p_Input == NULL) || (p_Output == NULL))
    {
        return false;
    }

    /* Find min and max values for normalization */
    for(uint32_t i = 0; i < (Width * Height); i++)
    {
        if(p_Input[i] < min) min = p_Input[i];
        if(p_Input[i] > max) max = p_Input[i];

        /* Reset watchdog periodically during min/max search */
        if((i & 0x3FF) == 0)  /* Every 1024 pixels */
        {
            esp_task_wdt_reset();
        }
    }

    /* Smooth min/max to reduce flicker */
    if(min_smooth == 0 && max_smooth == 16383)
    {
        /* First frame - initialize */
        min_smooth = min;
        max_smooth = max;
    }
    else
    {
        /* Exponential moving average */
        min_smooth = (uint16_t)(SMOOTH_FACTOR * min_smooth + (1.0f - SMOOTH_FACTOR) * min);
        max_smooth = (uint16_t)(SMOOTH_FACTOR * max_smooth + (1.0f - SMOOTH_FACTOR) * max);
    }

    range = max_smooth - min_smooth;

    /* Avoid division by zero */
    if(range < 10)
    {
        range = 10;
    }

    /* Apply iron palette */
    for(uint32_t i = 0; i < (Width * Height); i++)
    {
        /* Normalize to 0-255 range using smoothed min/max */
        uint32_t normalized = ((p_Input[i] - min_smooth) * 255) / range;
        if(normalized > 255)
        {
            normalized = 255;
        }

        p_Output[i * 3 + 0] = Lepton_Palette_Iron_R[normalized];
        p_Output[i * 3 + 1] = Lepton_Palette_Iron_G[normalized];
        p_Output[i * 3 + 2] = Lepton_Palette_Iron_B[normalized];

        /* Reset watchdog periodically during conversion */
        if((i & 0x3FF) == 0)  /* Every 1024 pixels */
        {
            esp_task_wdt_reset();
        }
    }

    return true;
}