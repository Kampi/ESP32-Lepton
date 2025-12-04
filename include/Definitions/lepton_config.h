 /*
 * lepton_config.h
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

#ifndef LEPTON_CONFIG_H_
#define LEPTON_CONFIG_H_

#include <driver/i2c.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>

#include <sdkconfig.h>

#if defined(CONFIG_LEPTON_VOSPI_SPI2_HOST)
    #define LEPTON_VOSPI_SPI_HOST                       SPI2_HOST
#elif defined(CONFIG_LEPTON_VOSPI_SPI3_HOST)
    #define LEPTON_VOSPI_SPI_HOST                       SPI3_HOST
#else
    #error "No SPI host configured for VoSPI. Define CONFIG_LEPTON_VOSPI_SPI2_HOST or CONFIG_LEPTON_VOSPI_SPI3_HOST in Kconfig."
 #endif

/** @brief              Default configuration object for the FLIR Lepton Thermal Imager.
 *  @param I2C_Handle   I2C Bus handle
 */
#define LEPTON_DEFAULT_CONF(I2C_Handle)                                                {                                                                                \
                                                                                            .useTelemetry = false,                                                      \
                                                                                            .useAGC = true,                                                             \
                                                                                            .useAGCCalculation = true,                                                  \
                                                                                            .useTLinear = true,                                                         \
                                                                                            .useAutoFFC = true,                                                         \
                                                                                            .Gain = static_cast<Lepton_Gain_t>(LEPTON_SYS_GAIN_MODE_AUTO),              \
                                                                                            .VideoFormat = static_cast<Lepton_VideoFormat_t>(LEPTON_FORMAT_RAW14),      \
                                                                                            .VSync = static_cast<gpio_num_t>(CONFIG_LEPTON_GPIO_VSYNC_PIN),             \
                                                                                            .Reset = NULL,                                                              \
                                                                                            .PowerDown = NULL,                                                          \
                                                                                            .CCI = {                                                                    \
                                                                                                .I2C_Init = NULL,                                                       \
                                                                                                .I2C_Write = NULL,                                                      \
                                                                                                .I2C_Read = NULL,                                                       \
                                                                                                .I2C_Deinit = NULL,                                                     \
                                                                                                .I2C_Bus_Config = NULL,                                                 \
                                                                                                .I2C_Bus_Handle = I2C_Handle,                                           \
                                                                                                .I2C_Dev_Handle = NULL,                                                 \
                                                                                                .Mutex = NULL,                                                          \
                                                                                                .isInitialized = false,                                                 \
                                                                                            },                                                                          \
                                                                                            .VoSPI = {                                                                  \
                                                                                                .Interface = {                                                          \
                                                                                                    .command_bits = 0,                                                  \
                                                                                                    .address_bits = 0,                                                  \
                                                                                                    .dummy_bits = 0,                                                    \
                                                                                                    .mode = 3,                                                          \
                                                                                                    .clock_source = SPI_CLK_SRC_DEFAULT,                                \
                                                                                                    .duty_cycle_pos = 0,                                                \
                                                                                                    .cs_ena_pretrans = 10,                                              \
                                                                                                    .cs_ena_posttrans = 0,                                              \
                                                                                                    .clock_speed_hz = 20000000,                                         \
                                                                                                    .input_delay_ns = 0,                                                \
                                                                                                    .sample_point = SPI_SAMPLING_POINT_PHASE_0,                         \
                                                                                                    .spics_io_num = CONFIG_LEPTON_VOSPI_CS,                             \
                                                                                                    .flags = SPI_DEVICE_HALFDUPLEX,                                     \
                                                                                                    .queue_size = 1,                                                    \
                                                                                                    .pre_cb = 0,                                                        \
                                                                                                    .post_cb = 0,                                                       \
                                                                                                },                                                                      \
                                                                                                .Master = {                                                             \
                                                                                                    .mosi_io_num = GPIO_NUM_NC,                                         \
                                                                                                    .miso_io_num = CONFIG_LEPTON_VOSPI_MISO,                            \
                                                                                                    .sclk_io_num = CONFIG_LEPTON_VOSPI_SCK,                             \
                                                                                                    .quadwp_io_num = GPIO_NUM_NC,                                       \
                                                                                                    .quadhd_io_num = GPIO_NUM_NC,                                       \
                                                                                                    .data4_io_num = GPIO_NUM_NC,                                        \
                                                                                                    .data5_io_num = GPIO_NUM_NC,                                        \
                                                                                                    .data6_io_num = GPIO_NUM_NC,                                        \
                                                                                                    .data7_io_num = GPIO_NUM_NC,                                        \
                                                                                                    .data_io_default_level = false,                                     \
                                                                                                    .max_transfer_sz = 0,                                               \
                                                                                                    .flags = 0,                                                         \
                                                                                                    .isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO,                           \
                                                                                                    .intr_flags = 0,                                                    \
                                                                                                },                                                                      \
                                                                                                .Host = LEPTON_VOSPI_SPI_HOST,                                          \
                                                                                                .DMA = SPI_DMA_CH_AUTO,                                                 \
                                                                                                .isInitialized = false,                                                 \
                                                                                                .isResync = false,                                                      \
                                                                                                .isCapturing = false,                                                   \
                                                                                                .useTelemetry = false,                                                  \
                                                                                                .Handle = NULL,                                                         \
                                                                                                .Packet = NULL,                                                         \
                                                                                                .ImageHeight = 0,                                                       \
                                                                                                .ImageWidth = 0,                                                        \
                                                                                                .BytesPerPixel = 0,                                                     \
                                                                                                .Image_Buffer = { NULL },                                               \
                                                                                                .CurrentBuffer = 0,                                                     \
                                                                                                .SyncErrors = 0,                                                        \
                                                                                                .FrameCounter = 0,                                                      \
                                                                                                .ResyncStartUs = 0,                                                     \
                                                                                            }                                                                           \
                                                                                        }

