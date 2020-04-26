
#include "ch.h"
#include "hal.h"
#include <main.h>

#include "pi_regulator.h"
#include "motors.h"
#include <arm_math.h>


//PI regulator implementation
int16_t pi_regulator(float distance, float goal)
{

	float error = 0;
	float speed = 0;

	static float sum_error = 0;

	error = distance - goal;

	//disables the PI regulator if the error is to small
	if(fabs(error) < ERROR_THRESHOLD){
		return 0;
	}

	sum_error += error;

	//we set a maximum and a minimum for the sum to avoid an uncontrolled growth
	if(sum_error > MAX_SUM_ERROR){
		sum_error = MAX_SUM_ERROR;
	}else if(sum_error < -MAX_SUM_ERROR){
		sum_error = -MAX_SUM_ERROR;
	}

	speed = KP_TRACK * error + KI_TRACK * sum_error;

    return (int16_t)speed;
}
