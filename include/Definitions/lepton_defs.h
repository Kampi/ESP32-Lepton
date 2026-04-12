/*
 * lepton_defs.h
 *
 *  Copyright (C) Daniel Kampert, 2026
 *  Website: www.kampis-elektroecke.de
 *  File info: Type definitions for Lepton driver.
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

#ifndef ESP32_LEPTON_TYPES_H_
#define ESP32_LEPTON_TYPES_H_

#include <driver/gpio.h>
#include <driver/i2c_master.h>
#include <driver/spi_master.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <freertos/queue.h>

#include <string>
#include <stdint.h>
#include <stdbool.h>

/** @brief          Set a GPIO to the given level.
 *  @param Level    Output state
 */
typedef void (*GPIO_Set)(bool Level);

/** @brief              I2C initialization function prototype.
 *  @param p_Config     Pointer to I2C configuration
 *  @param p_Bus_handle Pointer to I2C bus handle
 *  @return             0 when successful
 *                      -1 when not successful
 */
typedef int32_t (*I2C_Init_t)(i2c_master_bus_config_t *p_Config, i2c_master_bus_handle_t *p_Bus_Handle);

/** @brief              I2C write function prototype.
 *  @param p_Dev_Handle Pointer to I2C device handle
 *  @param p_Buffer     Pointer to data buffer
 *  @param Length       Buffer length
 *  @return             0 when successful
 *                      -1 when not successful
 */
typedef int32_t (*I2C_Write_t)(i2c_master_dev_handle_t *p_Dev_Handle, const uint8_t *p_Buffer, uint32_t Length);

/** @brief              I2C read function prototype.
 *  @param p_Dev_Handle Pointer to I2C device handle
 *  @param p_Buffer     Pointer to data buffer
 *  @param Length       Buffer length
 *  @return             0 when successful
 *                      -1 when not successful
 */
typedef int32_t (*I2C_Read_t)(i2c_master_dev_handle_t *p_Dev_Handle, uint8_t *p_Buffer, uint32_t Length);

/** @brief              I2C combined write-then-read function prototype (atomic, uses repeated-START).
 *                      Preferred over separate I2C_Write + I2C_Read for register reads because it
 *                      prevents other I2C devices from injecting a START condition between the
 *                      register-address phase and the data phase.
 *  @param p_Dev_Handle Pointer to I2C device handle
 *  @param p_WriteData  Pointer to write data (register address bytes)
 *  @param WriteLength  Number of bytes to write
 *  @param p_ReadData   Pointer to read buffer
 *  @param ReadLength   Number of bytes to read
 *  @return             0 when successful
 *                      -1 when not successful
 */
typedef int32_t (*I2C_WriteRead_t)(i2c_master_dev_handle_t *p_Dev_Handle,
                                   const uint8_t *p_WriteData, uint32_t WriteLength,
                                   uint8_t *p_ReadData, uint32_t ReadLength);

/** @brief              I2C deinitialization function prototype.
 *  @param Bus_handle   I2C bus handle
 *  @return             0 when successful
 *                      -1 when not successful
 */
typedef int32_t (*I2C_Deinit_t)(i2c_master_bus_handle_t Bus_handle);

/** @brief Material emissivity values scaled by 100.
 */
