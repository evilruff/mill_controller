/*
 * ports.h
 *
 * Created: 10/5/2020 3:52:18 PM
 *  Author: evilruff
 */ 


#ifndef PORTS_H_
#define PORTS_H_

#include <avr/io.h>
#include "utils.h"

#ifdef PORTA
	struct PortA
	{
		static uint8_t port() { return PORTA; }
		static void port(uint8_t v) { PORTA = v; }

		static uint8_t pin() { return PINA; }
		static void pin(uint8_t v) { PINA = v; }

		static uint8_t dir() { return DDRA; }
		static void dir(uint8_t v) { DDRA = v; }
			
	};
#endif

#ifdef PORTB
	struct PortB
	{
		static uint8_t port() { return PORTB; }
		static void port(uint8_t v) { PORTB = v; }

		static uint8_t pin() { return PINB; }
		static void pin(uint8_t v) { PINB = v; }

		static uint8_t dir() { return DDRB; }
		static void dir(uint8_t v) { DDRB = v; }
	};
#endif

#ifdef PORTC
	struct PortC
	{
		static uint8_t port() { return PORTC; }
		static void port(uint8_t v) { PORTC = v; }

		static uint8_t pin() { return PINC; }
		static void pin(uint8_t v) { PINC = v; }

		static uint8_t dir() { return DDRC; }
		static void dir(uint8_t v) { DDRC = v; }
	};
#endif

struct PortD
{
	static uint8_t port() { return PORTD; }
	static void port(uint8_t v) { PORTD = v; }

	static uint8_t pin() { return PIND; }
	static void pin(uint8_t v) { PIND = v; }

	static uint8_t dir() { return DDRD; }
	static void dir(uint8_t v) { DDRD = v; }
};

template <typename Port, int pin>
struct Pin
{
	static void set(bool value = true)
	{
		if (value)
		Port::port(Port::port() | (1<<pin));
		else
		Port::port(Port::port() & ~(1<<pin));
	}

	static void clear() { set(false); }
	
	static void toggle() { Port::port(Port::port() ^ (1<<pin)); }

	static bool get() { return (Port::port() & (1<<pin)) != 0; }

	static bool value() { return (Port::pin() & (1<<pin)) != 0; }

	static void output(bool value)
	{
		if (value)
		Port::dir(Port::dir() | (1<<pin));
		else
		Port::dir(Port::dir() & ~(1<<pin));
	}

	static bool output() { return (Port::dir() & (1<<pin)) != 0; }

	static void make_output() { output(true); }
	static void make_input() { output(false); clear(); }
	static void make_low() { clear(); output(true); }
	static void make_high() { set(); output(true); }
	static void set_value(bool value) { set(value); }
	static void set_high() { set(true); }
	static void set_low() { set(false); }
	static bool read() { return value(); }
	static void pullup() { set_high(); }
};

template <typename Port, int pin>
struct Inv_Pin
{
	static void set(bool value = true)
	{
		if (value)
		Port::port(Port::port() & ~(1<<pin));
		else
		Port::port(Port::port() | (1<<pin));
	}

	static void clear() { set(false); }
	
	static void toggle() { Port::port(Port::port() ^ (1<<pin)); }

	static bool get() { return (Port::port() & (1<<pin)) == 0; }

	static bool value() { return (Port::pin() & (1<<pin)) == 0; }

	static void output(bool value)
	{
		if (value)
		Port::dir(Port::dir() | (1<<pin));
		else
		Port::dir(Port::dir() & ~(1<<pin));
	}

	static bool output() { return (Port::dir() & (1<<pin)) != 0; }

	static void make_output() { output(true); }
	static void make_input() { output(false); clear(); }
	static void make_low() { clear(); output(true); }
	static void make_high() { set(); output(true); }
	static void set_value(bool value) { set(value); }
	static void set_high() { set(true); }
	static void set_low() { set(false); }
	static bool read() { return value(); }
	static void pullup() { set_low(); }
};

template<typename dataPin, typename clockPin> void shiftOut(uint8_t bitOrder, uint8_t val) {
	uint8_t i;

	for (i = 0; i < 8; i++)  {
		if (bitOrder == LSBFIRST)
			dataPin::set(!!(val & (1 << i)));
        else      
			dataPin::set(!!(val & (1 << (7 - i))));
                 
        clockPin::set_high();
        clockPin::set_low();            
	}
}

template<typename dataPin, typename clockPin> uint8_t shiftIn(uint8_t bitOrder) {

 uint8_t value = 0;
 uint8_t i = 7;
 
 do {

 clockPin::set_high();

 if (bitOrder == LSBFIRST)
	value |= (dataPin::value() << (7-i));
 else
	value |= (dataPin::value()) << i;

 clockPin::set_low();

 } while (i--);

 return value;

}


#endif /* PORTS_H_ */