/*
 * firmware.cpp
 *
 * Created: 10/5/2020 12:28:57 PM
 * Author : evilruff
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

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
#include "clock.h"
#include "joystick.h"
#include "logic.h"

Clock	clock(100, Timer0);

Encoder<Pin<PortC, PINC4>, Pin<PortC, PINC5> > encoder0;
Encoder<Pin<PortD, PIND6>, Pin<PortD, PIND7> > encoder1;

Button< Pin<PortC, PINC3>, Delay >	encoderSwitch(BUTTON_LONG_PRESS_DELAY, &clock);

CREATE_VIRTUAL_PORT(PanelButtons);
CREATE_VIRTUAL_PORT(PanelLEDS);

TM1638< Pin<PortC, PINC0> /*STB*/, 
		Pin<PortC, PINC1> /*CLK*/,
		Pin<PortC, PINC2> /*DIO*/,
		PanelButtons,
		PanelLEDS >	tm1638;
		
Pin<PortD, PIND2>		DiagnosticPin;

Stepper< Pin<PortD, PIND3> /* STEP	 */, 
		 Pin<PortD, PIND4> /* DIR	 */, 
		 Pin<PortD, PIND5> /* ENABLED */ >		stepper;
		 
Settings<FirmwareSettings>		settings(&defaultSettings);	

Button< Pin<PanelButtons, PIN0>, Delay >	PanelButton1(BUTTON_LONG_PRESS_DELAY, &clock);
Button< Pin<PanelButtons, PIN1>, Delay >	PanelButton2(BUTTON_LONG_PRESS_DELAY, &clock);
Button< Pin<PanelButtons, PIN2>, Delay >	PanelButton3(BUTTON_LONG_PRESS_DELAY, &clock);
Button< Pin<PanelButtons, PIN3>, Delay >	PanelButton4(BUTTON_LONG_PRESS_DELAY, &clock);
Button< Pin<PanelButtons, PIN4>, Delay >	PanelButton5(BUTTON_LONG_PRESS_DELAY, &clock);
Button< Pin<PanelButtons, PIN5>, Delay >	PanelButton6(BUTTON_LONG_PRESS_DELAY, &clock);
Button< Pin<PanelButtons, PIN6>, Delay >	PanelButton7(BUTTON_LONG_PRESS_DELAY, &clock);
Button< Pin<PanelButtons, PIN7>, Delay >	PanelButton8(BUTTON_LONG_PRESS_DELAY, &clock);

Button< Pin<PortB, PINB4> >	Limit1;
Button< Pin<PortB, PINB5> >	Limit2;

Joystick< Pin<PortB, PINB0>,  // left
		  Pin<PortB, PINB1>,  // right
		  Pin<PortB, PINB2>,  // top
		  Pin<PortB, PINB3> > /*bottom*/	joystick;

Pin<PanelLEDS, PIN0>		PanelLed1;
Pin<PanelLEDS, PIN1>		PanelLed2;
Pin<PanelLEDS, PIN2>		PanelLed3;
Pin<PanelLEDS, PIN3>		PanelLed4;
Pin<PanelLEDS, PIN4>		PanelLed5;
Pin<PanelLEDS, PIN5>		PanelLed6;
Pin<PanelLEDS, PIN6>		PanelLed7;
Pin<PanelLEDS, PIN7>		PanelLed8;

Logger	logger(uart0_puts);

Logic	logic(&clock);

//------------------------------------------------------
void updateDisplay();
void initHardware();
void encoderButtonPressed();
void encoderButtonLongPressed();

void panelButton1Pressed();
void panelButton1LongPressed();
void panelButton2Pressed();
void panelButton2LongPressed();
void panelButton3Pressed();
void panelButton3LongPressed();
void panelButton4Pressed();
void panelButton4LongPressed();
void panelButton5Pressed();
void panelButton5LongPressed();
void panelButton6Pressed();
void panelButton6LongPressed();
void panelButton7Pressed();
void panelButton7LongPressed();
void panelButton8Pressed();
void panelButton8LongPressed();

void encoder0Rotated(int8_t direction, int16_t currentValue);
void encoder1Rotated(int8_t direction, int16_t currentValue);


void limit1Pressed();
void limit1Released();
void limit2Pressed();
void limit2Released();

void onJoystickLeft();
void onJoystickRight();
void onJoystickTop();
void onJoystickBottom();
void onJoystickNeutral();

