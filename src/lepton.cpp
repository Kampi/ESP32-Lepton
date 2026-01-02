/*
 * lepton.cpp
 *
 *  Copyright (C) Daniel Kampert, 2026
 *  Website: www.kampis-elektroecke.de
 *  File info: Lepton driver implementation.
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
#include <esp_heap_caps.h>
#include <esp_task_wdt.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include <string.h>

#include "lepton.h"
#include "cci.h"
#include "vospi.h"

#include <sdkconfig.h>

static const char *TAG = "Lepton";

Lepton_Error_t Lepton_Init(Lepton_t *p_Device, const Lepton_Conf_t *const p_Init, Lepton_Result_t *p_Status)
{
    bool useTelemetry;
    Lepton_Error_t Error;

    if ((p_Device == NULL) || (p_Init == NULL) || (p_Init->CCI.I2C_Read == NULL) || (p_Init->CCI.I2C_Write == NULL)) {
        return LEPTON_ERR_INVALID_ARG;
    } else if (p_Device->Internal.isInitialized) {
        return LEPTON_ERR_OK;
    }

    Error = LEPTON_ERR_OK;
    p_Device->Internal.CCI = p_Init->CCI;
    p_Device->Internal.VoSPI = p_Init->VoSPI;
    p_Device->Internal.VoSPI.SyncErrors = 0;
    p_Device->Internal.VoSPI.isCapturing = false;
    p_Device->Internal.isRadiometric = false;
    p_Device->Internal.VSync = p_Init->VSync;
    p_Device->Internal.Reset = p_Init->Reset;
    p_Device->Internal.PowerDown = p_Init->PowerDown;
    p_Device->Internal.isInitialized = false;

    ESP_LOGD(TAG, "Lepton configuration:");
    ESP_LOGD(TAG, " V-Sync: %i", p_Init->VSync);
    ESP_LOGD(TAG, " SPI:");
    ESP_LOGD(TAG, "  Interface: %u", static_cast<unsigned int>(p_Device->Internal.VoSPI.Host));
    ESP_LOGD(TAG, "  Clock: %u", static_cast<unsigned int>(p_Device->Internal.VoSPI.Interface.clock_speed_hz));
    ESP_LOGD(TAG, "  SCLK: %u", static_cast<unsigned int>(p_Device->Internal.VoSPI.Master.sclk_io_num));
    ESP_LOGD(TAG, "  MISO: %u", static_cast<unsigned int>(p_Device->Internal.VoSPI.Master.miso_io_num));
    ESP_LOGD(TAG, "  CS: %u", static_cast<unsigned int>(p_Device->Internal.VoSPI.Interface.spics_io_num));
    ESP_LOGD(TAG, "  DMA channel: %u", static_cast<unsigned int>(p_Device->Internal.VoSPI.DMA));

    if (p_Device->Internal.Reset != NULL) {
        Lepton_HardReset(p_Device);
    }

    if (p_Device->Internal.PowerDown != NULL) {
        Lepton_EnablePowerDown(p_Device, false);
    }

    /* The application must wait at least 950 ms after deasserting the reset */
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    LEPTON_ERROR_CHECK(CCI_Init(&p_Device->Internal.CCI));

    if (CCI_WaitForBoot(&p_Device->Internal.CCI, p_Status) != LEPTON_ERR_OK) {
        Error = LEPTON_ERR_TIMEOUT;

        goto Lepton_Init_Error_1;
    }

    LEPTON_ERROR_CHECK(CCI_GetPartNumber(&p_Device->Internal.CCI, p_Device->PartNumber, p_Status));
    if (strncmp(p_Device->PartNumber, "500-0771-01", sizeof(p_Device->PartNumber)) == 0) {
        ESP_LOGD(TAG, "  Radiometric Lepton 3.5");
        p_Device->Internal.isRadiometric = true;
    } else if (strncmp(p_Device->PartNumber, "500-0726-01", sizeof(p_Device->PartNumber)) == 0) {
        ESP_LOGD(TAG, "  Non-radiometric Lepton 3.0");
        p_Device->Internal.isRadiometric = false;
    } else {
        ESP_LOGE(TAG, "  Unsupported Module! %s", p_Device->PartNumber);
        Error = LEPTON_ERR_UNKNOWN_DEVICE;

        goto Lepton_Init_Error_1;
    }

    if ((CCI_GetSerialNumber(&p_Device->Internal.CCI, p_Device->SerialNumber, p_Status) != LEPTON_ERR_OK) ||
        (CCI_SetVideoSource(&p_Device->Internal.CCI, LEPTON_SOURCE_COOKED, 0, p_Status) != LEPTON_ERR_OK)) {
        Error = LEPTON_ERR_FAIL;

        goto Lepton_Init_Error_1;
    }

    LEPTON_ERROR_CHECK(CCI_GetSoftwareVersion(&p_Device->Internal.CCI, &p_Device->SoftwareVersion, p_Status));

    ESP_LOGD(TAG, "Sync configuration with Lepton:");
    if ((CCI_SetRadiometry(&p_Device->Internal.CCI, p_Device->Internal.isRadiometric, p_Status) != LEPTON_ERR_OK) ||
        (CCI_GetRadiometry(&p_Device->Internal.CCI, &p_Device->Internal.isRadiometric, p_Status) != LEPTON_ERR_OK)) {
        goto Lepton_Init_Error_1;
    }
    ESP_LOGD(TAG, " Lepton Radiometry: %u", p_Device->Internal.isRadiometric);

    /* Make sure telemetry is disabled when using RGB888 format */
    useTelemetry = p_Init->VoSPI.useTelemetry;
    if (p_Init->VideoFormat == LEPTON_FORMAT_RGB888) {
        useTelemetry = false;
    }

    if ((CCI_SetTelemetry(&p_Device->Internal.CCI, useTelemetry, p_Status) != LEPTON_ERR_OK) ||
        (CCI_GetTelemetry(&p_Device->Internal.CCI, &p_Device->Internal.VoSPI.useTelemetry, p_Status) != LEPTON_ERR_OK)) {
        goto Lepton_Init_Error_1;
    }
    ESP_LOGD(TAG, " Use Telemetry: %s", p_Device->Internal.VoSPI.useTelemetry ? "true" : "false");

    if ((CCI_SetTelemetryPosition(&p_Device->Internal.CCI, p_Init->VoSPI.TelemetryPosition, p_Status) != LEPTON_ERR_OK) ||
        (CCI_GetTelemetryPosition(&p_Device->Internal.CCI, &p_Device->Internal.VoSPI.TelemetryPosition,
                                  p_Status) != LEPTON_ERR_OK)) {
        goto Lepton_Init_Error_1;
    }
    ESP_LOGD(TAG, " Telemetry Position: %u", p_Device->Internal.VoSPI.TelemetryPosition);

   if ((CCI_SetTLinearEnabled(&p_Device->Internal.CCI, p_Init->useTLinear, p_Status) != LEPTON_ERR_OK) ||
       (CCI_GetTLinearEnabled(&p_Device->Internal.CCI, &p_Device->Internal.useTLinear, p_Status) != LEPTON_ERR_OK)) {
        goto Lepton_Init_Error_1;
    }
    ESP_LOGD(TAG, " T-Linear enabled: %s", p_Device->Internal.useTLinear ? "true" : "false");

    if ((CCI_SetRadiometryTLinearAutoRes(&p_Device->Internal.CCI, p_Init->useTLinear, p_Status) != LEPTON_ERR_OK) ||
        (CCI_GetRadiometryTLinearAutoRes(&p_Device->Internal.CCI, &p_Device->Internal.isTLinearAutoRes,
                                         p_Status) != LEPTON_ERR_OK)) {
        goto Lepton_Init_Error_1;
    }
    ESP_LOGD(TAG, " Radiometry Auto Resolution: %s", p_Device->Internal.isTLinearAutoRes ? "true" : "false");

    if ((CCI_SetAGCEnabled(&p_Device->Internal.CCI, p_Init->useAGC, p_Status) != LEPTON_ERR_OK) ||
        (CCI_GetAGCEnabled(&p_Device->Internal.CCI, &p_Device->Internal.useAGC, p_Status) != LEPTON_ERR_OK)) {
        goto Lepton_Init_Error_1;
    }
    ESP_LOGD(TAG, " AGC enabled: %s", p_Device->Internal.useAGC ? "true" : "false");

    if ((CCI_SetAGCCalc(&p_Device->Internal.CCI, p_Init->useAGCCalculation, p_Status) != LEPTON_ERR_OK) ||
        (CCI_GetAGCCalc(&p_Device->Internal.CCI, &p_Device->Internal.useAGCCalc, p_Status) != LEPTON_ERR_OK)) {
        goto Lepton_Init_Error_1;
    }
    ESP_LOGD(TAG, " Use AGC calculation: %s", p_Device->Internal.useAGCCalc ? "true" : "false");

    if ((CCI_SetAGCPolicy(&p_Device->Internal.CCI, p_Init->AGCPolicy, p_Status) != LEPTON_ERR_OK) ||
        (CCI_GetAGCPolicy(&p_Device->Internal.CCI, &p_Device->Internal.AGCPolicy, p_Status) != LEPTON_ERR_OK)) {
        goto Lepton_Init_Error_1;
    }
    ESP_LOGD(TAG, " AGC Policy: %u", p_Device->Internal.AGCPolicy);

    if ((CCI_SetGainMode(&p_Device->Internal.CCI, p_Init->Gain, p_Status) != LEPTON_ERR_OK) ||
        (CCI_GetGainMode(&p_Device->Internal.CCI, &p_Device->Internal.Gain, p_Status) != LEPTON_ERR_OK)) {
        goto Lepton_Init_Error_1;
    }
    ESP_LOGD(TAG, " Gain Mode: %u", p_Device->Internal.Gain);

    if (CCI_GetVideoFreeze(&p_Device->Internal.CCI, &p_Device->Internal.isVideoFreezeEnabled, p_Status) != LEPTON_ERR_OK) {
        goto Lepton_Init_Error_1;
    }
    ESP_LOGD(TAG, " Video Freeze enabled: %s", p_Device->Internal.isVideoFreezeEnabled ? "true" : "false");

    if (p_Device->Internal.isVideoFreezeEnabled) {
        ESP_LOGD(TAG, " Disabling Video Freeze...");
        if (CCI_SetVideoFreeze(&p_Device->Internal.CCI, false, p_Status) != LEPTON_ERR_OK) {
            goto Lepton_Init_Error_1;
        }

        p_Device->Internal.isVideoFreezeEnabled = false;
    }

    p_Device->Internal.isInitialized = true;

    if (Error != LEPTON_ERR_OK) {
        goto Lepton_Init_Error_1;
    }

