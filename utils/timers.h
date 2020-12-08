/**
 *  Copyright 2020 by Yuri Alexandrov <evilruff@gmail.com>
 *
 * This file is part of some open source application.
 *
 * Some open source application is free software: you can redistribute
 * it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * Some open source application is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with QLogView.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
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