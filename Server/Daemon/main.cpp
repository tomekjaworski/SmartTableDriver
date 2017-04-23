#include <stdio.h>
#include <string.h>
#include <stdexcept>
#include <termios.h>
#include <fcntl.h>
#include <list>

#include "CRC.hpp"
#include <string>
#include <algorithm>

#include "SerialPort.hpp"
#include "Environment.hpp"

#include "../../SmartTableFirmware/MessageType.h"
#include "../../SmartTableFirmware/protocol.h"

#include "Message.h"

#define AGREEN	"\e[32m"
#define ARED	"\e[31m"
#define AYELLOW	"\e[33m"
#define ARESET	"\e[0m"

#include "timeout_error.hpp"

bool SendAndWaitForResponse(SerialPort& serial, const Message& query, Message& response);


int main(int argc, char **argv)
{
	printf("Smart Table Reconstruction Daemon, by Tomasz Jaworski, 2017\n");
	printf("Built on %s @ %s\n\n", __DATE__, __TIME__);
	
	// 
	// show available serial ports
	std::string s = "";
	auto port_name_list = SerialPort::getSerialDevices();
	for (const auto& pname : port_name_list)
		if (!s.empty())
			s += ", " + pname;
		else
			s += pname;
			
	printf("Available serial ports (%d): %s\n", port_name_list.size(), s.c_str());
	
	//
	// open all serial ports
	std::list<SerialPort::Ptr> ports;
	for (const auto& pname : port_name_list)
	{
		try
		{
			printf("Opening port %s... ", pname.c_str());
			SerialPort::Ptr sp(new SerialPort());
			sp->init(pname+"0", false);
			ports.push_back(sp);
			
			printf(AGREEN "Done.\n" ARESET);
		} catch (const std::exception &ex)
		{
			printf(ARED "FAILED: " AYELLOW "%s\n" ARESET, ex.what());
		}
	}
	
	//
	// declare device groups
	/*
	 * 		Device MAP:						Group MAP:
	 * 										
	 * 		xx  xx  xx  xx  xx  xx			xx  xx  xx  xx  xx  xx
	 * 										
	 *		xx  xx  xx  xx  xx  xx			xx  xx  xx  xx  xx  xx
	 * 										
	 *		xx  xx  xx  xx  xx  xx			xx  xx  xx  xx  xx  xx
	 * 										
	 *		xx  xx  xx  xx  xx  xx			xx  xx  xx  xx  xx  xx
	 * 										
	 */
	std::vector<device_address_t> g1 = { 0x10, 0x11, 0x12, 0x13 };
	std::vector<device_address_t> g2 = { 0x20, 0x21, 0x22 };
	std::vector<device_address_t> g3 = { 0x30, 0x31, 0x32, 0x33 };
	std::vector<device_address_t> g4 = { 0x40, 0x41, 0x42 };
	std::vector<device_address_t> g5 = { 0x50, 0x51, 0x52 };
	std::vector<device_address_t> g6 = { 0x60, 0x61, 0x62 };
	std::vector<device_address_t> g7 = { 0x70, 0x71, 0x72, 0x73 };
	
	std::vector<std::vector<device_address_t> > groups = { g1, g2, g3, g4, g5, g6, g7 };
	

	//
	// concatenate all available addresses
	printf("Building topological information... ");
	std::list<device_address_t> addresses;
	for(const auto& group : groups)
		for(device_address_t dev_addr : group)
			if (std::find(addresses.begin(), addresses.end(), dev_addr) != addresses.end())
			{
				printf(ARED "Duplicated device address: 0x%02x!; quitting...\n" ARESET, dev_addr);
				exit(1);
			} else
				addresses.push_back(dev_addr);

	printf(AGREEN "Done. Got %d devices in %d groups\n" ARESET, addresses.size(), groups.size());
	
	
	//
	// Find all Smart Table devices that we need to communicate with and create a map
	
	printf("Scanning for SmartTable devices...\n");
	SerialPort::Ptr device2serial[256] = {};
	
	for(device_address_t dev_addr : addresses) {
		printf(" Looking for " AYELLOW "0x%02x" ARESET "... ", dev_addr);
		
		for(SerialPort::Ptr& sp : ports) {
			try {
				
				// send a ping to selected device on selected serial port and wait for an answer
				Message mping(dev_addr, MessageType::Ping);
				Message response;
				
				SendAndWaitForResponse(*sp, mping, response);
				if (response.getType() != MessageType::Pong || response.getAddress() != dev_addr)
					continue; // error
					
				device2serial[dev_addr] = sp;
			
			} catch(const timeout_error& te) {
				// ok, timeout means no reponse. Thats ok, i guess..
				continue;
			} catch (const std::exception& ex) {
				printf(ARED "Failed: %s\n" ARESET, ex.what());
			}
		}
	}
	
	
	// match groups to USB devices, since theirs order can change every time the system boots up
	
	
	
	
//	SerialPort sp;
	
	//sp.init("", false);
	
	
	//Message mping(0x50, MessageType::Ping);
	//sp.send(mping.getBinary(), mping.getBinaryLength());
	
	
	return 0;
}

