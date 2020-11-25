//
// Created by Tomek on 29.09.2020.
//

#include "SerialPort.hpp"
#include <cassert>

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

#include <filesystem>
#include <regex>
namespace fs = std::filesystem;



using namespace std::string_literals;

SerialPort::SerialPort(const std::string& deviceName, int baudRate)
    : fd(-1), bytes_sent(0), bytes_received(0)
{
    if (deviceName.empty())
        throw std::invalid_argument("deviceName");

    this->port_name = deviceName;
    this->baud_rate = baudRate;
    this->impl_Open(deviceName, baudRate, true);
}

SerialPort::~SerialPort(void)
{
    if (this->fd != -1)
        this->Close();
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


void SerialPort::impl_Open(const std::string& portName, int baudRate, bool showDebug) {

    this->fd = open(portName.c_str(), O_RDWR | O_NOCTTY);
    if (this->fd == -1)
        throw std::runtime_error("Error opening serial port device "s + portName);

#if 0
    if (this->fd == -1)
    {
        if (fake_serial_port)
            this->fd = open("/dev/null", O_RDWR | O_NOCTTY | O_NDELAY);
    } else
        fake_serial_port = false;
#endif

#ifndef __CYGWIN__
    struct termios2 tio;
    int ret = ioctl(fd, TCGETS2, &tio);
    if (ret != 0)
        throw std::system_error(errno, std::system_category(), "ioctl (1)");

    tio.c_cflag &= ~(CBAUD | CBAUDEX);
    tio.c_cflag |= BOTHER;
    tio.c_ispeed = tio.c_ospeed = baudRate;

    ret = ioctl(fd, TCSETS2, &tio);
    if (ret != 0)
        throw std::system_error(errno, std::system_category(), "ioctl (1)");
#endif


    struct termios ser;
    int ret2 = tcgetattr(this->fd, &ser);
    if (ret2 != 0)
        throw std::system_error(errno, std::system_category(), "tcgetattr");

#ifdef __CYGWIN__
    speed_t speed = B0;

    if (baudRate == 9600)
        speed = B9600;
    else if (baudRate == 19200)
        speed = B19200;
    else if (baudRate == 57600)
        speed = B57600;
    else
        assert(false && "This baud rate was ignored during implementation");

    int ret = cfsetospeed(&ser, speed);
    if (ret != 0)
        throw std::system_error(errno, std::system_category(), "cfsetospeed");

    ret = cfsetispeed(&ser, speed); // set transmission speed same as outgoing
    if (ret != 0)
        throw std::system_error(errno, std::system_category(), "cfsetispeed");
#endif

    ser.c_cflag |= PARENB;	// enable parity checking/generation
    ser.c_cflag &= ~PARODD;	// !odd = even
    ser.c_cflag &= ~CSTOPB; // one stop bit

    ser.c_cflag |= CREAD;	// enable receiver

    ser.c_cflag &= ~CSIZE;	// clear bit number mask
    ser.c_cflag |= CS8;		// set 8 bits per byte

    ser.c_cflag &= ~CRTSCTS;
    ser.c_cflag |= CREAD | CLOCAL;

    ser.c_iflag &= ~(IXON | IXOFF | IXANY);
    ser.c_iflag &= ~(INLCR | ICRNL);
    ser.c_iflag |= IGNPAR;

    ser.c_lflag &= ~(ICANON | IEXTEN | ECHO | ECHOE | ISIG);
    ser.c_oflag &= ~OPOST;

    if (showDebug) {
        printf("  VTIME=%d; VMIN=%d\n", ser.c_cc[VTIME], ser.c_cc[VMIN]);
        printf("  c_cflag=%08x, c_iflag=%08x, c_oflag=%08x\n", ser.c_cflag, ser.c_iflag, ser.c_oflag);
        printf("  c_lflag=%08x, c_line=%08x\n", ser.c_lflag, ser.c_line);
    }

    //TODO: jak ustawić długość kolejki FIFO dla wejscia i wyjscia. Albo jak pobrac jej długość?


    // ustaw parametry
    ret = tcsetattr(this->fd, TCSANOW, &ser);
    if (ret == -1)
        throw std::system_error(errno, std::system_category(), "tcsetattr");

    // discard both buffers

    this->DiscardAllData();

    //printf("%s: this->fd = %d\n", this->port_name.c_str(), this->fd);
    //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}



void SerialPort::DiscardAllData(void)
{
    int ret = tcflush(this->fd, TCIOFLUSH);
    if (ret != 0) {
        int e = errno;
       // throw std::system_error(errno, std::system_category(), "tcflush");
    }
}

void SerialPort::Close(void)
{
    printf("*** Zamykanie %d\n", this->fd);
    this->impl_Close();
}

void SerialPort::impl_Close(void) {
    close(this->fd);
    this->fd = -1;
}


ssize_t SerialPort::Send(const void* data, size_t length) {
    return this->impl_Send(data, length);
}

ssize_t SerialPort::Receive(void* buffer, size_t capacity) {
    return this->impl_Receive(buffer, capacity);
}


/////////////////////////////////////////////////////////


ssize_t SerialPort::impl_Send(const void* data, size_t length)
{
    ssize_t sent = ::write(this->fd, data, length);
    if (sent == -1)
        perror("SerialPort::impl_Send");
        //throw std::system_error(errno, std::system_category(), "write");
    bytes_sent += sent;
    return sent;
}

ssize_t SerialPort::impl_Receive(void* data, size_t capacity)
{
    ssize_t recvd = ::read(this->fd, data, capacity);
    if (recvd == -1)
        throw std::system_error(errno, std::system_category(), "read");

    bytes_received += recvd;
    return recvd;
}


void SerialPort::Restart(void) {
    this->DiscardAllData();
    this->impl_Close();
    this->impl_Open(this->port_name, this->baud_rate, false);
    //

}


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

std::vector<std::string> SerialPort::GetSerialDevices(void)
{
    std::vector<std::string> names;
    char dev_name[64];

/*
    //
    // Znajdź wszystkie urządzenia, będące konwerterami USB/RS232
    const std::regex rx_ttyname("/dev/ttyUSB\\d+");
    std::vector<std::string> port_name_collection;
    for(const auto& entry : fs::directory_iterator("/dev")) {
        if (std::regex_match(entry.path().string(), rx_ttyname))
            port_name_collection.push_back(entry.path().string());
        std::cout << entry.path().string() << std::endl;
    }
*/

    const char* patterns[] = {"/dev/ttyUSB%d", "/dev/ttyS%d", NULL};

    for (int i = 0; patterns[i] != NULL; i++)
        for (int n = 0; n < 256; n++) {
            sprintf(dev_name, patterns[i], n);
            char target[1024];
            __attribute__((unused)) int len = readlink(dev_name, target, sizeof(target));

            int fd = open(dev_name, O_RDWR | O_NONBLOCK);
            if (fd != -1) {
                close(fd);
                names.push_back(dev_name);
            }
        }

    return names;
}

