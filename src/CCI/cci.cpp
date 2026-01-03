/*
 * cci.cpp
 *
 *  Copyright (C) Daniel Kampert, 2026
 *  Website: www.kampis-elektroecke.de
 *  File info: Lepton 3.5 CCI interface implementation.
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

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include <string.h>

#include "cci.h"

#define CCI_ADDRESS                                 0x2A

// CCI register locations
#define CCI_REG_STATUS                              0x0002
#define CCI_REG_COMMAND                             0x0004
#define CCI_REG_DATA_LENGTH                         0x0006
#define CCI_REG_DATA_0                              0x0008
#define CCI_REG_DATA_1                              0x000A
#define CCI_REG_DATA_2                              0x000C
#define CCI_REG_DATA_3                              0x000E
#define CCI_REG_DATA_4                              0x0010
#define CCI_REG_DATA_5                              0x0012
#define CCI_REG_DATA_6                              0x0014
#define CCI_REG_DATA_7                              0x0016
#define CCI_REG_DATA_8                              0x0018
#define CCI_REG_DATA_9                              0x001A
#define CCI_REG_DATA_10                             0x001C
#define CCI_REG_DATA_11                             0x001E
#define CCI_REG_DATA_12                             0x0020
#define CCI_REG_DATA_13                             0x0022
#define CCI_REG_DATA_14                             0x0024
#define CCI_REG_DATA_15                             0x0026
#define CCI_BLOCK_BUF_0                             0xF800
#define CCI_BLOCK_BUF_1                             0xFC00

/**
 * @brief   CCI Commands
 *          Each command is defined by
 *              - A SDK Module ID
 *              - A base
 *              - An offset
 *          Protected commands needs an offset of 0x4000 to disable the protection.
 */
#define CCI_CMD_SDK_MODULE_AGC                      0x0100
#define CCI_CMD_AGC_GET_ENABLE                      (CCI_CMD_SDK_MODULE_AGC + 0x00)
#define CCI_CMD_AGC_SET_ENABLE                      (CCI_CMD_SDK_MODULE_AGC + 0x01)
#define CCI_CMD_AGC_GET_POLICY                      (CCI_CMD_SDK_MODULE_AGC + 0x04)
#define CCI_CMD_AGC_SET_POLICY                      (CCI_CMD_SDK_MODULE_AGC + 0x05)
#define CCI_CMD_AGC_GET_ROI                         (CCI_CMD_SDK_MODULE_AGC + 0x08)
#define CCI_CMD_AGC_SET_ROI                         (CCI_CMD_SDK_MODULE_AGC + 0x09)
#define CCI_CMD_AGC_GET_HISTOGRAM_STATISTICS        (CCI_CMD_SDK_MODULE_AGC + 0x0C)
#define CCI_CMD_AGC_GET_HEQ_DAMPENING               (CCI_CMD_SDK_MODULE_AGC + 0x24)
#define CCI_CMD_AGC_SET_HEQ_DAMPENING               (CCI_CMD_SDK_MODULE_AGC + 0x25)
#define CCI_CMD_AGC_GET_HEQ_CLIP_LIMIT_HIGH         (CCI_CMD_SDK_MODULE_AGC + 0x2C)
#define CCI_CMD_AGC_SET_HEQ_CLIP_LIMIT_HIGH         (CCI_CMD_SDK_MODULE_AGC + 0x2D)
#define CCI_CMD_AGC_GET_HEQ_CLIP_LIMIT_LOW          (CCI_CMD_SDK_MODULE_AGC + 0x30)
#define CCI_CMD_AGC_SET_HEQ_CLIP_LIMIT_LOW          (CCI_CMD_SDK_MODULE_AGC + 0x31)
#define CCI_CMD_AGC_GET_HEQ_EMPTY_COUNTS            (CCI_CMD_SDK_MODULE_AGC + 0x3C)
#define CCI_CMD_AGC_SET_HEQ_EMPTY_COUNTS            (CCI_CMD_SDK_MODULE_AGC + 0x3D)
#define CCI_CMD_AGC_GET_HEQ_OUTPUT_SCALE            (CCI_CMD_SDK_MODULE_AGC + 0x44)
#define CCI_CMD_AGC_SET_HEQ_OUTPUT_SCALE            (CCI_CMD_SDK_MODULE_AGC + 0x45)
#define CCI_CMD_AGC_GET_CALC_ENABLE_STATE           (CCI_CMD_SDK_MODULE_AGC + 0x48)
#define CCI_CMD_AGC_SET_CALC_ENABLE_STATE           (CCI_CMD_SDK_MODULE_AGC + 0x49)
#define CCI_CMD_AGC_GET_HEQ_LINEAR                  (CCI_CMD_SDK_MODULE_AGC + 0x4C)
#define CCI_CMD_AGC_SET_HEQ_LINEAR                  (CCI_CMD_SDK_MODULE_AGC + 0x4D)

#define CCI_CMD_SDK_MODULE_SYS                      0x0200
#define CCI_CMD_SYS_RUN_PING                        (CCI_CMD_SDK_MODULE_SYS + 0x02)
#define CCI_CMD_SYS_GET_SERIALNUMBER                (CCI_CMD_SDK_MODULE_SYS + 0x08)
#define CCI_CMD_SYS_GET_UPTIME                      (CCI_CMD_SDK_MODULE_SYS + 0x0C)
#define CCI_CMD_SYS_GET_AUX_TEMP                    (CCI_CMD_SDK_MODULE_SYS + 0x10)
#define CCI_CMD_SYS_GET_FPA_TEMP                    (CCI_CMD_SDK_MODULE_SYS + 0x14)
#define CCI_CMD_SYS_GET_TELEMETRY_ENABLE_STATE      (CCI_CMD_SDK_MODULE_SYS + 0x18)
#define CCI_CMD_SYS_SET_TELEMETRY_ENABLE_STATE      (CCI_CMD_SDK_MODULE_SYS + 0x19)
#define CCI_CMD_SYS_GET_TELEMETRY_LOCATION          (CCI_CMD_SDK_MODULE_SYS + 0x1C)
#define CCI_CMD_SYS_SET_TELEMETRY_LOCATION          (CCI_CMD_SDK_MODULE_SYS + 0x1D)
#define CCI_CMD_SYS_GET_SCENE_STATISTICS            (CCI_CMD_SDK_MODULE_SYS + 0x2C)
#define CCI_CMD_SYS_GET_SCENE_ROI                   (CCI_CMD_SDK_MODULE_SYS + 0x30)
#define CCI_CMD_SYS_SET_SCENE_ROI                   (CCI_CMD_SDK_MODULE_SYS + 0x31)
#define CCI_CMD_SYS_GET_SHUTTER_POSITION            (CCI_CMD_SDK_MODULE_SYS + 0x38)
#define CCI_CMD_SYS_SET_SHUTTER_POSITION            (CCI_CMD_SDK_MODULE_SYS + 0x39)
#define CCI_CMD_SYS_RUN_FFC                         (CCI_CMD_SDK_MODULE_SYS + 0x42)
#define CCI_CMD_SYS_GET_GAIN_MODE                   (CCI_CMD_SDK_MODULE_SYS + 0x48)
#define CCI_CMD_SYS_SET_GAIN_MODE                   (CCI_CMD_SDK_MODULE_SYS + 0x49)