typedef enum {
    LEPTON_EMISSIVITY_ASPHALT = 95,                 /**< Emissivity value for asphalt (0.90-0.98, avg 0.95) */
    LEPTON_EMISSIVITY_CONCRETE = 92,                /**< Emissivity value for concrete (0.92) */
    LEPTON_EMISSIVITY_SOIL_DRY = 90,                /**< Emissivity value for dry soil (0.90) */
    LEPTON_EMISSIVITY_SOIL_WET = 95,                /**< Emissivity value for wet soil (0.95) */
    LEPTON_EMISSIVITY_WOOD = 90,                    /**< Emissivity value for wood (0.90) */
    LEPTON_EMISSIVITY_WATER = 94,                   /**< Emissivity value for water (0.92-0.96, avg 0.94) */
    LEPTON_EMISSIVITY_ICE = 97,                     /**< Emissivity value for ice (0.96-0.98, avg 0.97) */
    LEPTON_EMISSIVITY_SNOW = 83,                    /**< Emissivity value for snow (0.83) */
    LEPTON_EMISSIVITY_BRICK = 95,                   /**< Emissivity value for brick (0.93-0.96, avg 0.95) */
    LEPTON_EMISSIVITY_PAINT = 90,                   /**< Emissivity value for lacquer/paint (0.80-0.95, avg 0.90) */
    LEPTON_EMISSIVITY_PAINT_BLACK = 97,             /**< Emissivity value for flat black lacquer (0.97) */
    LEPTON_EMISSIVITY_TEXTILES = 90,                /**< Emissivity value for textiles (0.90) */
    LEPTON_EMISSIVITY_SKIN_HUMAN = 98,              /**< Emissivity value for human skin (0.98) */
    LEPTON_EMISSIVITY_ALUMINUM_POLISHED = 5,        /**< Emissivity value for polished aluminum (0.04-0.06, avg 0.05) */
    LEPTON_EMISSIVITY_ALUMINUM_ANODIZED = 55,       /**< Emissivity value for anodized aluminum (0.55) */
    LEPTON_EMISSIVITY_STEEL_RUSTY = 69,             /**< Emissivity value for rusty steel (0.69) */
    LEPTON_EMISSIVITY_STEEL_STAINLESS = 30,         /**< Emissivity value for stainless steel (0.16-0.45, avg 0.30) */
} Lepton_Emissivity_t;

/** @brief Lepton error codes from the software driver (Chapter 2.3 - FLIR LEPTON Software IDD).
 */
typedef enum {
    LEPTON_ERR_OK                       = 0,    /**< Camera ok. */
    LEPTON_ERROR                        = -1,   /**< Camera general error. */
    LEPTON_NOT_READY                    = -2,   /**< Camera not ready error. */
    LEPTON_RANGE_ERROR                  = -3,   /**< Camera range error. */
    LEPTON_CHECKSUM_ERROR               = -4,   /**< Camera checksum error. */
    LEPTON_BAD_ARG_POINTER_ERROR        = -5,   /**< Camera Bad argument error. */
    LEPTON_DATA_SIZE_ERROR              = -6,   /**< Camera byte count error. */
    LEPTON_UNDEFINED_FUNCTION_ERROR     = -7,   /**< Camera undefined function error. */
    LEPTON_FUNCTION_NOT_SUPPORTED       = -8,   /**< Camera function not yet supported error. */
    LEPTON_DATA_OUT_OF_RANGE_ERROR      = -9,   /**< Camera input DATA is out of valid range error. */
    LEPTON_COMMAND_NOT_ALLOWED          = -11,  /**< Camera unable to execute command due to current camera state. */

    /* OTP access errors */
    LEPTON_OTP_WRITE_ERROR              = -15,  /**< Camera OTP write error */
    LEPTON_OTP_READ_ERROR               = -16,  /**< double bit error detected (uncorrectible) */
    LEPTON_OTP_NOT_PROGRAMMED_ERROR     = -18,  /**< Flag read as non-zero */

    /* I2C Errors */
    LEPTON_ERROR_I2C_BUS_NOT_READY      = -20,  /**< I2C Bus Error - Bus Not Avaialble. */
    LEPTON_ERROR_I2C_BUFFER_OVERFLOW    = -22,  /**< I2C Bus Error - Buffer Overflow. */
    LEPTON_ERROR_I2C_ARBITRATION_LOST   = -23,  /**< I2C Bus Error - Bus Arbitration Lost. */
    LEPTON_ERROR_I2C_BUS_ERROR          = -24,  /**< I2C Bus Error - General Bus Error. */
    LEPTON_ERROR_I2C_NACK_RECEIVED      = -25,  /**< I2C Bus Error - NACK Received. */
    LEPTON_ERROR_I2C_FAIL               = -26,  /**< I2C Bus Error - General Failure. */

    /* Processing Errors */
    LEPTON_DIV_ZERO_ERROR               = -80,  /**< Attempted div by zero. */

    /* Comm Errors */
    LEPTON_COMM_PORT_NOT_OPEN           = -101, /**< Comm port not open. */
    LEPTON_COMM_INVALID_PORT_ERROR      = -102, /**< Comm port no such port error. */
    LEPTON_COMM_RANGE_ERROR             = -103, /**< Comm port range error. */
    LEPTON_ERROR_CREATING_COMM          = -104, /**< Error creating comm. */
    LEPTON_ERROR_STARTING_COMM          = -105, /**< Error starting comm. */
    LEPTON_ERROR_CLOSING_COMM           = -106, /**< Error closing comm. */
    LEPTON_COMM_CHECKSUM_ERROR          = -107, /**< Comm checksum error. */
    LEPTON_COMM_NO_DEV                  = -108, /**< No comm device. */
    LEPTON_ERR_TIMEOUT_ERROR            = -109, /**< Comm timeout error. */
    LEPTON_COMM_ERROR_WRITING_COMM      = -110, /**< Error writing comm. */
    LEPTON_COMM_ERROR_READING_COMM      = -111, /**< Error reading comm. */
    LEPTON_COMM_COUNT_ERROR             = -112, /**< Comm byte count error. */

    /* Other Errors */
    LEPTON_OPERATION_CANCELED           = -126, /**< Camera operation canceled. */
    LEPTON_UNDEFINED_ERROR_CODE         = -127  /**< Undefined error. */
} Lepton_Result_t;

