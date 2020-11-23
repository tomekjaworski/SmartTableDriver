#include <cassert>

#include "Hardware/TableDevice.hpp"
#include "Utility/AnsiCodes.h"
#include <boost/algorithm/string.hpp>
#include "ImageReconstructor.hpp"
#include "Protocol/InputMessageBuilder.hpp"
#include "Protocol/TimeoutError.h"
#include "Protocol/Communication.hpp"
#include "Visualizer/ImageVisualizer.hpp"
#include "App.hpp"

namespace nc {
#include <curses.h>
}


void App::ShowAvailableSerialPorts(void) {
    std::string s;
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
            printf("Opening port %s...\n", pname.c_str());
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



//
//void Communication::Transcive(std::list<SerialPort::Ptr> serials, const OutputMessage& query, InputMessage& response, int timeout) {
//
//    if (serial == nullptr)
//        throw std::invalid_argument("serial");
//
//    InputMessageBuilder mr;
//    std::array<uint8_t, 256> recv_buffer;
//
//    serial->DiscardAllData();
//    serial->Send(query.GetDataPointer(), query.GetDataCount());
//
//    std::chrono::time_point<std::chrono::steady_clock> start_time = std::chrono::steady_clock::now();
//
//    // now wait for response
//    int loops = 0;
//    ssize_t ss;
//    do {
//        fd_set rfd;
//        FD_ZERO(&rfd);
//        FD_SET(serial->GetHandle(), &rfd);
//
//        loops++;
//        timeval tv = {.tv_sec = 0, .tv_usec = 50 * 1000};
//        int sret = ::select(serial->GetHandle() + 1, &rfd, nullptr, nullptr, &tv);
//
//        if (sret == -1)
//            throw std::runtime_error("select");
//
//        if (sret > 0) {
//            ssize_t recv_bytes = serial->Receive(recv_buffer);
//            mr.AddCollectedData(recv_buffer, 0, recv_bytes);
//
//            if (mr.ExtractMessage(response))
//                break;
//        }
//
//        // check for timeout
//        _check_timeout:;
//        std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
//        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count() > timeout) {
//            throw TimeoutError(serial->GetPortName());
//        }
//
//    } while (true);
//}

void sigpipe_handler(int unused)
{
    // ignoruj
    printf("Coś nie pykło.\n");
}

int App::Main(const std::vector<std::string>& arguments) {

    //
    // Intro
    printf("Smart Table Reconstruction Daemon, by Tomasz Jaworski, 2017\n");
    printf("Built on %s @ %s %lu\n\n", __DATE__, __TIME__, sizeof(void *));
    setbuf(stdout, NULL);
    struct sigaction handler;

    handler.sa_handler = sigpipe_handler;
    int result = sigaction(SIGPIPE, &handler, NULL);
    assert(result == 0);

    //
    // Show available serial ports
    this->ShowAvailableSerialPorts();

    //
    // Open all serial ports
    std::list<SerialPort::Ptr> ports = this->OpenAllSerialPorts();

    //
    // Reset all modules
    {
        printf("Rebooting all photomodules and trigger generator...\n");
        OutputMessage message_reboot = OutputMessage(MessageType::RebootRequest);
        for (int i = 0; i < 10; i++) {
            Communication::SendToMultiple(ports, message_reboot);
            usleep(100 * 1000);
        }

        // Wait some time for devices to fully reboot
        printf("Ok; waiting for devices to fully bootup...\n");
        usleep(20'000 * 1000);
    }



    //
    // Initialize table description
    TableDevice tdev;
    tdev.ShowTopology();

    SerialPort::Ptr trigger_generator_serial = nullptr;

    //
    // Initialize communication on each searial port and detect device id
    for (SerialPort::Ptr serial_port : ports) {

        OutputMessage message_ping = OutputMessage(MessageType::PingRequest);
        int successes = 0;
        bool last_was_ok;
        printf("Serial %s: ", serial_port->GetPortName().c_str());

        for (int i = 0; i < 5; i++) {
            usleep(50 * 1000);
            serial_port->DiscardAllData();

            InputMessage response;
            try {
                last_was_ok = false;
                printf(".");
                Communication::Transcive(serial_port, message_ping, response, 250);
            } catch (const TimeoutError &te) {
                printf("t");
                continue;
            }

            if (response.GetMessageType() != MessageType::PingResponse || response.GetPayloadSize() != 0) {
                printf("e");
                continue;
            }

            successes++;
            last_was_ok = true;
        }
        printf("\n");
        //
        // assess the communication
        if (successes == 0) {
            // The port is dead; there is probably no interesting equipment
            printf(AYELLOW "Serial port %s has NO compatible devices attached.\n" ARESET,
                   serial_port->GetPortName().c_str());
            continue;
        }

        if (successes < 3 || !last_was_ok) {
            // There were some successful communications, however not enough and what is more - last one has failed.
            printf(ARED "Serial port %s communication problems; intervention required.\n" ARESET,
                   serial_port->GetPortName().c_str());
            continue;
        }

        //
        // Ok, looks like we have a working photo device there.
        // Lets get its data
        InputMessage response;
        OutputMessage message_device_query = OutputMessage(MessageType::DeviceIdentifierRequest);

        try {
            last_was_ok = false;
            Communication::Transcive(serial_port, message_device_query, response, 250);
        } catch (const TimeoutError &te) {
            printf(ARED "Timeout in %s during DeviceIdentifierRequest\n" ARESET, serial_port->GetPortName().c_str());
            continue;
        }

        std::string device_info(response.GetPayloadPointer<char>(),
                                response.GetPayloadPointer<char>() + response.GetPayloadSize());


        std::vector<std::string> entries;
        boost::split(entries, device_info, boost::is_any_of(";"));
        std::map<std::string, std::string> info;
        for (const std::string &entry : entries) {
            std::vector<std::string> tokens;
            boost::split(tokens, entry, boost::is_any_of("="));
            if (tokens.size() == 2) {
                info[tokens[0]] = tokens[1];
            }
        }

        device_identifier_t devid = static_cast<device_identifier_t>(std::stoi(info["id"]));
        printf(AYELLOW "   Port %s, device 0x%02X: version=[%s], date=[%s], time=[%s]\n" ARESET,
               serial_port->GetPortName().c_str(),
               devid,
               info["version"].c_str(),
               info["date"].c_str(),
               info["time"].c_str()
        );

        // Trigger generator, while in the same adressing space, will be treated differently.
        if (devid == 0x1F) // Trigger Generator?
        {
            trigger_generator_serial = serial_port;
        } else {
            PhotoModule::Ptr pmodule = tdev.GetPhotoModuleByID(devid);
            if (pmodule != nullptr)
                tdev.SetSerialPort(pmodule, serial_port);
        }
        //} catch
    }

    //
    //
    // ###############################################################
    //
    //


    InputMessage response;
    std::array<uint8_t, 256> recv_buffer;
    ImageVisualizer visualizer;
    ImageReconstructor img(TableDevice::TableWidth, TableDevice::TableHeight);
    img.SetTestPattern();
    visualizer.ShowReconstruction(img);


    //
    //
    // ###############################################################
    //
    //

    {
        std::list<InputMessage> responses;
        bool timeout_occured;

        for (int attempt = 0; attempt < 5; attempt++) {

            TriggeredMeasurementEnterPayload config{.data_size = 8};
            OutputMessage msg_config = OutputMessage(MessageType::TriggeredMeasurementEnterRequest, &config,
                                                     sizeof(TriggeredMeasurementEnterPayload));

            printf("Entering triggered measurement mode: ");
            responses = Communication::SendToMultipleAndWaitForResponse(tdev.GetSerialPortCollection(), msg_config,
                                                                        1000,
                                                                        timeout_occured);

            printf(" Got responses from %zu/%zu devices (%s)\n", responses.size(),
                   tdev.GetSerialPortCollection().size(),
                   timeout_occured ? "TIMEOUT" : "ok");
            if (!timeout_occured)
                break;
        }


        if (!timeout_occured) {
          //  tdev.
        } else {
            printf("ERROR: One or more photomodules is not responding; pleas check the hardware.");
            getchar();
        }
    }


    //
    //
    // ###############################################################
    //
    //

    {
        printf("Press any key...\n");
        while (getchar() != '\n');
    }


    //
    //
    // ###############################################################
    //
    //
    {
        TriggerGeneratorPayload tgp;
        tgp.trigger1.high_interval = 5;
        tgp.trigger1.low_interval = 100;
        tgp.trigger1.echo_delay = 70;
        tgp.trigger1.mode = TriggerGeneratorSetMode::SetAndRun;
        //tgp.trigger1.is_single_shot = true;

        tgp.trigger2.mode = TriggerGeneratorSetMode::TurnOff;

        OutputMessage msg_setup_trigger = OutputMessage(MessageType::SetTriggerGeneratorRequest, &tgp,
                                                        sizeof(TriggerGeneratorPayload));
        InputMessage response;
        try{
            printf("Configuring trigger device... ");
            Communication::Transcive(trigger_generator_serial, msg_setup_trigger, response, 2000);
            printf("Ok\n");
        } catch (const TimeoutError &te) {
            printf("Timeout\n");
        }

        // Wait for all transmissions to end
        usleep(500 * 1000);

        // Flush all buffers on all serial ports
        trigger_generator_serial->DiscardAllData();
        for(auto pserial : tdev.GetSerialPortCollection())
            pserial->DiscardAllData();
    }
    //
    //
    // ###############################################################
    //
    //

    {
        //OutputMessage msg_do_single_measurement = OutputMessage(MessageType::SingleMeasurement8Request);

        std::map<int, InputMessageBuilder> fd2builder;
        std::map<int, Location> fd2location;
        std::vector<SerialPort::Ptr> serial_ports;
        for (PhotoModule::Ptr pmodule : tdev.GetPhotoModulesCollection()) {
            SerialPort::Ptr pserial = pmodule->GetSerialPort();
            if (pserial == nullptr)
                continue;
            fd2builder[pserial->GetHandle()] = InputMessageBuilder();
            fd2location[pserial->GetHandle()] = pmodule->GetLocation();
            serial_ports.push_back(pmodule->GetSerialPort());
        }

        //int stdout_save;
        //stdout_save = dup(STDOUT_FILENO); /* save */

        nc::initscr();
        nc::nodelay(nc::stdscr, TRUE);
        nc::noecho();
        nc::resize_term(25, 100);
        nc::clear();
        nc::refresh();

        std::chrono::time_point<std::chrono::steady_clock> last_update = std::chrono::steady_clock::now();
        while (true) {

            int ch;
            if ((ch = nc::wgetch(nc::stdscr)) != ERR) {
                ch = toupper(ch);
                if (ch == 'Q')
                    break;
                if (ch == '+')
                    visualizer.DoZoomIn();
                if (ch == '-')
                    visualizer.DoZoomOut();
                if (ch == 'R')
                    visualizer.DoResetNormalization();
            }

            fd_set rfd;
            FD_ZERO(&rfd);

            int max_handle = INT32_MIN;
            for (SerialPort::Ptr pserial : serial_ports) {
                FD_SET(pserial->GetHandle(), &rfd);
                max_handle = std::max(max_handle, pserial->GetHandle());
            }

            // end of trigger source
            FD_SET(trigger_generator_serial->GetHandle(), &rfd);
            max_handle = std::max(max_handle, trigger_generator_serial->GetHandle());


            //loops++;
            timeval tv = {.tv_sec = 0, .tv_usec = 500 * 1000};
            int sret = ::select(max_handle + 1, &rfd, nullptr, nullptr, &tv);

            if (sret == -1)
                throw std::runtime_error("select");

            if (sret == 0) {
                usleep(10 * 1000);
                continue;
            }

            bool got_some_data = false;
            for (SerialPort::Ptr pserial : tdev.GetSerialPortCollection()) {
                int fd = pserial->GetHandle();
                if (FD_ISSET(fd, &rfd)) {
                    ssize_t recv_bytes = pserial->Receive(recv_buffer);
                    auto &builder = fd2builder[fd];
                    builder.AddCollectedData(recv_buffer, 0, recv_bytes);


                    if (builder.ExtractMessage(response) == MessageExtractionResult::Ok) {
                        const Location &location = fd2location[fd];

                        const uint8_t *ptr = response.GetPayloadPointer<std::uint8_t>();
                        //int payload_length = response.GetPayloadSize();
                        img.ProcessMeasurementPayload(ptr, 8, location, 15);
                        got_some_data = true;

                        PhotoModule::Ptr pmodule = tdev.GetPhotoModuleByID(response.GetDeviceID());
                        assert(pmodule != nullptr);

                        struct status_descriptor_t& descriptor = pmodule->GetStatusDescriptor();
                        descriptor.last_sequence_number = response.GetSequenceNumber();
                        descriptor.received_images++;
                    }
                }
            }

            if (got_some_data)
                continue;

            //std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
            //if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_update).count() > timeout)


            // Check trigger information
            if (FD_ISSET(trigger_generator_serial->GetHandle(), &rfd)) {
                ssize_t recv_bytes = trigger_generator_serial->Receive(recv_buffer);
                if (recv_buffer[recv_bytes - 1] == END_OF_TRIGGER_MARKER)
                    visualizer.ShowReconstruction(img);
            }


            //printf("\x1b[%dA", TableDevice::TableHeight / PhotoModule::ModuleHeight);
            for (int row = 0; row < TableDevice::TableHeight / PhotoModule::ModuleHeight; row++) {
                nc::move(row, 0);
                for (int col = 0; col < TableDevice::TableWidth / PhotoModule::ModuleWidth; col++) {
                    PhotoModule::Ptr pmodule = tdev.GetPhotoModuleByLocation(1 + col, 1 + row);
                    assert(pmodule != nullptr);

                    struct status_descriptor_t &td = pmodule->GetStatusDescriptor();
                    nc::printw("[%5d;%5d]  ", td.last_sequence_number, td.received_images);
                }

            }

            nc::refresh();
        }

        //
        nc::endwin();
    }


    //
    //
    // ###############################################################
    //
    //
    {
        TriggerGeneratorPayload tgp;
        tgp.trigger1.mode = TriggerGeneratorSetMode::TurnOff;
        tgp.trigger2.mode = TriggerGeneratorSetMode::TurnOff;

        OutputMessage msg_setup_trigger = OutputMessage(MessageType::SetTriggerGeneratorRequest, &tgp,
                                                        sizeof(TriggerGeneratorPayload));
        InputMessage response;
        try{
            printf("Shutting down trigger generator... ");
            Communication::Transcive(trigger_generator_serial, msg_setup_trigger, response, 2000);
            printf("Ok\n");
        } catch (const TimeoutError &te) {
            printf("Timeout\n");
        }

        // Wait for all transmissions to end
        usleep(1000 * 1000);

        // Flush all buffers on all serial ports
        trigger_generator_serial->DiscardAllData();
        for(auto pserial : tdev.GetSerialPortCollection())
            pserial->DiscardAllData();
    }


    //
    //
    // ###############################################################
    //
    //

    {
        std::list<InputMessage> responses;
        bool timeout_occured;

        for (int attempt = 0; attempt < 5; attempt++) {

            OutputMessage msg_config = OutputMessage(MessageType::TriggeredMeasurementLeaveRequest);

            printf("leaving triggered measurement mode: ");
            responses = Communication::SendToMultipleAndWaitForResponse(tdev.GetSerialPortCollection(), msg_config,
                                                                        1000,
                                                                        timeout_occured);

            printf(" Got responses from %zu/%zu devices (%s)\n", responses.size(),
                   tdev.GetSerialPortCollection().size(),
                   timeout_occured ? "TIMEOUT" : "ok");
            if (!timeout_occured)
                break;
        }


        if (!timeout_occured) {
            //  tdev.
        } else {
            printf("ERROR: One or more photomodules is not responding; pleas check the hardware.");
            getchar();
        }
    }


    //
    //
    // ###############################################################
    //
    //

    {
        printf("Closing %zu serial ports... ", ports.size());
        for(SerialPort::Ptr pserial : ports)
            pserial->Close();

        printf(AGREEN "Ok.\n");
    }

    return 0;
}


int main(int argc, const char** argv, const char** env) {
//
//
//    while(true) {
//        int c = nc::wgetch(nc::stdscr);
//        if (c == ERR)
//            continue;
//
//        nc::printw("%d ", c);
//        nc::refresh();
//    }
//

    App app(argc, argv, env);
    app.Run();
    return app.GetErrorCode();
}
