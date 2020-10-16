//
// Created by Tomek on 29.09.2020.
//

#ifndef UNTITLED_INPUTMESSAGE_HPP
#define UNTITLED_INPUTMESSAGE_HPP

#include <cstdint>
#include <vector>
#include "../../SmartTableFirmware/protocol.h"


class InputMessage {
    std::vector<uint8_t> data;

public:
    InputMessage()
        : data() {
        //
    }
    InputMessage(const void* buffer, int32_t count)
        : data(static_cast<const uint8_t*>(buffer),
               static_cast<const uint8_t*>(buffer) + count)
    {
        //
    }

    template <typename T>
    const T* GetPayloadPointer(void) const {
        return reinterpret_cast<const T*>(this->internal_GetPayloadPointer());
    }


    MessageType GetMessageType(void) const;
    int GetPayloadSize(void) const;
    int GetSequenceNumber(void) const;

private:
    const void* internal_GetPayloadPointer(void) const;
};


#endif //UNTITLED_INPUTMESSAGE_HPP
