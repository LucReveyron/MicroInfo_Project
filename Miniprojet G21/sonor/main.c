#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "memory_protection.h"
#include <usbcfg.h>
#include <main.h>
#include <chprintf.h>
#include <motors.h>
#include <audio/microphone.h>
#include <fft.h>
#include "sensors/proximity.h"
#include "sound_detection.h"
#include "displacement.h"

messagebus_t bus;

MUTEX_DECL(bus_lock);

CONDVAR_DECL(bus_condvar);



int main(void)
{
	//========== INITIALISATION =========

    halInit();

    chSysInit();

    //Initializes the Inter Process Communication Bus

    messagebus_init(&bus,&bus_lock,&bus_condvar);

    //start the proximity sensor

    proximity_start();

    //Initializes the motor

    motors_init();

    //Gives time to complete all initializations
    chThdSleepMilliseconds(100);


    //Start microphone thread and sensor

    mic_start(&process_audio_data);

    calibrate_ir();

    //Start displacement thread

    displacement_start();


}



#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}
