/*
 * TriggerFirmware.cpp
 *
 * Created: 10/18/2020 19:24:53
 * Author : Tomek
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "hardware.h"
#include "..\TableFirmware\protocol.h"
#include "..\TableFirmware\comm.h"
#include "..\TableFirmware\eeprom_config.h"
#include "cpu.h"

struct TriggerGeneratorConfig trigger_config = { 0 };


inline static void memmove(volatile void* dst, volatile void* src, size_t size) {
	memmove((void*)dst, (void*)src, size);
}




int main(void)
{
	cpu_init();
	comm_reset_receiver();
	configuration_load();
	sei();

	// trigger
	trigger_config.trigger1.active = true;
	trigger_config.trigger1.state = PinState::Low;
	trigger_config.trigger1.low_interval = 1000;
	trigger_config.trigger1.high_interval = 500;
	trigger_config.trigger1.echo.delay = 250;


	trigger_config.trigger2.active = false;
	trigger_config.trigger2.state = PinState::Low;
	trigger_config.trigger2.low_interval = 700;
	trigger_config.trigger2.high_interval = 35;
	
	while(1) {
		
		if (trigger_config.trigger1.echo.transmission_pending) {
			trigger_config.trigger1.echo.transmission_pending = false;
			
			// is this the proper way?
			UDR0 = 'T';
		}
	
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
			char* ptr = (char*)tx.payload;
			sprintf(ptr, "id=%d;version=%s;date=%s;time=%s", device_identifier, FIRMWARE_VERSION, FIRMWARE_BUILD_DATE, FIRMWARE_BUILD_TIME);
			comm_send(MessageType::DeviceIdentifierResponse, (const uint8_t*)ptr, strlen((const char*)ptr));
		}
	
	
		if (rx.buffer.header.type == MessageType::RebootRequest) {
			comm_send(MessageType::RebootResponse, NULL, 0);
			cpu_reboot();
		}


		if (rx.buffer.header.type == MessageType::SetTriggerStateRequest) {
			
			volatile TriggerStatePayload* p = reinterpret_cast<volatile TriggerStatePayload*>(rx.buffer.payload);

			if (p->trigger1 == TriggerStateSetMode::SetHigh)
				TRIGGER1_SET_HIGH();
			if (p->trigger1 == TriggerStateSetMode::SetLow)
				TRIGGER1_SET_LOW();
		
			if (p->trigger2 == TriggerStateSetMode::SetHigh)
				TRIGGER2_SET_HIGH();
			if (p->trigger2 == TriggerStateSetMode::SetLow)
				TRIGGER2_SET_LOW();
		
			comm_send(MessageType::SetTriggerStateResponse, NULL, 0);
		}
		
		if (rx.buffer.header.type == MessageType::SetTriggerGeneratorRequest) {
			volatile TriggerGeneratorPayload* p = reinterpret_cast<volatile TriggerGeneratorPayload*>(rx.buffer.payload);

			// turn off trigger 1?
			if (p->trigger1.mode == TriggerGeneratorSetMode::TurnOff) {
				TRIGGER1_SET_LOW();
				trigger_config.trigger1.active = false;
			}
			
			// turn off trigger 2?
			if (p->trigger2.mode == TriggerGeneratorSetMode::TurnOff) {
				TRIGGER2_SET_LOW();
				trigger_config.trigger2.active = false;
			}
			
			//
			// Setup trigger 1?
			if (p->trigger1.mode == TriggerGeneratorSetMode::SetAndRun) {
				cli();
				TRIGGER1_SET_LOW();
				trigger_config.trigger1.state = PinState::Low;
				trigger_config.trigger1.low_interval = p->trigger1.low_interval;
				trigger_config.trigger1.high_interval = p->trigger1.high_interval;
				trigger_config.trigger1.state_counter = p->trigger1.low_interval;
				trigger_config.trigger1.active = true;
				
				trigger_config.trigger1.echo.active = false;
				trigger_config.trigger1.echo.delay = p->trigger1.echo_delay;

				sei();
			}

			//
			// Setup trigger 2?
			if (p->trigger2.mode == TriggerGeneratorSetMode::SetAndRun) {
				cli();
				TRIGGER2_SET_LOW();
				trigger_config.trigger2.state = PinState::Low;
				trigger_config.trigger2.low_interval = p->trigger2.low_interval;
				trigger_config.trigger2.high_interval = p->trigger2.high_interval;
				trigger_config.trigger2.state_counter = p->trigger2.low_interval;
				trigger_config.trigger2.active = true;
				sei();
			}
			
			comm_send(MessageType::SetTriggerGeneratorResponse, NULL, 0);
		}

/*
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
			bool ok = true;
			int8_t new_trigger_data_size = -1;

			// 1
			if (rx.buffer.header.payload_length != 1)
			ok = false;
		
			// 2
			if (ok) {
				new_trigger_data_size = *(const int8_t*)rx.buffer.payload;
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
	 */
	
		comm_reset_receiver();
	}	
}

