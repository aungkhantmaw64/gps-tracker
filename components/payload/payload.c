#include "payload.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_sntp.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mqtt_mgt.h"
#include "timestamp.h"
#include "utils.h"
#include <stdint.h>
#include <stdio.h>

/**
 * @brief Size of the payload task stack in bytes.
 */
#define PAYLOAD_TASK_SIZE (4096)

/**
 * @brief Priority of the payload task.
 */
#define PAYLOAD_TASK_PRIORITY (tskIDLE_PRIORITY + 2)

/**
 * @brief Interval in milliseconds between payload generations.
 */
#define PAYLOAD_GENERATION_INTERVAL_MS (5000)

/**
 * @brief Size of the payload data in bytes (LNG (4) + LAT (4) + BAT (2) + NULL
 * (1)).
 */
#define PAYLOAD_DATA_SIZE (11)

/**
 * @brief Maximum size of the payload message in bytes.
 */
#define PAYLOAD_MSG_SIZE (100)

/********************************************************************************
 *
 *                              Private Global Variables
 *
 ********************************************************************************/

/**
 * @brief Handle for the payload task.
 *
 * Used to manage and reference the FreeRTOS task responsible for payload
 * generation.
 */
static TaskHandle_t g_payload_task_handle = NULL;

/**
 * @brief Tag used for logging messages from the payload module.
 */
static char *TAG = "payload";

/**
 * @brief Buffer for storing the generated payload message.
 *
 * The buffer size is defined by PAYLOAD_MSG_SIZE.
 */
static char g_msg[PAYLOAD_MSG_SIZE] = {0};

/********************************************************************************
 *
 *                              Private Function Prototypes
 *
 ********************************************************************************/

/**
 * @brief Entry point function for the payload task.
 *
 * This function contains the main loop for generating and handling payload
 * data. It is intended to be run as a FreeRTOS task.
 *
 * @param user_ctx Pointer to user-defined context data (can be NULL).
 */
static void payload_task_entry(void *user_ctx);

/********************************************************************************
 *
 *                              Public Function Definitions
 *
 ********************************************************************************/
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

/********************************************************************************
 *
 *                              Private Function Definitions
 *
 ********************************************************************************/
static void payload_task_entry(void *user_ctx) {
  while (true) {
    uint16_t lat = (uint16_t)esp_random();
    uint16_t lng = (uint16_t)esp_random();
    uint8_t bat_percent = (uint8_t)esp_random();
    ESP_LOGI(TAG, ">>>>>>> PAYLOAD MESSAGE <<<<<<<<");

    // Map raw 16-bit to real-world coordinates
    float latitude = ((float)lat / 65535.0) * 180.0 - 90.0;   // -90° to +90°
    float longitude = ((float)lng / 65535.0) * 360.0 - 180.0; // -180° to +180°
    float battery = ((float)bat_percent / 255.0) * 100.0;     // 0-100%

    ESP_LOGI(TAG, "Latitude: %.3f", latitude);
    ESP_LOGI(TAG, "Logitude: %.3f", longitude);
    ESP_LOGI(TAG, "Battery Percentage: %.3f", battery);

    char payload[PAYLOAD_DATA_SIZE] = {0};

    sprintf(payload, "%04X%04X%02X", lat, lng, bat_percent);
    ESP_LOGD(TAG, "Payload: %s", payload);

    int offset = 0;
    offset += snprintf(g_msg + offset, sizeof(g_msg) - offset, "{\n");
    offset += snprintf(g_msg + offset, sizeof(g_msg) - offset,
                       "\"id\": \"%s\",\n", UTILS_DEVICE_ID);
    offset += snprintf(g_msg + offset, sizeof(g_msg) - offset,
                       "\"payload\": \"%s\",\n", payload);

    timestamp_t timestamp;
    if (ESP_OK != timestamp_now(&timestamp)) {
      ESP_LOGE(TAG, "Failed to get current timestamp!");
    }
    offset += snprintf(g_msg + offset, sizeof(g_msg) - offset,
                       "\"date\": \"%s\",\n", timestamp.date);
    offset += snprintf(g_msg + offset, sizeof(g_msg) - offset,
                       "\"time\": \"%s\"\n", timestamp.time);
    offset += snprintf(g_msg + offset, sizeof(g_msg) - offset, "}\n");

    ESP_LOGI(TAG, "%s", g_msg);
    if (ESP_OK != mqtt_mgt_queue_msg(g_msg, strlen(g_msg))) {
      ESP_LOGE(TAG, "Failed to queue the payload!");
    }
    vTaskDelay(pdMS_TO_TICKS(PAYLOAD_GENERATION_INTERVAL_MS));
  }
  vTaskDelete(NULL);
}
