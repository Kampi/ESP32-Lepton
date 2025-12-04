/**
 * @file lepton.h
 * @brief ESP32 component for FLIR Lepton 3.5 Thermal Camera
 * @author Daniel Kampert
 * @version 1.0.0
 */

#ifndef LEPTON_H_
#define LEPTON_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the Lepton camera driver
 * @return ESP_OK on success
 */
esp_err_t Lepton_Init(void);

/**
 * @brief Deinitialize the Lepton camera driver
 * @return ESP_OK on success
 */
esp_err_t Lepton_Deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* LEPTON_H_ */