//------------------------------------------------------
void doOnce();

int main(void)
{			
	uart_init(UART_BAUD_SELECT(115200, 16000000UL));
	
	LOG(PSTR("-------- FIRMWARE Started ---------"));
	
	/*if (!settings.load()) {
		LOG("Settings invalid, reset to default, storing");
		settings.store();
	} else {	
		LOG("Settings loaded, EEPROM.. Ok");
	}*/
			
	initHardware();
					
	updateDisplay();
    
	uint8_t previousStateLed = 0x00;
				
	doOnce();
					
	while (1) 
    {
		joystick.tick();			
			
		encoder0.tick();
		encoder1.tick();			
		
		tm1638.tick();
		
		DiagnosticPin.toggle();		

		encoderSwitch.tick();		
		
		PanelButton1.tick();
		PanelButton2.tick();
		PanelButton3.tick();
		PanelButton4.tick();
		PanelButton5.tick();
		PanelButton6.tick();
		PanelButton7.tick();
		PanelButton8.tick();
		
		
		Limit1.tick();
		Limit2.tick();

		updateDisplay();			
		
		logic.tick();
    }
}
//------------------------------------------------------
void doOnce() {
	joystick.tick();
	switch(joystick.state()) {
		case JOYSTICK_BOTTOM: logic.setMode(Logic::ModeAuto); break;
		case JOYSTICK_TOP: logic.setMode(Logic::ModeStep); break;
		case JOYSTICK_LEFT: logic.setMode(Logic::ModeToNeutral); break;
		case JOYSTICK_RIGHT: logic.setMode(Logic::ModeToNeutral); break;
		case JOYSTICK_NEUTRAL: logic.setMode(Logic::ModeFeed); break;
	}
	
	logic.setInitialDisplayMode();	
}
//------------------------------------------------------
void initHardware() {
	
	clock.start();
	
	encoderSwitch.setup();
	encoder0.setup();
	encoder1.setup();	
	
	Limit1.setup();
	Limit2.setup();
	
	joystick.setup();
		
	LOG_P("STEPPERS PARAMS");
	LOG_P("-----------------------------------");
	LOG_P("Steps per turn:  %6d", settings.d.stepsPerTurn);
	LOG_P("Feed per turn:   %6d mm", settings.d.feedPerTurn);
	LOG_P("Step width:      %6d us", settings.d.stepWidth);
	LOG_P("Dir change delay:%6d us", settings.d.dirDelay);	
	LOG_P("Min accel speed: %6d Hz", settings.d.minAccelFrequency);
	LOG_P("Maximum speed:   %6d Hz", settings.d.maxFrequency);
	LOG_P("-----------------------------------");
	
	stepper.setHardwareParameters( settings.d.stepsPerTurn, settings.d.feedPerTurn);		
	stepper.setup();
	stepper.setStepWidth(settings.d.stepWidth);
	stepper.setDirChangeDelay(settings.d.dirDelay);
	stepper.setMaximumSpeed(settings.d.maxFrequency);
	stepper.setMinimumAccelerationSpeed(settings.d.minAccelFrequency);
	stepper.setEnabled(True);
	
	encoder0.setTicks(stepper.feed());
	
	tm1638.init(7);
		
	encoder0.onTicksChanged		= encoder0Rotated;
	encoder1.onTicksChanged		= encoder1Rotated;
	
	encoderSwitch.onClicked		= encoderButtonPressed;
	encoderSwitch.onLongClicked = encoderButtonLongPressed;
	
	PanelButton1.onClicked		= panelButton1Pressed;
	PanelButton1.onLongClicked  = panelButton1LongPressed;
	PanelButton2.onClicked		= panelButton2Pressed;
	PanelButton2.onLongClicked  = panelButton2LongPressed;
	PanelButton3.onClicked		= panelButton3Pressed;
	PanelButton3.onLongClicked  = panelButton3LongPressed;
	PanelButton4.onClicked		= panelButton4Pressed;
	PanelButton4.onLongClicked  = panelButton4LongPressed;
	PanelButton5.onClicked		= panelButton5Pressed;
	PanelButton5.onLongClicked  = panelButton5LongPressed;
	PanelButton6.onClicked		= panelButton6Pressed;
	PanelButton6.onLongClicked  = panelButton6LongPressed;
	PanelButton7.onClicked		= panelButton7Pressed;
	PanelButton7.onLongClicked  = panelButton7LongPressed;
	PanelButton8.onClicked		= panelButton8Pressed;
	PanelButton8.onLongClicked  = panelButton8LongPressed;
	
	joystick.onLeft				= onJoystickLeft;
	joystick.onRight			= onJoystickRight;
	joystick.onTop				= onJoystickTop;
	joystick.onBottom			= onJoystickBottom;
	joystick.onNeutral			= onJoystickNeutral;
	
	Limit1.onPressed  = limit1Pressed;
	Limit1.onReleased = limit1Released;
	Limit2.onPressed  = limit2Pressed;
	Limit2.onReleased = limit2Released;

	DiagnosticPin.make_output();		
}
//------------------------------------------------------
void encoderButtonPressed() {
	LOG_P("Encoder Button pressed");
}
//------------------------------------------------------
void encoderButtonLongPressed() {
	LOG_P("Encoder Button LONG pressed");
	stepper.setCurrentPosition(0);
}
//------------------------------------------------------
void panelButton1Pressed() {
	LOG_P("Panel Button 1 pressed");
	PanelLed1.set_high();
}
//------------------------------------------------------
void panelButton1LongPressed() {
	LOG_P("Panel LONG Button 1 pressed");
	PanelLed1.set_low();
}
//------------------------------------------------------
void panelButton2Pressed() {
	PanelLed2.set_high();
	LOG_P("Panel Button 2 pressed");	
}
//------------------------------------------------------
void panelButton2LongPressed() {
	PanelLed2.set_low();
	LOG_P("Panel LONG Button 2 pressed");
}
//------------------------------------------------------
void panelButton3Pressed() {
	PanelLed3.set_high();
	LOG_P("Panel Button 3 pressed");
}
//------------------------------------------------------
void panelButton3LongPressed() {
	PanelLed3.set_low();
	LOG_P("Panel LONG Button 3 pressed");
}
//------------------------------------------------------
void panelButton4Pressed() {
	PanelLed4.set_high();
	LOG_P("Panel Button 4 pressed");
}
//------------------------------------------------------
void panelButton4LongPressed() {
	PanelLed4.set_low();
	LOG_P("Panel LONG Button 4 pressed");
}
//------------------------------------------------------
void panelButton5Pressed() {
	PanelLed5.set_high();
	LOG_P("Panel Button 5 pressed");
}
//------------------------------------------------------
void panelButton5LongPressed() {
	PanelLed5.set_low();
	LOG_P("Panel LONG Button 5 pressed");
}
//------------------------------------------------------
void panelButton6Pressed() {
	PanelLed6.set_high();
	LOG_P("Panel Button 6 pressed");
}
//------------------------------------------------------
void panelButton6LongPressed() {
	PanelLed6.set_low();
	LOG_P("Panel LONG Button 6 pressed");
}
//------------------------------------------------------
void panelButton7Pressed() {
	PanelLed7.set_high();	
	LOG_P("Panel Button 7 pressed");
}
//------------------------------------------------------
void panelButton7LongPressed() {
	PanelLed7.set_low();
	LOG_P("Panel LONG Button 7 pressed");
}
//------------------------------------------------------
void panelButton8Pressed() {
	LOG_P("Panel Button 8 pressed");	
	logic.nextPositionScale();
}
//------------------------------------------------------
void panelButton8LongPressed() {
	LOG_P("Panel LONG Button 8 pressed");
}
//------------------------------------------------------
void limit1Pressed() {	
	LOG_P("Limit 1 pressed");
	stepper.emergencyStop();
}
//------------------------------------------------------
void limit1Released() {
	LOG_P("Limit 1 released");
}
//------------------------------------------------------
void  limit2Pressed() {
	LOG_P("Limit 2 pressed");
	stepper.emergencyStop();
}
//------------------------------------------------------
void limit2Released() {
	LOG_P("Limit 2 released");
}
//------------------------------------------------------
void onJoystickLeft() {	
	int32_t	steps = FEED_MAX_STEPS;		
	stepper.move( steps, 500, 500, 5000);	
}
//------------------------------------------------------
void onJoystickRight() {
	int32_t	steps = -FEED_MAX_STEPS;
	stepper.move( steps, 500, 500, 5000);
}
//------------------------------------------------------
void onJoystickTop() {
	logic.setMode(Logic::ModeStep);	
	stepper.stop();
	LOG_P("Joystick top");
}
//------------------------------------------------------
void onJoystickBottom() {
	logic.setMode(Logic::ModeAuto);	
	stepper.stop();
	LOG_P("Joystick bottom");
}
//------------------------------------------------------
void onJoystickNeutral() {
	logic.setMode(Logic::ModeFeed);	
	stepper.stop();
	LOG_P("Joystick neutral");
}
//------------------------------------------------------

