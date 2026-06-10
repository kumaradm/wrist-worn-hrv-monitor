idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES spi_bus i2c_bus max30101 w25q128 wifi_manager mqtt_manager calc_handler
)
