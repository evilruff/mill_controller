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


#ifndef LOGGER_H_
#define LOGGER_H_


#include <string.h>
#include <stdio.h>
// #define	NO_LOGGING

class Logger;

static	Logger * globalLogger = NULL;

#ifdef NO_LOGGING
	#define LOG(format, ...)	
#else
	#define LOG(format, ...) if (globalLogger) { globalLogger->log(format , ##__VA_ARGS__ ); }
#endif	

#ifdef NO_LOGGING
#define LOG_P(format, ...)
#else
#define LOG_P(format, ...) if (globalLogger) { globalLogger->log_p(PSTR(format) , ##__VA_ARGS__ ); }
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
		
		void	log_p(const char * format, ...) {
			va_list argptr;
			va_start(argptr, format);
			vsprintf_P(temp, format, argptr);
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