/** @brief TLinear resolution definitions (Chapter 4.8.10 - FLIR LEPTON Software IDD).
 */
typedef enum {
    LEPTON_TLINEAR_0_1_RESOLUTION = 0,      /**< Scale factor 10. */
    LEPTON_TLINEAR_0_01_RESOLUTION,         /**< Scale factor 100. */
} Lepton_TLinear_Resolution_t;

/** @brief Shutter position definitions (Chapter 4.5.15 - FLIR LEPTON Software IDD).
 */
typedef enum {
    LEPTON_SHUTTER_POS_UNKNOWN = -1,            /**< Unknown shutter position. */
    LEPTON_SHUTTER_POS_IDLE = 0,                /**< Shutter is idle. */
    LEPTON_SHUTTER_POS_OPEN,                    /**< Shutter is open. */
    LEPTON_SHUTTER_POS_CLOSED,                  /**< Shutter is closed. */
    LEPTON_SHUTTER_POS_BRAKE_ON,                /**< Shutter brake is on. */
    LEPTON_SHUTTER_POS_END,                     /**< End of shutter positions. */
} Lepton_ShutterPos_t;

/** @brief GPIO mode definitions (Chapter 4.7.15 - FLIR LEPTON Software IDD).
 */
typedef enum {
    LEPTON_OEM_GPIO_MODE_GPIO,                  /**< General Purpose Input/Output mode. */
    LEPTON_OEM_GPIO_MODE_I2C_MASTER,            /**< I2C Master mode. */
    LEPTON_OEM_GPIO_MODE_SPI_MASTER_VLB_DATA,   /**< SPI Master VLB Data mode. */
    LEPTON_OEM_GPIO_MODE_SPIO_MASTER_REG_DATA,  /**< SPI Master Register Data mode. */
    LEPTON_OEM_GPIO_MODE_SPI_SLAVE_VLB_DATA,    /**< SPI Slave VLB Data mode. */
    LEPTON_OEM_GPIO_MODE_VSYNC,                 /**< V-Sync mode. */
} Lepton_GPIO_t;

/** @brief Telemetry location options definitions (Chapter 4.5.8 - FLIR LEPTON Software IDD).
 */
typedef enum {
    LEPTON_TELEMETRY_LOCATION_HEADER,           /**< Place the telemetry at the image header. */
    LEPTON_TELEMETRY_LOCATION_FOOTER,           /**< Place the telemetry at the image footer. */
} Lepton_TelemetryPos_t;

/** @brief
 */
typedef enum {
    LEPTON_SYS_GAIN_MODE_HIGH,                  /**< High gain mode. */
    LEPTON_SYS_GAIN_MODE_LOW,                   /**< Low gain mode. */
    LEPTON_SYS_GAIN_MODE_AUTO                   /**< Automatic gain mode. */
} Lepton_Gain_t;

/** @brief Video output format definitions (Chapter 4.6.8 - FLIR LEPTON Software IDD).
 */
