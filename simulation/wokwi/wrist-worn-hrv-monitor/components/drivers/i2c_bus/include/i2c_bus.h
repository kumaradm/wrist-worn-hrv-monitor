#ifndef I2C_BUS_H_
#define I2C_BUS_H_

#include "esp_err.h"
#include "driver/i2c_master.h"

#define I2C_GLITCH_IGONER_CNT   7
#define I2C_MASTER_SCL_IO       2
#define I2C_MASTER_SDA_IO       8

esp_err_t init_i2c_bus(i2c_master_bus_handle_t* bus_handle);

#endif