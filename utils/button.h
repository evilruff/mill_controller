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

#define		Pressed		1
#define		Released	0

typedef void (*buttonEvent)(void);

template <typename InputPin> class Button {

	public:
		Button() {
				m_state  = 0;
				m_buffer = 0;
				m_mask	 = 0x0F; // default to 4

				onPressed  = NULL;
				onReleased = NULL;
				onClicked  = NULL;
		}		
		
		void    setup() {
			InputPin::make_input();
			InputPin::pullup();
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
						
						if (onClicked) {
							onClicked();
						}
						
						return;
					}
				}
				
				if (!d) { // on
					if (!m_state) {
						m_state = 1; // state changed;
						
						if (onPressed) {
							onPressed();
						}
						
						return;
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
		
	protected:

		uint8_t			m_mask;	
		uint8_t			m_buffer;	
		uint8_t			m_state;				
		
};


#endif /* BUTTON_H_ */