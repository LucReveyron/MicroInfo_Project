#ifndef SOUND_DETECTION_H
#define SOUND_DETECTION_H


#define FFT_SIZE 	1024


typedef struct {
    float real;
    float imaginary;

}complex_number_t;

void process_audio_data(int16_t *data, uint16_t num_samples);

int16_t calculate_loudest_frequency(float* data);

float calculate_phase(complex_number_t fft_at_frequency);

float detect_sound_phase_shift(void);

int8_t get_frequency_status(void);


#endif /* SOUND_DETECTION_H */
