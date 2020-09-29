cmake_minimum_required(VERSION 3.17)
project(untitled)

set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_VERBOSE_MAKEFILE ON)
#add_library(stdc++fs STATIC IMPORTED)

add_executable(
        untitled
        AnsiCodes.h

        main.cpp
        Location.hpp PhotoModule.hpp

        ProgramBase.cpp ProgramBase.hpp

        SerialPort.cpp SerialPort.hpp
        SerialPortReactor.cpp SerialPortReactor.hpp
        TableDevice.cpp TableDevice.hpp

        Crc16.hpp OutputMessage.hpp
        OutputMessage.cpp)

target_link_libraries(${PROJECT_NAME} stdc++fs)