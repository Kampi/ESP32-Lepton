 /*
 * cci.h
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

#ifndef CCI_H_
#define CCI_H_

#include "lepton_defs.h"
#include "lepton_errors.h"

/** @brief 			    Initialize the Lepton CCI.
 *  @param p_Interface  Pointer to CCI interface object
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_Init(CCI_t* p_Interface);

/** @brief              Deinitialize the Lepton CCI.
 *  @param p_Interface  Pointer to CCI interface object
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_Deinit(CCI_t* p_Interface);

/** @brief 			    Generic register "SET" access.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param Command	    Command instruction
 *  @param Length	    Data length (0 to 512 words (16 bit))
 *  @param p_Buffer	    Pointer to data buffer
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_Set(CCI_t* p_Interface, uint16_t Command, uint16_t Length, const uint16_t* p_Buffer, Lepton_Result_t* p_Status = NULL);

/** @brief 			    Generic register "GET" access.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param Command	    Command instruction
 *  @param Length	    Data length (0 to 512 words (16 bit))
 *  @param p_Buffer	    Pointer to data buffer
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_Get(CCI_t* p_Interface, uint16_t Command, uint16_t Length, uint16_t* p_Buffer, Lepton_Result_t* p_Status = NULL);

/** @brief 			    Ping the camera and wait until the boot process has finished.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_WaitForBoot(CCI_t* p_Interface, Lepton_Result_t* p_Status = NULL);

/** @brief 				Read the part number from the camera.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_PartNumber	Pointer to device part number
 *  @param p_Status		(Optional) Pointer to device status
 *  @return				LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_GetPartNumber(CCI_t* p_Interface, char* p_PartNumber, Lepton_Result_t* p_Status = NULL);

/** @brief 				Read the software version from both processors.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_Version	Pointer to software version object
 *  @param p_Status		(Optional) Pointer to device status
 *  @return				LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_GetSoftwareVersion(CCI_t* p_Interface, Lepton_Version_t* p_Version, Lepton_Result_t* p_Status = NULL);

/** @brief 				Read the serial number (64 bit) from the camera.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_Serial	    Pointer to device serial number (8 bytes)
 *  @param p_Status		(Optional) Pointer to device status
 *  @return				LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_GetSerialNumber(CCI_t* p_Interface, uint8_t* p_Serial, Lepton_Result_t* p_Status = NULL);

/** @brief 				Read the camera uptime.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_Uptime		Pointer to camera uptime
 *  @param p_Status		(Optional) Pointer to device status
 *  @return				LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_GetUptime(CCI_t* p_Interface, uint32_t* p_Uptime, Lepton_Result_t* p_Status = NULL);

/** @brief 				Get the AUX (case) temperature in Kelvin x 100 (16-bit result).
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_Temp		Pointer to temperature
 *  @param p_Status		(Optional) Pointer to device status
 *  @return				LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_GetAuxTemp(CCI_t* p_Interface, uint16_t* p_Temp, Lepton_Result_t* p_Status = NULL);

/** @brief 				Get the FPA (sensor) temperature in Kelvin x 100 (16-bit result).
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_Temp		Pointer to temperature
 *  @param p_Status		(Optional) Pointer to device status
 *  @return				LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_GetFPATemp(CCI_t* p_Interface, uint16_t* p_Temp, Lepton_Result_t* p_Status = NULL);

/** @brief 			    Reboot the camera.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_RebootCamera(CCI_t* p_Interface, Lepton_Result_t* p_Status = NULL);

/** @brief 			    Run FFC Normalization
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_RunFFC(CCI_t* p_Interface, Lepton_Result_t* p_Status = NULL);

/** @brief 			    Set the GPIO mode.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param Mode		    Pointer to GPIO mode
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_SetGPIOMode(CCI_t* p_Interface, Lepton_GPIO_t Mode, Lepton_Result_t* p_Status = NULL);

/** @brief 			    Get the GPIO mode.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_Mode	    Pointer to GPIO mode
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_GetGPIOMode(CCI_t* p_Interface, Lepton_GPIO_t* p_Mode, Lepton_Result_t* p_Status = NULL);

/** @brief 			    Enable / Disable radiometry.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param Enable	    Enable / Disable
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_SetRadiometry(CCI_t* p_Interface, bool Enable, Lepton_Result_t* p_Status = NULL);

/** @brief 			    Get the radiometry state.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_Enable	    Pointer to state
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_GetRadiometry(CCI_t* p_Interface, bool* p_Enable, Lepton_Result_t* p_Status = NULL);

/** @brief 			    Enable / Disable AGC.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param Enable	    Enable / Disable
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_SetAGC(CCI_t* p_Interface, bool Enable, Lepton_Result_t* p_Status = NULL);

/** @brief 			    Get the AGC state.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_Enable	    Pointer to state
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_GetAGC(CCI_t* p_Interface, bool* p_Enable, Lepton_Result_t* p_Status = NULL);

/** @brief 			    Enable / Disable AGC calculation.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_Enable	    Pointer to state
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_SetAGCCalc(CCI_t* p_Interface, bool Enable, Lepton_Result_t* p_Status = NULL);

/** @brief 			    Get the AGC calculation state.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_Enable	    Pointer to state
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_GetAGCCalc(CCI_t* p_Interface, bool* p_Enable, Lepton_Result_t* p_Status = NULL);

/** @brief 			    Enable / Disable Radiometry T-Linear.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param Enable	    Enable / Disable
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_SetTLinear(CCI_t* p_Interface, bool Enable, Lepton_Result_t* p_Status = NULL);

/** @brief 			    Get the Radiometry T-Linear state.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_Enable	    Pointer to state
 *  @param p_Status 	(Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_GetTLinear(CCI_t* p_Interface, bool* p_Enable, Lepton_Result_t* p_Status = NULL);

/** @brief 			    Enable / Disable telemetry.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param Enable	    Enable / Disable
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_SetTelemetry(CCI_t* p_Interface, bool Enable, Lepton_Result_t* p_Status = NULL);

/** @brief 			    Get the telemetry state.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_Enable	    Pointer to state
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_GetTelemetry(CCI_t* p_Interface, bool* p_Enable, Lepton_Result_t* p_Status = NULL);

/** @brief 			    Set the telemetry position.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param Position	    Telemetry position
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_SetTelemetryPosition(CCI_t* p_Interface, Lepton_TelemetryPos_t Position, Lepton_Result_t* p_Status = NULL);

/** @brief 			    Get the telemetry position.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_Enable	    Pointer to telemetry position
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_GetTelemetry(CCI_t* p_Interface, Lepton_TelemetryPos_t* p_Position, Lepton_Result_t* p_Status = NULL);

/** @brief 			    Enable / Disable Radiometry T-Linear Auto Res.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param Enable	    Enable / Disable
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_SetRadiometryTLinearAutoRes(CCI_t* p_Interface, bool Enable, Lepton_Result_t* p_Status = NULL);

/** @brief 			    Get the Radiometry T-Linear Auto Res state.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_Enable	    Pointer to state
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_GetRadiometryTLinearAutoRes(CCI_t* p_Interface, bool* p_Enable, Lepton_Result_t* p_Status = NULL);

/** @brief 			
 *  @param p_Interface  Pointer to CCI interface object
 *  @param Mode		    Gain mode
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_SetGainMode(CCI_t* p_Interface, Lepton_Gain_t Mode, Lepton_Result_t* p_Status = NULL);

/** @brief 			
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_Mode	    Pointer to gain mode
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_GetGainMode(CCI_t* p_Interface, Lepton_Gain_t* p_Mode, Lepton_Result_t* p_Status = NULL);

/** @brief 			
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_Mode	    Pointer to gain mode
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_SetRadiometryFluxLinearParams(CCI_t* p_Interface, Lepton_FluxLinearParams_t* p_Params, Lepton_Result_t* p_Status = NULL);

/** @brief 			
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_Mode	    Pointer to gain mode
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_GetRadiometryFluxLinearParams(CCI_t* p_Interface, Lepton_FluxLinearParams_t* p_Params, Lepton_Result_t* p_Status = NULL);

/** @brief 			    Set the video format.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param Format		Video format
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_SetVideoFormat(CCI_t* p_Interface, Lepton_VideoFormat_t Format, Lepton_Result_t* p_Status = NULL);

/** @brief 			    Get the video format.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_Format	    Pointer to video format
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_GetVideoFormat(CCI_t* p_Interface, Lepton_VideoFormat_t* p_Format, Lepton_Result_t* p_Status = NULL);

/** @brief 			    
 *  @param p_Interface  Pointer to CCI interface object
 *  @param Position
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_SetShutterPosition(CCI_t* p_Interface, Lepton_ShutterPos_t Position, Lepton_Result_t* p_Status = NULL);

/** @brief 			    
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_Position
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_GetShutterPosition(CCI_t* p_Interface, Lepton_ShutterPos_t* p_Position, Lepton_Result_t* p_Status = NULL);

/** @brief 			    Set the FFC shutter mode.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param Mode		    FFC shutter mode
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_SetFFCMode(CCI_t* p_Interface, Lepton_FFC_ShutterMode_t Mode, Lepton_Result_t* p_Status = NULL);

/** @brief 			    Get the FFC shutter mode.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_Mode		Pointer to FFC shutter mode
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_GetFFCMode(CCI_t* p_Interface, Lepton_FFC_ShutterMode_t* p_Mode, Lepton_Result_t* p_Status = NULL);

/** @brief 			    Set the video output source.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param Source		Video output source
 *  @param Constant     (Optional) Constant value
 *                      NOTE: Only used when Source is set to LEPTON_SOURCE_CONSTANT
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_SetVideoSource(CCI_t* p_Interface, Lepton_VideoSource_t Source, uint16_t Constant = 0, Lepton_Result_t* p_Status = NULL);

/** @brief 			    Get the video output source.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_Source	    Pointer to video output source
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_GetVideoSource(CCI_t* p_Interface, Lepton_VideoSource_t* p_Source, Lepton_Result_t* p_Status = NULL);

/** @brief 			    Set the AGC ROI.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_ROI		Pointer to ROI object
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_SetAGCROI(CCI_t* p_Interface, const Lepton_ROI_t* p_ROI, Lepton_Result_t* p_Status);

/** @brief 			    Get the AGC ROI.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_ROI		Pointer to ROI object
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_GetAGCROI(CCI_t* p_Interface, Lepton_ROI_t* p_ROI, Lepton_Result_t* p_Status);

/** @brief 			    Set the scene ROI.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_ROI		Pointer to ROI object
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_SetSceneROI(CCI_t* p_Interface, const Lepton_ROI_t* p_ROI, Lepton_Result_t* p_Status);

/** @brief 			    Get the scene ROI.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_ROI		Pointer to ROI object
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_GetSceneROI(CCI_t* p_Interface, Lepton_ROI_t* p_ROI, Lepton_Result_t* p_Status);

/** @brief 			    Set the spotmeter ROI.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_ROI		Pointer to ROI object
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_SetSpotmeterROI(CCI_t* p_Interface, const Lepton_ROI_t* p_ROI, Lepton_Result_t* p_Status);

/** @brief 			    Get the spotmeter ROI.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_ROI		Pointer to ROI object
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_GetSpotmeterROI(CCI_t* p_Interface, Lepton_ROI_t* p_ROI, Lepton_Result_t* p_Status);

/** @brief 			    Get the spotmeter results.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_Spot		Pointer to spotmeter object
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_GetSpotmeter(CCI_t* p_Interface, Lepton_Spotmeter_t* p_Spot, Lepton_Result_t* p_Status);

/** @brief 			    Returns the current scene statistics for the video frame defined by the SYS ROI (see section 4.5.13).
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_Statistics Pointer to screne statistics object
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_GetSceneStatistics(CCI_t* p_Interface, Lepton_SceneStatistics_t* p_Statistics, Lepton_Result_t* p_Status = NULL);

/** @brief 			    
 *  @param p_Interface  Pointer to CCI interface object
 *  @param Enable       
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_SetVideoFreeze(CCI_t* p_Interface, bool Enable, Lepton_Result_t* p_Status = NULL);

/** @brief 			    
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_Enable     
 *  @param p_Status	    (Optional) Pointer to device status
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t CCI_GetVideoFreeze(CCI_t* p_Interface, bool* p_Enable, Lepton_Result_t* p_Status = NULL);

#endif /* CCI_H_ */