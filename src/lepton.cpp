/**
 * @file lepton.cpp
 * @brief ESP32 component for FLIR Lepton 3.5 Thermal Camera
 * @author Daniel Kampert
 * @version 1.0.0
 */

#include "esp_err.h"
#include "esp_log.h"

#include "lepton.h"

static const char* TAG = "Lepton";

esp_err_t Lepton_Init(void)
{
    ESP_LOGI(TAG, "Lepton camera driver initialized");
    return ESP_OK;
}

esp_err_t Lepton_Deinit(void)
{
    ESP_LOGI(TAG, "Lepton camera driver deinitialized");
    return ESP_OK;
}
