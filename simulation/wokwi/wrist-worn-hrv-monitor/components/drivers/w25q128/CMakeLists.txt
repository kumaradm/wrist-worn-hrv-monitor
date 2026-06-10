idf_component_register(
    SRCS "w25q128.c"
    INCLUDE_DIRS "include"
    REQUIRES driver spi_flash esp_timer
)