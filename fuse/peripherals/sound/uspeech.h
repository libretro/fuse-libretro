/* uspeech.h: Routines for handling the Currah uSpeech interface
   Copyright (c) 2007-2023 Stuart Brady

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

   Philip: philip-fuse@shadowmagic.org.uk

   Stuart: stuart.brady@gmail.com

*/

#ifndef FUSE_USPEECH_H
#define FUSE_USPEECH_H

#include "libspectrum.h"

extern int uspeech_active;
extern int uspeech_available;

void uspeech_register_startup( void );

void uspeech_page( void );
void uspeech_unpage( void );
void uspeech_toggle( void );

libspectrum_byte uspeech_busy( void );

void uspeech_write( libspectrum_word address, libspectrum_byte b );

int uspeech_unittest( void );

#endif				/* #ifndef FUSE_USPEECH_H */
