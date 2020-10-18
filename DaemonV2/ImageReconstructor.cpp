//
// Created by Tomek on 10/16/2020.
//

#include "ImageReconstructor.hpp"
#include "Hardware/PhotoModule.hpp"
#include <cassert>

static int photomodule_lookuptable[] = {
        61,	68, 75,	45, 44,	43, 42,	46, 47,	48,
        60,	67, 74,	38, 37,	36, 35,	39, 40,	41,
        59,	66, 73,	31, 30,	29, 28,	32, 33,	34,
        58,	65, 72,	24, 23,	22, 21,	25, 26,	27,
        62,	69, 76,	17, 16,	15, 14,	18, 19,	20,
        63,	70, 77,	10, 9,	8,  7,	11, 12,	13,
        64,	71, 78,	3,  2,	1,  0,	4,  5,	6,
        55,	56, 50,	96, 95,	94, 93,	97, 98,	99,
        51,	49, 57,	89, 88,	87, 86,	90, 91,	92,
        52,	53, 54,	82, 81,	80, 79,	83, 84,	85
};


void ImageReconstructor::ProcessMeasurementPayload(const void* vpayload, int bits, const Location& ploc)
{
    assert(bits == 16 || bits == 8); // other cases not implemented at the moment

    int xoffset = PhotoModule::ModuleWidth * (ploc.GetColumn() - 1);
    int yoffset = PhotoModule::ModuleHeight * (ploc.GetRow() - 1);

    if (bits == 16) {
        auto *upayload = reinterpret_cast<const std::uint16_t*>(vpayload);
        for (int x = 0; x < PhotoModule::ModuleWidth; x++)
            for (int y = 0; y < PhotoModule::ModuleHeight; y++) {
                int payload_index = x + y * PhotoModule::ModuleWidth;

                payload_index = photomodule_lookuptable[payload_index];
                uint16_t value = upayload[payload_index];

                data[xoffset + x + width * (yoffset + y)] = value;
            }
    } else {
        auto *upayload = reinterpret_cast<const std::uint8_t*>(vpayload);
        for (int x = 0; x < PhotoModule::ModuleWidth; x++)
            for (int y = 0; y < PhotoModule::ModuleHeight; y++) {
                int payload_index = x + y * PhotoModule::ModuleWidth;

                payload_index = photomodule_lookuptable[payload_index];
                uint16_t value = upayload[payload_index];

                data[xoffset + x + width * (yoffset + y)] = value;
            }
    }

}