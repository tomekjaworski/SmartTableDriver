#if !defined (_CRC_HPP_)
#define _CRC_HPP_

class CRC16
{
public:
	static uint16_t Calc(const void* data, size_t count)
	{
		uint16_t chk = 0xFFFF;
		const uint8_t* ptr = (const uint8_t*)data;
		
		for (size_t i = 0; i < count; i++)
		{
			uint8_t byte = *ptr++;
			chk ^= (uint16_t)byte;

			for (int bit = 8; bit > 0; bit--)
			{
				if ((chk & 0x0001) > 0)
					chk = (chk >> 1) ^ 0xA001;
				else
					chk >>= 1;
			}
		}

		return chk;
	}
};


#endif // _CRC_HPP_

