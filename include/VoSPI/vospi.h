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

/** @brief Packet number that contains the segment ID (TTT field).
 */
#define VOSPI_PACKET_WITH_SEGMENT_ID            20

/** @brief Minimum resync time in milliseconds (CS held HIGH).
 *         Lepton requires ~185ms to reset VoSPI state machine.
 */
#define VOSPI_RESYNC_MS                         200

/** @brief Pixels per packet for Lepton 3.5 (half a line).
 */
#define VOSPI_PIXELS_PER_PACKET                 80

/** @brief Segments per frame.
 */
#define LEPTON_VOSPI_SEGMENTS_PER_FRAME         4

/** @brief              Initialise the VoSPI interface.
 *  @param p_Interface  Pointer to VoSPI interface object
 *  @return             LEPTON_ERR_OK when successful
 *                      LEPTON_ERR_FAIL when the initialization has failed
 *                      LEPTON_ERR_NO_MEM when no memory is available for the SPI
 */
Lepton_Error_t VoSPI_Init(VoSPI_t* p_Interface);

/** @brief              Deinitialise the VoSPI interface.
 *  @param p_Interface  Pointer to VoSPI interface object
 *  @return			    LEPTON_ERR_OK when successful
 */
Lepton_Error_t VoSPI_Deinit(VoSPI_t* p_Interface);

/** @brief              Request a VoSPI resync.
 *                      This holds CS high for ~200ms to reset the Lepton's VoSPI state machine.
 *                      Call this when sync is lost.
 *  @param p_Interface  Pointer to VoSPI interface object
 */
void VoSPI_RequestResync(VoSPI_t* p_Interface);

/** @brief              Check if VoSPI is currently in resync period.
 *  @param p_Interface  Pointer to VoSPI interface object
 *  @return             #true if still resyncing (CS held high), false when ready to capture
 */
bool VoSPI_IsResyncing(VoSPI_t* p_Interface);

/** @brief                  Capture a complete frame from the Lepton.
 *  @param p_Interface      Pointer to VoSPI interface object
 *  @param p_BufferIndex    Output: index of the buffer that was written (valid only when LEPTON_ERR_OK)
 *  @return                 LEPTON_ERR_OK when a frame was received successfully
 *                          LEPTON_ERR_NOT_FINISHED when no frame is ready yet (call again)
 *                          LEPTON_ERR_FAIL on sync error (resync will be triggered automatically)
 */
Lepton_Error_t VoSPI_CaptureImage(VoSPI_t* p_Interface, uint8_t* p_BufferIndex);

#endif /* VOSPI_H_ */