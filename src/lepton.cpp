 /*
 * lepton.cpp
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

static const char* TAG = "Lepton";

Lepton_Error_t Lepton_Init(Lepton_t* p_Device, const Lepton_Conf_t* const p_Init, Lepton_Result_t* p_Status)
{
    Lepton_Error_t Error;

    if((p_Device == NULL) || (p_Init == NULL) || (p_Init->CCI.I2C_Read == NULL) || (p_Init->CCI.I2C_Write == NULL))
    {
        return LEPTON_ERR_INVALID_ARG;
    }
    else if(p_Device->Internal.isInitialized)
    {
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

    if(p_Device->Internal.Reset != NULL)
    {
        Lepton_HardReset(p_Device);
    }

    if(p_Device->Internal.PowerDown != NULL)
    {
        Lepton_EnablePowerDown(p_Device, false);
    }

    /* The application must wait at least 950 ms after deasserting the reset */
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    LEPTON_ERROR_CHECK(CCI_Init(&p_Device->Internal.CCI));

    if(CCI_WaitForBoot(&p_Device->Internal.CCI, p_Status) != LEPTON_ERR_OK)
    {
        Error = LEPTON_ERR_TIMEOUT;

        goto Lepton_Init_Error_1;
    }

    LEPTON_ERROR_CHECK(CCI_GetPartNumber(&p_Device->Internal.CCI, p_Device->PartNumber, p_Status));
    if(strncmp(p_Device->PartNumber, "500-0771-01", 32) == 0)
    {
        ESP_LOGD(TAG, "  Radiometric Lepton 3.5");
        p_Device->Internal.isRadiometric = true;
    }
    else if(strncmp(p_Device->PartNumber, "500-0726-01", 32) == 0)
    {
        ESP_LOGD(TAG, "  Non-radiometric Lepton 3.0");
        p_Device->Internal.isRadiometric = false;
    } 
    else
    {
        ESP_LOGE(TAG, "  Unsupported Module! %s", p_Device->PartNumber);
        Error = LEPTON_ERR_UNKNOWN_DEVICE;

        goto Lepton_Init_Error_1;
    }

    if((CCI_GetSerialNumber(&p_Device->Internal.CCI, p_Device->SerialNumber, p_Status) != LEPTON_ERR_OK) ||
       (CCI_SetVideoSource(&p_Device->Internal.CCI, LEPTON_SOURCE_COOKED) != LEPTON_ERR_OK))
    {
        Error = LEPTON_ERR_FAIL;

        goto Lepton_Init_Error_1;
    }

    ESP_LOGD(TAG, "Sync configuration with Lepton:");
    if((CCI_SetRadiometry(&p_Device->Internal.CCI, p_Device->Internal.isRadiometric, p_Status) != LEPTON_ERR_OK) ||
       (CCI_GetRadiometry(&p_Device->Internal.CCI, &p_Device->Internal.isRadiometric, p_Status) != LEPTON_ERR_OK))
    {
        goto Lepton_Init_Error_1;
    }
    ESP_LOGD(TAG, " Lepton Radiometry: %u", p_Device->Internal.isRadiometric);

        // TLinear depends on AGC
        /*
        val = (lep_stP->agc_set_enabled) ? CCI_RADIOMETRY_TLINEAR_DISABLED : CCI_RADIOMETRY_TLINEAR_ENABLED;
        cci_set_radiometry_tlinear_enable_state(val);
        rsp = cci_get_radiometry_tlinear_enable_state();
        ESP_LOGD(TAG, "Lepton Radiometry TLinear = %d", rsp);
        if (rsp != val) {
            ESP_LOGE(TAG, "Lepton communication failed (%d)", rsp);
              return ESP_FAIL;
        }*/

    if((CCI_SetTelemetry(&p_Device->Internal.CCI, p_Init->VoSPI.useTelemetry, p_Status) != LEPTON_ERR_OK) || 
       (CCI_GetTelemetry(&p_Device->Internal.CCI, &p_Device->Internal.VoSPI.useTelemetry, p_Status) != LEPTON_ERR_OK))
    {
        goto Lepton_Init_Error_1;
    }
    ESP_LOGD(TAG, " Use Telemetry: %s", p_Device->Internal.VoSPI.useTelemetry ? "true" : "false");

    if((CCI_SetTelemetryPosition(&p_Device->Internal.CCI, p_Init->VoSPI.TelemetryPosition, p_Status) != LEPTON_ERR_OK) ||
       (CCI_GetTelemetryPosition(&p_Device->Internal.CCI, &p_Device->Internal.VoSPI.TelemetryPosition, p_Status) != LEPTON_ERR_OK))
    {
        goto Lepton_Init_Error_1;
    }
    ESP_LOGD(TAG, " Telemetry Position: %u", p_Device->Internal.VoSPI.TelemetryPosition);

    if((CCI_SetRadiometryTLinearAutoRes(&p_Device->Internal.CCI, p_Init->useTLinear, p_Status) != LEPTON_ERR_OK) ||
       (CCI_GetRadiometryTLinearAutoRes(&p_Device->Internal.CCI, &p_Device->Internal.isTLinearAutoRes, p_Status) != LEPTON_ERR_OK))
    {
        goto Lepton_Init_Error_1;
    }
    ESP_LOGD(TAG, " Radiometry Auto Resolution: %s", p_Device->Internal.isTLinearAutoRes ? "true" : "false");

    if((CCI_SetAGCCalc(&p_Device->Internal.CCI, p_Init->useAGCCalculation, p_Status) != LEPTON_ERR_OK) ||
       (CCI_GetAGCCalc(&p_Device->Internal.CCI, &p_Device->Internal.useAGCCalc, p_Status) != LEPTON_ERR_OK))
    {
        goto Lepton_Init_Error_1;
    }
    ESP_LOGD(TAG, " Use AGC calculation: %s", p_Device->Internal.useAGCCalc ? "true" : "false");

    if((CCI_SetAGC(&p_Device->Internal.CCI, p_Init->useAGC, p_Status) != LEPTON_ERR_OK) ||
       (CCI_GetAGC(&p_Device->Internal.CCI, &p_Device->Internal.useAGC, p_Status) != LEPTON_ERR_OK))
    {
        goto Lepton_Init_Error_1;
    }
    ESP_LOGD(TAG, " AGC enabled: %s", p_Device->Internal.useAGC ? "true" : "false");

    if((CCI_SetGainMode(&p_Device->Internal.CCI, p_Init->Gain, p_Status) != LEPTON_ERR_OK) ||
       (CCI_GetGainMode(&p_Device->Internal.CCI, &p_Device->Internal.Gain, p_Status) != LEPTON_ERR_OK))
    {
        goto Lepton_Init_Error_1;
    }
    ESP_LOGD(TAG, " Gain Mode: %u", p_Device->Internal.Gain);

    p_Device->Internal.isInitialized = true;

    Error = Lepton_SetVideoFormat(p_Device, p_Init->VideoFormat, p_Status);
    if(Error != LEPTON_ERR_OK)
    {
        goto Lepton_Init_Error_1;
    }

    /*
    if(p_Device->Internal.isRadiometric)
    {
        Error = Lepton_Emissivity(p_Device, 100);
        if(Error != LEPTON_ERR_OK)
        {
            goto Lepton_Init_Error_1;
        }
    }
*/