typedef enum {
    LEPTON_FORMAT_RGB888 = 3,                   /**< 24-bit color mode. */
    LEPTON_FORMAT_RAW14 = 7,                    /**< 14-bit raw data. */
} Lepton_VideoFormat_t;

/** @brief Video output source definitions (Chapter 4.7.8 - FLIR LEPTON Software IDD).
 */
typedef enum {
    LEPTON_SOURCE_RAW = 0,                      /*< Before video processing. */
    LEPTON_SOURCE_COOKED,                       /*< Post video processing - Normal mode. */
    LEPTON_SOURCE_RAMP,                         /*< Software Ramp pattern - Increase in X and Y. */
    LEPTON_SOURCE_CONSTANT,                     /*< Software Constant value pattern. */
    LEPTON_SOURCE_RAMP_H,                       /*< Software Ramp pattern - Increase in X only. */
    LEPTON_SOURCE_RAMP_V,                       /*< Software Ramp pattern - Increase in Y only. */
    LEPTON_SOURCE_RAMP_CUSTOM,                  /*< Software Ramp pattern - Uses custom settings. */
} Lepton_VideoSource_t;

/** @brief AGC Policy definitions (Chapter 4.4.2 - FLIR LEPTON Software IDD).
 */
typedef enum {
    LEPTON_AGC_LINEAR = 0,                      /**< Linear AGC mode. */
    LEPTON_AGC_HEQ,                             /**< Histogram Equalization AGC mode. */
} Lepton_AGC_Mode_t;

/** @brief Radiometry Flux Linear parameter (Chapter 4.8.7 - FLIR LEPTON Software IDD).
 */
typedef struct {
    uint16_t SceneEmissivity;                   /**< */
    uint16_t TBkgK;                             /**< Background temperature (ambient temperature of surrounding objects) in Kelvin (Factor 100). */
    uint16_t TauWindow;                         /**< Transmission value of the window in front of the Lepton (8192 = 100%). */
    uint16_t TWindowK;                          /**< Temperature of the window in front of the Lepton in Kelvin (Factor 100). */
    uint16_t TauAtm;                            /**< */
    uint16_t TAtmK;                             /**< */
    uint16_t ReflWindow;                        /**< */
    uint16_t TReflK;                            /**< */
} Lepton_FluxLinearParams_t;

/** @brief AGC Histogram statistics (Chapter 4.4.4 - FLIR LEPTON Software IDD).
 */
typedef struct {
    uint32_t Intensity_Min;                     /**< Minimum intensity value. */
    uint32_t Intensity_Max;                     /**< Maximum intensity value. */
    uint32_t Intensity_Avg;                     /**< Average intensity value. */
    uint32_t Pixels;                            /**< Total number of pixels counted in the histogram (19200). */
} Lepton_AGC_Histogram_Statistics_t;

/** @brief Buffer definition for the Lepton.
 */
typedef struct {
    uint16_t Height;                            /**< Image buffer height in pixel. */
    uint16_t Width;                             /**< Image buffer width in pixel. */
    uint8_t BytesPerPixel;                      /**< Number of bytes per pixel. */
    uint16_t *Image_Buffer;                     /**< Pointer to memory location for image buffer.
                                                     Allocated with MALLOC_CAP_SIMD and MALLOC_CAP_SPIRAM, if PSRAM is enabled. */
    uint16_t *Telemetry_Buffer;                 /**< Pointer to memory location for telemetry buffer. */
} Lepton_FrameBuffer_t;

/** @brief Spotmeter object definition (Chapter 4.8.13 - FLIR LEPTON Software IDD).
 */
typedef struct {
    float Value;                                /**< Spotmeter temperature value in Kelvin. */
    float Max;                                  /**< Maximum temperature value in Kelvin within the spotmeter ROI. */
    float Min;                                  /**< Minimum temperature value in Kelvin within the spotmeter ROI. */
    uint16_t Population;                        /**< Number of pixels within the spotmeter ROI. */
} Lepton_Spotmeter_t;

/** @brief Image scene statistics object definition (Chapter 4.5.12 - FLIR LEPTON Software IDD).
 */
