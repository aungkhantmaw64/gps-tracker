#include "freertos/FreeRTOS.h"
#include "network_manager.h"
#include "payload.h"

#include <stdbool.h>

void app_main(void) {
  ESP_ERROR_CHECK(network_manager_init());
  ESP_ERROR_CHECK(payload_init());
  while (true) {
    vTaskDelay(1000);
  }
}
