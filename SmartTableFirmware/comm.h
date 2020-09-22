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
#include "protocol.h"
#include "checksum.h"

enum class TransmitterState : uint8_t {
	IDLE,
	SendingHeader,
	SendingPayload,
	SendingCRC,
};

struct TX
{
	TransmitterState state;

	PROTO_HEADER header;
	checksum_t crc;

	const uint8_t* window_position;
	const uint8_t* window_end;

	const uint8_t* ppayload;

	uint8_t payload[TX_PAYLOAD_CAPACITY];
	
};


struct RX
{
	uint8_t* buffer_position;
	bool got_data;
	uint8_t idle_timer;

	// buffer - mind the order!
	struct {
		PROTO_HEADER header;
		uint8_t payload[RX_PAYLOAD_CAPACITY];
	} __attribute__((packed)) buffer;
};

#define RX_COUNT (rx.buffer_position - (volatile uint8_t*)&rx.buffer)

extern volatile TX tx;
extern volatile RX rx;

#endif /* COMM_H_ */