 /*
 * lepton.h
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

#ifndef LEPTON_H_
#define LEPTON_H_

#include "lepton_defs.h"
#include "lepton_errors.h"
#include "lepton_config.h"
#include "lepton_palette.h"

/** @brief Image width in pixels.
 */
#define LEPTON_IMAGE_WIDTH  160

/** @brief Image height in pixels.
 */
#define LEPTON_IMAGE_HEIGHT 120

/** @brief          Convert a temperature from Kelvin (in Lepton format) into degree Celsius.
 *                  The Lepton sensor returns temperature values in Kelvin * 0.01 format.
 *  @param Kelvin 	Temperature in Lepton format (Kelvin * 0.01)
 *  @return			Temperature [Degree Celsius]
 */
inline __attribute__((always_inline)) float Lepton_KelvinToCelsius(uint32_t Kelvin)
{
    return (static_cast<float>(Kelvin) * 100.0) - 273.15;
}

/** @brief          Return the capturing status.
 *  @param p_Device Pointer to device instance
 *  @return         #true when the device is capturing images
 */
inline __attribute__((always_inline)) bool Lepton_isCapturing(Lepton_t* p_Device)
{
    if(p_Device == NULL)
    {
        return false;
    }

    return p_Device->Internal.VoSPI.isCapturing;
}

/** @brief          Return the number of sync errors since initialization.
 *  @param p_Device Pointer to device instance
 *  @return         Number of sync errors since initialization
 */
inline __attribute__((always_inline)) int32_t Lepton_getSyncErrors(Lepton_t* p_Device)
{
    if(p_Device == NULL)
    {
        return -1;
    }

    return p_Device->Internal.VoSPI.SyncErrors;
}

/** @brief          Return the number of valid frames since initialization.
 *  @param p_Device Pointer to device instance
 *  @return         Number of valid frames since initialization
 */
inline __attribute__((always_inline)) int32_t Lepton_getFrameCounter(Lepton_t* p_Device)
{
    if(p_Device == NULL)
    {
        return -1;
    }

    return p_Device->Internal.VoSPI.FrameCounter;
}

/** @brief  Get the version number of the library.
 *  @return Library version
 */
inline __attribute__((always_inline)) const std::string Lepton_LibVersion(void)
{
    return "";
    //return std::string(STRINGIFY(CONFIG_LEPTON_LIB_MAJOR)) + "." + std::string(STRINGIFY(CONFIG_LEPTON_LIB_MINOR)) + "." + std::string(STRINGIFY(CONFIG_LEPTON_LIB_BUILD));
}

/** @brief          Initialize the Lepton thermal camera.
 *  @param p_Device Pointer to device instance
 *  @param p_Init	Pointer to initial device configuration
 *  @param p_Status (Optional) Pointer to device status
 *  @return         LEPTON_ERR_OK when successful
 */
Lepton_Error_t Lepton_Init(Lepton_t* p_Device, const Lepton_Conf_t* const p_Init, Lepton_Result_t* p_Status = NULL);

/** @brief          Deinitialize the Lepton thermal camera.
 *  @param p_Device Pointer to device instance
 */
void Lepton_Deinit(Lepton_t* p_Device);

/** @brief          Perform a hardware reset of the camera.
 *  @param p_Device Pointer to device instance
 */
void Lepton_HardReset(Lepton_t* p_Device) __attribute__((weak));

/** @brief          Enable / Disable the power-down mode of the camera.
 *  @param p_Device Pointer to device instance
 *  @param Enable   Enable / Disable
 */
void Lepton_EnablePowerDown(Lepton_t* p_Device, bool Enable) __attribute__((weak));

/** @brief          Get the device temperatures.
 *  @param p_Device Pointer to device instance
 *  @param p_FPA    Pointer to FPA temperature in Lepton format (Kelvin * 0.01)
 *  @param p_AUX    Pointer to AUX temperature in Lepton format (Kelvin * 0.01)
 *  @param p_Status (Optional) Pointer to device status
 *  @return         LEPTON_ERR_OK when successful
 */
Lepton_Error_t Lepton_GetTemp(Lepton_t* p_Device, uint16_t* p_FPA, uint16_t* p_AUX, Lepton_Result_t* p_Status = NULL);

/** @brief          Enable / Disable the automatic gain control.
 *  @param p_Device Pointer to device instance
 *  @param Enable   Enable / Disable
 *  @param p_Status (Optional) Pointer to device status
 *  @return         LEPTON_ERR_OK when successful
 */
Lepton_Error_t Lepton_EnableAGC(Lepton_t* p_Device, bool Enable, Lepton_Result_t* p_Status = NULL);