#define CCI_CMD_SDK_MODULE_VID                      0x0300
#define CCI_CMD_VID_GET_VIDEO_LOOKUP                (CCI_CMD_SDK_MODULE_VID + 0x04)
#define CCI_CMD_VID_SET_VIDEO_LOOKUP                (CCI_CMD_SDK_MODULE_VID + 0x05)
#define CCI_CMD_VID_GET_VIDEO_CUSTOM_LOOKUP         (CCI_CMD_SDK_MODULE_VID + 0x08)
#define CCI_CMD_VID_SET_VIDEO_CUSTOM_LOOKUP         (CCI_CMD_SDK_MODULE_VID + 0x09)
#define CCI_CMD_VID_GET_VIDEO_FOCUS_CALC_ENABLE     (CCI_CMD_SDK_MODULE_VID + 0x0C)
#define CCI_CMD_VID_SET_VIDEO_FOCUS_CALC_ENABLE     (CCI_CMD_SDK_MODULE_VID + 0x0D)
#define CCI_CMD_VID_GET_VIDEO_FOCUS_ROI             (CCI_CMD_SDK_MODULE_VID + 0x10)
#define CCI_CMD_VID_SET_VIDEO_FOCUS_ROI             (CCI_CMD_SDK_MODULE_VID + 0x11)
#define CCI_CMD_VID_GET_VIDEO_FOCUS_THRESHOLD       (CCI_CMD_SDK_MODULE_VID + 0x14)
#define CCI_CMD_VID_SET_VIDEO_FOCUS_THRESHOLD       (CCI_CMD_SDK_MODULE_VID + 0x15)
#define CCI_CMD_VID_GET_VIDEO_FOCUS_METRIC          (CCI_CMD_SDK_MODULE_VID + 0x18)
#define CCI_CMD_VID_GET_VIDEO_FREEZE                (CCI_CMD_SDK_MODULE_VID + 0x24)
#define CCI_CMD_VID_SET_VIDEO_FREEZE                (CCI_CMD_SDK_MODULE_VID + 0x25)
#define CCI_CMD_VID_GET_VIDEO_OUTPUT_FORMAT         (CCI_CMD_SDK_MODULE_VID + 0x30)
#define CCI_CMD_VID_SET_VIDEO_OUTPUT_FORMAT         (CCI_CMD_SDK_MODULE_VID + 0x31)
#define CCI_CMD_VID_GET_VIDEO_PSEUDO_COLOR_SELECT   (CCI_CMD_SDK_MODULE_VID + 0x34)
#define CCI_CMD_VID_SET_VIDEO_PSEUDO_COLOR_SELECT   (CCI_CMD_SDK_MODULE_VID + 0x35)

// Protected commands
#define CCI_CMD_SDK_MODULE_OEM                      0x0800
#define CCI_CMD_OEM_GET_PART_NUM                    (0x4000 + CCI_CMD_SDK_MODULE_OEM + 0x1C)
#define CCI_CMD_OEM_GET_SOFTWARE_REVISION           (0x4000 + CCI_CMD_SDK_MODULE_OEM + 0x20)
#define CCI_CMD_OEM_GET_VIDEO_OUTPUT_SOURCE         (0x4000 + CCI_CMD_SDK_MODULE_OEM + 0x2C)
#define CCI_CMD_OEM_SET_VIDEO_OUTPUT_SOURCE         (0x4000 + CCI_CMD_SDK_MODULE_OEM + 0x2D)
#define CCI_CMD_OEM_GET_VIDEO_OUTPUT_CONSTANT       (0x4000 + CCI_CMD_SDK_MODULE_OEM + 0x3C)
#define CCI_CMD_OEM_SET_VIDEO_OUTPUT_CONSTANT       (0x4000 + CCI_CMD_SDK_MODULE_OEM + 0x3D)
#define CCI_CMD_OEM_RUN_REBOOT                      (0x4000 + CCI_CMD_SDK_MODULE_OEM + 0x42)
#define CCI_CMD_OEM_GET_GPIO_MODE                   (0x4000 + CCI_CMD_SDK_MODULE_OEM + 0x54)
#define CCI_CMD_OEM_SET_GPIO_MODE                   (0x4000 + CCI_CMD_SDK_MODULE_OEM + 0x55)

#define CCI_CMD_SDK_MODULE_RAD                      0x0E00
#define CCI_CMD_RAD_GET_ENABLE_STATE                (0x4000 + CCI_CMD_SDK_MODULE_RAD + 0x10)
#define CCI_CMD_RAD_SET_ENABLE_STATE                (0x4000 + CCI_CMD_SDK_MODULE_RAD + 0x11)
#define CCI_CMD_RAD_GET_FLUX_LINEAR_PARAMS          (0x4000 + CCI_CMD_SDK_MODULE_RAD + 0xBC)
#define CCI_CMD_RAD_SET_FLUX_LINEAR_PARAMS          (0x4000 + CCI_CMD_SDK_MODULE_RAD + 0xBD)
#define CCI_CMD_RAD_GET_TLINEAR_ENABLE_STATE        (0x4000 + CCI_CMD_SDK_MODULE_RAD + 0xC0)
#define CCI_CMD_RAD_SET_TLINEAR_ENABLE_STATE        (0x4000 + CCI_CMD_SDK_MODULE_RAD + 0xC1)
#define CCI_CMD_RAD_GET_TLINEAR_RESOLUTION          (0x4000 + CCI_CMD_SDK_MODULE_RAD + 0xC4)
#define CCI_CMD_RAD_SET_TLINEAR_RESOLUTION          (0x4000 + CCI_CMD_SDK_MODULE_RAD + 0xC5)
#define CCI_CMD_RAD_GET_TLINEAR_AUTO_RES            (0x4000 + CCI_CMD_SDK_MODULE_RAD + 0xC8)
#define CCI_CMD_RAD_SET_TLINEAR_AUTO_RES            (0x4000 + CCI_CMD_SDK_MODULE_RAD + 0xC9)
#define CCI_CMD_RAD_GET_SPOT_ROI                    (0x4000 + CCI_CMD_SDK_MODULE_RAD + 0xCC)
#define CCI_CMD_RAD_SET_SPOT_ROI                    (0x4000 + CCI_CMD_SDK_MODULE_RAD + 0xCD)
#define CCI_CMD_RAD_GET_SPOT                        (0x4000 + CCI_CMD_SDK_MODULE_RAD + 0xD0)

