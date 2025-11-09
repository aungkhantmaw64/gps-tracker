#include "utils.h"

void utils_mac_uint8_to_string(char *mac_str, size_t mac_str_len,
                               uint8_t *mac) {
  snprintf(mac_str, mac_str_len, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0],
           mac[1], mac[2], mac[3], mac[4], mac[5]);
}
