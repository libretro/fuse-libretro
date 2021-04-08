/* timer.h: Speed routines for Fuse
   Copyright (c) 1999-2008 Philip Kendall

   $Id: timer.h 3934 2009-01-06 13:01:37Z pak21 $

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

   Author contact information:

   E-mail: philip-fuse@shadowmagic.org.uk

*/

#ifndef FUSE_TIMER_H
#define FUSE_TIMER_H

#include <libspectrum.h>

int timer_estimate_reset( void );
int timer_estimate_speed( void );

int timer_init(void);
void timer_end( void );

extern float current_speed;
extern int timer_event;

/* Internal routines */

double timer_get_time( void );
void timer_sleep( int ms );

#endif			/* #ifndef FUSE_TIMER_H */
