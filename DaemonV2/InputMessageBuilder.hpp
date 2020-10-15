#ifndef MESSAGERECEIVER_HPP
#define MESSAGERECEIVER_HPP

#include <stdint.h>

#include "SerialPort.hpp"
#include "InputMessage.hpp"

#include <array>

class SerialPort;

class InputMessageBuilder
{
    std::array<uint8_t, 64 * 1024> queue;
	uint32_t position;
	
public:
	InputMessageBuilder(void);
	~InputMessageBuilder();
	

	template <typename T, size_t N> void AddCollectedData(std::array<T, N>& array, uint32_t offset, uint32_t count) {
	    if (offset > array.size())
            throw std::out_of_range("offset");
	    if (offset + count > array.size())
	        throw std::out_of_range("offset, count");

	    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(array.data()) + offset;
	    this->InternalAddCollectedData(ptr, count);
	}
	
	bool GetMessage(InputMessage& message);
	void PurgeAllData(void);

private:
    void InternalAddCollectedData(const void* ptr, uint32_t count);

};


#endif // MESSAGERECEIVER_HPP
