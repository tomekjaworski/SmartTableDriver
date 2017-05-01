#include <unistd.h>
#include "MessageReceiver.hpp"
#include "CRC.hpp"
#include <assert.h>

#include "SerialPort.hpp"


MessageReceiver::MessageReceiver(void)
{
	this->capacity = 64 * 1024;
	this->position = 0;
	this->data = new uint8_t[this->capacity];
}

MessageReceiver::~MessageReceiver()
{
	if (this->data != nullptr)
	{
		delete[] this->data;
		this->data = nullptr;
	}
}

void MessageReceiver::purgeAllData(void)
{
	this->position = 0;
}

ssize_t MessageReceiver::receive(SerialPort& source)
{
	uint32_t space_left = this->capacity - this->position;
	if (space_left < 1024)
	{
		// make some room
		uint32_t new_cap = this->capacity * 1.5;
		uint8_t* new_ptr = new uint8_t[new_cap];
		
		memcpy(new_ptr, this->data, this->position);
		
		std::swap(new_ptr, this->data);
		std::swap(new_cap, this->capacity);
		
		delete[] new_ptr;
	}
	
	ssize_t bytes_read = source.receive(data + position, capacity - position);
	assert(bytes_read > 0);
	
	position += bytes_read;
	
	return bytes_read;
}


bool MessageReceiver::getMessage(Message& output_message)
{
	uint32_t offset = 0;
	bool got_message = false;
	while (true)
	{
		// is there enough data for the shortest message?
		if (this->position < offset + sizeof(PROTO_HEADER) + sizeof(uint16_t))
			break; // nope - need more data
		
		// Header verification: address
		const PROTO_HEADER* phdr = (const PROTO_HEADER*)this->data;
		
		if ((phdr->address >= 0xF0 || phdr->address == 0x00) && phdr->address != ADDRESS_BROADCAST) // addresses are only 0x01 - 0xEF
		{
			// remove one byte and loop
			offset += 1;
			continue;
		}
		
		// Header verification: message type
		if (phdr->type < MessageType::__MIN || phdr->type > MessageType::__MAX)
		{
			// remove one byte and loop
			offset += 1;
			continue;
		}
		
		// is there enough data in buffer?
		if (this->position < offset + sizeof(PROTO_HEADER) + phdr->payload_length + sizeof(uint16_t))
			break; // nope - need more data
		
		// ok, there is; now verify the checksum
		uint16_t calculated = CRC16::Calc(this->data, sizeof(PROTO_HEADER) + phdr->payload_length);
		uint16_t received = this->data[sizeof(PROTO_HEADER) + phdr->payload_length + 0];
		received |= (uint16_t)(this->data[sizeof(PROTO_HEADER) + phdr->payload_length + 1] << 8);
		
		if (calculated != received)
		{
			// remove one byte and loop
			offset += 1;
			continue;
		}
		
		// The Checksum is OK!
		output_message = Message(data + offset, sizeof(PROTO_HEADER) + phdr->payload_length + sizeof(uint16_t));
		
		got_message = true;
		break;
	}

	if (offset > 0)
	{
		// move the receive buffer back by offset bytes
		memmove(this->data, this->data + offset, this->position - offset);
		this->position -= offset;
	}
	
	return got_message;
	
}