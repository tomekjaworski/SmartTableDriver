/*
 * config.h
 *
 * Created: 26.04.2017 00:04:59
 *  Author: Tomek
 */ 


#ifndef CONFIG_H_
#define CONFIG_H_

#include "../DaemonV2/TableFirmware/protocol.h"

#define SERIAL_BAUD				19200ULL	// 8E1 (!!!)
#define SERIAL_IDLE_LIMIT		10			// [0.5ms]

//#define DEVICE_IDENTIFIER		(device_identifier_t)0x14
#define FIRMWARE_VERSION		"2.0-tbl"
#define FIRMWARE_BUILD_DATE		__DATE__
#define FIRMWARE_BUILD_TIME		__TIME__


extern device_identifier_t device_identifier;

#endif /* CONFIG_H_ */