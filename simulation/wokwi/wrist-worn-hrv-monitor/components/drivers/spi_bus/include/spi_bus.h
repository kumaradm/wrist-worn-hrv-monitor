#ifndef SPI_BUS_H_
#define SPI_BUS_H_

#include "esp_flash.h"
#include "driver/spi_common.h"
#include "driver/gpio.h"
#include "esp_flash_spi_init.h"
#include "esp_err.h"

#define SPI_MOSI_IO_NUM 7
#define SPI_MISO_IO_NUM 5
#define SPI_SCLK_IO_NUM 6

esp_err_t init_spi_bus();

#endif