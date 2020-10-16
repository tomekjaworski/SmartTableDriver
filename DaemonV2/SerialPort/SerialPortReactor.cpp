//
// Created by Tomek on 29.09.2020.
//

#include "SerialPortReactor.hpp"


SerialPortReactor::SerialPortReactor(void) {

}


SerialPortReactor::~SerialPortReactor() {

}

void SerialPortReactor::RegisterSerialPort(SerialPort::Ptr pserial) {
    const auto it = std::find_if(this->ports.begin(), this->ports.end(), [pserial](SerialPort::Ptr p) {
        return (p->GetPortName() == pserial->GetPortName());
    });
    if (it == this->ports.end())
        this->ports.push_back(pserial);
    else
        throw std::logic_error("SerialPortReactor::RegisterSerialPort port already registered.");
}

void SerialPortReactor::RemoveSerialPort(SerialPort::Ptr pserial) {
    const auto it = std::find_if(this->ports.begin(), this->ports.end(), [pserial](SerialPort::Ptr p) {
        return (p->GetPortName() == pserial->GetPortName());
    });
    if (it != this->ports.end())
        this->ports.erase(it);
}

void SerialPortReactor::DiscardAllReceivedData(void) {
    for(SerialPort::Ptr port : this->ports) {
        port->DiscardAllData();
    }
}

SerialPort::Ptr SerialPortReactor::GetPortByName(const std::string& portName) {
    for(SerialPort::Ptr port : this->ports) {
        if (port->GetPortName() == portName)
            return port;
    }
    return nullptr;
}

void SerialPortReactor::SendToAll(const void* buffer, int32_t count) {
    for(SerialPort::Ptr port : this->ports) {
        port->Send(buffer, count);
    }
}