// Bit positions
#define CCI_BIT_BOOT_STATUS_BIT                     0x02
#define CCI_BIT_BOOT_MODE_BIT                       0x01
#define CCI_BIT_BUSY                                0x00

/** @brief Buffer for the I2C burst transmissions.
 */
static uint8_t _CCI_Buffer[1026];

/** @brief Lepton CCI I2C device configuration object.
 */
static i2c_device_config_t _CCI_I2C_Config = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = CCI_ADDRESS,
    .scl_speed_hz = 400000,
    .scl_wait_us = 0,
    .flags = {
        .disable_ack_check = 0,
    },
};

static const char *TAG                  = "cci";

/** @brief              Wait as lon as the camera is busy.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param p_Status     (Optional) Response error code from the camera
 *                      NOTE: This field can be set to NULL to ignore the status
 *  @param Timeout      (Optional) Timeout in milliseconds
 *  @return             LEPTON_ERR_OK when successful
 */
static Lepton_Error_t CCI_WaitBusy(CCI_t *p_Interface, Lepton_Result_t *p_Status = NULL, const uint32_t Timeout = 5000)
{
    Lepton_Error_t Error;
    uint32_t TimeoutCounter = 0;

    if ((p_Interface == NULL) || (p_Interface->I2C_Write == NULL) || (p_Interface->I2C_Read == NULL) ||
        (p_Interface->Mutex == NULL)) {
        return LEPTON_ERR_INVALID_ARG;
    } else if (p_Interface->isInitialized == false) {
        return LEPTON_ERR_INVALID_STATE;
    }

    Error = LEPTON_ERR_OK;

    do {
        _CCI_Buffer[0] = (CCI_REG_STATUS >> 8) & 0xFF;
        _CCI_Buffer[1] = CCI_REG_STATUS & 0xFF;

        xSemaphoreTake(p_Interface->Mutex, portMAX_DELAY);
        if ((p_Interface->I2C_Write(&p_Interface->I2C_Dev_Handle, _CCI_Buffer, 2) != 0) ||
            (p_Interface->I2C_Read(&p_Interface->I2C_Dev_Handle, _CCI_Buffer, 2) != 0)) {
            ESP_LOGE(TAG, "Failed to access Status register!");

            xSemaphoreGive(p_Interface->Mutex);

            return LEPTON_ERR_FAIL;
        }

        xSemaphoreGive(p_Interface->Mutex);

        vTaskDelay(10 / portTICK_PERIOD_MS);
        TimeoutCounter += 10;

        if (TimeoutCounter >= Timeout) {
            ESP_LOGE(TAG, "Timeout waiting for camera ready (Status: 0x%02X%02X, Expected: 0xXX06)", _CCI_Buffer[0], _CCI_Buffer[1]);
            return LEPTON_ERR_TIMEOUT;
        }
    } while ((_CCI_Buffer[1] & 0x07) != ((0x01 << CCI_BIT_BOOT_STATUS_BIT) | (0x01 << CCI_BIT_BOOT_MODE_BIT)));

    if (p_Status != NULL) {
        *p_Status = static_cast<Lepton_Result_t>(_CCI_Buffer[0]);
    }

    return Error;
}

/** @brief              Write a 16-bit value into a register.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param Register     Register address
 *  @param Value        Register value
 *  @return             LEPTON_ERR_OK when successful
 */
static Lepton_Error_t CCI_WriteRegister(CCI_t *p_Interface, uint16_t Register, uint16_t Value)
{
    Lepton_Error_t Error;

    if ((p_Interface == NULL) || (p_Interface->I2C_Write == NULL) || (p_Interface->Mutex == NULL)) {
        return LEPTON_ERR_INVALID_ARG;
    } else if (p_Interface->isInitialized == false) {
        return LEPTON_ERR_INVALID_STATE;
    }

    Error = LEPTON_ERR_OK;
    _CCI_Buffer[0] = static_cast<uint8_t>((Register >> 8) & 0xFF);
    _CCI_Buffer[1] = static_cast<uint8_t>(Register & 0xFF);
    _CCI_Buffer[2] = static_cast<uint8_t>((Value >> 8) & 0xFF);
    _CCI_Buffer[3] = static_cast<uint8_t>(Value & 0xFF);

    xSemaphoreTake(p_Interface->Mutex, portMAX_DELAY);
    if (p_Interface->I2C_Write(&p_Interface->I2C_Dev_Handle, _CCI_Buffer, 4) != 0) {
        xSemaphoreGive(p_Interface->Mutex);

        ESP_LOGE(TAG, "Failed to write CCI register %02x with value %02x!", Register, Value);

        return LEPTON_ERR_FAIL;
    };
    xSemaphoreGive(p_Interface->Mutex);

    return Error;
}

/** @brief              Perform a write burst to the register (up to 512 words).
 *  @param p_Interface  Pointer to CCI interface object
 *  @param Start        Start address
 *  @param Length       Transmission length
 *  @param p_Buf        Pointer to buffer
 *  @return             LEPTON_ERR_OK when successful
 */
static Lepton_Error_t CCI_WriteBurst(CCI_t *p_Interface, uint16_t Start, uint16_t Length, const uint16_t *p_Buf)
{
    Lepton_Error_t Error;

    if ((p_Interface == NULL) || (Length == 0) || (p_Buf == NULL) || (p_Interface->I2C_Write == NULL) ||
        (p_Interface->Mutex == NULL)) {
        return LEPTON_ERR_INVALID_ARG;
    } else if (p_Interface->isInitialized == false) {
        return LEPTON_ERR_INVALID_STATE;
    }

    Error = LEPTON_ERR_OK;

    ESP_LOGD(TAG, "Perform write burst. Start at %u, write %u from %p", Start, Length, p_Buf);

    // Copy the start address in the buffer.
    _CCI_Buffer[0] = static_cast<uint8_t>(Start >> 8);
    _CCI_Buffer[1] = static_cast<uint8_t>(Start & 0xFF);

    // Copy the data in the buffer.
    for (uint32_t i = 1; i <= Length; i++) {
        _CCI_Buffer[i << 1] = static_cast<uint8_t>(*p_Buf >> 8);
        _CCI_Buffer[(i << 1) + 1] = static_cast<uint8_t>(*p_Buf++ & 0xFF);
    }

    xSemaphoreTake(p_Interface->Mutex, portMAX_DELAY);
    if (p_Interface->I2C_Write(&p_Interface->I2C_Dev_Handle, _CCI_Buffer, (Length << 1) + 2) != 0) {
        ESP_LOGE(TAG, "Failed to perform write burst at CCI register %02x with length %u!", Start, Length);

        Error = LEPTON_ERR_FAIL;
    };
    xSemaphoreGive(p_Interface->Mutex);

    return Error;
}

