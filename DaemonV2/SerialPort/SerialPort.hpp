//
// Created by Tomek on 29.09.2020.
//

#ifndef UNTITLED_SERIALPORT_HPP
#define UNTITLED_SERIALPORT_HPP

#include <vector>
#include <string>
#include <array>
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
    SerialPort(const std::string& deviceName, int baudRate);
    SerialPort(const SerialPort& sp);
    SerialPort(SerialPort&& sp);
    ~SerialPort(void);

    SerialPort& operator=(SerialPort&& sp);

    void Close(void);

    ssize_t Send(const void* data, size_t length);
    ssize_t Receive(void* data, size_t capacity);

    template <typename T, std::size_t N> ssize_t Send(const std::array<T, N>& buffer);
    template <typename T, std::size_t N> ssize_t Receive(std::array<T, N>& buffer);


    int GetHandle(void) const { return this->fd; }

private:
    ssize_t impl_Send(const void* buffer, size_t bufferLength);
    ssize_t impl_Receive(void* buffer, size_t bufferCapacity);

public:
    static std::vector<std::string> GetSerialDevices(void);


};

template <typename T, std::size_t N> ssize_t SerialPort::Send(const std::array<T, N>& buffer) {
    ssize_t sent = this->impl_Send(buffer.data(), buffer.size());
    return sent;
}


template <typename T, std::size_t N> ssize_t SerialPort::Receive(std::array<T, N>& buffer) {
    ssize_t recvd = this->impl_Receive(buffer.data(), buffer.size());
    return recvd;
}



#endif //UNTITLED_SERIALPORT_HPP
