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


#ifndef ENCODER_H_
#define ENCODER_H_

typedef void (*encoderEvent)(int8_t currentStep, int16_t currentTicks);

template <typename DtPin, typename ClkPin> class Encoder {
	public:
	
		Encoder() {
			m_state			= 0;
			m_currentTicks  = 0;
			onTicksChanged  = NULL;
			
			m_minValue = 0xFFFF;
			m_maxValue = 0xFFFF;
		}
		
		void max() const { return m_maxValue; };
		void min() const { return m_minValue; };
			
		void setMax(int16_t v) { m_maxValue = v; if (m_currentTicks > v) m_currentTicks = v; };		
		void setMin(int16_t v) { m_minValue = v; if (m_currentTicks < v) m_currentTicks = v; };
			
		void setup() {
			DtPin::make_input();
			ClkPin::make_input();
		}
		
		int8_t	tick() {
				int8_t currentStep = encoderStep();
				if (currentStep != 0) {
					
					int16_t previousTicks = m_currentTicks;
					
					m_currentTicks += currentStep;
					
					if ((uint16_t)m_minValue != 0xFFFF) {
						if (m_currentTicks < m_minValue) m_currentTicks = m_minValue;
					}
					if ((uint16_t)m_maxValue != 0xFFFF) {
						if (m_currentTicks > m_maxValue) m_currentTicks = m_maxValue;
					}
					
					if (onTicksChanged && (previousTicks != m_currentTicks)) {
						onTicksChanged(currentStep, m_currentTicks);
					}
				}
				
				return currentStep;
		}
		
		int16_t currentTicks() const { return m_currentTicks; };
		void	resetTicks() { m_currentTicks = 0; };		
		void	setTicks(int16_t t) { m_currentTicks = t; };
			
		encoderEvent	onTicksChanged;
		
	protected:
		
		int8_t	encoderStep() {
			static int8_t rot_enc_table[] = {0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0};

			uint8_t  prevNextCode	= m_state & 0xFF;
			uint16_t store			= (m_state & 0xFFFF0000) >> 16;

			prevNextCode <<= 2;
			if ( DtPin::value()  )    prevNextCode |= 0x02;		  // data
			if ( ClkPin::value() )	prevNextCode |= 0x01; // clk
			prevNextCode &= 0x0f;

			// If valid then store as 16 bit data.
			if  (rot_enc_table[prevNextCode] ) {
				store <<= 4;
				store |= prevNextCode;
				//if (store==0xd42b) return 1;
				//if (store==0xe817) return -1;
				if ((store & 0xff)==0x2b) {
					m_state = ((uint32_t)store << 16) | prevNextCode;
					return 1;
				}
				if ((store & 0xff)==0x17) {
					m_state = ((uint32_t)store << 16) | prevNextCode;
					return -1;
				}
			}
			
			m_state = ((uint32_t)store << 16) | prevNextCode;
			return 0;
		}
	
		int16_t		m_currentTicks;	
		int16_t		m_minValue;
		int16_t		m_maxValue;
		uint32_t	m_state;
};


#endif /* ENCODER_H_ */