//
// Created by Tomek on 29.09.2020.
//

#include "InputMessage.hpp"


MessageType InputMessage::GetMessageType(void) const {
    auto pheader = reinterpret_cast<const TX_PROTO_HEADER*>(this->payload.data());
    return pheader->type;
}

int InputMessage::GetPayloadSize(void) const {
    auto pheader = reinterpret_cast<const TX_PROTO_HEADER*>(this->payload.data());
    return pheader->payload_length;
}

int InputMessage::GetSequenceNumber(void) const {
    auto pheader = reinterpret_cast<const TX_PROTO_HEADER*>(this->payload.data());
    return pheader->sequence_counter;
}

device_identifier_t InputMessage::GetDeviceID (void) const {
    auto pheader = reinterpret_cast<const TX_PROTO_HEADER*>(this->payload.data());
    return pheader->device_id;
}


const void* InputMessage::internal_GetPayloadPointer(void) const {
    auto pheader = reinterpret_cast<const TX_PROTO_HEADER*>(this->payload.data());
    return reinterpret_cast<const void*>(pheader + 1);
}

