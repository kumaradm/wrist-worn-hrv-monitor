#ifndef MQTT_MANAGER_H_
#define MQTT_MANAGER_H_

#include "esp_log.h"
#include "mqtt_client.h"

#define THINGNAME "wrist_worn_hrv_monitor"
#define AWS_BROKER_URI "a3mb2j4hcx47jo-ats.iot.ap-southeast-1.amazonaws.com:443"
#define AWS_ALPN_PROTOCOL "x-amzn-mqtt-ca"

void log_error_if_nonzero(const char *message, int error_code);
void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
esp_err_t init_mqtt(esp_mqtt_client_handle_t* client);
void mqtt_publish(esp_mqtt_client_handle_t client, const char* topic, const char* data);
esp_err_t stop_mqtt(esp_mqtt_client_handle_t client);

#endif