#include <cstdio>
#include <cassert>
#include <climits>
#include <list>
#include <chrono>
#include <thread>
#include <string>
#include <algorithm>
#include <sstream>
#include <unordered_map>

#include "SerialPort.hpp"
#include "Environment.hpp"
#include "MessageReceiver.hpp"


#include "Message.h"
#include "ansi.h"
#include "timeout_error.hpp"
#include "ImageDebuggerClient.h"


#include "Location.hpp"
#include "TableDevice.hpp"

#include "TableGroup.hpp"
#include "Image.hpp"


int kbhit (void);


bool SendAndWaitForResponse(SerialPort::Ptr serial, const Message& query, Message& response, int timeout);
bool SendAndWaitForResponse(std::list<SerialPort::Ptr>& serials, const Message& query, Message& response, SerialPort::Ptr& response_port, int timeout);

void AcquireFullImage(std::vector<TableGroup::Ptr>& tgroups, Image& img);
void ShowTopology(const std::string& prompt, const std::vector<std::vector<TableDevice::Ptr> >& groups);


int main(int argc, char **argv)
{
	printf("Smart Table Reconstruction Daemon, by Tomasz Jaworski, 2017\n");
	printf("Built on %s @ %s\n\n", __DATE__, __TIME__);
	setbuf(stdout, NULL);
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
	
	std::vector<TableDevice::Ptr> g1 = {	TableDevice::Ptr(new TableDevice(0x01, Location(00, 00))),
											TableDevice::Ptr(new TableDevice(0x02, Location(10, 00))),
											TableDevice::Ptr(new TableDevice(0x03, Location(20, 00))),
											TableDevice::Ptr(new TableDevice(0x04, Location(30, 00))) 
										};

	std::vector<TableDevice::Ptr> g2 = {	TableDevice::Ptr(new TableDevice(0x07, Location(00, 10))),
											TableDevice::Ptr(new TableDevice(0x08, Location(10, 10))),
											TableDevice::Ptr(new TableDevice(0x09, Location(20, 10))),
											TableDevice::Ptr(new TableDevice(0x0A, Location(30, 10))) 
										};

	std::vector<TableDevice::Ptr> g3 = {	TableDevice::Ptr(new TableDevice(0x0D, Location(00, 20))),
											TableDevice::Ptr(new TableDevice(0x0E, Location(10, 20))),
											TableDevice::Ptr(new TableDevice(0x0F, Location(20, 20))),
											TableDevice::Ptr(new TableDevice(0x10, Location(30, 20))) 
										};

	std::vector<TableDevice::Ptr> g4 = {	TableDevice::Ptr(new TableDevice(0x13, Location(00, 30))),
											TableDevice::Ptr(new TableDevice(0x14, Location(10, 30))),
											TableDevice::Ptr(new TableDevice(0x15, Location(20, 30))) 
										};

	std::vector<TableDevice::Ptr> g5 = {	TableDevice::Ptr(new TableDevice(0x16, Location(30, 30))),
											TableDevice::Ptr(new TableDevice(0x17, Location(40, 30))),
											TableDevice::Ptr(new TableDevice(0x18, Location(50, 30))) 
										};

	std::vector<TableDevice::Ptr> g6 = {	TableDevice::Ptr(new TableDevice(0x05, Location(40, 00))),
											TableDevice::Ptr(new TableDevice(0x0B, Location(40, 10))),
											TableDevice::Ptr(new TableDevice(0x11, Location(40, 20))) 
										};

	std::vector<TableDevice::Ptr> g7 = {	TableDevice::Ptr(new TableDevice(0x06, Location(50, 00))),
											TableDevice::Ptr(new TableDevice(0x0C, Location(50, 10))),
											TableDevice::Ptr(new TableDevice(0x12, Location(50, 20))) 
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
		
		try {
			
			// send a ping to selected device on selected serial port and wait for an answer
			Message mping(dev_addr->getAddress(), MessageType::Ping);
			Message response;
			
			SerialPort::Ptr response_port;
			
			SendAndWaitForResponse(ports, mping, response, response_port, 400);
			if (response.getType() != MessageType::Pong || response.getAddress() != dev_addr->getAddress())
			{
				throw std::runtime_error("protocol");
				continue;
			}
			
			printf(AGREEN "Found" ARESET " on %s\n", response_port->getPortName().c_str());
			dev_addr->setSerialPort(response_port);
			
		} catch(const std::exception &ex)
		{
			printf(ARED "Missing" ARESET " (%s)\n", ex.what());
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
	
	// get target image dimensions
	int minx = INT_MAX, miny = INT_MAX, maxx = INT_MIN, maxy = INT_MIN;
	for(auto& group : groups)
		for(TableDevice::Ptr& pdevice : group) {
			minx = std::min(minx, pdevice->getLocation().getColumn());
			miny = std::min(miny, pdevice->getLocation().getRow());
			maxx = std::max(maxx, 10 + pdevice->getLocation().getColumn() - 1);
			maxy = std::max(maxy, 10 + pdevice->getLocation().getRow() - 1);
		}
	
	assert(minx >= 0 && miny >= 0 && maxx <= 6*10 && maxy <= 4 * 10);
	
	int image_width = maxx - minx + 1;
	int image_height = maxy - miny + 1;
	printf("Target image information: Width = %d, Height = %d; Left = %d, Top = %d, Right = %d, Bottom = %d\n",
		image_width, image_height, minx, miny, maxx, maxy);
		
	
	// match groups to USB devices, since theirs order can change every time the system boots up
	std::vector <TableGroup::Ptr> tgroups;
	printf("Matching groups to USB devices...\n");
	for(auto& group : groups) {
		SerialPort::Ptr expected_port = nullptr;
		for(TableDevice::Ptr& pdevice : group) {
			
			// find a table group with the same serial port as the device
			auto pos = std::find_if(tgroups.begin(), tgroups.end(), [&pdevice] (const TableGroup::Ptr& ptg) { return ptg->getSerialPort() == pdevice->getSerialPort(); });
			TableGroup::Ptr pgroup = nullptr;
			if (pos == tgroups.end())
			{
				// no such group
				pgroup = TableGroup::Ptr(new TableGroup(pdevice->getSerialPort()));
				tgroups.push_back(pgroup);
			} else
				pgroup = *pos;
			
			
			TableDevice::Ptr pdev = pgroup->findByAddress(pdevice->getAddress());
			if (pdev != nullptr)
			{
				// that shouldn't happen...
				printf("?????");
				exit(1);
			}
			
			pgroup->addTableDevice(pdevice);
		}
	}
	
	//
	// sort table devices in each table group and then sort table groups for first device's address
	for(TableGroup::Ptr& pgroup : tgroups)
		std::sort(pgroup->begin(), pgroup->end(), [](const TableDevice::Ptr& p1, const TableDevice::Ptr& p2) { return p1->getAddress() <= p2->getAddress(); });
	std::sort(tgroups.begin(), tgroups.end(), [](const TableGroup::Ptr& g1, const TableGroup::Ptr& g2) { return (*g1->begin())->getAddress() <= (*g2->begin())->getAddress(); });


	
	//
	// Show Serial port-centric topology
	printf("Serial port-based topology:\n");
	for(const TableGroup::Ptr& pgroup : tgroups)
	{
		printf(" Port " AGREEN "%s" ARESET " with devices: ", pgroup->getSerialPort()->getPortName().c_str());

		bool first = true;
		int count = 0;
		for (const TableDevice::Ptr& pdev : *pgroup)
		{
			if (!first)
				printf(" ");
				
			
			printf(AYELLOW "%02X" ARESET, pdev->getAddress());
			count++;
			first = false;
		}
		
		printf(" (count=%d)\n", count);
	}
	
	//
	// calculate time points for full burst transmission
	uint16_t measure_time = 100; // measurement time, counted from broadcasted start
	uint16_t transmission_time = 10; // transmistion time that each device nned to fully send it's data
	
	// Transmission timing diagram:
	// 
	// | <- start
	// |    (broadcast)
	// |--- measurement ---|-- 1st transmission --|-- 2nd transmission --|-- 3rd transmission --|-- 4tht transmission --|-- ..... LAST transmission |
	// |    measure_time      transmission-time       transmission-time      transmission-time      transmission-time        transmission-time
	
	printf("Setting timing information... \n");
	int gid = 1;
	for(TableGroup::Ptr& pgroup : tgroups)
	{
		printf(" Group " AGREEN "%d" ARESET ": ", gid++);
		
		uint16_t current_time = measure_time;
		uint16_t silence_interval = measure_time + pgroup->getDeviceCount() * transmission_time;

		pgroup->setBitsPerPoint(16);
		
		for (TableDevice::Ptr& pdev : *pgroup)
		{
			try {
				
				printf(AYELLOW "%02X" ARESET, pdev->getAddress());
				
				BURST_CONFIGURATION bc;
				bc.bits_per_point = pgroup->getBitsPerPoint();
				bc.time_point = current_time;
				bc.silence_interval = silence_interval;
				
				device_address_t addr = pdev->getAddress();
				Message msg_response, msg_timing(addr, MessageType::SetBurstConfiguration, &bc, sizeof(BURST_CONFIGURATION));
				
				SendAndWaitForResponse(pdev->getSerialPort(), msg_timing, msg_response, 1000);
				if (msg_response.getType() != MessageType::SetBurstConfiguration || msg_response.getAddress() != pdev->getAddress() || !msg_response.getPayloadAsBoolean())
					throw std::runtime_error("msg_response.getType() != MessageType::SetBurstConfiguration || msg_response.getAddress() != pdev->getAddress()");

				printf(AGREEN "-ok " ARESET);
				printf("(tp=%dms; si=%dms)", bc.time_point, bc.silence_interval);
				current_time += transmission_time;

			
			} catch(const timeout_error& te) {
				printf(ARED "-timeout " ARESET);
				continue;
			} catch (const std::exception& ex) {
				printf(ARED "-err(%s) " ARESET, ex.what());
			}
			
		}
		
		printf("\n");
	}


	Image img(60, 40);
	while(!kbhit())
		AcquireFullImage(tgroups, img);

	// get burst statistics
	TableDevice::Ptr p = *tgroups[0]->begin();
	Message response, mstats(p->getAddress(), MessageType::GetBurstMeasurementStatistics);
			
	SendAndWaitForResponse(p->getSerialPort(), mstats, response, 1000);
	assert(response.getType() == MessageType::GetBurstMeasurementStatistics && response.getAddress() == p->getAddress() && response.getPayloadLength() == sizeof(BURST_STATISTICS));
	BURST_STATISTICS *pstats = (BURST_STATISTICS *)response.getPayload();
	
	printf("  Device %02x: Measure count=%d; measure time=%dms; transmission time=%dms\n", response.getAddress(), pstats->count, pstats->last_measure_time, pstats->last_transmission_time);

	std::this_thread::sleep_for(std::chrono::seconds(10));

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
	/*
	while(1)
	{
		TableGroup::Ptr ptg = tgroups[0];
		SerialPort::Ptr pserial = ptg->getSerialPort();
		
		device_address_t addr = 0x14;
		Message msg_response, msg_meas(addr, MessageType::GetFullResolutionSyncMeasurement);
		
		SendAndWaitForResponse(pserial, msg_meas, msg_response, 5000);
		assert(msg_response.getType() == MessageType::GetFullResolutionSyncMeasurement || msg_response.getAddress() == addr);
		
		int payload_length = msg_response.getPayloadLength();
		printf("Payload length = %d\n", payload_length);
		const uint16_t* ptr = (const uint16_t*)msg_response.getPayload();
		
		IDBG_ShowImage("obrazek", 10, 10, ptr, "U16");
		
		for (int i = 0; i < 10; i++)
		{
			for (int i = 0; i < 10; i++)
				printf("%5d ", *ptr++);
			printf("\n");
		}
		
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
	*/
	
	
	return 0;
}

const Location* getLocationByAddress(device_address_t addr, std::vector<TableGroup::Ptr>& tgroups)
{
	for(const TableGroup::Ptr& pgroup : tgroups)
		for(const TableDevice::Ptr& pdevice : *pgroup)
			if (pdevice->getAddress() == addr)
				return &pdevice->getLocation();
				
	return nullptr;
}

void AcquireFullImage(std::vector<TableGroup::Ptr>& tgroups, Image& img)
{
	int32_t timeout = 200;

	
	//
	// discard old data
	for(TableGroup::Ptr& pgroup : tgroups)
	{
		SerialPort::Ptr pserial = pgroup->getSerialPort();
		pserial->discardAllData();
		pgroup->getReceiver().purgeAllData();
	}
	
	//
	// send broadcast message
	Message msg(ADDRESS_BROADCAST, MessageType::DoBurstMeasurement);
	int device_count = 0;
	for(TableGroup::Ptr& pgroup : tgroups)
	{
		SerialPort::Ptr pserial = pgroup->getSerialPort();
		pserial->send(msg.getBinary(), msg.getBinaryLength());
		device_count += pgroup->getDeviceCount();
	}
		
	std::chrono::time_point<std::chrono::steady_clock> start_time = std::chrono::steady_clock::now();

	do {
		fd_set rfd;
		FD_ZERO(&rfd);
		
		// add all serial ports to read list
		int max_fd = 0;
		for(TableGroup::Ptr& pgroup : tgroups)
		{
			SerialPort::Ptr pserial = pgroup->getSerialPort();
			FD_SET(pserial->getHandle(), &rfd);
			max_fd = std::max(max_fd, pserial->getHandle());
		}
		
		// wait for data
		timeval tv = { .tv_sec = 0, .tv_usec = 75 * 1000 };
		int sret = ::select(max_fd + 1, &rfd, nullptr, nullptr, &tv);
		
		if (sret == 0)
			goto _check_timeout; // timeout - let's loop		
			
		if (sret == -1) {
			perror(__func__);
			Environment::terminateOnError("select", 0);
		}			
		
		// got some data
		for(TableGroup::Ptr& pgroup : tgroups)
		{
			SerialPort::Ptr pserial = pgroup->getSerialPort();
			if (!FD_ISSET(pserial->getHandle(), &rfd))
				continue;
				
			// put data int
			MessageReceiver& receiver = pgroup->getReceiver();
			receiver.receive(*pserial);
			
			// check for complete message
			Message response;
			while (receiver.getMessage(response))
			{
				const Location* ploc = getLocationByAddress(response.getAddress(), tgroups);
				img.processMeasurementPayload(response.getPayload(), pgroup->getBitsPerPoint(), ploc);
				device_count--;
			}
		}
		
		if (device_count == 0)
			break;
		
		
_check_timeout:;
		std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count() > timeout)
			{
				printf("TO(%d)", device_count);
				break;
			}
		
	} while(true);
	
	printf("Got all images!\n");
	IDBG_ShowImage("obrazek", 4*10, 6*10, img.getData(), "U16");

	std::this_thread::sleep_for(std::chrono::milliseconds(10));

}


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

