/*
 * demo_boot.cpp
 *
 * Created: 4/17/2017 11:56:40
 *  Author: Tomasz Jaworski
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <stddef.h>
#include "uart.h"

//
// For Arduino Pro Mini 328 the LED_TOGGLE toggles on-board LED diode.
// If this bootloader is used on other boards, change this line or remove it completely
#define LED_ON do { PORTB |= _BV(PORTB5); } while (0);
#define LED_OFF do { PORTB &= ~_BV(PORTB5); } while (0);
#define LED_TOGGLE do { PORTB ^= _BV(PORTB5); } while (0);
	

void ___boot_demo(void) __attribute__ ((__used__, section (".BL")));

void ___boot_demo(void)
{
	uint8_t arr[3];
	arr[0] = 'A'; arr[1] = 'B'; arr[2] = 'C';
	
	DDRB = 0x00;
	DDRB |= _BV(PORTB5); //	SCK
	
	asm volatile("nop\n");
	asm volatile("nop\n");
	asm volatile("nop\n");

	while(1)
	{
		LED_ON;
#if defined(DEBUG)		
		for (uint32_t j = 0; j < 5000; j++) asm volatile("nop");
		LED_OFF;
		for (uint32_t j = 0; j < 50000; j++) asm volatile("nop");
		LED_ON;
		for (uint32_t j = 0; j < 5000; j++) asm volatile("nop");
		LED_OFF;
		for (uint32_t j = 0; j < 100000; j++) asm volatile("nop");
#else // Release
		for (uint32_t j = 0; j < 3124ULL * F_CPU / 1000000ULL; j++) asm volatile("nop");
		LED_OFF;
		for (uint32_t j = 0; j < 31250ULL * F_CPU / 1000000ULL; j++) asm volatile("nop");
		LED_ON;
		for (uint32_t j = 0; j < 3124ULL * F_CPU / 1000000ULL; j++) asm volatile("nop");
		LED_OFF;
		for (uint32_t j = 0; j < 62500ULL * F_CPU / 1000000ULL; j++) asm volatile("nop");
#endif 
	}

	asm volatile("nop\n");
	asm volatile("nop\n");
	asm volatile("nop\n");
}

