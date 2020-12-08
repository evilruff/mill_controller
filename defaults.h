/*
 * defaults.h
 *
 * Created: 10/6/2020 12:50:04 PM
 *  Author: evilruff
 */ 


#ifndef DEFAULTS_H_
#define DEFAULTS_H_

#define BUTTON_LONG_PRESS_DELAY		1000
#define MODE_SWITCH_DELAY			2000
#define FEED_MAX_STEPS				1000000000

typedef struct {
	uint8_t		stepWidth;
	uint8_t		dirDelay;
	uint16_t	stepsPerTurn;	
	uint8_t		feedPerTurn;	
	uint16_t	minAccelFrequency;
	uint16_t	maxFrequency;
} FirmwareSettings;

FirmwareSettings defaultSettings = {
	.stepWidth    = 10,
	.dirDelay     = 8,
	.stepsPerTurn = 800,
	.feedPerTurn  = 2,
	.minAccelFrequency = 200,
	.maxFrequency = 20000
	
};

#endif /* DEFAULTS_H_ */