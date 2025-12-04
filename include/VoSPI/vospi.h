 /*
 * vospi.h
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

#ifndef VOSPI_H_
#define VOSPI_H_

#include "lepton_defs.h"
#include "lepton_errors.h"

/** @brief VoSPI segment length in packets when telemetry is disabled.
 */
#define LEPTON_VOSPI_PACKETS_PER_SEGMENT        60

/** @brief Segments per frame.
 */
#define LEPTON_VOSPI_SEGMENTS_PER_FRAME         4

/** @brief VoSPI packet length in bytes.
 */
#define LEPTON_VOSPI_PACKET_LENGTH              (LEPTON_IMAGE_WIDTH + 4)

/** @brief VoSPI segment length in words (16 bit) when telemetry is disabled.
 */
#define LEPTON_VOSPI_WORDS_PER_SEGMENT          (LEPTON_VOSPI_PACKETS_PER_SEGMENT * (LEPTON_IMAGE_WIDTH / 2))

/** @brief Telemetry length in words (16 bit).
 */
#define LEPTON_VOSPI_TELEMETRY_LENGTH            (LEPTON_IMAGE_WIDTH / 2)

/** @brief Number of lines used for the telemetry.
 */
#define LEPTON_VOSPI_TELEMETRY_ROWS             3

/** @brief              Initialise the VoSPI interface.
 *  @param p_Interface  Pointer to VoSPI interface object
 *  @param UseTelemetry (Optional) Set to #true when telemetry is included
 *  @return             LEPTON_ERR_OK when successful
 *                      LEPTON_ERR_FAIL when the initialization has failed
 *                      LEPTON_ERR_NO_MEM when no memory is available for the SPI
 */
Lepton_Error_t VoSPI_Init(VoSPI_t* p_Interface, bool UseTelemetry = false);

/** @brief              Deinitialise the VoSPI interface.
 *  @param p_Interface  Pointer to VoSPI interface object
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t VoSPI_Deinit(VoSPI_t* p_Interface);

/** @brief				
 *  @param p_Interface	
 *  @return			    LEPTON_ERR_OK when successful
 */
int VoSPI_SoftSync(VoSPI_t* p_Interface);

/** @brief              
 *  @param p_Interface  Pointer to VoSPI interface object
 *  @return             LEPTON_ERR_OK when a frame was received successfully
 */
int VoSPI_CaptureImage(VoSPI_t* p_Interface);

#endif /* VOSPI_H_ */