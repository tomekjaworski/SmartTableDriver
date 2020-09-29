//
// Created by Tomek on 29.09.2020.
//

#ifndef UNTITLED_SERIALPORTREACTOR_HPP
#define UNTITLED_SERIALPORTREACTOR_HPP

#include "SerialPort.hpp"


class SerialPortReactor {

    std::vector<SerialPort::Ptr> ports;

public:
    SerialPortReactor(void);
    ~SerialPortReactor();

    void RegisterSerialPort(SerialPort::Ptr pserial);
    void RemoveSerialPort(SerialPort::Ptr pserial);

    void DiscardAllReceivedData(void);

    SerialPort::Ptr GetPortByName(const std::string& portName);
};


#endif //UNTITLED_SERIALPORTREACTOR_HPP
