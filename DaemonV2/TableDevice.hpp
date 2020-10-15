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
public:
    TableDevice(void);
    ~TableDevice();
public:
    Location GetLocationByID(device_identifier_t id) const;
    PhotoModule::Ptr GetPhotoModuleByID(device_identifier_t id);
    void ShowTopology(void);
};


#endif //UNTITLED_TABLEDEVICE_HPP
