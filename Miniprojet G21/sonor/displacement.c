#include "ch.h"
#include "hal.h"
#include <main.h>

#include "displacement.h"
#include "motors.h"
#include "sensors/proximity.h"
#include "sound_detection.h"
#include "pi_regulator.h"
#include <arm_math.h>

#define TOO_CLOSE  70
#define STOP_SPEED 0

#define IR_FRONT_RIGHT  		0
#define IR_FRONT_RIGHT_45  	1
#define IR_RIGHT  			2
#define IR_FRONT_LEFT  		7
#define IR_FRONT_LEFT_45  	6
#define IR_LEFT  			5


static THD_WORKING_AREA(waDisplacementProcedure, 256);
static THD_FUNCTION(DisplacementProcedure, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

// Finite state machine routine
    while(1)
    	{
    		if(get_frequency_status())
    		{

    			if(check_proximity())
    			{
    				avoid_obstacle();
    			}


    			else
    			{
    				while( (!check_proximity()) && get_frequency_status() )
    				{
    					go_to_sound(detect_sound_phase_shift());
    				}
    			}
    		}

    		else
    		{
    			right_motor_set_speed(STOP_SPEED);
    			left_motor_set_speed(STOP_SPEED);

    		}

    	}
}

void go_to_sound(float phase_difference)
{
	//applies the speed correction from the PI regulator
	right_motor_set_speed(MOTOR_SPEED_LIMIT/2 + pi_regulator(phase_difference, GOAL_PHASE_DIFFERENCE));
	left_motor_set_speed(MOTOR_SPEED_LIMIT/2 - pi_regulator(phase_difference, GOAL_PHASE_DIFFERENCE));

}

void avoid_obstacle()
{

	int16_t leftSpeed = 0, rightSpeed = 0;

    messagebus_topic_t *prox_topic = messagebus_find_topic_blocking(&bus, "/proximity");
    proximity_msg_t prox_values;

	messagebus_topic_wait(prox_topic, &prox_values, sizeof(prox_values));

	// P regulator
	leftSpeed = MOTOR_SPEED_LIMIT - prox_values.delta[0]*KP_AVOID_NEAR - prox_values.delta[1]*KP_AVOID_FAR;
	rightSpeed = MOTOR_SPEED_LIMIT - prox_values.delta[7]*KP_AVOID_NEAR - prox_values.delta[6]*KP_AVOID_FAR;

	right_motor_set_speed(rightSpeed);
	left_motor_set_speed(leftSpeed);
}

int8_t check_proximity()
{
	if(get_calibrated_prox(IR_FRONT_RIGHT) > TOO_CLOSE || get_calibrated_prox(IR_FRONT_RIGHT_45) > TOO_CLOSE
			|| get_calibrated_prox(IR_FRONT_LEFT) > TOO_CLOSE || get_calibrated_prox(IR_FRONT_LEFT_45) > TOO_CLOSE)
	{
		return 1; // an obstacle has been detected
	}

	else return 0;
}


void displacement_start()
{
	chThdCreateStatic(waDisplacementProcedure, sizeof(waDisplacementProcedure), NORMALPRIO, DisplacementProcedure, NULL);
}
