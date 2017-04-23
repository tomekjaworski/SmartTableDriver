#ifndef MESSAGERECEIVER_HPP
#define MESSAGERECEIVER_HPP

#include <stdint.h>
#include "Message.h"

class MessageReceiver
{
	uint8_t* data;
	uint32_t capacity;
	uint32_t position;
	
public:
	MessageReceiver(void);
	~MessageReceiver();
	
	
	int receive(int fd);
	
	bool getMessage(Message& output_message);
	void purgeAllData(void);

	
};


#endif // MESSAGERECEIVER_HPP
