/*
 * lepton_cci.cpp
 *
 *  Copyright (C) Daniel Kampert, 2026
 *  Website: www.kampis-elektroecke.de
 *  File info: Lepton CCI command implementation.
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

#include "lepton.h"
#include "cci.h"

static const char *TAG          = "Lepton-CCI";

Lepton_Error_t Lepton_GetTemp(Lepton_t *p_Device, uint16_t *p_FPA, uint16_t *p_AUX, Lepton_Result_t *p_Status)
{
    if (p_Device == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    } else if (p_Device->Internal.isInitialized == false) {
        return LEPTON_ERR_NOT_INITIALIZED;
    }

    LEPTON_ERROR_CHECK(CCI_GetAuxTemp(&p_Device->Internal.CCI, p_AUX, p_Status));

    return CCI_GetFPATemp(&p_Device->Internal.CCI, p_FPA, p_Status);
}

Lepton_Error_t Lepton_EnableAGC(Lepton_t *p_Device, bool Enable, Lepton_Result_t *p_Status)
{
    if (p_Device == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    } else if (p_Device->Internal.isInitialized == false) {
        return LEPTON_ERR_NOT_INITIALIZED;
    }

    if (Enable) {
        if (p_Device->Internal.isRadiometric) {
            LEPTON_ERROR_CHECK(CCI_SetTLinearEnabled(&p_Device->Internal.CCI, false, p_Status));

            p_Device->Internal.useTLinear = false;
        }

        LEPTON_ERROR_CHECK(CCI_SetAGCEnabled(&p_Device->Internal.CCI, true, p_Status));
        LEPTON_ERROR_CHECK(CCI_GetAGCEnabled(&p_Device->Internal.CCI, &p_Device->Internal.useAGC, p_Status));
    } else {
        if (p_Device->Internal.isRadiometric) {
            LEPTON_ERROR_CHECK(CCI_SetTLinearEnabled(&p_Device->Internal.CCI, true, p_Status));
            LEPTON_ERROR_CHECK(CCI_GetTLinearEnabled(&p_Device->Internal.CCI, &p_Device->Internal.useTLinear, p_Status));
        }

        LEPTON_ERROR_CHECK(CCI_SetAGCEnabled(&p_Device->Internal.CCI, false, p_Status));
        LEPTON_ERROR_CHECK(CCI_GetAGCEnabled(&p_Device->Internal.CCI, &p_Device->Internal.useAGC, p_Status));
    }

    return LEPTON_ERR_OK;
}

Lepton_Error_t Lepton_Emissivity(Lepton_t *p_Device, uint16_t Emissivity, Lepton_Result_t *p_Status)
{
    Lepton_FluxLinearParams_t FluxValues;

    if (p_Device == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    } else if (p_Device->Internal.isInitialized == false) {
        return LEPTON_ERR_NOT_INITIALIZED;
    }

    ESP_LOGD(TAG, "Set Emissivity: %u%%", Emissivity);

    if (p_Device->Internal.isRadiometric) {
        // Scale percentage into Lepton scene emissivity values (1-100% -> 82-8192).
        if (Emissivity < 1) {
            Emissivity = 1;
        }

        if (Emissivity > 100) {
            Emissivity = 100;
        }

        FluxValues.sceneEmissivity = Emissivity * 8192 / 100;

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

uint32_t Lepton_GetUptime(Lepton_t *p_Device, Lepton_Result_t *p_Status)
{
    uint32_t Time;

    if ((p_Device == NULL) || (p_Device->Internal.isInitialized == false) ||
        (CCI_GetUptime(&p_Device->Internal.CCI, &Time, p_Status) != LEPTON_ERR_OK)) {
        return 0;
    }

    return Time;
}

Lepton_Error_t Lepton_GetSceneStatistics(Lepton_t *p_Device, Lepton_SceneStatistics_t *p_Statistics,
                                         Lepton_Result_t *p_Status)
{
    if (p_Device == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    } else if (p_Device->Internal.isInitialized == false) {
        return LEPTON_ERR_NOT_INITIALIZED;
    }

    return CCI_GetSceneStatistics(&p_Device->Internal.CCI, p_Statistics, p_Status);
}

Lepton_Error_t Lepton_SetSpotmeterROI(Lepton_t *p_Device, Lepton_ROI_t *p_ROI, Lepton_Result_t *p_Status)
{
    if (p_Device == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    } else if (p_Device->Internal.isInitialized == false) {
        return LEPTON_ERR_NOT_INITIALIZED;
    }

    return CCI_SetSpotmeterROI(&p_Device->Internal.CCI, p_ROI, p_Status);
}

Lepton_Error_t Lepton_GetSpotmeterROI(Lepton_t *p_Device, Lepton_ROI_t *p_ROI, Lepton_Result_t *p_Status)
{
    if (p_Device == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    } else if (p_Device->Internal.isInitialized == false) {
        return LEPTON_ERR_NOT_INITIALIZED;
    }

    return CCI_GetSpotmeterROI(&p_Device->Internal.CCI, p_ROI, p_Status);
}

Lepton_Error_t Lepton_GetSpotmeter(Lepton_t *p_Device, Lepton_Spotmeter_Float_t *p_Spot, Lepton_Result_t *p_Status)
{
    Lepton_TLinear_Resolution_t Resolution;
    Lepton_Spotmeter_t Spotmeter;

    if (p_Device == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    } else if (p_Device->Internal.isInitialized == false) {
        return LEPTON_ERR_NOT_INITIALIZED;
    }

    LEPTON_ERROR_CHECK(CCI_GetSpotmeter(&p_Device->Internal.CCI, &Spotmeter, p_Status));
    LEPTON_ERROR_CHECK(CCI_GetTLinearResolution(&p_Device->Internal.CCI, &Resolution, p_Status));

    ESP_LOGD(TAG, "Spotmeter Resolution: %u", Resolution);
    ESP_LOGD(TAG, "Spotmeter Raw Values - Value: %u, Min: %u, Max: %u, Population: %u",
             Spotmeter.Value, Spotmeter.Min, Spotmeter.Max, Spotmeter.Population);

    p_Spot->Population = Spotmeter.Population;

    if (Resolution == LEPTON_TLINEAR_0_1_RESOLUTION) {
        ESP_LOGD(TAG, "Spotmeter values in 0.1K resolution, converting to K");

        p_Spot->Value = Spotmeter.Value / 10.0f;
        p_Spot->Min = Spotmeter.Min / 10.0f;
        p_Spot->Max = Spotmeter.Max / 10.0f;
    } else if (Resolution == LEPTON_TLINEAR_0_01_RESOLUTION) {
        ESP_LOGD(TAG, "Spotmeter values in 0.01K resolution, converting to K");

        p_Spot->Value = Spotmeter.Value / 100.0f;
        p_Spot->Min = Spotmeter.Min / 100.0f;
        p_Spot->Max = Spotmeter.Max / 100.0f;
    }

    return LEPTON_ERR_OK;
}

Lepton_Error_t Lepton_SetVideoSource(Lepton_t *p_Device, Lepton_VideoSource_t Source, uint16_t Constant,
                                     Lepton_Result_t *p_Status)
{
    if (p_Device == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    } else if (p_Device->Internal.isInitialized == false) {
        return LEPTON_ERR_NOT_INITIALIZED;
    }

    return CCI_SetVideoSource(&p_Device->Internal.CCI, Source, Constant, p_Status);
}

Lepton_Error_t Lepton_GetVideoSource(Lepton_t *p_Device, Lepton_VideoSource_t *p_Source, Lepton_Result_t *p_Status)
{
    if (p_Device == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    } else if (p_Device->Internal.isInitialized == false) {
        return LEPTON_ERR_NOT_INITIALIZED;
    }

    return CCI_GetVideoSource(&p_Device->Internal.CCI, p_Source, p_Status);
}

Lepton_Error_t Lepton_SetTLinearResolution(Lepton_t *p_Device, Lepton_TLinear_Resolution_t Resolution,
                                           Lepton_Result_t *p_Status)
{
    if (p_Device == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    } else if (p_Device->Internal.isInitialized == false) {
        return LEPTON_ERR_NOT_INITIALIZED;
    }

    return CCI_SetTLinearResolution(&p_Device->Internal.CCI, Resolution, p_Status);
}

Lepton_Error_t Lepton_GetTLinearResolution(Lepton_t *p_Device, Lepton_TLinear_Resolution_t *p_Resolution,
                                           Lepton_Result_t *p_Status)
{
    if (p_Device == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    } else if (p_Device->Internal.isInitialized == false) {
        return LEPTON_ERR_NOT_INITIALIZED;
    }

    return CCI_GetTLinearResolution(&p_Device->Internal.CCI, p_Resolution, p_Status);
}

Lepton_Error_t Lepton_FrezeVideo(Lepton_t *p_Device, bool Freeze, Lepton_Result_t *p_Status)
{
    if (p_Device == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    } else if (p_Device->Internal.isInitialized == false) {
        return LEPTON_ERR_NOT_INITIALIZED;
    } else if (p_Device->Internal.isVideoFreezeEnabled == Freeze) {
        return LEPTON_ERR_OK;
    }

    LEPTON_ERROR_CHECK(CCI_SetVideoFreeze(&p_Device->Internal.CCI, Freeze, p_Status));

    p_Device->Internal.isVideoFreezeEnabled = Freeze;

    return LEPTON_ERR_OK;
}