/** @brief              Read a 16-bit value from a register.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param Register     Register address
 *  @param p_Value      Pointer to register value
 *  @return             LEPTON_ERR_OK when successful
 */
static Lepton_Error_t CCI_ReadRegister(CCI_t *p_Interface, uint16_t Register, uint16_t *p_Value)
{
    if ((p_Interface == NULL) || (p_Value == NULL) || (p_Interface->I2C_Write == NULL) || (p_Interface->I2C_Read == NULL) ||
        (p_Interface->Mutex == NULL)) {
        return LEPTON_ERR_INVALID_ARG;
    } else if (p_Interface->isInitialized == false) {
        return LEPTON_ERR_INVALID_STATE;
    }

    _CCI_Buffer[0] = static_cast<uint8_t>(Register >> 8);
    _CCI_Buffer[1] = static_cast<uint8_t>(Register & 0xFF);

    xSemaphoreTake(p_Interface->Mutex, portMAX_DELAY);
    if ((p_Interface->I2C_Write(&p_Interface->I2C_Dev_Handle, _CCI_Buffer, 2) != 0) ||
        (p_Interface->I2C_Read(&p_Interface->I2C_Dev_Handle, _CCI_Buffer, 2) != 0)) {
        ESP_LOGE(TAG, "Failed to access CCI register %02x!", Register);

        xSemaphoreGive(p_Interface->Mutex);
        return LEPTON_ERR_FAIL;
    }

    *p_Value = (_CCI_Buffer[0] << 8) | _CCI_Buffer[1];

    xSemaphoreGive(p_Interface->Mutex);

    return LEPTON_ERR_OK;
}

/** @brief              Perform a read burst to the register (up to 512 words).
 *  @param p_Interface  Pointer to CCI interface object
 *  @param Start        Start address
 *  @param Length       Transmission length
 *  @param p_Buf        Pointer to buffer
 *  @return             LEPTON_ERR_OK when successful
 */
static Lepton_Error_t CCI_ReadBurst(CCI_t *p_Interface, uint16_t Start, uint16_t Length, uint16_t *p_Buf)
{
    Lepton_Error_t Error;

    if ((p_Interface == NULL) || (Length == 0) || (p_Buf == NULL) || (p_Interface->I2C_Read == NULL) ||
        (p_Interface->Mutex == NULL)) {
        return LEPTON_ERR_INVALID_ARG;
    } else if (p_Interface->isInitialized == false) {
        return LEPTON_ERR_INVALID_STATE;
    }

    Error = LEPTON_ERR_OK;

    ESP_LOGD(TAG, "Perform read burst. Start at %u, read %u to %p", Start, Length, p_Buf);

    // Copy the start address in the buffer.
    _CCI_Buffer[0] = static_cast<uint8_t>(Start >> 8);
    _CCI_Buffer[1] = static_cast<uint8_t>(Start & 0xFF);

    xSemaphoreTake(p_Interface->Mutex, portMAX_DELAY);
    if ((p_Interface->I2C_Write(&p_Interface->I2C_Dev_Handle, _CCI_Buffer, 2) != 0) ||
        (p_Interface->I2C_Read(&p_Interface->I2C_Dev_Handle, _CCI_Buffer, Length << 1) != 0)) {
        ESP_LOGE(TAG, "Failed to initiate CCI read burst at register %02x!", Start);

        Error = LEPTON_ERR_FAIL;
        goto CCI_ReadBurst_Exit;
    }

    // Copy the data in the buffer.
    for (uint32_t i = 0; i < Length; i++) {
        *p_Buf++ = (_CCI_Buffer[i << 1] << 8) | _CCI_Buffer[(i << 1) + 1];
    }

CCI_ReadBurst_Exit:
    xSemaphoreGive(p_Interface->Mutex);

    return Error;
}

/** @brief              Sets a ROI structure to the camera.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param Command      Command to set the ROI
 *  @param p_ROI        Pointer to ROI structure
 *  @param p_Status     (Optional) Response error code from the camera
 *  @return             LEPTON_ERR_OK when successful
 */
static Lepton_Error_t CCI_SetROI(CCI_t *p_Interface, uint16_t Command, const Lepton_ROI_t *p_ROI, Lepton_Result_t *p_Status)
{
    uint16_t Buffer[4];

    if (p_ROI == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    }

    Buffer[0] = p_ROI->Start_Col;
    Buffer[1] = p_ROI->Start_Row;
    Buffer[2] = p_ROI->End_Row;
    Buffer[3] = p_ROI->End_Col;

    return CCI_Set(p_Interface, Command, 4, Buffer, p_Status);
}

/** @brief              Gets a ROI structure from the camera.
 *  @param p_Interface  Pointer to CCI interface object
 *  @param Command      Command to set the ROI
 *  @param p_ROI        Pointer to ROI structure
 *  @param p_Status     (Optional) Response error code from the camera
 *  @return             LEPTON_ERR_OK when successful
 */
static Lepton_Error_t CCI_GetROI(CCI_t *p_Interface, uint16_t Command, Lepton_ROI_t *p_ROI, Lepton_Result_t *p_Status)
{
    uint16_t Buffer[4];
    Lepton_Error_t Error;

    if (p_ROI == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    }

    Error = CCI_Get(p_Interface, CCI_CMD_AGC_GET_ROI, 4, Buffer, p_Status);
    if (Error != LEPTON_ERR_OK) {
        return Error;
    }

    p_ROI->Start_Col = Buffer[0];
    p_ROI->Start_Row = Buffer[1];
    p_ROI->End_Row = Buffer[2];
    p_ROI->End_Col = Buffer[3];

    return LEPTON_ERR_OK;
}

