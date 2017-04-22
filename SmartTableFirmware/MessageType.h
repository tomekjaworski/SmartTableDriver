/*
 * MessageType.h
 *
 * Created: 22.04.2017 15:06:45
 *  Author: Tomek Jaworski
 */ 


#ifndef MESSAGETYPE_H_
#define MESSAGETYPE_H_

enum class MessageType : uint8_t
{
	__BroadcastFlag = 0x80,
	Invalid = 0,
	None = 1,

	Ping = 2,					// send by the processing CPU (PC) to selected board
	Pong = 3,					// response to Ping message
	GetVersion = 4,				// get version of the firmware
	StartFullMeasurement = 5,	// initiate full resolution range measurement
	
	__MIN = Ping,
	__MAX = Pong,
};



#endif /* MESSAGETYPE_H_ */