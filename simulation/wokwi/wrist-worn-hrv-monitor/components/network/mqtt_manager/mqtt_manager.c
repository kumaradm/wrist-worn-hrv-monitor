#include "mqtt_manager.h"

static const char *TAG = "MQTT_MANAGER";

static const uint8_t root_ca_pem_start[] asm("_binary_root_ca_pem_start");
static const uint8_t public_key_pem_start[] asm("_binary_wrist_worn_hrv_monitor_cert_pem_start");
static const uint8_t private_key_pem_start[] asm("_binary_wrist_worn_hrv_monitor_private_key_start");
static bool is_mqtt_connected = false;

void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id) {
    
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT Server Connected!");
        is_mqtt_connected = true; 
        esp_mqtt_client_subscribe(client, "device/commands", 1);
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT Connection Lost.");
        is_mqtt_connected = false;
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGD(TAG, "Chunk ID %d successfully reached the cloud", event->msg_id);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "Incoming Cloud Command: %.*s", event->data_len, event->data);
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "MQTT Error Occurred");
        break;

    default:
        break;
    }
}

esp_err_t init_mqtt(esp_mqtt_client_handle_t* client)
{
  const esp_mqtt_client_config_t mqtt_cfg = {
    .broker.address.uri = AWS_BROKER_URI,
    .broker.verification.alpn_protos = (const char *[]) { AWS_ALPN_PROTOCOL, NULL },
    .broker.verification.certificate = (const char *)root_ca_pem_start,
    .credentials = {
      .authentication = {
        .certificate = (const char *)public_key_pem_start,
        .key = (const char *)private_key_pem_start,
      },
    }
  };

    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    *client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(*client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(*client);
    return ESP_OK;
}

void mqtt_publish(esp_mqtt_client_handle_t client, const char* topic, const char* payload) {
        size_t payload_size = strlen(payload);
        int msg_id = esp_mqtt_client_publish(client, topic, payload, payload_size, 1, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
}

esp_err_t stop_mqtt(esp_mqtt_client_handle_t client) {
    ESP_ERROR_CHECK(esp_mqtt_client_disconnect(client));
    return esp_mqtt_client_stop(client);
}