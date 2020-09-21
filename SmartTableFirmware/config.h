/*
 * config.h
 *
 * Created: 26.04.2017 00:04:59
 *  Author: Tomek
 */ 


#ifndef CONFIG_H_
#define CONFIG_H_

#define SERIAL_BAUD			500000ULL	// 8E1 (!!!)
#define SERIAL_IDLE_LIMIT	10			// [0.5ms]

#define DEVICE_IDENTIFIER		(device_address_t)0x14

//extern const device_address_t device_address_block[] PROGMEM;
//extern device_address_t device_address;

#endif /* CONFIG_H_ */