#if CONFIG_LEPTON_GPIO_VSYNC_PIN >= 0
    Error = CCI_SetGPIOMode(&p_Device->Internal.CCI, LEPTON_OEM_GPIO_MODE_VSYNC, p_Status);
    if (Error != LEPTON_ERR_OK) {
        goto Lepton_Init_Error_1;
    }
#endif

    Error = Lepton_SetVideoFormat(p_Device, p_Init->VideoFormat, p_Status);

    return Error;

Lepton_Init_Error_1:
    CCI_Deinit(&p_Device->Internal.CCI);

    p_Device->Internal.isInitialized = false;

    return Error;
}

void Lepton_Deinit(Lepton_t *p_Device)
{
    if ((p_Device == NULL) || (p_Device->Internal.isInitialized == false)) {
        return;
    }

    if (Lepton_isCapturing(p_Device)) {
        Lepton_StopCapture(p_Device);
    }

    CCI_Deinit(&p_Device->Internal.CCI);
    VoSPI_Deinit(&p_Device->Internal.VoSPI);
}

void Lepton_HardReset(Lepton_t *p_Device)
{
    if ((p_Device == NULL) || (p_Device->Internal.Reset == NULL)) {
        return;
    }

    p_Device->Internal.Reset(true);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    p_Device->Internal.Reset(false);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}

