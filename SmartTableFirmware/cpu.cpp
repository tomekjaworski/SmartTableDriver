/*
 * cpu.cpp
 *
 * Created: 3/27/2017 14:39:22
 *  Author: Tomek
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "hardware.h"
#include "comm.h"

#define BAUD 19200	// 8E1 (!!!)
#define BAUD_UX2

//////////////////////////////////////////////////////////////////////////

#if defined(BAUD_UX2)
// UX2 = 1
#define UBR0_VALUE (F_CPU/(8UL*BAUD))-1
#else
// UX2 = 0
#define UBR0_VALUE (F_CPU/(16UL*BAUD))-1
#endif

#define TIMER0_1MS_RELOAD 10

void cpu_init(void)
{
	DDRB |= _BV(PORTB2);
	DDRB |= _BV(PORTB1);
	DDRB |= _BV(PORTB5);
	DDRD = 0b00000110; // 0:RX, 1:TX, 3:DIR			1-wyjscie (1 i 3) ddr - data direction

	// port szergowy
	uint16_t br = UBR0_VALUE;
	UBRR0H = (uint8_t)(br >> 8);
	UBRR0L = (uint8_t)br;
	
	#if defined(BAUD_UX2)
	UCSR0A = _BV(U2X0);
	#else
	UCSR0A = 0x00;
	#endif
	
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00) | _BV(UPM01);	// 8E1
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);	// wlacz rx i txs
	UCSR0B |= (1 << RXCIE0);

	// struktury portu szeregowego
	tx.done = 1;


	// ustaw timer 0
	TCCR0A |= (1 << WGM01); // tryb CTC
	OCR0A = 0xF9;
	TIMSK0 |= (1 << OCIE0A);
	TCCR0B |= (1 << CS02) | (1<<CS00);

	// czy ja zyjê????
	for(int i = 0; i < 100; i++)
	{
		LED0_TOGGLE;
		LED1_TOGGLE;
		LED_TOGGLE;
		_delay_ms(20);
	}

	LED0_OFF; LED1_OFF; LED_OFF;
	_delay_ms(1000);

	// start przerwan
	sei();
}
