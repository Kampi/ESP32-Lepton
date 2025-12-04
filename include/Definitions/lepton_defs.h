 /*
 * lepton_defs.h
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

#ifndef LEPTON_TYPES_H_
#define LEPTON_TYPES_H_

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

typedef uint8_t Lepton_BufferIndex_t;

/** @brief 			Set a GPIO to the given level.
 *  @param Level    Output state
 */
typedef void (*GPIO_Set)(bool Level);

/** @brief 				I2C initialization function prototype.
 *  @param p_Config	    Pointer to I2C configuration
 *  @param p_Bus_handle Pointer to I2C bus handle
 *  @return				0 when successful
 * 						-1 when not successful
 */
typedef int32_t (*I2C_Init_t)(i2c_master_bus_config_t* p_Config, i2c_master_bus_handle_t* p_Bus_Handle);

/** @brief 				I2C write function prototype.
 *  @param p_Dev_Handle Pointer to I2C device handle
 *  @param p_Buffer		Pointer to data buffer
 *  @param Length		Buffer length
 *  @return				0 when successful
 * 						-1 when not successful
 */
typedef int32_t (*I2C_Write_t)(i2c_master_dev_handle_t* p_Dev_Handle, const uint8_t* p_Buffer, uint32_t Length);

/** @brief 				I2C read function prototype.
 *  @param p_Dev_Handle Pointer to I2C device handle
 *  @param p_Buffer		Pointer to data buffer
 *  @param Length		Buffer length
 *  @return				0 when successful
 * 						-1 when not successful
 */
typedef int32_t (*I2C_Read_t)(i2c_master_dev_handle_t* p_Dev_Handle, uint8_t* p_Buffer, uint32_t Length);

/** @brief 				I2C deinitialization function prototype.
 *  @param Bus_handle   I2C bus handle
 *  @return				0 when successful
 * 						-1 when not successful
 */
typedef int32_t (*I2C_Deinit_t)(i2c_master_bus_handle_t Bus_handle);

/** @brief Lepton error codes from the software driver.
 */
typedef enum
{
    LEPTON_ERR_OK 						= 0,	/**< Camera ok. */
    LEPTON_ERROR 						= -1,	/**< Camera general error. */
    LEPTON_NOT_READY 					= -2,	/**< Camera not ready error. */
    LEPTON_RANGE_ERROR 					= -3,	/**< Camera range error. */
    LEPTON_CHECKSUM_ERROR 				= -4,	/**< Camera checksum error. */
    LEPTON_BAD_ARG_POINTER_ERROR 		= -5,	/**< Camera Bad argument error. */
    LEPTON_DATA_SIZE_ERROR 				= -6,	/**< Camera byte count error. */
    LEPTON_UNDEFINED_FUNCTION_ERROR 	= -7,	/**< Camera undefined function error. */
    LEPTON_FUNCTION_NOT_SUPPORTED 		= -8,	/**< Camera function not yet supported error. */
    LEPTON_DATA_OUT_OF_RANGE_ERROR 		= -9,	/**< Camera input DATA is out of valid range error. */
    LEPTON_COMMAND_NOT_ALLOWED 			= -11,	/**< Camera unable to execute command due to current camera state. */

    /* OTP access errors */
    LEPTON_OTP_WRITE_ERROR 				= -15,	/**< Camera OTP write error */
    LEPTON_OTP_READ_ERROR 				= -16,	/**< double bit error detected (uncorrectible) */
    LEPTON_OTP_NOT_PROGRAMMED_ERROR 	= -18,	/**< Flag read as non-zero */

    /* I2C Errors */
    LEPTON_ERROR_I2C_BUS_NOT_READY 		= -20,	/**< I2C Bus Error - Bus Not Avaialble. */
    LEPTON_ERROR_I2C_BUFFER_OVERFLOW 	= -22, 	/**< I2C Bus Error - Buffer Overflow. */
    LEPTON_ERROR_I2C_ARBITRATION_LOST 	= -23, 	/**< I2C Bus Error - Bus Arbitration Lost. */
    LEPTON_ERROR_I2C_BUS_ERROR 			= -24,	/**< I2C Bus Error - General Bus Error. */
    LEPTON_ERROR_I2C_NACK_RECEIVED 		= -25,	/**< I2C Bus Error - NACK Received. */
    LEPTON_ERROR_I2C_FAIL 				= -26,	/**< I2C Bus Error - General Failure. */

    /* Processing Errors */
    LEPTON_DIV_ZERO_ERROR 				= -80,	/**< Attempted div by zero. */

    /* Comm Errors */
    LEPTON_COMM_PORT_NOT_OPEN 			= -101,	/**< Comm port not open. */
    LEPTON_COMM_INVALID_PORT_ERROR 		= -102,	/**< Comm port no such port error. */
    LEPTON_COMM_RANGE_ERROR 			= -103,	/**< Comm port range error. */
    LEPTON_ERROR_CREATING_COMM 			= -104,	/**< Error creating comm. */
    LEPTON_ERROR_STARTING_COMM 			= -105,	/**< Error starting comm. */
    LEPTON_ERROR_CLOSING_COMM 			= -106,	/**< Error closing comm. */
    LEPTON_COMM_CHECKSUM_ERROR 			= -107,	/**< Comm checksum error. */
    LEPTON_COMM_NO_DEV 					= -108,	/**< No comm device. */
    LEPTON_ERR_TIMEOUT_ERROR 			= -109,	/**< Comm timeout error. */
    LEPTON_COMM_ERROR_WRITING_COMM 		= -110,	/**< Error writing comm. */
    LEPTON_COMM_ERROR_READING_COMM 		= -111,	/**< Error reading comm. */
    LEPTON_COMM_COUNT_ERROR 			= -112,	/**< Comm byte count error. */

    /* Other Errors */
    LEPTON_OPERATION_CANCELED 			= -126,	/**< Camera operation canceled. */
    LEPTON_UNDEFINED_ERROR_CODE 		= -127	/**< Undefined error. */
} Lepton_Result_t;

