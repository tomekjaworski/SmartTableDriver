/*
 * crc16.c
 *
 * Created: 25/03/2017 13:55:41
 *  Author: Tomasz Jaworski
 */ 


#include <stdint.h>
#include <avr/pgmspace.h>
#include "checksum.h"

uint16_t calc_checksum(const void *buf, uint8_t size)
{
	uint16_t crc;
	uint8_t i, j;
	
	crc = 0xFFFF;
	const uint8_t* ptr = (const uint8_t*)buf;
	for (i = 0; i < size; i++)
	{
		crc = crc ^ *ptr++;
		for (j = 8; j > 0; j--)
			if (crc & 0x0001)
				crc = (crc >> 1) ^ 0xA001;
			else
				crc >>= 1;
	}
	return crc;
}

uint16_t calc_checksum(const void *buf1, uint8_t size1, const void* buf2, uint8_t size2)
{
	uint16_t crc;
	uint8_t i, j;
	
	crc = 0xFFFF;
	const uint8_t* ptr = (const uint8_t*)buf1;

	for (i = 0; i < size1; i++)
	{
		crc = crc ^ *ptr++;
		for (j = 8; j > 0; j--)
			if (crc & 0x0001)
				crc = (crc >> 1) ^ 0xA001;
			else
				crc >>= 1;
	}

	ptr = (const uint8_t*)buf2;
	for (i = 0; i < size2; i++)
	{
		crc = crc ^ *ptr++;
		for (j = 8; j > 0; j--)
			if (crc & 0x0001)
		crc = (crc >> 1) ^ 0xA001;
			else
		crc >>= 1;
	}

	return crc;
}
