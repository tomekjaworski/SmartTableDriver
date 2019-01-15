#include <cstdio>
#include <cassert>
#include <climits>
#include <list>
#include <chrono>
#include <thread>
#include <string>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <unordered_map>

#include "SerialPort.hpp"
#include "Environment.hpp"
#include "MessageReceiver.hpp"


#include "Message.h"
#include "timeout_error.hpp"

static void dump(const void* ptr, int count)
{
	printf("[");
	const uint8_t* p = static_cast<const uint8_t*>(ptr);
	for (int i = 0; i < count; i++)
		if (i < count - 1)
			printf("%02x ", p[i]);
		else
			printf("%02x]\n", p[i]);
			
	fflush(stdout);
}

bool SendAndWaitForResponse(SerialPort::Ptr serial, const Message& query, Message& response, int timeout)
{

	MessageReceiver mr;

	serial->discardAllData();
	serial->send(query.getDataPointer(), query.getDataCount());
	//dump(query.getBinary(), query.getBinaryLength());
	
	std::chrono::time_point<std::chrono::steady_clock> start_time = std::chrono::steady_clock::now();
	
	// now wait for response
	do {
		fd_set rfd;
		FD_ZERO(&rfd);
		FD_SET(serial->getHandle(), &rfd);
		
		timeval tv = { .tv_sec = 0, .tv_usec = 50 * 1000 };
		int sret = ::select(serial->getHandle() + 1, &rfd, nullptr, nullptr, &tv);
		
		if (sret == 0)
			goto _check_timeout; // timeout - let's loop
			
		if (sret == -1) {
			perror(__func__);
			Environment::terminateOnError("select", 0);
		}
	
		mr.receive(*serial);
		//ssize_t bytes_read = ::read(serial.getHandle(), buffer + position, sizeof(buffer) - position);
		//assert(bytes_read > 0);
		//position += bytes_read;
		
		if (mr.getMessage(response))
			return true;
		
		
		// check for timeout
_check_timeout:;
		std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count() > timeout)
		{
			char buffer[256];
			sprintf(buffer, "Timeout (SendAndWaitForResponse): addr=%02X; command=%d", query.getAddress(), static_cast<int>(query.getType()));
			throw timeout_error(std::string(buffer));
		}
		
	} while (true);
	
	return false;
}



bool SendAndWaitForResponse(std::list<SerialPort::Ptr>& serials, const Message& query, Message& response, SerialPort::Ptr& response_port, int timeout)
{

	MessageReceiver mr;

	for (SerialPort::Ptr& p : serials)
	{
		p->discardAllData();
		int sent = p->send(query.getDataPointer(), query.getDataCount());
		
		//printf("SendAndWaitForResponse SEND: requested=%d; sent=%d\n", query.getBinaryLength(), sent);
	}
	
	std::chrono::time_point<std::chrono::steady_clock> start_time = std::chrono::steady_clock::now();

	SerialPort::Ptr last_serial = nullptr;
	response_port = nullptr;
	
	// now wait for response
	do {
		fd_set rfd;
		FD_ZERO(&rfd);

		int fd_max = 0;
		for (SerialPort::Ptr& p : serials)
		{
			FD_SET(p->getHandle(), &rfd);
			fd_max = std::max(fd_max, p->getHandle());
		}
		
		
		timeval tv = { .tv_sec = 0, .tv_usec = 75 * 1000 };
		int sret = ::select(fd_max + 1, &rfd, nullptr, nullptr, &tv);
		
		if (sret == 0)
			goto _check_timeout; // timeout - let's loop
			
		if (sret == -1) {
			perror(__func__);
			Environment::terminateOnError("select", 0);
		}
	
		// assumption: ONLY ONE device on ONLY ONE serial port will respond to sent message
		for (SerialPort::Ptr& p : serials)
		{
			if (!FD_ISSET(p->getHandle(), &rfd))
				continue;

			if (last_serial == nullptr)
				last_serial = p;
				
			assert(p == last_serial);
			mr.receive(*p);

		}
		//ssize_t bytes_read = ::read(serial.getHandle(), buffer + position, sizeof(buffer) - position);
		//assert(bytes_read > 0);
		//position += bytes_read;
		
		if (mr.getMessage(response))
		{
			response_port = last_serial;
			return true;
		}
		
		
		// check for timeout
_check_timeout:;
		std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count() > timeout)
			throw timeout_error("SendAndWaitForResponse");
		
	} while (true);
	
	return false;
}
