#ifndef MESSAGERECEIVER_HPP
#define MESSAGERECEIVER_HPP

#include <stdint.h>

#include "SerialPort.hpp"
#include "InputMessage.hpp"

#include <array>

class SerialPort;

class MessageReceiver
{
    std::array<uint8_t, 64 * 1024> queue;
	uint32_t position;
	
public:
	MessageReceiver(void);
	~MessageReceiver();
	
	
	ssize_t Receive(SerialPort::Ptr psource);
	
	bool getMessage(InputMessage& message);
	void PurgeAllData(void);

public:

};


#endif // MESSAGERECEIVER_HPP
