/*
 * vospi.cpp
 *
 *  Copyright (C) Daniel Kampert, 2026
 *   Website: www.kampis-elektroecke.de
 *  File info: VoSPI interface implementation.
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

#include <esp_log.h>
#include <esp_timer.h>
#include <esp_task_wdt.h>

#include <stdint.h>
#include <string.h>

#include "vospi.h"
#include "lepton.h"

static const char *TAG = "VoSPI";

/** @brief              Read a single VoSPI packet.
 *  @param p_Interface  Pointer to VoSPI interface
 *  @param p_Header     Output: raw 16-bit header word
 *  @param p_PacketData Output: pointer to payload data (inside Packet buffer)
 *  @return             @c ESP_OK on success
 */
static esp_err_t VoSPI_ReadPacket(VoSPI_t *p_Interface, uint16_t *p_Header, uint8_t **p_PacketData)
{
    uint8_t *RawPacket;
    esp_err_t Error;
    spi_transaction_t Trans;
    size_t PacketSize;

    /* Calculate packet size based on format:
     * RAW14: 4 bytes header/CRC + 160 bytes payload (80 pixels × 2 bytes)
     * RGB888: 4 bytes header/CRC + 240 bytes payload (80 pixels × 3 bytes) */
    PacketSize = 4 + (VOSPI_PIXELS_PER_PACKET * p_Interface->BytesPerPixel);

    __builtin_memset(&Trans, 0, sizeof(Trans));
    Trans.rxlength = PacketSize * 8;  /* Bits */
    Trans.rx_buffer = p_Interface->Packet;
    Trans.tx_buffer = NULL;
    Trans.flags = 0;

    Error = spi_device_transmit(p_Interface->Handle, &Trans);
    if (Error != ESP_OK) {
        return Error;
    }

    /* VoSPI sends bytes in big-endian order, but ESP32 SPI receives them as-is.
     * The packet buffer is uint16_t* but we need to read individual bytes.
     * Cast to uint8_t* for correct byte access.
     */
    RawPacket = reinterpret_cast<uint8_t *>(p_Interface->Packet);

    /* Header is first 2 bytes, big-endian */
    *p_Header = (static_cast<uint16_t>(RawPacket[0]) << 8) | RawPacket[1];

    /* Payload starts at byte 4 (after 2-byte header + 2-byte CRC) */
    *p_PacketData = &RawPacket[4];

    return ESP_OK;
}

