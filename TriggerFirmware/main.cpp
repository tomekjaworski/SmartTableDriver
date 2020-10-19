/*
 * TriggerFirmware.cpp
 *
 * Created: 10/18/2020 19:24:53
 * Author : Tomek
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
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
	DDRB |= _BV(PORTB0); //	TRIGGER 1
	DDRB |= _BV(PORTB1); //	TRIGGER 2
	
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
	OCR0A = (F_CPU / 64UL) / 4000UL - 1;
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
	sei();	
	
}


void cpu_reboot(void) {
	// start watchdog and lock so we can reboot
	wdt_enable(WDTO_15MS);
	while(1);
}



inline static void memmove(volatile void* dst, volatile void* src, size_t size) {
	memmove((void*)dst, (void*)src, size);
}




int main(void)
{
	cpu_init();
	comm_reset_receiver();
	configuration_load();


	while(1) {
	
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
			comm_send(MessageType::RebootResponse, (const uint8_t*)tx.payload, 0);
			cpu_reboot();
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

