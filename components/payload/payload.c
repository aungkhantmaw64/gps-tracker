#include "payload.h"
#include "esp_log.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "utils.h"
#include <stdint.h>
#include <stdio.h>

#define PAYLOAD_TASK_SIZE (4096)
#define PAYLOAD_TASK_PRIORITY (tskIDLE_PRIORITY + 2)
#define PAYLOAD_GENERATION_INTERVAL_MS (5000)
#define PAYLOAD_DATA_SIZE (11) // (LNG (4) + LAT (4) + BAT (2) + NULL (1)
#define PAYLOAD_MSG_SIZE (100)

static TaskHandle_t g_payload_task_handle = NULL;
static char *TAG = "payload";
static char g_msg[PAYLOAD_MSG_SIZE] = {0};

static void payload_task_entry(void *user_ctx);

esp_err_t payload_init(void) {
  BaseType_t ret = xTaskCreatePinnedToCore(
      payload_task_entry, "payload_task", PAYLOAD_TASK_SIZE, NULL,
      PAYLOAD_TASK_PRIORITY, &g_payload_task_handle, 1);
  if (pdPASS != ret) {
    ESP_LOGE(TAG, "Failed to create payload task!");
  }
  ESP_LOGI(TAG, "Payload generation has started successfully.");
  return ESP_OK;
}

static void payload_task_entry(void *user_ctx) {
  while (true) {
    uint16_t lat = (uint16_t)esp_random();
    uint16_t lng = (uint16_t)esp_random();
    uint8_t bat_percent = (uint8_t)esp_random();
    ESP_LOGI(TAG, ">>>>>>> PAYLOAD MESSAGE <<<<<<<<");
    ESP_LOGD(TAG, "Latitude: %" PRIu16, lat);
    ESP_LOGD(TAG, "Logitude: %" PRIu16, lng);
    ESP_LOGD(TAG, "Battery Percentage: %" PRIu8, bat_percent);

    char payload[PAYLOAD_DATA_SIZE] = {0};

    sprintf(payload, "%04X%04X%02X", lat, lng, bat_percent);
    ESP_LOGD(TAG, "Payload: %s", payload);

    int offset = 0;
    offset += snprintf(g_msg + offset, sizeof(g_msg) - offset, "{\n");
    offset += snprintf(g_msg + offset, sizeof(g_msg) - offset,
                       "\"id\": \"%s\",\n", UTILS_DEVICE_ID);
    offset += snprintf(g_msg + offset, sizeof(g_msg) - offset,
                       "\"payload\": \"%s\",\n", payload);
    offset +=
        snprintf(g_msg + offset, sizeof(g_msg) - offset, "\"date\": \"\",\n");
    offset +=
        snprintf(g_msg + offset, sizeof(g_msg) - offset, "\"time\": \"\",\n");
    offset += snprintf(g_msg + offset, sizeof(g_msg) - offset, "}\n");

    ESP_LOGI(TAG, "%s", g_msg);

    vTaskDelay(pdMS_TO_TICKS(PAYLOAD_GENERATION_INTERVAL_MS));
  }
  vTaskDelete(NULL);
}