Lepton_Error_t CCI_Init(CCI_t *p_Interface)
{
    if (p_Interface == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    } else if (p_Interface->isInitialized == true) {
        return LEPTON_ERR_OK;
    }

    if (p_Interface->Mutex == NULL) {
        p_Interface->Mutex = xSemaphoreCreateMutex();
        if (p_Interface->Mutex == NULL) {
            return LEPTON_ERR_FAIL;
        }
    }

    if ((p_Interface->I2C_Init != NULL) && (p_Interface->I2C_Bus_Config != NULL)) {
        ESP_LOGD(TAG, " I2C:");
        ESP_LOGD(TAG, "  Interface: %u", static_cast<unsigned int>(p_Interface->I2C_Bus_Config->i2c_port));
        ESP_LOGD(TAG, "  Clock: %u", static_cast<unsigned int>(_CCI_I2C_Config.scl_speed_hz));
        ESP_LOGD(TAG, "  SCL: %u", static_cast<unsigned int>(p_Interface->I2C_Bus_Config->scl_io_num));
        ESP_LOGD(TAG, "  SDA: %u", static_cast<unsigned int>(p_Interface->I2C_Bus_Config->sda_io_num));

        if (p_Interface->I2C_Init(p_Interface->I2C_Bus_Config, &p_Interface->I2C_Bus_Handle)) {
            return LEPTON_ERR_FAIL;
        }
    }

    if (i2c_master_bus_add_device(p_Interface->I2C_Bus_Handle, &_CCI_I2C_Config, &p_Interface->I2C_Dev_Handle) != ESP_OK) {
        return LEPTON_ERR_FAIL;
    }

    p_Interface->isInitialized = true;

    return LEPTON_ERR_OK;
}

Lepton_Error_t CCI_Deinit(CCI_t *p_Interface)
{
    if (p_Interface == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    } else if (p_Interface->isInitialized == false) {
        return LEPTON_ERR_OK;
    }

    if (p_Interface->I2C_Deinit != NULL) {
        p_Interface->I2C_Deinit(p_Interface->I2C_Bus_Handle);
    }

    if (p_Interface->Mutex != NULL) {
        vSemaphoreDelete(p_Interface->Mutex);
        p_Interface->Mutex = NULL;
    }

    p_Interface->isInitialized = false;

    return LEPTON_ERR_OK;
}

Lepton_Error_t CCI_Set(CCI_t *p_Interface, uint16_t Command, uint16_t Length, const uint16_t *p_Buffer,
                       Lepton_Result_t *p_Status)
{
    uint16_t Start;

    if ((Length > 512) || (Length == 0)) {
        return LEPTON_ERR_INVALID_ARG;
    }

    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));

    Start = CCI_REG_DATA_0;
    if ((Length > 16) && (Length <= 512)) {
        Start = CCI_BLOCK_BUF_0;
    }

    // First: Write the data.
    LEPTON_ERROR_CHECK(CCI_WriteBurst(p_Interface, Start, Length, p_Buffer));

    // Second: Execute the command when the data transmission was successful.
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, Length));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, Command));

    return CCI_WaitBusy(p_Interface, p_Status);
}

Lepton_Error_t CCI_Get(CCI_t *p_Interface, uint16_t Command, uint16_t Length, uint16_t *p_Buffer,
                       Lepton_Result_t *p_Status)
{
    uint16_t Start;

    if ((Length == 0) || (p_Buffer == NULL)) {
        return LEPTON_ERR_INVALID_ARG;
    }

    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));

    // First: Execute the command when the device is not busy anymore.
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, Length));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, Command));

    // Wait for command to complete
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));

    // Second: Read the data.
    Start = CCI_REG_DATA_0;
    if ((Length > 16) && (Length <= 512)) {
        Start = CCI_BLOCK_BUF_0;
    }

    return CCI_ReadBurst(p_Interface, Start, Length, p_Buffer);
}

Lepton_Error_t CCI_WaitForBoot(CCI_t *p_Interface, Lepton_Result_t *p_Status)
{
    ESP_LOGI(TAG, "Waiting for camera boot (timeout: 10s)...");
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status, 10000));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_SYS_RUN_PING));

    return CCI_WaitBusy(p_Interface, p_Status, 10000);
}

Lepton_Error_t CCI_GetPartNumber(CCI_t *p_Interface, char *p_PartNumber, Lepton_Result_t *p_Status)
{
    uint16_t Buffer[16];

    if (p_PartNumber == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    }

    memset(p_PartNumber, '\0', 33);

    LEPTON_ERROR_CHECK(CCI_Get(p_Interface, CCI_CMD_OEM_GET_PART_NUM, 16, Buffer, p_Status));

    for (uint8_t i = 0; i < 16; i++) {
        *(p_PartNumber++) = static_cast<char>(Buffer[i] & 0xFF);
        *(p_PartNumber++) = static_cast<char>(Buffer[i] >> 8);
    }

    return LEPTON_ERR_OK;
}

Lepton_Error_t CCI_GetSoftwareVersion(CCI_t *p_Interface, Lepton_Version_t *p_Version, Lepton_Result_t *p_Status)
{
    if (p_Version == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    }

    LEPTON_ERROR_CHECK(CCI_Get(p_Interface, CCI_CMD_OEM_GET_SOFTWARE_REVISION, sizeof(Lepton_Version_t),
                               reinterpret_cast<uint16_t *>(p_Version), p_Status));

    return LEPTON_ERR_OK;
}

Lepton_Error_t CCI_GetSerialNumber(CCI_t *p_Interface, uint8_t *p_Serial, Lepton_Result_t *p_Status)
{
    uint16_t Buffer[4] = {0};
    Lepton_Error_t Error;

    if (p_Serial == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    }

    /* Initialize serial buffer */
    memset(p_Serial, 0, 8);

    Error = CCI_Get(p_Interface, CCI_CMD_SYS_GET_SERIALNUMBER, 4, Buffer, p_Status);
    if (Error != LEPTON_ERR_OK) {
        return Error;
    }

    /* Convert from 16-bit words to byte array - matches Lepton CCI byte order */
    for (int i = 0; i < 4; i++) {
        p_Serial[i * 2] = (Buffer[i] >> 8) & 0xFF;      // High byte first
        p_Serial[i * 2 + 1] = Buffer[i] & 0xFF;         // Low byte second
    }

    return LEPTON_ERR_OK;
}

Lepton_Error_t CCI_GetUptime(CCI_t *p_Interface, uint32_t *p_Uptime, Lepton_Result_t *p_Status)
{
    uint16_t Low;
    uint16_t High;

    if (p_Uptime == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    }

    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 2));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_SYS_GET_UPTIME));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_0, &Low));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_1, &High));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));

    *p_Uptime = static_cast<uint32_t>((High << 16) | Low);

    return LEPTON_ERR_OK;
}

Lepton_Error_t CCI_GetAuxTemp(CCI_t *p_Interface, uint16_t *p_Temp, Lepton_Result_t *p_Status)
{
    if (p_Temp == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    }

    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 1));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_SYS_GET_AUX_TEMP));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_0, p_Temp));

    return CCI_WaitBusy(p_Interface, p_Status);
}

