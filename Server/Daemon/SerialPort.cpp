#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <thread>
#include <chrono>
#include <sstream>
#include <iostream>

#include "SerialPort.hpp"
#include "Environment.hpp"


SerialPort::SerialPort(void)
{
	this->fd = -1;
}

void SerialPort::init(const std::string& device_name, bool fake_serial_port)
{

	std::string fixed_ports[] = {
		"/dev/ttyS2",	// com3 na moim kompie
		"/dev/ttyUSB0",	// Raspberry Pi 2 - PL2303 na USB
		"/dev/ttyAMA0",	// Raspberry Pi 2 - USART0
		"/dev/ttyS0",	// COM1 na moim kompie
		NULL};
	
	if (!device_name.empty())
	{
		this->port_name = device_name;
		
		this->fd = open(device_name.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
		if (this->fd != -1)
			Environment::terminateOnError(std::string("Error opening serial port device ") + device_name, 1);
	}
		
	for (const std::string& pname : fixed_ports)
	{
		this->fd = open(pname.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
		if (this->fd != -1)
		{
			this->port_name = pname;
			break;
		}
	}
		
	if (this->fd == -1)
	{
		if (fake_serial_port)
			this->fd = open("/dev/null", O_RDWR | O_NOCTTY | O_NDELAY);
	} else
		fake_serial_port = false;
			

	if (this->fd == -1)
		Environment::terminateOnError(std::string("Error opening serial port device ") + device_name, 1);

	if (fake_serial_port)
	{
		this->port_name = "/fakeSER0";
		std::cout << "Entering fake serial port mode... " << std::endl;
	} else
	{
		int ret = fcntl(this->fd, F_SETFL, NULL);
		if (ret == -1) Environment::terminateOnError("fcntl", 2);
		
		struct termios ser;
		tcgetattr(this->fd, &ser);
		
		speed_t speed = B115200;
		ret = cfsetospeed(&ser, speed);
		if (ret == -1) Environment::terminateOnError("cfsetospeed", 2);
			
		ret = cfsetispeed(&ser, speed);
		if (ret == -1) Environment::terminateOnError("cfsetispeed", 3);
		
		ser.c_cflag &= ~PARENB; // brak parzystosci
		ser.c_cflag &= ~CSTOPB; // 1 bit stopu
		ser.c_cflag |= CREAD;

		ser.c_cflag &= ~CSIZE;	// o bitów na bajt
		ser.c_cflag |= CS8;

		// dokumentcja jest niezwykle OBSZERNA na temat tych opcji....
		ser.c_cflag |= CLOCAL;
		ser.c_lflag = ICANON;
		ser.c_oflag &= ~OPOST; 
		
		cfmakeraw(&ser);
		
		//TODO: jak ustawić długość kolejki FIFO dla wejscia i wyjscia. Albo jak pobrac jej długość?
		
	
		// ustaw parametry
		ret = tcsetattr(this->fd, TCSANOW, &ser);
		if (ret == -1) Environment::terminateOnError("tcsetattr", 4);
		
		// wyczysc bufory We i Wy
		ret = tcflush(this->fd, TCIOFLUSH);
		if (ret == -1) Environment::terminateOnError("tcflush", 5);
	}
	printf("%s: this->fd = %d\n", this->port_name.c_str(), this->fd);
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}


void SerialPort::done(void)
{
	close(this->fd);
	this->fd = -1;
}