#include <time.h>
#include <assert.h>

uint64_t getMilliseconds(void)
{
	timespec ts;
	int ret = clock_gettime(CLOCK_MONOTONIC, &ts);
	assert (ret == 0);
	uint64_t ms = ts.tv_nsec / 1000000 + ts.tv_sec * 1000;

	return ms;
}


class FifoBuffer
{
	public:
		FifoBuffer(){}
		
	public:
		void appendData(const void* data, size_t count) {}
};

#include <unistd.h>

class MessageReceiver
{
		uint8_t* data;
		uint32_t capacity;
		uint32_t position;
		
	public:
		MessageReceiver(void);
		~MessageReceiver();
		
		
		ssize_t receive(int fd);
		
		bool getMessage(void);
	
};

MessageReceiver::MessageReceiver(void)
{
	this->capacity = 64 * 1024;
	this->data = new uint8_t[this->capacity];
}

MessageReceiver::~MessageReceiver()
{
	if (this->data != nullptr)
	{
		delete[] this->data;
		this->data = nullptr;
	}
}


ssize_t MessageReceiver::receive(int fd)
{
	uint32_t space_left = this->capacity - this->position;
	if (space_left < 1024)
	{
		// make some room
		uint32_t new_cap = this->capacity * 1.5;
		uint8_t* new_ptr = new uint8_t[new_cap];
		
		memcpy(new_ptr, this->data, this->position);
		
		std::swap(new_ptr, this->data);
		std::swap(new_cap, this->capacity);
		
		delete[] new_ptr;
	}
	
	ssize_t bytes_read = ::read(fd, data + position, capacity - position);
	assert(bytes_read > 0);
	
	position += bytes_read;
	
	return bytes_read;
}


bool MessageReceiver::getMessage()
{
	// is there enough data for the shortest message?
	if (this->position < sizeof(PROTO_HEADER) + sizeof(uint16_t))
		return false; // nope - need more data
	
	// Header verification: address
	const PROTO_HEADER* phdr = (const PROTO_HEADER*)this->data;
	
	if (phdr->address >= 0xF0 || phdr->address == 0x00) // addresses are only 0x01 - 0xEF
	{
		// remove one byte and loop
	}
	
	// Header verification: message type
	if (phdr->type < MessageType::__MIN || phdr->type > MessageType::__MAX)
	{
		// remove one byte and loop
	}
	
	// is there enough data in buffer?
	if (this->position < sizeof(PROTO_HEADER) + phdr->payload_length + sizeof(uint16_t))
		return false; // nope - need more data
	
	// ok, there is; now verify the checksum
	uint16_t calculated = CRC16::Calc(this->data, sizeof(PROTO_HEADER) + payload_length);
	uint16_t received = this->data[sizeof(PROTO_HEADER) + payload_length + 0];
	received |= (uint16_t)(this->data[sizeof(PROTO_HEADER) + payload_length + 1] << 8);
	
	if (calculated != received)
	{
		// remove one byte and loop
	}
	
	// The Checksum is OK!
	
	
}


bool SendAndWaitForResponse(SerialPort& serial, const Message& query, Message& response)
{

	uint64_t timeout = 2000;

	
	uint64_t start_time = getMilliseconds();
	uint8_t buffer[1024];
	ssize_t position = 0;
	//FifoBuffer fb;
	
	do {
		fd_set rfd;
		FD_ZERO(&rfd);
		FD_SET(serial.getHandle(), &rfd);
		
		timeval tv = { .tv_sec = 0, .tv_usec = 50 * 1000 };
		int sret = ::select(serial.getHandle() + 1, &rfd, nullptr, nullptr, &tv);
		
		if (sret == 0)
			continue; // timeout - let's loop
			
		if (sret == -1) {
			perror(__func__);
			Environment::terminateOnError("select", 0);
		}
	
		ssize_t bytes_read = ::read(serial.getHandle(), buffer + position, sizeof(buffer) - position);
		assert(bytes_read > 0);
		
		position += bytes_read;
		
		
		
		
		
		
		
		// check for timeout
		if (getMilliseconds() - start_time > timeout)
			throw timeout_error("Timeout");
		
	} while (true);
}
