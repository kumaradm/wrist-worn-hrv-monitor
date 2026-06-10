#ifndef WIFI_MANAGER_H_
#define WIFI_MANAGER_H_

#include <string.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_sntp.h"
#include "portmacro.h"
#include "esp_log.h"

#define WIFI_CONNECTED_BIT  BIT0
#define WIFI_FAIL_BIT       BIT1
#define WIFI_MAXIMUM_RETRY  5

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
void init_wifi();
void stop_wifi();
void sync_time();

#endif