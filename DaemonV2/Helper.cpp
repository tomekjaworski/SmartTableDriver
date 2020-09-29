//
// Created by Tomek on 29.09.2020.
//

#include "Helper.hpp"

#include <cstdio>

void Helper::HexDump(const void* ptr, int count)
{
    FILE* out = stdout;
    std::fprintf(out, "[");
    const uint8_t* p = static_cast<const uint8_t*>(ptr);
    for (int i = 0; i < count; i++)
        if (i < count - 1)
            std::fprintf(out, "%02x ", p[i]);
        else
            std::fprintf(out, "%02x]\n", p[i]);

    std::fflush(out);
}