Lepton_Error_t CCI_GetFPATemp(CCI_t *p_Interface, uint16_t *p_Temp, Lepton_Result_t *p_Status)
{
    if (p_Temp == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    }

    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 1));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_SYS_GET_FPA_TEMP));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_0, p_Temp));

    return CCI_WaitBusy(p_Interface, p_Status);
}

Lepton_Error_t CCI_RebootCamera(CCI_t *p_Interface, Lepton_Result_t *p_Status)
{
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_OEM_RUN_REBOOT));

    /* Wait for the reboot */
    vTaskDelay(6000 / portTICK_PERIOD_MS);

    return CCI_WaitBusy(p_Interface, p_Status);
}

Lepton_Error_t CCI_SetGPIOMode(CCI_t *p_Interface, Lepton_GPIO_t Mode, Lepton_Result_t *p_Status)
{
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_0, Mode & 0xFFFF));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_1, (Mode >> 16) & 0xFFFF));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 2));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_OEM_SET_GPIO_MODE));

    return CCI_WaitBusy(p_Interface, p_Status);
}

Lepton_Error_t CCI_GetGPIOMode(CCI_t *p_Interface, Lepton_GPIO_t *p_Mode, Lepton_Result_t *p_Status)
{
    uint16_t Low;
    uint16_t High;

    if (p_Mode == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    }

    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 2));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_OEM_GET_GPIO_MODE));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_0, &Low));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_1, &High));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));

    *p_Mode = static_cast<Lepton_GPIO_t>((High << 16) | Low);

    ESP_LOGD(TAG, "GPIO Mode: %u", static_cast<unsigned int>(*p_Mode));

    return LEPTON_ERR_OK;
}

Lepton_Error_t CCI_SetRadiometry(CCI_t *p_Interface, bool Enable, Lepton_Result_t *p_Status)
{
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_0, Enable & 0xFFFF));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_1, (Enable >> 16) & 0xFFFF));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 2));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_RAD_SET_ENABLE_STATE));

    return CCI_WaitBusy(p_Interface, p_Status);
}

Lepton_Error_t CCI_GetRadiometry(CCI_t *p_Interface, bool *p_Enable, Lepton_Result_t *p_Status)
{
    uint16_t Low;
    uint16_t High;

    if (p_Enable == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    }

    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 2));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_RAD_GET_ENABLE_STATE));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_0, &Low));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_1, &High));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));

    *p_Enable = static_cast<bool>((High << 16) | Low);

    return LEPTON_ERR_OK;
}

Lepton_Error_t CCI_SetAGCEnabled(CCI_t *p_Interface, bool Enable, Lepton_Result_t *p_Status)
{
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_0, Enable & 0xFFFF));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_1, (Enable >> 16) & 0xFFFF));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 2));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_AGC_SET_ENABLE));

    return CCI_WaitBusy(p_Interface, p_Status);
}

Lepton_Error_t CCI_GetAGCEnabled(CCI_t *p_Interface, bool *p_Enable, Lepton_Result_t *p_Status)
{
    uint16_t Low;
    uint16_t High;

    if (p_Enable == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    }

    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 2));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_AGC_GET_ENABLE));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_0, &Low));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_1, &High));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));

    *p_Enable = static_cast<bool>((High << 16) | Low);

    return LEPTON_ERR_OK;
}

Lepton_Error_t CCI_SetAGCCalc(CCI_t *p_Interface, bool Enable, Lepton_Result_t *p_Status)
{
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_0, Enable & 0xFFFF));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_1, (Enable >> 16) & 0xFFFF));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 2));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_AGC_SET_CALC_ENABLE_STATE));

    return CCI_WaitBusy(p_Interface, p_Status);
}

Lepton_Error_t CCI_GetAGCCalc(CCI_t *p_Interface, bool *p_Enable, Lepton_Result_t *p_Status)
{
    uint16_t Low;
    uint16_t High;

    if (p_Enable == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    }

    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 2));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_AGC_GET_CALC_ENABLE_STATE));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_0, &Low));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_1, &High));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));

    *p_Enable = static_cast<bool>((High << 16) | Low);

    return LEPTON_ERR_OK;
}

Lepton_Error_t CCI_SetTLinearEnabled(CCI_t *p_Interface, bool Enable, Lepton_Result_t *p_Status)
{
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_0, Enable & 0xFFFF));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_1, (Enable >> 16) & 0xFFFF));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 2));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_RAD_SET_TLINEAR_ENABLE_STATE));

    return CCI_WaitBusy(p_Interface, p_Status);
}

Lepton_Error_t CCI_GetTLinearEnabled(CCI_t *p_Interface, bool *p_Enable, Lepton_Result_t *p_Status)
{
    uint16_t Low;
    uint16_t High;

    if (p_Enable == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    }

    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 2));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_RAD_GET_TLINEAR_ENABLE_STATE));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_0, &Low));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_1, &High));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));

    *p_Enable = static_cast<bool>((High << 16) | Low);

    return LEPTON_ERR_OK;
}

Lepton_Error_t CCI_SetTelemetry(CCI_t *p_Interface, bool Enable, Lepton_Result_t *p_Status)
{
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_0, Enable & 0xFFFF));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_1, (Enable >> 16) & 0xFFFF));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 2));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_SYS_SET_TELEMETRY_ENABLE_STATE));

    return CCI_WaitBusy(p_Interface, p_Status);
}

Lepton_Error_t CCI_GetTelemetry(CCI_t *p_Interface, bool *p_Enable, Lepton_Result_t *p_Status)
{
    uint16_t Low;
    uint16_t High;

    if (p_Enable == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    }

    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 2));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_SYS_GET_TELEMETRY_ENABLE_STATE));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_0, &Low));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_1, &High));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));

    *p_Enable = static_cast<bool>((High << 16) | Low);

    return LEPTON_ERR_OK;
}

Lepton_Error_t CCI_SetTelemetryPosition(CCI_t *p_Interface, Lepton_TelemetryPos_t Position, Lepton_Result_t *p_Status)
{
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_0, Position & 0xFFFF));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_1, (Position >> 16) & 0xFFFF));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 2));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_SYS_SET_TELEMETRY_LOCATION));

    return CCI_WaitBusy(p_Interface, p_Status);
}

Lepton_Error_t CCI_GetTelemetryPosition(CCI_t *p_Interface, Lepton_TelemetryPos_t *p_Position,
                                        Lepton_Result_t *p_Status)
{
    uint16_t Low;
    uint16_t High;

    if (p_Position == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    }

    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 2));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_SYS_GET_TELEMETRY_LOCATION));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_0, &Low));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_1, &High));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));

    *p_Position = static_cast<Lepton_TelemetryPos_t>((High << 16) | Low);

    return LEPTON_ERR_OK;
}

