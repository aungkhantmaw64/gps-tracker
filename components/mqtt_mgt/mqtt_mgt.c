#include "mqtt_mgt.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mqtt_client.h"
#include "utils.h"
#include <stdbool.h>
#include <stdio.h>

#define MQTT_MGT_DEFAULT_BROKER_URL ("mqtt://test.mosquitto.org")
#define MQTT_MGT_TASK_SIZE (4096)
#define MQTT_MGT_TASK_PRIORITY (tskIDLE_PRIORITY + 2)

#define MQTT_MGT_QUEUE_SIZE 10
#define MQTT_MGT_TOPIC_MAX_LEN 64
#define MQTT_MGT_DATA_MAX_LEN 256
#define MQTT_MGT_DEFAULT_QOS (1)
#define MQTT_MGT_DEFAULT_RETAIN (true)

typedef struct {
  char *data;
  size_t len;
} mqtt_mgt_msg_t;

static char *TAG = "mqtt_mgt";

typedef struct {
  bool initialized;
  TaskHandle_t task_handle;
  QueueHandle_t msg_queue;
  esp_mqtt_client_handle_t mqtt_client;
  bool is_connected;
  char topic[MQTT_MGT_TOPIC_MAX_LEN];
} mqtt_mgt_t;

static mqtt_mgt_t g_mqtt = {0};

static void mqtt_mgt_event_handler(void *handler_args, esp_event_base_t base,
                                   int32_t event_id, void *event_data);

static void mqtt_mgt_task_entry(void *user_ctx);

esp_err_t mqtt_mgt_init(void) {
  if (g_mqtt.initialized) {
    ESP_LOGI(TAG, "mqtt_mgt is already initialized!");
    return ESP_OK;
  }
  esp_mqtt_client_config_t mqtt_cfg = {
      .broker.address.uri = MQTT_MGT_DEFAULT_BROKER_URL,
  };
  g_mqtt.mqtt_client = esp_mqtt_client_init(&mqtt_cfg);

  esp_mqtt_client_register_event(g_mqtt.mqtt_client, ESP_EVENT_ANY_ID,
                                 mqtt_mgt_event_handler, NULL);
  esp_mqtt_client_start(g_mqtt.mqtt_client);

  BaseType_t ret = xTaskCreatePinnedToCore(
      mqtt_mgt_task_entry, "mqtt_mgt_task", MQTT_MGT_TASK_SIZE, NULL,
      MQTT_MGT_TASK_PRIORITY, &g_mqtt.task_handle, 1);
  if (pdPASS != ret) {
    ESP_LOGE(TAG, "Failed to create payload task!");
    return ESP_FAIL;
  }

  g_mqtt.msg_queue =
      xQueueCreate(MQTT_MGT_QUEUE_SIZE, sizeof(mqtt_mgt_msg_t *));

  if (NULL == g_mqtt.msg_queue) {
    ESP_LOGE(TAG, "Failed to create a message queue!");
    return ESP_FAIL;
  }

  uint8_t mac[6];

  if (esp_read_mac(mac, ESP_MAC_WIFI_STA) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to read MAC.");
    return ret;
  }

  // sprintf(g_mqtt.topic, "/egress/%02X:%02X:%02X:%02X:%02X:%02X", mac[0],
  // mac[1],
  //         mac[2], mac[3], mac[4], mac[5]);

  sprintf(g_mqtt.topic, "/egress/ESP_01");
  ESP_LOGI(TAG, "Successfully initialized MQTT. Topic: %s", g_mqtt.topic);
  g_mqtt.initialized = true;
  g_mqtt.is_connected = false;
  return ESP_OK;
}

esp_err_t mqtt_mgt_queue_msg(const void *data, size_t len) {
  if (!g_mqtt.initialized) {
    ESP_LOGE(TAG, "MQTT has not been initialized yet!");
    return ESP_FAIL;
  }
  mqtt_mgt_msg_t *p_msg = MALLOC(sizeof(mqtt_mgt_msg_t));
  if (NULL == p_msg) {
    ESP_LOGE(TAG, "Memory allocation failed!");
    return ESP_ERR_NO_MEM;
  }

  p_msg->data = MALLOC(len);

  if (NULL == p_msg->data) {
    ESP_LOGE(TAG, "Memory allocation failed!");
    return ESP_ERR_NO_MEM;
  }
  memcpy(p_msg->data, data, len);

  p_msg->len = len;

  if (xQueueSend(g_mqtt.msg_queue, &p_msg, portMAX_DELAY) != pdPASS) {
    ESP_LOGE(TAG, "Failed to queue a message!");
    FREE(p_msg->data);
    FREE(p_msg);
    return ESP_FAIL;
  }

  ESP_LOGI(TAG, "A message is queued!");

  return ESP_OK;
}

static void mqtt_mgt_event_handler(void *handler_args, esp_event_base_t base,
                                   int32_t event_id, void *event_data) {
  ESP_LOGD(TAG,
           "Event dispatched from event loop base=%s, event_id=%" PRIi32 "",
           base, event_id);
  switch ((esp_mqtt_event_id_t)event_id) {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED!");
    g_mqtt.is_connected = true;
    break;
  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGW(TAG, "MQTT_EVENT_DISCONNECTED");
    g_mqtt.is_connected = false;
    break;
  default:
    break;
  }
}

static void mqtt_mgt_task_entry(void *user_ctx) {
  mqtt_mgt_msg_t *p_msg = {0};
  while (true) {
    if (xQueueReceive(g_mqtt.msg_queue, &p_msg, portMAX_DELAY) == pdTRUE &&
        g_mqtt.is_connected) {
      if (p_msg) {
        esp_mqtt_client_publish(g_mqtt.mqtt_client, g_mqtt.topic, p_msg->data,
                                p_msg->len, MQTT_MGT_DEFAULT_QOS,
                                MQTT_MGT_DEFAULT_RETAIN);
        FREE(p_msg->data);
        FREE(p_msg);
        ESP_LOGI(TAG, "Successfully sent egress message!");
      } else {
        ESP_LOGE(TAG, "NULL message!");
      }
    } else {
      ESP_LOGI(TAG, "MQTT is not connected!");
    }
  }
  vTaskDelete(NULL);
}
