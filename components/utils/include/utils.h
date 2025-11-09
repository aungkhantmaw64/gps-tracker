#ifndef _UTILS_H_
#define _UTILS_H_

#include "esp_err.h"
#include "esp_heap_caps.h"
#include <stdint.h>

/**
 * @brief Size of the hex string representation for MAC addresses.
 */
#define UTILS_HEX_STRING_SIZE (11)

/**
 * @brief Device identifier string.
 */
#define UTILS_DEVICE_ID "ESP32_001"

/**
 * @brief Allocate memory using heap_caps_malloc with default capabilities.
 *
 * Allocates memory of the specified size and logs a warning if allocation
 * fails, including the requested size, returned pointer, and current free heap
 * size.
 *
 * @param size Number of bytes to allocate.
 * @return Pointer to the allocated memory, or NULL if allocation fails.
 */
#define MALLOC(size)                                                           \
  ({                                                                           \
    void *ptr = heap_caps_malloc(size, MALLOC_CAP_DEFAULT);                    \
    if (!ptr) {                                                                \
      ESP_LOGW(TAG,                                                            \
               "<ESP_ERR_NO_MEM> Malloc size: %d, ptr: %p, heap free: %d",     \
               (int)size, ptr, esp_get_free_heap_size());                      \
    }                                                                          \
    ptr;                                                                       \
  })

/**
 * @brief Convert a MAC address from uint8_t array to string representation.
 *
 * @param mac_str      Output buffer for the MAC address string.
 * @param mac_str_len  Length of the output buffer.
 * @param mac          Pointer to the uint8_t array containing the MAC address.
 */
void utils_mac_uint8_to_string(char *mac_str, size_t mac_str_len, uint8_t *mac);

#endif
