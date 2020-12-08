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

extern void	emptyHandler();

#endif /* UTILS_H_ */