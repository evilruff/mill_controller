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

#include "utils.h"

unsigned long sqrt(unsigned long x) {
	register unsigned long xr;  // result register
	register unsigned long q2;  // scan-bit register
	register unsigned char f;   // flag (one bit)

	xr = 0;                     // clear result
	q2 = 0x40000000L;           // higest possible result bit
	do
	{
		if((xr + q2) <= x)
		{
			x -= xr + q2;
			f = 1;                  // set flag
		}
		else{
			f = 0;                  // clear flag
		}
		xr >>= 1;
		if(f){
			xr += q2;               // test flag
		}
	} while(q2 >>= 2);          // shift twice
	if(xr < x){
		return xr +1;             // add for rounding
	}
	else{
		return xr;
	}
}


void	emptyHandler() {};
