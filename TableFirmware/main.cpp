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
#include "comm.h"
#include "config.h"
#include "intensity_measurements.h"
#include "eeprom_config.h"
#include "cpu.h"


inline static void memmove(volatile void* dst, volatile void* src, size_t size)
{
	memmove((void*)dst, (void*)src, size);
}


volatile uint8_t dummy;

void uart_putchar(char c) {
	loop_until_bit_is_set(UCSR0A, UDRE0); // Wait until data register empty.
	UDR0 = c;
}

void send_string(const char* s) {
	while (*s)
		uart_putchar(*s++);
}

int main(void)
{
	cpu_init();
	configuration_load();
	comm_reset_receiver();
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
	
	bool prev_trigger = GET_TRIGGER1();
	int8_t trigger_data_size = 0;

	while(1) {
		bool current_trigger = GET_TRIGGER1();
		if (current_trigger && !prev_trigger) {
			LED1_TOGGLE;
			if (trigger_data_size == 8) {
				im_measure8();
				comm_send(MessageType::SingleMeasurement8Response, im_data.primary.raw8, 10 * 10 * sizeof(uint8_t));
			}
			if (trigger_data_size == 10) {
				im_measure10();
				comm_send(MessageType::SingleMeasurement10Response, im_data.primary.raw16, 10 * 10 * sizeof(uint16_t));
			}
		}
		prev_trigger = current_trigger;
		
		if (!rx.got_data) {
			if (rx.idle_timer > SERIAL_IDLE_LIMIT) {
				comm_reset_receiver();
				rx.idle_timer = 0;
			}
			continue; // not yet
		}
		
		rx.idle_timer = 0;
		
		if (!comm_check_receiver())
			continue; // not yet again

		// ok, we have data
		if (rx.buffer.header.type == MessageType::PingRequest) {
			memmove(tx.payload, rx.buffer.payload, rx.buffer.header.payload_length);
			comm_send(MessageType::PingResponse, (const uint8_t*)tx.payload, rx.buffer.header.payload_length);
		}

		if (rx.buffer.header.type == MessageType::DeviceIdentifierRequest) {
			char* ptr = const_cast<char*>(reinterpret_cast<volatile char*>(tx.payload));
			sprintf(ptr, "id=%d;version=%s;date=%s;time=%s", device_identifier, FIRMWARE_VERSION, FIRMWARE_BUILD_DATE, FIRMWARE_BUILD_TIME);
			comm_send(MessageType::DeviceIdentifierResponse, (const uint8_t*)ptr, strlen((const char*)ptr));
		}

	
		if (rx.buffer.header.type == MessageType::RebootRequest) {
			comm_send(MessageType::RebootResponse, (const uint8_t*)tx.payload, 0);
			cpu_reboot();
		}
		


		if (rx.buffer.header.type == MessageType::SingleMeasurement8Request) {
			//im_full_resolution_synchronized();
			im_measure8();
			comm_send(MessageType::SingleMeasurement8Response, im_data.primary.raw8, 10*10*sizeof(uint8_t));
		}

		if (rx.buffer.header.type == MessageType::SingleMeasurement10Request) {
			//im_full_resolution_synchronized();
			im_measure10();
			comm_send(MessageType::SingleMeasurement10Response, im_data.primary.raw16, 10*10*sizeof(uint16_t));
		}

		if (rx.buffer.header.type == MessageType::TriggeredMeasurementEnterRequest) {
			volatile TriggeredMeasurementEnterPayload* p = reinterpret_cast<volatile TriggeredMeasurementEnterPayload*>(rx.buffer.payload);
			
			bool ok = true;
			int8_t new_trigger_data_size = -1;

			// 1
			if (rx.buffer.header.payload_length != 1)
				ok = false;
			
			// 2
			if (ok) {
				new_trigger_data_size = p->data_size;
				if (new_trigger_data_size < 1 || new_trigger_data_size > 10)
					ok = false;
			}
			
			// 3
			if (ok) {
				if (trigger_data_size != new_trigger_data_size) {
					if (new_trigger_data_size == 8)
						im_initialize8();
					if (new_trigger_data_size == 10)
						im_initialize10();
				}
				trigger_data_size = new_trigger_data_size;
				comm_send(MessageType::TriggeredMeasurementEnterResponse, &trigger_data_size, 1);
			} else {
				int8_t response = -1;
				comm_send(MessageType::TriggeredMeasurementEnterResponse, &response, 1);
			}
		}

		if (rx.buffer.header.type == MessageType::TriggeredMeasurementLeaveRequest) {
			trigger_data_size = 0;
			comm_send(MessageType::TriggeredMeasurementLeaveResponse, NULL, 0);
		}
		
		comm_reset_receiver();
	}


	while(1);
}
