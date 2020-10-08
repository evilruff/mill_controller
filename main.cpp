/*
 * firmware.cpp
 *
 * Created: 10/5/2020 12:28:57 PM
 * Author : evilruff
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#include "button.h"
#include "encoder.h"
#include "utils.h"
#include "uart.h"
#include "TM1638.h"
#include "ports.h"
#include "timers.h"
#include "stepper.h"
#include "defaults.h"
#include "settings.h"
#include "logger.h"

#define		DISPLAY_MODE_KEEP	0
#define		DISPLAY_MODE_FEED	1
#define		DISPLAY_MODE_CP		2
#define		DISPLAY_MODE_TP		3


	
void updateDisplay(uint8_t left, uint8_t right);
void initHardware();
void encoderButtonPressed();
void encoder0Rotated(int8_t direction, int16_t currentValue);
void encoder1Rotated(int8_t direction, int16_t currentValue);

Encoder<Pin<PortC, PINC3>, Pin<PortC, PINC4> > encoder0;
Encoder<Pin<PortB, PINB1>, Pin<PortB, PINB0> > encoder1;

Button< Pin<PortC, PINC5> >	encoderSwitch;

TM1638< Pin<PortC, PINC0> /*STB*/, 
		Pin<PortC, PINC1> /*CLK*/,
		Pin<PortC, PINC2> /*DIO*/ >	tm1638;
		
Pin<PortD, PIND2>		DiagnosticPin;

Stepper< Pin<PortD, PIND3> /* STEP	 */, 
		 Pin<PortD, PIND4> /* DIR	 */, 
		 Pin<PortD, PIND5> /* ENABLED */ >		stepper;
		 
Settings<FirmwareSettings>		settings(&defaultSettings);	

Logger	logger(uart0_puts);	
//------------------------------------------------------

int main(void)
{			
	uart_init(UART_BAUD_SELECT(115200, 16000000UL));
	
	LOG("------------ FIRMWARE Started --------------- ");
	
	/*if (!settings.load()) {
		LOG("Settings invalid, reset to default, storing");
		settings.store();
	} else {	
		LOG("Settings loaded, EEPROM.. Ok");
	}*/
	
	initHardware();
			
	updateDisplay(DISPLAY_MODE_FEED, DISPLAY_MODE_CP);
    	
	while (1) 
    {
		encoderSwitch.tick();
		encoder0.tick();
		encoder1.tick();	
		tm1638.readButtons();
		
		DiagnosticPin.toggle();		
		
		updateDisplay(DISPLAY_MODE_KEEP, DISPLAY_MODE_CP);	
    }
}
//------------------------------------------------------
void initHardware() {
	
	encoderSwitch.setup();
	encoder0.setup();
	encoder1.setup();	
	
	LOG("Stepper params:");
	uint16_t	v = 10;
	LOG("test:%d mm", v);
	LOG("Steps per turn:%d, feed per turn:%d mm", settings.d.stepsPerTurn, settings.d.feedPerTurn);
	LOG("Step width:%d us, Dir change delay:%d us", settings.d.stepWidth, settings.d.dirDelay);
	LOG("Minimum acceleration speed:%d, Maximum speed:%d", settings.d.minAccelFrequency, settings.d.maxFrequency);
	
	stepper.setHardwareParameters( settings.d.stepsPerTurn, settings.d.feedPerTurn);		
	stepper.setup();
	stepper.setStepWidth(settings.d.stepWidth);
	stepper.setDirChangeDelay(settings.d.dirDelay);
	stepper.setMaximumSpeed(settings.d.maxFrequency);
	stepper.setMinimumAccelerationSpeed(settings.d.minAccelFrequency);
	stepper.setEnabled(True);
	
	encoder0.setTicks(stepper.feed());
	
	tm1638.init(7);
		
	encoder0.onTicksChanged  = encoder0Rotated;
	encoder1.onTicksChanged  = encoder1Rotated;
	encoderSwitch.onClicked  = encoderButtonPressed;
	
	DiagnosticPin.make_output();
		
	
}
//------------------------------------------------------
void encoderButtonPressed() {
	uart_puts("Encoder Button pressed\r\n");
}
//------------------------------------------------------
void encoder0Rotated(int8_t direction, int16_t currentValue) {
	
	if (direction > 0) {
		LOG( "Enc0 CW: %i", currentValue);
	} else {
		LOG( "Enc0 CCW: %i", currentValue);
	}
	
	#if 0	
		stepper.setFeed(currentValue);
		sprintf(str, "Feed: %i  Speed:%lu\r\n", currentValue, stepper.speed());
	#endif
	
	updateDisplay(DISPLAY_MODE_FEED, DISPLAY_MODE_CP);		
}
//------------------------------------------------------
void encoder1Rotated(int8_t direction, int16_t currentValue) {	
	stepper.move( direction*1500, 200, 200, 5000);
	
	updateDisplay(DISPLAY_MODE_TP, DISPLAY_MODE_CP);		
}
//------------------------------------------------------
void formatDisplayValue(uint8_t type, char * buffer) {
	
	static int32_t	previouslyShownCurrentPosition = 0xFFFF;
	
	*buffer = 0;
	switch (type) {
		case DISPLAY_MODE_FEED:
		sprintf(buffer, "F.%3i", stepper.feed());
		break;
		case DISPLAY_MODE_CP: {
			int32_t	p = stepper.currentPosition();
			int16_t pMM = stepper.toMetric10th(p);
			
			if (previouslyShownCurrentPosition != p) {
				if (pMM < -999) {
					sprintf(buffer, "%4i", (int16_t)(pMM / 10));
				} else {
					sprintf(buffer, "%3i.%d", (int16_t)(pMM / 10), abs((int16_t)(pMM % 10)));
				}
				previouslyShownCurrentPosition = p;
			}
		}
		break;
		case DISPLAY_MODE_TP:{
			int32_t	p = stepper.targetPosition();	
			int16_t pMM = stepper.toMetric10th(p);
					
			if (pMM < -999) {
				sprintf(buffer, "%4i", (int16_t)(pMM / 10));
			} else {
				sprintf(buffer, "%3i.%d", (int16_t)(pMM / 10), abs((int16_t)(pMM % 10)));		
			}
		}
		break;
	}
}
//------------------------------------------------------
void updateDisplay(uint8_t left, uint8_t right) {
	char str1[16];
	char str2[16];
	
	char f = 0;
	formatDisplayValue(left, str1);	
	if (*str1) {
		f = 1;
		tm1638.setString(0, str1);
	}
		
	formatDisplayValue(right, str2);
	
	if (*str2) {
		f = 1;
		tm1638.setString(4, str2);
	}

#if 0	
	if (f) {
	char buf[32];			
	sprintf(buf, "[%s] [%s]\r\n", str1, str2);
	uart_puts(buf);		
	}
#endif

}
//------------------------------------------------------
ISR(TIMER1_COMPA_vect) {
	stepper.doStep();
}
//------------------------------------------------------
