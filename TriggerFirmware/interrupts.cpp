/*
 * interrupts.cpp
 *
 * Created: 19/10/2020 10:45:45
 *  Author: Tomek Jaworski
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <string.h>

#include "hardware.h"
#include "../TableFirmware/comm.h"
//#include "dbg_putchar.h"
//#include "intensity_measurements.h"

uint16_t __cnt;

ISR(TIMER0_COMPA_vect)
{
	if (__cnt++ > 1000) {
		__cnt = 0;
		LED0_TOGGLE;
	}

	rx.idle_timer++;
}
