//
// Created by Tomek on 29.09.2020.
//

#ifndef UNTITLED_INPUTMESSAGE_HPP
#define UNTITLED_INPUTMESSAGE_HPP

#include <cstdint>
#include <vector>
#include "../../TableFirmware/protocol.h"


class InputMessage {
    std::vector<uint8_t> payload;
    MessageType type;

public:
    InputMessage()
        : payload() {
        this->type = MessageType::Invalid;
    }
    InputMessage(const void* payloadBuffer, int32_t payloadSize)
        : payload(static_cast<const uint8_t*>(payloadBuffer),
               static_cast<const uint8_t*>(payloadBuffer) + payloadSize)
    {
        this->type = this->GetMessageType();
    }

    InputMessage(InputMessage&& msg) noexcept
        : payload(std::move(msg.payload)), type(msg.type)
    {
        //
    }

    InputMessage(const InputMessage&) = default;
    InputMessage& operator=(const InputMessage&) = default;


    template <typename T>
    const T* GetPayloadPointer(void) const {
        return reinterpret_cast<const T*>(this->internal_GetPayloadPointer());
    }


    MessageType GetMessageType(void) const;
    int GetPayloadSize(void) const;
    int GetSequenceNumber(void) const;
    device_identifier_t GetDeviceID (void) const;

private:
    const void* internal_GetPayloadPointer(void) const;
};


#endif //UNTITLED_INPUTMESSAGE_HPP
