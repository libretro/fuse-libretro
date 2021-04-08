/* simpleide.c: Simple 8-bit IDE interface routines
   Copyright (c) 2003-2004 Garry Lancaster,
		 2004 Philip Kendall,
		 2008 Fredrick Meunier

   $Id: simpleide.c 4972 2013-05-19 16:46:43Z zubzero $

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
#include "module.h"
#include "periph.h"
#include "settings.h"
#include "simpleide.h"
#include "ui/ui.h"

/* Private function prototypes */

static libspectrum_byte simpleide_read( libspectrum_word port, int *attached );
static void simpleide_write( libspectrum_word port, libspectrum_byte data );

/* Data */

static const periph_port_t simpleide_ports[] = {
  { 0x0010, 0x0000, simpleide_read, simpleide_write },
  { 0, 0, NULL, NULL }
};

static const periph_t simpleide_periph = {
  &settings_current.simpleide_active,
  simpleide_ports,
  1,
  NULL
};

static libspectrum_ide_channel *simpleide_idechn;

static void simpleide_from_snapshot( libspectrum_snap *snap );
static void simpleide_to_snapshot( libspectrum_snap *snap );

static module_info_t simpleide_module_info = {

  simpleide_reset,
  NULL,
  NULL,
  simpleide_from_snapshot,
  simpleide_to_snapshot,

};

/* Housekeeping functions */

int
simpleide_init( void )
{
  int error;

  simpleide_idechn = libspectrum_ide_alloc( LIBSPECTRUM_IDE_DATA8 );

  ui_menu_activate( UI_MENU_ITEM_MEDIA_IDE_SIMPLE8BIT_MASTER_EJECT, 0 );
  ui_menu_activate( UI_MENU_ITEM_MEDIA_IDE_SIMPLE8BIT_SLAVE_EJECT, 0 );

  if( settings_current.simpleide_master_file ) {
    error = libspectrum_ide_insert( simpleide_idechn, LIBSPECTRUM_IDE_MASTER,
				    settings_current.simpleide_master_file );
    if( error ) return error;
    ui_menu_activate( UI_MENU_ITEM_MEDIA_IDE_SIMPLE8BIT_MASTER_EJECT, 1 );
  }

  if( settings_current.simpleide_slave_file ) {
    error = libspectrum_ide_insert( simpleide_idechn, LIBSPECTRUM_IDE_SLAVE,
				    settings_current.simpleide_slave_file );
    if( error ) return error;
    ui_menu_activate( UI_MENU_ITEM_MEDIA_IDE_SIMPLE8BIT_SLAVE_EJECT, 1 );
  }

  module_register( &simpleide_module_info );
  periph_register( PERIPH_TYPE_SIMPLEIDE, &simpleide_periph );

  return 0;
}

int
simpleide_end( void )
{
  return libspectrum_ide_free( simpleide_idechn );
}

void
simpleide_reset( int hard_reset GCC_UNUSED )
{
  libspectrum_ide_reset( simpleide_idechn );
}

int
simpleide_insert( const char *filename, libspectrum_ide_unit unit )
{
  char **setting;
  ui_menu_item item;

  switch( unit ) {

  case LIBSPECTRUM_IDE_MASTER:
    setting = &settings_current.simpleide_master_file;
    item = UI_MENU_ITEM_MEDIA_IDE_SIMPLE8BIT_MASTER_EJECT;
    break;
    
  case LIBSPECTRUM_IDE_SLAVE:
    setting = &settings_current.simpleide_slave_file;
    item = UI_MENU_ITEM_MEDIA_IDE_SIMPLE8BIT_SLAVE_EJECT;
    break;
    
    default: return 1;
  }

  return ide_insert( filename, simpleide_idechn, unit, simpleide_commit,
		     setting, item );
}

int
simpleide_commit( libspectrum_ide_unit unit )
{
  int error;

  error = libspectrum_ide_commit( simpleide_idechn, unit );

  return error;
}

int
simpleide_eject( libspectrum_ide_unit unit )
{
  char **setting;
  ui_menu_item item;

  switch( unit ) {
  case LIBSPECTRUM_IDE_MASTER:
    setting = &settings_current.simpleide_master_file;
    item = UI_MENU_ITEM_MEDIA_IDE_SIMPLE8BIT_MASTER_EJECT;
    break;
    
  case LIBSPECTRUM_IDE_SLAVE:
    setting = &settings_current.simpleide_slave_file;
    item = UI_MENU_ITEM_MEDIA_IDE_SIMPLE8BIT_SLAVE_EJECT;
    break;
    
  default: return 1;
  }

  return ide_eject( simpleide_idechn, unit, simpleide_commit, setting, item );
}

/* Port read/writes */

static libspectrum_byte
simpleide_read( libspectrum_word port, int *attached )
{
  libspectrum_ide_register idereg;
  
  *attached = 1;
  
  idereg = ( ( port >> 8 ) & 0x01 ) | ( ( port >> 11 ) & 0x06 );
  
  return libspectrum_ide_read( simpleide_idechn, idereg ); 
}  

static void
simpleide_write( libspectrum_word port, libspectrum_byte data )
{
  libspectrum_ide_register idereg;
  
  idereg = ( ( port >> 8 ) & 0x01 ) | ( ( port >> 11 ) & 0x06 );
  
  libspectrum_ide_write( simpleide_idechn, idereg, data ); 
}

static void
simpleide_from_snapshot( libspectrum_snap *snap )
{
  settings_current.simpleide_active =
    libspectrum_snap_simpleide_active( snap );
}

static void
simpleide_to_snapshot( libspectrum_snap *snap )
{
  if( !settings_current.simpleide_active ) return;

  libspectrum_snap_set_simpleide_active( snap, 1 );
}