Lepton_Error_t CCI_SetRadiometryTLinearAutoRes(CCI_t *p_Interface, bool Enable, Lepton_Result_t *p_Status)
{
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_0, Enable & 0xFFFF));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_1, (Enable >> 16) & 0xFFFF));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 2));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_RAD_SET_TLINEAR_AUTO_RES));

    return CCI_WaitBusy(p_Interface, p_Status);
}

Lepton_Error_t CCI_GetRadiometryTLinearAutoRes(CCI_t *p_Interface, bool *p_Enable, Lepton_Result_t *p_Status)
{
    uint16_t Low;
    uint16_t High;

    if (p_Enable == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    }

    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 2));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_RAD_GET_TLINEAR_AUTO_RES));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_0, &Low));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_1, &High));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));

    *p_Enable = static_cast<bool>((High << 16) | Low);

    return LEPTON_ERR_OK;
}

Lepton_Error_t CCI_SetGainMode(CCI_t *p_Interface, Lepton_Gain_t Mode, Lepton_Result_t *p_Status)
{
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_0, Mode & 0xFFFF));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_1, (Mode >> 16) & 0xFFFF));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 2));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_SYS_SET_GAIN_MODE));

    return CCI_WaitBusy(p_Interface, p_Status);
}

Lepton_Error_t CCI_GetGainMode(CCI_t *p_Interface, Lepton_Gain_t *p_Mode, Lepton_Result_t *p_Status)
{
    uint16_t Low;
    uint16_t High;

    if (p_Mode == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    }

    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 2));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_SYS_GET_GAIN_MODE));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_0, &Low));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_1, &High));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));

    *p_Mode = static_cast<Lepton_Gain_t>((High << 16) | Low);

    return LEPTON_ERR_OK;
}

Lepton_Error_t CCI_SetRadiometryFluxLinearParams(CCI_t *p_Interface, Lepton_FluxLinearParams_t *p_Params,
                                                 Lepton_Result_t *p_Status)
{
    if (p_Params == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    }

    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_0, p_Params->sceneEmissivity));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_1, p_Params->TBkgK));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_2, p_Params->tauWindow));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_3, p_Params->TWindowK));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_4, p_Params->tauAtm));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_5, p_Params->TAtmK));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_6, p_Params->reflWindow));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_7, p_Params->TReflK));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 8));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_RAD_SET_FLUX_LINEAR_PARAMS));

    return CCI_WaitBusy(p_Interface, p_Status);
}

Lepton_Error_t CCI_GetRadiometryFluxLinearParams(CCI_t *p_Interface, Lepton_FluxLinearParams_t *p_Params,
                                                 Lepton_Result_t *p_Status)
{
    if (p_Params == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    }

    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 8));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_RAD_GET_FLUX_LINEAR_PARAMS));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_0, &p_Params->sceneEmissivity));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_1, &p_Params->TBkgK));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_2, &p_Params->tauWindow));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_3, &p_Params->TWindowK));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_4, &p_Params->tauAtm));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_5, &p_Params->TAtmK));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_6, &p_Params->reflWindow));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_7, &p_Params->TReflK));

    return CCI_WaitBusy(p_Interface, p_Status);
}

Lepton_Error_t CCI_SetVideoFormat(CCI_t *p_Interface, Lepton_VideoFormat_t Format, Lepton_Result_t *p_Status)
{
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_0, Format & 0xFFFF));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_1, (Format >> 16) & 0xFFFF));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 2));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_VID_SET_VIDEO_OUTPUT_FORMAT));

    return CCI_WaitBusy(p_Interface, p_Status);
}

Lepton_Error_t CCI_GetVideoFormat(CCI_t *p_Interface, Lepton_VideoFormat_t *p_Format, Lepton_Result_t *p_Status)
{
    uint16_t Low;
    uint16_t High;

    if (p_Format == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    }

    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 2));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_VID_GET_VIDEO_OUTPUT_FORMAT));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_0, &Low));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_1, &High));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));

    *p_Format = static_cast<Lepton_VideoFormat_t>((High << 16) | Low);

    return LEPTON_ERR_OK;
}

Lepton_Error_t CCI_SetVideoSource(CCI_t *p_Interface, Lepton_VideoSource_t Source, uint16_t Constant,
                                  Lepton_Result_t *p_Status)
{
    if ((Source == LEPTON_SOURCE_CONSTANT) && (Constant > 16383)) {
        return LEPTON_ERR_INVALID_ARG;
    } else if (Source == LEPTON_SOURCE_RAMP_CUSTOM) {
        // TODO: Not supported yet
        return LEPTON_ERR_INVALID_ARG;
    }

    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_0, Source & 0xFFFF));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_1, (Source >> 16) & 0xFFFF));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 2));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_OEM_SET_VIDEO_OUTPUT_SOURCE));

    /* Wait for VIDEO_OUTPUT_SOURCE command to complete */
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));

    if (Source == LEPTON_SOURCE_CONSTANT) {
        LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_0, Constant & 0xFFFF));
        LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_1, (Constant >> 16) & 0xFFFF));
        LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 2));
        LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_OEM_SET_VIDEO_OUTPUT_CONSTANT));

        /* Wait for VIDEO_OUTPUT_CONSTANT command to complete */
        return CCI_WaitBusy(p_Interface, p_Status);
    }

    return LEPTON_ERR_OK;
}

Lepton_Error_t CCI_GetVideoSource(CCI_t *p_Interface, Lepton_VideoSource_t *p_Source, Lepton_Result_t *p_Status)
{
    uint16_t Low;
    uint16_t High;

    if (p_Source == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    }

    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 2));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_OEM_GET_VIDEO_OUTPUT_SOURCE));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_0, &Low));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_1, &High));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));

    *p_Source = static_cast<Lepton_VideoSource_t>((High << 16) | Low);

    return LEPTON_ERR_OK;
}

Lepton_Error_t CCI_SetShutterPosition(CCI_t *p_Interface, Lepton_ShutterPos_t Position, Lepton_Result_t *p_Status)
{
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_0, Position & 0xFFFF));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_1, (Position >> 16) & 0xFFFF));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 2));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_SYS_SET_SHUTTER_POSITION));

    return CCI_WaitBusy(p_Interface, p_Status);
}

Lepton_Error_t CCI_GetShutterPosition(CCI_t *p_Interface, Lepton_ShutterPos_t *p_Position, Lepton_Result_t *p_Status)
{
    uint16_t Low;
    uint16_t High;

    if (p_Position == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    }

    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 2));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_SYS_GET_SHUTTER_POSITION));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_0, &Low));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_1, &High));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));

    *p_Position = static_cast<Lepton_ShutterPos_t>((High << 16) | Low);

    return LEPTON_ERR_OK;
}