#if CONFIG_LEPTON_GPIO_VSYNC_PIN >= 0
    Error = CCI_SetGPIOMode(&p_Device->Internal.CCI, LEPTON_OEM_GPIO_MODE_VSYNC, p_Status);
    if(Error != LEPTON_ERR_OK)
    {
        goto Lepton_Init_Error_1;
    }
#endif

    return LEPTON_ERR_OK;

Lepton_Init_Error_1:
    CCI_Deinit(&p_Device->Internal.CCI);

    p_Device->Internal.isInitialized = false;

    return Error;
}

void Lepton_Deinit(Lepton_t* p_Device)
{
    if((p_Device == NULL) || (p_Device->Internal.isInitialized == false))
    {
        return;
    }

    if(Lepton_isCapturing(p_Device))
    {
        Lepton_StopCapture(p_Device);
    }

    CCI_Deinit(&p_Device->Internal.CCI);
    VoSPI_Deinit(&p_Device->Internal.VoSPI);
}

void Lepton_HardReset(Lepton_t* p_Device)
{
    if((p_Device == NULL) || (p_Device->Internal.Reset == NULL))
    {
        return;
    }

    p_Device->Internal.Reset(true);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    p_Device->Internal.Reset(false);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}

void Lepton_EnablePowerDown(Lepton_t* p_Device, bool Enable)
{
    if((p_Device == NULL) || (p_Device->Internal.PowerDown == NULL))
    {
        return;
    }

    p_Device->Internal.PowerDown(Enable);
}

