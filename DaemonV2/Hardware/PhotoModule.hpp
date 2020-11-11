#ifndef TABLEDEVICE_HPP
#define TABLEDEVICE_HPP

#include <memory>

#include "Location.hpp"
#include "../SerialPort/SerialPort.hpp"

#include "../../TableFirmware/protocol.h"

struct status_descriptor_t {
    int received_images;
    int last_sequence_number;
};

class PhotoModule {

public:
    typedef std::shared_ptr<PhotoModule> Ptr;
    constexpr static int ModuleHeight = 10;
    constexpr static int ModuleWidth = 10;

private:
    device_identifier_t id;
    Location location;
    SerialPort::Ptr sp;

    struct status_descriptor_t status;

public:
    const Location &GetLocation(void) const {
        return this->location;
    }

    struct status_descriptor_t &GetStatusDescriptor(void) {
        return this->status;
    }

    device_identifier_t GetID(void) const {
        return this->id;
    }

    const SerialPort::Ptr &GetSerialPort(void) const {
        return this->sp;
    }

    SerialPort::Ptr &GetSerialPort(void) {
        return this->sp;
    }

    void SetSerialPort(SerialPort::Ptr pserial) {
        this->sp = pserial;
    }

    PhotoModule(device_identifier_t id, Location loc)
            : id(id), location(loc), sp(nullptr), status({0, 0}) {
        //
    }

};

#endif // TABLEDEVICE_HPP
