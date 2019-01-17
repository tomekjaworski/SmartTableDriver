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

#include "dbg_putchar.h"
#include "crc16.h"
#include "hardware.h"
#include "eeprom_config.h"
#include "comm.h"
#include "config.h"
#include "intensity_measurements.h"

const char* build_date = __DATE__;
const char* build_time = __TIME__;
const char* build_version = "1.0";

void cpu_init(void);
void send(device_address_t addr, MessageType type, const uint8_t* payload, uint8_t payload_length);
bool check_rx(void);

inline static void memmove(volatile void* dst, volatile void* src, size_t size)
{
	memmove((void*)dst, (void*)src, size);
}

#define RX_RESET do { rx.buffer_position = (uint8_t*)&rx.buffer; } while (0);

extern int otable[10][10];
volatile uint8_t dummy;

char b[15 * (7 * (4+1) + 1) + 1];

void uart_putchar(char c) {
	loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
	UDR0 = c;
}

void send_string(const char* s)
{
	while (*s)
		uart_putchar(*s++);
}


void int2hex(char* buffer, int value)
{
	uint8_t a = (value >> 12) & 0xf;
	uint8_t b = (value >> 8) & 0xf;
	uint8_t c = (value >> 4) & 0xf;
	uint8_t d = (value >> 0) & 0xf;
	
	*buffer++ = a > 9 ? 'A' + a - 10 : '0' + a;
	*buffer++ = b > 9 ? 'A' + b - 10 : '0' + b;
	*buffer++ = c > 9 ? 'A' + c - 10 : '0' + c;
	*buffer++ = d > 9 ? 'A' + d - 10 : '0' + d;
}

//extern union im_raw_measurement_t raw;

