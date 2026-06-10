#include "max30101.h"

static bool on_i2c_connect(void *user_data, uint32_t address, bool connect) {
    max30101_t *chip = (max30101_t*)user_data;
    chip->is_first_byte = true;
    chip->byte_index = 0;
    return (address == ADDRESS);
}

static bool on_i2c_write(void *user_data, uint8_t data) {
    max30101_t *chip = (max30101_t*)user_data;
    
    if (chip->is_first_byte) {
        chip->reg_address = data;
        chip->is_first_byte = false;
    } else {
        chip->registers[chip->reg_address] = data;
        chip->reg_address++;
    }
        
    return true; 
}

static uint8_t on_i2c_read(void *user_data) {
    max30101_t *chip = (max30101_t*)user_data;

    if (chip->reg_address == REG_FIFO_DATA) {
        if (chip->byte_index == 0) {
            ppg_signal_maker(chip);
            pin_write(chip->pin_int, HIGH);
        }

        uint8_t data = (chip->current_sample >> (16 - (chip->byte_index * 8))) & 0xFF;
        
        chip->byte_index = (chip->byte_index + 1) % 3;
        return data;
    } else if (chip->reg_address == REG_INT_STATUS1) {
        pin_write(chip->pin_int, HIGH);
        
        return 0x40;
    }
    return 0x00;
}

void reading_timer_callback(void *user_data) {
    max30101_t *chip = (max30101_t*)user_data;
    
    pin_write(chip->pin_int, LOW);
  
    timer_start(chip->reading_timer, SCALED_PERIOD_NS / 1000, false);
}

double random_normal(double mean, double stddev) {
    double u1 = (double)rand() / RAND_MAX;
    double u2 = (double)rand() / RAND_MAX;
    double z = sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
    return mean + z * stddev;
}

void ppg_signal_maker(void *user_data) {
    max30101_t *chip = (max30101_t*)user_data;
    double fs = 100.0;
    double dt = 1.0 / fs;

    double t_rel = chip->total_time_s - chip->current_beat_start_s;
    if (t_rel >= chip->next_beat_interval_s) {
        chip->current_beat_start_s = chip->total_time_s;
        chip->next_beat_interval_s = random_normal(60.0 / MEAN_BPM, STD_DEV_S);
        t_rel = 0;
    }

    double systolic = 0.8 * exp(-pow(t_rel - 0.15, 2) / 0.005);
    double diastolic = 0.3 * exp(-pow(t_rel - 0.35, 2) / 0.01);
    double clean_signal = systolic + diastolic;

    double hiss = random_normal(0, 0.05); 
    double wander = 15.0 * sin(2.0 * M_PI * 0.2 * chip->total_time_s);
    if (chip->motion_samples_left > 0) {
        chip->motion_samples_left--;
        if (chip->motion_samples_left == 0) chip->motion_offset = 0;
    } else {
        if ((rand() % 1000) < 5) { 
            chip->motion_samples_left = 50;
            chip->motion_offset = ((double)rand() / RAND_MAX) - 0.5;
        }
    }
    
    double val = BASELINE_OFFSET + (clean_signal * 10000.0) + (wander * 500.0) + (hiss * 5000.0) + (chip->motion_offset * 8000.0);
    
    chip->current_sample = (uint32_t)val;
    chip->total_time_s += dt;
}

void chip_init() {
    static max30101_t chip_instance;
    max30101_t *chip = &chip_instance;
    chip->pin_int = pin_init("INT", OUTPUT_HIGH);
    chip->byte_index = 0;

    const i2c_config_t i2c_config = {
        .user_data = chip,
        .address = ADDRESS,
        .scl = pin_init("SCL", INPUT),
        .sda = pin_init("SDA", INPUT),
        .connect = on_i2c_connect,
        .read = on_i2c_read,
        .write = on_i2c_write,
    };
    i2c_init(&i2c_config);

    const timer_config_t reading_timer_config = {
      .callback = reading_timer_callback,
      .user_data = chip
    };
    chip->reading_timer = timer_init(&reading_timer_config);
    
    srand(get_sim_nanos());
    
    timer_start(chip->reading_timer, MAX30101_INIT_DELAY, false);
}