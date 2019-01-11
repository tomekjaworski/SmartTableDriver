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
#include "dbg_putchar.h"
#include "intensity_measurements.h"

uint8_t __cnt;

ISR(TIMER0_COMPA_vect)
{
	if (__cnt++ > 200)
	{
		__cnt = 0;
		LED0_TOGGLE;
	}

	burst.timer++;
	rx.idle_timer++;
}

ISR(USART_TX_vect) // goes off after transmitter have sent one byte (its only byte)
{
	if (tx.window_position != tx.window_end) 
	{
		// send a byte and move forward in outgoing buffer
		UDR0 = *tx.window_position++;
		return;
	}

	if (tx.state == TransmitterState::SendingHeader)
	{
		if (tx.header.payload_length == 0)
			goto __skip_payload;
		tx.state = TransmitterState::SendingPayload;
		tx.window_position = tx.ppayload;
		tx.window_end = tx.window_position + tx.header.payload_length;
		UDR0 = *tx.window_position++;
		return;
	}

	if (tx.state == TransmitterState::SendingPayload)
	{
__skip_payload:
		tx.state = TransmitterState::SendingCRC;
		tx.window_position = (uint8_t*)&tx.crc;
		tx.window_end = tx.window_position + sizeof(tx.crc);
		UDR0 = *tx.window_position++;
		return;
	}

	if (tx.state == TransmitterState::SendingCRC)
	{
		UCSR0B &= ~_BV(TXCIE0); // off
		RS485_DIR_RECEIVE;
		tx.state = TransmitterState::IDLE;
		return;
	}
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
