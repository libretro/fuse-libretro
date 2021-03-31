/* ttx2000s.h: Routines for handling the TTX2000S teletext adapter
   Copyright (c) 2018 Alistair Cree

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

*/

#ifndef FUSE_TTX2000S_H
#define FUSE_TTX2000S_H

extern int ttx2000s_paged;

void ttx2000s_register_startup( void );
void ttx2000s_page( void );
void ttx2000s_unpage( void );
int ttx2000s_unittest( void );
libspectrum_byte ttx2000s_sram_read( libspectrum_word address );
void ttx2000s_sram_write( libspectrum_word address, libspectrum_byte b );

#endif				/* #ifndef FUSE_TTX2000S_H */
