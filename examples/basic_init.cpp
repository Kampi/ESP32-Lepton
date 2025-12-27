/*
 * basic_init.cpp
 *
 *  Copyright (C) Daniel Kampert, 2025
 *  Website: www.kampis-elektroecke.de
 *  File info: Basic initialization example for ESP32-Lepton component.
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

static i2c_master_bus_handle_t I2C_Handle = NULL;
static const char* TAG = "Lepton-Example";

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-Lepton Minimal Example");
    ESP_LOGI(TAG, "============================");
    ESP_LOGI(TAG, "Waiting for Lepton to boot...");

    vTaskDelay(2000 / portTICK_PERIOD_MS);

    Lepton_Conf_t config;
    config = LEPTON_DEFAULT_CONF;
    LEPTON_ASSIGN_FUNC(config, I2CM_Init, I2CM_Deinit, I2CM_Write, I2CM_Read);

    // ...

    LEPTON_ASSIGN_I2C_HANDLE(config, I2C_Handle);

    Lepton_t device;
    Lepton_Error_t error = Lepton_Init(&device, &config);
    if (error != LEPTON_ERR_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize Lepton: %d", error);
        ESP_LOGE(TAG, "Check wiring and power supply!");
        return;
    }

    ESP_LOGI(TAG, "Lepton initialized successfully!");

    Lepton_Deinit(&device);
}