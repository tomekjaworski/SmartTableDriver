import intelhex as ih

f = ih.IntelHex()

for addr in range(0, 0x400):
	f[addr] = 0xff

# photo modules
for row in range(1, 5):
	for col in range(1, 7):
		d = 0x10 * row + col
		f[0] = d
		f[1] = d
		f[2] = d
	
		f.write_hex_file(f"{d:02x}.hex")

# trigger module
f[0] = 0x1f
f[1] = 0x1f
f[2] = 0x1f
d = 0x1f
f.write_hex_file(f"{d:02X}.hex")
