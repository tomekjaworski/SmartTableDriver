//
// Created by Tomek on 29.09.2020.
//

#include "InputMessage.hpp"


MessageType InputMessage::GetMessageType(void) const {
    const TX_PROTO_HEADER* pheader = reinterpret_cast<const TX_PROTO_HEADER*>(this->data.data());
    return pheader->type;
}

int InputMessage::GetPayloadSize(void) const {
    const TX_PROTO_HEADER* pheader = reinterpret_cast<const TX_PROTO_HEADER*>(this->data.data());
    return pheader->payload_length;
}

int InputMessage::GetSequenceNumber(void) const {
    const TX_PROTO_HEADER* pheader = reinterpret_cast<const TX_PROTO_HEADER*>(this->data.data());
    return pheader->sequence_counter;
}

const void* InputMessage::internal_GetPayloadPointer(void) const {
    const TX_PROTO_HEADER* pheader = reinterpret_cast<const TX_PROTO_HEADER*>(this->data.data());
    return reinterpret_cast<const void*>(pheader + 1);
}

