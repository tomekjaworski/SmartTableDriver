/*
 * eeprom_config.h
 *
 * Created: 25/03/2017 13:53:03
 *  Author: Tomasz Jaworski
 */ 
 
 #ifndef EEPROM_CONFIG_H_
 #define EEPROM_CONFIG_H_


 extern struct EEPROM_CONFIGURATION {
	 uint8_t address; // adres modu?u pomiarowego
	 uint8_t address_copy1; // j/w do weryfikacji
	 uint8_t address_copy2; // j/w do weryfikacji

 } configuration;
 

 //void configuration_load_default_values(void);
 //void configuration_store(void);
 void configuration_load(void);


 #endif /* EEPROM_CONFIG_H_ */
