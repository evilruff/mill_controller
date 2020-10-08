/*
 * settings.h
 *
 * Created: 10/7/2020 4:50:56 PM
 *  Author: evilruff
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