#if !defined(_MESSAGE_H_)
#define _MESSAGE_H_


class Message
{
private:
	uint8_t* data;
	size_t payload_length;
	size_t message_size;
	
	
	
public:
	Message(uint8_t device_address, MessageType type, void* payload = nullptr, size_t payload_length = 0)
	{
		if (payload_length > RX_PAYLOAD_CAPACITY)
			throw std::length_error("payload_length too large");
			
			
		this->message_size = sizeof(PROTO_HEADER) + payload_length + sizeof(uint16_t);
		this->data = new uint8_t[this->message_size];
		
	
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
	
	
public:
	const PROTO_HEADER& getHeader(void) const { return *(const PROTO_HEADER*)this->data; }
	uint8_t getChecksum(void) const { return *(uint16_t*)(this->data + this->message_size - sizeof(uint16_t)); }
	
	const void* getBinary(void) const { return this->data; }
	size_t getBinaryLength(void) const { return this->message_size; }
	
};

#endif // _MESSAGE_H_


