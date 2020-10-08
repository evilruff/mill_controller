/*
 * clock.h
 *
 * Created: 10/8/2020 3:46:51 PM
 *  Author: evilruff
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
			}
	
		void restart() {
				m_start = m_clock->millis();	
		}	
		
		uint8_t	isExpired(uint16_t msPeriod) const {
			return (m_clock->millis()-m_start > msPeriod);
		}
		
		uint16_t	elapsed() const {
			return (m_clock->millis()-m_start);
		}
		
	
	protected:	
		Clock *			m_clock;
		uint32_t		m_start;
};



#endif /* CLOCK_H_ */