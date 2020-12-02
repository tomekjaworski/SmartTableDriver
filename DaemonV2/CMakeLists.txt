cmake_minimum_required(VERSION 3.13)
project(untitled)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_COLOR_MAKEFILE OFF)
#set(CMAKE_VERBOSE_MAKEFILE ON)
#add_library(stdc++fs STATIC IMPORTED)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    message(">>> DEBUG mode")
    add_compile_options( -g)
    add_link_options()
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    message(">>> RELEASE mode")
    add_compile_options(-O3 -Ofast)
    add_link_options(-flto)
endif ()

add_compile_options("-Wall" "-Wextra" "-pedantic" "-Wno-unused-parameter" "-fno-diagnostics-color")


add_executable(
        ${PROJECT_NAME}
        Utility/AnsiCodes.h

        ImageReconstructor.hpp

        main.cpp App.hpp
        Hardware/Location.hpp Hardware/PhotoModule.hpp

        ProgramBase.cpp ProgramBase.hpp

        SerialPort/SerialPort.cpp SerialPort/SerialPort.hpp
        SerialPort/SerialPortReactor.cpp SerialPort/SerialPortReactor.hpp
        Hardware/TableDevice.cpp Hardware/TableDevice.hpp

        Utility/Crc16.hpp
        Protocol/OutputMessage.cpp Protocol/OutputMessage.hpp

        Protocol/InputMessage.cpp Protocol/InputMessage.hpp TableFirmware/protocol.h
        Protocol/InputMessageBuilder.cpp Protocol/InputMessageBuilder.hpp
        Utility/Helper.cpp Utility/Helper.hpp Protocol/TimeoutError.h

        Protocol/Communication.cpp Protocol/Communication.hpp

        ImageReconstructor.cpp Visualizer/ImageVisualizer.cpp Visualizer/ImageVisualizer.hpp)

find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED)

target_link_libraries(${PROJECT_NAME} stdc++fs ${OpenCV_LIBS} ${boost_LIBS} curses)