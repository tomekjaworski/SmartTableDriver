#if !defined(_MESSAGE_H_)
#define _MESSAGE_H_

//#include <stddef.h>
#include <stdexcept>
#include <string.h>
#include <vector>

#include "Crc16.hpp"
#include <cstdint>
#include "../SmartTableFirmware/protocol.h"

/// @brief Klasa reprezentująca komunikat przesyłany przez serwer (komputer) do modułu światłoczułego.
class OutputMessage
{
	
private:
	std::vector<uint8_t> data;
	
public:

    OutputMessage()
		: OutputMessage(MessageType::Invalid)
	{		
	}

    OutputMessage(const OutputMessage& msg)
	{
	    this->data = msg.data;
	}

    OutputMessage(OutputMessage&& m)
	    : data(std::move(m.data))
	{
	}

    OutputMessage& operator = (OutputMessage&& m) {
		std::swap(this->data, m.data);
		return *this;
	}


    OutputMessage(MessageType type, const void* payload = nullptr, size_t payload_length = 0)
	{
        if (payload_length > 0 && payload != nullptr)
            throw std::invalid_argument("payload_length > 0 && payload != nullptr");

		if (payload_length > RX_PAYLOAD_CAPACITY)
			throw std::length_error("payload_length too large");

//		size_t message_size = sizeof(TX_PROTO_HEADER) + payload_length + sizeof(uint16_t);

        TX_PROTO_HEADER header;
        header.magic = PROTO_MAGIC;
        header.payload_length = static_cast<uint8_t>(payload_length);
        header.type = type;

        uint8_t* ptr = (uint8_t*)&header;
        this->data.insert(this->data.end(), ptr, ptr + sizeof(TX_PROTO_HEADER));

		// calculate and append crc16 checksum
		uint16_t crc = Crc16::Calc(this->data.data(), this->data.size());
		this->data.push_back(crc & 0x00FF); // lsb first
		this->data.push_back((crc >> 8) & 0x00FF);
	}

	~OutputMessage()
	{
	}
	
public:
//	uint16_t getChecksum(void) const { return *(uint16_t*)(this->data + sizeof(PROTO_HEADER) + getHeader().payload_length); }
	
//	device_address_t getAddress(void) const { return this->getHeader().address; }
//	MessageType getType(void) const { return this->getHeader().type; }
//
//	const void* getDataPointer(void) const { return this->data; }
//	size_t getDataCount(void) const { return sizeof(PROTO_HEADER) + getHeader().payload_length + sizeof(uint16_t); }
//
//	const void* getPayload(void) const { return this->data + sizeof(PROTO_HEADER); }
//	size_t getPayloadLength(void) const { return getHeader().payload_length; }
//
//	bool isResponse(void) const { return static_cast<uint8_t>(this->getHeader().type) & static_cast<uint8_t>(MessageType::__ResponseFlag); }
//
//	bool getPayloadAsBoolean(void) const {
//		if (this->getPayloadLength() == 0)
//			return false;
//		return *(bool*)this->getPayload();
//	}
	
};

#endif // _MESSAGE_H_


