#include "spi_bus.h"

esp_err_t init_spi_bus() {
    spi_bus_config_t bus_config = {
        .mosi_io_num = SPI_MOSI_IO_NUM,
        .miso_io_num = SPI_MISO_IO_NUM,
        .sclk_io_num = SPI_SCLK_IO_NUM,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };
    spi_bus_initialize(SPI2_HOST, &bus_config, SPI_DMA_CH_AUTO);
    
    return ESP_OK;
}