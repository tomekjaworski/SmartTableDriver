/*
 * comm.cpp
 *
 * Created: 3/27/2017 13:50:27
 *  Author: Tomek
 */ 


 #include "comm.h"
 #include "config.h"

 volatile TX tx = {};
 volatile RX rx = {};

#define RX_COUNT (rx.buffer_position - (volatile uint8_t*)&rx.buffer)
#define RX_RESET do { rx.buffer_position = (uint8_t*)&rx.buffer; } while (0);

void comm_reset_receiver(void) {
	RX_RESET;
}

void comm_initialize(void) {
	
	
	tx.header.magic = PROTOCOL_HEADER_VALUE;
	tx.header.device_id = device_identifier;
}

bool comm_check_receiver(void)
{
	rx.got_data = 0;

	// First the receive buffer has to have at least sizeof(RX_PROTO_HEADER) bytes
	if (RX_COUNT < (int)sizeof(RX_PROTO_HEADER))
	{
		// If there is not enough data then purge the receiver
		if (RX_COUNT == 0)
		return false; // No data - no worry...

		if (rx.buffer.header.magic != PROTOCOL_HEADER_VALUE)
		{
			// Remove one byte at the start of the rx buffer
			RX_RESET;
		}
		
		// Wait for more data
		return false;
	}

	// Ok! We've just received complete header
	
	// CHECK: is payload length less than buffer's capacity
	if (rx.buffer.header.payload_length > RX_PAYLOAD_CAPACITY) {
		//YES: message length is not acceptable
		RX_RESET;
		return false;
	}

	// check if whole message is here
	if (RX_COUNT < (int)(sizeof(RX_PROTO_HEADER) + rx.buffer.header.payload_length + sizeof(checksum_t)))
	return false; // we have received only a part of the message; let us wait for the rest

	// ok, we have received at least whole message; check CRC
	checksum_t calculated_crc = calc_checksum((void*)&rx.buffer, sizeof(RX_PROTO_HEADER) + rx.buffer.header.payload_length);
	checksum_t received_crc = *(checksum_t*)(rx.buffer.payload + rx.buffer.header.payload_length);
	if (calculated_crc != received_crc)
	{
		// checksums does not match
		RX_RESET;
		return false;
	}

	// ok - everything seems to good...
	return true;
}

void comm_send(MessageType type, const void* payload, uint8_t payload_length)
{

	// prepare header
	tx.header.type = type;
	tx.header.payload_length = payload_length;
	tx.ppayload = (uint8_t*)payload;
	tx.header.sequence_counter++;

	// setup the transmitter
	tx.state = TransmitterState::SendingHeader;
	tx.window_position = (const uint8_t*)&tx.header;
	tx.window_end = tx.window_position + sizeof(TX_PROTO_HEADER);

	// calculate crc16 of the header and user's payload
	tx.crc = calc_checksum((void*)&tx.header, sizeof(TX_PROTO_HEADER), payload, payload_length);

	// nadanie pierwszego bajta uruchamia potok przerwañ
	UDR0 = *tx.window_position++;
	UCSR0B |= _BV(TXCIE0); // uruchom przerwania informuj¹ce o zakoñczeniu nadawania bajta

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
		tx.window_end = tx.window_position + sizeof(checksum_t);
		UDR0 = *tx.window_position++;
		return;
	}

	if (tx.state == TransmitterState::SendingCRC)
	{
		UCSR0B &= ~_BV(TXCIE0); // off
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

	if (RX_COUNT == RX_PAYLOAD_CAPACITY + sizeof(RX_PROTO_HEADER)) // buffer overflow
	return;

	*rx.buffer_position++ = data;
	rx.got_data = 1;
}


