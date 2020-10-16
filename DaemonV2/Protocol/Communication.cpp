//
// Created by Tomek on 10/16/2020.
//

#include "Communication.hpp"

#include <chrono>
#include "InputMessageBuilder.hpp"
#include "TimeoutError.h"

void Communication::Transcive(SerialPort::Ptr serial, const OutputMessage& query, InputMessage& response, int timeout) {

    if (serial == nullptr)
        throw std::invalid_argument("serial");

    InputMessageBuilder mr;
    std::array<uint8_t, 256> recv_buffer;

    serial->DiscardAllData();
    serial->Send(query.GetDataPointer(), query.GetDataCount());

    std::chrono::time_point<std::chrono::steady_clock> start_time = std::chrono::steady_clock::now();

    // now wait for response
    int loops = 0;
    ssize_t ss;
    do {
        fd_set rfd;
        FD_ZERO(&rfd);
        FD_SET(serial->GetHandle(), &rfd);

        loops++;
        timeval tv = {.tv_sec = 0, .tv_usec = 50 * 1000};
        int sret = ::select(serial->GetHandle() + 1, &rfd, nullptr, nullptr, &tv);

        if (sret == -1)
            throw std::runtime_error("select");

        if (sret > 0) {
            ssize_t recv_bytes = serial->Receive(recv_buffer);
            mr.AddCollectedData(recv_buffer, 0, recv_bytes);

            if (mr.Extractmessage(response) == MessageExtractionResult::Ok)
                break;
        }

        // check for timeout
        //_check_timeout:;
        std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count() > timeout) {
            throw TimeoutError(serial->GetPortName());
        }

    } while (true);
}

void Communication::SendToMultiple(const std::vector<SerialPort::Ptr>& serialPortCollection, const OutputMessage& query) {

    for (SerialPort::Ptr pserial : serialPortCollection) {
        pserial->DiscardAllData();
        pserial->Send(query.GetDataPointer(), query.GetDataCount());
    }
}

