/*
 * logger.h
 *
 * Created: 10/7/2020 5:31:10 PM
 *  Author: evilruff
 */ 


#ifndef LOGGER_H_
#define LOGGER_H_

#include <string.h>
// #define	NO_LOGGING

class Logger;

static	Logger * globalLogger = NULL;

#ifdef NO_LOGGING
	#define LOG(format, ...)	
#else
	#define LOG(format, ...) if (globalLogger) { globalLogger->log(format , ##__VA_ARGS__ ); }
#endif	

typedef void (*dumpString)(const char *);

class Logger {
	public:
		Logger( dumpString logFunc ) {
			dmp = logFunc;
			globalLogger = this;
		}
		
		void	log(const char * format, ...) {
			   va_list argptr;
			   va_start(argptr, format);
			   vsprintf(temp, format, argptr);
			   va_end(argptr);
			   if (dmp) {
				   strcat(temp, "\r\n");
				   dmp(temp);
			   }
		}
				
	protected:
	
		char		temp[128];
		dumpString	dmp;	
};


#endif /* LOGGER_H_ */