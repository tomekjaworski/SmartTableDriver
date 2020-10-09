/*
 * comm.h
 *
 * Created: 4/16/2017 23:29:05
 *  Author: Tomasz Jaworski
 */ 

#ifndef COMM_H_
#define COMM_H_

#define MAX_PAYLOAD_SIZE	(128+4)
#include "config.h"

enum class MessageType : uint8_t
{
	Activate = 'A',
	Deactivate = 'B',

	Ping = '?',
	Reboot = 'R',

	ReadFlashPage = 'X',
	WriteFlashPage = 'W',
	ReadEepromPage = 'E',
	WriteEepromPage = 'F',

#if defined(PROTOCOL_READ_SIGNATURE)
	ReadSignature = 'S',
#endif

#if defined(PROTOCOL_READ_BOOTLOADER_VERSION)
	ReadBootloaderVersion = 'V'
#endif

};

struct RX {
	uint8_t data[MAX_PAYLOAD_SIZE];
	uint8_t *ptr, *endptr;

	bool timeout;
};

extern RX rx;



#endif /* COMM_H_ */