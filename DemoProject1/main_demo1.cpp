/*
 * DemoProject.cpp
 *
 * Created: 10/11/2020 18:09:55
 * Author : Tomek
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <stddef.h>

//
// For Arduino Pro Mini 328 the LED_TOGGLE toggles on-board LED diode.
// If this bootloader is used on other boards, change this line or remove it completely
#define LED_ON do { PORTB |= _BV(PORTB5); } while (0);
#define LED_OFF do { PORTB &= ~_BV(PORTB5); } while (0);
#define LED_TOGGLE do { PORTB ^= _BV(PORTB5); } while (0);

void blink(void) {
	LED_ON;
	_delay_ms(50);
	LED_OFF;
}
#define A	600
#define B	300
int main(void)
{
	DDRB = 0x00;
	DDRB |= _BV(DDB5); //	SCK

	_delay_ms(1000);

    while (1) 
    {
		blink();
		_delay_ms(A);
		blink();
		_delay_ms(A);


		blink();
		_delay_ms(B);
		blink();
		_delay_ms(B);
		blink();


		_delay_ms(A);


		blink();
		_delay_ms(B);
		blink();
		_delay_ms(B);
		blink();
		_delay_ms(B);
		blink();


		_delay_ms(A);

		blink();
		_delay_ms(B);
		blink();
		_delay_ms(B);


		_delay_ms(3000);
    }
}