Lepton_Error_t VoSPI_Init(VoSPI_t *p_Interface)
{
    uint32_t Caps;
    Lepton_Error_t Error;
    size_t PacketBufferSize;

    if (p_Interface == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    } else if (p_Interface->IsInitialized) {
        return LEPTON_ERR_OK;
    }

    if (spi_bus_initialize(p_Interface->Host, &p_Interface->Master, p_Interface->DMA) != ESP_OK) {
        ESP_LOGE(TAG, "SPI Master initialization failed!");
        Error = LEPTON_ERR_FAIL;
        return Error;
    }

    if (spi_bus_add_device(p_Interface->Host, &p_Interface->Interface, &p_Interface->Handle) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add SPI device!");
        Error = LEPTON_ERR_FAIL;
        goto VoSPI_Init_Error_1;
    }

    /* Packet buffer must always be in internal DMA-capable SRAM (never PSRAM).
     * Using PSRAM here would force esp_cache_msync() on every SPI transaction
     * (~244 times per frame), causing severe CPU stalls and WDT timeouts. */
    Caps = MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL;

    /* Allocate DMA-capable packet buffer (4 bytes header/CRC + payload)
     * RAW14: 4 + 160 = 164 bytes
     * RGB888: 4 + 240 = 244 bytes */
    PacketBufferSize = 4 + (VOSPI_PIXELS_PER_PACKET * p_Interface->BytesPerPixel);
    p_Interface->Packet = reinterpret_cast<uint16_t *>(heap_caps_malloc(PacketBufferSize, Caps));
    if (p_Interface->Packet == NULL) {
        ESP_LOGE(TAG, "Failed to allocate DMA packet buffer!");
        Error = LEPTON_ERR_NO_MEM;
        goto VoSPI_Init_Error_2;
    }

#ifdef CONFIG_SPIRAM
    Caps = MALLOC_CAP_SIMD | MALLOC_CAP_SPIRAM;
#else
    Caps = MALLOC_CAP_SIMD;
#endif

    /* Allocate frame buffer in PSRAM */
    for (uint8_t i = 0; i < CONFIG_LEPTON_VOSPI_FRAME_BUFFERS; i++) {
        p_Interface->Image_Buffer[i] = reinterpret_cast<uint16_t *>(heap_caps_malloc(p_Interface->ImageWidth *
                                                                                     p_Interface->ImageHeight * p_Interface->BytesPerPixel, Caps));
        if (p_Interface->Image_Buffer[i] == NULL) {
            ESP_LOGE(TAG, "Failed to allocate frame buffer!");
            Error = LEPTON_ERR_NO_MEM;
            goto VoSPI_Init_Error_3;
        }
    }

    if (p_Interface->UseTelemetry) {
#ifdef CONFIG_SPIRAM
        Caps = MALLOC_CAP_SPIRAM;
#else
        Caps = 0;
#endif

        /* Telemetry contains 4 packets (or 3 lines) = 4 * 80 pixels * 2 bytes = 640 bytes */
        size_t telemetryBufferSize = 4 * VOSPI_PIXELS_PER_PACKET * sizeof(uint16_t);
        for (uint8_t i = 0; i < CONFIG_LEPTON_VOSPI_FRAME_BUFFERS; i++) {
            p_Interface->Telemetry_Buffer[i] = reinterpret_cast<uint16_t *>(heap_caps_malloc(telemetryBufferSize,
                                                                                             Caps));
            if (p_Interface->Telemetry_Buffer[i] == NULL) {
                ESP_LOGE(TAG, "Failed to allocate telemetry buffer!");
                Error = LEPTON_ERR_NO_MEM;
                goto VoSPI_Init_Error_4;
            }
        }
    }

    p_Interface->FrameCounter = 0;
    p_Interface->IsInitialized = true;

    /* Initialize resync state - start with a resync to establish sync */
    VoSPI_RequestResync(p_Interface);

    ESP_LOGD(TAG, "VoSPI initialized successfully");

    return LEPTON_ERR_OK;

VoSPI_Init_Error_4:
    for (uint8_t i = 0; i < CONFIG_LEPTON_VOSPI_FRAME_BUFFERS; i++) {
        heap_caps_free(p_Interface->Telemetry_Buffer[i]);
        p_Interface->Telemetry_Buffer[i] = NULL;
    }

VoSPI_Init_Error_3:
    for (uint8_t i = 0; i < CONFIG_LEPTON_VOSPI_FRAME_BUFFERS; i++) {
        heap_caps_free(p_Interface->Image_Buffer[i]);
        p_Interface->Image_Buffer[i] = NULL;
    }

    heap_caps_free(p_Interface->Packet);

VoSPI_Init_Error_2:
    spi_bus_remove_device(p_Interface->Handle);

VoSPI_Init_Error_1:
    spi_bus_free(p_Interface->Host);

    return Error;
}

Lepton_Error_t VoSPI_Deinit(VoSPI_t *p_Interface)
{
    if (p_Interface == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    } else if (p_Interface->IsInitialized == false) {
        return LEPTON_ERR_OK;
    }

    spi_bus_remove_device(p_Interface->Handle);
    spi_bus_free(p_Interface->Host);

    for (uint8_t i = 0; i < CONFIG_LEPTON_VOSPI_FRAME_BUFFERS; i++) {
        heap_caps_free(p_Interface->Telemetry_Buffer[i]);
        p_Interface->Telemetry_Buffer[i] = NULL;

        heap_caps_free(p_Interface->Image_Buffer[i]);
        p_Interface->Image_Buffer[i] = NULL;
    }

    heap_caps_free(p_Interface->Packet);
    p_Interface->Packet = NULL;

    p_Interface->IsInitialized = false;
    return LEPTON_ERR_OK;
}

void VoSPI_RequestResync(VoSPI_t *p_Interface)
{
    if (p_Interface == NULL) {
        return;
    }

    ESP_LOGD(TAG, "Resync requested");

    p_Interface->IsResync = true;
    p_Interface->ResyncStartUs = esp_timer_get_time();
}

bool VoSPI_IsResyncing(VoSPI_t *p_Interface)
{
    int64_t Elapsed_us;

    if ((p_Interface == NULL) || (p_Interface->IsResync == false)) {
        return false;
    }

    Elapsed_us = esp_timer_get_time() - p_Interface->ResyncStartUs;
    if (Elapsed_us >= (VOSPI_RESYNC_MS * 1000)) {
        p_Interface->IsResync = false;
        ESP_LOGD(TAG, "Resync complete");

        return false;
    }

    return true;
}

