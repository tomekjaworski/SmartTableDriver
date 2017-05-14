/*
 * config.cpp
 *
 * Created: 11.05.2017 10:07:37
 *  Author: Tomek Jaworski
 */ 

#include <avr/pgmspace.h>

#include "protocol.h"
#include "config.h"

const  device_address_t device_address_block[] PROGMEM = {0xaa, 0x11, 0x0d, 0x4d, __DEVICE_ADDRESS, 0x75, 0x87, 0x60, 0x64};
device_address_t device_address;

