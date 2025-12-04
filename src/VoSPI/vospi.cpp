/*
* vospi.cpp
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
#include <esp_timer.h>
#include <esp_task_wdt.h>

#include <stdint.h>
#include <string.h>

#include "vospi.h"
#include "lepton.h"

static const char* TAG = "VoSPI";

/** @brief              Read a single VoSPI packet.
 *  @param p_Interface  Pointer to VoSPI interface
 *  @param p_Header     Output: raw 16-bit header word
 *  @param p_PacketData Output: pointer to payload data (inside Packet buffer)
 *  @return             ESP_OK on success
 */
static esp_err_t VoSPI_ReadPacket(VoSPI_t* p_Interface, uint16_t* p_Header, uint8_t** p_PacketData)
{
    uint8_t* rawPacket;
    esp_err_t ret;
    spi_transaction_t trans;

    memset(&trans, 0, sizeof(trans));
    trans.rxlength = (p_Interface->ImageWidth + 4) * 8;  /* Bits */
    trans.rx_buffer = p_Interface->Packet;
    trans.tx_buffer = NULL;

    ret = spi_device_transmit(p_Interface->Handle, &trans);
    if(ret != ESP_OK)
    {
        return ret;
    }

    /* VoSPI sends bytes in big-endian order, but ESP32 SPI receives them as-is.
     * The packet buffer is uint16_t* but we need to read individual bytes.
     * Cast to uint8_t* for correct byte access.
     */
    rawPacket = reinterpret_cast<uint8_t*>(p_Interface->Packet);
    
    /* Header is first 2 bytes, big-endian */
    *p_Header = (static_cast<uint16_t>(rawPacket[0]) << 8) | rawPacket[1];
    
    /* Payload starts at byte 4 (after 2-byte header + 2-byte CRC) */
    *p_PacketData = &rawPacket[4];

    return ESP_OK;
}

Lepton_Error_t VoSPI_Init(VoSPI_t* p_Interface)
{
    Lepton_Error_t Error;

    if(p_Interface == NULL)
    {
        return LEPTON_ERR_INVALID_ARG;
    }
    else if(p_Interface->isInitialized)
    {
        return LEPTON_ERR_OK;
    }

    if(spi_bus_initialize(p_Interface->Host, &p_Interface->Master, p_Interface->DMA) != ESP_OK)
    {
        ESP_LOGE(TAG, "Lepton SPI Master initialization failed!");
        Error = LEPTON_ERR_FAIL;
        goto VoSPI_Init_Error_1;
    }

    if(spi_bus_add_device(p_Interface->Host, &p_Interface->Interface, &p_Interface->Handle) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add Lepton SPI device!");
        Error = LEPTON_ERR_FAIL;
        goto VoSPI_Init_Error_1;
    }

    /* Allocate DMA-capable packet buffer (4 bytes header + 160 / 240 bytes payload) */
    p_Interface->Packet = reinterpret_cast<uint16_t*>(
        heap_caps_malloc(p_Interface->ImageWidth + 4, MALLOC_CAP_DMA));
    if(p_Interface->Packet == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate Lepton DMA packet buffer!");
        Error = LEPTON_ERR_NO_MEM;
        goto VoSPI_Init_Error_2;
    }

    /* Allocate frame buffer in PSRAM */
    for(uint8_t i = 0; i < CONFIG_LEPTON_VOSPI_FRAME_BUFFERS; i++)
    {
        p_Interface->Image_Buffer[i] = reinterpret_cast<uint16_t*>(heap_caps_malloc(p_Interface->ImageWidth * p_Interface->ImageHeight * p_Interface->BytesPerPixel, MALLOC_CAP_SPIRAM));
        if(p_Interface->Image_Buffer[i] == NULL)
        {
            ESP_LOGE(TAG, "Failed to allocate Lepton frame buffer!");
            Error = LEPTON_ERR_NO_MEM;
            goto VoSPI_Init_Error_3;
        }
    }

    p_Interface->FrameCounter = 0;
    p_Interface->isInitialized = true;

    /* Initialize resync state - start with a resync to establish sync */
    p_Interface->isResync = true;
    p_Interface->ResyncStartUs = esp_timer_get_time();

    ESP_LOGI(TAG, "VoSPI initialized successfully");
    return LEPTON_ERR_OK;

VoSPI_Init_Error_3:
    heap_caps_free(p_Interface->Packet);

VoSPI_Init_Error_2:
    spi_bus_remove_device(p_Interface->Handle);

VoSPI_Init_Error_1:
    spi_bus_free(p_Interface->Host);

    return Error;
}

