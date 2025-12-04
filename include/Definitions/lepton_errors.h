 /*
 * lepton_errors.h
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

#ifndef LEPTON_ERRORS_H_
#define LEPTON_ERRORS_H_

#include <esp_log.h>

#include <sdkconfig.h>

#ifndef CONFIG_LEPTON_ERROR_BASE
    #define CONFIG_LEPTON_MISC_ERROR_BASE                       0xB000
#endif

typedef uint32_t Lepton_Error_t;

/** @brief Lepton error base.
 */
#define LEPTON_ERROR_BASE                                       CONFIG_LEPTON_MISC_ERROR_BASE

/** @brief Everything ok.
 */
#define LEPTON_ERR_OK                                           (LEPTON_ERROR_BASE + 0)

/** @brief Unknown device error.
 */
#define LEPTON_ERR_UNKNOWN_DEVICE                               (LEPTON_ERROR_BASE + 1)

/** @brief Invalid function parameter.
 */
#define LEPTON_ERR_INVALID_ARG                                  (LEPTON_ERROR_BASE + 2)

/** @brief Generic error.
 */
#define LEPTON_ERR_FAIL                                         (LEPTON_ERROR_BASE + 3)

/** @brief No memory available.
 */
#define LEPTON_ERR_NO_MEM                                       (LEPTON_ERROR_BASE + 4)

/** @brief A timeout has occured.
 */
#define LEPTON_ERR_TIMEOUT                                      (LEPTON_ERROR_BASE + 5)

/** @brief Invalid device state.
 */
#define LEPTON_ERR_INVALID_STATE                                (LEPTON_ERROR_BASE + 6)

/** @brief Device is not initialized.
 */
#define LEPTON_ERR_NOT_INITIALIZED                              (LEPTON_ERROR_BASE + 7)

/** @brief Device busy.
 */
#define LEPTON_ERR_BUSY                                         (LEPTON_ERROR_BASE + 8)

/** @brief Operation not supported.
 */
#define LEPTON_ERR_NOT_SUPPORTED                                (LEPTON_ERROR_BASE + 9)

/** @brief
 */
#define LEPTON_ERROR_CHECK(Func)                                do                                                                                  \
                                                                {                                                                                   \
                                                                    Lepton_Error_t Error = Func;                                                    \
                                                                    if(Error != LEPTON_ERR_OK)                                                      \
                                                                    {                                                                               \
                                                                        ESP_LOGI("Lepton_Error", "Error: 0x%X", static_cast<unsigned int>(Error));  \
                                                                        return Error;                                                               \
                                                                    }                                                                               \
                                                                } while(0);

#endif /* LEPTON_ERRORS_H_ */