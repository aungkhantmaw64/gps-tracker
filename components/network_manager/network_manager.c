#include "network_manager.h"
#include "esp_check.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "utils.h"
#include <string.h>

#define NETWORK_MANAGER_FACTORY_WIFI_SSID ("gps-tracker")
#define NETWORK_MANAGER_FACTORY_WIFI_PASSWORD ("abcdefgh")
#define NETWORK_MANAGER_MAXIMUM_RETRY (10)

enum {
  WIFI_CONNECTED_BIT = 0x01,
  WIFI_FAIL_BIT = 0x02,
};

typedef struct network_manager {
  esp_netif_t *netif;
  wifi_config_t config;
  EventGroupHandle_t event_group;
} network_manager_t;

static char *TAG = "network_manager";
static network_manager_t *g_net = NULL;
static int8_t g_retry_count = 0;

static void network_manager_wifi_event_cb(void *arg,
                                          esp_event_base_t event_base,
                                          int32_t event_id, void *event_data);

static void network_manager_ip_event_cb(void *arg, esp_event_base_t event_base,
                                        int32_t event_id, void *event_data);

esp_err_t network_manager_init(void) {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  g_net = MALLOC(sizeof(network_manager_t));
  g_net->event_group = xEventGroupCreate();
  g_net->netif = esp_netif_create_default_wifi_sta();

  wifi_init_config_t init_config = WIFI_INIT_CONFIG_DEFAULT();

  ESP_ERROR_CHECK(esp_wifi_init(&init_config));
  ESP_ERROR_CHECK(esp_event_handler_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, &network_manager_wifi_event_cb, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(
      IP_EVENT, ESP_EVENT_ANY_ID, &network_manager_ip_event_cb, NULL));

  wifi_config_t config = {.sta = {
                              .ssid = NETWORK_MANAGER_FACTORY_WIFI_SSID,
                              .password = NETWORK_MANAGER_FACTORY_WIFI_PASSWORD,
                              .threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK,
                              .owe_enabled = false,
                          }};
  memcpy(&g_net->config, &config, sizeof(config));

  ESP_LOGI(TAG, "******** Default Network Configuration ******");
  ESP_LOGI(TAG, "WiFi SSID: %s", config.sta.ssid);
  ESP_LOGI(TAG, "WiFi Password: %s", config.sta.password);
  ESP_LOGI(TAG, "Threshold authmode: WIFI_AUTH_WPA_WPA2_PSK");

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &g_net->config));
  ESP_ERROR_CHECK(esp_wifi_set_bandwidth(ESP_IF_WIFI_STA, WIFI_BW_HT20));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG, "Wifi STA configured successfully.");

  /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or
   * connection failed for the maximum number of re-tries (WIFI_FAIL_BIT). The
   * bits are set by event_handler() (see above) */
  EventBits_t bits = xEventGroupWaitBits(g_net->event_group,
                                         WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                         pdFALSE, pdFALSE, portMAX_DELAY);

  /* xEventGroupWaitBits() returns the bits before the call returned, hence we
   * can test which event actually happened. */
  if (bits & WIFI_CONNECTED_BIT) {
    ESP_LOGI(TAG, "connected to ap SSID:%s. password:%s.", config.sta.ssid,
             config.sta.password);
  } else if (bits & WIFI_FAIL_BIT) {
    ESP_LOGE(TAG, "failed to connect to SSID:%s, password:%s", config.sta.ssid,
             config.sta.password);
  } else {
    ESP_LOGE(TAG, "UNEXPECTED EVENT");
  }
  return ESP_OK;
}

static void network_manager_wifi_event_cb(void *arg,
                                          esp_event_base_t event_base,
                                          int32_t event_id, void *event_data) {
  switch (event_id) {
  case WIFI_EVENT_STA_START: {
    ESP_LOGI(TAG, "WIFI_EVENT_STA_START!");
    ESP_ERROR_CHECK(esp_wifi_connect());
    break;
  }
  case WIFI_EVENT_STA_DISCONNECTED: {
    ESP_LOGE(TAG, "WIFI_EVENT_STA_DISCONNECTED!");
    if (g_retry_count < NETWORK_MANAGER_MAXIMUM_RETRY) {
      ESP_ERROR_CHECK(esp_wifi_connect());
      g_retry_count++;
      ESP_LOGI(TAG, "retry to connect to the AP. Total retries: %" PRIu8,
               g_retry_count);
    } else {
      xEventGroupSetBits(g_net->event_group, WIFI_FAIL_BIT);
    }
    ESP_LOGI(TAG, "connect to the AP fail");
  }
  default:
    break;
  }
}

static void network_manager_ip_event_cb(void *arg, esp_event_base_t event_base,
                                        int32_t event_id, void *event_data) {
  switch (event_id) {
  case IP_EVENT_STA_GOT_IP: {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    g_retry_count = 0;
    xEventGroupSetBits(g_net->event_group, WIFI_CONNECTED_BIT);
    break;
  }
  default:
    break;
  }
}
