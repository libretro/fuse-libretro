/* sp0256.h: SP0256 speech synthesis routines
   Copyright (c) 1998-2000 Joseph Zbiciak
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

#ifndef FUSE_SP0256_H
#define FUSE_SP0256_H

#include "libspectrum.h"

int sp0256_init( libspectrum_byte *sp0256rom );
int sp0256_reset( libspectrum_byte *sp0256rom );
void sp0256_end( void );

void sp0256_play( int allophone );
void sp0256_set_clock( libspectrum_dword clock );
void sp0256_change_clock( libspectrum_dword clock );
int sp0256_busy( void );
void sp0256_do_frame( void );

#endif				/* #ifndef FUSE_SP0256_H */
