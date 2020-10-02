#include <system_error>
#include <cassert>

#include "ProgramBase.hpp"
#include "SerialPort.hpp"
#include <list>
#include "TableDevice.hpp"
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

    printf("Available serial ports (%zu): %s\n", port_name_list.size(), s.c_str());
}

std::list<SerialPort::Ptr> App::OpenAllSerialPorts(void) {
    std::list<SerialPort::Ptr> ports;
    for (const auto& pname : SerialPort::GetSerialDevices()) {
        try {
            printf("Opening port %s... ", pname.c_str());
            SerialPort::Ptr sp(new SerialPort(pname, 19200));
            ports.push_back(sp);

            printf(AGREEN "Done.\n" ARESET);
        } catch (const std::exception &ex)
        {
            printf(ARED "FAILED: " AYELLOW "%s\n" ARESET, ex.what());
        }
    }
    return ports;
}
/*


*/
#include "PhotoModule.hpp"
#include "InputMessageBuilder.hpp"

#include "OutputMessage.hpp"
#include "TimeoutError.h"
bool SendAndWaitForResponse(SerialPort::Ptr serial, const OutputMessage& query, InputMessage& response, int timeout) {
    InputMessageBuilder mr;


    serial->DiscardAllData();
    serial->Send(query.GetDataPointer(), query.GetDataCount());
    //dump(query.getDataPointer(), query.getDataCount());

    std::chrono::time_point<std::chrono::steady_clock> start_time = std::chrono::steady_clock::now();

    // now wait for response
    int loops = 0;
    ssize_t ss;
    do {
        fd_set rfd;
        FD_ZERO(&rfd);
        FD_SET(serial->GetHandle(), &rfd);

        loops++;
        timeval tv = {.tv_sec = 0, .tv_usec = 50 * 1000};
        int sret = ::select(serial->GetHandle() + 1, &rfd, nullptr, nullptr, &tv);

        if (sret == -1) {
            perror(__func__);
            throw std::runtime_error("select");
        }

        if (sret > 0) {

            std::array<uint8_t, 3> recv_buffer;
            ssize_t recv_bytes = serial->Receive(recv_buffer);
            mr.AddCollectedData(recv_buffer, 0, recv_bytes);
//		printf("mr.receive=%d\n", ss);
            //ssize_t bytes_read = ::read(serial.getHandle(), buffer + position, sizeof(buffer) - position);
            //assert(bytes_read > 0);
            //position += bytes_read;

            if (mr.GetMessage(response))
                return true;

        }

        // check for timeout
        _check_timeout:;
        std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count() > timeout) {
            char buffer[256];
            //   sprintf(buffer, "Timeout (SendAndWaitForResponse): addr=%02X; command=%d; recvd=%d; loops=%d",
            //           query.getAddress(), static_cast<int>(query.getType()),
            //           mr.getDataCount(), loops);
            throw TimeoutError(std::string(buffer));
        }

    } while (true);

    return false;
}


int App::Main(const std::vector<std::string>& arguments) {

    //
    // Intro
    printf("Smart Table Reconstruction Daemon, by Tomasz Jaworski, 2017\n");
    printf("Built on %s @ %s %lu\n\n", __DATE__, __TIME__, sizeof(void*));
    setbuf(stdout, NULL);

    //
    // Show available serial ports
    this->ShowAvailableSerialPorts();

    //
    // Open all serial ports
    std::list<SerialPort::Ptr> ports = this->OpenAllSerialPorts();

    //
    // Initialize table description
    TableDevice tdev;
    tdev.ShowTopology();


    SerialPort::Ptr sp = ports.front();
    sp->DiscardAllData();

    std::array<uint8_t, 5>  buffer1 = {0xAB, 0x04, 0x00, 0x00, 0x00};
    std::array<uint8_t, 5>  buffer2  = {0xAB, 0x02, 0x00, 0x00, 0x00};
    sp->Send(buffer1);
    sp->Send(buffer2);
    sp->Send(buffer1);
    sp->Send(buffer2);

    std::array<uint8_t, 100> b{};
    InputMessageBuilder mr;

    int ret;
    ret = sp->Receive(b);
    mr.AddCollectedData(b, 0, ret);
    ret = sp->Receive(b);
    mr.AddCollectedData(b, 0, ret);

    InputMessage msg;
    bool status;
    status = mr.GetMessage(msg);
    status = mr.GetMessage(msg);
    status = mr.GetMessage(msg);
    status = mr.GetMessage(msg);

    OutputMessage q(MessageType::DeviceIdentifierRequest);
    while(1) {
        bool result;
        try {
            result = SendAndWaitForResponse(sp, q, msg, 500);
            printf("%s", result ? "+" : ".");
        } catch(const TimeoutError& te) {
            printf("t");
        }
    }
    return 0;
}

int main(int argc, const char** argv, const char** env) {
    App app(argc, argv, env);
    app.Run();
    return app.GetErrorCode();
}
