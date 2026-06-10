#include "i2c_bus.h"

esp_err_t init_i2c_bus(i2c_master_bus_handle_t* bus_handle) {
    i2c_master_bus_config_t bus_config = {
      .i2c_port = I2C_NUM_0,
      .scl_io_num = I2C_MASTER_SCL_IO,
      .sda_io_num = I2C_MASTER_SDA_IO,
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .glitch_ignore_cnt = 7,
      .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, bus_handle));
 
    return ESP_OK;
}