#include <stdio.h>
#include "CRC.hpp"

#include "SerialPort.hpp"
#include "Environment.hpp"


int main(int argc, char **argv)
{
	printf("dummy");
	
	SerialPort sp;
	
	sp.init("", false);
	
	
	return 0;
}
