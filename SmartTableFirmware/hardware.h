/*
 * hardware.h
 *
 * Created: 25/03/2017 13:50:13
 *  Author: Tomasz Jaworski
 */ 


#ifndef HARDWARE_H_
#define HARDWARE_H_


#define LED0_OFF do { PORTB |= _BV(PORTB2); } while (0);
#define LED0_ON do { PORTB &= ~_BV(PORTB2); } while (0);
#define LED0_TOGGLE do { PORTB ^= _BV(PORTB2); } while (0);

#define LED1_ON do { PORTB |= _BV(PORTB5); } while (0);
#define LED1_OFF do { PORTB &= ~_BV(PORTB5); } while (0);
#define LED1_TOGGLE do { PORTB ^= _BV(PORTB5); } while (0);

#define GET_TRIGGER()	!(PIND & _BV(PIND3))

#define LEGACY_RS485_DIR_OUTPUT()	do { PORTD |= _BV(PORTD2); } while(0); //1
#define LEGACY_RS485_DIR_INPUT()	do { PORTD &= ~_BV(PORTD2); } while(0);//0

#define RESET1_HIGH		do { PORTD |= _BV(PORTD5); } while(0); //1
#define RESET2_HIGH		do { PORTD |= _BV(PORTD4); } while(0); //1
#define RESET1_LOW		do { PORTD &= ~_BV(PORTD5); } while(0);//0
#define RESET2_LOW		do { PORTD &= ~_BV(PORTD4); } while(0);//0

#define SET_RECEIVER_INTERRUPT(__state) do {	if (__state)	\
		UCSR0B |= (1 << RXCIE0);								\
		else													\
		UCSR0B &= ~(1 << RXCIE0);								\
	} while (0);


#endif /* HARDWARE_H_ */