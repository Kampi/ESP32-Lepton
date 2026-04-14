/*
 * lepton.h
 *
 *  Copyright (C) Daniel Kampert, 2026
 *  Website: www.kampis-elektroecke.de
 *  File info: FLIR Lepton thermal imaging sensor driver for ESP32.
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

#ifndef ESP32_LEPTON_H_
#define ESP32_LEPTON_H_

#include "lepton_defs.h"
#include "lepton_errors.h"
#include "lepton_config.h"
#include "lepton_palette.h"

#include <string>

#ifndef STRINGIFY
#define STRINGIFY_HELPER(x) #x
#define STRINGIFY(x)        STRINGIFY_HELPER(x)
#endif

/** @brief Image width in pixels.
 */
#define LEPTON_IMAGE_WIDTH  160

/** @brief Image height in pixels.
 */
#define LEPTON_IMAGE_HEIGHT 120

#ifdef __cplusplus
extern "C" {
#endif

/** @brief          Return the current video format.
 *  @param p_Device Pointer to device instance
 *  @param p_Format Pointer to output video format
 *  @return         LEPTON_ERR_OK when successful
 */
inline __attribute__((always_inline)) Lepton_Error_t Lepton_GetVideoFormat(Lepton_t *p_Device,
                                                                           Lepton_VideoFormat_t *p_Format)
{
    if (p_Device == NULL || p_Format == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    }

    *p_Format = p_Device->Internal.VideoFormat;

    return LEPTON_ERR_OK;
}


/** @brief          Convert a temperature from Kelvin (in Lepton format) into degree Celsius.
 *                  The Lepton sensor returns temperature values in centi-Kelvin (Kelvin * 100).
 *  @param Kelvin   Temperature in Lepton format (centi-Kelvin, i.e., Kelvin * 100)
 *  @return         Temperature [Degree Celsius]
 */
inline __attribute__((always_inline)) float Lepton_KelvinToCelsius(uint32_t Kelvin)
{
    return (static_cast<float>(Kelvin) / 100.0) - 273.15;
}

/** @brief          Return the capturing status.
 *  @param p_Device Pointer to device instance
 *  @return         true when the device is capturing images
 */
inline __attribute__((always_inline)) bool Lepton_isCapturing(Lepton_t *p_Device)
{
    if (p_Device == NULL) {
        return false;
    }

    return p_Device->Internal.VoSPI.IsCapturing;
}

/** @brief          Return the number of sync errors since initialization.
 *  @param p_Device Pointer to device instance
 *  @return         Number of sync errors since initialization
 */
inline __attribute__((always_inline)) int32_t Lepton_GetSyncErrors(Lepton_t *p_Device)
{
    if (p_Device == NULL) {
        return -1;
    }

    return p_Device->Internal.VoSPI.SyncErrors;
}

/** @brief          Return the number of valid frames since initialization.
 *  @param p_Device Pointer to device instance
 *  @return         Number of valid frames since initialization
 */
inline __attribute__((always_inline)) int32_t Lepton_GetFrameCounter(Lepton_t *p_Device)
{
    if (p_Device == NULL) {
        return -1;
    }

    return p_Device->Internal.VoSPI.FrameCounter;
}

/** @brief  Get the version number of the library.
 *  @return Library version
 */
inline __attribute__((always_inline)) std::string Lepton_LibVersion(void)
{
    return std::string(STRINGIFY(ESP32_LEPTON_LIB_MAJOR)) + "." + std::string(STRINGIFY(
                                                                                  ESP32_LEPTON_LIB_MINOR)) + "." + std::string(STRINGIFY(ESP32_LEPTON_LIB_BUILD));
}

/** @brief          Initialize the Lepton thermal camera.
 *  @param p_Device Pointer to device instance
 *  @param p_Init   Pointer to initial device configuration
 *  @param p_Status (Optional) Pointer to device status
 *  @return         LEPTON_ERR_OK when successful
 */
Lepton_Error_t Lepton_Init(Lepton_t *p_Device, const Lepton_Conf_t *const p_Init, Lepton_Result_t *p_Status = NULL);

/** @brief          Deinitialize the Lepton thermal camera.
 *  @param p_Device Pointer to device instance
 */
void Lepton_Deinit(Lepton_t *p_Device);

/** @brief          Perform a hardware reset of the camera.
 *  @param p_Device Pointer to device instance
 */
void Lepton_HardReset(Lepton_t *p_Device) __attribute__((weak));

/** @brief          Enable / Disable the power-down mode of the camera.
 *  @param p_Device Pointer to device instance
 *  @param Enable   Enable / Disable
 */
void Lepton_EnablePowerDown(Lepton_t *p_Device, bool Enable) __attribute__((weak));

/** @brief          Set the video output format.
 *                  NOTE: This function triggers a resynchronization of the VoSPI interface.
 *  @param p_Device Pointer to device instance
 *  @param Format   Video format
 *  @param p_Status (Optional) Pointer to device status
 *  @return         LEPTON_ERR_OK when successful
 */
Lepton_Error_t Lepton_SetVideoFormat(Lepton_t *p_Device, Lepton_VideoFormat_t Format, Lepton_Result_t *p_Status = NULL);

/** @brief          Enable / Disable the telemetry data in the video stream.
 *  @param p_Device Pointer to device instance
 *  @param Enable   Enable / Disable
 *  @param p_Status (Optional) Pointer to device status
 *  @return         LEPTON_ERR_OK when successful
 */
Lepton_Error_t Lepton_EnableTelemetry(Lepton_t *p_Device, bool Enable, Lepton_Result_t *p_Status = NULL);

/** @brief          Get the device temperatures.
 *  @param p_Device Pointer to device instance
 *  @param p_FPA    Pointer to FPA temperature in Lepton format (Kelvin * 0.01)
 *  @param p_AUX    Pointer to AUX temperature in Lepton format (Kelvin * 0.01)
 *  @param p_Status (Optional) Pointer to device status
 *  @return         LEPTON_ERR_OK when successful
 */
Lepton_Error_t Lepton_GetTemperature(Lepton_t *p_Device, uint16_t *p_FPA, uint16_t *p_AUX,
                                     Lepton_Result_t *p_Status = NULL);

/** @brief          Enable / Disable the automatic gain control.
 *  @param p_Device Pointer to device instance
 *  @param Enable   Enable / Disable
 *  @param p_Status (Optional) Pointer to device status
 *  @return         LEPTON_ERR_OK when successful
 */
Lepton_Error_t Lepton_EnableAGC(Lepton_t *p_Device, bool Enable, Lepton_Result_t *p_Status = NULL);

/** @brief              Set the scene parameters for radiometric measurements.
 *  @param p_Device     Pointer to device instance
 *  @param Emissivity   Emissivity value (0-100)
 *  @param p_Status     (Optional) Pointer to device status
 *  @return             LEPTON_ERR_OK when successful
 */
Lepton_Error_t Lepton_SetEmissivity(Lepton_t *p_Device, Lepton_Emissivity_t Emissivity,
                                    Lepton_Result_t *p_Status = NULL);

/** @brief              Get the scene parameters for radiometric measurements.
 *  @param p_Device     Pointer to device instance
 *  @param p_Parameters Pointer to flux linear parameters object
 *  @param p_Status     (Optional) Pointer to device status
 *  @return             LEPTON_ERR_OK when successful
 */
Lepton_Error_t Lepton_GetFluxLinearParameters(Lepton_t *p_Device, Lepton_FluxLinearParams_t *p_Parameters,
                                              Lepton_Result_t *p_Status = NULL);

/** @brief              Set the scene parameters for radiometric measurements.
 *  @param p_Device     Pointer to device instance
 *  @param p_Parameters Pointer to flux linear parameters object
 *  @param p_Status     (Optional) Pointer to device status
 *  @return             LEPTON_ERR_OK when successful
 */
Lepton_Error_t Lepton_SetFluxLinearParameters(Lepton_t *p_Device, Lepton_FluxLinearParams_t *p_Parameters,
                                              Lepton_Result_t *p_Status = NULL);

/** @brief          Get the uptime of the device in milliseconds.
 *  @param p_Device Pointer to device instance
 *  @param p_Status (Optional) Pointer to device status
 *  @return         Device uptime
 */
uint32_t Lepton_GetUptime(Lepton_t *p_Device, Lepton_Result_t *p_Status = NULL);

/** @brief              Get the scene statistics of the current image. Convert the values to Kelvin when TLinear is active.
 *  @param p_Device     Pointer to device instance
 *  @param p_Statistics Pointer to scene statistics object
 *  @param p_Status     (Optional) Pointer to device status
 *  @return             LEPTON_ERR_OK when successful
 */
Lepton_Error_t Lepton_GetSceneStatistics(Lepton_t *p_Device, Lepton_SceneStatistics_t *p_Statistics,
                                         Lepton_Result_t *p_Status = NULL);

/** @brief          Get the spotmeter values.
 *  @param p_Device Pointer to device instance
 *  @param p_Spot   Pointer to spotmeter object
 *  @param p_Status (Optional) Pointer to device status
 *  @return         LEPTON_ERR_OK when successful
 */
Lepton_Error_t Lepton_GetSpotmeter(Lepton_t *p_Device, Lepton_Spotmeter_t *p_Spot, Lepton_Result_t *p_Status = NULL);

/** @brief          Get the spotmeter ROI.
 *  @param p_Device Pointer to device instance
 *  @param p_ROI    Pointer to Lepton ROI object
 *  @param p_Status (Optional) Pointer to device status
 *  @return         LEPTON_ERR_OK when successful
 */
Lepton_Error_t Lepton_GetSpotmeterROI(Lepton_t *p_Device, Lepton_ROI_t *p_ROI, Lepton_Result_t *p_Status = NULL);

/** @brief          Set the spotmeter ROI.
 *  @param p_Device Pointer to device instance
 *  @param p_ROI    Pointer to Lepton ROI object
 *  @param p_Status (Optional) Pointer to device status
 *  @return         LEPTON_ERR_OK when successful
 */
Lepton_Error_t Lepton_SetSpotmeterROI(Lepton_t *p_Device, Lepton_ROI_t *p_ROI, Lepton_Result_t *p_Status = NULL);

/** @brief          Get the scene ROI.
 *  @param p_Device Pointer to device instance
 *  @param p_ROI    Pointer to Lepton ROI object
 *  @param p_Status (Optional) Pointer to device status
 *  @return         LEPTON_ERR_OK when successful
 */
Lepton_Error_t Lepton_GetSceneROI(Lepton_t *p_Device, Lepton_ROI_t *p_ROI, Lepton_Result_t *p_Status = NULL);

/** @brief          Set the scene ROI.
 *  @param p_Device Pointer to device instance
 *  @param p_ROI    Pointer to Lepton ROI object
 *  @param p_Status (Optional) Pointer to device status
 *  @return         LEPTON_ERR_OK when successful
 */
Lepton_Error_t Lepton_SetSceneROI(Lepton_t *p_Device, Lepton_ROI_t *p_ROI, Lepton_Result_t *p_Status = NULL);

/** @brief          Get the AGC ROI.
 *  @param p_Device Pointer to device instance
 *  @param p_ROI    Pointer to Lepton ROI object
 *  @param p_Status (Optional) Pointer to device status
 *  @return         LEPTON_ERR_OK when successful
 */
Lepton_Error_t Lepton_GetAGCROI(Lepton_t *p_Device, Lepton_ROI_t *p_ROI, Lepton_Result_t *p_Status = NULL);

/** @brief          Set the AGC ROI.
 *  @param p_Device Pointer to device instance
 *  @param p_ROI    Pointer to Lepton ROI object
 *  @param p_Status (Optional) Pointer to device status
 *  @return         LEPTON_ERR_OK when successful
 */
Lepton_Error_t Lepton_SetAGCROI(Lepton_t *p_Device, Lepton_ROI_t *p_ROI, Lepton_Result_t *p_Status = NULL);

/** @brief          Get the video focus ROI.
 *  @param p_Device Pointer to device instance
 *  @param p_ROI    Pointer to Lepton ROI object
 *  @param p_Status (Optional) Pointer to device status
 *  @return         LEPTON_ERR_OK when successful
 */
Lepton_Error_t Lepton_GetVideoFocusROI(Lepton_t *p_Device, Lepton_ROI_t *p_ROI, Lepton_Result_t *p_Status = NULL);

/** @brief          Set the video focus ROI.
 *  @param p_Device Pointer to device instance
 *  @param p_ROI    Pointer to Lepton ROI object
 *  @param p_Status (Optional) Pointer to device status
 *  @return         LEPTON_ERR_OK when successful
 */
Lepton_Error_t Lepton_SetVideoFocusROI(Lepton_t *p_Device, Lepton_ROI_t *p_ROI, Lepton_Result_t *p_Status = NULL);

/** @brief          Set the video source.
 *  @param p_Device Pointer to device instance
 *  @param Source   Video source
 *  @param Constant (Optional) Constant value
 *                  NOTE: Only used when Source is set to LEPTON_SOURCE_CONSTANT
 *  @param p_Status (Optional) Pointer to device status
 *  @return         LEPTON_ERR_OK when successful
 */
Lepton_Error_t Lepton_SetVideoSource(Lepton_t *p_Device, Lepton_VideoSource_t Source, uint16_t Constant = 0,
                                     Lepton_Result_t *p_Status = NULL);

/** @brief          Get the video source.
 *  @param p_Device Pointer to device instance
 *  @param p_Source Video source
 *  @param p_Status (Optional) Pointer to device status
 *  @return         LEPTON_ERR_OK when successful
 */
Lepton_Error_t Lepton_GetVideoSource(Lepton_t *p_Device, Lepton_VideoSource_t *p_Source,
                                     Lepton_Result_t *p_Status = NULL);

/** @brief              Set the TLinear resolution.
 *  @param p_Device     Pointer to device instance
 *  @param Resolution   TLinear resolution
 *  @param p_Status     (Optional) Pointer to device status
 *  @return             LEPTON_ERR_OK when successful
 */
Lepton_Error_t Lepton_SetTLinearResolution(Lepton_t *p_Device, Lepton_TLinear_Resolution_t Resolution,
                                           Lepton_Result_t *p_Status = NULL);

/** @brief              Get the TLinear resolution.
 *  @param p_Device     Pointer to device instance
 *  @param p_Resolution Pointer to TLinear resolution
 *  @param p_Status     (Optional) Pointer to device status
 *  @return             LEPTON_ERR_OK when successful
 */
Lepton_Error_t Lepton_GetTLinearResolution(Lepton_t *p_Device, Lepton_TLinear_Resolution_t *p_Resolution,
                                           Lepton_Result_t *p_Status = NULL);

/** @brief          Enable / Disable video freeze.
 *  @param p_Device Pointer to device instance
 *  @param Freeze   true to enable video freeze, false to disable
 *  @param p_Status (Optional) Pointer to device status
 *  @return         LEPTON_ERR_OK when successful
 */
Lepton_Error_t Lepton_FreezeVideo(Lepton_t *p_Device, bool Freeze, Lepton_Result_t *p_Status = NULL);

/** @brief          Start the capture task to read new frames from the camera.
 *  @param p_Device Pointer to device instance
 *  @param p_Queue  Queue handle to receive frames from the capture task.
 *                  The queue must uses the type Lepton_FrameBuffer_t that contains a pointer to the frame
 *                  buffer which becomes ready. Make sure the queue is large enough to hold multiple
 *                  frames or process the frames fast enough to avoid frame drops. You can use
 *                  If required, you can use CONFIG_LEPTON_VOSPI_FRAME_BUFFERS to increase the amount of
 *                  frame buffers in the VoSPI driver to reduce the chance of frame drops.
 *  @return         LEPTON_ERR_OK when successful
 *                  LEPTON_ERR_NO_MEM when either the capture task nor the capture ISR can be initialized
 */
Lepton_Error_t Lepton_StartCapture(Lepton_t *p_Device, QueueHandle_t p_Queue);

/** @brief          Stop the image capturing.
 *  @param p_Device Pointer to device instance
 *  @return         LEPTON_ERR_OK when successful
 */
Lepton_Error_t Lepton_StopCapture(Lepton_t *p_Device);

/** @brief              Convert a raw 14-bit pixel value to temperature in Celsius.
 *                      This function converts radiometric pixel values to actual temperature using TLinear mode.
 *                      Only works with radiometric Lepton modules (3.5) and requires TLinear to be enabled.
 *  @param p_Device     Pointer to device instance
 *  @param PixelValue   Raw 14-bit pixel value from the thermal image
 *  @param p_Temperature Pointer to output temperature in degrees Celsius
 *  @return             LEPTON_ERR_OK when successful
 *                      LEPTON_ERR_INVALID_ARG when parameters are NULL
 *                      LEPTON_ERR_NOT_INITIALIZED when device is not initialized
 *                      LEPTON_ERR_NOT_SUPPORTED when device is not radiometric or TLinear is disabled
 */
Lepton_Error_t Lepton_GetPixelTemperature(Lepton_t *p_Device, uint16_t PixelValue, float *p_Temperature);

/** @brief          Convert a thermal value to RGB color using iron palette.
 *                  The function applies an iron palette pseudocolor mapping optimized for thermal imaging.
 *                  Color mapping: blue (cold) -> cyan -> green -> yellow -> red (hot)
 *  @param p_Device Pointer to device instance
 *  @param p_Input  Pointer to input buffer containing thermal values (14-bit per pixel)
 *  @param p_Output Pointer to RGB output buffer
 *  @param p_Min    Pointer to minimum value in the image
 *  @param p_Max    Pointer to maximum value in the image
 *  @param Width    Image width in pixels
 *  @param Height   Image height in pixels
 *  @return         true on success, false on failure
 */
bool Lepton_Raw14ToRGB(Lepton_t *p_Device, uint16_t *p_Input, uint8_t *p_Output, int16_t *p_Min, int16_t *p_Max,
                       uint16_t Width,
                       uint16_t Height);

#ifdef __cplusplus
}
#endif

#endif /* ESP32_LEPTON_H_ */