/*
 * timers.cpp
 *
 * Created: 10/5/2020 10:43:06 PM
 *  Author: evilruff
 */ 

#include <math.h>

#include "timers.h"
#include "logger.h"

timerDefinition	Timer0 = {
	.setup = setupTimer0,
	.start = startTimer0,
	.stop  = stopTimer0
};


//----------------------------------------------------------------------------------------------------------
uint16_t	calculatePrescaler(uint32_t		desiredFrequency, uint8_t & ocrMask, uint8_t & counter, uint8_t type) {
	float	prescalers[] = { (float)F_CPU/1.0, (float)F_CPU/8.0, (float)F_CPU/64.0, (float)F_CPU/256.0, (float)F_CPU/1024.0 };
	uint32_t	compareMatch = 0;
	uint8_t		n = 0;
	for ( n = 0; n < 5; n++) {
		compareMatch = round(prescalers[n]/(float)desiredFrequency)-1;
		if (compareMatch < (type ? 65535 : 255))
		break;
	}
	
	if (compareMatch > (type ? 65535 : 255) ) 
		return False;
		
	switch (n) {
		case 0:
			ocrMask = (0 << CS02) | (0 << CS01) | (1 << CS00);
		break;
		case 1:
			ocrMask = (0 << CS02) | (1 << CS01) | (0 << CS00);
		break;
		case 2:
			ocrMask = (0 << CS02) | (1 << CS01) | (1 << CS00);
		break;
		case 3:
			ocrMask = (1 << CS02) | (0 << CS01) | (0 << CS00);
		break;
		case 4:
			ocrMask = (1 << CS02) | (0 << CS01) | (1 << CS00);
		break;
	}
	
	counter = (uint16_t)compareMatch;
	
	return True;	
}
//----------------------------------------------------------------------------------------------------------
void setupTimer0( uint32_t  desiredFrequency ) {
	uint8_t	mask = 0;
	uint8_t cnt	 = 0;
	
	if (calculatePrescaler(desiredFrequency, mask, cnt, TIMER_8BIT)) {
		setupTimer0(mask, cnt);	
	}	
}
//----------------------------------------------------------------------------------------------------------
void setupTimer0( uint8_t prescaler, uint8_t cmp) {
		
	TIMSK0 &= ~(1 << OCIE0A);
	TCCR0A = 0; 
	TCCR0B = 0; 
	TCNT0  = 0; 
	
	OCR0A = (uint8_t)cmp;
	
	TCCR0A |= (1 << WGM01);
	TCCR0B |= prescaler;
}
//----------------------------------------------------------------------------------------------------------
void startTimer0() {
	TCNT0  = 0; 
	TIMSK0 |= (1 << OCIE0A);
}
//----------------------------------------------------------------------------------------------------------
void stopTimer0() {
	TIMSK0 &= ~(1 << OCIE0A);
}
//----------------------------------------------------------------------------------------------------------
