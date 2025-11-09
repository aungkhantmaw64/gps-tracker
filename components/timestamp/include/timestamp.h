#ifndef _TIMESTAMP_H_
#define _TIMESTAMP_H_

#include "esp_err.h"

/**
 * @brief Maximum length for the date string, including the null terminator.
 *
 * Example format: "YYYY-MM-DD" (10 characters + 1 NULL)
 */
#define TIMESTAMP_MAX_DATE_LEN (11)

/**
 * @brief Maximum length for the time string, including the null terminator.
 *
 * Example format: "HH:MM:SS" (8 characters + 1 NULL)
 */
#define TIMESTAMP_MAX_TIME_LEN (9)

/**
 * @brief Structure to hold date and time strings.
 *
 * The date and time are stored as null-terminated strings.
 * - date: formatted as "YYYY-MM-DD"
 * - time: formatted as "HH:MM:SS"
 */
typedef struct timestamp {
  char date[TIMESTAMP_MAX_DATE_LEN]; ///< Date string (e.g., "2024-06-09")
  char time[TIMESTAMP_MAX_TIME_LEN]; ///< Time string (e.g., "14:30:45")
} timestamp_t;

/**
 * @brief Update the system time from an external source (e.g., SNTP).
 *
 * @return
 *    - ESP_OK on success
 *    - Appropriate error code otherwise
 */
esp_err_t timestamp_update_time(void);

/**
 * @brief Get the current date and time in Thailand (default) timezone.
 *
 * Fills the provided timestamp_t structure with the current date and time
 * as strings in the formats "YYYY-MM-DD" and "HH:MM:SS".
 *
 * @param[out] stamp Pointer to timestamp_t structure to fill.
 * @return
 *    - ESP_OK on success
 *    - ESP_ERR_INVALID_ARG if stamp is NULL
 *    - Appropriate error code otherwise
 */
esp_err_t timestamp_now(timestamp_t *stamp);

#endif
