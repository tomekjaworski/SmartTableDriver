#if !defined(_IMAGE_HPP_)
#define _IMAGE_HPP_

class Image
{
private:
	uint16_t *data;
	int width, height;
	
	
public:
	Image(int width, int height)
		: width(width), height(height)
	{
		this->data = (uint16_t*)malloc(height * width * sizeof(uint16_t));
		clear();
	}
	
	void clear(void)
	{
		for (int i = 0; i < width * height; i++)
			data[i] = 0x0000;
	}
	
	void processMeasurementPayload(const void* vpayload, int bits, const Location& ploc)
	{
		assert(bits == 16); // other cases not implemented at the moment
		
		uint16_t* upayload = (uint16_t*)vpayload;
		for (int x = 0; x < 10; x++)
			for (int y = 0; y < 10; y++)
			{
				data[ploc.getColumn() + x + width * (ploc.getRow() + y)] = upayload[x + y * 10];
			}
		
	}
	
	const uint16_t* getData(void) const { return this->data; }
	
};

#endif // _IMAGE_HPP_
