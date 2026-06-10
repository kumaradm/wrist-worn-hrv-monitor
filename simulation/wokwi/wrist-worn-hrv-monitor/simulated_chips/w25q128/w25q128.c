#include "w25q128.h"

#define STATUS_BUSY_BIT 0x01
#define STATUS_WEL_BIT  0x02

static void on_pin_change(void *user_data, pin_t pin, uint32_t value) {
  w25q128_t *chip = (w25q128_t *)user_data;
  if (pin == chip->cs_pin) {
    if (value == LOW) {
      chip->state = STATE_IDLE; 
      chip->addr_count = 0;
      chip->address = 0;
      spi_start(chip->spi_handle, &chip->spi_buffer, 1);
    } else {
      chip->state = STATE_IDLE; 
      spi_stop(chip->spi_handle); 
    }
  }
}

static void on_spi_done(void *user_data, uint8_t *buffer, uint32_t count) {
  w25q128_t *chip = (w25q128_t *)user_data;
  if (!count) return; 
  
  uint8_t byte = buffer[0]; 

  switch (chip->state) {
    case STATE_IDLE:
      if (byte == CMD_READ_ID) {
        chip->state = STATE_ID_DATA;
        buffer[0] = FLASH_MANUFACTURER_ID; 
      } else if (byte == CMD_READ_STATUS) {
        chip->state = STATE_STATUS_REG;
        buffer[0] = (chip->write_enabled ? STATUS_WEL_BIT : 0x00);
      } else if (byte == CMD_WREN) {
        chip->write_enabled = true; 
      } else if (byte == CMD_ERASE && chip->write_enabled) {
        chip->state = STATE_ADDRESS;
        chip->next_state = STATE_IDLE;
        chip->write_enabled = false;
      } else if (byte == CMD_WRITE && chip->write_enabled) {
        chip->state = STATE_ADDRESS;
        chip->next_state = STATE_WRITE_DATA;
        chip->write_enabled = false;
      } else if (byte == CMD_READ) {
        chip->state = STATE_ADDRESS;
        chip->next_state = STATE_READ_DATA;
      }
      break;
    case STATE_ADDRESS:
      chip->address = (chip->address << 8) | byte;
      chip->addr_count++;
      if (chip->addr_count == 3) {
        chip->state = chip->next_state;
        if (chip->state == STATE_READ_DATA) {
          buffer[0] = chip->data[chip->address];
        }
      }
      break;
    case STATE_READ_DATA:
      chip->address++;
      buffer[0] = chip->data[chip->address];
      break;
    case STATE_WRITE_DATA:
      chip->data[chip->address] = byte;
      chip->address++;
      break;
    case STATE_ID_DATA:
      chip->addr_count++;
      if (chip->addr_count == 1) buffer[0] = FLASH_CAPACITY_TYPE_ID;
      else if (chip->addr_count == 2) buffer[0] = FLASH_SIZE_ID;
      else buffer[0] = 0x00;
      break;
    case STATE_STATUS_REG:
      buffer[0] = (chip->write_enabled ? STATUS_WEL_BIT : 0x00);
      break;
  }

  if (pin_read(chip->cs_pin) == LOW) {
      spi_start(chip->spi_handle, buffer, 1);
  }
}

void chip_init() {
  static w25q128_t chip_instance;
  w25q128_t *chip = &chip_instance;
  chip->cs_pin = pin_init("CS", INPUT_PULLUP);
  chip->data = calloc(1, FLASH_SIZE);
  chip->state = STATE_IDLE;
  chip->write_enabled = false;
  
  const pin_watch_config_t watch_config = {
    .edge = BOTH,
    .pin_change = on_pin_change,
    .user_data = chip,
  };
  pin_watch(chip->cs_pin, &watch_config);

  const spi_config_t spi_config = {
    .sck = pin_init("CLK", INPUT),
    .mosi = pin_init("MOSI", INPUT),
    .miso = pin_init("MISO", OUTPUT),
    .done = on_spi_done,
    .user_data = chip,
  };
  chip->spi_handle = spi_init(&spi_config);
}