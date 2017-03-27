/*
 * eeprom_config.h
 *
 * Created: 25/03/2017 13:53:03
 *  Author: Iza
 */ 
 
 #ifndef EEPROM_CONFIG_H_
 #define EEPROM_CONFIG_H_


 struct __EEPROM_CONFIGURATION
 {
	 uint16_t checksum;
	 char magic[12];

	 uint8_t address; // adres modu³u pomiarowego

 };
 
 typedef struct __EEPROM_CONFIGURATION EEPROM_CONFIGURATION;
 extern EEPROM_CONFIGURATION configuration;

 void configuration_load_default_values(void);
 void configuration_store(void);
 uint8_t configuration_load(void);


 #endif /* EEPROM_CONFIG_H_ */