int main(void)
{
	cpu_init();
	RX_RESET;
	
	im_initialize8();
	device_address = pgm_read_byte(device_address_block + 4);


	// --------------
	// --------------
	// --------------
	// --------------
	
	cli();
	RS485_DIR_SEND;
	send_string("\n");

/*	while (1)
	{
		
		LED0_ON;
		_delay_ms(1000);
		LED0_OFF;
		_delay_ms(1000);
	}
*/

	while(1)
	{	
		_delay_ms(500);
		im_measure8();
		//im_measure8();
	
		b[0] = '\x0';
		char* ptr = b;
	
		send_string("\n");
	
		for (int r = 0; r < 15; r++)
		{
			for (int c = 0; c < 7; c++)
			{
				uint16_t val = im_data.raw8[r * 7 + c] << 2;
				int2hex(ptr, val);
				ptr += 4;
				*ptr++ = val > 0x20 ? '*' : ' ';
			}
			*ptr++ = '\n';
		}
		*ptr = '\x0';
	
	
	
		send_string(b);

	}
	
	while(1);

	
	
	while(1)
	{
		if (!rx.got_data)
		{
			if (rx.idle_timer > SERIAL_IDLE_LIMIT)
			{
				RX_RESET;
				rx.idle_timer = 0;
			}
			continue; // not yet
		}
		
		rx.idle_timer = 0;
		
		if (!check_rx())
			continue; // not yet again

		// ok, we have data
		if (rx.buffer.header.type == MessageType::PingRequest)
		{
			memmove(tx.payload, rx.buffer.payload, rx.buffer.header.payload_length);
			send(rx.buffer.header.address, MessageType::PingResponse, (const uint8_t*)tx.payload, rx.buffer.header.payload_length);
		}

		if (rx.buffer.header.type == MessageType::GetVersionRequest)
		{
			strcpy((char*)tx.payload, "version="); strcat((char*)tx.payload, build_version);
			strcat((char*)tx.payload, ";date="); strcat((char*)tx.payload, build_date);
			strcat((char*)tx.payload, ";time="); strcat((char*)tx.payload, build_time);
			send(rx.buffer.header.address, MessageType::GetVersionResponse, (const uint8_t*)tx.payload, strlen((const char*)tx.payload));
		}

		if (rx.buffer.header.type == MessageType::GetFullResolutionSyncMeasurementRequest)
		{
			//im_full_resolution_synchronized();
			im_execute_sync();
			send(rx.buffer.header.address, MessageType::GetFullResolutionSyncMeasurementResponse, (const uint8_t*)otable, 10*10*sizeof(uint16_t));
		}

		if (rx.buffer.header.type == MessageType::SetBurstConfigurationRequest)
		{
			// rx.buffer.header.payload_length mus be equal to sizeof(struct BURST_CONFIGURATION)
			bool ok = sizeof(struct BURST_CONFIGURATION) == rx.buffer.header.payload_length;
			if (ok) {
				BURST_CONFIGURATION *pburst_config = (BURST_CONFIGURATION *)rx.buffer.payload;
				memcpy((void*)&burst.config, pburst_config, sizeof(BURST_CONFIGURATION));
			}
			tx.payload[0] = ok;
			send(rx.buffer.header.address, MessageType::SetBurstConfigurationResponse, (const uint8_t*)tx.payload, 1);
		}

		if (rx.buffer.header.type == MessageType::DoBurstMeasurementRequest)
		{
			// shut down receiver
			SET_RECEIVER_INTERRUPT(false);

			// do the measurements and get it's time
			ATOMIC_BLOCK(ATOMIC_FORCEON) { burst.timer = 0x0000; }
			im_execute_sync();
			ATOMIC_BLOCK(ATOMIC_FORCEON) {
				burst.stats.last_measure_time = burst.timer;
				burst.stats.count++;
			}


			// wait for precise point in time
			uint16_t timer_copy;
			do {
			     ATOMIC_BLOCK(ATOMIC_FORCEON) { timer_copy = burst.timer; }
			} while (timer_copy < burst.config.time_point); // wait 

			// synchronized send - start async and wait for finish
			send(rx.buffer.header.address, MessageType::DoBurstMeasurementResponse, (const uint8_t*)otable, 10*10*sizeof(uint16_t));
			while (tx.state != TransmitterState::IDLE);

			// Store transmission time
			ATOMIC_BLOCK(ATOMIC_FORCEON) {
				burst.stats.last_transmission_time = burst.timer - burst.config.time_point;
			}
			
			// wait for the rest of silence time
			do {
				ATOMIC_BLOCK(ATOMIC_FORCEON) { timer_copy = burst.timer; }
			} while (timer_copy < burst.config.silence_interval); // wait


			// enable receiver
			dummy = UDR0;
			SET_RECEIVER_INTERRUPT(true);
		}

		if (rx.buffer.header.type == MessageType::GetBurstMeasurementStatisticsRequest)
		{
			memmove(tx.payload, &burst.stats, sizeof(BURST_STATISTICS));
			send(rx.buffer.header.address, MessageType::GetBurstMeasurementStatisticsResponse, (const uint8_t*)tx.payload, sizeof(BURST_STATISTICS));
		}

		RX_RESET;
	}


	while(1);
}


