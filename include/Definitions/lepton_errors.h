/*
 * lepton_errors.h
 *
 *  Copyright (C) Daniel Kampert, 2026
 *  Website: www.kampis-elektroecke.de
 *  File info: Error definitions for Lepton driver.
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

#ifndef LEPTON_ERRORS_H_
#define LEPTON_ERRORS_H_

#include <esp_log.h>

#include <sdkconfig.h>

#ifndef CONFIG_LEPTON_MISC_ERROR_BASE
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

/** @brief Operation not finished.
 */
#define LEPTON_ERR_NOT_FINISHED                                 (LEPTON_ERROR_BASE + 10)

/** @brief      Macro to check for Lepton errors in function calls.
 *  @param Func Function call to check
 */
#define LEPTON_ERROR_CHECK(Func)                                do                                                                                  \
                                                                {                                                                                   \
                                                                    Lepton_Error_t Error = Func;                                                    \
                                                                    if(Error != LEPTON_ERR_OK)                                                      \
                                                                    {                                                                               \
                                                                        ESP_LOGE("Lepton_Error", "Error: 0x%X", static_cast<unsigned int>(Error));  \
                                                                        return Error;                                                               \
                                                                    }                                                                               \
                                                                } while(0)

#endif /* LEPTON_ERRORS_H_ */