/* ide.c: Generic routines shared between the various IDE devices
   Copyright (c) 2005 Philip Kendall

   $Id: ide.c 4972 2013-05-19 16:46:43Z zubzero $

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

#include <config.h>

#include <libspectrum.h>

#include "ide.h"
#include "ui/ui.h"
#include "settings.h"

int
ide_insert( const char *filename, libspectrum_ide_channel *chn,
	    libspectrum_ide_unit unit,
	    int (*commit_fn)( libspectrum_ide_unit unit ), char **setting,
	    ui_menu_item item )
{
  int error;

  /* Remove any currently inserted disk; abort if we want to keep the current
     disk */
  if( *setting )
    if( ide_eject( chn, unit, commit_fn, setting, item ) )
      return 0;

  settings_set_string( setting, filename );

  error = libspectrum_ide_insert( chn, unit, filename );
  if( error ) return error;

  error = ui_menu_activate( item, 1 ); if( error ) return error;

  return 0;
}

int
ide_eject( libspectrum_ide_channel *chn, libspectrum_ide_unit unit,
	   int (*commit_fn)( libspectrum_ide_unit unit ), char **setting,
	   ui_menu_item item )
{
  int error;

  if( libspectrum_ide_dirty( chn, unit ) ) {
    
    ui_confirm_save_t confirm = ui_confirm_save(
      "Hard disk has been modified.\nDo you want to save it?"
    );
  
    switch( confirm ) {

    case UI_CONFIRM_SAVE_SAVE:
      error = commit_fn( unit ); if( error ) return error;
      break;

    case UI_CONFIRM_SAVE_DONTSAVE: break;
    case UI_CONFIRM_SAVE_CANCEL: return 1;

    }
  }

  free( *setting ); *setting = NULL;
  
  error = libspectrum_ide_eject( chn, unit );
  if( error ) return error;

  error = ui_menu_activate( item, 0 );
  if( error ) return error;

  return 0;
}
