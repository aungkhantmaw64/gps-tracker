#ifndef _MQTT_MGT_H_
#define _MQTT_MGT_H_

#include "esp_err.h"

esp_err_t mqtt_mgt_init(void);

esp_err_t mqtt_mgt_queue_msg(const void *data, size_t len);

#endif
