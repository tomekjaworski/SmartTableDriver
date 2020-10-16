cmake_minimum_required(VERSION 3.17)
project(untitled)

set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_VERBOSE_MAKEFILE ON)
#add_library(stdc++fs STATIC IMPORTED)

add_compile_options("-Wall" "-Wextra" "-pedantic" "-Wno-unused-parameter" "-fno-diagnostics-color")


add_executable(
        untitled
        Utility/AnsiCodes.h

        main.cpp
        Hardware/Location.hpp Hardware/PhotoModule.hpp

        ProgramBase.cpp ProgramBase.hpp

        SerialPort/SerialPort.cpp SerialPort/SerialPort.hpp
        SerialPort/SerialPortReactor.cpp SerialPort/SerialPortReactor.hpp
        Hardware/TableDevice.cpp Hardware/TableDevice.hpp

        Utility/Crc16.hpp
        Protocol/OutputMessage.cpp Protocol/OutputMessage.hpp

        Protocol/InputMessage.cpp Protocol/InputMessage.hpp
        Protocol/InputMessageBuilder.cpp Protocol/InputMessageBuilder.hpp
        Utility/Helper.cpp Utility/Helper.hpp Protocol/TimeoutError.h

        Protocol/Communication.cpp Protocol/Communication.hpp

)


target_link_libraries(${PROJECT_NAME} stdc++fs)