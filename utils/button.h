/*
 * button.h
 *
 * Created: 10/5/2020 12:29:42 PM
 *  Author: evilruff
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


template <typename InputPin, typename DelayedTimer = NoDelayedClick > class Button {

	public:
	
		Button(uint16_t longClickDelay = 1000, Clock * pClock = NULL):
			m_delay(pClock) {
			
				m_state  = 0;
				m_buffer = 0;
				m_mask	 = 0x0F; // default to 4
				m_fired	 = 0;

				onPressed  = NULL;
				onReleased = NULL;
				onClicked  = NULL;
				onLongClicked = NULL;
				
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
						m_state = 0; // state changed;
						
						if (onReleased) {
							onReleased();
						}
												
						if (!m_fired && onClicked) {
							onClicked();
						}
												
						return;
					}
				}
				
				if (!d) { // on
					if (!m_state) {
						m_state = 1; // state changed;
						m_fired = 0;
						m_delay.restart();
						
						if (onPressed) {
							onPressed();
						}
						
						return;
					} else {
						if (!m_fired && m_delay.isExpired(m_longClickDelay)) {
							m_fired = 1;
							if (onLongClicked) {
								onLongClicked();
							}
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