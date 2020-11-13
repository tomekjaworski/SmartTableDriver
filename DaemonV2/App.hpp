#if !defined(_APP_HPP_)
#define _APP_HPP_

#include "ProgramBase.hpp"
#include <list>

#include "SerialPort/SerialPort.hpp"


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

#endif // _APP_HPP_