typedef struct {
    float MeanIntensity;                        /**< Mean intensity of the image scene. */
    float MaxIntensity;                         /**< Maximum intensity of the image scene. */
    float MinIntensity;                         /**< Minimum intensity of the image scene. */
    uint16_t Pixels;                            /**< Number of pixels in the image scene. */
} Lepton_SceneStatistics_t;

/** @brief ROI object definition.
 */
typedef struct {
    uint16_t Start_Col;                         /**< Start column of the ROI. */
    uint16_t Start_Row;                         /**< Start row of the ROI. */
    uint16_t End_Col;                           /**< End column of the ROI. */
    uint16_t End_Row;                           /**< End row of the ROI. */
} Lepton_ROI_t;

/** @brief Pixel definition for custom look-up table (Chapter 4.6.2 - FLIR LEPTON Software IDD).
 */
typedef struct {
    uint8_t Reserved;                           /**< Reserved. */
    uint8_t R;                                  /**< Red pixel. */
    uint8_t G;                                  /**< Green pixel. */
    uint8_t B;                                  /**< Blue pixel. */
} Lepton_LookUp_Pixel_t;

/** @brief Custom look-up table definition (Chapter 4.6.2 - FLIR LEPTON Software IDD).
 */
typedef struct {
    Lepton_LookUp_Pixel_t Binary[256];          /**< Look-Up table data. */
} Lepton_LookUp_t;

/** @brief Lepton Thermal Camera software revision object definition (Chapter 4.7.5 - FLIR LEPTON Software IDD).
 */
typedef struct {
    uint8_t gpp_major;                          /**< */
    uint8_t gpp_minor;                          /**< */
    uint8_t gpp_build;                          /**< */
    uint8_t dsp_major;                          /**< */
    uint8_t dsp_minor;                          /**< */
    uint8_t dsp_build;                          /**< */
    uint16_t Reserved;                          /**< Reserved. */
} Lepton_Version_t;

/** @brief Lepton CCI communication object.
 */
typedef struct {
    I2C_Init_t I2C_Init;                        /**< I2C initialization function pointer. */
    /**< NOTE: You must set I2C_Bus_Config to use this function! */
    I2C_Write_t I2C_Write;                      /**< I2C write function pointer. */
    I2C_Read_t I2C_Read;                        /**< I2C read function pointer. */
    I2C_WriteRead_t I2C_WriteRead;              /**< I2C atomic write-then-read function pointer (optional, preferred for register reads). */
    I2C_Deinit_t I2C_Deinit;                    /**< I2C deinitialization function pointer. */
    i2c_master_bus_config_t *I2C_Bus_Config;    /**< Pointer to I2C bus configuration. */
    /**< NOTE: Only needed when I2C_Init is used! */
    i2c_master_bus_handle_t I2C_Bus_Handle;     /**< Pointer to I2C bus handle. */
    i2c_master_dev_handle_t I2C_Dev_Handle;     /**< Pointer to I2C device handle. */
    SemaphoreHandle_t Mutex;                    /**< Mutex for the I2C communication.
                                                     NOTE: Managed by the device driver. */
    bool isInitialized;                         /**< true when the device is initialized.
                                                     NOTE: Managed by the device driver. */
} CCI_t;

/** @brief Lepton VoSPI communication object.
 */
