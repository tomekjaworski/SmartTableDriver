/*
 * comm.h
 *
 * Created: 3/27/2017 13:50:39
 *  Author: Tomek
 */ 


#ifndef COMM_H_
#define COMM_H_

#include <avr/io.h>
#include <avr/interrupt.h>


#define HEADER_MAGIC	((uint8_t)36)
#define RX_BUFFER_SIZE	32

#define ADDRESS_BROADCAST	(uint8_t)0xFF

enum class MessageType : uint8_t
{
	__BroadcastFlag = 0x80,
	Invalid = 0,
	None = 1,

	Ping = 2,	// send by the processing CPU (PC) to selected board
	Pong = 3,	// response to Ping message
};

struct PROTO_HEADER {
	uint8_t magic;			// sync header
	uint8_t address;		// receiverd data (if given) or ADDRESS_BROADCAST
	MessageType type;		// type of the received message
	uint8_t payload_length;	// 

	uint8_t payload[0];		// data, can be empty (payload_length == 0)

};


struct TX
{
	// wskaüniki na dane do transmisji
	volatile const uint8_t* buffer_start;
	volatile const uint8_t* buffer_position;
	volatile const uint8_t* buffer_end;

	volatile uint8_t done;
};


struct RX
{
	volatile uint8_t* buffer_position;
	volatile bool got_data;
	union {
		volatile uint8_t buffer[RX_BUFFER_SIZE]; // czy tyle wystarczy?
		volatile PROTO_HEADER header;
	};
};

#define RX_COUNT (rx.buffer_position - rx.buffer)

extern volatile TX tx;
extern volatile RX rx;

#endif /* COMM_H_ */