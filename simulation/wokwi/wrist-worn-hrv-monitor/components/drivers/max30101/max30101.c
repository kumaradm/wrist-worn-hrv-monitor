#include "max30101.h"

esp_err_t init_max30101(i2c_master_bus_handle_t bus_handle, i2c_master_dev_handle_t* dev_handle) {
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_NEGEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << MAX30101_INT_IO),
        .pull_up_en = 1
    };
    gpio_config(&io_conf);

    i2c_device_config_t max30101_config = {
      .dev_addr_length = I2C_ADDR_BIT_LEN_7,
      .device_address = MAX30101_ADDRESS,
      .scl_speed_hz = I2C_CLK_SPEED,
    };

    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &max30101_config, dev_handle));
    max30101_register_config(*dev_handle);

    return ESP_OK;
}

esp_err_t max30101_register_config(i2c_master_dev_handle_t dev_handle) {
    // Set the interrupt to active
    uint8_t int_en = 0x40;
    write_to_max30101(dev_handle, REG_INT_ENABLE1, int_en);

    // Reset MAX30101
    uint8_t reset = 0x40;
    write_to_max30101(dev_handle, REG_MODE_CONFIG, reset);

    // Activate FIFO Rolls on Full (to ensure real-time data)
    uint8_t fifo_config = 0x10;
    write_to_max30101(dev_handle, REG_FIFO_CONFIG, fifo_config);

    // Set mode to Multi-LED
    uint8_t mode_config = 0x07;
    write_to_max30101(dev_handle, REG_MODE_CONFIG, mode_config);

    // Configure ADC range, sample rate, and LED pulse width
    uint8_t spo2_config = 0x27;
    write_to_max30101(dev_handle, REG_SPO2_CONFIG, spo2_config);

    // Configure the green LED current
    uint8_t led_current = 0x24;
    write_to_max30101(dev_handle, REG_LED3_PA, led_current);

    // Only use green LED
    uint8_t slot_config = 0x03;
    write_to_max30101(dev_handle, REG_MULTI_LED_CTRL1, slot_config);

    return ESP_OK;
}

esp_err_t write_to_max30101(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t data) {
    uint8_t write_buffer[2] = {reg_addr, data};

    i2c_master_transmit(dev_handle, write_buffer, 2, -1);

    return ESP_OK;
}