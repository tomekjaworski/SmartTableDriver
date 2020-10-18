#if !defined(_MESSAGE_H_)
#define _MESSAGE_H_

//#include <stddef.h>
#include <stdexcept>
#include <string.h>
#include <vector>

#include "../Utility/Crc16.hpp"
#include <cstdint>
#include "../../TableFirmware/protocol.h"

/// @brief Klasa reprezentująca komunikat przesyłany przez serwer (komputer) do modułu światłoczułego.
class OutputMessage
{
	
private:
	std::vector<uint8_t> data;
	
public:

    OutputMessage()
		: OutputMessage(MessageType::Invalid) {
	}

    OutputMessage(const OutputMessage& msg)	{
	    this->data = msg.data;
	}

    OutputMessage(OutputMessage&& m)
	    : data(std::move(m.data)) {
	}

    OutputMessage& operator = (OutputMessage&& m) {
		std::swap(this->data, m.data);
		return *this;
	}


    OutputMessage(MessageType type, const void* payload = nullptr, size_t payload_length = 0);

	~OutputMessage() {
	}
	
public:
//	uint16_t getChecksum(void) const { return *(uint16_t*)(this->data + sizeof(PROTO_HEADER) + getHeader().payload_length); }
	
//	device_address_t getAddress(void) const { return this->getHeader().address; }
//	MessageType getType(void) const { return this->getHeader().type; }
//
	const void* GetDataPointer(void) const {
	    return reinterpret_cast<const void*>(this->data.data());
	}

	size_t GetDataCount(void) const {
	    return this->data.size();
	}
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


