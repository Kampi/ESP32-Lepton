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

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include <string.h>

#include "lepton.h"
#include "cci.h"
#include "vospi.h"

#include <sdkconfig.h>

#define STRINGIFY(s)                            STR(s)
#define STR(s)                                  #s

static const char* TAG 							= "Lepton";

Lepton_Error_t Lepton_Init(Lepton_t* p_Device, const Lepton_Conf_t* const p_Init, Lepton_Result_t* p_Status)
{
    Lepton_Error_t Error;

    if((p_Device == NULL) || (p_Init == NULL) || (p_Init->CCI.I2C_Init == NULL) || (p_Init->CCI.I2C_Deinit == NULL) || (p_Init->CCI.I2C_Read == NULL) || (p_Init->CCI.I2C_Write == NULL))
    {
        return LEPTON_ERR_INVALID_ARG;
    }
    else if(p_Device->Internal.isInitialized)
    {
        return LEPTON_ERR_OK;
    }

    p_Device->Internal.CCI = p_Init->CCI;
    p_Device->Internal.VoSPI = p_Init->VoSPI;
    p_Device->Internal.VoSPI.SyncErrors = 0;
    p_Device->Internal.isRadiometric = false;
    p_Device->Internal.VSync = p_Init->VSync;
    p_Device->Internal.Reset = p_Init->Reset;
    p_Device->Internal.PowerDown = p_Init->PowerDown;
    p_Device->Internal.VideoFormat = p_Init->VideoFormat;
    p_Device->Internal.isInitialized = false;
    p_Device->Internal.isCapturing = false;

    ESP_LOGI(TAG, "Lepton configuration:");
    ESP_LOGI(TAG, " V-Sync: %i", p_Init->VSync);
    ESP_LOGI(TAG, " SPI:");
    ESP_LOGI(TAG, "  Interface: %u", static_cast<unsigned int>(p_Device->Internal.VoSPI.Host));
    ESP_LOGI(TAG, "  Clock: %u", static_cast<unsigned int>(p_Device->Internal.VoSPI.Interface.clock_speed_hz));
    ESP_LOGI(TAG, "  SCLK: %u", static_cast<unsigned int>(p_Device->Internal.VoSPI.Master.sclk_io_num));
    ESP_LOGI(TAG, "  MISO: %u", static_cast<unsigned int>(p_Device->Internal.VoSPI.Master.miso_io_num));
    ESP_LOGI(TAG, "  CS: %u", static_cast<unsigned int>(p_Device->Internal.VoSPI.Interface.spics_io_num));
    ESP_LOGI(TAG, "  DMA channel: %u", static_cast<unsigned int>(p_Device->Internal.VoSPI.DMA));

    gpio_set_direction(static_cast<gpio_num_t>(p_Device->Internal.VoSPI.Interface.spics_io_num), GPIO_MODE_OUTPUT);

    if(p_Device->Internal.Reset != NULL)
    {
        Lepton_HardReset(p_Device);
    }

    if(p_Device->Internal.PowerDown != NULL)
    {
        Lepton_EnablePowerDown(p_Device, false);
    }

    /* The application must wait at least 950 ms after deasserting the reset */
    vTaskDelay(pdMS_TO_TICKS(1000));

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


    Lepton_VideoSource_t Source;
    CCI_SetVideoSource(&p_Device->Internal.CCI, LEPTON_SOURCE_RAMP_V);
    CCI_GetVideoSource(&p_Device->Internal.CCI, &Source);
    ESP_LOGI(TAG, "Source: %u", Source);


    ESP_LOGI(TAG, "Video Format: %u", p_Device->Internal.VideoFormat);
    Error = CCI_SetVideoFormat(&p_Device->Internal.CCI, p_Init->VideoFormat, p_Status) | CCI_GetVideoFormat(&p_Device->Internal.CCI, &p_Device->Internal.VideoFormat, p_Status);
    if(Error != LEPTON_ERR_OK)
    {
        goto Lepton_Init_Error_1;
    }

    ESP_LOGI(TAG, "Lepton Radiometry: %u", p_Device->Internal.isRadiometric);
    Error = CCI_SetRadiometry(&p_Device->Internal.CCI, p_Device->Internal.isRadiometric, p_Status) | CCI_GetRadiometry(&p_Device->Internal.CCI, &p_Device->Internal.isRadiometric, p_Status);
    if(Error != LEPTON_ERR_OK)
    {
        goto Lepton_Init_Error_1;
    }

        // TLinear depends on AGC
        /*
        val = (lep_stP->agc_set_enabled) ? CCI_RADIOMETRY_TLINEAR_DISABLED : CCI_RADIOMETRY_TLINEAR_ENABLED;
        cci_set_radiometry_tlinear_enable_state(val);
        rsp = cci_get_radiometry_tlinear_enable_state();
        ESP_LOGI(TAG, "Lepton Radiometry TLinear = %d", rsp);
        if (rsp != val) {
            ESP_LOGE(TAG, "Lepton communication failed (%d)", rsp);
              return ESP_FAIL;
        }*/

    Error = CCI_SetRadiometryTLinearAutoRes(&p_Device->Internal.CCI, true, p_Status) | CCI_GetRadiometryTLinearAutoRes(&p_Device->Internal.CCI, &p_Device->Internal.isTLinearAutoRes, p_Status);
    ESP_LOGI(TAG, "Lepton Radiometry Auto Resolution: %u", p_Device->Internal.isTLinearAutoRes);
    if(Error != LEPTON_ERR_OK)
    {
        goto Lepton_Init_Error_1;
    }

    Error = CCI_SetAGCCalc(&p_Device->Internal.CCI, p_Init->UseAGCCalculation, p_Status) | CCI_GetAGCCalc(&p_Device->Internal.CCI, &p_Device->Internal.isAGCCalc, p_Status);
    ESP_LOGI(TAG, "Use AGC calculation: %u...", p_Device->Internal.isAGCCalc);
    if(Error != LEPTON_ERR_OK)
    {
        goto Lepton_Init_Error_1;
    }

    Error = CCI_SetAGC(&p_Device->Internal.CCI, p_Init->UseAGC, p_Status) | CCI_GetAGC(&p_Device->Internal.CCI, &p_Device->Internal.isAGC, p_Status);
    ESP_LOGI(TAG, "Use AGC: %u...", p_Device->Internal.isAGC);
    if(Error != LEPTON_ERR_OK)
    {
        goto Lepton_Init_Error_1;
    }

    Error = CCI_SetTelemetry(&p_Device->Internal.CCI, p_Init->UseTelemetry, p_Status) | CCI_GetTelemetry(&p_Device->Internal.CCI, &p_Device->Internal.useTelemetry, p_Status);
    ESP_LOGI(TAG, "Use telemetry: %u...", p_Device->Internal.useTelemetry);
    if(Error != LEPTON_ERR_OK)
    {
        goto Lepton_Init_Error_1;
    }

    Error = CCI_SetGainMode(&p_Device->Internal.CCI, p_Init->Gain, p_Status) | CCI_GetGainMode(&p_Device->Internal.CCI, &p_Device->Internal.Gain, p_Status);
    ESP_LOGI(TAG, "Lepton Gain Mode: %u", p_Device->Internal.Gain);
    if(Error != LEPTON_ERR_OK)
    {
        goto Lepton_Init_Error_1;
    }

    if(p_Device->Internal.isRadiometric)
    {
        Error = Lepton_Emissivity(p_Device, 100);
        if(Error != LEPTON_ERR_OK)
        {
            goto Lepton_Init_Error_1;
        }
    }

    Error = CCI_SetGPIOMode(&p_Device->Internal.CCI, LEPTON_OEM_GPIO_MODE_VSYNC, p_Status);
    if(Error != LEPTON_ERR_OK)
    {
        goto Lepton_Init_Error_1;
    }

    Error = VoSPI_Init(&p_Device->Internal.VoSPI, p_Device->Internal.useTelemetry);
    if(Error != LEPTON_ERR_OK)
    {
        goto Lepton_Init_Error_2;
    }

    p_Device->Internal.isInitialized = true;

    return LEPTON_ERR_OK;

Lepton_Init_Error_2:
    VoSPI_Deinit(&p_Device->Internal.VoSPI);

Lepton_Init_Error_1:
    gpio_reset_pin(static_cast<gpio_num_t>(p_Device->Internal.VoSPI.Interface.spics_io_num));

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
    vTaskDelay(pdMS_TO_TICKS(1000));
    p_Device->Internal.Reset(false);
    vTaskDelay(pdMS_TO_TICKS(1000));
}

