/*
 * DemoProject.cpp
 *
 * Created: 10/11/2020 18:09:55
 * Author : Tomek
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <stddef.h>
#include <stdlib.h>
//
// For Arduino Pro Mini 328 the LED_TOGGLE toggles on-board LED diode.
// If this bootloader is used on other boards, change this line or remove it completely
#define LED_ON do { PORTB |= _BV(PORTB5); } while (0);
#define LED_OFF do { PORTB &= ~_BV(PORTB5); } while (0);
#define LED_TOGGLE do { PORTB ^= _BV(PORTB5); } while (0);

void sleep(int ms)
{
	while (ms > 0) {
		_delay_ms(1);
		--ms;
	}
}

int main(void)
{
	DDRB = 0x00;
	DDRB |= _BV(DDB5); //	SCK

	_delay_ms(1000);

	int32_t sum;
	uint8_t* ptr = 0;
	for (int i = 0; i < 1024 * 2; i++)
		sum += *ptr++;
		
	srand(sum);
	
	while(1) {
		int off_delay = 50 + rand() % 400;
		
		LED_ON;
		sleep(30);
		LED_OFF;
		sleep(off_delay);
	}
		
}