/** @brief              Set the scene emissivity for radiometric measurements.
 *                      Only applicable for radiometric Lepton modules.
 *  @param p_Device     Pointer to device instance
 *  @param Emissivity   Scene emissivity value in percent (1-100%)
 *  @param p_Status     (Optional) Pointer to device status
 *  @return             LEPTON_ERR_OK when successful
 *                      LEPTON_ERR_INVALID_STATE when called on non-radiometric module
 */
Lepton_Error_t Lepton_Emissivity(Lepton_t* p_Device, uint16_t Emissivity, Lepton_Result_t* p_Status = NULL);

/** @brief          Get the uptime of the device in milliseconds.
 *  @param p_Device Pointer to device instance
 *  @param p_Status (Optional) Pointer to device status
 *  @return         Device uptime
 */
uint32_t Lepton_GetUptime(Lepton_t* p_Device, Lepton_Result_t* p_Status = NULL);

/** @brief          	Get the scene statistics of the current image.
 *  @param p_Device 	Pointer to device instance
 *  @param p_Statistics	Pointer to scene statistics object
 *  @param p_Status 	(Optional) Pointer to device status
 *  @return         	LEPTON_ERR_OK when successful
 */
Lepton_Error_t Lepton_GetSceneStatistics(Lepton_t* p_Device, Lepton_SceneStatistics_t* p_Statistics, Lepton_Result_t* p_Status = NULL);

/** @brief			Set the spotmeter ROI.
 *  @param p_Device	Pointer to device instance
 *  @param ROI		Lepton ROI object
 *  @param p_Status (Optional) Pointer to device status
 *  @return			LEPTON_ERR_OK when successful
 */
Lepton_Error_t Lepton_SetSpotmeterROI(Lepton_t* p_Device, Lepton_ROI_t ROI, Lepton_Result_t* p_Status = NULL);

/** @brief			Get the spotmeter ROI.
 *  @param p_Device	Pointer to device instance
 *  @param p_ROI	Pointer to Lepton ROI object
 *  @param p_Status (Optional) Pointer to device status
 *  @return			LEPTON_ERR_OK when successful
 */
Lepton_Error_t Lepton_GetSpotmeterROI(Lepton_t* p_Device, Lepton_ROI_t* p_ROI, Lepton_Result_t* p_Status = NULL);

/** @brief          Get the spotmeter values.
 *  @param p_Device Pointer to device instance
 *  @param p_Spot   Pointer to spotmeter object
 *  @param p_Status (Optional) Pointer to device status
 *  @return         LEPTON_ERR_OK when successful
 */
Lepton_Error_t Lepton_GetSpotmeter(Lepton_t* p_Device, Lepton_Spotmeter_t* p_Spot, Lepton_Result_t* p_Status = NULL);

/** @brief          Set the video output format.
 *  @param p_Device Pointer to device instance
 *  @param Format   Video format
 *  @param p_Status (Optional) Pointer to device status
 *  @return         LEPTON_ERR_OK when successful
 */
Lepton_Error_t Lepton_SetVideoFormat(Lepton_t* p_Device, Lepton_VideoFormat_t Format, Lepton_Result_t* p_Status = NULL);

/** @brief          Start the capture task to read new frames from the camera.
 *  @param p_Device Pointer to device instance
 *  @param p_Queue  Queue handle for frame ready events (can be NULL)
 *  @return         LEPTON_ERR_OK when successful
 *                  LEPTON_ERR_NO_MEM when either the capture task nor the capture ISR can be initialized
 */
Lepton_Error_t Lepton_StartCapture(Lepton_t* p_Device, QueueHandle_t p_Queue = NULL);

/** @brief          Stop the image capturing.
 *  @param p_Device Pointer to device instance
 *  @return         LEPTON_ERR_OK when successful
 */
Lepton_Error_t Lepton_StopCapture(Lepton_t* p_Device);

/** @brief          Convert a thermal value to RGB color using iron palette.
 *                  The function applies an iron palette pseudocolor mapping optimized for thermal imaging.
 *                  Color mapping: blue (cold) -> cyan -> green -> yellow -> red (hot)
 *  @param p_Input  Pointer to input buffer containing thermal values (14-bit per pixel)
 *  @param p_Output Pointer to RGB output buffer (must be at least 3 bytes)
 *  @param Width    Image width in pixels
 *  @param Height   Image height in pixels
 *  @return         true on success, false on failure
 */
bool Lepton_Raw14ToRGB(uint16_t* p_Input, uint8_t* p_Output, uint16_t Width, uint16_t Height);

#endif /* LEPTON_H_ */