void Lepton_EnablePowerDown(Lepton_t *p_Device, bool Enable)
{
    if ((p_Device == NULL) || (p_Device->Internal.PowerDown == NULL)) {
        return;
    }

    p_Device->Internal.PowerDown(Enable);
}

Lepton_Error_t Lepton_SetVideoFormat(Lepton_t *p_Device, Lepton_VideoFormat_t Format, Lepton_Result_t *p_Status)
{
    Lepton_Error_t Error;

    if (p_Device == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    } else if (p_Device->Internal.VoSPI.isCapturing) {
        return LEPTON_ERR_BUSY;
    } else if (p_Device->Internal.VideoFormat == Format) {
        return LEPTON_ERR_OK;
    }

    Error = CCI_SetVideoFormat(&p_Device->Internal.CCI, Format, p_Status);
    if (Error != LEPTON_ERR_OK) {
        return Error;
    }

    Error = CCI_GetVideoFormat(&p_Device->Internal.CCI, &p_Device->Internal.VideoFormat, p_Status);
    if (Error != LEPTON_ERR_OK) {
        return LEPTON_ERR_FAIL;
    }

    p_Device->Internal.VoSPI.ImageWidth = 160;
    p_Device->Internal.VoSPI.ImageHeight = 120;
    p_Device->Internal.VoSPI.PacketsPerFrame = 60;

    if (Format == LEPTON_FORMAT_RAW14) {
        p_Device->Internal.VoSPI.BytesPerPixel = 2;

        if (p_Device->Internal.VoSPI.useTelemetry) {
            p_Device->Internal.VoSPI.PacketsPerFrame = 61;
        }
    } else if (Format == LEPTON_FORMAT_RGB888) {
        p_Device->Internal.VoSPI.BytesPerPixel = 3;

        if (p_Device->Internal.VoSPI.useTelemetry) {
            ESP_LOGE(TAG, "Please disable telemetry!");
            return LEPTON_ERR_INVALID_ARG;
        }
    } else {
        ESP_LOGE(TAG, "Unsupported video format!");
        return LEPTON_ERR_INVALID_ARG;
    }

    ESP_LOGD(TAG, "Video Format: %u", p_Device->Internal.VideoFormat);
    ESP_LOGD(TAG, " Width: %u", p_Device->Internal.VoSPI.ImageWidth);
    ESP_LOGD(TAG, " Height: %u", p_Device->Internal.VoSPI.ImageHeight);
    ESP_LOGD(TAG, " Bytes Per Pixel: %u", p_Device->Internal.VoSPI.BytesPerPixel);
    ESP_LOGD(TAG, " Packets Per Frame: %u", p_Device->Internal.VoSPI.PacketsPerFrame);

    /* We must reinitialize the VoSPI interface to apply images changes */
    VoSPI_Deinit(&p_Device->Internal.VoSPI);
    return VoSPI_Init(&p_Device->Internal.VoSPI);
}

