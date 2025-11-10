#include "timestamp.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_netif_sntp.h"
#include "esp_sntp.h"
#include "esp_wifi.h"
#include "include/timestamp.h"

#define TIMESTAMP_DEFAULT_TIMEZONE (CONFIG_GPS_TRACKER_SNTP_TIME_ZONE)
#define TIMESTAMP_SNTP_SERVER (CONFIG_GPS_TRACKER_SNTP_TIME_SERVER)

/********************************************************************************
 *
 *                              Private Global Variables
 *
 ********************************************************************************/

/**
 * @brief Tag used for logging messages from the timestamp module.
 */
static char *TAG = "timestamp";

/********************************************************************************
 *
 *                              Private Function Prototypes
 *
 ********************************************************************************/

/**
 * @brief Callback function for timestamp notifications.
 *
 * This function is called when a timestamp event occurs.
 * The provided timeval structure contains the current time.
 *
 * @param[in] tv Pointer to a struct timeval containing the current time.
 */
static void timestamp_notification_cb(struct timeval *tv);

/********************************************************************************
 *
 *                              Public Function Definitions
 *
 ********************************************************************************/
esp_err_t timestamp_update_time(void) {
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);
  // Is time set? If not, tm_year will be (1970 - 1900).
  if (timeinfo.tm_year >= (2016 - 1900)) {
    return ESP_OK;
  }

  ESP_LOGI(TAG, "Initializing and starting SNTP");
  /*
   * This is the basic default config with one server and starting the service
   */
  esp_sntp_config_t config =
      ESP_NETIF_SNTP_DEFAULT_CONFIG(TIMESTAMP_SNTP_SERVER);
  config.sync_cb =
      timestamp_notification_cb; // Note: This is only needed if we want

  esp_netif_sntp_init(&config);

  // wait for time to be set
  int retry = 0;
  const int retry_count = 15;
  while (esp_netif_sntp_sync_wait(2000 / portTICK_PERIOD_MS) ==
             ESP_ERR_TIMEOUT &&
         ++retry < retry_count) {
    ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry,
             retry_count);
  }
  time(&now);
  setenv("TZ", "ICT-7", 1);
  tzset();
  localtime_r(&now, &timeinfo);

  char strftime_buf[64];
  strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
  ESP_LOGI(TAG, "The current date/time (%s) is: %s", TIMESTAMP_DEFAULT_TIMEZONE,
           strftime_buf);

  esp_netif_sntp_deinit();
  return ESP_OK;
}

esp_err_t timestamp_now(timestamp_t *stamp) {
  ESP_RETURN_ON_FALSE(NULL != stamp, ESP_ERR_INVALID_ARG, TAG,
                      "stamp is NULL!");
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);

  strftime(stamp->date, TIMESTAMP_MAX_DATE_LEN, "%Y-%m-%d", &timeinfo);
  strftime(stamp->time, TIMESTAMP_MAX_TIME_LEN, "%H:%M:%S", &timeinfo);
  return ESP_OK;
}

/********************************************************************************
 *
 *                              Private Function Definitions
 *
 ********************************************************************************/
static void timestamp_notification_cb(struct timeval *tv) {
  ESP_LOGI(TAG, "Notification of a time synchronization event");
}