/** @brief 
 */
typedef enum
{
    LEPTON_OEM_GPIO_MODE_GPIO,					/**< */
    LEPTON_OEM_GPIO_MODE_I2C_MASTER,           	/**< */
    LEPTON_OEM_GPIO_MODE_SPI_MASTER_VLB_DATA,	/**< */
    LEPTON_OEM_GPIO_MODE_SPIO_MASTER_REG_DATA, 	/**< */
    LEPTON_OEM_GPIO_MODE_SPI_SLAVE_VLB_DATA,   	/**< */
    LEPTON_OEM_GPIO_MODE_VSYNC,                	/**< */
} Lepton_GPIO_t;

/** @brief Telemetry location options definition.
 */
typedef enum
{
    LEPTON_TELEMETRY_LOCATION_HEADER,			/**< Place the telemetry at the image header. */
    LEPTON_TELEMETRY_LOCATION_FOOTER,			/**< Place the telemetry at the image footer. */
} Lepton_TelemetryPos_t;

/** @brief 
 */
typedef enum
{
    LEPTON_SYS_GAIN_MODE_HIGH,					/**< */
    LEPTON_SYS_GAIN_MODE_LOW,					/**< */
    LEPTON_SYS_GAIN_MODE_AUTO					/**< */
} Lepton_Gain_t;

/** @brief 
 */
typedef enum
{
    LEPTON_FORMAT_RGB888 	            = 3,    /**< 24-bit color mode. */
    LEPTON_FORMAT_RAW14 	            = 7,	/**< 14-bit raw data. */
} Lepton_VideoFormat_t;

/** @brief Video output source definitions.
 */
typedef enum
{
    LEPTON_SOURCE_RAW 		            = 0, 	/*< Before video processing. */
    LEPTON_SOURCE_COOKED, 						/*< Post video processing - Normal mode. */
    LEPTON_SOURCE_RAMP, 						/*< Software Ramp pattern - Increase in X and Y. */
    LEPTON_SOURCE_CONSTANT,						/*< Software Constant value pattern. */
    LEPTON_SOURCE_RAMP_H,						/*< Software Ramp pattern - Increase in X only. */
    LEPTON_SOURCE_RAMP_V,						/*< Software Ramp pattern - Increase in Y only. */
    LEPTON_SOURCE_RAMP_CUSTOM,					/*< Software Ramp pattern - Uses custom settings. */
} Lepton_VideoSource_t;

