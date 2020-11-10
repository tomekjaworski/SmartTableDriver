/*
 * hardware.h
 *
 * Created: 25/03/2017 13:50:13
 *  Author: Tomasz Jaworski
 */ 


#ifndef HARDWARE_H_
#define HARDWARE_H_

#define LED0_TOGGLE do { PORTB ^= _BV(PORTB5); } while (0);

#define TRIGGER1_TOGGLE()	do { PORTB ^= _BV(PORTB0); } while (0);
#define TRIGGER2_TOGGLE()	do { PORTB ^= _BV(PORTB1); } while (0);

#define TRIGGER1_SET_HIGH()		do { PORTB |= _BV(PORTB0); } while(0) //1
#define TRIGGER2_SET_HIGH()		do { PORTB |= _BV(PORTB1); } while(0) //1
#define TRIGGER1_SET_LOW()		do { PORTB &= ~_BV(PORTB0); } while(0)//0
#define TRIGGER2_SET_LOW()		do { PORTB &= ~_BV(PORTB1); } while(0)//0


//
//#define GET_TRIGGER()	!(PIND & _BV(PIND3))

#define LEGACY_RS485_DIR_OUTPUT()	do { PORTD |= _BV(PORTD2); } while(0); //1
#define LEGACY_RS485_DIR_INPUT()	do { PORTD &= ~_BV(PORTD2); } while(0);//0

//#define RESET1_HIGH		do { PORTD |= _BV(PORTD5); } while(0); //1
//#define RESET2_HIGH		do { PORTD |= _BV(PORTD4); } while(0); //1
//#define RESET1_LOW		do { PORTD &= ~_BV(PORTD5); } while(0);//0
//#define RESET2_LOW		do { PORTD &= ~_BV(PORTD4); } while(0);//0

#define SET_RECEIVER_INTERRUPT(__state) do {	if (__state)	\
		UCSR0B |= (1 << RXCIE0);								\
		else													\
		UCSR0B &= ~(1 << RXCIE0);								\
	} while (0);



enum class PinState : uint8_t {
	Low = 0x00,
	High = 0x01,
};

/*
 *         __________                 __________
 * _______|          |_______________|          |_______________
 *        |<-- hi -->|<---- low ---->|
 * 	  
 * _______________________[TE]_______________________[TE]
 *        |<---- tet ---->|
 * 
 *    hi  - TriggerGeneratorConfig::high_interval - time in milliseconds during which triggering output is set to HIGH
 *    low - TriggerGeneratorConfig::low_interval - time [ms] during which the triggering output is set to LOW
 *    tet - TriggerGeneratorConfig::echo_delay - delay [ms] between LOW-to-HIGH transition of the output and the beginning of trigger echo transmission. 
 */

struct TriggerGeneratorConfig {
	struct {
		volatile bool is_active;
		volatile bool is_single_shot;
		volatile PinState state;
		volatile int16_t state_counter;
		
		volatile int16_t low_interval;
		volatile int16_t high_interval;

		struct {		
			volatile bool is_active;
			volatile int16_t delay;
			volatile int16_t counter;
		} echo;
		
	} trigger1, trigger2;
};


extern struct TriggerGeneratorConfig trigger_config;

#endif /* HARDWARE_H_ */