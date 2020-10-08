/*
 * utils.h
 *
 * Created: 10/5/2020 12:53:16 PM
 *  Author: evilruff
 */ 


#ifndef UTILS_H_
#define UTILS_H_

unsigned long sqrt(unsigned long x);
#define MIN( a, b ) ((a < b) ? a : b)
#define MAX( a, b ) ((a > b) ? a : b)

//Definitions
#define Input		0
#define Output		!Input
#define Low			0
#define High		!Low
#define False		0
#define True		!False

#define LSBFIRST	0
#define MSBFIRST	1
//------------------

#endif /* UTILS_H_ */