/*
 * vospi.h
 *
 *  Copyright (C) Daniel Kampert, 2026
 *  Website: www.kampis-elektroecke.de
 *  File info: VoSPI interface driver for Lepton.
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

/** @brief Maximum number of consecutive discard packets (ID = 0x0F) before a resync
 *         is triggered. Prevents an infinite spin when the Lepton is unresponsive.
 *         At 20 MHz SPI a 164-byte packet takes ~65 µs; 2000 discards ≤ 130 ms,
 *         which comfortably covers the worst-case inter-frame gap (~115 ms at 8.7 fps).
 */
#define VOSPI_MAX_DISCARD_PACKETS               2000

/** @brief Pixels per packet for Lepton 3.5 (half a line).
 */
#define VOSPI_PIXELS_PER_PACKET                 80

/** @brief Segments per frame.
 */
#define LEPTON_VOSPI_SEGMENTS_PER_FRAME         4

/** @brief              Initialize the VoSPI interface and trigger a resynchronization.
 *  @param p_Interface  Pointer to VoSPI interface object
 *  @return             LEPTON_ERR_OK when successful
 *                      LEPTON_ERR_FAIL when the initialization has failed
 *                      LEPTON_ERR_NO_MEM when no memory is available for the SPI
 */
Lepton_Error_t VoSPI_Init(VoSPI_t *p_Interface);

/** @brief              Deinitialize the VoSPI interface.
 *  @param p_Interface  Pointer to VoSPI interface object
 *  @return             LEPTON_ERR_OK when successful
 */
Lepton_Error_t VoSPI_Deinit(VoSPI_t *p_Interface);

/** @brief              Request a VoSPI resync.
 *                      This holds CS high for ~200ms to reset the Lepton's VoSPI state machine.
 *                      Call this when sync is lost.
 *  @param p_Interface  Pointer to VoSPI interface object
 */
void VoSPI_RequestResync(VoSPI_t *p_Interface);

/** @brief              Check if VoSPI is currently in resync period.
 *  @param p_Interface  Pointer to VoSPI interface object
 *  @return             true if still resyncing (CS held high), false when ready to capture
 */
bool VoSPI_IsResyncing(VoSPI_t *p_Interface);

/** @brief                  Capture a complete frame from the Lepton.
 *  @param p_Interface      Pointer to VoSPI interface object
 *  @param p_BufferIndex    Output: index of the buffer that was written (valid only when LEPTON_ERR_OK)
 *  @return                 LEPTON_ERR_OK when a frame was received successfully
 *                          LEPTON_ERR_NOT_FINISHED when no frame is ready yet (call again)
 *                          LEPTON_ERR_FAIL on sync error (resync will be triggered automatically)
 */
Lepton_Error_t VoSPI_CaptureImage(VoSPI_t *p_Interface, uint8_t *p_BufferIndex);

#endif /* VOSPI_H_ */