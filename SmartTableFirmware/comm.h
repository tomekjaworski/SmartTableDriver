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
#include "MessageType.h"
#include "protocol.h"

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