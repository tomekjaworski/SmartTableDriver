//
// Created by Tomek on 10/16/2020.
//

#ifndef UNTITLED_COMMUNICATION_HPP
#define UNTITLED_COMMUNICATION_HPP

#include <list>
#include "../SerialPort/SerialPort.hpp"
#include "OutputMessage.hpp"
#include "InputMessage.hpp"


class Communication {

public:
    static void Transcive(SerialPort::Ptr serial, const OutputMessage& query, InputMessage& response, int timeout);
    static void SendToMultiple(const std::list<SerialPort::Ptr>& serialPortCollection, const OutputMessage& query);
    static std::list<InputMessage> SendToMultipleAndWaitForResponse(const std::list<SerialPort::Ptr>& serialPortCollection, const OutputMessage& query, int timeout, bool& timeoutOccured);
};


#endif //UNTITLED_COMMUNICATION_HPP
