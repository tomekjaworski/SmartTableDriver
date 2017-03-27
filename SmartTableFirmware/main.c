/*
 * SmartTableFirmware.c
 *
 * Created: 24/03/2017 16:30:33
 * Author : Iza
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "hardware.h"
#include "eeprom_config.h"

enum MessageType
{
	__BroadcastFlag = 0x80,
	Invalid = 0,
	None = 1,

	Ping = 2,	// ping wysy³a komputer do konkretnego modu³u
	Pong = 3,	// konkretny modu³ odpowiada na Ping wiadomoœci¹ Pong
};

typedef struct
{
	uint8_t magic;			// nag³ówek
	uint8_t address;		// adres nadawcy (jeœli nadaje modu³) lub odbiorcy (jeœli nadaje komputer)
	enum MessageType type;	// typ wiadomoœci
	uint8_t size;

	uint8_t data[0];		// tutaj id¹ dane

} PROTO_HEADER;

// testy na czas kompilacji
STATIC_ASSERT(sizeof(enum MessageType) == 1, message_type_ma_zly_rozmiar);
STATIC_ASSERT(sizeof(PROTO_HEADER) == 4, proto_header_ma_zly_rozmiar);


void cpu_init(void);
void begin_transmission(const void* data, uint8_t count);
void check_rx(void);


struct TX
{
	// wskaŸniki na dane do transmisji
	const uint8_t* buffer_start;
	const uint8_t* buffer_position;
	const uint8_t* buffer_end;

	uint8_t done;
} volatile tx = { .done = 1 };

#define HEADER_MAGIC	((uint8_t)36)
#define RX_BUFFER_SIZE	32

struct RX
{
	uint8_t* buffer_position;
	uint8_t got_data;
	union {
		uint8_t buffer[RX_BUFFER_SIZE]; // czy tyle wystarczy?
		PROTO_HEADER header;
	};
} volatile rx = {};

#define RX_COUNT (rx.buffer_position - rx.buffer)

const char* test = "Ala ma kota!";

int main(void)
{

	cpu_init();
	configuration_load();


	while(1)
	{
		if (!rx.got_data)
			continue; // nuuuudy

		rx.got_data = 0;
		check_rx();
	}

    while (1) 
    {
	begin_transmission(test, 12);
	_delay_ms(100);
    }
}
#define RX_RESET do { rx.buffer_position = rx.buffer; } while (0);


void check_rx(void)
{
	// zanim cokolwiek zrobimy, w buforze musi byæ minimum sizeof(PROTO_HEADER) bajtów
	if (RX_COUNT < sizeof(PROTO_HEADER))
	{
		// ale jeœli nie ma, to spróbuj trochê pomóc szczêsciu i zrobiæ wstêpn¹ sychronizacjê strumienia danych
		if (RX_COUNT == 0)
			return; // nie ma czego synchronizowaæ

		if (rx.header.magic != HEADER_MAGIC)
		{
			RX_RESET;
			return;
		}

		memmove(rx.buffer + 1, rx.buffer, RX_COUNT - 1);
		return;
	}

	// Ok! Ca³y nag³ówek jest ju¿ odebrany!
	
	// Sprawdzamy adres!
	if (rx.header.address != 0xFF && rx.header.address != configuration.address)
	{
		// to nie do nas
		RX_RESET;
		return;
	}

	// czy adres jest spójny z typem wiadomoœci?
	if ((rx.header.address == 0xFF) ^ ((rx.header.type & MessageType::__BroadcastFlag) == MessageType::__BroadcastFlag ))
	{
		// brak spójnoœci
		RX_RESET;
		return;
	}
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

	// timer0 = 2kHz (500us) - do modbusa
	//TCCR0  = (0b110 << CS00); // clkT0S = clkOSC/1024
	//TIMSK |= (1<<OCIE0) |  (1<<TOIE0);
	//TIMSK &= ~_BV(OCIE0);
	//TCNT0 = TIMER0_1MS_RELOAD;

	sei();

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
}


ISR(USART_TX_vect) //uruchamia sie jak wysle 1 bajt danych
{
	if (tx.buffer_position == tx.buffer_end) // jeœli koniec bufora, to wy³¹cz nadajnik i ustaw flagê
	{
		UCSR0B &= ~_BV(TXCIE0);//wy³¹czanie przerwañ nadajnika
		RS485_DIR_RECEIVE;
		tx.done = 1;
		return;
	}

	// wyslij bajt i zwieksz wskaŸnik
	UDR0 = *tx.buffer_position++;
}

ISR(USART_RX_vect)
{
	__attribute__((unused)) uint8_t status = UCSR0A;
	__attribute__((unused)) uint8_t data = UDR0;

	if (status & (_BV(FE0) | _BV(DOR0) | _BV(UPE0))) // bledy: frame error, data overrun, parity error
		return;

	if (RX_COUNT == RX_BUFFER_SIZE) // przepe³nienie bufora
		return;

	*rx.buffer_position++ = data;
	rx.got_data = 1;
		

}

void begin_transmission(const void* data, uint8_t count)
{
	// przygotowanie mechanizmu transmisyjnego
	tx.buffer_start = data;
	tx.buffer_position = tx.buffer_start;
	tx.buffer_end = tx.buffer_start + count;

	// nadanie pierwszego bajta uruchamia potok przerwañ
	RS485_DIR_SEND;
	UDR0 = *tx.buffer_position++;
	UCSR0B |= _BV(TXCIE0); // uruchom przerwania informuj¹ce o zakoñczeniu nadawania bajta
	tx.done = 0;
}