/** @brief          Assign the I2C functions to a Lepton configuration object.
 *  @param Conf     Lepton configuration object
 *  @param Init     I2C initialization function
 *                  NOTE: Can be set to NULL to skip the initialization.
 *  @param Deinit   I2C deinitialization function
 *                  NOTE: Can be set to NULL to skip the initialization.
 *  @param Write    I2C write function
 *  @param Read     I2C read function
 */
#define LEPTON_ASSIGN_FUNC(Conf, Init, Deinit, Write, Read)                             do {                                                                            \
                                                                                            Conf.CCI.I2C_Init   = Init;                                                 \
                                                                                            Conf.CCI.I2C_Deinit = Deinit;                                               \
                                                                                            Conf.CCI.I2C_Write  = Write;                                                \
                                                                                            Conf.CCI.I2C_Read   = Read;                                                 \
                                                                                        } while (0)

/** @brief              Add a default I2C configuration to the Lepton configuration object.
 *  @param Conf         Lepton configuration object
 *  @param SDA          I2C SDA pin
 *  @param SCL          I2C SCL pin
 *  @param EnablePullup Set to #true to enable internal pullups for SDA and SCL lines
 *  @param AutoPD       Set to #true to enable automatic power down management
 */
#define LEPTON_DEFAULT_I2C(Conf, SDA, SCL, EnablePullup, AutoPD)                        do {                                                                            \
                                                                                            Conf.CCI.I2C_Bus_Config->i2c_port = CONFIG_LEPTON_I2C_HOST;                 \
                                                                                            Conf.CCI.I2C_Bus_Config->sda_io_num = SDA;                                  \
                                                                                            Conf.CCI.I2C_Bus_Config->scl_io_num = SCL;                                  \
                                                                                            Conf.CCI.I2C_Bus_Config->clk_source = I2C_CLK_SRC_DEFAULT;                  \
                                                                                            Conf.CCI.I2C_Bus_Config->glitch_ignore_cnt = 7;                             \
                                                                                            Conf.CCI.I2C_Bus_Config->intr_priority = 0;                                 \
                                                                                            Conf.CCI.I2C_Bus_Config->trans_queue_depth = 0;                             \
                                                                                            Conf.CCI.I2C_Bus_Config->flags = {                                          \
                                                                                                .enable_internal_pullup = EnablePullup,                                 \
                                                                                                .allow_pd = AutoPD,                                                     \
                                                                                            };                                                                          \
                                                                                        } while (0)

#endif /* LEPTON_CONFIG_H_ */