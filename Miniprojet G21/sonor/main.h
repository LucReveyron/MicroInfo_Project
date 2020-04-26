#ifndef MAIN_H
#define MAIN_H

#ifdef __cplusplus
extern "C" {
#endif


#include "msgbus/messagebus.h"
#include "parameter/parameter.h"

/** Robot wide IPC bus. */
extern messagebus_t bus;

extern parameter_namespace_t parameter_root;


//constants for the differents parts of the project
#define KI_TRACK 				3.5f
#define KP_TRACK 				400.0f
#define KP_AVOID_NEAR 			2.5f
#define KP_AVOID_FAR 			2.0f
#define MAX_SUM_ERROR 			(MOTOR_SPEED_LIMIT/(2*KI_TRACK))
#define ERROR_THRESHOLD 			0.11f
#define GOAL_PHASE_DIFFERENCE	0.0f


/** Robot wide IPC bus. */
extern messagebus_t bus;

extern parameter_namespace_t parameter_root;



#ifdef __cplusplus
}
#endif

#endif
