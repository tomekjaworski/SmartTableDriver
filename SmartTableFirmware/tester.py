#
# tester.py
#
# Created: 22/09/2020 18:53:00
#  Author: Tomek Jaworski
# 

import serial, time
s = serial.Serial("COM74", 200000, parity=serial.PARITY_EVEN, timeout=0.1)

xt = [
    61,	68, 75,	45, 44,	43, 42,	46, 47,	48,
    60,	67, 74,	38, 37,	36, 35,	39, 40,	41,	
    59,	66, 73,	31, 30,	29, 28,	32, 33,	34,										
    58,	65, 72,	24, 23,	22, 21,	25, 26,	27,	
    62,	69, 76,	17, 16,	15, 14,	18, 19,	20,					
    63,	70, 77,	10, 9,	8,  7,	11, 12,	13,	
    64,	71, 78,	3,  2,	1,  0,	4,  5,	6,	
    55,	56, 50,	96, 95,	94, 93,	97, 98,	99,				
    51,	49, 57,	89, 88,	87, 86,	90, 91,	92,			
    52,	53, 54,	82, 81,	80, 79,	83, 84,	85]						

while(True):
    s.write(b'\xAB\x10\x00\x00\x00')
    x = s.read(3+100+2)
    if len(x) != 105:
        print("B?ad komunikacji...")
        continue
    x = x[3:-2]
    for r in range(0, 10):
        for c in range(0, 10):
            id = r * 10 + c
            id = xt[id]
            print(f"{x[id]:4d} ", end="")
        print()

    print("----------------")
    time.sleep(0.2)

while(False):
    s.write(b'\xAB\x10\x00\x00\x00')
    x = s.read(106)
    
    x = x[3:-2]
    print("Odczytano", len(x))
    for i in range(0, 100):
        if x[i] > 20:
            print(f"{i}:{x[i]}")
    time.sleep(0.2)

s.close()
