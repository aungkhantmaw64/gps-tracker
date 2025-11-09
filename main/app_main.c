#include "esp_event.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "network_manager.h"
#include "nvs_flash.h"
#include "payload.h"

#include <stdbool.h>

void app_main(void) {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  ESP_ERROR_CHECK(network_manager_init());
  ESP_ERROR_CHECK(payload_init());
  while (true) {
    vTaskDelay(1000);
  }
}