/** @brief Radiometry Flux Linear parameter.
 */
typedef struct
{
    uint16_t sceneEmissivity;					/**< */
    uint16_t TBkgK;								/**< */
    uint16_t tauWindow;							/**< */
    uint16_t TWindowK;							/**< */
    uint16_t tauAtm;							/**< */
    uint16_t TAtmK;								/**< */
    uint16_t reflWindow;						/**< */
    uint16_t TReflK;							/**< */
} Lepton_FluxLinearParams_t;

/** @brief Buffer definition for the Lepton.
 */
typedef struct
{
    bool Telemetry_Valid;						/**< #true when the telemetry is valid. */
    bool Image_Valid;							/**< */
    uint16_t Heigth;							/**< Image height in pixel. */
    uint16_t Width;								/**< Image width in pixel. */
    uint16_t* Image_Buffer;						/**< Pointer to memory location for image buffer. */
    uint16_t* Telemetry_Buffer;					/**< Pointer to memory location for telemetry buffer. */
} Lepton_Buffer_t;

/** @brief Image scene statistics object definition.
 */
typedef struct
{
    uint16_t MeanIntensity;						/**< */
    uint16_t MaxIntensity;						/**< */
    uint16_t MinIntensity;						/**< */
    uint16_t Pixels;							/**< */
} Lepton_SceneStatistics_t;

/** @brief ROI object definition.
 */
typedef struct
{
    uint16_t StartColumn;						/**< Start column. */
    uint16_t StartRow;							/**< Start row. */
    uint16_t EndColumn;							/**< End column. */
    uint16_t EndRow;							/**< End row. */
} Lepton_ROI_t;

/** @brief Pixel definition for custom look-up table.
 */
typedef struct
{
    uint8_t Reserved;							/**< Reserved. */
    uint8_t R;									/**< Red pixel. */
    uint8_t G;									/**< Green pixel. */
    uint8_t B;									/**< Blue pixel. */
} Lepton_LookUp_Pixel_t;

/** @brief Custom look-up table definition.
 */
typedef struct
{
    Lepton_LookUp_Pixel_t Binary[256];			/**< Look-Up table data. */
} Lepton_LookUp_t;

/** @brief Lepton CCI communication object.
 */
typedef struct
{
    I2C_Init_t I2C_Init;						/**< I2C initialization function pointer. */
                                                /**< NOTE: You must set I2C_Bus_Config to use this function! */
    I2C_Write_t I2C_Write;						/**< I2C write function pointer. */
    I2C_Read_t I2C_Read;						/**< I2C read function pointer. */
    I2C_Deinit_t I2C_Deinit;					/**< I2C deinitialization function pointer. */
    i2c_master_bus_config_t* I2C_Bus_Config;    /**< Pointer to I2C bus configuration. */
                                                /**< NOTE: Only needed when I2C_Init is used! */
    i2c_master_bus_handle_t I2C_Bus_Handle;     /**< Pointer to I2C bus handle. */
    i2c_master_dev_handle_t I2C_Dev_Handle;     /**< Pointer to I2C device handle. */    
    struct
    {
        SemaphoreHandle_t Mutex;				/**< Mutex for the I2C communication.
                                                     NOTE: Managed by the device driver. */
        bool isInitialized;						/**< #true when the device is initialized.
                                                     NOTE: Managed by the device driver. */
    } Internal;
} CCI_t;

/** @brief Lepton VoSPI communication object.
 */
typedef struct
{
    spi_device_interface_config_t Interface;	/**< SPI configuration. */
    spi_bus_config_t Master;					/**< SPI master configuration. */
    spi_host_device_t Host;						/**< SPI host interface used by the driver. */
    int DMA;									/**< DMA channel used by the VoSPI driver. */
    struct
    {
        bool isInitialized;						/**< #true when the device is initialized.
                                                      NOTE: Managed by the device driver. */
        spi_device_handle_t Handle;				/**< SPI device handle for communication.
                                                      NOTE: Managed by the device driver. */
        uint16_t* Packet;					    /**< Pointer to allocated array to store one Lepton packet (DMA capable).
                                                      NOTE: Managed by the device driver. */
        uint16_t* Frame;					    /**< 
                                                     NOTE: Managed by the device driver. */
    } Internal;
    uint32_t SyncErrors;						/**< */
    uint32_t ValidFrames;						/**< */
} VoSPI_t;