bool check_rx(void)
{
	rx.got_data = 0;

	// zanim cokolwiek zrobimy, w buforze musi by� minimum sizeof(PROTO_HEADER) bajt�w
	if (RX_COUNT < (int)sizeof(PROTO_HEADER))
	{
		// ale je�li nie ma, to spr�buj troch� pom�c szcz�sciu i zrobi� wst�pn� sychronizacj� strumienia danych
		if (RX_COUNT == 0)
			return false; // nie ma czego synchronizowa�

		if (rx.buffer.header.address != ADDRESS_BROADCAST
			//&& !(rx.buffer.header.address >= 0x01 && rx.buffer.header.address <= 0x18)
			//&& !(rx.buffer.header.address >= 0x01 && rx.buffer.header.address <= 0x04)
			//&& rx.buffer.header.address != 0x13
			//&& rx.buffer.header.address != 0x15
			&& rx.buffer.header.address != device_address)
		{
			// remove one byte at the start of the rx buffer
			RX_RESET;
			//memmove((uint8_t*)&rx.buffer + 1, (uint8_t*)&rx.buffer, RX_COUNT - 1);
			//rx.buffer_position--;
		}
		
		// wait for more data
		return false;
	}

	// Ok! We've just received complete header
	
	// CHECK: if address is consistent with message type
	if ((rx.buffer.header.address == ADDRESS_BROADCAST) ^ (((uint8_t)rx.buffer.header.type & (uint8_t)MessageType::__BroadcastFlag) == (uint8_t)MessageType::__BroadcastFlag ))
	{
		// NO: got broadcast address and non-broadcast message (or the other way around)
		RX_RESET;
		return false;
	}
	
	// CHECK: is the message type valid?
	if (((uint8_t)rx.buffer.header.type & ~(uint8_t)MessageType::__BroadcastFlag) < (uint8_t)MessageType::__RequestMinCode || ((uint8_t)rx.buffer.header.type & ~(uint8_t)MessageType::__BroadcastFlag) > (uint8_t)MessageType::__RequestMaxCode)
	{
		// NO: message type is out of range
		RX_RESET;
		return false;
	}

	// CHECK: is payload length less than buffer's capacity
	if (rx.buffer.header.payload_length > RX_PAYLOAD_CAPACITY) {
		//YES: message length is not acceptable
		RX_RESET;
		return false;
	}

	// check if whole message is here
	if (RX_COUNT < (int)(sizeof(PROTO_HEADER) + rx.buffer.header.payload_length + sizeof(uint16_t)))
		return false; // we have received only a part of the message; let us wait for the rest

	// ok, we have received at least whole message; check CRC
	uint16_t calculated_crc = calc_crc16((void*)&rx.buffer, sizeof(PROTO_HEADER) + rx.buffer.header.payload_length);
	uint16_t received_crc = *(uint16_t*)(rx.buffer.payload + rx.buffer.header.payload_length);
	if (calculated_crc != received_crc)
	{
		// checksums does not match
		RX_RESET;
		return false;
	}

	// reset broadcast bit in message type
	rx.buffer.header.type = (MessageType)((uint8_t)rx.buffer.header.type & ~(uint8_t)MessageType::__BroadcastFlag);
	
	// ok - everything seems to good...
	return true;
}

void send(device_address_t addr, MessageType type, const uint8_t* payload, uint8_t payload_length)
{	

	// prepare header
	tx.header.type = type;
	tx.header.payload_length = payload_length;
	tx.ppayload = payload;

	// set sender address
#define SEND_ADDRESS_MODE	3

#if SEND_ADDRESS_MODE == 0
	// set sender address to same as incoming (mind the broadcast!)
	tx.header.address = rx.buffer.header.address;
#elseif SEND_ADDRESS_MODE == 1
	// set sender address always to the configured one
	tx.header.address = device_address;
#else
	// mix mode 0 and 1
	if (rx.buffer.header.address == ADDRESS_BROADCAST)
		tx.header.address = device_address;
	else
		tx.header.address = rx.buffer.header.address;
#endif



	// setup the transmitter
	tx.state = TransmitterState::SendingHeader;
	tx.window_position = (const uint8_t*)&tx.header;
	tx.window_end = tx.window_position + sizeof(PROTO_HEADER);

	// calculate crc16 of the header and user's payload
	tx.crc = calc_crc16((void*)&tx.header, sizeof(PROTO_HEADER), payload, payload_length);

	// let's go!
	//begin_transmission((void*)&tx.buffer, sizeof(PROTO_HEADER) + payload_length + sizeof(uint16_t));

	// nadanie pierwszego bajta uruchamia potok przerwa�
	RS485_DIR_SEND;
	UDR0 = *tx.window_position++;
	UCSR0B |= _BV(TXCIE0); // uruchom przerwania informuj�ce o zako�czeniu nadawania bajta

}
