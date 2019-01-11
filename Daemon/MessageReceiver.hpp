#ifndef MESSAGERECEIVER_HPP
#define MESSAGERECEIVER_HPP

#include <stdint.h>
#include "Message.h"

class SerialPort;

class MessageReceiver
{
	uint8_t* data;
	uint32_t capacity;
	uint32_t position;
	
public:
	MessageReceiver(void);
	~MessageReceiver();
	
	
	ssize_t receive(SerialPort& source);
	
	bool getMessage(Message& output_message);
	void purgeAllData(void);

public:
	const uint8_t* getDataPointer(void) const { return this->data; }
	uint32_t getDataCount(void) const { return this->position; }
	
};


#endif // MESSAGERECEIVER_HPP
