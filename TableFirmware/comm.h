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
#include "../DaemonV2/TableFirmware/protocol.h"
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

	TX_PROTO_HEADER header;
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
		RX_PROTO_HEADER header;
		uint8_t payload[RX_PAYLOAD_CAPACITY];
	} __attribute__((packed)) buffer;
};

extern volatile TX tx;
extern volatile RX rx;

void comm_initialize(void);
void comm_send(MessageType type, const void* payload, uint8_t payload_length);
bool comm_check_receiver(void);
void comm_reset_receiver(void);



#endif /* COMM_H_ */