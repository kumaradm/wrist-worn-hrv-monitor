#ifndef TEST_DATA_H_
#define TEST_DATA_H_

#include "../wokwi-api.h"
#include <stdlib.h>
#include <math.h>
// #include <stdio.h>

#define ADDRESS           0x57
#define TIME_SCALE        1
#define REAL_FREQ_HZ      100
#define SCALED_PERIOD_NS  ((1e9 / REAL_FREQ_HZ) * TIME_SCALE)

// MAX30101 Register Addresses
#define REG_INT_STATUS1         0x00
#define REG_INT_ENABLE1         0x02
#define REG_FIFO_DATA           0x07
#define REG_FIFO_CONFIG         0x08
#define REG_MODE_CONFIG         0x09
#define REG_SPO2_CONFIG         0x0A
#define REG_LED3_PA             0x0E
#define REG_MULTI_LED_CTRL1     0x11

#define MEAN_BPM                70.0
#define STD_DEV_S               (60.0 / 1000.0) // 60ms to seconds
#define BASELINE_OFFSET         100000.0

#define MAX30101_INIT_DELAY     60000

typedef struct {
    pin_t pin_int;
    timer_t reading_timer;
    
    uint32_t current_sample;
    double total_time_s;
    double current_beat_start_s;
    double next_beat_interval_s;
    double motion_offset;
    int motion_samples_left;
    uint8_t byte_index;
    uint8_t registers[0xFF];
    uint8_t reg_address;
    bool is_first_byte;
} max30101_t;

static bool on_i2c_connect(void *user_data, uint32_t address, bool connect);
static bool on_i2c_write(void *user_data, uint8_t data);
static uint8_t on_i2c_read(void *user_data);
void reading_timer_callback(void *user_data);
double random_normal(double mean, double stddev);
void ppg_signal_maker(void *user_data);
void chip_init();

#endif
