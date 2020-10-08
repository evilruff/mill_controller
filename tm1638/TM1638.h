
#ifndef TM16XX_h
#define TM16XX_h

#include <avr/io.h>

#include "ports.h"
#include "TM16XXFonts.h"

#define OFF 0
#define ON 1

// Data instruction set

// Data write mode set
#define WRITE_DATA_TO_REGISTER 0b01000000
#define READ_KEY_SCAN_DATA     0b01000010

// Address add mode set
#define AUTO_ADDRESS_ADD       0b01000000
#define FIXED_ADDRESS          0b01000100

// Test mode set (inner use)
#define NORMAL_MODE            0b01000000
#define TEST_MODE              0b01001000


// Display control instruction set

// clean light quantity set
#define SET_PULSE_WIDTH_1_16   0b10000000
#define SET_PULSE_WIDTH_2_16   0b10000001
#define SET_PULSE_WIDTH_4_16   0b10000010
#define SET_PULSE_WIDTH_10_16  0b10000011
#define SET_PULSE_WIDTH_11_16  0b10000100
#define SET_PULSE_WIDTH_12_16  0b10000101
#define SET_PULSE_WIDTH_13_16  0b10000110
#define SET_PULSE_WIDTH_14_16  0b10000111

// Display turn on/off set
#define SHOW_TURN_OFF          0b10000000
#define SHOW_TURN_ON           0b10001000

#define ADDRESS_INSTRUCTION_SET 0b11000000

#define FONT_MAX 24

template <typename StbPin, typename ClkPin, typename DioPin> class TM1638
{
	public:
	
	TM1638() {};

	
	void init(uint8_t brightness = 7) {
		
		StbPin::make_output();
		ClkPin::make_output();
		DioPin::make_output();

		StbPin::set_high();
		ClkPin::set_low();
		DioPin::set_low();

		setBrightness(brightness);
		clear();
	};

	void setBrightness(uint8_t value) {
		// 0 <= value <= 7
		if (value > 7) { value = 0; }
		sendCommand(value | SHOW_TURN_ON); // set brightness and activate
	};

	void clear() {
		sendCommand(AUTO_ADDRESS_ADD);
		StbPin::set_low();
		//shiftOut(data_pin, clock_pin, LSBFIRST, AUTO_ADDRESS_ADD);
		shiftOut<DioPin, ClkPin>(LSBFIRST, 0xc0);   // set starting address to 0
		for (uint8_t i = 0; i < 16; i++) {
			shiftOut<DioPin, ClkPin>(LSBFIRST, 0x00);
		}
		StbPin::set_high();
	};

	void setAddress(uint8_t address, uint8_t data) {
		sendCommand(FIXED_ADDRESS);
		StbPin::set_low();
		
		//shiftOut(data_pin, clock_pin, LSBFIRST, FIXED_ADDRESS);
		shiftOut<DioPin, ClkPin>(LSBFIRST, ADDRESS_INSTRUCTION_SET | address);
		shiftOut<DioPin, ClkPin>(LSBFIRST, data);
		
		StbPin::set_high();
	};

	void setLEDs(uint8_t leds) {
		for (uint8_t i = 0; i < 8; i++) {
			setLED(i, leds & (1 << (7-i)));
		}
	};

	void setLED(uint8_t led, uint8_t status) {
		setAddress(led * 2 + 1, status ? 1 : 0);
	};

	void setString(uint8_t digit, const char * str) {
		const char * ptr = str;
		while (*ptr) {
			uint8_t data = FONT_DEFAULT[*ptr-32];	
			if ((*(ptr+1)) && (*(ptr+1) == '.')) {
				setAddress((digit++) * 2, data | 0b10000000);
				ptr++;
			} else {	
				setAddress((digit++) * 2, data);
			}
			ptr++;
		};
	};
	
	void setDigit(uint8_t digit, uint8_t number, bool dotPoint = false) {		
		uint8_t data = FONT_DEFAULT[number + 16];
		if (dotPoint) {
			data = data | 0b10000000;
		}
		setAddress(digit * 2, data);
	};


	uint8_t readButtons() {
		StbPin::set_low();

		shiftOut<DioPin, ClkPin>(LSBFIRST, READ_KEY_SCAN_DATA);
		
		DioPin::make_input();
		
		uint8_t current_button_state = 0;
		current_button_state = shiftIn<DioPin, ClkPin>(LSBFIRST);
		current_button_state = current_button_state | (shiftIn<DioPin, ClkPin>(LSBFIRST)) << 1;
		current_button_state = current_button_state | (shiftIn<DioPin, ClkPin>(LSBFIRST)) << 2;
		current_button_state = current_button_state | (shiftIn<DioPin, ClkPin>(LSBFIRST)) << 3;
		
		DioPin::make_output();

		StbPin::set_high();
		return current_button_state;
	};

	protected:

	void sendCommand(uint8_t value) {
		
		StbPin::set_low();
		shiftOut<DioPin, ClkPin>(LSBFIRST, value);
		StbPin::set_high();
		
	};

};

#endif
