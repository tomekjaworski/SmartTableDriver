/*
 * config.cpp
 *
 * Created: 11.05.2017 10:07:37
 *  Author: Tomek Jaworski
 */ 

#include <avr/pgmspace.h>

#include "protocol.h"
#include "config.h"

device_identifier_t device_identifier = (device_identifier_t)0xFF;