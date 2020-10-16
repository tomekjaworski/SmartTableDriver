//
// Created by Tomek on 10/16/2020.
//

#ifndef UNTITLED_COMMUNICATION_HPP
#define UNTITLED_COMMUNICATION_HPP

#include <vector>
#include "../SerialPort/SerialPort.hpp"
#include "OutputMessage.hpp"
#include "InputMessage.hpp"


class Communication {

public:
    static void Transcive(SerialPort::Ptr serial, const OutputMessage& query, InputMessage& response, int timeout);
    static void SendToMultiple(const std::vector<SerialPort::Ptr>& serialPortCollection, const OutputMessage& query);
};


#endif //UNTITLED_COMMUNICATION_HPP
