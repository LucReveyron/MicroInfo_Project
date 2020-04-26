#include "ch.h"
#include "hal.h"
#include <main.h>
#include <usbcfg.h>
#include <chprintf.h>

#include "sound_detection.h"
#include <audio/microphone.h>
#include <fft.h>
#include <arm_math.h>

//2 times FFT_SIZE because these arrays contain complex numbers (real + imaginary)
static float micLeft_cmplx_input[2 * FFT_SIZE];
static float micRight_cmplx_input[2 * FFT_SIZE];

static float micLeft_output[FFT_SIZE];

static complex_number_t micRight_fft_data;
static complex_number_t micLeft_fft_data;

static int8_t frequency_on;

#define MIN_VALUE_THRESHOLD	10000

#define MIN_FREQ				20	//we don't analyze before this index to not use resources for nothing
#define FREQ_LA_MINUS   		27	//422Hz	(the frequency is approximately equivalent to index*15,625 [Hz])
#define FREQ_LA				28  //437Hz
#define FREQ_LA_PLUS			29	//453Hz
#define MAX_FREQ				40	//we don't analyze after this index to not use resources for nothing


void process_audio_data(int16_t *data, uint16_t num_samples)
{
	/*
		*
		*	We get 160 samples per mic every 10ms
		*	So we fill the samples buffers to reach
		*	1024 samples, then we compute the FFTs.
		*
		*/


		static uint16_t nb_samples = 0;

		//loop to fill the buffers
		for(uint16_t i = 0 ; i < num_samples ; i+=4){

			//construct an array of complex numbers. Put 0 to the imaginary part
			micRight_cmplx_input[nb_samples] = (float)data[i + MIC_RIGHT];
			micLeft_cmplx_input[nb_samples] = (float)data[i + MIC_LEFT];

			nb_samples++;

			micRight_cmplx_input[nb_samples] = 0;
			micLeft_cmplx_input[nb_samples] = 0;

			nb_samples++;

			//stop when buffer is full
			if(nb_samples >= (2 * FFT_SIZE)){
				break;
			}
		}

		if(nb_samples >= (2 * FFT_SIZE)){
			/*	FFT proccessing
			*
			*	This FFT function stores the results in the input buffer given.
			*	This is an "In Place" function.
			*/

			doFFT_optimized(FFT_SIZE, micRight_cmplx_input);
			doFFT_optimized(FFT_SIZE, micLeft_cmplx_input);

			arm_cmplx_mag_f32(micLeft_cmplx_input,micLeft_output,FFT_SIZE); // compute the magnitude

			nb_samples = 0;

			//find the loudest frequency
			int16_t loudest_frequency_index = 0;
			loudest_frequency_index = calculate_loudest_frequency(micLeft_output);

			if((loudest_frequency_index >= FREQ_LA_MINUS) && (loudest_frequency_index <= FREQ_LA_PLUS))
			{
                frequency_on = 1;

                //save the relevant data to compute the phase
                micRight_fft_data.real = micRight_cmplx_input[2*loudest_frequency_index];
                micRight_fft_data.imaginary = micRight_cmplx_input[2*loudest_frequency_index+1];

                micLeft_fft_data.real = micLeft_cmplx_input[2*loudest_frequency_index];
                micLeft_fft_data.imaginary = micLeft_cmplx_input[2*loudest_frequency_index+1];

                //reduces the number of times process_audio_data is called
                chThdSleepMilliseconds(10);

            }

			else
            {
            		frequency_on = 0;
            }

		}
}

/*
*	Simple function used to detect the highest value in a buffer
*/
int16_t calculate_loudest_frequency(float* data)
{
	float max_norm = MIN_VALUE_THRESHOLD;
	int16_t max_norm_index = -1;

	//search for the highest peak
	for(uint16_t i = MIN_FREQ ; i <= MAX_FREQ ; i++){
		if(data[i] > max_norm){
			max_norm = data[i];
			max_norm_index = i;
		}
	}
	return max_norm_index;
}

float calculate_phase(complex_number_t fft_at_frequency)
{
    float phase=0;
    phase = atan(fft_at_frequency.imaginary/fft_at_frequency.real);

    if(fft_at_frequency.real < 0)
    {
        phase += PI;
    }

    return phase;
}

float detect_sound_phase_shift()
{
    float micRight_phase = 0;
    float micLeft_phase = 0;
    float phase_difference = 0;

    micRight_phase = calculate_phase(micRight_fft_data);
    micLeft_phase = calculate_phase(micLeft_fft_data);

    phase_difference = micLeft_phase - micRight_phase;

	return phase_difference;
}

int8_t get_frequency_status()
{
	return frequency_on;
}




