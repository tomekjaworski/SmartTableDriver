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
 #include "crc16.h"

 EEPROM_CONFIGURATION configuration;

 uint8_t configuration_load(void)
 {
	// wczytaj blok z pamiêci EEPROM
	eeprom_read_block(&configuration, 0, sizeof(EEPROM_CONFIGURATION));
	uint16_t current_checksum = configuration.checksum;

	// oblicz now¹ sumê kontroln¹ konfiguracji
	configuration.checksum = 0x0000;
	configuration.checksum = calc_crc16(&configuration, sizeof(EEPROM_CONFIGURATION));

	if (configuration.checksum == current_checksum)
		return 1; // jest OK!

	configuration_load_default_values();
	configuration_store(); // zapisz od razu nowe, domyœlne

	return 0;
 }

 void configuration_store(void)
 {
	// dodaj sumê kontroln¹ do bloku pamiêci
	configuration.checksum = 0x0000;
	configuration.checksum = calc_crc16(&configuration, sizeof(EEPROM_CONFIGURATION));

	// zapisz blok pamiêci konfiguracji do EEPROMu
	eeprom_write_block (&configuration, 0, sizeof(EEPROM_CONFIGURATION));

	// no i spoko luzik.

 }

 void configuration_load_default_values(void)
 {
	// ustaw domyœlne wartoœci w bloku konfiguracyjnym EEPROM

	memset(&configuration, 0, sizeof(EEPROM_CONFIGURATION));

	// sygnaturka
	const uint8_t magic[] = {116,117,32,98,121,108,101,109,32,84,74};
	memcpy(configuration.magic, magic, sizeof(magic));

 }