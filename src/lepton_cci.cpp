 /*
 * lepton_cci.cpp
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

#include "lepton.h"
#include "cci.h"

static const char* TAG 			= "Lepton-CCI";

Lepton_Error_t Lepton_GetTemp(Lepton_t* p_Device, uint16_t* p_FPA, uint16_t* p_AUX, Lepton_Result_t* p_Status)
{
    if(p_Device == NULL)
    {
        return LEPTON_ERR_INVALID_ARG;
    }

    LEPTON_ERROR_CHECK(CCI_GetAuxTemp(&p_Device->Internal.CCI, p_AUX, p_Status));

    return CCI_GetFPATemp(&p_Device->Internal.CCI, p_FPA, p_Status);
}

Lepton_Error_t Lepton_EnableAGC(Lepton_t* p_Device, bool Enable, Lepton_Result_t* p_Status)
{
    if(p_Device == NULL)
    {
        return LEPTON_ERR_INVALID_ARG;
    }

    if(Enable)
    {
        if(p_Device->Internal.isRadiometric)
        {
            LEPTON_ERROR_CHECK(CCI_SetTLinear(&p_Device->Internal.CCI, false, p_Status));

            p_Device->Internal.isTLinear = false;
        }

        LEPTON_ERROR_CHECK(CCI_SetAGC(&p_Device->Internal.CCI, true, p_Status));
        LEPTON_ERROR_CHECK(CCI_GetAGC(&p_Device->Internal.CCI, &p_Device->Internal.isAGC, p_Status));
    }
    else
    {
        if(p_Device->Internal.isRadiometric)
        {
            LEPTON_ERROR_CHECK(CCI_SetTLinear(&p_Device->Internal.CCI, true, p_Status));
            LEPTON_ERROR_CHECK(CCI_GetTLinear(&p_Device->Internal.CCI, &p_Device->Internal.isTLinear, p_Status));
        }

        LEPTON_ERROR_CHECK(CCI_SetAGC(&p_Device->Internal.CCI, false, p_Status));
        LEPTON_ERROR_CHECK(CCI_GetAGC(&p_Device->Internal.CCI, &p_Device->Internal.isAGC, p_Status));
    }

    return LEPTON_ERR_OK;
}

Lepton_Error_t Lepton_Emissivity(Lepton_t* p_Device, uint16_t Emissitivity, Lepton_Result_t* p_Status)
{
    Lepton_FluxLinearParams_t FluxValues;

    if(p_Device == NULL)
    {
        return LEPTON_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Set Emissivity: %u%%", Emissitivity);

    if(p_Device->Internal.isRadiometric)
    {
        // Scale percentage into Lepton scene emissivity values (1-100% -> 82-8192).
        if(Emissitivity < 1)
        {
            Emissitivity = 1;
        }

        if(Emissitivity > 100)
        {
            Emissitivity = 100;
        }

        FluxValues.sceneEmissivity = Emissitivity * 8192 / 100;

        // Set default (no lens) values for the remaining parameters.
        FluxValues.TBkgK      = 29515;
        FluxValues.tauWindow  = 8192;
        FluxValues.TWindowK   = 29515;
        FluxValues.tauAtm     = 8192;
        FluxValues.TAtmK      = 29515;
        FluxValues.reflWindow = 0;
        FluxValues.TReflK     = 29515;

        return CCI_SetRadiometryFluxLinearParams(&p_Device->Internal.CCI, &FluxValues, p_Status);
    }

    return LEPTON_ERR_INVALID_STATE;
}

uint32_t Lepton_GetUptime(Lepton_t* p_Device, Lepton_Result_t* p_Status)
{
    uint32_t Time;

    if((p_Device == NULL) || (CCI_GetUptime(&p_Device->Internal.CCI, &Time, p_Status) != LEPTON_ERR_OK))
    {
        return 0;
    }
    
    return Time;
}

Lepton_Error_t Lepton_GetSceneStatistics(Lepton_t* p_Device, Lepton_SceneStatistics_t* p_Statistics, Lepton_Result_t* p_Status)
{
    return CCI_GetSceneStatistics(&p_Device->Internal.CCI, p_Statistics, p_Status);
}

Lepton_Error_t Lepton_SetROI(Lepton_t* p_Device, Lepton_ROI_t ROI, Lepton_Result_t* p_Status)
{
    return CCI_SetROI(&p_Device->Internal.CCI, &ROI, p_Status);
}

Lepton_Error_t Lepton_GetROI(Lepton_t* p_Device, Lepton_ROI_t* p_ROI, Lepton_Result_t* p_Status)
{
    return CCI_GetROI(&p_Device->Internal.CCI, p_ROI, p_Status);
}