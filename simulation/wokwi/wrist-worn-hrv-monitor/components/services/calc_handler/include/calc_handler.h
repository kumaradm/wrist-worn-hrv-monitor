#ifndef CALC_HANDLER_H_
#define CALC_HANDLER_H_

#include "dsps_biquad.h"
#include "esp_dsp.h"
#include "esp_timer.h"

// ===== SAMPLING CONFIG =====
#define SAMPLING_FREQUENCY      100
#define WINDOW_SIZE_SEC         60
#define NUMBER_OF_SAMPLES       (SAMPLING_FREQUENCY * WINDOW_SIZE_SEC)

// ===== FILTER CONFIG =====
#define DERIV_LEN               5
#define MWI_LEN                 20

typedef struct {
    float coeffs[5];
    float w[2];
} biquad_filter_t;

typedef struct {
    float coeffs[DERIV_LEN];
    float deriv[DERIV_LEN];
    fir_f32_t fir_filter;
} deriv_filter_t;

typedef struct {
    float coeffs[MWI_LEN];
    float delay[MWI_LEN];
    fir_f32_t fir_filter;
} mwi_filter_t;

typedef struct {
    uint32_t timestamp_us[NUMBER_OF_SAMPLES];
    uint32_t t_start;
    uint32_t t_end;
    uint32_t t_peak;
    uint32_t last_peak;
    uint32_t ibi[NUMBER_OF_SAMPLES];
    float value[NUMBER_OF_SAMPLES];
    float signal_level;
    float noise_level;
    float threshold;
    bool is_above_threshold;
    bool is_steady_state;
} ppg_packet_t;

void bandpass_filter_init(biquad_filter_t *lpf, biquad_filter_t *hpf);
float bandpass_filter_process(biquad_filter_t *lpf, biquad_filter_t *hpf, float input);
void deriv_filter_init(deriv_filter_t *deriv);
float deriv_filter_process(deriv_filter_t *deriv, float input);
void mwi_filter_init(mwi_filter_t *mwi);
float mwi_filter_process(mwi_filter_t *mwi, float input);
void ppg_init(ppg_packet_t *ppg);
float peak_detection(ppg_packet_t *ppg, float current_sample);

#endif