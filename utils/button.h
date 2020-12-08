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

#ifndef BUTTON_H_
#define BUTTON_H_

#include <avr/io.h>

#include "utils.h"
#include "ports.h"
#include "clock.h"

#define		Pressed		1
#define		Released	0

typedef void (*buttonEvent)(void);

class NoDelayedClick {
	public:
		NoDelayedClick(Clock *) {};
		void		restart() {};		
		uint8_t		isExpired(uint16_t) const { return 0; };		
		uint16_t	elapsed() const { return 0; };
};

#define		BUTTON_STATE_OFF	0x00
#define		BUTTON_STATE_ON		0x01
#define		BUTTON_STATE_FIRED	0x02

template <typename InputPin, typename DelayedTimer = NoDelayedClick > class Button {

	public:
	
		Button(uint16_t longClickDelay = 1000, Clock * pClock = NULL):
			m_delay(pClock) {
			
				m_state  = BUTTON_STATE_OFF;
				m_buffer = 0;
				m_mask	 = 0x0F; // default to 4
				
				onPressed     = emptyHandler;
				onReleased    = emptyHandler;
				onClicked     = emptyHandler;
				onLongClicked = emptyHandler;
				
				m_longClickDelay = longClickDelay;				
		}		
		
		void    setup() {
			InputPin::make_input();
			InputPin::pullup();
		}
		
		void	setLongClickDelay(uint16_t d) {
			m_longClickDelay - d;
		}
		
		void	tick() {
				m_buffer = (m_buffer << 1) | InputPin::value();
				
				uint8_t d = m_buffer & m_mask;
				
				if (d & m_mask) { // off
					if (m_state) {
						if (m_state == BUTTON_STATE_ON) {
							onClicked();
						}						
						
						m_state = BUTTON_STATE_OFF; // state changed;					
						onReleased();
												
						return;
					}
				}
				
				if (!d) { // on
					if (!m_state) {
							m_state = BUTTON_STATE_ON; // state changed;
							m_delay.restart();
							onPressed();					
							return;
					} else if (m_state == BUTTON_STATE_ON) {
						if (m_delay.isExpired(m_longClickDelay)) {
							m_state = BUTTON_STATE_FIRED;							
							onLongClicked();							
						}
					}
				}
		}
		
		void	setMask(uint8_t mask) {
			m_mask = mask;			
		}
		
		uint8_t	mask() const {  return m_mask; }			
		uint8_t state() const { return m_state; }

		
	public:
	
		buttonEvent		onPressed;
		buttonEvent		onReleased;
		buttonEvent		onClicked;
		buttonEvent		onLongClicked;
		
	protected:

		uint8_t			m_mask;	
		uint8_t			m_buffer;	
		uint8_t			m_state;		
		
		DelayedTimer	m_delay;	
		uint16_t		m_longClickDelay;	
		
		uint8_t			m_fired;	
};


#endif /* BUTTON_H_ */