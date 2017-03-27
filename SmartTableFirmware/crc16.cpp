/*
 * crc16.c
 *
 * Created: 25/03/2017 13:55:41
 *  Author: Tomasz Jaworski
 */ 


#include <stdint.h>
#include <avr/pgmspace.h>
#include "crc16.h"

uint16_t calc_crc16(const void *buf, uint8_t size)
{
	uint16_t crc;
	uint8_t i, j;
	
	crc = 0xFFFF;
	for (i = 0; i < size; i++)
	{
		crc = crc ^ (uint16_t)((uint8_t*)buf)[i];
		for (j = 8; j > 0; j--)
		if (crc & 0x0001)
		crc = (crc >> 1) ^ 0xA001;
		else
		crc >>= 1;
	}
	return crc;
}
