/*
 * comm.h
 *
 * Created: 3/27/2017 13:50:39
 *  Author: Tomasz Jaworski
 */ 


#ifndef COMM_H_
#define COMM_H_

#include <avr/io.h>
#include <avr/interrupt.h>


//#define HEADER_MAGIC		((uint8_t)36)
#define RX_PAYLOAD_CAPACITY	32
#define TX_PAYLOAD_CAPACITY	128

#define ADDRESS_BROADCAST	(uint8_t)0xFF
#define DEVICE_ADDRESS		(uint8_t)0x50

enum class MessageType : uint8_t
{
	__BroadcastFlag = 0x80,
	Invalid = 0,
	None = 1,

	Ping = 2,					// send by the processing CPU (PC) to selected board
	Pong = 3,					// response to Ping message
	GetVersion = 4,				// get version of the firmware
	StartFullMeasurement = 5,	// initiate full resolution range measurement
	
	__MIN = Ping,
	__MAX = Pong,
};

struct PROTO_HEADER {
	//uint8_t magic;			// sync header
	uint8_t address;		// receiver address (if given) or ADDRESS_BROADCAST
	MessageType type;		// type of the received message
	uint8_t payload_length;	// 
};


struct TX
{
	// wskaüniki na dane do transmisji
	volatile const uint8_t* buffer_start;
	volatile const uint8_t* buffer_position;
	volatile const uint8_t* buffer_end;

	volatile uint8_t done;

	struct  
	{
		PROTO_HEADER header;
		uint8_t payload[TX_PAYLOAD_CAPACITY];
	} __attribute__((packed)) buffer;
};


struct RX
{
	volatile uint8_t* buffer_position;
	volatile bool got_data;

	// buffer - mind the order!
	struct {
		volatile PROTO_HEADER header;
		volatile uint8_t payload[RX_PAYLOAD_CAPACITY];
	} __attribute__((packed)) buffer;
};

#define RX_COUNT (rx.buffer_position - (volatile uint8_t*)&rx.buffer)

extern volatile TX tx;
extern volatile RX rx;

#endif /* COMM_H_ */