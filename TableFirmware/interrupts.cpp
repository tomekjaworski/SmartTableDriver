/*
 * interrupts.cpp
 *
 * Created: 3/27/2017 13:49:46
 *  Author: Tomek
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <string.h>

#include "hardware.h"
#include "comm.h"
#include "dbg_putchar.h"
#include "intensity_measurements.h"

uint8_t __cnt;

ISR(TIMER0_COMPA_vect)
{
	if (__cnt++ > 254) {
		__cnt = 0;
		LED0_TOGGLE;
	}

	rx.idle_timer++;
}
