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
