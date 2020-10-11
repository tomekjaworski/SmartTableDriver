#!python
# tested with Python 3.8
import intelhex as ih
import random as rnd


def generate_random_data(fileName: str, size: int):
	file = ih.IntelHex()
	for addr in range(0, size):
		file[addr] = rnd.randint(0, 255)

	file.write_hex_file(fileName)
	
## main

# for 1k eeprom testing
generate_random_data("eeprom_1k_01.hex", 1024)
generate_random_data("eeprom_1k_02.hex", 1024)
generate_random_data("eeprom_1k_03.hex", 1024)

# for 30k flash testing
# the used CPU has a 32k space, however last 2k are occupied by the bootloadeer, hence 30k
generate_random_data("flash_30k_01.hex", 30 * 1024)
generate_random_data("flash_30k_02.hex", 30 * 1024)
generate_random_data("flash_30k_03.hex", 30 * 1024)

print("Done.")
