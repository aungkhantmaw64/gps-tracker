#ifndef _UTILS_H_
#define _UTILS_H_

#include "esp_err.h"
#include "esp_heap_caps.h"
#include <stdint.h>

#define UTILS_HEX_STRING_SIZE (11)
#define UTILS_DEVICE_ID "ESP32_001"

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

void utils_mac_uint8_to_string(char *mac_str, size_t mac_str_len, uint8_t *mac);

#endif
