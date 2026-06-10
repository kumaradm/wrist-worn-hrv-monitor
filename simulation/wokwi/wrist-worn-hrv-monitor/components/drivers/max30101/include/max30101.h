#ifndef MAX30101_H_
#define MAX30101_H_

#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "esp_err.h"

#define MAX30101_INT_IO         1
#define I2C_CLK_SPEED           400000
#define MAX30101_ADDRESS        0x57
#define REG_INT_STATUS1         0x00
#define REG_INT_ENABLE1         0x02
#define REG_FIFO_DATA           0x07
#define REG_FIFO_CONFIG         0x08
#define REG_MODE_CONFIG         0x09
#define REG_SPO2_CONFIG         0x0A
#define REG_LED3_PA             0x0E
#define REG_MULTI_LED_CTRL1     0x11

esp_err_t init_max30101(i2c_master_bus_handle_t bus_handle, i2c_master_dev_handle_t* dev_handle);
esp_err_t max30101_register_config(i2c_master_dev_handle_t dev_handle);
esp_err_t write_to_max30101(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t data);

#endif