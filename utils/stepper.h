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


#ifndef STEPPER_H_
#define STEPPER_H_

#include <stdlib.h>
#include <util/atomic.h>
#include <util/delay_basic.h>

#include "timers.h"
#include "logger.h"

#define LIMIT_LOW	0x01
#define LIMIT_HIGH	0x02


#define T1_FREQ (F_CPU/8)
#define T1_FREQ_148 ((uint16_t)(((float)T1_FREQ*0.676)/100)) // divided by 100 and scaled by 0.676

#define STOP  0
#define ACCEL 1
#define DECEL 2
#define RUN   3

#define CW	  0
#define CCW   1

#define LONG_STEP	1

template <typename StepPin, typename DirPin, typename EnablePin> class Stepper {
	
	protected:
	
		float	 __ALPHA;
	    uint32_t __A_T_x100;
		uint32_t __A_SQ;
		uint16_t __A_x20000;
		
		static const	uint16_t	usPerLoop = 1000000L/(F_CPU/1000000L)*4/1000;

	public:
				
		Stepper() {			
			
			__ALPHA    = 0.0;
			__A_T_x100 = 0;
			__A_SQ     = 0;
			__A_x20000 = 0;
			
			setStepWidth(15);
			setDirChangeDelay(15);	
			
			m_stepsPerTurn	= 200;
			m_feedPerTurn	= 2;							
			m_stepCount = 0;			
			m_state.max_delay = 100;		
			
			m_maximumSpeed = 0;	
			m_currentPosition = 0;	
		}
		
		void	setMinimumAccelerationSpeed(uint16_t s) {
			m_state.max_delay = __A_T_x100 / s;
		}
		
		void	setMaximumSpeed(uint16_t s) {
			m_maximumSpeed = s;
		}
		
		void	precalc(uint16_t stepsPerTurn) {
			
			__ALPHA		= 2*3.14159/(float)stepsPerTurn;
			__A_T_x100  = (uint32_t)(__ALPHA*T1_FREQ*100);
			__A_SQ		= (uint32_t)((float)__ALPHA*2*10000000000L);
			__A_x20000  = (uint16_t)(__ALPHA*(float)20000);
			m_state.max_delay = __A_T_x100 / 140;				
			
			LOG("STEPPER PRECALC");			
			LOG("-----------------------------------");
								
			LOG("T1_FREQ     %10lu",  T1_FREQ);
			LOG("T1_FREQ_148 %10u",   T1_FREQ_148);
			LOG("__ALPHA     %10lf",  __ALPHA);
			LOG("__A_T_x100  %10lu", __A_T_x100);
			LOG("__A_SQ      %10lu", __A_SQ);
			LOG("__A_x20000  %10u",  __A_x20000);
			LOG("-----------------------------------");			
		}
		
		void	setHardwareParameters(uint16_t stepsPerTurn, uint8_t feedPerTurn) {
			m_stepsPerTurn	= stepsPerTurn;
			m_feedPerTurn	= feedPerTurn;
			
			precalc(stepsPerTurn);			
		}
		
		uint16_t	feedPerTurn()  const { return m_feedPerTurn; };
		uint16_t	stepsPerTurn() const { return m_stepsPerTurn; };	
		
		void	setup() {
			
				
			StepPin::make_output();
			DirPin::make_output();
			EnablePin::make_output();	
			
			StepPin::set_low();		
			DirPin::set_low();		
			EnablePin::set_low();	
			
			// Tells what part of speed ramp we are in.
			m_state.run_state = STOP;
			// Timer/Counter 1 in mode 4 CTC (Not running).
			TCCR1B = (1<<WGM12);
			// Timer/Counter 1 Output Compare A Match Interrupt enable.
			TIMSK1 = (1<<OCIE1A);	
		}
		
		void    setStepWidth(uint16_t w) {
			if (w>15) 
				w = 15;
				
			m_stepDelay = w*1000000L/usPerLoop/1000;
		}
		
		void    setDirChangeDelay(uint16_t w) {
			if (w>15)
			w = 15;
			
			m_dirDelay = w*1000000L/usPerLoop/1000;
		}
		
		uint16_t stepDelay() const {
			return m_stepDelay;
		}
		
		void	doStep() {			
			  // Holds next delay period.
			  uint16_t new_step_delay = 0;
			  // Remember the last step delay used when accelerating.
			  static uint16_t last_accel_delay;
			  
			  // Keep track of remainder from new_step-delay calculation to increase accuracy
			  static uint16_t rest = 0;

			  OCR1A = m_state.step_delay;

			  switch(m_state.run_state) {
				  case STOP:
					m_stepCount = 0;
					rest = 0;
					// Stop Timer/Counter 1.
					TCCR1B &= ~((1<<CS12)|(1<<CS11)|(1<<CS10));
				  break;

				  case ACCEL:
					driverStep();				 
					m_stepCount++;
					m_state.accel_count++;
					new_step_delay = m_state.step_delay - (((2 * (long)m_state.step_delay) + rest)/(4 * m_state.accel_count + 1));
					rest = ((2 * (long)m_state.step_delay)+rest)%(4 * m_state.accel_count + 1);
					// Check if we should start deceleration.
					if(m_stepCount >= m_state.decel_start) {
						  m_state.accel_count = m_state.decel_val;
						m_state.run_state   = DECEL;
					}
					// Check if we hit max speed.
					else if(new_step_delay <= m_state.min_delay) {
						  last_accel_delay = new_step_delay;
						new_step_delay = m_state.min_delay;
						rest = 0;
						m_state.run_state = RUN;
					}
				  break;

				  case RUN:
					driverStep();
					m_stepCount++;
					new_step_delay = m_state.min_delay;
					// Check if we should start deceleration.
					if(m_stepCount >= m_state.decel_start) {
						  m_state.accel_count = m_state.decel_val;
						// Start deceleration with same delay as accel ended with.
						new_step_delay = last_accel_delay;
						m_state.run_state = DECEL;
					}
				  break;

				  case DECEL:
					driverStep();
					m_stepCount++;
					m_state.accel_count++;
					new_step_delay = m_state.step_delay - (((2 * (long)m_state.step_delay) + rest)/(4 * m_state.accel_count + 1));
					rest = ((2 * (long)m_state.step_delay)+rest)%(4 * m_state.accel_count + 1);
					// Check if we at last step
					if (new_step_delay > m_state.max_delay) {
						new_step_delay = m_state.max_delay;
					}
					
					if (m_state.accel_count >= 0) {
						  m_state.run_state = STOP;
					}
					break;
			  }
			  m_state.step_delay = new_step_delay;	
			  
		}
		
		void	driverStep() {
			uint8_t		currentDir = DirPin::get();
			if (currentDir != m_state.dir) {
				DirPin::set(m_state.dir);
				_delay_loop_2(m_dirDelay);
			}
			
			StepPin::set_high();
			#ifdef LONG_STEP
			_delay_loop_2(m_stepDelay);			
			_delay_loop_2(m_stepDelay);
			#else
			_delay_loop_2(m_stepDelay);
			#endif
			StepPin::set_low();
			
			m_currentPosition += m_state.dir ? 1 : -1;
		}
		
		void	setEnabled(uint8_t b) {
					
			EnablePin::set_value(!b);
		}
		
		void move(int32_t step, uint16_t accel, uint16_t decel, uint16_t speed) {
			 if (m_state.max_delay == 0)
				return;

			if (m_maximumSpeed && speed > m_maximumSpeed) {
				speed = m_maximumSpeed;
			}
			
			 ATOMIC_BLOCK(ATOMIC_FORCEON)
			 {
				if ((m_state.run_state == RUN) || (m_state.run_state == ACCEL)) {
					uint8_t requestedDir = step < 0 ? CCW : CW;
					if (requestedDir == m_state.dir) {
						m_state.decel_start += abs(step);
						return;
					} else {
						m_state.decel_start = m_stepCount+1;
						return;
					}
				}
			
				if (m_state.run_state != STOP)
					return;
			 }
			 
		//! Number of steps before we hit max speed.
		uint16_t max_s_lim = 0;
		//! Number of steps before we must start deceleration (if accel does not hit max speed).
		uint16_t accel_lim = 0;

		// Set direction from sign on step value.
		if(step < 0){
			m_state.dir = CCW;
			step = -step;
		}
		else{
			m_state.dir = CW;
		}

		// If moving only 1 step.
		if(step == 1){
			// Move one step...
			m_state.accel_count = -1;
			// ...in DECEL state.
			m_state.run_state = DECEL;
			// Just a short delay so main() can act on 'running'.
			m_state.step_delay = 1000;
			
			OCR1A = 10;
			// Run Timer/Counter 1 with prescaler = 8.
			TCCR1B |= ((0<<CS12)|(1<<CS11)|(0<<CS10));
		}
		// Only move if number of steps to move is not zero.
		else if(step != 0){
			// Refer to documentation for detailed information about these calculations.

			// Set max speed limit, by calc min_delay to use in timer.
			// min_delay = (alpha / tt)/ w
			m_state.min_delay = (uint16_t)(((float)T1_FREQ/(float)speed)); // __A_T_x100 / speed;

			// Set acceleration by calc the first (c0) step delay .
			// step_delay = 1/tt * sqrt(2*alpha/accel)
			// step_delay = ( tfreq*0.676/100 )*100 * sqrt( (2*alpha*10000000000) / (accel*100) )/10000
			m_state.step_delay = (T1_FREQ_148 * sqrt(__A_SQ / accel))/100;
			if (m_state.step_delay > m_state.max_delay) {
				m_state.step_delay = m_state.max_delay;
			}
			// Find out after how many steps does the speed hit the max speed limit.
			// max_s_lim = speed^2 / (2*alpha*accel)
			max_s_lim = (long)speed*speed/(long)(((long)__A_x20000*accel)/100);
			// If we hit max speed limit before 0,5 step it will round to 0.
			// But in practice we need to move at least 1 step to get any speed at all.
			if(max_s_lim == 0){
				max_s_lim = 1;
			}

			// Find out after how many steps we must start deceleration.
			// n1 = (n1+n2)decel / (accel + decel)
			accel_lim = ((long)step*decel) / (accel+decel);
			// We must accelerate at least 1 step before we can start deceleration.
			if(accel_lim == 0){
				accel_lim = 1;
			}

			// Use the limit we hit first to calc decel.
			if(accel_lim <= max_s_lim){
				m_state.decel_val = accel_lim - step;
			}
			else{
				m_state.decel_val = -((long)max_s_lim*accel)/decel;
			}
			// We must decelerate at least 1 step to stop.
			if(m_state.decel_val == 0){
				m_state.decel_val = -1;
			}

			// Find step to start deceleration.
			m_state.decel_start = step + m_state.decel_val;

			// If the maximum speed is so low that we dont need to go via acceleration state.
			if(m_state.step_delay <= m_state.min_delay){
				m_state.step_delay = m_state.min_delay;
				m_state.run_state = RUN;
			}
			else{
				m_state.run_state = ACCEL;
			}

			// Reset counter.
			m_state.accel_count = 0;
			
			OCR1A = 10;
			// Set Timer/Counter to divide clock by 8
			TCCR1B |= ((0<<CS12)|(1<<CS11)|(0<<CS10));
		}
	}
		
		void    run() {		
			
		}
		
		void	stop() {
			ATOMIC_BLOCK(ATOMIC_FORCEON)
			{
					 if ((m_state.run_state == RUN) || (m_state.run_state == ACCEL)) {
							 m_state.decel_start = m_stepCount+1;
							 return;
					 }
					 
					 if (m_state.run_state == DECEL) {
						 m_state.run_state = STOP;
					 }			
			}		
		}
		
		void	emergencyStop() { // ignore accel
			ATOMIC_BLOCK(ATOMIC_FORCEON)
			{
				m_state.run_state = STOP;
			}
		}
		
		int32_t		targetPosition() const { return 0; };
		
		int32_t		currentPosition() const {
			ATOMIC_BLOCK(ATOMIC_FORCEON)
			{					  
				return m_currentPosition;
			}
		};
		
		void		setCurrentPosition(int32_t position) {
			ATOMIC_BLOCK(ATOMIC_FORCEON)
			{
				m_currentPosition = position;
			}
		};
		
		int32_t	toMetric(uint32_t v,uint8_t scale=1) const { 
			return v*(uint32_t)scale/((uint32_t)m_stepsPerTurn/(uint32_t)m_feedPerTurn); 
		};	
			
		uint16_t	feed() const { return 0; };
		void 		setFeed(uint16_t v) const {};	

			
		
	protected:

		struct speedRampData {
			uint8_t run_state : 3;
			//! Direction stepper motor should move.
			uint8_t dir : 1;
			//! Period of next timer delay. At start this value set the accelration rate.
			uint16_t step_delay;
			//! What step_pos to start decelaration
			volatile uint32_t decel_start;
			//! Sets deceleration rate.
			volatile int16_t decel_val;
			//! Minimum time delay (max speed)
			uint16_t min_delay;
			//! Maximum time delay (min speed)
			uint16_t max_delay;			
			//! Counter used when accelerateing/decelerateing to calculate step_delay.
			volatile int16_t accel_count;
		}	m_state;
		
		// Counting steps when moving.
		
		uint32_t			m_stepCount;		
		
		uint16_t			m_stepsPerTurn;
		uint16_t			m_feedPerTurn;

		uint16_t			m_stepDelay;
		uint16_t			m_dirDelay;		
		
		uint16_t			m_maximumSpeed;
		
		volatile int32_t	m_currentPosition;			
};

#endif /* STEPPER_H_ */