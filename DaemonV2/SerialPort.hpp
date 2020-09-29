//
// Created by Tomek on 29.09.2020.
//

#ifndef UNTITLED_SERIALPORT_HPP
#define UNTITLED_SERIALPORT_HPP

#include <vector>
#include <string>

#include <memory>


class SerialPort {
    std::string port_name;
    int fd;
    uint64_t bytes_sent, bytes_received;

public:
    typedef std::shared_ptr<SerialPort> Ptr;

    const std::string& GetPortName(void) const { return this->port_name; }
    void DiscardAllData(void);


public:
    SerialPort(const std::string& deviceName);
    SerialPort(const SerialPort& sp);
    SerialPort(SerialPort&& sp);
    ~SerialPort(void);

    SerialPort& operator=(SerialPort&& sp);

    void Close(void);

    int Send(const void* data, size_t length);
    int Receive(void* data, size_t capacity);


    int GetHandle(void) const { return this->fd; }


public:
    static std::vector<std::string> GetSerialDevices(void);

};


#endif //UNTITLED_SERIALPORT_HPP
