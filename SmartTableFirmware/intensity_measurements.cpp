/*
 * intensity_measurements.cpp
 *
 * Created: 3/27/2017 17:32:04
 *  Author: Izabela Perenc
			Tomasz Jaworski
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "intensity_measurements.h"
#include "hardware.h"
#include "dbg_putchar.h"

struct BURST burst = {};

#define NOP asm volatile("nop");


inline static uint16_t __adc10(uint8_t channel)
{
	// REF: Capacitor at AREF to GND
	ADMUX = _BV(REFS0) | (0x0F & channel);
	// start the conversion
	ADCSRA |= _BV(ADSC);

	// ADSC is cleared when the conversion finishes
	while (ADCSRA & _BV(ADSC));

	//ADCW;
	//uint16_t low  = ADCL;
	//uint16_t high = ADCH;

	// combine the two bytes
	//return (high << 8) | low;
	return ADCW;
}

inline static uint8_t __adc8(uint8_t channel)
{
	// REF: Capacitor at AREF to GND
	ADMUX = _BV(REFS0) | _BV(ADLAR) | (0x0F & channel);
	// start the conversion
	ADCSRA |= _BV(ADSC);

	// ADSC is cleared when the conversion finishes
	while (ADCSRA & _BV(ADSC));

	return ADCH;
}


void im_initialize8(void)
{
	ADCSRA = 0x00;

	// set ADC prescaler to 1:32
	ADCSRA |= _BV(ADPS2) | /*_BV(ADPS1) |*/ _BV(ADPS0);

	// power up the ADC
	ADCSRA |= _BV(ADEN);

	// Aref = AVcc, channel ADC0; adjust left
	ADMUX = _BV(REFS0) | _BV(ADLAR);
	
	// turn off digital input circuity
	DIDR0 = 0xFF;

	// dummy read
	__attribute__((unused)) volatile uint16_t dummy;
	for (uint8_t i = 0; i < 4; i++)
	{
		for (uint8_t channel = 0; channel < 8; channel++)
			dummy = __adc8(channel);
	}

}


void im_initialize10(void)
{
	ADCSRA = 0x00;

	// set ADC prescaler to 1:32
	ADCSRA |= _BV(ADPS2) | /*_BV(ADPS1) |*/ _BV(ADPS0);

	// power up the ADC
	ADCSRA |= _BV(ADEN);

	// Aref = AVcc, channel ADC0, adjust right
	ADMUX = _BV(REFS0);

	// turn off digital input circuity
	DIDR0 = 0xFF;

	// dummy read
	__attribute__((unused)) volatile uint16_t dummy;
	for (uint8_t i = 0; i < 4; i++)
	{
		for (uint8_t channel = 0; channel < 8; channel++)
		dummy = __adc10(channel);
	}
}

#define IM_DATA_PIN(__state)	do { if (__state) PORTB |= _BV(PORTB0); else PORTB &= ~_BV(PORTB0); } while(0);	//	
#define IM_CLOCK_PIN(__state)	do { if (__state) PORTD |= _BV(PORTD7); else PORTD &= ~_BV(PORTD7); } while(0);		//
//#define IM_ADC_READ(__id)		__adc(__id)			//


#define IM_CLOCK_PULSE do {			\
			IM_CLOCK_PIN(true);		\
			NOP;					\
			NOP;					\
			IM_CLOCK_PIN(false);	\
			} while(0);

//zamiana 10 i 14
const int readToNumbers[15][7] = {
	{43,44,45,46,47,48,49},
	{36,37,38,39,40,41,42},
	{29,30,31,32,33,34,35},
	{22,23,24,25,26,27,28},
	{15,16,17,18,19,20,21},
	{ 8, 9,10,11,12,13,14},
	{ 1, 2, 3, 4, 5, 6, 7},
	{ 0, 0, 0, 0, 0,99,100},
	{92,93,94,95,96,97,98},//
	{85,86,87,88,89,90,91},
	{78,79,80,81,82,83,84},
	{71,72,73,74,75,76,77},
	{64,65,66,67,68,69,70},//
	{57,58,59,60,61,62,63},
	{50,51,52,53,54,55,56}
};

 
 int numbersToVisible[10][10] = {
	{85,78, 71, 1, 2, 3, 4, 5, 6, 7},
	{86,79, 72, 8, 9,10,11,12,13,14},
	{87,80, 73,15,16,17,18,19,20,21},
	{88,81, 74,22,23,24,25,26,27,28},
	{89,82, 75,29,30,31,32,33,34,35},
	{90,83, 76,36,37,38,39,40,41,42},
	{91,84, 77,43,44,45,46,47,48,49},
	{96,97,100,50,51,52,53,54,55,56},
	{95,99, 98,57,58,59,60,61,62,63},
	{94,93, 92,64,65,66,67,68,69,70}
};

int utable[15][7];
int otable[10][10];

