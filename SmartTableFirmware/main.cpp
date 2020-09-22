/*
 * SmartTableFirmware.c
 *
 * Created: 24/03/2017 16:30:33
 * Author : Tomasz Jaworski
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <util/atomic.h>
#include <stdio.h>

#include "dbg_putchar.h"
#include "checksum.h"
#include "hardware.h"
#include "eeprom_config.h"
#include "comm.h"
#include "config.h"
#include "intensity_measurements.h"

void cpu_init(void);
void send(MessageType type, const void* payload, uint8_t payload_length);
bool check_rx(void);

inline static void memmove(volatile void* dst, volatile void* src, size_t size)
{
	memmove((void*)dst, (void*)src, size);
}

#define RX_RESET do { rx.buffer_position = (uint8_t*)&rx.buffer; } while (0);

volatile uint8_t dummy;

void uart_putchar(char c) {
	loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
	UDR0 = c;
}

void send_string(const char* s) {
	while (*s)
		uart_putchar(*s++);
}

int main(void)
{
	cpu_init();
	RX_RESET;
	
	im_initialize8();
#if DEBUG	
	send_string("%%TEST%%");
#endif

#if 0
	while (1) {
		LED0_ON;
		_delay_ms(1000);
		LED0_OFF;
		_delay_ms(1000);
	}
#endif
	
	bool prev_trigger = GET_TRIGGER();
	int8_t trigger_data_size = 0;

	while(1) {
		bool current_trigger = GET_TRIGGER();
		if (current_trigger && !prev_trigger) {
			LED1_TOGGLE;
			if (trigger_data_size == 8) {
				im_measure8();
				send(MessageType::SingleMeasurement8Response, im_data.raw8, 10 * 10 * sizeof(uint8_t));
			}
			if (trigger_data_size == 10) {
				im_measure10();
				send(MessageType::SingleMeasurement10Response, im_data.raw16, 10 * 10 * sizeof(uint16_t));
			}
		}
		
		if (!rx.got_data) {
			if (rx.idle_timer > SERIAL_IDLE_LIMIT) {
				RX_RESET;
				rx.idle_timer = 0;
			}
			continue; // not yet
		}
		
		rx.idle_timer = 0;
		
		if (!check_rx())
			continue; // not yet again

		// ok, we have data
		if (rx.buffer.header.type == MessageType::PingRequest) {
			memmove(tx.payload, rx.buffer.payload, rx.buffer.header.payload_length);
			send(MessageType::PingResponse, (const uint8_t*)tx.payload, rx.buffer.header.payload_length);
		}

		if (rx.buffer.header.type == MessageType::DeviceIdentifierRequest) {
			char* ptr = (char*)tx.payload;
			sprintf(ptr, "id=%d;version=%s;date=%s;time=%s", DEVICE_IDENTIFIER, FIRMWARE_VERSION, FIRMWARE_BUILD_DATE, FIRMWARE_BUILD_TIME);
			send(MessageType::DeviceIdentifierResponse, (const uint8_t*)ptr, strlen((const char*)ptr));
		}

		//if (rx.buffer.header.type == MessageType::GetFullResolutionSyncMeasurementRequest)
		//{
			////im_full_resolution_synchronized();
			//im_execute_sync();
			//send(rx.buffer.header.address, MessageType::GetFullResolutionSyncMeasurementResponse, (const uint8_t*)otable, 10*10*sizeof(uint16_t));
		//}



		if (rx.buffer.header.type == MessageType::SingleMeasurement8Request) {
			//im_full_resolution_synchronized();
			im_measure8();
			send(MessageType::SingleMeasurement8Response, im_data.raw8, 10*10*sizeof(uint8_t));
		}

		if (rx.buffer.header.type == MessageType::SingleMeasurement10Request) {
			//im_full_resolution_synchronized();
			im_measure10();
			send(MessageType::SingleMeasurement10Response, im_data.raw16, 10*10*sizeof(uint16_t));
		}

		if (rx.buffer.header.type == MessageType::TriggeredMeasurementEnterRequest) {
			bool ok = true;
			int8_t new_data_size = -1;

			// 1
			if (rx.buffer.header.payload_length != 1)
				ok = false;
			
			// 2
			if (ok) {
				new_data_size = *(const int8_t*)rx.buffer.payload;
				if (new_data_size < 1 || new_data_size > 10)
					ok = false;
			}
			
			// 3
			if (ok) {
				trigger_data_size = new_data_size;
				send(MessageType::TriggeredMeasurementEnterResponse, &trigger_data_size, 1);
			} else {
				int8_t response = -1;
				send(MessageType::TriggeredMeasurementEnterResponse, &response, 1);
			}
		}

		if (rx.buffer.header.type == MessageType::TriggeredMeasurementLeaveRequest) {
			trigger_data_size = 0;
			send(MessageType::TriggeredMeasurementLeaveResponse, NULL, 0);
		}
		
		RX_RESET;
	}


	while(1);
}


bool check_rx(void)
{
	rx.got_data = 0;

	// zanim cokolwiek zrobimy, w buforze musi byæ minimum sizeof(PROTO_HEADER) bajtów
	if (RX_COUNT < (int)sizeof(PROTO_HEADER))
	{
		// ale jeœli nie ma, to spróbuj trochê pomóc szczêsciu i zrobiæ wstêpn¹ sychronizacjê strumienia danych
		if (RX_COUNT == 0)
			return false; // nie ma czego synchronizowaæ

		if (rx.buffer.header.magic != PROTO_MAGIC)
		{
			// remove one byte at the start of the rx buffer
			RX_RESET;
		}
		
		// wait for more data
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
	if (RX_COUNT < (int)(sizeof(PROTO_HEADER) + rx.buffer.header.payload_length + sizeof(checksum_t)))
		return false; // we have received only a part of the message; let us wait for the rest

	// ok, we have received at least whole message; check CRC
	checksum_t calculated_crc = calc_checksum((void*)&rx.buffer, sizeof(PROTO_HEADER) + rx.buffer.header.payload_length);
	checksum_t received_crc = *(checksum_t*)(rx.buffer.payload + rx.buffer.header.payload_length);
	if (false && (calculated_crc != received_crc))
	{
		// checksums does not match
		RX_RESET;
		return false;
	}

	// ok - everything seems to good...
	return true;
}

void send(MessageType type, const void* payload, uint8_t payload_length)
{	

	// prepare header
	tx.header.type = type;
	tx.header.payload_length = payload_length;
	tx.ppayload = (uint8_t*)payload;
	tx.header.magic = PROTO_MAGIC;

	// setup the transmitter
	tx.state = TransmitterState::SendingHeader;
	tx.window_position = (const uint8_t*)&tx.header;
	tx.window_end = tx.window_position + sizeof(PROTO_HEADER);

	// calculate crc16 of the header and user's payload
	tx.crc = calc_checksum((void*)&tx.header, sizeof(PROTO_HEADER), payload, payload_length);

	// nadanie pierwszego bajta uruchamia potok przerwañ
	//RS485_DIR_SEND;
	UDR0 = *tx.window_position++;
	UCSR0B |= _BV(TXCIE0); // uruchom przerwania informuj¹ce o zakoñczeniu nadawania bajta

}
