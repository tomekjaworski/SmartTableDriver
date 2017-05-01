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
	
	
	int receive(SerialPort& source);
	
	bool getMessage(Message& output_message);
	void purgeAllData(void);

	
};


#endif // MESSAGERECEIVER_HPP
