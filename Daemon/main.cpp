#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <list>
#include <chrono>
#include <thread>

#include <string>
#include <algorithm>
#include <sstream>

#include "SerialPort.hpp"
#include "Environment.hpp"
#include "MessageReceiver.hpp"


#include "Message.h"
#include "ansi.h"
#include "timeout_error.hpp"
#include "ImageDebuggerClient.h"


#include "Location.hpp"
#include "TableDevice.hpp"

bool SendAndWaitForResponse(SerialPort& serial, const Message& query, Message& response);

void ShowTopology(const std::string& prompt, const std::vector<std::vector<TableDevice::Ptr> >& groups)
{
	printf("%s:\n", prompt.c_str());
	for (size_t gid = 0; gid < groups.size(); gid++)
	{
		printf(" Group %d: ", gid + 1);
		for(TableDevice::Ptr pdev : groups[gid])
			printf(AYELLOW "%02x " ARESET, pdev->getAddress());
			
		if (groups[gid].empty())
			printf(ARED "None" ARESET);
		printf("\n");
	}
}

#include <unordered_map>

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
	/*
	
	SerialPort::Ptr pserial = ports.front();
	device_address_t addr = 0x14;
	
	while(1)
	{
		Message msg_response, msg_meas(addr, MessageType::GetFullResolutionSyncMeasurement);
		SendAndWaitForResponse(*pserial, msg_meas, msg_response);

		uint16_t* img = (uint16_t*)msg_response.getPayload();
		
		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 10; j++)
			{
				printf("%04d ", img[j + i * 10]);
			}
			
			printf("\n");
		}
		printf("\n");
		
		
		
		IDBG_ShowImage("GetFullResolutionSyncMeasurement", 10, 10, img, "U16");
		
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
	
	return 0;
	*/
	//
    // declare device groups
    /*
     *    Group MAP:
	 *                     6   7 
     *                                         
     *  1  01--02--03--04  05  06
     *                     |   |
     *  2  07--08--09--0A  0B  0C
     *                     |   |
     *  3  0D--0E--0F--10  11  12
     *                                                             
     *  4  13--14--15  16--17--18  5
     *                                         
     */
	
	std::vector<TableDevice::Ptr> g1 = {	TableDevice::Ptr(new TableDevice(0x01, Location(0, 0))),
											TableDevice::Ptr(new TableDevice(0x02, Location(1, 0))),
											TableDevice::Ptr(new TableDevice(0x03, Location(2, 0))),
											TableDevice::Ptr(new TableDevice(0x04, Location(3, 0))) 
										};

	std::vector<TableDevice::Ptr> g2 = {	TableDevice::Ptr(new TableDevice(0x07, Location(0, 1))),
											TableDevice::Ptr(new TableDevice(0x08, Location(1, 1))),
											TableDevice::Ptr(new TableDevice(0x09, Location(2, 1))),
											TableDevice::Ptr(new TableDevice(0x0A, Location(3, 1))) 
										};

	std::vector<TableDevice::Ptr> g3 = {	TableDevice::Ptr(new TableDevice(0x0D, Location(0, 2))),
											TableDevice::Ptr(new TableDevice(0x0E, Location(1, 2))),
											TableDevice::Ptr(new TableDevice(0x0F, Location(2, 2))),
											TableDevice::Ptr(new TableDevice(0x10, Location(3, 2))) 
										};

	std::vector<TableDevice::Ptr> g4 = {	TableDevice::Ptr(new TableDevice(0x13, Location(0, 3))),
											TableDevice::Ptr(new TableDevice(0x14, Location(1, 3))),
											TableDevice::Ptr(new TableDevice(0x15, Location(2, 3))) 
										};

	std::vector<TableDevice::Ptr> g5 = {	TableDevice::Ptr(new TableDevice(0x16, Location(3, 3))),
											TableDevice::Ptr(new TableDevice(0x17, Location(4, 3))),
											TableDevice::Ptr(new TableDevice(0x18, Location(5, 3))) 
										};

	std::vector<TableDevice::Ptr> g6 = {	TableDevice::Ptr(new TableDevice(0x05, Location(4, 0))),
											TableDevice::Ptr(new TableDevice(0x0B, Location(4, 1))),
											TableDevice::Ptr(new TableDevice(0x11, Location(4, 2))) 
										};

	std::vector<TableDevice::Ptr> g7 = {	TableDevice::Ptr(new TableDevice(0x06, Location(5, 0))),
											TableDevice::Ptr(new TableDevice(0x0C, Location(5, 1))),
											TableDevice::Ptr(new TableDevice(0x12, Location(5, 2))) 
										};

	std::vector<std::vector<TableDevice::Ptr> > groups = { g1, g2, g3, g4, g5, g6, g7 };
	

	//
	// concatenate all available addresses
	printf("Building topological information... ");
	std::unordered_map<device_address_t, TableDevice::Ptr> addresses;
	for(const auto& group : groups)
		for(TableDevice::Ptr pdevice : group)
			if (addresses.find(pdevice->getAddress()) != addresses.end())
			{
				printf(ARED "Duplicated device address: 0x%02x!; quitting...\n" ARESET, pdevice->getAddress());
				exit(1);
			} else
				addresses[pdevice->getAddress()] = pdevice;

	printf(AGREEN "Done. Got %d devices in %d groups\n" ARESET, addresses.size(), groups.size());
	ShowTopology("Expected topology", groups);
	
	
	//
	// Find all Smart Table devices that we need to communicate with and create a map
	
	printf("Scanning for SmartTable devices...\n");
	std::list<TableDevice::Ptr> missing_devices;
	
	for(auto& address_tuple : addresses) {
		TableDevice::Ptr& dev_addr = address_tuple.second;
		
		printf(" Looking for " AYELLOW "0x%02x" ARESET "... ", dev_addr->getAddress());
		
		bool found = false;
		std::string port_name = "unknown";
		for(SerialPort::Ptr& sp : ports) {
			try {
				
				// send a ping to selected device on selected serial port and wait for an answer
				Message mping(dev_addr->getAddress(), MessageType::Ping);
				Message response;
				
				SendAndWaitForResponse(*sp, mping, response);
				if (response.getType() != MessageType::Pong || response.getAddress() != dev_addr->getAddress())
					continue; // error
					
				dev_addr->setSerialPort(sp);
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
		for(TableDevice::Ptr pmissing_device : missing_devices) {
			auto pos = std::find_if(group.begin(), group.end(), [&pmissing_device](const auto& arg) { return arg->getAddress() == pmissing_device->getAddress(); });
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
	ShowTopology("Current topology (after device detection)", groups);
		
	
	// match groups to USB devices, since theirs order can change every time the system boots up
	printf("Matching groups to USB devices...\n");
	bool ok = true;
	for(const auto& group : groups) {
		SerialPort::Ptr expected_port = nullptr;
		for(const TableDevice::Ptr& dev_addr : group) {
			
			// set first pointer to SerialPort as expected pointer
			if (expected_port == nullptr)
				expected_port = dev_addr->getSerialPort();
				
			// we expect, that port's pointer will be the same
			if (dev_addr->getSerialPort() == expected_port)
				continue; // it's the same port, so group is not divided
			
			ok = false;
			
			// first device ws not found, so it has no port
			if (expected_port == nullptr) {
				printf(" Device 0x%02X: expected null pointer\n", dev_addr->getAddress());
				
				continue;
			}

			// second and further device was not found
			if (dev_addr->getSerialPort() == nullptr) {
				printf(" Device 0x%02X: expected serial %s (%p) but got null pointer\n", dev_addr->getAddress(),
				expected_port->getPortName().c_str(), expected_port.get());
				continue;
			}

			// there's a mismatch between logical description (groups) and physical connections
			printf(" Device 0x%02X: expected serial %s (%p) but got %s (%p)\n", dev_addr->getAddress(),
				expected_port->getPortName().c_str(), expected_port.get(),
				dev_addr->getSerialPort()->getPortName().c_str(), dev_addr.get());
		}
	}
	
	if (ok)
		printf("Ok.\n");

	//
	// get version of each device
	/*
	for (device_address_t dev_addr = 0; dev_addr < 0xF0; dev_addr++)
	{
		if (device2serial[dev_addr] == nullptr)
			continue; // no device at this point
			
		Message response, mver(dev_addr, MessageType::GetVersion);
				
		SendAndWaitForResponse(*device2serial[dev_addr], mver, response);
		assert(response.getType() == MessageType::GetVersion || response.getAddress() == dev_addr);
		
		std::string sver((const char*)response.getPayload(), response.getPayloadLength());
		printf("Device " AYELLOW "%02X" ARESET ": " AYELLOW "%s" ARESET "\n", dev_addr, sver.c_str());
	}
	 * */


	// tests
	{
		device_address_t addr = 0x14;
		Message msg_response, msg_meas(addr, MessageType::GetFullResolutionSyncMeasurement);

	}
	
	
	
	return 0;
}


bool SendAndWaitForResponse(SerialPort& serial, const Message& query, Message& response)
{

	int32_t timeout = 1500;

	
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
