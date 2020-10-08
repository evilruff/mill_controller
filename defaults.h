/*
 * defaults.h
 *
 * Created: 10/6/2020 12:50:04 PM
 *  Author: evilruff
 */ 


#ifndef DEFAULTS_H_
#define DEFAULTS_H_

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