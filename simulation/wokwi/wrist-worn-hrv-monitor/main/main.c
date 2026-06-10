/*
    Source:
        - https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/i2c.html
        - https://docs.wokwi.com/chips-api/getting-started
        - https://www.snapeda.com/parts/MAX30101EFD+T/Analog%20Devices/datasheet/
*/

#include <complex.h>
#include <stdio.h>
#include <string.h>
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "i2c_bus.h"
#include "spi_bus.h"
#include "max30101.h"
#include "w25q128.h"
#include "wifi_manager.h"
#include "mqtt_manager.h"
#include "calc_handler.h"

#define MODE_BTN_IO 3

typedef enum {
    STATE_OFF,
    STATE_SENSING,
    STATE_SENDING
} system_state_t;

typedef struct {
    i2c_master_dev_handle_t ppg_sensor_handle;
    esp_flash_t* external_flash_chip;
} device_handles_t;

// ===== FREERTOS CONFIG =====
static QueueHandle_t max30101_queue;
volatile bool is_mode_btn_pressed = false;
uint32_t last_mode_btn_press_time = 0;

static void IRAM_ATTR max30101_isr_handler(void* arg) {
    uint32_t io_num = (uint32_t) arg;
    xQueueSendFromISR(max30101_queue, &io_num, NULL);
}

static void IRAM_ATTR mode_btn_isr_handler(void* arg) {
    uint32_t current_time = xTaskGetTickCountFromISR();

    if (current_time - last_mode_btn_press_time > pdMS_TO_TICKS(200)) {
        is_mode_btn_pressed = true;
        last_mode_btn_press_time = esp_timer_get_time() / 1000;
        return;
    }
}

void sys_manager_task(void* pvParameters) {
    device_handles_t* handles = (device_handles_t*)pvParameters;
    printf("HRV RMSSD Calculation initialized...\n");
    system_state_t current_state = STATE_SENSING;
    uint32_t io_num;
    i2c_master_dev_handle_t dev_handle = handles->ppg_sensor_handle;
    esp_flash_t* external_flash_chip = handles->external_flash_chip;
    uint8_t status_reg_addr = REG_INT_STATUS1;
    uint8_t status;
    uint8_t fifo_reg_addr = REG_FIFO_DATA;
    uint8_t read_buffer[3];
    static ppg_packet_t ppg_data;

    biquad_filter_t lpf;
    biquad_filter_t hpf;

    deriv_filter_t deriv;
    mwi_filter_t mwi;
    
    bandpass_filter_init(&lpf, &hpf);
    deriv_filter_init(&deriv);
    mwi_filter_init(&mwi);
    ppg_init(&ppg_data);
    
    uint32_t t_init = esp_timer_get_time() / 1000;

    ESP_ERROR_CHECK(i2c_master_transmit_receive(dev_handle, &status_reg_addr, 1, &status, 1, -1));
    
    while(1) {
        if (is_mode_btn_pressed) {
            current_state = (current_state == STATE_SENSING) ? STATE_SENDING : STATE_SENSING;
            is_mode_btn_pressed = false;
        }
        switch (current_state) {
            case STATE_SENSING:
                if(xQueueReceive(max30101_queue, &io_num, portMAX_DELAY)) {
                    uint32_t now = esp_timer_get_time() / 1000;
        
                    uint64_t t_start = esp_timer_get_time();
                    ESP_ERROR_CHECK(i2c_master_transmit_receive(dev_handle, &fifo_reg_addr, 1, read_buffer, 3, -1));
        
                    uint32_t raw_ppg_sample = (read_buffer[0] << 16) | (read_buffer[1] << 8) | read_buffer[2];
                    raw_ppg_sample &= 0x3FFFF;

                    ESP_ERROR_CHECK(write_ppg_sample_to_w25q128(external_flash_chip, raw_ppg_sample));
        
                    uint64_t t_end = esp_timer_get_time();
                    ESP_ERROR_CHECK(i2c_master_transmit_receive(dev_handle, &status_reg_addr, 1, &status, 1, -1));
                } else {
                    printf("While loop in rmssd_calc initiated but no data received\n");
                    vTaskDelay(pdMS_TO_TICKS(5));
                    ESP_ERROR_CHECK(i2c_master_transmit_receive(dev_handle, &status_reg_addr, 1, &status, 1, -1));
                }
                break;
            case STATE_SENDING:
                init_wifi();
                sync_time();
                
                esp_mqtt_client_handle_t mqtt_client;
                ESP_ERROR_CHECK(init_mqtt(&mqtt_client));
                
                uint32_t total_pages = get_total_page_written_to_w25q128();
                printf("Total Pages Written to Flash: %lu\n", total_pages);
                for (uint32_t page_num = 0; page_num < total_pages; page_num++) {
                    uint32_t page_data[ENTRIES_PER_PAGE];
                    get_page_from_w25q128(external_flash_chip, page_num, page_data);
                       
                }

                ESP_ERROR_CHECK(stop_mqtt(mqtt_client));
                stop_wifi();

                current_state = STATE_SENSING;
                vTaskDelay(pdMS_TO_TICKS(1000));
                break;
            case STATE_OFF:
                vTaskDelay(pdMS_TO_TICKS(100));
                break;
            default:
                break;
        }
    }
}

void app_main() {
    printf("Configuring device...\n");
    i2c_master_bus_handle_t i2c_master_bus_handle;
    static device_handles_t device_handles;

    init_i2c_bus(&i2c_master_bus_handle);
    init_max30101(i2c_master_bus_handle, &device_handles.ppg_sensor_handle);
    
    init_spi_bus();
    init_w25q128(&device_handles.external_flash_chip);

    if (device_handles.external_flash_chip == NULL) {
        printf("FATAL: Flash initialization failed!\n");
        return; 
    }

    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_NEGEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << MODE_BTN_IO),
        .pull_up_en = 1
    };
    gpio_config(&io_conf);

    max30101_queue = xQueueCreate(10, sizeof(uint32_t));
    
    gpio_install_isr_service(0);
    gpio_isr_handler_add(MAX30101_INT_IO, max30101_isr_handler, (void*)MAX30101_INT_IO);
    gpio_isr_handler_add(MODE_BTN_IO, mode_btn_isr_handler, (void*)MODE_BTN_IO);
    
    xTaskCreate(sys_manager_task, "sys_manager_task", 8192, &device_handles, 10, NULL);
}