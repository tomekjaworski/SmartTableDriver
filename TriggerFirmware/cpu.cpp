/*
 * cpu.cpp
 *
 * Created: 10/19/2020 19:32:08
 *  Author: Tomek
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "hardware.h"
#include "..\TableFirmware\protocol.h"
#include "..\TableFirmware\comm.h"
#include "..\TableFirmware\eeprom_config.h"

#if defined(BAUD_UX2)
// UX2 = 1
#define UBR0_VALUE (F_CPU/(8UL*SERIAL_BAUD))-1
#else
// UX2 = 0
#define UBR0_VALUE (F_CPU/(16UL*SERIAL_BAUD))-1
#endif




void cpu_init(void) {
	
	DDRB = 0x00;
	DDRB |= _BV(PORTB5); //	LED
	DDRB |= _BV(PORTB0); //	TRIGGER 1 (Arduino D8)
	DDRB |= _BV(PORTB1); //	TRIGGER 2 (Arduino D9)
	
	DDRD = 0x00;

	// configure serial port
	uint16_t br = UBR0_VALUE;
	UBRR0H = (uint8_t)(br >> 8);
	UBRR0L = (uint8_t)br;

	#if defined(BAUD_UX2)
	UCSR0A = _BV(U2X0);
	#else
	UCSR0A = 0x00;
	#endif
	//
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00) | _BV(UPM01);	// 8E1
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);	// wlacz rx i txs
	UCSR0B |= (1 << RXCIE0);
	SET_RECEIVER_INTERRUPT(true);


	// Set timer to 250us
	TCCR0A |= (1 << WGM01); // CTC mode
	OCR0A = (F_CPU / 64UL) / 1000UL - 1;
	TIMSK0 |= (1 << OCIE0A);
	TCCR0B |= (1 << CS01) | (1 << CS00); // clk / 64

	// Turn off watchdog
	MCUSR = 0x00;
	WDTCSR = 0x00;
	wdt_disable();

	// reset photodiodes selectors
	//RESET1_LOW;
	//RESET2_LOW;

	// I'm I alive?
	for(int i = 0; i < 100; i++)
	{
		LED0_TOGGLE;
		//LED1_TOGGLE;
		//LED_TOGGLE;
		_delay_ms(20);

	}

	// RS485 to pozosta?o?c po pierwszej wersji; aby nie modyfikoaw? p?ytki (wylutowywac kostki)
	// wystarczy prze??czy? j? w tryb nadawania.
	LEGACY_RS485_DIR_OUTPUT();
	//LED0_OFF; LED1_OFF;
	_delay_ms(1000);

	////RESET1_HIGH;
	//RESET2_HIGH;

	// start przerwan
	//sei();
	
}

void cpu_reboot(void) {
	// start watchdog and lock so we can reboot
	wdt_enable(WDTO_15MS);
	while(1);
}

