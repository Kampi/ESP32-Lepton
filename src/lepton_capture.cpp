 /*
 * lepton_capture.cpp
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

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include "lepton.h"
#include "vospi.h"

#include <sdkconfig.h>

#ifndef CONFIG_LEPTON_TASK_STACK
    #define CONFIG_LEPTON_TASK_STACK          				2048
#endif

#ifndef CONFIG_LEPTON_TASK_PRIORITY
    #define CONFIG_LEPTON_TASK_PRIORITY          			12
#endif

#ifndef CONFIG_LEPTON_TASK_CORE_AFFINITY
    #ifndef CONFIG_LEPTON_TASK_CORE
        #define CONFIG_LEPTON_TASK_CORE          			1
    #endif
#endif

static TickType_t lepton_lastWakeTime = xTaskGetTickCount();
static const char* TAG = "Lepton-Capture";

/** @brief			
 *  @param p_Args	
 */
#ifdef CONFIG_LEPTON_VSYNC_PLACE_IRAM
    static void IRAM_ATTR Lepton_VSync_ISR_Handler(void* p_Arg)
#else
    static void Lepton_VSync_ISR_Handler(void* p_Args)
#endif
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xTaskNotifyFromISR((TaskHandle_t)p_Arg, 0, eNoAction, &xHigherPriorityTaskWoken);

    if(xHigherPriorityTaskWoken)
    {
        portYIELD_FROM_ISR();
    }
}

/** @brief			Camera image capture task.
 *  @param p_Args	Pointer to task arguments
 */
static void Lepton_CaptureTask(void* p_Args)
{
    Lepton_t* Device = static_cast<Lepton_t*>(p_Args);

    while(true)
    {
        esp_task_wdt_reset();
        vTaskDelay(1);
    }
}

Lepton_Error_t Lepton_StartCapture(Lepton_t* p_Device)
{
    Lepton_Error_t Error;

    if(p_Device == NULL)
    {
        return LEPTON_ERR_INVALID_ARG;
    }
    else if(p_Device->Internal.isInitialized == false)
    {
        return LEPTON_ERR_NOT_INITIALIZED;
    }
    else if(p_Device->Internal.CaptureHandle != NULL)
    {
        return LEPTON_ERR_BUSY;
    }

    Error = LEPTON_ERR_OK;

    #ifdef CONFIG_LEPTON_TASK_CORE_AFFINITY
        xTaskCreatePinnedToCore(&Lepton_CaptureTask, "Lepton_Capture", CONFIG_LEPTON_TASK_STACK, p_Device, CONFIG_LEPTON_TASK_PRIORITY, &p_Device->Internal.CaptureHandle, CONFIG_LEPTON_TASK_CORE);
    #else
        xTaskCreate(&Lepton_CaptureTask, "Lepton_Capture", CONFIG_LEPTON_TASK_STACK, p_Device, CONFIG_LEPTON_TASK_PRIORITY, &p_Device->Internal.CaptureHandle);
    #endif

    if(p_Device->Internal.CaptureHandle == NULL)
    {
        goto Lepton_StartCapture_Error_1;
        Error = LEPTON_ERR_NO_MEM;
    }

    esp_task_wdt_add(p_Device->Internal.CaptureHandle);

    /* V-Sync is a high-level signal. So we need to add a positive edge interrupt */
    gpio_set_direction(p_Device->Internal.VSync, GPIO_MODE_INPUT);
    gpio_set_pull_mode(p_Device->Internal.VSync, GPIO_PULLDOWN_ONLY);
    gpio_set_intr_type(p_Device->Internal.VSync, GPIO_INTR_POSEDGE);

    #ifdef CONFIG_LEPTON_VSYNC_PLACE_IRAM
    	gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
    #else
    	gpio_install_isr_service(0);
    #endif

    /* Hook ISR handler for specific GPIO */
    //gpio_isr_handler_add(p_Device->Internal.VSync, Lepton_VSync_ISR_Handler, p_Device->Internal.CaptureHandle);

    p_Device->Internal.isCapturing = true;

    return LEPTON_ERR_OK;

Lepton_StartCapture_Error_1:
    ESP_LOGE(TAG, "Initialization error: 0x%X", static_cast<unsigned int>(Error));

    return Error;
}

Lepton_Error_t Lepton_StopCapture(Lepton_t* p_Device)
{
    if(p_Device == NULL)
    {
        return LEPTON_ERR_INVALID_ARG;
    }
    else if(p_Device->Internal.isInitialized == false)
    {
        return LEPTON_ERR_NOT_INITIALIZED;
    }
    else if(p_Device->Internal.CaptureHandle == NULL)
    {
        return LEPTON_ERR_OK;
    }

    vTaskDelete(p_Device->Internal.CaptureHandle);
    esp_task_wdt_delete(p_Device->Internal.CaptureHandle);

    return LEPTON_ERR_OK;
}

Lepton_Error_t Lepton_GetFrameBuffer(Lepton_t* p_Device, Lepton_Buffer_t* p_Buffer, uint32_t Timeout)
{
    if((p_Device == NULL) || (p_Buffer == NULL))
    {
        return LEPTON_ERR_INVALID_ARG;
    }
    else if(p_Device->Internal.isInitialized == false)
    {
        return LEPTON_ERR_NOT_INITIALIZED;
    }
    else if(p_Device->Internal.CaptureHandle == NULL)
    {
        return LEPTON_ERR_INVALID_STATE;
    }

    //if(xQueueReceive(p_Device->FB.Queue, p_Buffer, Timeout / portTICK_PERIOD_MS) != pdPASS)
    //{
    //	return LEPTON_ERR_FAIL;
    //}

    return LEPTON_ERR_OK;
}

uint32_t Lepton_GetFramesInBuffer(Lepton_t* p_Device)
{
    if((p_Device == NULL) || (p_Device->Internal.isInitialized == false))
    {
        return 0;
    }

    //return uxQueueSpacesAvailable(p_Device->FB.Queue);
    return 0;
}