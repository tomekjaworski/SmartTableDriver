/*
 * protocol.h
 *
 * Created: 22.04.2017 15:08:48
 *  Author: Tomek Jaworski
 */ 


#ifndef PROTOCOL_H_
#define PROTOCOL_H_


#define RX_PAYLOAD_CAPACITY	32
#define TX_PAYLOAD_CAPACITY	128

#define ADDRESS_BROADCAST	(uint8_t)0xFF
#define DEVICE_ADDRESS		(uint8_t)0x50

typedef unsigned char device_address_t;

struct PROTO_HEADER {
	device_address_t address;		// receiver address (if given) or ADDRESS_BROADCAST
	MessageType type;				// type of the received message
	uint8_t payload_length;			//
};




#endif /* PROTOCOL_H_ */