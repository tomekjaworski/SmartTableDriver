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

static uint16_t __cnt;

ISR(TIMER0_COMPA_vect)
{
	if (trigger_config.trigger1.active) {
		if (trigger_config.trigger1.counter-- <= 0) {
			if (trigger_config.trigger1.state == PinState::Low) {
				trigger_config.trigger1.counter = trigger_config.trigger1.high_interval;
				trigger_config.trigger1.state = PinState::High;
				TRIGGER1_SET_HIGH();
			} else {
				trigger_config.trigger1.counter = trigger_config.trigger1.low_interval;
				trigger_config.trigger1.state = PinState::Low;
				TRIGGER1_SET_LOW();
			}
		}
	}
	
	if (trigger_config.trigger2.active) {
		if (trigger_config.trigger2.counter-- <= 0) {
			if (trigger_config.trigger2.state == PinState::Low) {
				trigger_config.trigger2.counter = trigger_config.trigger2.high_interval;
				trigger_config.trigger2.state = PinState::High;
				TRIGGER2_SET_HIGH();
				} else {
				trigger_config.trigger2.counter = trigger_config.trigger2.low_interval;
				trigger_config.trigger2.state = PinState::Low;
				TRIGGER2_SET_LOW();
			}
		}
	}	
	
	if (__cnt++ > 100) {
		__cnt = 0;
		LED0_TOGGLE;
	}
	//if (trigger_config.trigger1.active)
		//trigger_config.trigger1.counter--;
	//if (trigger_config.trigger2.active)
		//trigger_config.trigger2.counter--;

	rx.idle_timer++;
}
