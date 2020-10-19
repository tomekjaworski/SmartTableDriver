/*
 * eeprom_config.c
 *
 * Created: 25/03/2017 13:52:23
 *  Author: Tomasz Jaworski
 */ 
 
 #include <avr/io.h>
 #include <avr/interrupt.h>
 #include <avr/eeprom.h>
 #include <string.h>

 #include "eeprom_config.h"
 #include "config.h"

 EEPROM_CONFIGURATION configuration;

 void configuration_load(void) {

	// wczytaj blok z pami?ci EEPROM
	eeprom_read_block(&configuration, 0, sizeof(EEPROM_CONFIGURATION));

	// 1
	if (configuration.address == configuration.address_copy1)
		device_identifier = configuration.address;

	// 2
	if (configuration.address == configuration.address_copy2)
		device_identifier = configuration.address;

	// 3
	if (configuration.address_copy1 == configuration.address_copy2)
		device_identifier = configuration.address_copy1;
 }

 void configuration_store(void)
 {
	// zapisz blok pami?ci konfiguracji do EEPROMu
	eeprom_write_block (&configuration, 0, sizeof(EEPROM_CONFIGURATION));
 }
