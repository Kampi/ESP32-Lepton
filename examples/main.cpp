/*
 * main.cpp
 *
 *  Copyright (C) Daniel Kampert, 2025
 *  Website: www.kampis-elektroecke.de
 *  File info: Minimal example for ESP32-Lepton component.
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

#include <stdio.h>
#include <esp_log.h>
#include <esp_heap_caps.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "lepton.h"

/* Pin definitions - Adjust for your hardware */
#define LEPTON_SPI_MISO     GPIO_NUM_13
#define LEPTON_SPI_MOSI     GPIO_NUM_11
#define LEPTON_SPI_SCK      GPIO_NUM_12
#define LEPTON_SPI_CS       GPIO_NUM_10

#define LEPTON_I2C_SDA      GPIO_NUM_8
#define LEPTON_I2C_SCL      GPIO_NUM_9

static const char* TAG = "Lepton-Example";

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-Lepton Minimal Example");
    ESP_LOGI(TAG, "============================");
    ESP_LOGI(TAG, "Waiting for Lepton to boot...");

    vTaskDelay(2000 / portTICK_PERIOD_MS);

    Lepton_Config_t config;
    config = LEPTON_DEFAULT_CONF;
    LEPTON_ASSIGN_FUNC(config, I2CM_Init, I2CM_Deinit, I2CM_Write, I2CM_Read);

    ...

    LEPTON_ASSIGN_I2C_HANDLE(config, I2C_Handle);

    Lepton_t device;
    Lepton_Error_t error = Lepton_Init(&device, &config);
    if (error != LEPTON_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize Lepton: %d", error);
        ESP_LOGE(TAG, "Check wiring and power supply!");
        return;
    }

    ESP_LOGI(TAG, "Lepton initialized successfully!");

    uint16_t fpa_temp = 0;
    uint16_t aux_temp = 0;
    Lepton_Result_t status;

    error = Lepton_GetTemp(&device, &fpa_temp, &aux_temp, &status);
    if (error == LEPTON_OK)
    {
        float fpa_celsius = (fpa_temp / 100.0f) - 273.15f;
        float aux_celsius = (aux_temp / 100.0f) - 273.15f;
        ESP_LOGI(TAG, "FPA Temperature: %.2f °C", fpa_celsius);
        ESP_LOGI(TAG, "Housing Temperature: %.2f °C", aux_celsius);
    }

    Lepton_AGC_Enable_t agc_mode;
    error = Lepton_GetAGC(&device, &agc_mode);
    if (error == LEPTON_OK)
    {
        ESP_LOGI(TAG, "AGC Mode: %s", (agc_mode == LEPTON_AGC_ENABLE) ? "Enabled" : "Disabled");
    }

    size_t frame_size = LEPTON_IMAGE_SIZE_3 * sizeof(uint16_t);
    uint16_t* frame_buffer = (uint16_t*)heap_caps_malloc(frame_size, MALLOC_CAP_DMA);
    if (frame_buffer == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate frame buffer!");
        Lepton_Deinit(&device);
        return;
    }

    ESP_LOGI(TAG, "Frame buffer allocated (%d bytes)", frame_size);
    ESP_LOGI(TAG, "Starting continuous capture...");
    ESP_LOGI(TAG, "");

    uint32_t frame_count = 0;
    uint32_t total_capture_time = 0;

    while (true)
    {
        error = Lepton_CaptureFrameData(&device, frame_buffer);

        if (error == LEPTON_OK)
        {
            frame_count++;
            uint32_t capture_time = Lepton_GetCaptureTime(&device);
            total_capture_time += capture_time;

            uint16_t min_val = 0xFFFF;
            uint16_t max_val = 0;

            for (int i = 0; i < LEPTON_IMAGE_SIZE_3; i++)
            {
                if (frame_buffer[i] < min_val) min_val = frame_buffer[i];
                if (frame_buffer[i] > max_val) max_val = frame_buffer[i];
            }

            if (frame_count % 10 == 0)
            {
                float avg_capture_time = total_capture_time / 10.0f;
                float fps = 1000.0f / avg_capture_time;
                
                ESP_LOGI(TAG, "Frame #%lu | Time: %.1f ms (%.2f FPS) | Range: %u - %u", 
                         frame_count, avg_capture_time, fps, min_val, max_val);

                total_capture_time = 0;
            }
        }
        else
        {
            ESP_LOGW(TAG, "Frame capture failed: %d", error);
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    free(frame_buffer);
    Lepton_Deinit(&device);
}