Lepton_Error_t Lepton_EnableTelemetry(Lepton_t *p_Device, bool Enable, Lepton_Result_t *p_Status)
{
    Lepton_Error_t Error;

    if (p_Device == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    } else if (p_Device->Internal.VoSPI.isCapturing) {
        return LEPTON_ERR_BUSY;
    } else if (p_Device->Internal.VideoFormat == LEPTON_FORMAT_RGB888) {
        return LEPTON_ERR_NOT_SUPPORTED;
    } else if (p_Device->Internal.VoSPI.useTelemetry == Enable) {
        return LEPTON_ERR_OK;
    }

    Error = CCI_SetTelemetry(&p_Device->Internal.CCI, Enable, p_Status);
    if (Error != LEPTON_ERR_OK) {
        return Error;
    }

    Error = CCI_GetTelemetry(&p_Device->Internal.CCI, &p_Device->Internal.VoSPI.useTelemetry, p_Status);
    if (Error != LEPTON_ERR_OK) {
        return Error;
    }

    if (p_Device->Internal.VideoFormat == LEPTON_FORMAT_RAW14) {
        if (p_Device->Internal.VoSPI.useTelemetry) {
            p_Device->Internal.VoSPI.PacketsPerFrame = 61;
        } else {
            p_Device->Internal.VoSPI.PacketsPerFrame = 60;
        }
    } else {
        ESP_LOGE(TAG, "Unsupported video format!");
        return LEPTON_ERR_INVALID_ARG;
    }

    ESP_LOGD(TAG, "Use telemetry: %s", p_Device->Internal.VoSPI.useTelemetry ? "true" : "false");

    /* We must reinitialize the VoSPI interface to apply telemetry changes */
    VoSPI_Deinit(&p_Device->Internal.VoSPI);
    return VoSPI_Init(&p_Device->Internal.VoSPI);
}

Lepton_Error_t Lepton_GetPixelTemperature(Lepton_t *p_Device, uint16_t PixelValue, float *p_Temperature)
{
    float TemperatureKelvin;
    Lepton_TLinear_Resolution_t Resolution;

    if ((p_Device == NULL) || (p_Temperature == NULL)) {
        return LEPTON_ERR_INVALID_ARG;
    } else if (p_Device->Internal.isInitialized == false) {
        return LEPTON_ERR_NOT_INITIALIZED;
    } else if (p_Device->Internal.isRadiometric == false) {
        return LEPTON_ERR_NOT_SUPPORTED;
    } else if (p_Device->Internal.useTLinear == false) {
        /* TLinear must be enabled for temperature conversion */
        return LEPTON_ERR_NOT_SUPPORTED;
    }

    LEPTON_ERROR_CHECK(CCI_GetTLinearResolution(&p_Device->Internal.CCI, &Resolution));

    /* Convert raw pixel value to temperature in Celsius using TLinear mode
     * 
     * For Lepton 3.5 radiometric with TLinear enabled:
     * - Pixel values are in centi-Kelvin (Kelvin * 100)
     * - Resolution depends on TLinear resolution setting:
     *   - 0.1K resolution: value / 10 = Kelvin
     *   - 0.01K resolution: value / 100 = Kelvin
     * 
     * The default TLinear resolution is 0.1K (scale factor 10)
     */

    if (Resolution == LEPTON_TLINEAR_0_01_RESOLUTION) {
        /* With auto resolution, use 0.01K (scale factor 100) */
        TemperatureKelvin = static_cast<float>(PixelValue) / 100.0f;
    } else if (Resolution == LEPTON_TLINEAR_0_1_RESOLUTION) {
        /* Standard resolution is 0.1K (scale factor 10) */
        TemperatureKelvin = static_cast<float>(PixelValue) / 10.0f;
    } else {
        return LEPTON_ERR_FAIL;
    }

    /* Convert Kelvin to Celsius */
    *p_Temperature = TemperatureKelvin - 273.15f;

    return LEPTON_ERR_OK;
}