Lepton_Error_t Lepton_SetVideoFormat(Lepton_t* p_Device, Lepton_VideoFormat_t Format, Lepton_Result_t* p_Status)
{
    Lepton_Error_t Error;

    if(p_Device == NULL)
    {
        return LEPTON_ERR_INVALID_ARG;
    }
    else if(p_Device->Internal.VoSPI.isCapturing)
    {
        return LEPTON_ERR_BUSY;
    }
    else if(p_Device->Internal.VideoFormat == Format)
    {
        return LEPTON_ERR_OK;
    }

    Error = CCI_SetVideoFormat(&p_Device->Internal.CCI, Format, p_Status);
    if(Error != LEPTON_ERR_OK)
    {
        return Error;
    }

    Error = CCI_GetVideoFormat(&p_Device->Internal.CCI, &p_Device->Internal.VideoFormat, p_Status);
    if(Error != LEPTON_ERR_OK)
    {
        return LEPTON_ERR_FAIL;
    }

    if(Format == LEPTON_FORMAT_RAW14)
    {
        p_Device->Internal.VoSPI.ImageWidth = 160;
        p_Device->Internal.VoSPI.ImageHeight = 120;
        p_Device->Internal.VoSPI.BytesPerPixel = 2;

        if(p_Device->Internal.VoSPI.useTelemetry)
        {
            p_Device->Internal.VoSPI.PacketsPerFrame = 61;
        }
        else
        {
            p_Device->Internal.VoSPI.PacketsPerFrame = 60;
        }
    }
    else if(Format == LEPTON_FORMAT_RGB888)
    {
        p_Device->Internal.VoSPI.ImageWidth = 240;
        p_Device->Internal.VoSPI.ImageHeight = 120;
        p_Device->Internal.VoSPI.BytesPerPixel = 3;
        p_Device->Internal.VoSPI.PacketsPerFrame = 60;

        if(p_Device->Internal.VoSPI.useTelemetry)
        {
            ESP_LOGE(TAG, "Please disable telemtry!");
            return LEPTON_ERR_INVALID_ARG;
        }
    }
    else
    {
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

Lepton_Error_t Lepton_EnableTelemetry(Lepton_t* p_Device, bool Enable, Lepton_Result_t* p_Status)
{
    Lepton_Error_t Error;

    if(p_Device == NULL)
    {
        return LEPTON_ERR_INVALID_ARG;
    }
    else if(p_Device->Internal.VoSPI.isCapturing)
    {
        return LEPTON_ERR_BUSY;
    }
    else if(p_Device->Internal.VideoFormat == LEPTON_FORMAT_RGB888)
    {
        return LEPTON_ERR_NOT_SUPPORTED;
    }
    else if(p_Device->Internal.VoSPI.useTelemetry == Enable)
    {
        return LEPTON_ERR_OK;
    }

    Error = CCI_SetTelemetry(&p_Device->Internal.CCI, Enable, p_Status);
    if(Error != LEPTON_ERR_OK)
    {
        return Error;
    }
    
    Error = CCI_GetTelemetry(&p_Device->Internal.CCI, &p_Device->Internal.VoSPI.useTelemetry, p_Status);
    if(Error != LEPTON_ERR_OK)
    {
        return Error;
    }

    if(p_Device->Internal.VideoFormat == LEPTON_FORMAT_RAW14)
    {
        if(p_Device->Internal.VoSPI.useTelemetry)
        {
            p_Device->Internal.VoSPI.PacketsPerFrame = 61;
        }
        else
        {
            p_Device->Internal.VoSPI.PacketsPerFrame = 60;
        }
    }
    else
    {
        ESP_LOGE(TAG, "Unsupported video format!");
        return LEPTON_ERR_INVALID_ARG;
    }

    ESP_LOGD(TAG, "Use telemetry: %s", p_Device->Internal.VoSPI.useTelemetry ? "true" : "false");

    /* We must reinitialize the VoSPI interface to apply telemetry changes */
    VoSPI_Deinit(&p_Device->Internal.VoSPI);
    return VoSPI_Init(&p_Device->Internal.VoSPI);
}
