#include "w25q128.h"

static uint32_t page_buffer[ENTRIES_PER_PAGE];
static uint16_t buffer_index = 0;
static uint32_t current_flash_address = 0;

esp_err_t init_w25q128(esp_flash_t** flash) {
    *flash = NULL;
    esp_flash_spi_device_config_t device_config = {
        .host_id = SPI2_HOST,
        .cs_id = 0,
        .cs_io_num = SPI_CS_IO_NUM,
        .freq_mhz = ESP_FLASH_20MHZ,
    };

    esp_err_t err = spi_bus_add_flash_device(flash, &device_config);
    if (err != ESP_OK) {
        printf("Failed to add flash device: %s\n", esp_err_to_name(err));
        return err;
    }

    err = esp_flash_init(*flash); 
    if (err != ESP_OK) {
        printf("Failed to initialize W25Q128 flash device: %s\n", esp_err_to_name(err));
        return err;
    } else {
        uint32_t size;
        esp_flash_get_size(*flash, &size);
        printf("External Flash Found: %lu MB\n", size / (1024 * 1024));
    }

    ESP_ERROR_CHECK(esp_flash_erase_region(*flash, current_flash_address, 4096));
    uint64_t init_header = 0xDEADBEEFCAFEBABE; // Example 8-byte metadata 
    ESP_ERROR_CHECK(esp_flash_write(*flash, &init_header, 0, 8));

    current_flash_address = 4096;

    return ESP_OK;
}

esp_err_t write_ppg_sample_to_w25q128(esp_flash_t* flash, uint32_t ppg) {
    if (buffer_index == 0) {
        page_buffer[0] = esp_timer_get_time() / 1000;
        printf("New page started at time: %lu ms\n", page_buffer[0]);
    }
    buffer_index++;
    page_buffer[buffer_index] = ppg;
    printf("[%u] %lu \n", buffer_index, ppg);

    if (buffer_index >= ENTRIES_PER_PAGE - 1) {
        printf("Buffer full. Writing 256 bytes to Flash at address: %lu\n", current_flash_address);
        
        if (current_flash_address % 4096 == 0) {
            printf("New sector reached. Erasing 4KB at address: %lu\n", current_flash_address);
            ESP_ERROR_CHECK(esp_flash_erase_region(flash, current_flash_address, 4096));
        }

        ESP_ERROR_CHECK(esp_flash_write(flash, &page_buffer, current_flash_address, PAGE_SIZE_BYTES));
        current_flash_address += PAGE_SIZE_BYTES;
        buffer_index = 0;
    }


    return ESP_OK;
}

uint32_t get_total_page_written_to_w25q128() {
    return (current_flash_address / PAGE_SIZE_BYTES);
}

esp_err_t get_page_from_w25q128(esp_flash_t* flash, uint32_t page_number, uint32_t* read_buffer) {
    uint32_t addr = page_number * PAGE_SIZE_BYTES;
    return esp_flash_read(flash, read_buffer, addr, PAGE_SIZE_BYTES);
}