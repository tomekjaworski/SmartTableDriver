#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <termios.h>
#include <fcntl.h>
#include <list>


#include <string>
#include <algorithm>

#include "SerialPort.hpp"
#include "Environment.hpp"
#include "MessageReceiver.hpp"


#include "Message.h"

#include "ansi.h"

#include "timeout_error.hpp"

bool SendAndWaitForResponse(SerialPort& serial, const Message& query, Message& response);

void ShowTopology(const std::string& propmpt, const std::vector<std::vector<device_address_t> >& groups)
{
	printf("%s:\n", propmpt.c_str());
	for (size_t gid = 0; gid < groups.size(); gid++)
	{
		printf(" Group %d: ", gid + 1);
		for(device_address_t dev_id : groups[gid])
			printf(AYELLOW "%02x " ARESET, dev_id);
			
		if (groups[gid].empty())
			printf(ARED "None" ARESET);
		printf("\n");
	}
}

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
			sp->init(pname, false);
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
	 * 		xx  xx  xx  xx  xx  xx			xx--xx--xx--xx  xx  xx
	 * 														 |	 |
	 *		xx  xx  xx  xx  xx  xx			xx--xx--xx--xx  xx  xx
	 * 														 |	 |
	 *		xx  xx  xx  xx  xx  xx			xx--xx--xx--xx  xx  xx
	 * 															
	 *		xx  xx  xx  xx  xx  xx			xx--xx--xx  xx--xx--xx
	 * 										
	 */
	std::vector<device_address_t> g1 = { 0x1d, 0x11, 0x12, 0x13 };
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
	ShowTopology("Expected topology", groups);
	
	
	//
	// Find all Smart Table devices that we need to communicate with and create a map
	
	printf("Scanning for SmartTable devices...\n");
	SerialPort::Ptr device2serial[256] = {};
	std::list<device_address_t> missing_devices;
	
	for(device_address_t dev_addr : addresses) {
		printf(" Looking for " AYELLOW "0x%02x" ARESET "... ", dev_addr);
		
		bool found = false;
		std::string port_name = "unknown";
		for(SerialPort::Ptr& sp : ports) {
			try {
				
				// send a ping to selected device on selected serial port and wait for an answer
				Message mping(dev_addr, MessageType::Ping);
				Message response;
				
				SendAndWaitForResponse(*sp, mping, response);
				if (response.getType() != MessageType::Pong || response.getAddress() != dev_addr)
					continue; // error
					
				device2serial[dev_addr] = sp;
				found = true;
				port_name = sp->getPortName();
			
			} catch(const timeout_error& te) {
				// ok, timeout means no reponse. Thats ok, i guess..
				continue;
			} catch (const std::exception& ex) {
				printf(ARED "Failed: %s\n" ARESET, ex.what());
			}
		}
		
		if (found)
			printf(AGREEN "Found" ARESET " on %s\n", port_name.c_str());
		else
		{
			printf(ARED "Missing\n" ARESET);
			missing_devices.push_back(dev_addr);
		}
	}
	
	//
	// remove missing devices from groups
	int usable_dev_count = 0;
	for(auto& group : groups)
	{
		for(device_address_t missing_device : missing_devices) {
			auto pos = std::find(group.begin(), group.end(), missing_device);
			if (pos == group.end())
				continue; // not this group
			group.erase(pos);
		}
		usable_dev_count += group.size();
	}
	printf("Number of usable devices: %d\n", usable_dev_count);
	
	//
	// TODO: remove empty groups


	//
	// Show current topology
	ShowTopology("Current tepology (after device detection)", groups);
		
	
	// match groups to USB devices, since theirs order can change every time the system boots up
	printf("Matching groups to USB devices...\n");
	bool ok = true;
	for(const auto& group : groups) {
		SerialPort::Ptr expected_port = nullptr;
		for(device_address_t dev_addr : group) {
			
			// set first pointer to SerialPort as expected pointer
			if (expected_port == nullptr)
				expected_port = device2serial[dev_addr];
				
			// we expect, that port's pointer will be the same
			if (device2serial[dev_addr] == expected_port)
				continue; // it's the same port, so group is not divided
			
			ok = false;
			
			// first device ws not found, so it has no port
			if (expected_port == nullptr) {
				printf(" Device 0x%02X: expected null pointer\n", dev_addr);
				
				continue;
			}

			// second and further device was not found
			if (device2serial[dev_addr] == nullptr) {
				printf(" Device 0x%02X: expected serial %s (%p) but got null pointer\n", dev_addr,
				expected_port->getPortName().c_str(), expected_port.get());
				continue;
			}

			// there's a mismatch between logical description (groups) and physical connections
			printf(" Device 0x%02X: expected serial %s (%p) but got %s (%p)\n", dev_addr,
				expected_port->getPortName().c_str(), expected_port.get(),
				device2serial[dev_addr]->getPortName().c_str(), device2serial[dev_addr]);
		}
	}
	
	if (ok)
		printf("Ok.\n");

	//
	// get version of each device
	for (device_address_t dev_addr = 0; dev_addr < 0xF0; dev_addr++)
	{
		if (device2serial[dev_addr] == nullptr)
			continue; // no device at this point
			
		Message response, mver(dev_addr, MessageType::GetVersion);
				
		SendAndWaitForResponse(*device2serial[dev_addr], mver, response);
		assert(response.getType() == MessageType::GetVersion || response.getAddress() == dev_addr);
		
		std::string sver((const char*)response.getPayload(), response.getPayloadLength());
		printf("Device "AYELLOW"%02X"ARESET": "AYELLOW"%s"ARESET"\n", dev_addr, sver.c_str());
	}
	
	
//	SerialPort sp;
	
	//sp.init("", false);
	
	
	//Message mping(0x50, MessageType::Ping);
	//sp.send(mping.getBinary(), mping.getBinaryLength());
	
	
	return 0;
}

#include <assert.h>
#include <unistd.h>
#include <chrono>


bool SendAndWaitForResponse(SerialPort& serial, const Message& query, Message& response)
{

	int32_t timeout = 500;

	
	//uint8_t buffer[1024];
	//ssize_t position = 0;
	
	// prepare receiver and send query
	MessageReceiver mr;

	serial.discardAllData();
	serial.send(query.getBinary(), query.getBinaryLength());
	
	std::chrono::time_point<std::chrono::steady_clock> start_time = std::chrono::steady_clock::now();
	
	// now wait for response
	do {
		fd_set rfd;
		FD_ZERO(&rfd);
		FD_SET(serial.getHandle(), &rfd);
		
		timeval tv = { .tv_sec = 0, .tv_usec = 50 * 1000 };
		int sret = ::select(serial.getHandle() + 1, &rfd, nullptr, nullptr, &tv);
		
		if (sret == 0)
			goto _check_timeout; // timeout - let's loop
			
		if (sret == -1) {
			perror(__func__);
			Environment::terminateOnError("select", 0);
		}
	
		mr.receive(serial.getHandle());
		//ssize_t bytes_read = ::read(serial.getHandle(), buffer + position, sizeof(buffer) - position);
		//assert(bytes_read > 0);
		//position += bytes_read;
		
		if (mr.getMessage(response))
			return true;
		
		
		// check for timeout
_check_timeout:;
		std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count() > timeout)
			throw timeout_error("SendAndWaitForResponse");
		
	} while (true);
	
	return false;
}
