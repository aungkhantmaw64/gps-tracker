#ifndef _TIMESTAMP_H_
#define _TIMESTAMP_H_

#include "esp_err.h"

#define TIMESTAMP_MAX_DATE_LEN (20)
#define TIMESTAMP_MAX_TIME_LEN (20)

typedef struct timestamp {
  char date[TIMESTAMP_MAX_DATE_LEN];
  char time[TIMESTAMP_MAX_TIME_LEN];
} timestamp_t;

esp_err_t timestamp_update_time(void);

esp_err_t timestamp_now(timestamp_t *stamp);

#endif