void Lepton_EnablePowerDown(Lepton_t* p_Device, bool Enable)
{
    if((p_Device == NULL) || (p_Device->Internal.PowerDown == NULL))
    {
        return;
    }

    p_Device->Internal.PowerDown(Enable);
}

void Lepton_GetColor(double Value, double Min, double Max, uint8_t* p_R, uint8_t* p_G, uint8_t* p_B)
{
    // Start with white.
    double Color[3] = {1.0, 1.0, 1.0};
    double Value_Temp;
    double Diff;

    Value_Temp = Value;
    if(Value < Min)
    {
          Value_Temp = Min;
    }
       else if(Value > Max)
    {
          Value_Temp = Max;
    }

       Diff = Max - Min;

    if(Value_Temp < (Min + (Diff * 0.25)))
    {
        Color[0] = 0;
        Color[1] = 4.0 * (Value_Temp - Min) / Diff;
    }
    else if(Value < (Min + (Diff * 0.5)))
    {
        Color[0] = 0;
        Color[2] = 1.0 + 4.0 * (Min + (0.25 * Diff) - Value_Temp) / Diff;
    }
    else if(Value_Temp < (Min + (0.75 * Diff)))
    {
        Color[0] = 4.0 * (Value_Temp - Min - (0.5 * Diff)) / Diff;
        Color[2] = 0.0;
    }
    else
    {
        Color[1] = 1 + 4 * (Min + (0.75 * Diff) - Value_Temp) / Diff;
        Color[2] = 0.0;
    }
/*
    ESP_LOGI(TAG, "Value: %.2f", Value);
    ESP_LOGI(TAG, "Max: %.2f", Max);
    ESP_LOGI(TAG, "Min: %.2f", Min);
    ESP_LOGI(TAG, "Diff: %.2f", Diff);
    ESP_LOGI(TAG, "Red: %.2f", Color[0]);
    ESP_LOGI(TAG, "Green: %.2f", Color[0]);
    ESP_LOGI(TAG, "Blue: %.2f", Color[0]);
*/
    *p_R = Color[0] * 0x1F;
    *p_G = Color[1] * 0x3F;
    *p_B = Color[2] * 0x1F;
}