union im_raw_measurement_t im_data;


/*
	sensor selectors:
	
	0 1    2 3 4 5 6 7 8 9 10 11 12 13 14 15 
	8 DEAD 0 1 2 3 4 5 6 7 9  10 11 12 13 14 15 16
*/


void im_measure10(void)
{
	// inject LOW
	IM_DATA_PIN(false);
	IM_CLOCK_PULSE;

	// empty row
	IM_DATA_PIN(true);
	IM_CLOCK_PULSE;
	
	uint16_t* ptr = im_data.raw16;
	
	// 7 rows with 7 sensors
	for(int i = 0; i < 7; i++)
	{
		IM_CLOCK_PULSE;
		_delay_us(150);

		*ptr++ = __adc10(0);
		*ptr++ = __adc10(1);
		*ptr++ = __adc10(2);
		*ptr++ = __adc10(3);
		*ptr++ = __adc10(4);
		*ptr++ = __adc10(5);
		*ptr++ = __adc10(6);
	}
	
	// one row with 2 sensors only
	IM_CLOCK_PULSE;
	_delay_us(150);

	*ptr++ = __adc10(5);
	*ptr++ = __adc10(6);

	// 7 rows with 7 sensors
	for(int i = 0; i < 7; i++)
	{
		IM_CLOCK_PULSE;
		_delay_us(150);

		*ptr++ = __adc10(0);
		*ptr++ = __adc10(1);
		*ptr++ = __adc10(2);
		*ptr++ = __adc10(3);
		*ptr++ = __adc10(4);
		*ptr++ = __adc10(5);
		*ptr++ = __adc10(6);
	}
	
}



void im_measure8(void)
{
	// inject LOW
	IM_DATA_PIN(false);
	IM_CLOCK_PULSE;
	
	// empty row
	IM_DATA_PIN(true);
	IM_CLOCK_PULSE;
		
	uint8_t* ptr = im_data.raw8;

	/*
	for(int i = 0; i < 15; i++)
	{
		IM_CLOCK_PULSE;
		_delay_us(150);

		*ptr++ = __adc8(0);
		*ptr++ = __adc8(1);
		*ptr++ = __adc8(2);
		*ptr++ = __adc8(3);
		*ptr++ = __adc8(4);
		*ptr++ = __adc8(5);
		*ptr++ = __adc8(6);
	}
	*/
	
	// 7 rows with 7 sensors
	for(int i = 0; i < 7; i++)
	{
		IM_CLOCK_PULSE;
		_delay_us(150);

		*ptr++ = __adc8(0);
		*ptr++ = __adc8(1);
		*ptr++ = __adc8(2);
		*ptr++ = __adc8(3);
		*ptr++ = __adc8(4);
		*ptr++ = __adc8(5);
		*ptr++ = __adc8(6);
	}
	
	// one row with 2 sensors only
	IM_CLOCK_PULSE;
	_delay_us(150);

	*ptr++ = __adc8(5);
	*ptr++ = __adc8(6);

	// 7 rows with 7 sensors
	for(int i = 0; i < 7; i++)
	{
		IM_CLOCK_PULSE;
		_delay_us(150);

		*ptr++ = __adc8(0);
		*ptr++ = __adc8(1);
		*ptr++ = __adc8(2);
		*ptr++ = __adc8(3);
		*ptr++ = __adc8(4);
		*ptr++ = __adc8(5);
		*ptr++ = __adc8(6);
	}		
	
	
}

/*
 * void im_execute_sync(void)
 * A low-speed debug version of measurement routing
 */

void im_execute_sync(void)
{
	
	IM_DATA_PIN(false);
	IM_CLOCK_PIN(false);
	_delay_ms(2);
	IM_CLOCK_PIN(true);
	_delay_ms(2);
	IM_CLOCK_PIN(false);
	_delay_ms(2);
	IM_DATA_PIN(true);

	IM_CLOCK_PIN(false);
	_delay_ms(2);
	IM_CLOCK_PIN(true);
	_delay_ms(2);
  
	for(int i = 0; i < 15; i++)
	{
		IM_CLOCK_PIN(false);
		_delay_ms(2);
		IM_CLOCK_PIN(true);	
		_delay_ms(2);

		utable[i][0] = __adc10(3);
		utable[i][1] = __adc10(2);
		utable[i][2] = __adc10(1);
		utable[i][3] = __adc10(0);
		utable[i][4] = __adc10(4);
		utable[i][5] = __adc10(5);
		utable[i][6] = __adc10(6);
	}

	// reordering of measurements to match 10x10 points subimage
	int temp[105];
	for(int a = 0; a < 15; a++)
		for(int b = 0; b < 7; b++)
			temp[readToNumbers[a][b]] = utable[a][b];

	for(int a = 0; a<10; a++)
		for(int b = 0; b<10; b++)
			otable[a][b] = temp[numbersToVisible[a][b]];
}


