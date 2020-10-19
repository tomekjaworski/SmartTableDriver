/*
 * config.cpp
 *
 * Created: 11.05.2017 10:07:37
 *  Author: Tomek Jaworski
 */ 

#include <avr/pgmspace.h>

#include "../TableFirmware/protocol.h"
#include "config.h"

device_identifier_t device_identifier = (device_identifier_t)0xF0;

#include <avr/eeprom.h>
uint8_t EEMEM __addr[3] = {0xF0, 0xF0, 0xF0};