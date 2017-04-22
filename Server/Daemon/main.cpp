#include <stdio.h>
#include <string.h>
#include <stdexcept>
#include "CRC.hpp"

#include "SerialPort.hpp"
#include "Environment.hpp"

#include "../../SmartTableFirmware/MessageType.h"
#include "../../SmartTableFirmware/protocol.h"

#include "Message.h"

#include <termios.h>
#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <thread>
#include <chrono>
#include <sstream>
#include <iostream>
#include <sys/ioctl.h>

int main(int argc, char **argv)
{
	printf("dummy");
	
	
	
	
	SerialPort sp;
	
	sp.init("", false);
	
	
	Message mping(0x50, MessageType::Ping);
	sp.send(mping.getBinary(), mping.getBinaryLength());
	
	
	return 0;
}