typedef struct {
    spi_device_interface_config_t Interface;    /**< SPI configuration. */
    spi_bus_config_t Master;                    /**< SPI master configuration. */
    spi_host_device_t Host;                     /**< SPI host interface used by the driver. */
    spi_device_handle_t Handle;                 /**< SPI device handle for communication.
                                                     NOTE: Managed by the device driver. */
    int DMA;                                    /**< DMA channel used by the VoSPI driver. */
    bool isInitialized;                         /**< true when the device is initialized.
                                                     NOTE: Managed by the device driver. */
    bool isResync;                              /**< true when a resynchronization is in progress.
                                                     NOTE: Managed by the device driver. */
    bool isCapturing;                           /**< true when a frame capture is in progress.
                                                     NOTE: Managed by the device driver. */
    bool useTelemetry;                          /**< true when telemetry data is to be captured.
                                                     NOTE: Managed by the device driver. */
    Lepton_TelemetryPos_t TelemetryPosition;    /**< Telemetry position setting.
                                                     NOTE: Managed by the device driver. */
    uint16_t *Packet;                           /**< Pointer to allocated array to store one Lepton packet (DMA capable).
                                                     NOTE: Managed by the device driver. */
    uint16_t ImageHeight;                       /**< Image buffer height in pixel. */
    uint16_t ImageWidth;                        /**< Image buffer width in pixel. */
    uint8_t BytesPerPixel;                      /**< Number of bytes per pixel. */
    uint8_t PacketsPerFrame;                    /**< Number of packets per frame. */
    uint16_t *Image_Buffer[CONFIG_LEPTON_VOSPI_FRAME_BUFFERS];                  /**< Pointer to memory location for image buffer. */
    uint16_t *Telemetry_Buffer[CONFIG_LEPTON_VOSPI_FRAME_BUFFERS];              /**< Pointer to memory location for telemetry buffer. */
    uint8_t CurrentBuffer;                      /**< Current buffer index used by the driver.
                                                     NOTE: Managed by the device driver. */
    uint32_t SyncErrors;                        /**< Number of synchronization errors encountered. */
    uint32_t FrameCounter;                      /**< Number of frames captured. */
    int64_t ResyncStartUs;                      /**< Timestamp when the resynchronization started.
                                                     NOTE: Managed by the device driver. */
} VoSPI_t;

/** @brief Lepton Thermal Camera device object definition.
 */
typedef struct {
    char PartNumber[33];                        /**< Device part number.
                                                     NOTE: Managed by the device driver. */
    uint8_t SerialNumber[8];                    /**< Device serial number.
                                                     NOTE: Managed by the device driver. */
    Lepton_Version_t SoftwareVersion;           /**< Device software version.
                                                     NOTE: Managed by the device driver. */
    struct {
        GPIO_Set Reset;                         /**< Function pointer to control the reset line.
                                                     NOTE: Can be set to NULL to leave it unused. */
        GPIO_Set PowerDown;                     /**< Function pointer to control the power-down line.
                                                     NOTE: Can be set to NULL to leave it unused. */
        gpio_num_t VSync;                       /**< Pin uses for the VSync signal. */
        TaskHandle_t CapHandle;                 /**< Handle for the capture task.
                                                     NOTE: Managed by the device driver. */
        QueueHandle_t FrameQueue;               /**< Queue for frame ready events.
                                                     NOTE: Managed by the device driver. */
        bool isInitialized;                     /**< true when the device is initialized.
                                                     NOTE: Managed by the device driver. */
        bool isRadiometric;                     /**< Read-only flag indicating if the device is a radiometric model.
                                                     NOTE: Managed by the device driver. */
        bool useAGC;                            /**< true when Automatic Gain Control (AGC) is enabled.
                                                     NOTE: Managed by the device driver. */
        bool useAGCCalc;                        /**< true when AGC calculation is enabled.
                                                     NOTE: Managed by the device driver. */
        bool useTLinear;                         /**< true when TLinear mode is enabled.
                                                     NOTE: Managed by the device driver. */
        bool isAutoRes;                         /**< true when AutoRes mode is enabled.
                                                     NOTE: Managed by the device driver. */
        bool isTLinearAutoRes;                  /**< true when TLinear AutoRes mode is enabled.
                                                     NOTE: Managed by the device driver. */
        bool isVideoFreezeEnabled;              /**< true when Video Freeze is enabled.
                                                     NOTE: Managed by the device driver. */
        Lepton_Gain_t Gain;                     /**< Gain setting for the Lepton sensor.
                                                     NOTE: Managed by the device driver. */
        Lepton_VideoFormat_t VideoFormat;       /**< Video format setting for the Lepton sensor.
                                                     NOTE: Managed by the device driver. */
        Lepton_AGC_Mode_t AGCPolicy;            /**< AGC policy setting for the Lepton sensor.
                                                     NOTE: Managed by the device driver. */
        CCI_t CCI;                              /**< CCI device object used by the camera driver.
                                                     NOTE: Managed by the device driver. */
        VoSPI_t VoSPI;                          /**< VoSPI device object used by the camera driver.
                                                     NOTE: Managed by the device driver. */
        uint16_t MinSmooth;                     /**< Minimum smooth value for RGB conversion.
                                                     NOTE: Managed by the device driver. */
        uint16_t MaxSmooth;                     /**< Maximum smooth value for RGB conversion.
                                                     NOTE: Managed by the device driver. */
        const float SmoothFactor = 0.95f;       /**< Smoothing factor for min/max smoothing.
                                                     NOTE: Managed by the device driver. */
    } Internal;
} Lepton_t;

