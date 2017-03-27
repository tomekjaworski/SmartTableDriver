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
 #include "eeprom_config.h"
 #include "comm.h"
 
 ISR(TIMER0_COMPA_vect)
 {
	 LED0_TOGGLE;
 }

 ISR(USART_TX_vect) // goes off after transmitter have sent one byte (its only byte)
 {
	 if (tx.buffer_position == tx.buffer_end) // if at the end of the outgoing buffer then turn of transmitter
	 {
		 UCSR0B &= ~_BV(TXCIE0); // off
		 RS485_DIR_RECEIVE;
		 tx.done = 1;
		 return;
	 }

	 // send a byte and move forward in outgoing buffer
	 UDR0 = *tx.buffer_position++;
 }

 ISR(USART_RX_vect)
 {
	 __attribute__((unused)) uint8_t status = UCSR0A;
	 __attribute__((unused)) uint8_t data = UDR0;

	 if (status & (_BV(FE0) | _BV(DOR0) | _BV(UPE0))) // errors: frame error, data overrun, parity error
	 return;

	 if (RX_COUNT == RX_PAYLOAD_CAPACITY + sizeof(PROTO_HEADER)) // buffer overflow
		return;

	 *rx.buffer_position++ = data;
	 rx.got_data = 1;
 }