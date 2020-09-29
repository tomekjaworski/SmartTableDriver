#ifndef TABLEDEVICE_HPP
#define TABLEDEVICE_HPP

#include <memory>

#include "Location.hpp"
#include "SerialPort.hpp"


typedef unsigned char device_identifier_t; // Firmware: protocol_h

class PhotoModule {

public:
	typedef std::shared_ptr<PhotoModule> Ptr;

private:
    device_identifier_t id;
	Location location;
	SerialPort::Ptr sp;
	
public:
	const Location& GetLocation(void) const {
	    return this->location;
	}

    device_identifier_t GetID(void) const {
	    return this->id;
	}
	
	const SerialPort::Ptr& GetSerialPort(void) const {
	    return this->sp;
	}

	SerialPort::Ptr& GetSerialPort(void) {
	    return this->sp;
	}
	
	void SetSerialPort(SerialPort::Ptr& pserial) {
		this->sp = pserial;
	}
	
	PhotoModule(device_identifier_t id, Location loc)
		: id(id), location(loc), sp(nullptr)
	{
		//
	}
	
};

#endif // TABLEDEVICE_HPP
