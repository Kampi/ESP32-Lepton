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

/** @brief Packet number that contains the segment.
 */
#define VOSPI_PACKET_TTT		                20

static spi_transaction_t vospi_transaction;

static const char* TAG 			= "VoSPI";
 
Lepton_Error_t VoSPI_Init(VoSPI_t* p_Interface, bool UseTelemetry)
{
    Lepton_Error_t Error;

    if(p_Interface == NULL)
    {
        return LEPTON_ERR_INVALID_ARG;
    }
    else if(p_Interface->Internal.isInitialized)
    {
        return LEPTON_ERR_OK;
    }

    if(spi_bus_initialize(p_Interface->Host, &p_Interface->Master, p_Interface->DMA) != ESP_OK)
    {
        ESP_LOGE(TAG, "Lepton SPI Master initialization failed!");

        Error = LEPTON_ERR_FAIL;
        goto VoSPI_Init_Error_1;
    }

    if(spi_bus_add_device(p_Interface->Host, &p_Interface->Interface, &p_Interface->Internal.Handle) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add Lepton SPI device!");

        Error = LEPTON_ERR_FAIL;
        goto VoSPI_Init_Error_1;
    }

    /* NOTE: malloc needs the size in bytes! */
    p_Interface->Internal.Packet = reinterpret_cast<uint16_t*>(heap_caps_malloc(LEPTON_VOSPI_PACKET_LENGTH, MALLOC_CAP_DMA));
    if(p_Interface->Internal.Packet == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate Lepton DMA packet buffer!");

        Error = LEPTON_ERR_NO_MEM;
        goto VoSPI_Init_Error_2;
    }

    /* NOTE: malloc needs the size in bytes! */
    p_Interface->Internal.Frame = reinterpret_cast<uint16_t*>(heap_caps_malloc(LEPTON_IMAGE_WIDTH * LEPTON_IMAGE_HEIGHT * sizeof(uint16_t), MALLOC_CAP_SPIRAM));
    if(p_Interface->Internal.Frame == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate Lepton DMA frame buffer!");

        Error = LEPTON_ERR_NO_MEM;
        goto VoSPI_Init_Error_3;
    }

    p_Interface->ValidFrames = 0;
    p_Interface->SyncErrors = 0;
    p_Interface->Internal.isInitialized = true;

	memset(&vospi_transaction, 0, sizeof(spi_transaction_t));
    vospi_transaction.rxlength = LEPTON_VOSPI_PACKET_LENGTH * 8;
    vospi_transaction.tx_buffer = NULL;
    vospi_transaction.rx_buffer = p_Interface->Internal.Packet;

    return LEPTON_ERR_OK;

VoSPI_Init_Error_3:
    heap_caps_free(p_Interface->Internal.Packet);

VoSPI_Init_Error_2:
    spi_bus_remove_device(p_Interface->Internal.Handle);

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
    else if(p_Interface->Internal.isInitialized == false)
    {
        return LEPTON_ERR_OK;
    }

    spi_bus_remove_device(p_Interface->Internal.Handle);
    spi_bus_free(p_Interface->Host);

    if(p_Interface->Internal.Frame != NULL)
    {
        heap_caps_free(p_Interface->Internal.Frame);
    }

    if(p_Interface->Internal.Packet != NULL)
    {
        heap_caps_free(p_Interface->Internal.Packet);
    }

    return LEPTON_ERR_OK;
}
 
int VoSPI_SoftSync(VoSPI_t* p_Interface)
{
    return ESP_FAIL;
}
 
int VoSPI_CaptureImage(VoSPI_t* p_Interface)
{
    return ESP_OK;
}