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
			memmove(tx.payload, rx.buffer.payload, rx.buffer.header.payload_length);
			send(rx.buffer.header.address, MessageType::Pong, (const uint8_t*)tx.payload, rx.buffer.header.payload_length);
		}

		if (rx.buffer.header.type == MessageType::GetVersion)
		{
			strcpy((char*)tx.payload, "version="); strcat((char*)tx.payload, build_version);
			strcat((char*)tx.payload, ";date="); strcat((char*)tx.payload, build_date);
			strcat((char*)tx.payload, ";time="); strcat((char*)tx.payload, build_time);
			send(rx.buffer.header.address, MessageType::GetVersion, (const uint8_t*)tx.payload, strlen((const char*)tx.payload));
		}

		if (rx.buffer.header.type == MessageType::GetFullResolutionSyncMeasurement)
		{
			//im_full_resolution_synchronized();
			im_execute_sync();
			send(rx.buffer.header.address, MessageType::GetFullResolutionSyncMeasurement, (const uint8_t*)otable, 10*10*sizeof(uint16_t));
		}

		if (rx.buffer.header.type == MessageType::SetBurstConfiguration)
		{
			// rx.buffer.header.payload_length mus be equal to sizeof(struct BURST_CONFIGURATION)
			bool ok = sizeof(struct BURST_CONFIGURATION) == rx.buffer.header.payload_length;
			if (ok) {
				BURST_CONFIGURATION *pburst_config = (BURST_CONFIGURATION *)rx.buffer.payload;
				memcpy((void*)&burst.config, pburst_config, sizeof(BURST_CONFIGURATION));
			}
			tx.payload[0] = ok;
			send(rx.buffer.header.address, MessageType::SetBurstConfiguration, (const uint8_t*)tx.payload, 1);
		}

		if (rx.buffer.header.type == MessageType::DoBurstMeasurement)
		{
			// shut down receiver and do the measurements
			SET_RECEIVER_INTERRUPT(false);
			burst.timer = 0x0000;
			im_execute_sync();
			burst.stats.last_measure_time = burst.timer;
			burst.stats.count++;

			// wait for precise point in time
			uint16_t timer_copy;
			do {
			     ATOMIC_BLOCK(ATOMIC_FORCEON) { timer_copy = burst.timer; }
			} while (timer_copy < burst.config.time_point); // wait 

			// synchronized send - start async and wait for finish
			send(rx.buffer.header.address, MessageType::GetFullResolutionSyncMeasurement, (const uint8_t*)otable, 10*10*sizeof(uint16_t));
			while (tx.state != TransmitterState::IDLE);
			burst.stats.last_transmission_time = burst.timer - burst.config.time_point;

			// wait for the rest of silence time
			do {
				ATOMIC_BLOCK(ATOMIC_FORCEON) { timer_copy = burst.timer; }
			} while (timer_copy < burst.config.silence_interval); // wait

			// enable receiver
			dummy = UDR0;
			SET_RECEIVER_INTERRUPT(true);
		}

		if (rx.buffer.header.type == MessageType::GetBurstMeasurementStatistics)
		{
			memmove(tx.payload, &burst.stats, sizeof(BURST_STATISTICS));
			send(rx.buffer.header.address, MessageType::GetBurstMeasurementStatistics, (const uint8_t*)tx.payload, sizeof(BURST_STATISTICS));
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

		if (rx.buffer.header.address != ADDRESS_BROADCAST
			&& !(rx.buffer.header.address >= 0x01 && rx.buffer.header.address <= 0x18)
			&& !(rx.buffer.header.address >= 0x01 && rx.buffer.header.address <= 0x04)
			&& rx.buffer.header.address != 0x13
			&& rx.buffer.header.address != 0x15
			&& rx.buffer.header.address != DEVICE_ADDRESS)
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
	tx.header.address = addr;
	tx.header.type = type;
	tx.header.payload_length = payload_length;
	tx.ppayload = payload;

	// setup the transmitter
	tx.state = TransmitterState::SendingHeader;
	tx.window_position = (const uint8_t*)&tx.header;
	tx.window_end = tx.window_position + sizeof(PROTO_HEADER);

	// calculate crc16 of the header and user's payload
	tx.crc = calc_crc16((void*)&tx.header, sizeof(PROTO_HEADER), payload, payload_length);

	// let's go!
	//begin_transmission((void*)&tx.buffer, sizeof(PROTO_HEADER) + payload_length + sizeof(uint16_t));

	// nadanie pierwszego bajta uruchamia potok przerwañ
	RS485_DIR_SEND;
	UDR0 = *tx.window_position++;
	UCSR0B |= _BV(TXCIE0); // uruchom przerwania informuj¹ce o zakoñczeniu nadawania bajta

}
