#include <stdio.h>
#include <string.h>
#include <stdexcept>
#include <termios.h>
#include <fcntl.h>
#include <list>

#include "CRC.hpp"

#include "SerialPort.hpp"
#include "Environment.hpp"

#include "../../SmartTableFirmware/MessageType.h"
#include "../../SmartTableFirmware/protocol.h"

#include "Message.h"

#define AGREEN	"\e[32m"
#define ARED	"\e[31m"
#define AYELLOW	"\e[33m"
#define ARESET	"\e[0m"

int main(int argc, char **argv)
{
	printf("Smart Table Reconstruction Daemon, by Tomasz Jaworski, 2017\n");
	printf("Built on %s @ %s\n\n", __DATE__, __TIME__);
	
	std::string s = "";
	auto port_name_list = SerialPort::getSerialDevices();
	for (const auto& pname : port_name_list)
		if (!s.empty())
			s += ", " + pname;
		else
			s += pname;
			
	printf("Available serial ports (%d): %s\n", port_name_list.size(), s.c_str());
	std::list<SerialPort> ports;
	for (const auto& pname : port_name_list)
	{
		try
		{
			printf("Opening port %s... ", pname.c_str());
			SerialPort sp; 
			sp.init(pname+"0", false);
			ports.push_back(std::move(sp));
			
			printf(AGREEN "Done.\n" ARESET);
		} catch (const std::exception &ex)
		{
			printf(ARED "FAILED: " AYELLOW "%s\n" ARESET, ex.what());
		}
			
	}
	
	printf("xxxx");
	
	
	
	SerialPort sp;
	
	//sp.init("", false);
	
	
	//Message mping(0x50, MessageType::Ping);
	//sp.send(mping.getBinary(), mping.getBinaryLength());
	
	
	return 0;
}
