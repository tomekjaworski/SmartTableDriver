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
	if (trigger_config.trigger1.is_active) {
		// run state transition counter
		if (trigger_config.trigger1.state_counter-- <= 0) {
			if (trigger_config.trigger1.state == PinState::Low) {
				
				// switch internal state
				trigger_config.trigger1.state_counter = trigger_config.trigger1.high_interval;
				trigger_config.trigger1.state = PinState::High;
				
				// trigger photomodules
				TRIGGER1_SET_HIGH();
				
				// start echo counter
				trigger_config.trigger1.echo.counter = trigger_config.trigger1.echo.delay;
				trigger_config.trigger1.echo.is_active = true;
				
			} else {
				trigger_config.trigger1.state_counter = trigger_config.trigger1.low_interval;
				trigger_config.trigger1.state = PinState::Low;
				TRIGGER1_SET_LOW();
				
				if (trigger_config.trigger1.is_single_shot) {
					trigger_config.trigger1.is_active = false;
					trigger_config.trigger1.is_single_shot = false;
				}
			}
		}
		
		// Run trigger echo counter
		if (trigger_config.trigger1.echo.is_active)
		{
			trigger_config.trigger1.echo.counter--;
			if (trigger_config.trigger1.echo.counter == 0) {
				trigger_config.trigger1.echo.is_active = false;
				//trigger_config.trigger1.echo.transmission_pending = true;
				UDR0 = 'T';
			}
		}
	} // trigger1.active

#if defined(USE_TRIGGER_2)
	if (trigger_config.trigger2.is_active) {
		if (trigger_config.trigger2.state_counter-- <= 0) {
			if (trigger_config.trigger2.state == PinState::Low) {
				trigger_config.trigger2.state_counter = trigger_config.trigger2.high_interval;
				trigger_config.trigger2.state = PinState::High;
				TRIGGER2_SET_HIGH();
			} else {
				trigger_config.trigger2.state_counter = trigger_config.trigger2.low_interval;
				trigger_config.trigger2.state = PinState::Low;
				TRIGGER2_SET_LOW();
			}
		}
	}	
#endif
	
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
