/*
 * lepton_capture.cpp
 *
 *  Copyright (C) Daniel Kampert, 2026
 *  Website: www.kampis-elektroecke.de
 *  File info: Lepton frame capture implementation.
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

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include "lepton.h"
#include "vospi.h"
#include "dsps_sub.h"

#include <sdkconfig.h>

/** @brief  ESP32-S3 Xtensa LX7 + PIE SIMD via GCC vector extension.
 *          vector_size(16) maps to the 128-bit Q-registers of the PIE engine:
 *              u16x8_t  – 8 lanes of uint16_t  (used for the min/max scan)
 *              u32x4_t  – 4 lanes of uint32_t  (used for the normalization multiply)
 *          The compiler emits EE.* PIE instructions when -O2/-O3 is active.
 *          The __attribute__((optimize("O3"))) on Lepton_Raw14ToRGB enforces this
 *          even in debug builds (which default to -Og).
 */
typedef uint16_t u16x8_t __attribute__((vector_size(16), aligned(16)));
typedef uint32_t u32x4_t __attribute__((vector_size(16), aligned(16)));

#ifndef CONFIG_LEPTON_CAPTURE_TASK_STACK
#define CONFIG_LEPTON_CAPTURE_TASK_STACK                4096
#endif

#ifndef CONFIG_LEPTON_CAPTURE_TASK_PRIORITY
#define CONFIG_LEPTON_CAPTURE_TASK_PRIORITY             16
#endif

#ifndef CONFIG_LEPTON_CAPTURE_TASK_CORE_AFFINITY
#ifndef CONFIG_LEPTON_CAPTURE_TASK_CORE
#define CONFIG_LEPTON_CAPTURE_TASK_CORE                 1
#endif
#endif

static const char *TAG = "Lepton-Capture";

#ifdef CONFIG_LEPTON_GPIO_USE_VSYNC
/** @brief          VSync interrupt handler.
 *  @param p_Args   Pointer to task arguments
 */
#ifdef CONFIG_LEPTON_VSYNC_PLACE_IRAM
static void IRAM_ATTR Lepton_VSync_ISR_Handler(void *p_Args)
#else
static void Lepton_VSync_ISR_Handler(void *p_Args)
#endif
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xTaskNotifyFromISR((TaskHandle_t)p_Args, 0, eNoAction, &xHigherPriorityTaskWoken);

    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}
#endif

/** @brief          Camera image capture task.
 *  @param p_Args   Pointer to task arguments
 */
