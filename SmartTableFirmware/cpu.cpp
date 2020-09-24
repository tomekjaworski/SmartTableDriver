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
#include "config.h"

#define BAUD_UX2

//////////////////////////////////////////////////////////////////////////

#if defined(BAUD_UX2)
// UX2 = 1
#define UBR0_VALUE (F_CPU/(8UL*SERIAL_BAUD))-1
#else
// UX2 = 0
#define UBR0_VALUE (F_CPU/(16UL*SERIAL_BAUD))-1
#endif


/*
		 SCK			PB5
	D10- LED2			PB2
	D9 - LED1			PB1
	D8 - DATA			PB0
	D7 - CLOCK			PD7
	D6 - SYNC_IO		PD6
	D5 - RESET1			PD5
	D4 - RESET2			PD4
	D3 - NC				PD3
	D2 - DIR			PD2
*/


void cpu_init(void)
{
	DDRB = 0x00;
	DDRB |= _BV(PORTB2); // LED2
	DDRB |= _BV(PORTB1); //	LED1
	DDRB |= _BV(PORTB5); //	SCK
	DDRB |= _BV(PORTB0); //	DATA

	DDRD = 0x00;
	DDRD |= _BV(PORTD7); // CLOCK
	DDRD |= _BV(PORTD5); // RESET1
	DDRD |= _BV(PORTD4); // RESET2
	

	DDRD |= _BV(PORTD2); // DIR
	DDRD |= _BV(PORTD1); // TX
	
	PORTD |= _BV(PORTD3); // pull-up dla wej?cia synchronizuj?cego

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
	SET_RECEIVER_INTERRUPT(true);

	// struktury portu szeregowego
	tx.state = TransmitterState::IDLE;


	// Set timer to 1ms
	TCCR0A |= (1 << WGM01); // CTC mode
	OCR0A = (8000000UL / 64UL) / 2000UL - 1;
	TIMSK0 |= (1 << OCIE0A);
	TCCR0B |= (1 << CS00) | (1 << CS01); // clk / 64

	// reset photodiodes selectors
	RESET1_LOW;
	RESET2_LOW;

	// czy ja zyjê????
	for(int i = 0; i < 100; i++)
	{
		LED0_TOGGLE;
		LED1_TOGGLE;
		//LED_TOGGLE;
		_delay_ms(20);

	}

	// RS485 to pozosta?o?c po pierwszej wersji; aby nie modyfikoaw? p?ytki (wylutowywac kostki) 
	// wystarczy prze??czy? j? w tryb nadawania.
	LEGACY_RS485_DIR_OUTPUT();
	LED0_OFF; LED1_OFF;
	_delay_ms(1000);

	RESET1_HIGH;
	RESET2_HIGH;

	// start przerwan
	sei();
}
