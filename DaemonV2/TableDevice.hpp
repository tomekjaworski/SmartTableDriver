#ifndef TABLEDEVICE_HPP
#define TABLEDEVICE_HPP

#include <memory>

#include "Location.hpp"
#include "SerialPort.hpp"


typedef unsigned char device_identifier_t; // Firmware: protocol_h

class TableDevice {

public:
	typedef std::shared_ptr<TableDevice> Ptr;

private:
    device_identifier_t address;
	Location location;
	SerialPort::Ptr sp;
	
public:
	const Location& GetLocation(void) const {
	    return this->location;
	}

    device_identifier_t GetAddress(void) const {
	    return this->address;
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
	
	TableDevice(device_identifier_t addr, Location loc)
		: address(addr), location(loc), sp(nullptr)
	{
		//
	}
	
};

#endif // TABLEDEVICE_HPP
