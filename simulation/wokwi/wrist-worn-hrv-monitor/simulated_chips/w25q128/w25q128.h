#include "../wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FLASH_SIZE        16 * 1024 * 1024
#define CMD_READ_ID       0x9F
#define CMD_READ_STATUS   0x05
#define CMD_READ          0x03
#define CMD_WRITE         0x02
#define CMD_WREN          0x06
#define CMD_ERASE         0x20

#define FLASH_MANUFACTURER_ID   0xEF
#define FLASH_CAPACITY_TYPE_ID  0x40
#define FLASH_SIZE_ID           0x18

typedef enum {
  STATE_IDLE,
  STATE_ID_DATA,
  STATE_ADDRESS,
  STATE_READ_DATA,
  STATE_WRITE_DATA,
  STATE_STATUS_REG
} state_t;

typedef struct {
  pin_t cs_pin;
  uint32_t spi_handle;
  uint32_t address;
  uint8_t* data;
  uint8_t spi_buffer;
  uint8_t addr_count;
  bool write_enabled;
  state_t state;
  state_t next_state;
} w25q128_t;

static void on_pin_change(void *user_data, pin_t pin, uint32_t value);
static void on_spi_done(void *user_data, uint8_t *buffer, uint32_t count);
void chip_init();