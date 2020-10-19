//
// Created by Tomek on 29.09.2020.
//

#include "OutputMessage.hpp"

OutputMessage::OutputMessage(MessageType type, const void* payload , size_t payload_length) {
    if (payload_length > 0 && payload == nullptr)
        throw std::invalid_argument("payload_length > 0 && payload == nullptr");

    if (payload_length > RX_PAYLOAD_CAPACITY)
        throw std::length_error("payload_length too large");

//		size_t message_size = sizeof(TX_PROTO_HEADER) + payload_length + sizeof(uint16_t);

    TX_PROTO_HEADER header;
    header.magic = PROTO_MAGIC;
    header.payload_length = static_cast<uint8_t>(payload_length);
    header.type = type;

    const uint8_t *ptr = reinterpret_cast<const uint8_t *>(&header);
    this->data.insert(this->data.end(), ptr, ptr + sizeof(TX_PROTO_HEADER));

    if (payload != nullptr) {
        ptr = reinterpret_cast<const uint8_t *>(payload);
        this->data.insert(this->data.end(), ptr, ptr + payload_length);
    }

// calculate and append crc16 checksum
    uint16_t crc = Crc16::Calc(this->data.data(), this->data.size());
    this->data.push_back(crc & 0x00FF); // lsb first
    this->data.push_back((crc >> 8) & 0x00FF);
}

