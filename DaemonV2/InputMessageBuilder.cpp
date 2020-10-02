#include <unistd.h>
#include "InputMessageBuilder.hpp"
#include "Crc16.hpp"
#include <cassert>
#include "Helper.hpp"
#include "../SmartTableFirmware/protocol.h"
#include <algorithm>
InputMessageBuilder::InputMessageBuilder(void) {
	this->position = 0;
}

InputMessageBuilder::~InputMessageBuilder()
{
//	if (this->data != nullptr)
//	{
//		delete[] this->data;
//		this->data = nullptr;
//	}
}

void InputMessageBuilder::PurgeAllData(void)
{
	this->position = 0;
}
#include <cstring>
void InputMessageBuilder::InternalAddCollectedData(const void* ptr, uint32_t count)
{
    if (this->position + count >= this->queue.size())
        throw std::runtime_error("Out of memory in MessageReceive buffer");

    std::memcpy(this->queue.data() + this->position, ptr, count);
    this->position += count;
}

bool InputMessageBuilder::GetMessage(InputMessage& message)
{
    /*
     * This code tries to parse the stream of incoming bytes as an input message.
     * It is based on removing one byte at a time if no match is found.
     * While normally this approach is not optimal, the expected performance
     * is not crucial here.
     */

	bool got_message = false;
	while (true) {
		// is there enough data for the shortest message?
		if (this->position < sizeof(TX_PROTO_HEADER) + sizeof(uint16_t))
			break; // nope - need more data
		
		// Header verification: address
		const TX_PROTO_HEADER* phdr = reinterpret_cast<const TX_PROTO_HEADER*>(this->queue.data());
		
		if (phdr->magic != PROTO_MAGIC) {   // Is there any magic? :)
			// remove one byte and loop
			std::shift_left(this->queue.begin(), this->queue.end(), 1);
			this->position--;
			continue;
		}
		
		// Header verification: message type
		MessageType mt = phdr->type;
		if (mt != MessageType::DeviceIdentifierResponse &&
		    mt != MessageType::SingleMeasurement8Response &&
		    mt != MessageType::SingleMeasurement10Response &&
		    mt != MessageType::TriggeredMeasurementEnterResponse &&
		    mt != MessageType::TriggeredMeasurementLeaveResponse &&
		    mt != MessageType::PingResponse)
		{
            // remove one byte and loop
            std::shift_left(this->queue.begin(), this->queue.end(), 1);
            this->position--;
            continue;
		}
		
		// is there enough data in the queue?
		if (this->position < sizeof(TX_PROTO_HEADER) + phdr->payload_length + sizeof(checksum_t))
			break; // nope - need more data
		
		// ok, there is; now verify the checksum
		checksum_t calculated = Crc16::Calc(this->queue.data(), sizeof(TX_PROTO_HEADER) + phdr->payload_length);
        checksum_t  received = this->queue[sizeof(TX_PROTO_HEADER) + phdr->payload_length + 0];
		received |= (uint16_t)(this->queue[sizeof(TX_PROTO_HEADER) + phdr->payload_length + 1] << 8);
		
		if (calculated != received)
		{
			// remove one byte and loop
            std::shift_left(this->queue.begin(), this->queue.end(), 1);
            this->position--;
            continue;
		}
		
		// The Checksum is OK!
		message = InputMessage(this->queue.data(), sizeof(TX_PROTO_HEADER) + phdr->payload_length + sizeof(checksum_t));
		int offset = sizeof(TX_PROTO_HEADER) + phdr->payload_length + sizeof(uint16_t);
        std::shift_left(this->queue.begin(), this->queue.end(), offset);
        this->position -= offset;

		got_message = true;
		break;
	}

	return got_message;
	
}