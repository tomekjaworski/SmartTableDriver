#include <unistd.h>
#include <cassert>
#include <algorithm>
#include <cstring>

#include "InputMessageBuilder.hpp"
#include "../Utility/Crc16.hpp"
#include "../Utility/Helper.hpp"
#include "../TableFirmware/protocol.h"

typedef unsigned short checksum_t;

InputMessageBuilder::InputMessageBuilder(void)
    : queue(1024) {
}

InputMessageBuilder::~InputMessageBuilder() {
}

void InputMessageBuilder::PurgeAllData(void) {
}

void InputMessageBuilder::InternalAddCollectedData(const void* ptr, uint32_t count) {
    this->queue.EnqueueBytes(ptr, 0, count);
}

MessageExtractionResult InputMessageBuilder::ExtractMessage(InputMessage& message)
{
    /*
     * This code tries to parse the stream of incoming bytes as an input message.
     * It is based on removing one byte at a time if no match is found.
     * While normally this approach is not optimal, the expected performance
     * is not crucial here.
     */

	while (true) {
		// is there enough data for the shortest message?
		if (!this->queue.HasAtLeast(sizeof(TX_PROTO_HEADER) + sizeof(checksum_t)))
			return MessageExtractionResult::NeedMoreData; // nope - need more data
		
		// Header verification: address
		const TX_PROTO_HEADER* phdr = this->queue.GetDataPointer<TX_PROTO_HEADER>();
		
		if (phdr->magic != PROTOCOL_HEADER_VALUE) {   // Is there any magic? :)
			// remove one byte and loop
			//std::shift_left(this->queue.begin(), this->queue.end(), 1);
			this->queue.DequeueBytes(1);
			continue;
		}
		
		// Header verification: message type
		MessageType mt = phdr->type;
		if (mt != MessageType::DeviceIdentifierResponse &&
		    mt != MessageType::SingleMeasurement8Response &&
		    mt != MessageType::SingleMeasurement10Response &&
		    mt != MessageType::TriggeredMeasurementEnterResponse &&
		    mt != MessageType::TriggeredMeasurementLeaveResponse &&
		    mt != MessageType::PingResponse &&
		    mt != MessageType::RebootResponse &&
		    mt != MessageType::SetTriggerGeneratorResponse &&
		    mt != MessageType::SetTriggerStateResponse)
		{
            // remove one byte and loop
            //std::shift_left(this->queue.begin(), this->queue.end(), 1);
            this->queue.DequeueBytes(1);
            continue;
		}
		
		// is there enough data in the queue?
		if (!this->queue.HasAtLeast(sizeof(TX_PROTO_HEADER) + phdr->payload_length + sizeof(checksum_t)))
			return MessageExtractionResult::NeedMoreData; // nope - need more data
		
		// ok, there is; now verify the checksum
		checksum_t calculated = Crc16::Calc(this->queue.GetDataPointer(), sizeof(TX_PROTO_HEADER) + phdr->payload_length);
        checksum_t received = this->queue.PeekValue<checksum_t>(sizeof(TX_PROTO_HEADER) + phdr->payload_length);

		if (calculated != received)
		{
			// remove one byte and loop
            //std::shift_left(this->queue.begin(), this->queue.end(), 1);
            this->queue.DequeueBytes(1);
            continue;
		}
		
		// The Checksum is OK!
		message = InputMessage(
		        this->queue.GetDataPointer<void>(),
		        sizeof(TX_PROTO_HEADER) + phdr->payload_length + sizeof(checksum_t));
		uint32_t offset = sizeof(TX_PROTO_HEADER) + phdr->payload_length + sizeof(checksum_t);

        this->queue.DequeueBytes(offset);
        return MessageExtractionResult::Ok;
	}

	printf("?");
	assert(false);
	return MessageExtractionResult::Error;
	
}