void encoder0Rotated(int8_t direction, int16_t currentValue) {
	
	switch(logic.mode()) {
		case Logic::ModeStep:
			if (direction < 0) 
				logic.nextStepInterval();
			else 
				logic.previousStepInterval();										
		break;		
	}
			
	#if 0
		if (direction > 0) {
			LOG_P( "Enc0 CW: %i", currentValue);
		} else {
			LOG_P( "Enc0 CCW: %i", currentValue);
		}
	#endif
	
	#if 0	
		stepper.setFeed(currentValue);
		sprintf(str, "Feed: %i  Speed:%lu\r\n", currentValue, stepper.speed());
	#endif	
}
//------------------------------------------------------
void encoder1Rotated(int8_t direction, int16_t currentValue) {	
	
	switch(logic.mode()) {
		case Logic::ModeStep: {
			logic.makeStep(direction);
			
			uint32_t	steps = logic.ticksPerStep( stepper.feedPerTurn(), stepper.stepsPerTurn() );
			
			stepper.move( direction*steps, 500, 500, 5000);
		}
		break;
	}
	
	// updateDisplay(DISPLAY_MODE_TP, DISPLAY_MODE_CP);		
}
//------------------------------------------------------
void formatDisplayValue(uint8_t type, char * buffer) {
		
	*buffer = 0;
	switch (type) {
		case DISPLAY_MODE_FEED:
		sprintf(buffer, "F.%3i    ", stepper.feed());
		break;
		case DISPLAY_MODE_CP: {
			int32_t	p = stepper.currentPosition();
			if (logic.previouslyShownPosition() != p) {
								
				int32_t converted = stepper.toMetric(p, logic.positionScaleValue());											
					
				switch (logic.positionScale()) {
					case Logic::PositionScale1:
						sprintf(buffer, "%8i", (int16_t)(converted));
					break;
					case Logic::PositionScale10:				
						sprintf(buffer, "%7i.%0d", (int16_t)(converted/10), abs((int16_t)(converted % 10)));			
					break;
					case Logic::PositionScale100:
						sprintf(buffer, "%6i.%02d", (int16_t)(converted/100), abs((int16_t)(converted % 100)));
					break;
				}
			
				logic.setPrevioslyShownPosition(p);
			}
		}
		break;
		case DISPLAY_MODE_TP:{		
			sprintf(buffer,"        ");	
		}
		break;
		case DISPLAY_MODE_CLEAR: {
			sprintf(buffer,"        ");
		}
		break;
		case DISPLAY_MODE_MODE: {
			switch (logic.mode()) {
				case Logic::ModeUndef:
					strcpy_P(buffer, PSTR("----    "));
				break;
				case Logic::ModeStep:
					strcpy_P(buffer, PSTR("StEP    "));
				break;				
				case Logic::ModeFeed:
					strcpy_P(buffer, PSTR("FEED    "));
				break;
				case Logic::ModeAuto:
					strcpy_P(buffer, PSTR("AUtO    "));	
				break;
				case Logic::ModeToNeutral:
					strcpy_P(buffer, PSTR("[--]    "));	
				break;
			}
		}
		break;
		case DISPLAY_MODE_STEP_INTERVAL: {
			strcpy_P(buffer, logic.currentStepIntervalPGMString());
		}
		break;
	}
}
//------------------------------------------------------
void updateDisplay() {
	static char buffer1[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	static char buffer2[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	
	static char * bufferPtr = buffer1;
		
	formatDisplayValue(logic.displayMode(), bufferPtr);	
	if (*bufferPtr) {
		if (strcmp(bufferPtr, bufferPtr == buffer1 ? buffer2 : buffer1)) {
			tm1638.setString(0, bufferPtr);
			bufferPtr = (bufferPtr == buffer1) ? buffer2 : buffer1;
		}
	}
}
//------------------------------------------------------
ISR(TIMER1_COMPA_vect) {
	stepper.doStep();
}
//------------------------------------------------------
ISR(TIMER0_COMPA_vect) {
	clock.tick();
}
//------------------------------------------------------

