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


#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <util/crc16.h>
#include <avr/eeprom.h>
#include <string.h>

template <typename T> class Settings {
	public:
		Settings(T * defaultValues ) {
			m_defaults = defaultValues;
			memmove( &d, m_defaults, sizeof(T));
		}

		void	store() {
			uint8_t		i;			
			uint16_t	crc = 0;
			
			for (i = 0; i < sizeof(T); i++)
				crc = _crc16_update(crc, ((uint8_t*)&d)[i]);
				
			eeprom_update_block(&d, NULL, sizeof(T));
			eeprom_update_block(&crc,(void*)sizeof(T), sizeof(crc));			
		}
		
		uint8_t	load() {
			uint16_t	readedCrc = 0;
			eeprom_read_block(&d, NULL, sizeof(T));
			eeprom_read_block(&readedCrc,(void*)sizeof(T), sizeof(readedCrc));			
			
			uint16_t	crc = 0;
						
			for (uint8_t  i = 0; i < (sizeof(T)); i++)
				crc = _crc16_update(crc, ((uint8_t*)&d)[i]);
			
			if (crc != readedCrc) { 	
				memmove( &d, m_defaults, sizeof(T));
				return False;
			}
			
			return True;
		}
		
	public:
		
		T	d;
			
	protected:
	
		T	* m_defaults;			
};

#endif /* SETTINGS_H_ */