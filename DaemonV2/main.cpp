#include <iostream>
#include <filesystem>
#include <regex>
#include <system_error>
#include <unistd.h>
#include <limits.h>
#include <cassert>
namespace fs = std::filesystem;

#include "ProgramBase.hpp"
#include "SerialPort.hpp"
#include <list>

#include "AnsiCodes.h"

class App : public ProgramBase {
public:
    App(int argc, const char** argv, const char** env)
        : ProgramBase(argc, argv, env) {

    }

    int Main(const std::vector<std::string>& arguments) override;

public:
    void ShowAvailableSerialPorts(void);
    std::list<SerialPort::Ptr> OpenAllSerialPorts(void);
};

void App::ShowAvailableSerialPorts(void) {
    std::string s = "";
    auto port_name_list = SerialPort::GetSerialDevices();
    for (const auto& pname : port_name_list) // może boost?
        if (!s.empty())
            s += ", " + pname;
        else
            s += pname;

    printf("Available serial ports (%d): %s\n", port_name_list.size(), s.c_str());
}

std::list<SerialPort::Ptr> App::OpenAllSerialPorts(void) {
    std::list<SerialPort::Ptr> ports;
    for (const auto& pname : SerialPort::GetSerialDevices()) {
        try {
            printf("Opening port %s... ", pname.c_str());
            SerialPort::Ptr sp(new SerialPort(pname));
            ports.push_back(sp);

            printf(AGREEN "Done.\n" ARESET);
        } catch (const std::exception &ex)
        {
            printf(ARED "FAILED: " AYELLOW "%s\n" ARESET, ex.what());
        }
    }
    return ports;
}

#include "TableDevice.hpp"

int App::Main(const std::vector<std::string>& arguments) {

    //
    // Intro
    printf("Smart Table Reconstruction Daemon, by Tomasz Jaworski, 2017\n");
    printf("Built on %s @ %s %d\n\n", __DATE__, __TIME__, sizeof(void*));
    setbuf(stdout, NULL);

    //
    // Show available serial ports
    this->ShowAvailableSerialPorts();

    //
    // Open all serial ports
    std::list<SerialPort::Ptr> ports = this->OpenAllSerialPorts();


    //
    // Initialize table description
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
    std::vector<TableDevice::Ptr> geometry = {
        std::make_shared<TableDevice>(0x01, Location(0, 0)),
        std::make_shared<TableDevice>(0x02, Location(1, 0)),
        std::make_shared<TableDevice>(0x03, Location(2, 0)),
        std::make_shared<TableDevice>(0x04, Location(3, 0)),
        std::make_shared<TableDevice>(0x05, Location(4, 0)),
        std::make_shared<TableDevice>(0x06, Location(5, 0)),
        std::make_shared<TableDevice>(0x07, Location(0, 1)),
        std::make_shared<TableDevice>(0x08, Location(1, 1)),
        std::make_shared<TableDevice>(0x09, Location(2, 1)),
        std::make_shared<TableDevice>(0x0A, Location(3, 1)),
        std::make_shared<TableDevice>(0x0B, Location(4, 1)),
        std::make_shared<TableDevice>(0x0C, Location(5, 1)),
        std::make_shared<TableDevice>(0x0D, Location(0, 2)),
        std::make_shared<TableDevice>(0x0E, Location(1, 2)),
        std::make_shared<TableDevice>(0x0F, Location(2, 2)),
        std::make_shared<TableDevice>(0x10, Location(3, 2)),
        std::make_shared<TableDevice>(0x11, Location(4, 2)),
        std::make_shared<TableDevice>(0x12, Location(5, 2)),
        std::make_shared<TableDevice>(0x13, Location(0, 3)),
        std::make_shared<TableDevice>(0x14, Location(1, 3)),
        std::make_shared<TableDevice>(0x15, Location(2, 3)),
        std::make_shared<TableDevice>(0x16, Location(3, 3)),
        std::make_shared<TableDevice>(0x17, Location(4, 3)),
        std::make_shared<TableDevice>(0x18, Location(5, 3)),
    };

    return 0;
}

int main(int argc, const char** argv, const char** env) {
    App app(argc, argv, env);
    app.Run();
    return app.GetErrorCode();
}
