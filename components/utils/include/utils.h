#ifndef _UTILS_H_
#define _UTILS_H_

#include "esp_heap_caps.h"

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

#endif
