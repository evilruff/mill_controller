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


#ifndef JOYSTICK_H_
#define JOYSTICK_H_

typedef void (*JoystickEvent)(void);

#define JOYSTICK_LEFT    0x01
#define JOYSTICK_RIGHT   0x02
#define JOYSTICK_TOP     0x04
#define JOYSTICK_BOTTOM  0x08
#define JOYSTICK_NEUTRAL 0x00

template <	typename PinLeft,
			typename PinRight,
			typename PinTop,
			typename PinBottom			
> class Joystick {

	public:
	
	Joystick() {
		onLeft		= NULL;
		onRight		= NULL;
		onTop		= NULL;
		onBottom	= NULL;
		onNeutral	= NULL;	
		
		m_state  = 0;
		
		m_leftBuffer	= 0;
		m_rightBuffer   = 0;
		m_topBuffer		= 0;
		m_bottomBuffer  = 0;
		
		m_mask	 = 0x0F; // default to 4	
	}
		
	public:
		
		buttonEvent		onLeft;
		buttonEvent		onRight;
		buttonEvent		onTop;
		buttonEvent		onBottom;
		buttonEvent		onNeutral;
	
	void    setup() {
		PinLeft::make_input();
		PinLeft::pullup();
		PinRight::make_input();
		PinRight::pullup();
		PinTop::make_input();
		PinTop::pullup();
		PinBottom::make_input();
		PinBottom::pullup();		
	}
	
	void	tick() {
		
			uint8_t prevState = m_state;
			
			m_leftBuffer = (m_leftBuffer << 1) | PinLeft::value();			
			uint8_t d = m_leftBuffer & m_mask;
			
			if (d & m_mask) { // off				
				m_state &= ~(JOYSTICK_LEFT);				
			} else if (!d) {  // on
				m_state |= JOYSTICK_LEFT;
			}
			
			m_rightBuffer = (m_rightBuffer << 1) | PinRight::value();
			d = m_rightBuffer & m_mask;
						
			if (d & m_mask) { // off
					m_state &= ~(JOYSTICK_RIGHT);
				} else if (!d) {  // on
					m_state |= JOYSTICK_RIGHT;
			}
			
			m_topBuffer = (m_topBuffer << 1) | PinTop::value();
			d = m_topBuffer & m_mask;
			
			if (d & m_mask) { // off
				m_state &= ~(JOYSTICK_TOP);
				} else if (!d) {  // on
				m_state |= JOYSTICK_TOP;
			}
			
			m_bottomBuffer = (m_bottomBuffer << 1) | PinBottom::value();
			d = m_bottomBuffer & m_mask;
			
			if (d & m_mask) { // off
				m_state &= ~(JOYSTICK_BOTTOM);
				} else if (!d) {  // on
				m_state |= JOYSTICK_BOTTOM;
			}
			
			if (prevState == m_state) {
				return;
			}
						
			if (m_state == JOYSTICK_NEUTRAL && onNeutral) {
				onNeutral();
				return;
			}
			
			if (m_state & JOYSTICK_LEFT && onLeft) {
				onLeft();
				return;
			}

			if (m_state & JOYSTICK_RIGHT && onRight) {
				onRight();
				return;
			}

			if (m_state & JOYSTICK_TOP && onTop) {
				onTop();
				return;
			}
			
			if (m_state & JOYSTICK_BOTTOM && onBottom) {
				onBottom();
				return;
			}
	}
	
	uint8_t		state() const {
			
		if (m_state & JOYSTICK_LEFT) {			
			return JOYSTICK_LEFT;
		}

		if (m_state & JOYSTICK_RIGHT) {			
			return JOYSTICK_RIGHT;
		}

		if (m_state & JOYSTICK_TOP) {			
			return JOYSTICK_TOP;
		}
		
		if (m_state & JOYSTICK_BOTTOM) {			
			return JOYSTICK_BOTTOM;
		}
		
		return JOYSTICK_NEUTRAL;
	}
	
	protected:
		
		uint8_t			m_mask;		
		uint8_t			m_state;		
		uint8_t			m_leftBuffer;
		uint8_t			m_rightBuffer;
		uint8_t			m_topBuffer;
		uint8_t			m_bottomBuffer;		
};



#endif /* JOYSTICK_H_ */