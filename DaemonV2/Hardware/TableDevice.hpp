//
// Created by Tomek on 29.09.2020.
//

#ifndef UNTITLED_TABLEDEVICE_HPP
#define UNTITLED_TABLEDEVICE_HPP

#include <vector>

#include "Location.hpp"
#include "PhotoModule.hpp"

class TableDevice {
    std::vector<PhotoModule::Ptr> geometry;
    std::vector<SerialPort::Ptr> ports;
public:
    static constexpr int TableWidth = 60;
    static constexpr int TableHeight = 40;


    TableDevice(void);
    ~TableDevice();
public:
    Location GetLocationByID(device_identifier_t id) const;
    void ShowTopology(void);

    PhotoModule::Ptr GetPhotoModuleByID(device_identifier_t id);
    void SetSerialPort(PhotoModule::Ptr pmodule, SerialPort::Ptr pserial);

    std::vector<SerialPort::Ptr>& GetSerialPortCollection(void);
    std::vector<PhotoModule::Ptr>& GetPhotomodulesCollection(void);
};


#endif //UNTITLED_TABLEDEVICE_HPP
