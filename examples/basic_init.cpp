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

static i2c_master_bus_handle_t I2C_Handle = NULL;
static const char *TAG = "Lepton-Example";

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-Lepton Minimal Example");
    ESP_LOGI(TAG, "============================");
    ESP_LOGI(TAG, "Waiting for Lepton to boot...");

    vTaskDelay(pdMS_TO_TICKS(2000));

    Lepton_Conf_t Config;
    Config = LEPTON_DEFAULT_CONF;
    LEPTON_ASSIGN_FUNC(Config, I2CM_Init, I2CM_Deinit, I2CM_Write, I2CM_Read);

    // ...

    LEPTON_ASSIGN_I2C_HANDLE(Config, I2C_Handle);

    Lepton_t Device;
    Lepton_Error_t Error = Lepton_Init(&Device, &Config);
    if (Error != LEPTON_ERR_OK) {
        ESP_LOGE(TAG, "Failed to initialize Lepton: %d", Error);
        ESP_LOGE(TAG, "Check wiring and power supply!");
        return;
    }

    ESP_LOGI(TAG, "Lepton initialized successfully!");

    Lepton_Deinit(&Device);
}