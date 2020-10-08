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

typedef uint8_t (*setupTimerFunc)(uint16_t desiredFrequency, uint8_t autostart, uint8_t disableInterrupts);
typedef void (*startTimerFunc)();
typedef void (*stopTimerFunc)();

uint8_t setupTimer0(uint16_t desiredFrequency, uint8_t autostart = True, uint8_t disableInterrupts = True);
void startTimer0();
void stopTimer0();


#endif /* TIMERS_H_ */