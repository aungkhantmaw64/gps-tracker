#ifndef _PAYLOAD_H_
#define _PAYLOAD_H_

#include "esp_err.h"

/**
 * @brief Initialize the payload generator module.
 *
 * @return
 *      - ESP_OK on successful initialization
 *      - Appropriate esp_err_t error code otherwise
 */
esp_err_t payload_init(void);

#endif
