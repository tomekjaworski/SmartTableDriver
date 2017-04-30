#ifndef TABLEDEVICE_HPP
#define TABLEDEVICE_HPP

#include "Location.hpp"

class TableDevice {

public:
	typedef std::shared_ptr<TableDevice> Ptr;

private:	
	device_address_t address;
	Location location;
	SerialPort::Ptr sp;
	
public:
	const Location& getLocation(void) const { return this->location; }
	device_address_t getAddress(void) const { return this->address; }
	
	const SerialPort::Ptr& getSerialPort(void) const { return this->sp; }
	SerialPort::Ptr& getSerialPort(void) { return this->sp; }
	
	void setSerialPort(SerialPort::Ptr& pserial)
	{
		this->sp = pserial;
	}
	
	TableDevice(device_address_t addr, Location loc)
		: address(addr), location(loc), sp(nullptr)
	{
		
	}
	
};

#endif // TABLEDEVICE_HPP
