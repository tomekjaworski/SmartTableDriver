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


inline static uint16_t __adc(uint8_t channel)
{
	ADMUX = _BV(REFS0) | (0x07 & channel);

	// start the conversion
	ADCSRA |= _BV(ADSC);

	// ADSC is cleared when the conversion finishes
	while (ADCSRA & _BV(ADSC));

	uint16_t low  = ADCL;
	uint16_t high = ADCH;

	// combine the two bytes
	return (high << 8) | low;
}


void im_initialize(void)
{
	ADCSRA = 0;

	// set ADC prescaler to 125kHz
	ADCSRA |= _BV(ADPS2) | _BV(ADPS1);

	// enable ADC conversions and autotriggering
	//ADCSRA |= _BV(ADATE);
	ADCSRA |= _BV(ADEN);

	// Aref = AVcc, channel ADC0
	ADMUX = _BV(REFS0);
}

#define IM_DATA_PIN(__state)	do {} while(0);	//	
#define IM_CLOCK_PIN(__state)	do {} while(0);		//
#define IM_ADC_READ(__id)		__adc(__id)			//

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

void im_execute_sync(void)
{


	_delay_ms(500);
	// 8 -martwy  0-7 9-16
	IM_DATA_PIN(false);		//  digitalWrite(dataPin,LOW);
	IM_CLOCK_PIN(false);	//	digitalWrite(clockPin,LOW);
	_delay_ms(2);
	IM_CLOCK_PIN(true);		//	digitalWrite(clockPin,HIGH);
	_delay_ms(2);
	IM_CLOCK_PIN(false);	//	digitalWrite(clockPin,LOW);
	_delay_ms(2);
	IM_DATA_PIN(true);		//	digitalWrite(dataPin,HIGH);

	IM_CLOCK_PIN(false);	//	digitalWrite(clockPin,LOW);
	_delay_ms(2);
	IM_CLOCK_PIN(true);		//	digitalWrite(clockPin,HIGH);
	_delay_ms(2);
  
	for(int i = 0; i < 15; i++)
	{
		IM_CLOCK_PIN(false);	//	digitalWrite(clockPin,LOW);
		_delay_ms(2);
		IM_CLOCK_PIN(true);		//	digitalWrite(clockPin,HIGH);
		_delay_ms(2);

		utable[i][0] = IM_ADC_READ(0);
		utable[i][1] = IM_ADC_READ(1);
		utable[i][2] = IM_ADC_READ(2);
		utable[i][3] = IM_ADC_READ(3);
		utable[i][4] = IM_ADC_READ(4);
		utable[i][5] = IM_ADC_READ(5);
		utable[i][6] = IM_ADC_READ(6);
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


void im_execute_async(void)
{
}