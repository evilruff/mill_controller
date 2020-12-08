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


#ifndef CLOCK_H_
#define CLOCK_H_

#include <util/atomic.h>

#include "timers.h"

class Clock {
	public:	
		Clock(uint16_t		freq, const timerDefinition	& timer) {
			startTimer	 = timer.start;
			stopTimer	 = timer.stop;
			setupTimer   = timer.setup;
			m_freq_div_1000 = freq/10;		
		};
	
		void start(uint16_t		freq = 0) {
			if (freq) {
				m_freq_div_1000 = freq/10;
			}
			
			uint8_t	mask = 0;
			uint8_t	cnt  = 0;
			if (calculatePrescaler(m_freq_div_1000*1000, mask, cnt,TIMER_8BIT)) {
				if (setupTimer && startTimer) {
					setupTimer(mask, cnt);
					startTimer();
				}
			}
		}
		
		void stop() {
			if (stopTimer) {
				stopTimer();
			}
		}
		
		uint32_t	millis() {
			  if( m_freq_div_1000 ) {
				ATOMIC_BLOCK(ATOMIC_FORCEON)
				{
					return m_ticks / m_freq_div_1000;
				}
			  }
			  return 0;
		}
		
		void		tick() {
			m_ticks++;
		}
		
	public:
		startTimerFunc	startTimer;
		stopTimerFunc	stopTimer;
		setupTimerFunc	setupTimer;
		
	protected:
	
			volatile uint32_t		m_ticks;
			uint16_t				m_freq_div_1000;
};

class Delay {
	public:
		Delay(Clock * clock):
			m_clock(clock) {
				m_start = m_clock->millis();	
				m_started = 1;
			}
		
		void restart() {
				m_start = m_clock->millis();	
				m_started = 1;
		}	
		
		void stop() {
			m_started = 0;
		}
		
		uint8_t isStarted() const {
			return m_started;
		}
		
		uint8_t	isExpired(uint16_t msPeriod) const {
			return (m_started && (m_clock->millis()-m_start > msPeriod));
		}
		
		uint16_t	elapsed() const {
			return (m_clock->millis()-m_start);
		}
		
	
	protected:	
		uint8_t			m_started;
		Clock *			m_clock;
		uint32_t		m_start;
};



#endif /* CLOCK_H_ */