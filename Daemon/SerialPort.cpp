
#ifndef __CYGWIN__
#include <sys/ioctl.h>
#define termios asmtermios
#include <asm/termbits.h>
#undef termios
#include <termios.h>
#else
#include <termios.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <thread>
#include <chrono>
#include <sstream>
#include <iostream>
#include <string>
#include <iostream>
#include "SerialPort.hpp"
#include "Environment.hpp"

#ifndef __CYGWIN__
#include <termios.h>
#endif
 

using namespace std::string_literals;

SerialPort::SerialPort(void)
{
	this->fd = -1;
}

SerialPort::~SerialPort(void)
{
	if (this->fd != -1)
		this->done();
}

SerialPort::SerialPort(const SerialPort& sp)
	: port_name(sp.port_name), fd(sp.fd)
{
	// ?
}

SerialPort::SerialPort(SerialPort&& sp)
	: port_name(sp.port_name), fd(sp.fd)
{
	sp.fd = -1;
	sp.port_name = "";
}

SerialPort& SerialPort::operator=(SerialPort&& sp)
{
	this->fd = sp.fd;
	this->port_name = sp.port_name;
	sp.fd = -1;
	sp.port_name = "";
	
	return *this;
}


void SerialPort::init(const std::string& device_name, bool fake_serial_port)
{

	std::string fixed_ports[] = {
		"/dev/ttyS2",		// com3 na moim kompie
		"/dev/ttyUSB0",		// Raspberry Pi 2 - PL2303 na USB
		"/dev/ttyAMA0",		// Raspberry Pi 2 - USART0
		"/dev/ttyS0"};		// COM1 na moim kompie
	
	if (!device_name.empty())
	{
		this->port_name = device_name;
		
		this->fd = open(device_name.c_str(), O_RDWR | O_NOCTTY);
		if (this->fd == -1)
			throw std::runtime_error("Error opening serial port device "s + device_name);
	} else
	{
		for (const std::string& pname : fixed_ports)
		{
			this->fd = open(pname.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
			if (this->fd != -1)
			{
				this->port_name = pname;
				break;
			}
		}
	}
	
	if (this->fd == -1)
	{
		if (fake_serial_port)
			this->fd = open("/dev/null", O_RDWR | O_NOCTTY | O_NDELAY);
	} else
		fake_serial_port = false;
			

	if (this->fd == -1)
		throw std::runtime_error("Error opening serial port device " + device_name);

	if (fake_serial_port)
	{
		this->port_name = "/fakeSER0";
		std::cout << "Entering fake serial port mode... " << std::endl;
	} else
	{
		int ret;
		
		struct termios ser;
		tcgetattr(this->fd, &ser);
		
		speed_t speed = B19200;
		ret = cfsetospeed(&ser, speed);
		if (ret == -1) Environment::terminateOnError("cfsetospeed", 2);
			
		ret = cfsetispeed(&ser, B0); // set transmission speed same as outgoing
		if (ret == -1) Environment::terminateOnError("cfsetispeed", 3);
		/*
#ifndef __CYGWIN__
		struct termios2 tio;
		ret = ioctl(fd, TCGETS2, &tio);
		if (ret != 0) Environment::terminateOnError("ioctl", 4);
		
		tio.c_cflag &= ~CBAUD;
		tio.c_cflag |= BOTHER;
		tio.c_ispeed = tio.c_ospeed = 19200;

		ret = ioctl(fd, TCSETS2, &tio);		
		if (ret != 0) Environment::terminateOnError("ioctl", 5);
#endif
		*/
		
		ser.c_cflag |= PARENB;	// enable parity checking/generation
		ser.c_cflag &= ~PARODD;	// !odd = even
		ser.c_cflag &= ~CSTOPB; // one stop bit

		ser.c_cflag |= CREAD;	// enable receiver

		ser.c_cflag &= ~CSIZE;	// clear bit number mask
		ser.c_cflag |= CS8;		// set 8 bits per byte

		ser.c_cflag &= ~CRTSCTS;
		ser.c_cflag |= CREAD | CLOCAL;
		
		ser.c_iflag &= ~(IXON | IXOFF | IXANY);
		ser.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);
		ser.c_oflag &= ~OPOST;

		// dokumentcja jest niezwykle OBSZERNA na temat tych opcji....
		//ser.c_cflag |= CLOCAL;
		////ser.c_lflag = ICANON;
		//ser.c_oflag &= ~OPOST; 
		
		//cfmakeraw(&ser);
		
		//TODO: jak ustawić długość kolejki FIFO dla wejscia i wyjscia. Albo jak pobrac jej długość?
		
	
		// ustaw parametry
		ret = tcsetattr(this->fd, TCSANOW, &ser);
		if (ret == -1) Environment::terminateOnError("tcsetattr", 6);
		
		// discart both buffers
		this->discardAllData();
	}
	//printf("%s: this->fd = %d\n", this->port_name.c_str(), this->fd);
	//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

void SerialPort::discardAllData(void)
{
	int ret = tcflush(this->fd, TCIOFLUSH);
	//printf("tcflush: fd=%d; ret=%d\n", this->fd, ret);
	if (ret == -1)
		Environment::terminateOnError("tcflush", 5);

}

void SerialPort::done(void)
{
	printf("SerialPort::done\n");
	close(this->fd);
	this->fd = -1;
}

int SerialPort::send(const void* data, size_t length)
{
	int sent = ::write(this->fd, data, length);
	bytes_sent += sent;
	return sent;
}

int SerialPort::receive(void* data, size_t capacity)
{
	int recvd = ::read(this->fd, data, capacity);
	bytes_received += recvd;
	return recvd;
}


/////////////////////////////////////////////////////////


std::vector<std::string> SerialPort::getSerialDevices(void)
{
	std::vector<std::string> names;
	
	for (int n = 0; n < 255; n++)
	{
		char dev_name[64];
#ifndef __CYGWIN__
		// odroid
		sprintf(dev_name, "/dev/ttyUSB%d", n);
#else
		sprintf(dev_name, "/dev/ttyS%d", n);
#endif	
		char target[1000];
		__attribute__((unused)) int len = readlink(dev_name, target, sizeof(target));
			
		int fd = open(dev_name, O_RDWR | O_NONBLOCK);
		if (fd != -1)
		{
			close(fd);
			names.push_back(dev_name);
		}
	}
	
	return names;
}