/** @brief
 */
typedef struct {
    bool useAGC;                                /**< Set to true to enable Automatic Gain Control (AGC). */
    bool useAGCCalculation;                     /**< Set to true to enable AGC calculation. */
    bool useTLinear;                            /**< Set to true to enable TLinear mode. */
    Lepton_Gain_t Gain;                         /**< Gain setting for the Lepton sensor. */
    Lepton_VideoFormat_t VideoFormat;           /**< Video format setting for the Lepton sensor. */
    Lepton_AGC_Mode_t AGCPolicy;                /**< AGC policy setting for the Lepton sensor. */
    gpio_num_t VSync;                           /**< Pin used for the VSync signal. */
    GPIO_Set Reset;                             /**< Function pointer to control the reset line.
                                                     NOTE: Can be set to NULL to leave it unused. */
    GPIO_Set PowerDown;                         /**< Function pointer to control the power-down line.
                                                     NOTE: Can be set to NULL to leave it unused. */
    CCI_t CCI;                                  /**< CCI initialization object used by the camera driver. */
    VoSPI_t VoSPI;                              /**< VoSPI initialization object used by the camera driver. */
} Lepton_Conf_t;

/** @brief Lepton Telemetry Data (Table 3 - Engineering Datasheet Rev. 200).
 */
typedef struct {
    uint16_t Reserved: 3;                       /**< Reserved. */
    uint16_t FFC_Desired: 1;                    /**< 0 = FFC not desired, 1 = FFC desired. */
    uint16_t FFC_State:2;                       /**< 00 = FFC never commanded, 01 = FFC imminent, 10 = FFC in progress, 11 = FFC complete. */
    uint16_t Reserved1: 6;                      /**< Reserved. */
    uint16_t AGC_State: 1;                      /**< 0 = AGC disabled, 1 = AGC enabled. */
    uint16_t Reserved2: 2;                      /**< Reserved. */
    uint16_t Shutter_Lockout:1;                 /**< 0 = Shutter not locked out, 1 = Shutter locked out (outside of valid temperature range, -10°C to 80°C)). */
    uint16_t Reserved3: 4;                      /**< Reserved. */
    uint16_t Overtemp_Shutdown: 1;              /**< Goes true 10 seconds before shutdown. */
    uint16_t Reserved4: 11;                     /**< Reserved. */
} __attribute__((packed)) Lepton_TelemetryStatus_t;

/** @brief Lepton Telemetry Data (Table 2 - Engineering Datasheet Rev. 200).
 */
