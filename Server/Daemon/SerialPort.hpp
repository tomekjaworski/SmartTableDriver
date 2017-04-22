#if !defined(_COMM_HPP_)
#define _COMM_HPP_


#include <string>

class SerialPort
{
	std::string port_name;
public:
	int fd;
	
	uint8_t input_buffer[12800];
	int input_buffer_position;
	int error_counter;
	int frame_counter;
	int overflow_counter;
	int sync_counter;
	int bytes_received;
	

public:
	const std::string& getPortName(void) const { return this->port_name; }

	
public:
	SerialPort(void);
	
	void init(const std::string& device_name, bool fake_serial_port);
	void done(void);
	
};


#endif // _COMM_HPP_

