/* ide.h: Generic routines shared between the various IDE devices
   Copyright (c) 2005 Philip Kendall

   $Id: ide.h 4972 2013-05-19 16:46:43Z zubzero $

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

#ifndef FUSE_IDE_H
#define FUSE_IDE_H

#include "ui/ui.h"

int
ide_insert( const char *filename, libspectrum_ide_channel *chn,
	    libspectrum_ide_unit unit,
	    int (*commit_fn)( libspectrum_ide_unit unit ), char **setting,
	    ui_menu_item item );

int
ide_eject( libspectrum_ide_channel *chn, libspectrum_ide_unit unit,
	   int (*commit_fn)( libspectrum_ide_unit unit ), char **setting,
	   ui_menu_item item );

#endif			/* #ifndef FUSE_IDE_H */
