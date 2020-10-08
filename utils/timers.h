/*
 * timers.h
 *
 * Created: 10/5/2020 10:42:01 PM
 *  Author: evilruff
 */ 


#ifndef TIMERS_H_
#define TIMERS_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include "utils.h"

#define TIMER_16BIT	1
#define TIMER_8BIT	0

typedef void (*setupTimerFuncFreq)( uint32_t  desiredFrequency );
typedef void (*setupTimerFunc)( uint8_t prescaler, uint8_t cmp );
typedef void (*startTimerFunc)();
typedef void (*stopTimerFunc)();

typedef struct {
	setupTimerFunc	setup;
	startTimerFunc	start;
	stopTimerFunc	stop;
} timerDefinition;

uint16_t	calculatePrescaler(uint32_t		desiredFrequency, uint8_t & ocrMask, uint8_t & counter, uint8_t type);
	
void setupTimer0( uint32_t  desiredFrequency );
void setupTimer0( uint8_t prescaler, uint8_t cmp );
void startTimer0();
void stopTimer0();

extern timerDefinition	Timer0;

#endif /* TIMERS_H_ */