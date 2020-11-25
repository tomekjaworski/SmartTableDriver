/*
 * config.h
 *
 * Created: 4/17/2017 11:26:17
 *  Author: Tomasz Jaworski
 */ 


#ifndef CONFIG_H_
#define CONFIG_H_

//
// serial port baud rate
#define SERIAL_BAUD 19200	// 8E1 (!!!)

//
// bootloader address
#define BOOTLOADER_HARDWARE_ADDRESS	(uint8_t)0x36

//
// advertisement wait time in 5 ms - time that bootloader spends waiting for C&C software to send the activation byte.
// If the time given in ADVERTISEMENT_WAIT_TIME passes, bootloader jumps into user space code - addr 0x0000
#define ADVERTISEMENT_WAIT_TIME	15ULL * (25ULL * F_CPU / 1000000ULL)
// 2000 = 10,000 ms

//
// Compiled functionality: allows to read CPUs vendor signature
#define PROTOCOL_READ_SIGNATURE

// Compiled functionality: allows to read bootloader's version
#define PROTOCOL_READ_BOOTLOADER_VERSION

// Use RS485 driver, connected (DIR) to port RD2
#define USE_RS485
#undef USE_RS485

//
// Bootloader Protocol Version
#define PROTOCOL_VERSION 3

//
// Double speed USART port
#define SERIAL_UX2

#endif /* CONFIG_H_ */