//
// Created by Tomek on 29.09.2020.
//

#include "TableDevice.hpp"
#include <algorithm>


TableDevice::TableDevice(void) {
    /*
 *                     6   7
 *
 *  1  01--02--03--04  05  06
 *                     |   |
 *  2  07--08--09--0A  0B  0C
 *                     |   |
 *  3  0D--0E--0F--10  11  12
 *
 *  4  13--14--15  16--17--18  5
 *
 */
    this->geometry = {
            std::make_shared<PhotoModule>(0x11, Location(1, 1)),
            std::make_shared<PhotoModule>(0x12, Location(1, 2)),
            std::make_shared<PhotoModule>(0x13, Location(1, 3)),
            std::make_shared<PhotoModule>(0x14, Location(1, 4)),
            std::make_shared<PhotoModule>(0x15, Location(1, 5)),
            std::make_shared<PhotoModule>(0x16, Location(1, 6)),
            std::make_shared<PhotoModule>(0x21, Location(2, 1)),
            std::make_shared<PhotoModule>(0x22, Location(2, 2)),
            std::make_shared<PhotoModule>(0x23, Location(2, 3)),
            std::make_shared<PhotoModule>(0x24, Location(2, 4)),
            std::make_shared<PhotoModule>(0x25, Location(2, 5)),
            std::make_shared<PhotoModule>(0x26, Location(2, 6)),
            std::make_shared<PhotoModule>(0x31, Location(3, 1)),
            std::make_shared<PhotoModule>(0x32, Location(3, 2)),
            std::make_shared<PhotoModule>(0x33, Location(3, 3)),
            std::make_shared<PhotoModule>(0x34, Location(3, 4)),
            std::make_shared<PhotoModule>(0x35, Location(3, 5)),
            std::make_shared<PhotoModule>(0x36, Location(3, 6)),
            std::make_shared<PhotoModule>(0x41, Location(4, 1)),
            std::make_shared<PhotoModule>(0x42, Location(4, 2)),
            std::make_shared<PhotoModule>(0x43, Location(4, 3)),
            std::make_shared<PhotoModule>(0x44, Location(4, 4)),
            std::make_shared<PhotoModule>(0x45, Location(4, 5)),
            std::make_shared<PhotoModule>(0x46, Location(4, 6)),
    };

}

TableDevice::~TableDevice() {

}

Location TableDevice::GetLocationByID(device_identifier_t id) const {
    for(PhotoModule::Ptr pdevice : this->geometry)
        if (pdevice->GetID() == id)
            return pdevice->GetLocation();

    throw std::invalid_argument("ID not found");
}

PhotoModule::Ptr TableDevice::GetPhotoModuleByID(device_identifier_t id) {
    for(PhotoModule::Ptr pdevice : this->geometry)
        if (pdevice->GetID() == id)
            return pdevice;

    throw std::invalid_argument("ID not found"); // or null - into consideration...
}


void TableDevice::ShowTopology(void) {
    printf("Table device topology: \n");
    for(PhotoModule::Ptr pdevice : this->geometry) {
        printf("[ID=%d X=%d Y=%d] ",
               pdevice->GetID(),
               pdevice->GetLocation().GetColumn(),
               pdevice->GetLocation().GetRow());
    }
    printf("\n");
}

void TableDevice::SetSerialPort(PhotoModule::Ptr pmodule, SerialPort::Ptr pserial) {
    if (pmodule == nullptr || pserial == nullptr)
        throw std::invalid_argument("pmodule/pserial");

    pmodule->SetSerialPort(pserial);
    this->ports.push_back(pserial);
}

std::list<SerialPort::Ptr>& TableDevice::GetSerialPortCollection(void) {
    return this->ports;
}

std::list<PhotoModule::Ptr>& TableDevice::GetPhotoModulesCollection(void) {
    return this->geometry;

}

PhotoModule::Ptr TableDevice::GetPhotoModuleByLocation(int moduleColumn, int moduleRow) {
    for (PhotoModule::Ptr pdevice : this->geometry)
        if (pdevice->GetLocation().GetRow() == moduleRow &&
            pdevice->GetLocation().GetColumn() == moduleColumn)
            return pdevice;
    return nullptr;
}

void TableDevice::OnSerialPortClosed(SerialPort::Ptr pserial) {
    auto iter = std::find_if(std::begin(geometry), std::end(geometry), [&](PhotoModule::Ptr item){
        return item->GetSerialPort() == pserial;
    });
    if (iter == std::end(geometry))
        return; // ??

    (*iter)->SetSerialPort(nullptr);
    auto iter2 = std::find(std::begin(this->ports), std::end(this->ports), pserial);
    if (iter2 != std::end(this->ports))
        this->ports.erase(iter2);
}