Lepton_Error_t VoSPI_Deinit(VoSPI_t* p_Interface)
{
    if(p_Interface == NULL)
    {
        return LEPTON_ERR_INVALID_ARG;
    }
    else if(p_Interface->isInitialized == false)
    {
        return LEPTON_ERR_OK;
    }

    spi_bus_remove_device(p_Interface->Handle);
    spi_bus_free(p_Interface->Host);

    for(uint8_t i = 0; i < CONFIG_LEPTON_VOSPI_FRAME_BUFFERS; i++)
    {
        if(p_Interface->Image_Buffer[i] != NULL)
        {
            heap_caps_free(p_Interface->Image_Buffer[i]);
            p_Interface->Image_Buffer[i] = NULL;
        }
    }

    if(p_Interface->Packet != NULL)
    {
        heap_caps_free(p_Interface->Packet);
        p_Interface->Packet = NULL;
    }

    p_Interface->isInitialized = false;
    return LEPTON_ERR_OK;
}

void VoSPI_RequestResync(VoSPI_t* p_Interface)
{
    if(p_Interface == NULL)
    {
        return;
    }

    p_Interface->isResync = true;
    p_Interface->ResyncStartUs = esp_timer_get_time();
    ESP_LOGD(TAG, "Resync requested");
}

bool VoSPI_isResyncing(VoSPI_t* p_Interface)
{
    if(p_Interface == NULL)
    {
        return;
    }
    else if(p_Interface->isResync == false)
    {
        return false;
    }

    int64_t elapsed_us = esp_timer_get_time() - p_Interface->ResyncStartUs;
    if(elapsed_us >= (VOSPI_RESYNC_MS * 1000))
    {
        p_Interface->isResync = false;
        ESP_LOGD(TAG, "Resync complete");
        return false;
    }

    return true;
}

int VoSPI_CaptureImage(VoSPI_t* p_Interface, uint8_t* p_BufferIndex)
{
    uint16_t Header;
    uint8_t* PacketData;

    if(p_Interface == NULL || (p_Interface->isInitialized == false))
    {
        ESP_LOGE(TAG, "Invalid interface or not initialized");
        return ESP_ERR_INVALID_ARG;
    }

    /* If in resync period, CS is held high (no SPI activity) */
    if(VoSPI_isResyncing(p_Interface))
    {
        return ESP_ERR_NOT_FINISHED;
    }

    /* Capture all 4 segments */
    for(uint8_t Segment = 1; Segment <= LEPTON_VOSPI_SEGMENTS_PER_FRAME; Segment++)
    {
        bool discardSegment = false;

        for(uint8_t packet = 0; packet < p_Interface->PacketsPerFrame; packet++)
        {
            /* Read packet from camera */
            if(VoSPI_ReadPacket(p_Interface, &Header, &PacketData) != ESP_OK)
            {
                ESP_LOGE(TAG, "SPI read failed");
                VoSPI_RequestResync(p_Interface);
                return ESP_FAIL;
            }

            uint16_t packetNum = Header & 0x0FFF;
            uint8_t idField = (Header >> 8) & 0x0F;

            /* Check for discard packet (ID field = 0x0F) */
            if(idField == 0x0F)
            {
                /* Discard packet - retry this packet */
                packet--;
                continue;
            }

            /* Validate packet number */
            if(packetNum != packet)
            {
                /* Out of sync - start over */
                ESP_LOGD(TAG, "Packet num mismatch: got %u, expected %u (seg %u)", packetNum, packet, Segment);
                VoSPI_RequestResync(p_Interface);
                return ESP_FAIL;
            }

            /* Packet 20 contains segment ID */
            if(packetNum == VOSPI_PACKET_WITH_SEGMENT_ID)
            {
                uint8_t TTT = (Header >> 12) & 0x07;
                
                if(TTT == 0)
                {
                    /* Discard this segment and restart it */
                    discardSegment = true;
                }
                else if(TTT != Segment)
                {
                    /* Wrong segment - out of sync, start over */
                    ESP_LOGD(TAG, "Segment mismatch: TTT=%u, expected %u", TTT, Segment);
                    VoSPI_RequestResync(p_Interface);
                    return ESP_FAIL;
                }
            }

            /* Copy to frame buffer if not discarding */
            if(!discardSegment)
            {
                size_t frameOffset = ((Segment - 1) * p_Interface->PacketsPerFrame + packet) * VOSPI_PIXELS_PER_PACKET;
                
                uint16_t* dest = &p_Interface->Image_Buffer[p_Interface->CurrentBuffer][frameOffset];
                for(size_t i = 0; i < VOSPI_PIXELS_PER_PACKET; i++)
                {
                    dest[i] = (static_cast<uint16_t>(PacketData[i * 2]) << 8) | PacketData[i * 2 + 1];
                }
            }
        }

        /* If segment needs to be discarded, re-read it */
        if(discardSegment)
        {
            Segment--;
        }
    }

    /* Return the buffer index that was just written */
    if(p_BufferIndex != NULL)
    {
        *p_BufferIndex = p_Interface->CurrentBuffer;
    }

    /* Switch to next buffer for next frame */
    p_Interface->CurrentBuffer = (p_Interface->CurrentBuffer + 1) % CONFIG_LEPTON_VOSPI_FRAME_BUFFERS;
    p_Interface->FrameCounter++;

    ESP_LOGD(TAG, "Frame captured successfully to buffer %u (total: %" PRIu32 ")", p_Interface->CurrentBuffer, p_Interface->FrameCounter);

    return ESP_OK;
}