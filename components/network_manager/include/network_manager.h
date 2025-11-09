#ifndef _NETWORK_MANAGER_H_
#define _NETWORK_MANAGER_H_

#include "esp_err.h"

/**
 * @brief Initialize the network manager.
 *
 * This function sets up the network stack and prepares the ESP32 for network
 * operations, such as connecting to Wi-Fi and initializing network services. It
 * should be called once during application startup before any network-dependent
 * features are used.
 *
 * @return
 *      - ESP_OK on success
 *      - Appropriate esp_err_t error code otherwise
 */
esp_err_t network_manager_init(void);

#endif
