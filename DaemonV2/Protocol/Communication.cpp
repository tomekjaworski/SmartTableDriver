//
// Created by Tomek on 10/16/2020.
//

#include <chrono>
#include <algorithm>

#include "Communication.hpp"
#include "InputMessageBuilder.hpp"
#include "TimeoutError.h"
#include "../Utility/Helper.hpp"


void Communication::Transcive(SerialPort::Ptr serial, const OutputMessage& query, InputMessage& response, int timeout) {

    if (serial == nullptr)
        throw std::invalid_argument("serial");

    InputMessageBuilder mr;
    std::array<uint8_t, 256> recv_buffer;

    serial->DiscardAllData();
    serial->Send(query.GetDataPointer(), query.GetDataCount());
    //Helper::HexDump(query.GetDataPointer(), query.GetDataCount());

    std::chrono::time_point<std::chrono::steady_clock> start_time = std::chrono::steady_clock::now();

    // now wait for response
    int loops = 0;
    //ssize_t ss;
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

            if (mr.ExtractMessage(response) == MessageExtractionResult::Ok)
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

void Communication::SendToMultiple(const std::list<SerialPort::Ptr>& serialPortCollection, const OutputMessage& query) {

    for (SerialPort::Ptr pserial : serialPortCollection) {
        pserial->DiscardAllData();
        pserial->Send(query.GetDataPointer(), query.GetDataCount());
    }
}

#include <map>
#include <unordered_set>
std::list<InputMessage> Communication::SendToMultipleAndWaitForResponse(const std::list<SerialPort::Ptr>& serialPortCollection, const OutputMessage& query, int timeout, bool& timeoutOccured) {

    std::map<int, InputMessageBuilder> fd2builder;
    std::map<int, InputMessage> fd2message;
    std::map<int, SerialPort::Ptr> fd2serial;


    for (SerialPort::Ptr pserial : serialPortCollection) {
        fd2builder[pserial->GetHandle()] = InputMessageBuilder();
        fd2serial[pserial->GetHandle()] = pserial;
    }

    for (SerialPort::Ptr pserial : serialPortCollection) {
        pserial->DiscardAllData();
        pserial->Send(query.GetDataPointer(), query.GetDataCount());
    }


    std::array<uint8_t, 256> recv_buffer;

    std::chrono::time_point<std::chrono::steady_clock> start_time = std::chrono::steady_clock::now();

    // now wait for response
    int loops = 0;
    //ssize_t ss;
    do {
        fd_set rfd;
        FD_ZERO(&rfd);

        int max_handle = INT32_MIN;
        for (const std::pair<int, SerialPort::Ptr>& item : fd2serial) {
            FD_SET(item.second->GetHandle(), &rfd);
            max_handle = std::max(max_handle, item.second->GetHandle());
        }

        loops++;
        timeval tv = {.tv_sec = 0, .tv_usec = 50 * 1000};
        int sret = ::select(max_handle + 1, &rfd, nullptr, nullptr, &tv);

        if (sret == -1)
            throw std::runtime_error("select");

        if (sret > 0) {

            for (const std::pair<int, SerialPort::Ptr>& item : fd2serial) {
                SerialPort::Ptr pserial = item.second;
                if (!FD_ISSET(pserial->GetHandle(), &rfd))
                    continue;

                ssize_t recv_bytes = pserial->Receive(recv_buffer);

                auto& builder = fd2builder[pserial->GetHandle()];
                builder.AddCollectedData(recv_buffer, 0, recv_bytes);

                InputMessage response;
                if (builder.ExtractMessage(response) == MessageExtractionResult::Ok)
                    fd2message[pserial->GetHandle()] = response;

                // all devices have sent correct response?
                if (fd2message.size() == fd2serial.size())
                {
                    std::list<InputMessage> result;
                    std::transform(fd2message.begin(), fd2message.end(), std::back_inserter(result), [](const auto&p){ return p.second;});
                    timeoutOccured = false;
                    return result;
                }
            }
        }

        // check for timeout
        //_check_timeout:;
        std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count() > timeout) {
            break;
            //throw TimeoutError("SendToMultipleAndWaitForResponse");
        }

    } while (true);

    // return all messages that were received; the result set will NOT be complete
    std::list<InputMessage> result;
    std::transform(fd2message.begin(), fd2message.end(), std::back_inserter(result), [](const auto&p){ return p.second;});
    timeoutOccured = false;
    return result;
}

