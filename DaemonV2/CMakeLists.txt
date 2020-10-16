cmake_minimum_required(VERSION 3.17)
project(untitled)

set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_VERBOSE_MAKEFILE ON)
#add_library(stdc++fs STATIC IMPORTED)

add_compile_options("-Wall" "-Wextra" "-pedantic" "-Wno-unused-parameter" "-fno-diagnostics-color")


add_executable(
        untitled
        AnsiCodes.h

        main.cpp
        Location.hpp PhotoModule.hpp

        ProgramBase.cpp ProgramBase.hpp

        SerialPort.cpp SerialPort.hpp
        SerialPortReactor.cpp SerialPortReactor.hpp
        TableDevice.cpp TableDevice.hpp

        Crc16.hpp
        OutputMessage.cpp OutputMessage.hpp

        InputMessage.cpp InputMessage.hpp
        InputMessageBuilder.cpp InputMessageBuilder.hpp
        Helper.cpp Helper.hpp TimeoutError.h

        Communication.cpp Communication.hpp

)


target_link_libraries(${PROJECT_NAME} stdc++fs)