typedef struct {
    /* -------- Row A -------- */
    uint16_t Revision;                          /**< Telemetry revision (Major = Byte 1, Minor = Byte 0). */
    uint32_t TimeCounter;                       /**< Time counter in milliseconds. */
    Lepton_TelemetryStatus_t Status;            /**< Status bits. */
    uint16_t Serial[8];                         /**< Module serial number. */
    uint16_t Software[4];                       /**< Software revision. */
    uint16_t Reserved[3];                       /**< Reserved. */
    uint32_t FrameCounter;                      /**< Frame counter. */
    uint16_t FrameMean;                         /**< Frame mean value. */
    uint16_t FPA_Count;                         /**< FPA temperature in counts. */
    uint16_t FPA_Temp;                          /**< FPA temperature in Kelvin x 100. */
    uint16_t Housing_Count;                     /**< Housing temperature in counts. */
    uint16_t Housing_Temp;                      /**< Housing temperature in Kelvin x 100. */
    uint16_t Reserved1[2];                      /**< Reserved. */
    uint16_t FFC_FPA;                           /**< FPA temperature at last FFC (Kelvin x 100). */
    uint32_t FFC_Time;                          /**< Time counter at last FFC. */
    uint16_t Housing_Temp_FFC;                  /**< Housing temperature at last FFC (Kelvin x 100). */
    uint16_t Reserved2;                         /**< Reserved. */
    uint16_t AGC_ROI[4];                        /**< AGC ROI (top, left, bottom, right). */
    uint16_t AGC_Clip_High;                     /**< AGC clip high value. */
    uint16_t AGC_Clip_Low;                      /**< AGC clip low value. */
    uint16_t Reserved3[32];                     /**< Reserved. */
    uint32_t VideoFormat;                       /**< Video output format. */
    uint16_t FFC_Log2;                          /**< Log2 of number of FFC frames. */
    uint16_t Reserved4[5];                      /**< Reserved. */

    /* -------- Row B -------- */
    uint16_t Reserved5[19];                     /**< Reserved. */
    uint16_t Emissivity;                        /**< Emissivity (scaled by 8192). */
    uint16_t Temp_Background;                   /**< Background temperature (Kelvin x 100). */
    uint16_t Temp_Atmospheric_Scaled;           /**< Atmospheric transmission (scaled by 8192). */
    uint16_t Temp_Atmospheric;                  /**< Atmospheric temperature (Kelvin x 100). */
    uint16_t Window_Transmission;               /**< Window transmission (scaled by 8192). */
    uint16_t Window_Reflection;                 /**< Window reflection (scaled by 8192). */
    uint16_t Window_Temperature;                /**< Window temperature (Kelvin x 100). */
    uint16_t Window_Reflected_Temperature;      /**< Window reflected temperature (Kelvin x 100). */
    uint16_t Reserved6[53];                     /**< Reserved. */

    /* -------- Row C -------- */
    uint16_t Reserved7[48];                     /**< Reserved. */
    uint16_t Gain_Mode;                         /**< Gain mode (0 = High, 1 = Low, 2 = Auto). */
    uint16_t Gain_Mode_Eff;                     /**< Effective gain mode (0 = High, 1 = Low). */
    uint16_t Gain_Mode_Desired;                 /**< Desired gain mode. */
    uint16_t Gain_Temp_High_To_Low;             /**< Temperature threshold High -> Low (°C). */
    uint16_t Gain_Temp_Low_To_High;             /**< Temperature threshold Low -> High (°C). */
    uint16_t Gain_Temp_High_To_Low_K;           /**< Temperature threshold High -> Low (Kelvin). */
    uint16_t Gain_Temp_Low_To_High_K;           /**< Temperature threshold Low -> High (Kelvin). */
    uint16_t Reserved8[2];                      /**< Reserved. */
    uint16_t Gain_Pop_High_To_Low;              /**< Population threshold High -> Low. */
    uint16_t Gain_Pop_Low_To_High;              /**< Population threshold Low -> High. */
    uint16_t Reserved9[6];                      /**< Reserved. */
    uint16_t Gain_ROI[4];                       /**< Gain ROI (startRow, startCol, endRow, endCol). */
    uint16_t Reserved10[22];                    /**< Reserved. */
    uint16_t TLinear_Enable;                    /**< TLinear enable flag. */
    uint16_t TLinear_Resolution;                /**< TLinear resolution (0=0.1K, 1=0.01K). */
    uint16_t Spot_Mean;                         /**< Spotmeter mean temperature. */
    uint16_t Spot_Max;                          /**< Spotmeter max temperature. */
    uint16_t Spot_Min;                          /**< Spotmeter min temperature. */
    uint16_t Spot_Population;                   /**< Spotmeter population. */
    uint16_t Spot_ROI_Start_Row;                /**< Spotmeter ROI start row. */
    uint16_t Spot_ROI_Start_Col;                /**< Spotmeter ROI start column. */
    uint16_t Spot_ROI_End_Row;                  /**< Spotmeter ROI end row. */
    uint16_t Spot_ROI_End_Col;                  /**< Spotmeter ROI end column. */
    uint16_t Reserved11[22];                    /**< Reserved. */
} __attribute__((packed)) Lepton_Telemetry_t;

#endif /* ESP32_LEPTON_TYPES_H_ */