static void Lepton_CaptureTask(void *p_Args)
{
    Lepton_t *Device = static_cast<Lepton_t *>(p_Args);

    int ConsecutiveErrors = 0;

    esp_task_wdt_add(NULL);

    ESP_LOGD(TAG, "Capture task started");

    while (Device->Internal.VoSPI.IsCapturing) {
        int Error;
        uint8_t BufferIndex = 0;

        esp_task_wdt_reset();

#ifdef CONFIG_LEPTON_GPIO_USE_VSYNC
        uint32_t ulNotificationValue;
        if (xTaskNotifyWait(0x00, 0xFFFFFFFF, &ulNotificationValue, portMAX_DELAY) != pdTRUE) {
            ESP_LOGW(TAG, "VSync wait failed!");
            continue;
        }
#endif

        /* Capture the frame */
        Error = VoSPI_CaptureImage(&Device->Internal.VoSPI, &BufferIndex);
        if (Error == LEPTON_ERR_OK) {
            ConsecutiveErrors = 0;

            if (Device->Internal.VoSPI.FrameCounter % 100 == 0) {
                if (Device->Internal.FrameQueue != NULL) {
                    ESP_LOGD(TAG, "Captured %u frames, queue space: %d", static_cast<unsigned int>(Device->Internal.VoSPI.FrameCounter),
                             uxQueueSpacesAvailable(Device->Internal.FrameQueue));
                } else {
                    ESP_LOGD(TAG, "Captured %u frames", static_cast<unsigned int>(Device->Internal.VoSPI.FrameCounter));
                }
            }

            /* Frame successfully captured - send pointer to queue if available */
            if (Device->Internal.FrameQueue != NULL) {
                Lepton_FrameBuffer_t FrameBuffer;
                FrameBuffer.Height = Device->Internal.VoSPI.ImageHeight;
                FrameBuffer.Width = Device->Internal.VoSPI.ImageWidth;
                FrameBuffer.BytesPerPixel = Device->Internal.VoSPI.BytesPerPixel;

                /* Use the buffer index returned by VoSPI_CaptureImage */
                FrameBuffer.Image_Buffer = Device->Internal.VoSPI.Image_Buffer[BufferIndex];

                if (Device->Internal.VoSPI.UseTelemetry) {
                    FrameBuffer.Telemetry_Buffer = Device->Internal.VoSPI.Telemetry_Buffer[BufferIndex];
                } else {
                    FrameBuffer.Telemetry_Buffer = NULL;
                }

                /* Use overwrite to always update with latest frame */
                xQueueOverwrite(Device->Internal.FrameQueue, &FrameBuffer);
            } else {
                ESP_LOGE(TAG, "Frame queue is NULL!");
            }
        } else if (Error == LEPTON_ERR_FAIL) {
            /* Sync error - already handled in VoSPI_CaptureImage */
            Device->Internal.VoSPI.SyncErrors++;
            ConsecutiveErrors++;

            if (ConsecutiveErrors % 10 == 1) {
                ESP_LOGW(TAG, "Sync error occurred (consecutive: %d, total: %" PRIu32 ")", ConsecutiveErrors,
                         Device->Internal.VoSPI.SyncErrors);
            }

            /* Longer delay on errors to reduce CPU load during problematic conditions */
            vTaskDelay(pdMS_TO_TICKS(10));
        } else if (Error == LEPTON_ERR_NOT_FINISHED) {
            ESP_LOGD(TAG, "Resyncing...");
            /* Short delay during resyncing */
            vTaskDelay(pdMS_TO_TICKS(1));
        } else {
            ESP_LOGW(TAG, "Unexpected result: %d", Error);
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
}

Lepton_Error_t Lepton_StartCapture(Lepton_t *p_Device, QueueHandle_t p_Queue)
{
    Lepton_Error_t Error;

    if ((p_Device == NULL) || (p_Queue == NULL)) {
        return LEPTON_ERR_INVALID_ARG;
    } else if (p_Device->Internal.IsInitialized == false) {
        return LEPTON_ERR_NOT_INITIALIZED;
    } else if (p_Device->Internal.CapHandle != NULL) {
        return LEPTON_ERR_BUSY;
    }

    Error = LEPTON_ERR_OK;

    p_Device->Internal.FrameQueue = p_Queue;
    p_Device->Internal.VoSPI.CurrentBuffer = 0;
    p_Device->Internal.VoSPI.IsCapturing = true;
    p_Device->Internal.MinSmooth = 0;
    p_Device->Internal.MaxSmooth = 16383;

    ESP_LOGD(TAG, "Creating capture task...");

#ifdef CONFIG_LEPTON_CAPTURE_TASK_CORE_AFFINITY
    xTaskCreatePinnedToCore(&Lepton_CaptureTask, "Lepton_Capture", CONFIG_LEPTON_CAPTURE_TASK_STACK, p_Device,
                            CONFIG_LEPTON_CAPTURE_TASK_PRIORITY, &p_Device->Internal.CapHandle, CONFIG_LEPTON_CAPTURE_TASK_CORE);
#else
    xTaskCreate(&Lepton_CaptureTask, "Lepton_Capture", CONFIG_LEPTON_CAPTURE_TASK_STACK, p_Device,
                CONFIG_LEPTON_CAPTURE_TASK_PRIORITY, &p_Device->Internal.CapHandle);
#endif

    if (p_Device->Internal.CapHandle == NULL) {
        ESP_LOGE(TAG, "Failed to create capture task!");
        p_Device->Internal.VoSPI.IsCapturing = false;
        p_Device->Internal.FrameQueue = NULL;
        Error = LEPTON_ERR_NO_MEM;
        goto Lepton_StartCapture_Error_1;
    }

    ESP_LOGD(TAG, "Capture task created successfully");

    /* V-Sync is a high-level signal. So we need to add a positive edge interrupt */
#ifdef CONFIG_LEPTON_GPIO_USE_VSYNC
    gpio_set_direction(p_Device->Internal.VSync, GPIO_MODE_INPUT);
    gpio_set_pull_mode(p_Device->Internal.VSync, GPIO_PULLDOWN_ONLY);
    gpio_set_intr_type(p_Device->Internal.VSync, GPIO_INTR_POSEDGE);

#ifdef CONFIG_LEPTON_VSYNC_PLACE_IRAM
    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
#else
    gpio_install_isr_service(0);
#endif

    /* Hook ISR handler for specific GPIO */
    gpio_isr_handler_add(p_Device->Internal.VSync, Lepton_VSync_ISR_Handler, p_Device->Internal.CapHandle);
#endif

    ESP_LOGD(TAG, "Capture started successfully");

    return LEPTON_ERR_OK;

Lepton_StartCapture_Error_1:
    ESP_LOGE(TAG, "Initialization error: 0x%X!", static_cast<unsigned int>(Error));

    return Error;
}

Lepton_Error_t Lepton_StopCapture(Lepton_t *p_Device)
{
    if (p_Device == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    } else if (p_Device->Internal.IsInitialized == false) {
        return LEPTON_ERR_NOT_INITIALIZED;
    } else if (p_Device->Internal.CapHandle == NULL) {
        return LEPTON_ERR_OK;
    }

    p_Device->Internal.VoSPI.IsCapturing = false;

    vTaskDelete(p_Device->Internal.CapHandle);
    esp_task_wdt_delete(p_Device->Internal.CapHandle);

    return LEPTON_ERR_OK;
}

__attribute__((optimize("O3")))
bool Lepton_Raw14ToRGB(Lepton_t *p_Device, uint16_t *p_Input, uint8_t *p_Output, int16_t *p_Min, int16_t *p_Max,
                       uint16_t Width,
                       uint16_t Height)
{
    uint16_t min = INT16_MAX;
    uint16_t max = 0;
    uint32_t range;

    if ((p_Device == NULL) || (p_Device->Internal.IsInitialized == false) || ((p_Input == NULL) || (p_Output == NULL))) {
        return false;
    }

    /* -----------------------------------------------------------------------
     * Min/Max pass – 8-wide SIMD using ESP32-S3 PIE Q-registers.
     *
     * u16x8_t maps to a single 128-bit Q-register (8 × uint16_t).
     * The vector ternary  v = (cond) ? a : b  compiles to element-wise
     * PIE compare-and-move (EE.VCMP / EE.VSEL) instructions.
     * Each iteration processes 8 pixels in one set of SIMD operations,
     * reducing loop iterations by 8× compared to the scalar baseline.
     * ----------------------------------------------------------------------- */
    uint32_t PixelCount = static_cast<uint32_t>(Width) * Height;
    uint32_t i = 0;

    /* Initialise 8-lane SIMD accumulators */
    u16x8_t vmin8 = {UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX,
                     UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX};
    u16x8_t vmax8 = {0, 0, 0, 0, 0, 0, 0, 0};

    for (; (i + 8) <= PixelCount; i += 8) {
        u16x8_t v;

        __builtin_memcpy(&v, p_Input + i, sizeof(u16x8_t)); /* unaligned Q-reg load */
        vmin8 = vmin8 < v ? vmin8 : v;  /* EE.VMIN.U16 or equivalent */
        vmax8 = vmax8 > v ? vmax8 : v;  /* EE.VMAX.U16 or equivalent */

        /* Reset watchdog periodically during min/max search */
        if ((i & 0x3FF) == 0) { /* Every 1024 pixels */
            esp_task_wdt_reset();
        }
    }

    /* Horizontal reduction: fold 8 SIMD lanes down to a single scalar */
    for (uint8_t k = 0; k < 8; k++) {
        if (vmin8[k] < min) {
            min = vmin8[k];
        }

        if (vmax8[k] > max) {
            max = vmax8[k];
        }
    }

    /* Scalar tail for the remaining < 8 pixels (160×120 = 19200, remainder 0) */
    for (; i < PixelCount; i++) {
        if (p_Input[i] < min) {
            min = p_Input[i];
        }

        if (p_Input[i] > max) {
            max = p_Input[i];
        }
    }

    /* Smooth min/max to reduce flicker */
    if ((p_Device->Internal.MinSmooth == 0) && (p_Device->Internal.MaxSmooth == 16383)) {
        /* First frame - initialize */
        p_Device->Internal.MinSmooth = min;
        p_Device->Internal.MaxSmooth = max;
    } else {
        /* Exponential moving average */
        p_Device->Internal.MinSmooth = static_cast<uint16_t>(p_Device->Internal.SmoothFactor * p_Device->Internal.MinSmooth +
                                                             (1.0f - p_Device->Internal.SmoothFactor) * min);
        p_Device->Internal.MaxSmooth = static_cast<uint16_t>(p_Device->Internal.SmoothFactor * p_Device->Internal.MaxSmooth +
                                                             (1.0f - p_Device->Internal.SmoothFactor) * max);
    }

    range = p_Device->Internal.MaxSmooth - p_Device->Internal.MinSmooth;

    /* Avoid division by zero */
    if (range < 10) {
        range = 10;
    }

    /* -----------------------------------------------------------------------
     * Normalization pass – two-stage SIMD pipeline:
     *
     * Stage 1 – Subtraction  (dsps_sub_s16, resolved to dsps_sub_s16_aes3 on
     *             ESP32-S3 when CONFIG_DSP_OPTIMIZED=y):
     *   Uses ee.vsubs.s16.ld.incp to subtract MinSmooth from 8 pixels per
     *   PIE-instruction cycle.  Input is reinterpreted as int16_t (safe:
     *   all RAW14 values fit in [0, 16383] < INT16_MAX).
     *   Processing is row-by-row so the per-row delta fits in a 320-byte
     *   stack buffer (LEPTON_IMAGE_WIDTH × 2 bytes).
     *   The 16-byte alignment attribute on both stack buffers activates the
     *   fast aes3 path (no fallback to scalar).
     *
     * Stage 2 – Multiply + clamp  (GCC u32x4_t, 4-wide PIE Q-registers):
     *   Fixed-point multiply (delta * InvRange) >> 16 with cap [0, 255].
     *   Negative delta (pixel < MinSmooth) clamped to 0 during uint32 widen.
     *
     * Palette LUT gather remains scalar (no SIMD gather for 3-byte entries).
     * ----------------------------------------------------------------------- */
    uint32_t InvRange = (255U << 16) / range;
    uint16_t MinSmooth = p_Device->Internal.MinSmooth;

    /* 16-byte aligned stack buffers for aes3 SIMD alignment requirement */
    int16_t MinSmoothRow[LEPTON_IMAGE_WIDTH] __attribute__((aligned(16)));
    int16_t DeltaRow[LEPTON_IMAGE_WIDTH]     __attribute__((aligned(16)));

    /* Fill constant-subtrahend row once */
    for (uint8_t k = 0; k < LEPTON_IMAGE_WIDTH; k++) {
        MinSmoothRow[k] = static_cast<int16_t>(MinSmooth);
    }

    const u32x4_t InvRange4 = {InvRange, InvRange, InvRange, InvRange};
    const u32x4_t Cap255 = {255U, 255U, 255U, 255U};

    /* Apply iron palette – one row per outer iteration */
    for (uint32_t row = 0; row < Height; row++) {
        int k = 0;
        u32x4_t norm;
        uint32_t OutBase;
        const int16_t *RowIn  = reinterpret_cast<const int16_t *>(p_Input + row * Width);
        uint8_t *RowOut = p_Output + row * Width * 3U;

        /* Stage 1: vectorized subtract using dsps_sub_s16_aes3
         * DeltaRow[k] = RowIn[k] - MinSmooth  (shift = 0, no scaling) */
        dsps_sub_s16(RowIn, MinSmoothRow, DeltaRow, static_cast<int>(Width), 1, 1, 1, 0);

        /* Stage 2: 4-wide PIE multiply+shift+clamp, then scalar palette gather */
        for (; (k + 4) <= static_cast<int>(Width); k += 4) {
            u32x4_t delta = {
                DeltaRow[k + 0] > 0 ? static_cast<uint32_t>(DeltaRow[k + 0]) : 0U,
                DeltaRow[k + 1] > 0 ? static_cast<uint32_t>(DeltaRow[k + 1]) : 0U,
                DeltaRow[k + 2] > 0 ? static_cast<uint32_t>(DeltaRow[k + 2]) : 0U,
                DeltaRow[k + 3] > 0 ? static_cast<uint32_t>(DeltaRow[k + 3]) : 0U
            };

            norm = (delta * InvRange4) >> 16U;
            norm = norm > Cap255 ? Cap255 : norm;

            OutBase = static_cast<uint32_t>(k) * 3U;
            RowOut[OutBase +  0] = Lepton_Palette_Iron[norm[0]][0];
            RowOut[OutBase +  1] = Lepton_Palette_Iron[norm[0]][1];
            RowOut[OutBase +  2] = Lepton_Palette_Iron[norm[0]][2];
            RowOut[OutBase +  3] = Lepton_Palette_Iron[norm[1]][0];
            RowOut[OutBase +  4] = Lepton_Palette_Iron[norm[1]][1];
            RowOut[OutBase +  5] = Lepton_Palette_Iron[norm[1]][2];
            RowOut[OutBase +  6] = Lepton_Palette_Iron[norm[2]][0];
            RowOut[OutBase +  7] = Lepton_Palette_Iron[norm[2]][1];
            RowOut[OutBase +  8] = Lepton_Palette_Iron[norm[2]][2];
            RowOut[OutBase +  9] = Lepton_Palette_Iron[norm[3]][0];
            RowOut[OutBase + 10] = Lepton_Palette_Iron[norm[3]][1];
            RowOut[OutBase + 11] = Lepton_Palette_Iron[norm[3]][2];
        }

        /* Scalar tail (160 is divisible by 4 – this loop is never entered) */
        for (; k < static_cast<int>(Width); k++) {
            uint32_t Delta = DeltaRow[k] > 0 ? static_cast<uint32_t>(DeltaRow[k]) : 0U;
            uint32_t normalized = (Delta * InvRange) >> 16;

            if (normalized > 255U) {
                normalized = 255U;
            }

            RowOut[k * 3 + 0] = Lepton_Palette_Iron[normalized][0];
            RowOut[k * 3 + 1] = Lepton_Palette_Iron[normalized][1];
            RowOut[k * 3 + 2] = Lepton_Palette_Iron[normalized][2];
        }

        /* Reset watchdog once per row instead of once per 1024 pixels */
        esp_task_wdt_reset();
    }

    if (p_Min != NULL) {
        *p_Min = static_cast<int16_t>(min);
    }

    if (p_Max != NULL) {
        *p_Max = static_cast<int16_t>(max);
    }

    return true;
}