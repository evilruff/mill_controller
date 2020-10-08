/*
 * timers.cpp
 *
 * Created: 10/5/2020 10:43:06 PM
 *  Author: evilruff
 */ 

#include "timers.h"

//----------------------------------------------------------------------------------------------------------
uint8_t setupTimer0(uint16_t desiredFrequency, uint8_t autostart, uint8_t disableInterrupts) {

	uint16_t	prescalers[] = { 1, 8, 64, 256, 1024 };
	uint32_t	compareMatch = 0;
	uint8_t		n = 0;
	for ( n = 0; n < 4; n++) {
		compareMatch = (F_CPU/prescalers[n]/desiredFrequency)-1;
		if (compareMatch < 255)
		break;
	}
	
	if (disableInterrupts) {
		cli(); 
	}
		
	TIMSK0 &= ~(1 << OCIE0A);
	TCCR0A = 0; 
	TCCR0B = 0; 
	TCNT0  = 0; 
	
	OCR0A = (uint8_t)compareMatch;
	
	TCCR0A |= (1 << WGM01);
	
	switch (n) {
		case 0:
		TCCR0B |= (0 << CS02) | (0 << CS01) | (1 << CS00);
		break;
		case 1:
		TCCR0B |= (0 << CS02) | (1 << CS01) | (0 << CS00);
		break;
		case 2:
		TCCR0B |= (0 << CS02) | (1 << CS01) | (1 << CS00);
		break;
		case 3:
		TCCR0B |= (1 << CS02) | (0 << CS01) | (0 << CS00);
		break;
		case 4:
		TCCR0B |= (1 << CS02) | (0 << CS01) | (1 << CS00);
		break;

	}
	
	if (autostart) {
		TIMSK0 |= (1 << OCIE0A);
	}
	
	if (disableInterrupts) {
		sei(); // allow interrupts
	}

}
//----------------------------------------------------------------------------------------------------------
void startTimer0() {
	cli();
	TCNT0  = 0; 
	TIMSK0 |= (1 << OCIE0A);
	sei();
}
//----------------------------------------------------------------------------------------------------------
void stopTimer0() {
	cli();	
	TIMSK0 &= ~(1 << OCIE0A);
	sei();
}
//----------------------------------------------------------------------------------------------------------
