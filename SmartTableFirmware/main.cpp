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

#include "dbg_putchar.h"
#include "crc16.h"
#include "hardware.h"
#include "eeprom_config.h"
#include "comm.h"
#include "intensity_measurements.h"

const char* build_date = __DATE__;
const char* build_time = __TIMESTAMP__;
const char* build_version = "1.0";

// data size asserts
static_assert(sizeof(enum MessageType) == 1, "MessageType has invalid size");
static_assert(sizeof(PROTO_HEADER) == 3, "PROTO_HEADER has invalid size");

void cpu_init(void);
void begin_transmission(const void* data, uint8_t count);
void send(MessageType type, uint8_t payload_length);
bool check_rx(void);

inline static void memmove(volatile void* dst, volatile void* src, size_t size)
{
	memmove((void*)dst, (void*)src, size);
}

#define RX_RESET do { rx.buffer_position = (volatile uint8_t*)&rx.buffer; } while (0);

//const char* test = "Ala ma kota!";
const char* test = "A";

int main(void)
{
	cpu_init();
	RX_RESET;
	
	im_initialize();
	_delay_ms(1000);

//	while(1)
//	{
	//	begin_transmission(test, strlen(test));
	//	_delay_ms(200);
//	}


	//configuration_load();
	//im_execute_sync();



	while(1)
	{
		if (!rx.got_data)
			continue; // not yet
		
		if (!check_rx())
			continue; // not yet again

		// ok, we have data
		if (rx.buffer.header.type == MessageType::Ping)
		{
			memmove(tx.buffer.payload, rx.buffer.payload, rx.buffer.header.payload_length);
			send(MessageType::Pong, rx.buffer.header.payload_length);
			continue;
		}

		if (rx.buffer.header.type == MessageType::GetVersion)
		{
			strcpy((char*)tx.buffer.payload, "version="); strcat((char*)tx.buffer.payload, build_version);
			strcpy((char*)tx.buffer.payload, ";date="); strcat((char*)tx.buffer.payload, build_date);
			strcpy((char*)tx.buffer.payload, ";version="); strcat((char*)tx.buffer.payload, build_time);
			continue;
		}

		if (rx.buffer.header.type == MessageType::StartFullMeasurement)
		{
			// 
		}
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

		if (rx.buffer.header.address != ADDRESS_BROADCAST && rx.buffer.header.address != DEVICE_ADDRESS /*configuration.address*/)
		{
			// remove one byte at the start of the rx buffer
			memmove((uint8_t*)&rx.buffer + 1, (uint8_t*)&rx.buffer, RX_COUNT - 1);
			rx.buffer_position--;
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
	if (((uint8_t)rx.buffer.header.type & ~(uint8_t)MessageType::__BroadcastFlag) < (uint8_t)MessageType::__MIN || ((uint8_t)rx.buffer.header.type & ~(uint8_t)MessageType::__BroadcastFlag) > (uint8_t)MessageType::__MAX)
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
	if (calculated_crc != received_crc && false)
	{
		// checksums does not match
		RX_RESET;
		return false;
	}

	// ok - everything seems to good...

	return true;
}

void send(MessageType type, uint8_t payload_length)
{	
	// prepare header
	tx.buffer.header.address = DEVICE_ADDRESS;
	tx.buffer.header.type = type;
	tx.buffer.header.payload_length = payload_length;

	// calculate crc16 and attach it at the end of the payload
	uint16_t crc = calc_crc16((void*)&tx.buffer, payload_length + sizeof(PROTO_HEADER));
	*(uint16_t*)(tx.buffer.payload + payload_length) = crc;

	// let's go!
	begin_transmission((void*)&tx.buffer, sizeof(PROTO_HEADER) + payload_length + sizeof(uint16_t));
}


void begin_transmission(const void* data, uint8_t count)
{
	// przygotowanie mechanizmu transmisyjnego
	tx.buffer_start = (const uint8_t*)data;
	tx.buffer_position = tx.buffer_start;
	tx.buffer_end = tx.buffer_start + count;

	// nadanie pierwszego bajta uruchamia potok przerwañ
	RS485_DIR_SEND;
	UDR0 = *tx.buffer_position++;
	UCSR0B |= _BV(TXCIE0); // uruchom przerwania informuj¹ce o zakoñczeniu nadawania bajta
	tx.done = 0;
}