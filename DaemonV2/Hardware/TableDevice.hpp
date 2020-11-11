//
// Created by Tomek on 29.09.2020.
//

#ifndef UNTITLED_TABLEDEVICE_HPP
#define UNTITLED_TABLEDEVICE_HPP

#include <list>

#include "Location.hpp"
#include "PhotoModule.hpp"

class TableDevice {
    std::list<PhotoModule::Ptr> geometry;
    std::list<SerialPort::Ptr> ports;
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

    std::list<SerialPort::Ptr>& GetSerialPortCollection(void);
    std::list<PhotoModule::Ptr>& GetPhotoModulesCollection(void);

    PhotoModule::Ptr GetPhotoModuleByLocation(int moduleColumn, int moduleRow);
};


#endif //UNTITLED_TABLEDEVICE_HPP
