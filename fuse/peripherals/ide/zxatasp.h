/* zxatasp.h: ZXATASP interface routines
   Copyright (c) 2003-2004 Garry Lancaster,
		 2004 Philip Kendall

   $Id: zxatasp.h 4972 2013-05-19 16:46:43Z zubzero $

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

   E-mail: Philip Kendall <philip-fuse@shadowmagic.org.uk>

*/

#ifndef FUSE_ZXATASP_H
#define FUSE_ZXATASP_H

#include <libspectrum.h>

int zxatasp_init( void );
int zxatasp_end( void );
int zxatasp_insert( const char *filename, libspectrum_ide_unit unit );
int zxatasp_commit( libspectrum_ide_unit unit );
int zxatasp_eject( libspectrum_ide_unit unit );

int zxatasp_unittest( void );

#endif			/* #ifndef FUSE_ZXATASP_H */
