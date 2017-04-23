#if !defined(_MESSAGE_H_)
#define _MESSAGE_H_

#include <stddef.h>
#include <stdexcept>
#include <string.h>

#include "CRC.hpp"
#include "../../SmartTableFirmware/MessageType.h"
#include "../../SmartTableFirmware/protocol.h"

class Message
{
private:
	uint8_t* data;
	
public:

	Message()
		: Message(0, MessageType::Invalid)
	{		
	}

	Message(device_address_t device_address, MessageType type, const void* payload = nullptr, size_t payload_length = 0)
	{
		if (payload_length > RX_PAYLOAD_CAPACITY)
			throw std::length_error("payload_length too large");
			
			
		size_t message_size = sizeof(PROTO_HEADER) + payload_length + sizeof(uint16_t);
		this->data = new uint8_t[message_size];
		
	
		PROTO_HEADER* phdr = (PROTO_HEADER*)this->data;
		phdr->address = device_address;
		phdr->payload_length = payload_length;
		phdr->type = type;
	
		if (payload != nullptr)
			memcpy(this->data + sizeof(PROTO_HEADER), payload, payload_length);
	
		// calculate and append crc16 checksum
		uint16_t crc = CRC16::Calc(this->data, sizeof(PROTO_HEADER) + payload_length);
		this->data[sizeof(PROTO_HEADER) + payload_length + 0] = crc & 0x00FF; // lsb first
		this->data[sizeof(PROTO_HEADER) + payload_length + 1] = (crc >> 8) & 0x00FF;		
	}

	Message(const void* data, size_t size)
	{
		const PROTO_HEADER* phdr = (const PROTO_HEADER*)data;
		size_t msize = sizeof(PROTO_HEADER) + phdr->payload_length + sizeof(uint16_t);
		if (size < msize)
			throw std::runtime_error("Message format error");
			
		this->data = new uint8_t[msize];
		memcpy(this->data, data, msize);
	}
	
	~Message()
	{
		if (this->data != nullptr)
			delete [] this->data;
		this->data = nullptr;
	}
	
public:
	const PROTO_HEADER& getHeader(void) const { return *(const PROTO_HEADER*)this->data; }
	uint16_t getChecksum(void) const { return *(uint16_t*)(this->data + sizeof(PROTO_HEADER) + getHeader().payload_length); }
	
	device_address_t getAddress(void) const { return this->getHeader().address; }
	MessageType getType(void) const { return this->getHeader().type; }
	
	const void* getBinary(void) const { return this->data; }
	size_t getBinaryLength(void) const { return sizeof(PROTO_HEADER) + getHeader().payload_length + sizeof(uint16_t); }
	
};

#endif // _MESSAGE_H_


