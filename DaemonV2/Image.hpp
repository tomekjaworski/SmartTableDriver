#if !defined(_IMAGE_HPP_)
#define _IMAGE_HPP_

#include <cstdint>
#include <vector>

class Image
{
private:
	std::vector<std::uint16_t> data;
	int width, height;

public:
	Image(int width, int height)
		: width(width), height(height) {
		this->data.reserve(width * height);
		this->Clear();
	}
	
	void Clear(void) {
		for (int i = 0; i < width * height; i++)
			data[i] = 0x0000;
	}
	
	void ProcessMeasurementPayload(const void* vpayload, int bits, const Location& ploc)
	{
		assert(bits == 16); // other cases not implemented at the moment
		
		uint16_t* upayload = (uint16_t*)vpayload;
		for (int x = 0; x < 10; x++)
			for (int y = 0; y < 10; y++)
			{
			    uint16_t value = upayload[x + y * 10];
				data[ploc.GetColumn() + x + width * (ploc.GetRow() + y)] = value;
			}
		
	}
	
	const uint16_t* getData(void) const { return this->data.data(); }
	
};

#endif // _IMAGE_HPP_
