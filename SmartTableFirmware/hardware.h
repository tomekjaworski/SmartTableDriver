/*
 * hardware.h
 *
 * Created: 25/03/2017 13:50:13
 *  Author: Iza
 */ 


#ifndef HARDWARE_H_
#define HARDWARE_H_


#define LED_ON do { PORTB |= _BV(PORTB5); } while (0);
#define LED_OFF do { PORTB &= ~_BV(PORTB5); } while (0);
#define LED_TOGGLE do { PORTB ^= _BV(PORTB5); } while (0);

#define LED0_OFF do { PORTB |= _BV(PORTB2); } while (0);
#define LED0_ON do { PORTB &= ~_BV(PORTB2); } while (0);
#define LED0_TOGGLE do { PORTB ^= _BV(PORTB2); } while (0);

#define LED1_OFF do { PORTB |= _BV(PORTB1); } while (0);
#define LED1_ON do { PORTB &= ~_BV(PORTB1); } while (0);
#define LED1_TOGGLE do { PORTB ^= _BV(PORTB1); } while (0);

#define RS485_DIR_SEND		do { PORTD |= _BV(PORTD2); } while(0); //1
#define RS485_DIR_RECEIVE	do { PORTD &= ~_BV(PORTD2); } while(0);//0

#define STATIC_ASSERT(COND,MSG) typedef char static_assertion_##MSG[(COND)?1:-1]


#endif /* HARDWARE_H_ */