/** @brief Lepton Thermal Camera software version object definition.
 */
typedef struct
{
    uint8_t gpp_major;							/**< */
    uint8_t gpp_minor;							/**< */
    uint8_t gpp_build;							/**< */
    uint8_t dsp_major;							/**< */
    uint8_t dsp_minor;							/**< */
    uint8_t dsp_build;							/**< */
    uint16_t Reserved;							/**< Reserved. */
} Lepton_Version_t;

/** @brief Lepton Thermal Camera device object definition.
 */
typedef struct
{
    char PartNumber[33];						/**< Device part number.
                                                     NOTE: Managed by the device driver. */
    uint8_t SerialNumber[8];					/**< Device serial number.
                                                     NOTE: Managed by the device driver. */
    struct
    {
        GPIO_Set Reset;						    /**< Function pointer to control the reset line.
                                                     NOTE: Can be set to NULL to leave it unused. */
        GPIO_Set PowerDown;					    /**< Function pointer to control the power-down line.
                                                     NOTE: Can be set to NULL to leave it unused. */
        gpio_num_t VSync;						/**< Pin uses for the VSync signal. */
        TaskHandle_t CaptureHandle;				/**< Handle for the capture task.
                                                     NOTE: Managed by the device driver. */
        bool isInitialized;						/**< #true when the device is initialized.
                                                     NOTE: Managed by the device driver. */
        bool isRadiometric;						/**< 
                                                     NOTE: Managed by the device driver. */
        bool isAGC;								/**< 
                                                     NOTE: Managed by the device driver. */
        bool isAGCCalc;							/**< 
                                                     NOTE: Managed by the device driver. */
        bool isTLinear;							/**< 
                                                     NOTE: Managed by the device driver. */
        bool useTelemetry;						/**< 
                                                     NOTE: Managed by the device driver. */
        bool isAutoRes;							/**< 
                                                     NOTE: Managed by the device driver. */
        bool isTLinearAutoRes;					/**< 
                                                     NOTE: Managed by the device driver. */
        bool isCapturing;					    /**< 
                                                     NOTE: Managed by the device driver. */
        Lepton_Gain_t Gain;						/**< 
                                                      NOTE: Managed by the device driver. */
        Lepton_VideoFormat_t VideoFormat;		/**< 
                                                      NOTE: Managed by the device driver. */
        CCI_t CCI;								/**< CCI device object used by the camera driver.
                                                      NOTE: Managed by the device driver. */
        VoSPI_t VoSPI;							/**< VoSPI device object used by the camera driver.
                                                      NOTE: Managed by the device driver. */
        QueueHandle_t FrameQueue;               /**< Handle for the frame queue.
                                                     NOTE: Managed by the device driver. */
    } Internal;
} Lepton_t;

/** @brief 
 */
typedef struct
{
    bool UseTelemetry;							/**< */
    bool UseAGC;								/**< */
    bool UseAGCCalculation;						/**< */
    Lepton_Gain_t Gain;							/**< */
    Lepton_VideoFormat_t VideoFormat;			/**< */
    gpio_num_t VSync;							/**< Pin used for the VSync signal. */
    GPIO_Set Reset;						        /**< Function pointer to control the reset line.
                                                     NOTE: Can be set to NULL to leave it unused. */
    GPIO_Set PowerDown;					        /**< Function pointer to control the power-down line.
                                                     NOTE: Can be set to NULL to leave it unused. */
    CCI_t CCI;									/**< CCI initialization object used by the camera driver. */
    VoSPI_t VoSPI;								/**< VoSPI initialization object used by the camera driver. */
} Lepton_Conf_t;

#endif /* LEPTON_TYPES_H_ */