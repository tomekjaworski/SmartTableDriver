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

#include "crc16.h"
#include "hardware.h"
#include "eeprom_config.h"
#include "comm.h"


// testy na czas kompilacji
STATIC_ASSERT(sizeof(enum MessageType) == 1, message_type_ma_zly_rozmiar);
STATIC_ASSERT(sizeof(PROTO_HEADER) == 4, proto_header_ma_zly_rozmiar);


void cpu_init(void);
void begin_transmission(const void* data, uint8_t count);
bool check_rx(void);



const char* test = "Ala ma kota!";

int main(void)
{
	cpu_init();
	configuration_load();

	while(1)
	{
		if (!rx.got_data)
			continue; // not yet

		
		if (!check_rx())
			continue; // not yet again

		// ok, we have data
		
	}

    while (1) 
    {
		begin_transmission(test, 12);
		_delay_ms(100);
    }
}
#define RX_RESET do { rx.buffer_position = rx.buffer; } while (0);


bool check_rx(void)
{
	rx.got_data = 0;

	// zanim cokolwiek zrobimy, w buforze musi byæ minimum sizeof(PROTO_HEADER) bajtów
	if (RX_COUNT < sizeof(PROTO_HEADER))
	{
		// ale jeœli nie ma, to spróbuj trochê pomóc szczêsciu i zrobiæ wstêpn¹ sychronizacjê strumienia danych
		if (RX_COUNT == 0)
			return false; // nie ma czego synchronizowaæ

		if (rx.header.magic != HEADER_MAGIC)
		{
			RX_RESET;
			return false;
		}

		memmove((void*)(rx.buffer + 1), (void*)rx.buffer, RX_COUNT - 1);
		return false;
	}

	// Ok! Ca³y nag³ówek jest ju¿ odebrany!
	
	// Sprawdzamy adres!
	if (rx.header.address != 0xFF && rx.header.address != configuration.address)
	{
		// to nie do nas
		RX_RESET;
		return false;
	}

	// czy adres jest spójny z typem wiadomoœci?
	if ((rx.header.address == ADDRESS_BROADCAST) ^ (((uint8_t)rx.header.type & (uint8_t)MessageType::__BroadcastFlag) == (uint8_t)MessageType::__BroadcastFlag ))
	{
		// brak spójnoœci
		RX_RESET;
		return false;
	}

	// check if whole message is here
	if (RX_COUNT < sizeof(PROTO_HEADER) + rx.header.payload_length + sizeof(uint16_t))
		return false; // we have received only a part of the message; let us wait for the rest

	// ok, we have received at least whole message; check CRC
	uint16_t calculated_crc = calc_crc16((void*)rx.header.payload, rx.header.payload_length);
	uint16_t received_crc = *(uint16_t*)(rx.header.payload + rx.header.payload_length);
	if (calculated_crc != received_crc)
	{
		// checksums does not match
		RX_RESET;
		return false;
	}

	// ok - everything seems to good...
	return true;
}


#define BAUD 19200	// 8E1 (!!!)
#define BAUD_UX2

//////////////////////////////////////////////////////////////////////////

#if defined(BAUD_UX2)
// UX2 = 1
#define UBR0_VALUE (F_CPU/(8UL*BAUD))-1
#else
// UX2 = 0
#define UBR0_VALUE (F_CPU/(16UL*BAUD))-1
#endif

#define TIMER0_1MS_RELOAD 10

void cpu_init(void)
{
	DDRB |= _BV(PORTB2);
	DDRB |= _BV(PORTB1);
	DDRB |= _BV(PORTB5);
	DDRD = 0b00000110; // 0:RX, 1:TX, 3:DIR			1-wyjscie (1 i 3) ddr - data direction

	// port szergowy
	uint16_t br = UBR0_VALUE;
	UBRR0H = (uint8_t)(br >> 8);
	UBRR0L = (uint8_t)br;
	
	#if defined(BAUD_UX2)
	UCSR0A = _BV(U2X0);
	#else
	UCSR0A = 0x00;
	#endif
	
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00) | _BV(UPM01);	// 8E1
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);	// wlacz rx i txs
	UCSR0B |= (1 << RXCIE0);

	// struktury portu szeregowego
	tx.done = 1;


	// ustaw timer 0
	TCCR0A |= (1 << WGM01); // tryb CTC
	OCR0A = 0xF9;
	TIMSK0 |= (1 << OCIE0A);
	TCCR0B |= (1 << CS02) | (1<<CS00);

	// czy ja zyjê????
	for(int i = 0; i < 100; i++)
	{
		LED0_TOGGLE;
		LED1_TOGGLE;
		LED_TOGGLE;
		_delay_ms(20);
	}

	LED0_OFF; LED1_OFF; LED_OFF;
	_delay_ms(1000);

	// start przerwan
	sei();

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