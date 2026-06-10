#ifndef W25Q128_H_
#define W25Q128_H_

#include "esp_flash.h"
#include "driver/spi_common.h"
#include "esp_flash_spi_init.h"
#include "esp_err.h"
#include "esp_timer.h"

#define SPI_CS_IO_NUM       4
#define PAGE_SIZE_BYTES     256
#define ENTRIES_PER_PAGE    (PAGE_SIZE_BYTES / sizeof(uint32_t))

esp_err_t init_w25q128(esp_flash_t** flash);
esp_err_t write_ppg_sample_to_w25q128(esp_flash_t* flash, uint32_t ppg);
esp_err_t get_page_from_w25q128(esp_flash_t* flash, uint32_t page_number, uint32_t* read_buffer);
uint32_t get_total_page_written_to_w25q128();

#endif