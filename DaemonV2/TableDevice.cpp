//
// Created by Tomek on 29.09.2020.
//

#include "TableDevice.hpp"

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
            std::make_shared<PhotoModule>(0x01, Location(0, 0)),
            std::make_shared<PhotoModule>(0x02, Location(1, 0)),
            std::make_shared<PhotoModule>(0x03, Location(2, 0)),
            std::make_shared<PhotoModule>(0x04, Location(3, 0)),
            std::make_shared<PhotoModule>(0x05, Location(4, 0)),
            std::make_shared<PhotoModule>(0x06, Location(5, 0)),
            std::make_shared<PhotoModule>(0x07, Location(0, 1)),
            std::make_shared<PhotoModule>(0x08, Location(1, 1)),
            std::make_shared<PhotoModule>(0x09, Location(2, 1)),
            std::make_shared<PhotoModule>(0x0A, Location(3, 1)),
            std::make_shared<PhotoModule>(0x0B, Location(4, 1)),
            std::make_shared<PhotoModule>(0x0C, Location(5, 1)),
            std::make_shared<PhotoModule>(0x0D, Location(0, 2)),
            std::make_shared<PhotoModule>(0x0E, Location(1, 2)),
            std::make_shared<PhotoModule>(0x0F, Location(2, 2)),
            std::make_shared<PhotoModule>(0x10, Location(3, 2)),
            std::make_shared<PhotoModule>(0x11, Location(4, 2)),
            std::make_shared<PhotoModule>(0x12, Location(5, 2)),
            std::make_shared<PhotoModule>(0x13, Location(0, 3)),
            std::make_shared<PhotoModule>(0x14, Location(1, 3)),
            std::make_shared<PhotoModule>(0x15, Location(2, 3)),
            std::make_shared<PhotoModule>(0x16, Location(3, 3)),
            std::make_shared<PhotoModule>(0x17, Location(4, 3)),
            std::make_shared<PhotoModule>(0x18, Location(5, 3)),
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