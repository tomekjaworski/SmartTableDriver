#if !defined(_COMM_HPP_)
#define _COMM_HPP_


#include <string>
#include <vector>

class SerialPort
{
	std::string port_name;
public:
	int fd;
	
	//uint8_t input_buffer[12800];
	//int input_buffer_position;
	//int error_counter;
	//int frame_counter;
	//int overflow_counter;
	//int sync_counter;
	//int bytes_received;
	

public:
	const std::string& getPortName(void) const { return this->port_name; }
	void discardAllData(void);

	
public:
	SerialPort(void);
	
	void init(const std::string& device_name, bool fake_serial_port);
	void done(void);
	
	int send(const void* data, size_t length);
	
	
public:
	static std::vector<std::string> getSerialDevices(void);
};




#endif // _COMM_HPP_