Lepton_Error_t VoSPI_CaptureImage(VoSPI_t *p_Interface, uint8_t *p_BufferIndex)
{
    uint16_t Header;
    uint8_t *PacketData;

    if ((p_Interface == NULL) || (p_Interface->IsInitialized == false) || (p_BufferIndex == NULL)) {
        ESP_LOGE(TAG, "Invalid interface or not initialized!");
        return LEPTON_ERR_INVALID_ARG;
    }

    /* If in resync period, CS is held high (no SPI activity) */
    if (VoSPI_IsResyncing(p_Interface)) {
        return LEPTON_ERR_NOT_FINISHED;
    }

    /* Capture all 4 segments */
    for (uint8_t Segment = 1; Segment <= LEPTON_VOSPI_SEGMENTS_PER_FRAME; Segment++) {
        bool DiscardSegment = false;
        uint32_t DiscardCount = 0;

        /* Reset WDT per segment so a stuck PSRAM / discard-packet storm does not
         * starve lower-priority tasks while this task holds the CPU. */
        esp_task_wdt_reset();

        for (uint8_t packet = 0; packet < p_Interface->PacketsPerFrame; packet++) {
            uint16_t PacketNum;
            uint8_t IdField;

            /* Read packet from camera */
            if (VoSPI_ReadPacket(p_Interface, &Header, &PacketData) != ESP_OK) {
                ESP_LOGE(TAG, "SPI read failed!");
                VoSPI_RequestResync(p_Interface);

                return LEPTON_ERR_FAIL;
            }

            PacketNum = Header & 0x0FFF;
            IdField = (Header >> 8) & 0x0F;

            /* Check for discard packet (ID field = 0x0F) */
            if (IdField == 0x0F) {
                /* Discard packet: retry this packet slot, but with a safety ceiling
                 * to prevent an infinite spin when the Lepton is unresponsive.
                 * Reset WDT every 100 discards (~6.5 ms at 20 MHz) so other tasks
                 * keep running while we wait out the inter-frame gap. */
                DiscardCount++;
                if (DiscardCount % 100 == 0) {
                    esp_task_wdt_reset();
                }
                if (DiscardCount > VOSPI_MAX_DISCARD_PACKETS) {
                    ESP_LOGD(TAG, "Too many discard packets (%u), requesting resync",
                             static_cast<unsigned int>(DiscardCount));
                    VoSPI_RequestResync(p_Interface);
                    return LEPTON_ERR_FAIL;
                }
                packet--;
                continue;
            }

            /* Validate packet number */
            if (PacketNum != packet) {
                /* Out of sync - start over */
                ESP_LOGD(TAG, "Packet num mismatch: got %u, expected %u (seg %u)", PacketNum, packet, Segment);
                VoSPI_RequestResync(p_Interface);

                return LEPTON_ERR_FAIL;
            }

            /* Packet 20 contains segment ID */
            if (PacketNum == VOSPI_PACKET_WITH_SEGMENT_ID) {
                uint8_t TTT = (Header >> 12) & 0x07;

                if (TTT == 0) {
                    /* Discard this segment and restart it */
                    DiscardSegment = true;
                } else if (TTT != Segment) {
                    /* Wrong segment - out of sync, start over */
                    ESP_LOGD(TAG, "Segment mismatch: TTT=%u, expected %u", TTT, Segment);
                    VoSPI_RequestResync(p_Interface);

                    return LEPTON_ERR_FAIL;
                }
            }

            /* Copy to frame buffer if not discarding */
            if (DiscardSegment == false) {
                size_t FrameOffset;
                uint16_t *Dest;

                /* We must take care about telemetry packets */
                if (p_Interface->UseTelemetry) {
                    /* The first four packets of segment 1 contain telemetry data */
                    if (p_Interface->TelemetryPosition == LEPTON_TELEMETRY_LOCATION_HEADER) {
                        /* Store the telemetry data for the first four packets of segment 1 */
                        if ((Segment == 1) && (packet < 4)) {
                            FrameOffset = packet * VOSPI_PIXELS_PER_PACKET;
                            Dest = &p_Interface->Telemetry_Buffer[p_Interface->CurrentBuffer][FrameOffset];
                        }
                        /* Segments 1-4: offset by (Segment - 1) * 60 - 4 packets (because Segment 1 lost 4 packets to telemetry) */
                        else {
                            FrameOffset = ((Segment - 1) * p_Interface->PacketsPerFrame + packet - 4) * VOSPI_PIXELS_PER_PACKET;
                            Dest = &p_Interface->Image_Buffer[p_Interface->CurrentBuffer][FrameOffset];
                        }
                    }
                    /* The last four packets of segment 4 contain telemetry data */
                    else if (p_Interface->TelemetryPosition == LEPTON_TELEMETRY_LOCATION_FOOTER) {
                        /* Segments 1-3 and segment 4 packets 0-56: normal offset */
                        if ((Segment < 4) || (packet <= 56)) {
                            FrameOffset = ((Segment - 1) * p_Interface->PacketsPerFrame + packet) * VOSPI_PIXELS_PER_PACKET;
                            Dest = &p_Interface->Image_Buffer[p_Interface->CurrentBuffer][FrameOffset];
                        }
                        /* Store the telemetry data for the last four packets of segment 4 (57, 58, 59, 60) */
                        else {
                            FrameOffset = (packet - 57) * VOSPI_PIXELS_PER_PACKET;
                            Dest = &p_Interface->Telemetry_Buffer[p_Interface->CurrentBuffer][FrameOffset];
                        }
                    }
                    /* Catch invalid cases */
                    else {
                        ESP_LOGE(TAG, "Invalid telemetry position setting!");
                        VoSPI_RequestResync(p_Interface);

                        return LEPTON_ERR_FAIL;
                    }
                }
                /* Otherwise use the regular calculation */
                else {
                    p_Interface->Telemetry_Buffer[p_Interface->CurrentBuffer] = NULL;
                    FrameOffset = ((Segment - 1) * p_Interface->PacketsPerFrame + packet) * VOSPI_PIXELS_PER_PACKET;
                    Dest = &p_Interface->Image_Buffer[p_Interface->CurrentBuffer][FrameOffset];
                }

                /* Copy packet data from the SPI buffer to the output buffer */
                if (p_Interface->BytesPerPixel == 3) {
                    /* RGB888 mode: Data is already in byte format (R, G, B), copy directly as bytes
                     * Need to recalculate byte offset since Image_Buffer is uint16_t* but contains RGB bytes */
                    size_t ByteOffset = ((Segment - 1) * p_Interface->PacketsPerFrame + packet) * VOSPI_PIXELS_PER_PACKET * 3;
                    uint8_t *ImageBufferBytes = reinterpret_cast<uint8_t *>(p_Interface->Image_Buffer[p_Interface->CurrentBuffer]);
                    __builtin_memcpy(&ImageBufferBytes[ByteOffset], PacketData, VOSPI_PIXELS_PER_PACKET * 3);
                } else {
                    /* RAW14 mode: Convert big-endian 16-bit values to little-endian.
                     * SIMD optimisation: process 2 pixels (4 bytes) per iteration using
                     * __builtin_bswap32.  On Xtensa LX7 this maps to SSAI+SRC or equivalent,
                     * cutting the per-packet load/store count roughly in half compared to
                     * processing one pixel at a time. */
                    const uint8_t *__restrict__ Src = PacketData;
                    size_t OutIdx = 0;

                    for (; (OutIdx + 2) <= VOSPI_PIXELS_PER_PACKET; OutIdx += 2, Src += 4) {
                        uint32_t Val;
                        uint32_t Swapped;

                        __builtin_memcpy(&Val, Src, sizeof(uint32_t));
                        Swapped = __builtin_bswap32(Val);
                        Dest[OutIdx] = static_cast<uint16_t>(Swapped >> 16);
                        Dest[OutIdx + 1] = static_cast<uint16_t>(Swapped & 0xFFFFU);
                    }

                    /* Remainder: handles odd VOSPI_PIXELS_PER_PACKET (never taken for 80 pixels) */
                    if (OutIdx < VOSPI_PIXELS_PER_PACKET) {
                        Dest[OutIdx] = (static_cast<uint16_t>(Src[0]) << 8) | Src[1];
                    }
                }
            }
        }

        /* If segment needs to be discarded, re-read it */
        if (DiscardSegment) {
            Segment--;
        }
    }

    /* Return the buffer index that was just written */
    if (p_BufferIndex != NULL) {
        *p_BufferIndex = p_Interface->CurrentBuffer;
    }

    /* Switch to next buffer for next frame */
    p_Interface->CurrentBuffer = (p_Interface->CurrentBuffer + 1) % CONFIG_LEPTON_VOSPI_FRAME_BUFFERS;

    ESP_LOGD(TAG, "Frame captured successfully to buffer %u (total: %" PRIu32 ")", p_Interface->CurrentBuffer,
             p_Interface->FrameCounter);

    p_Interface->FrameCounter++;

    return LEPTON_ERR_OK;
}