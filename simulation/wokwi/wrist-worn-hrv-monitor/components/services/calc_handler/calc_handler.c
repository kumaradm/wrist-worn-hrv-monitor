#include "calc_handler.h"

void bandpass_filter_init(biquad_filter_t *lpf, biquad_filter_t *hpf) {
    lpf->w[0] = 0;
    lpf->w[1] = 0;
    hpf->w[0] = 0;
    hpf->w[1] = 0;

    dsps_biquad_gen_lpf_f32(lpf->coeffs, 5.0/SAMPLING_FREQUENCY, 0.707);
    dsps_biquad_gen_hpf_f32(hpf->coeffs, 0.5/SAMPLING_FREQUENCY, 0.707);
}

float bandpass_filter_process(biquad_filter_t *lpf, biquad_filter_t *hpf, float input) {
    float output_lpf = 0;
    float output_hpf = 0;

    dsps_biquad_f32(&input, &output_lpf, 1, lpf->coeffs, lpf->w);
    dsps_biquad_f32(&output_lpf, &output_hpf, 1, hpf->coeffs, hpf->w);

    return output_hpf;
}

void deriv_filter_init(deriv_filter_t *deriv) {
    deriv->coeffs[0] = -2.0;
    deriv->coeffs[1] = -1.0;
    deriv->coeffs[2] = 0.0;
    deriv->coeffs[3] = 1.0;
    deriv->coeffs[4] = 2.0;
    
    for(int i = 0; i < DERIV_LEN; i++) {
        deriv->deriv[i] = 0;
    }

    dsps_fir_init_f32(&deriv->fir_filter, deriv->coeffs, deriv->deriv, DERIV_LEN);
}

float deriv_filter_process(deriv_filter_t *deriv, float input) {
    float deriv_output = 0;
    dsps_fir_f32(&deriv->fir_filter, &input, &deriv_output, 1);
    return deriv_output;
}

void mwi_filter_init(mwi_filter_t *mwi) {
    float avg_value = 1.0 / MWI_LEN;
    for(int i = 0; i < MWI_LEN; i++) {
        mwi->coeffs[i] = avg_value;
        mwi->delay[i] = 0;
    }

    dsps_fir_init_f32(&mwi->fir_filter, mwi->coeffs, mwi->delay, MWI_LEN);
}

float mwi_filter_process(mwi_filter_t *mwi, float input) {
    float mwi_output = 0;
    dsps_fir_f32(&mwi->fir_filter, &input, &mwi_output, 1);
    return mwi_output;
}

void ppg_init(ppg_packet_t *ppg) {
    ppg->signal_level = 0.5;
    ppg->noise_level = 0.1;
    ppg->threshold = 0.3;
    ppg->t_start = 0;
    ppg->t_end = 0;
    ppg->last_peak = 0;
    ppg->is_above_threshold = false;
    ppg->is_steady_state = false;
}

float peak_detection(ppg_packet_t *ppg, float current_sample) {
    ppg->threshold = ppg->noise_level + 0.25f * (ppg->signal_level - ppg->noise_level);
    if(!ppg->is_steady_state) {
        ppg->signal_level = 0.125f * current_sample + 0.875f * ppg->signal_level;
        ppg->noise_level = 0.125f * current_sample + 0.875f * ppg->noise_level;
        return 0;
    }

    if(current_sample > ppg->threshold && !ppg->is_above_threshold) {
        ppg->is_above_threshold = true;
        ppg->t_start = esp_timer_get_time() / 1000;
    } else if((current_sample < ppg->threshold) && ppg->is_above_threshold) {
        ppg->is_above_threshold = false;
        
        ppg->t_end = esp_timer_get_time() / 1000;
        ppg->t_peak = (ppg->t_start + ppg->t_end) / 2;
        
        uint32_t ibi = ppg->t_peak - ppg->last_peak;
        
        ppg->last_peak = ppg->t_peak;
        if(ibi > 300 && ibi < 1500) {
            ppg->signal_level = 0.125f * current_sample + 0.875f * ppg->signal_level;
            return current_sample;
        }
    }
    else if(!ppg->is_above_threshold) {
        ppg->noise_level = 0.125f * current_sample + 0.875f * ppg->noise_level;
    }
    return 0;
}