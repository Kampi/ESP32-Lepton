/*
 * basic_capture.cpp
 *
 *  Copyright (C) Daniel Kampert, 2026
 *  Website: www.kampis-elektroecke.de
 *  File info: Basic capture example for ESP32-Lepton component.
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
#include <esp_task_wdt.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <stdio.h>

#include "lepton.h"

static Lepton_Conf_t Config;
static Lepton_t Device;
static i2c_master_bus_handle_t I2C_Handle = NULL;
static EventGroupHandle_t EventGroup;
static SemaphoreHandle_t BufferMutex;
static QueueHandle_t RawFrameQueue;
static Lepton_FrameBuffer_t RawFrame;
static uint8_t CurrentReadBuffer;
static uint8_t *RGB_Buffer[2];

static const char *TAG = "Lepton-Example";

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-Lepton Minimal Example");
    ESP_LOGI(TAG, "============================");
    ESP_LOGI(TAG, "Waiting for Lepton to boot...");

    vTaskDelay(pdMS_TO_TICKS(2000));

    Config = LEPTON_DEFAULT_CONF;
    LEPTON_ASSIGN_I2C_FUNC(Config, I2CM_Init, I2CM_Deinit, I2CM_Write, I2CM_Read, I2CM_WriteRead);

    // ...

    LEPTON_ASSIGN_I2C_HANDLE(Config, I2C_Handle);

    /* Create event group */
    EventGroup = xEventGroupCreate();
    if (EventGroup == NULL) {
        ESP_LOGE(TAG, "Failed to create event group!");
        return;
    }

    /* Create mutex for buffer synchronization */
    BufferMutex = xSemaphoreCreateMutex();
    if (BufferMutex == NULL) {
        ESP_LOGE(TAG, "Failed to create buffer mutex!");
        vEventGroupDelete(EventGroup);
        return;
    }

    /* Allocate 2 RGB buffers for ping-pong buffering */
    /* RGB888 frame buffers: 160 × 120 × 3 = 57,600 bytes each (PSRAM) */
    RGB_Buffer[0] = static_cast<uint8_t *>(heap_caps_malloc(LEPTON_IMAGE_WIDTH * LEPTON_IMAGE_HEIGHT * 3,
                                                            MALLOC_CAP_SIMD | MALLOC_CAP_SPIRAM));
    RGB_Buffer[1] = static_cast<uint8_t *>(heap_caps_malloc(LEPTON_IMAGE_WIDTH * LEPTON_IMAGE_HEIGHT * 3,
                                                            MALLOC_CAP_SIMD | MALLOC_CAP_SPIRAM));
    if ((RGB_Buffer[0] == NULL) || (RGB_Buffer[1] == NULL)) {
        ESP_LOGE(TAG, "Can not allocate RGB buffers!");

        if (RGB_Buffer[0]) {
            heap_caps_free(RGB_Buffer[0]);
        }

        if (RGB_Buffer[1]) {
            heap_caps_free(RGB_Buffer[1]);
        }

        vSemaphoreDelete(BufferMutex);
        vEventGroupDelete(EventGroup);

        return;
    }

    /* Create internal queue to receive raw frames from VoSPI capture task */
    RawFrameQueue = xQueueCreate(1, sizeof(Lepton_FrameBuffer_t));
    if (RawFrameQueue == NULL) {
        ESP_LOGE(TAG, "Failed to create raw frame queue!");

        heap_caps_free(RGB_Buffer[0]);
        heap_caps_free(RGB_Buffer[1]);
        vSemaphoreDelete(BufferMutex);
        vEventGroupDelete(EventGroup);

        return;
    }

    Lepton_Error_t Error = Lepton_Init(&Device, &Config);
    if (Error != LEPTON_ERR_OK) {
        ESP_LOGE(TAG, "Failed to initialize Lepton: %d!", Error);

        vQueueDelete(RawFrameQueue);
        heap_caps_free(RGB_Buffer[0]);
        heap_caps_free(RGB_Buffer[1]);
        vSemaphoreDelete(BufferMutex);
        vEventGroupDelete(EventGroup);

        return;
    }

    ESP_LOGI(TAG, "Lepton initialized successfully!");

    char SerialStr[24];
    snprintf(SerialStr, sizeof(SerialStr), "%02X%02X-%02X%02X-%02X%02X-%02X%02X",
             Device.SerialNumber[0], Device.SerialNumber[1],
             Device.SerialNumber[2], Device.SerialNumber[3],
             Device.SerialNumber[4], Device.SerialNumber[5],
             Device.SerialNumber[6], Device.SerialNumber[7]);
    ESP_LOGI(TAG, "\tPart number: %s", Device.PartNumber);
    ESP_LOGI(TAG, "\tSerial number: %s", SerialStr);

    ESP_LOGI(TAG, "Start image capturing...");
    if (Lepton_StartCapture(&Device, RawFrameQueue) != LEPTON_ERR_OK) {
        ESP_LOGE(TAG, "Can not start image capturing!");
    }

    while (1) {
        esp_task_wdt_reset();

        /* Wait for a new raw frame with longer timeout to avoid busy waiting */
        if (xQueueReceive(RawFrameQueue, &RawFrame, pdMS_TO_TICKS(500)) == pdTRUE) {
            uint8_t WriteBufferIdx;
            uint8_t *WriteBuffer;

            /* When telemetry is available */
            /*
            Lepton_Telemetry_t Telemetry;
            if (RawFrame.TelemetryBuffer != NULL) {
                memcpy(&Telemetry, RawFrame.TelemetryBuffer, sizeof(Lepton_Telemetry_t));
                ESP_LOGD(TAG, "Telemetry - FrameCounter: %u, FPA_Temp: %uK, Housing_Temp: %uK",
                         Telemetry.FrameCounter,
                         Telemetry.FPA_Temp,
                         Telemetry.Housing_Temp);
            }
            */

            ESP_LOGI(TAG, "Processing frame...");

            /* Determine which buffer to write to (ping-pong) */
            if (xSemaphoreTake(BufferMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                /* Find a buffer that's not currently being read */
                WriteBufferIdx = (CurrentReadBuffer + 1) % 2;
                WriteBuffer = RGB_Buffer[WriteBufferIdx];
                xSemaphoreGive(BufferMutex);
            } else {
                ESP_LOGW(TAG, "Failed to acquire mutex for buffer selection!");
                continue;
            }

            Lepton_Raw14ToRGB(&Device, RawFrame.ImageBuffer, WriteBuffer, NULL, NULL, RawFrame.Width, RawFrame.Height,
                             Lepton_Palette_Table[LEPTON_PALETTE_IRON]);

            /* Mark buffer as ready and update read buffer index */
            if (xSemaphoreTake(BufferMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                CurrentReadBuffer = WriteBufferIdx;
                xSemaphoreGive(BufferMutex);
            } else {
                ESP_LOGW(TAG, "Failed to acquire mutex for buffer ready!");
                continue;
            }

            /* Do something with the frame */

        } else {
            /* Timeout waiting for frame */
            ESP_LOGW(TAG, "No raw frame received from VoSPI");
        }
    }

    Lepton_Deinit(&Device);
}