Lepton_Error_t CCI_SetAGCROI(CCI_t *p_Interface, const Lepton_ROI_t *p_ROI, Lepton_Result_t *p_Status)
{
    return CCI_SetROI(p_Interface, CCI_CMD_AGC_SET_ROI, p_ROI, p_Status);
}

Lepton_Error_t CCI_GetAGCROI(CCI_t *p_Interface, Lepton_ROI_t *p_ROI, Lepton_Result_t *p_Status)
{
    return CCI_GetROI(p_Interface,CCI_CMD_AGC_GET_ROI, p_ROI, p_Status);
}

Lepton_Error_t CCI_SetSceneROI(CCI_t *p_Interface, const Lepton_ROI_t *p_ROI, Lepton_Result_t *p_Status)
{
    return CCI_SetROI(p_Interface, CCI_CMD_SYS_SET_SCENE_ROI, p_ROI, p_Status);
}

Lepton_Error_t CCI_GetSceneROI(CCI_t *p_Interface, Lepton_ROI_t *p_ROI, Lepton_Result_t *p_Status)
{
    return CCI_GetROI(p_Interface, CCI_CMD_SYS_GET_SCENE_ROI, p_ROI, p_Status);
}

Lepton_Error_t CCI_SetSpotmeterROI(CCI_t *p_Interface, const Lepton_ROI_t *p_ROI, Lepton_Result_t *p_Status)
{
    return CCI_SetROI(p_Interface, CCI_CMD_RAD_SET_SPOT_ROI, p_ROI, p_Status);
}

Lepton_Error_t CCI_GetSpotmeterROI(CCI_t *p_Interface, Lepton_ROI_t *p_ROI, Lepton_Result_t *p_Status)
{
    return CCI_GetROI(p_Interface, CCI_CMD_RAD_GET_SPOT_ROI, p_ROI, p_Status);
}

Lepton_Error_t CCI_GetSpotmeter(CCI_t *p_Interface, Lepton_Spotmeter_t *p_Spot, Lepton_Result_t *p_Status)
{
    if (p_Spot == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    }

    uint16_t temp_value, temp_max, temp_min;
    
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 8));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_RAD_GET_SPOT));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_0, &temp_value));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_1, &temp_max));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_2, &temp_min));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_3, &p_Spot->Population));
    
    p_Spot->Value = temp_value;
    p_Spot->Max = temp_max;
    p_Spot->Min = temp_min;
    
    return CCI_WaitBusy(p_Interface, p_Status);
}

Lepton_Error_t CCI_GetSceneStatistics(CCI_t *p_Interface, Lepton_SceneStatistics_t *p_Statistics,
                                      Lepton_Result_t *p_Status)
{
    uint16_t Buffer[4];
    Lepton_Error_t Error;

    if (p_Statistics == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    }

    Error = CCI_Get(p_Interface, CCI_CMD_SYS_GET_SCENE_STATISTICS, 4, Buffer, p_Status);
    if (Error != LEPTON_ERR_OK) {
        return Error;
    }

    p_Statistics->MinIntensity = Buffer[0];
    p_Statistics->MaxIntensity= Buffer[1];
    p_Statistics->MeanIntensity = Buffer[2];
    p_Statistics->Pixels = Buffer[3];

    return LEPTON_ERR_OK;
}

Lepton_Error_t CCI_SetTLinearResolution(CCI_t *p_Interface, Lepton_TLinear_Resolution_t Resolution,
                                        Lepton_Result_t *p_Status)
{
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_0, Resolution & 0xFFFF));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_1, (Resolution >> 16) & 0xFFFF));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 2));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_RAD_SET_TLINEAR_RESOLUTION));

    return CCI_WaitBusy(p_Interface, p_Status);
}

Lepton_Error_t CCI_GetTLinearResolution(CCI_t *p_Interface, Lepton_TLinear_Resolution_t *p_Resolution,
                                        Lepton_Result_t *p_Status)
{
    uint16_t Low;
    uint16_t High;

    if (p_Resolution == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    }

    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 2));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_RAD_GET_TLINEAR_RESOLUTION));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_0, &Low));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_1, &High));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));

    *p_Resolution = static_cast<Lepton_TLinear_Resolution_t>((High << 16) | Low);

    return LEPTON_ERR_OK;
}

Lepton_Error_t CCI_SetAGCPolicy(CCI_t *p_Interface, Lepton_AGC_Mode_t Policy, Lepton_Result_t *p_Status)
{
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_0, Policy & 0xFFFF));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_1, (Policy >> 16) & 0xFFFF));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 2));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_AGC_SET_POLICY));

    return CCI_WaitBusy(p_Interface, p_Status);
}

Lepton_Error_t CCI_GetAGCPolicy(CCI_t *p_Interface, Lepton_AGC_Mode_t *p_Policy, Lepton_Result_t *p_Status)
{
    uint16_t Low;
    uint16_t High;

    if (p_Policy == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    }

    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 2));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_AGC_GET_POLICY));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_0, &Low));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_1, &High));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));

    *p_Policy = static_cast<Lepton_AGC_Mode_t>((High << 16) | Low);

    return LEPTON_ERR_OK;
}

Lepton_Error_t CCI_SetVideoFreeze(CCI_t *p_Interface, bool Freeze, Lepton_Result_t *p_Status)
{
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_0, Freeze & 0xFFFF));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_1, (Freeze >> 16) & 0xFFFF));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 2));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_VID_SET_VIDEO_FREEZE));

    return CCI_WaitBusy(p_Interface, p_Status);
}

Lepton_Error_t CCI_GetVideoFreeze(CCI_t *p_Interface, bool *p_Freeze, Lepton_Result_t *p_Status)
{
    uint16_t Low;
    uint16_t High;

    if (p_Freeze == NULL) {
        return LEPTON_ERR_INVALID_ARG;
    }

    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_DATA_LENGTH, 2));
    LEPTON_ERROR_CHECK(CCI_WriteRegister(p_Interface, CCI_REG_COMMAND, CCI_CMD_VID_GET_VIDEO_FREEZE));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_0, &Low));
    LEPTON_ERROR_CHECK(CCI_ReadRegister(p_Interface, CCI_REG_DATA_1, &High));
    LEPTON_ERROR_CHECK(CCI_WaitBusy(p_Interface, p_Status));

    *p_Freeze = static_cast<bool>((High << 16) | Low);

    return LEPTON_ERR_OK;
}