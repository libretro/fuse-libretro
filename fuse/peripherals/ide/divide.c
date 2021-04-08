/* divide.c: DivIDE interface routines
   Copyright (c) 2005-2008 Matthew Westcott, Philip Kendall

   $Id: divide.c 4972 2013-05-19 16:46:43Z zubzero $

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

#include <string.h>

#include "debugger/debugger.h"
#include "ide.h"
#include "machine.h"
#include "module.h"
#include "periph.h"
#include "settings.h"
#include "ui/ui.h"
#include "unittests/unittests.h"
#include "divide.h"

/* Private function prototypes */

static libspectrum_byte divide_ide_read( libspectrum_word port, int *attached );
static void divide_ide_write( libspectrum_word port, libspectrum_byte data );
static void divide_control_write( libspectrum_word port, libspectrum_byte data );
static void divide_control_write_internal( libspectrum_byte data );
static void divide_page( void );
static void divide_unpage( void );
static libspectrum_ide_register port_to_ide_register( libspectrum_byte port );
static void divide_activate( void );

/* Data */

static const periph_port_t divide_ports[] = {
  { 0x00e3, 0x00a3, divide_ide_read, divide_ide_write },
  { 0x00ff, 0x00e3, NULL, divide_control_write },
  { 0, 0, NULL, NULL }
};

static const periph_t divide_periph = {
  &settings_current.divide_enabled,
  divide_ports,
  1,
  divide_activate
};

static const libspectrum_byte DIVIDE_CONTROL_CONMEM = 0x80;
static const libspectrum_byte DIVIDE_CONTROL_MAPRAM = 0x40;

int divide_automapping_enabled = 0;
int divide_active = 0;
static libspectrum_byte divide_control;

/* divide_automap tracks opcode fetches to entry and exit points to determine
   whether DivIDE memory *would* be paged in at this moment if mapram / wp
   flags allowed it */
static int divide_automap = 0;

static libspectrum_ide_channel *divide_idechn0;
static libspectrum_ide_channel *divide_idechn1;

#define DIVIDE_PAGES 4
#define DIVIDE_PAGE_LENGTH 0x2000
static libspectrum_byte *divide_ram[ DIVIDE_PAGES ];
static libspectrum_byte *divide_eprom;
static memory_page divide_memory_map_eprom[ MEMORY_PAGES_IN_8K ];
static memory_page divide_memory_map_ram[ DIVIDE_PAGES ][ MEMORY_PAGES_IN_8K ];
static int memory_allocated = 0;
static int divide_memory_source_eprom;
static int divide_memory_source_ram;

static void divide_reset( int hard_reset );
static void divide_memory_map( void );
static void divide_enabled_snapshot( libspectrum_snap *snap );
static void divide_from_snapshot( libspectrum_snap *snap );
static void divide_to_snapshot( libspectrum_snap *snap );

static module_info_t divide_module_info = {

  divide_reset,
  divide_memory_map,
  divide_enabled_snapshot,
  divide_from_snapshot,
  divide_to_snapshot,

};

/* Debugger events */
static const char *event_type_string = "divide";
static int page_event, unpage_event;

/* Housekeeping functions */

int
divide_init( void )
{
  int error, i, j;

  divide_idechn0 = libspectrum_ide_alloc( LIBSPECTRUM_IDE_DATA16 );
  divide_idechn1 = libspectrum_ide_alloc( LIBSPECTRUM_IDE_DATA16 );
  
  ui_menu_activate( UI_MENU_ITEM_MEDIA_IDE_DIVIDE_MASTER_EJECT, 0 );
  ui_menu_activate( UI_MENU_ITEM_MEDIA_IDE_DIVIDE_SLAVE_EJECT, 0 );

  if( settings_current.divide_master_file ) {
    error = libspectrum_ide_insert( divide_idechn0, LIBSPECTRUM_IDE_MASTER,
				    settings_current.divide_master_file );
    if( error ) return error;
    ui_menu_activate( UI_MENU_ITEM_MEDIA_IDE_DIVIDE_MASTER_EJECT, 1 );
  }

  if( settings_current.divide_slave_file ) {
    error = libspectrum_ide_insert( divide_idechn0, LIBSPECTRUM_IDE_SLAVE,
				    settings_current.divide_slave_file );
    if( error ) return error;
    ui_menu_activate( UI_MENU_ITEM_MEDIA_IDE_DIVIDE_SLAVE_EJECT, 1 );
  }

  module_register( &divide_module_info );

  divide_memory_source_eprom = memory_source_register( "DivIDE EPROM" );
  divide_memory_source_ram = memory_source_register( "DivIDE RAM" );

  for( i = 0; i < MEMORY_PAGES_IN_8K; i++ ) {
    memory_page *page = &divide_memory_map_eprom[i];
    page->source = divide_memory_source_eprom;
    page->page_num = 0;
  }

  for( i = 0; i < DIVIDE_PAGES; i++ ) {
    for( j = 0; j < MEMORY_PAGES_IN_8K; j++ ) {
      memory_page *page = &divide_memory_map_ram[i][j];
      page->source = divide_memory_source_ram;
      page->page_num = i;
    }
  }

  periph_register( PERIPH_TYPE_DIVIDE, &divide_periph );
  periph_register_paging_events( event_type_string, &page_event,
                                 &unpage_event );

  return 0;
}

int
divide_end( void )
{
  int error;
  
  error = libspectrum_ide_free( divide_idechn0 );
  error = libspectrum_ide_free( divide_idechn1 ) || error;

  return error;
}

/* DivIDE does not page in immediately on a reset condition (we do that by
   trapping PC instead); however, it needs to perform housekeeping tasks upon
   reset */
static void
divide_reset( int hard_reset )
{
  divide_active = 0;

  if( !settings_current.divide_enabled ) return;

  if( hard_reset ) {
    divide_control = 0;
  } else {
    divide_control &= DIVIDE_CONTROL_MAPRAM;
  }
  divide_automap = 0;
  divide_refresh_page_state();

  libspectrum_ide_reset( divide_idechn0 );
  libspectrum_ide_reset( divide_idechn1 );
}

int
divide_insert( const char *filename, libspectrum_ide_unit unit )
{
  char **setting;
  ui_menu_item item;

  switch( unit ) {
  case LIBSPECTRUM_IDE_MASTER:
    setting = &settings_current.divide_master_file;
    item = UI_MENU_ITEM_MEDIA_IDE_DIVIDE_MASTER_EJECT;
    break;
    
  case LIBSPECTRUM_IDE_SLAVE:
    setting = &settings_current.divide_slave_file;
    item = UI_MENU_ITEM_MEDIA_IDE_DIVIDE_SLAVE_EJECT;
    break;
    
  default: return 1;
  }

  return ide_insert( filename, divide_idechn0, unit, divide_commit, setting,
		     item );
}

int
divide_commit( libspectrum_ide_unit unit )
{
  int error;

  error = libspectrum_ide_commit( divide_idechn0, unit );

  return error;
}

int
divide_eject( libspectrum_ide_unit unit )
{
  char **setting;
  ui_menu_item item;

  switch( unit ) {
  case LIBSPECTRUM_IDE_MASTER:
    setting = &settings_current.divide_master_file;
    item = UI_MENU_ITEM_MEDIA_IDE_DIVIDE_MASTER_EJECT;
    break;

  case LIBSPECTRUM_IDE_SLAVE:
    setting = &settings_current.divide_slave_file;
    item = UI_MENU_ITEM_MEDIA_IDE_DIVIDE_SLAVE_EJECT;
    break;
    
  default: return 1;
  }

  return ide_eject( divide_idechn0, unit, divide_commit, setting, item );
}

/* Port read/writes */

static libspectrum_ide_register
port_to_ide_register( libspectrum_byte port )
{
  switch( port & 0xff ) {
    case 0xa3:
      return LIBSPECTRUM_IDE_REGISTER_DATA;
    case 0xa7:
      return LIBSPECTRUM_IDE_REGISTER_ERROR_FEATURE;
    case 0xab:
      return LIBSPECTRUM_IDE_REGISTER_SECTOR_COUNT;
    case 0xaf:
      return LIBSPECTRUM_IDE_REGISTER_SECTOR;
    case 0xb3:
      return LIBSPECTRUM_IDE_REGISTER_CYLINDER_LOW;
    case 0xb7:
      return LIBSPECTRUM_IDE_REGISTER_CYLINDER_HIGH;
    case 0xbb:
      return LIBSPECTRUM_IDE_REGISTER_HEAD_DRIVE;
    default: /* 0xbf */
      return LIBSPECTRUM_IDE_REGISTER_COMMAND_STATUS;
  }
}

libspectrum_byte
divide_ide_read( libspectrum_word port, int *attached )
{
  int ide_register;

  *attached = 1;
  ide_register = port_to_ide_register( port );

  return libspectrum_ide_read( divide_idechn0, ide_register );
}

static void
divide_ide_write( libspectrum_word port, libspectrum_byte data )
{
  int ide_register;
  
  ide_register = port_to_ide_register( port );
  
  libspectrum_ide_write( divide_idechn0, ide_register, data );
}

static void
divide_control_write( libspectrum_word port GCC_UNUSED, libspectrum_byte data )
{
  int old_mapram;

  /* MAPRAM bit cannot be reset, only set */
  old_mapram = divide_control & DIVIDE_CONTROL_MAPRAM;
  divide_control_write_internal( data | old_mapram );
}

static void
divide_control_write_internal( libspectrum_byte data )
{
  divide_control = data;
  divide_refresh_page_state();
}

void
divide_set_automap( int state )
{
  divide_automap = state;
  divide_refresh_page_state();
}

void
divide_refresh_page_state( void )
{
  if( divide_control & DIVIDE_CONTROL_CONMEM ) {
    /* always paged in if conmem enabled */
    divide_page();
  } else if( settings_current.divide_wp
    || ( divide_control & DIVIDE_CONTROL_MAPRAM ) ) {
    /* automap in effect */
    if( divide_automap ) {
      divide_page();
    } else {
      divide_unpage();
    }
  } else {
    divide_unpage();
  }
}

static void
divide_page( void )
{
  divide_active = 1;
  machine_current->ram.romcs = 1;
  machine_current->memory_map();

  debugger_event( page_event );
}

static void
divide_unpage( void )
{
  divide_active = 0;
  machine_current->ram.romcs = 0;
  machine_current->memory_map();

  debugger_event( unpage_event );
}

void
divide_memory_map( void )
{
  int i;
  int upper_ram_page;
  int lower_page_writable, upper_page_writable;
  memory_page *lower_page, *upper_page;

  if( !divide_active ) return;

  /* low bits of divide_control register give page number to use in upper
     bank; only lowest two bits on original 32K model */
  upper_ram_page = divide_control & (DIVIDE_PAGES - 1);
  
  if( divide_control & DIVIDE_CONTROL_CONMEM ) {
    lower_page = divide_memory_map_eprom;
    lower_page_writable = !settings_current.divide_wp;
    upper_page = divide_memory_map_ram[ upper_ram_page ];
    upper_page_writable = 1;
  } else {
    if( divide_control & DIVIDE_CONTROL_MAPRAM ) {
      lower_page = divide_memory_map_ram[3];
      lower_page_writable = 0;
      upper_page = divide_memory_map_ram[ upper_ram_page ];
      upper_page_writable = ( upper_ram_page != 3 );
    } else {
      lower_page = divide_memory_map_eprom;
      lower_page_writable = 0;
      upper_page = divide_memory_map_ram[ upper_ram_page ];
      upper_page_writable = 1;
    }
  }

  for( i = 0; i < MEMORY_PAGES_IN_8K; i++ ) {
    lower_page[i].writable = lower_page_writable;
    upper_page[i].writable = upper_page_writable;
  }

  memory_map_romcs_8k( 0x0000, lower_page );
  memory_map_romcs_8k( 0x2000, upper_page );
}

static void
divide_enabled_snapshot( libspectrum_snap *snap )
{
  if( libspectrum_snap_divide_active( snap ) )
    settings_current.divide_enabled = 1;
}

static void
divide_from_snapshot( libspectrum_snap *snap )
{
  size_t i;

  if( !libspectrum_snap_divide_active( snap ) ) return;

  settings_current.divide_wp =
    libspectrum_snap_divide_eprom_writeprotect( snap );
  divide_control_write_internal( libspectrum_snap_divide_control( snap ) );

  if( libspectrum_snap_divide_eprom( snap, 0 ) ) {
    memcpy( divide_eprom,
	    libspectrum_snap_divide_eprom( snap, 0 ), DIVIDE_PAGE_LENGTH );
  }

  for( i = 0; i < libspectrum_snap_divide_pages( snap ); i++ )
    if( libspectrum_snap_divide_ram( snap, i ) )
      memcpy( divide_ram[ i ], libspectrum_snap_divide_ram( snap, i ),
	      DIVIDE_PAGE_LENGTH );

  if( libspectrum_snap_divide_paged( snap ) ) {
    divide_page();
  } else {
    divide_unpage();
  }
}

static void
divide_to_snapshot( libspectrum_snap *snap )
{
  size_t i;
  libspectrum_byte *buffer;

  if( !settings_current.divide_enabled ) return;

  libspectrum_snap_set_divide_active( snap, 1 );
  libspectrum_snap_set_divide_eprom_writeprotect( snap,
                                                  settings_current.divide_wp );
  libspectrum_snap_set_divide_paged( snap, divide_active );
  libspectrum_snap_set_divide_control( snap, divide_control );

  buffer = malloc( DIVIDE_PAGE_LENGTH * sizeof( libspectrum_byte ) );
  if( !buffer ) {
    ui_error( UI_ERROR_ERROR, "Out of memory at %s:%d", __FILE__, __LINE__ );
    return;
  }

  memcpy( buffer, divide_eprom, DIVIDE_PAGE_LENGTH );
  libspectrum_snap_set_divide_eprom( snap, 0, buffer );

  libspectrum_snap_set_divide_pages( snap, DIVIDE_PAGES );

  for( i = 0; i < DIVIDE_PAGES; i++ ) {

    buffer = malloc( DIVIDE_PAGE_LENGTH * sizeof( libspectrum_byte ) );
    if( !buffer ) {
      ui_error( UI_ERROR_ERROR, "Out of memory at %s:%d", __FILE__, __LINE__ );
      return;
    }

    memcpy( buffer, divide_ram[ i ], DIVIDE_PAGE_LENGTH );
    libspectrum_snap_set_divide_ram( snap, i, buffer );
  }
}

static void
divide_activate( void )
{
  if( !memory_allocated ) {
    int i, j;
    libspectrum_byte *memory =
      memory_pool_allocate_persistent( DIVIDE_PAGES * DIVIDE_PAGE_LENGTH, 1 );

    for( i = 0; i < DIVIDE_PAGES; i++ ) {
      divide_ram[i] = memory + i * DIVIDE_PAGE_LENGTH;
      for( j = 0; j < MEMORY_PAGES_IN_8K; j++ ) {
        memory_page *page = &divide_memory_map_ram[i][j];
        page->page = divide_ram[i] + j * MEMORY_PAGE_SIZE;
        page->offset = j * MEMORY_PAGE_SIZE;
      }
    }

    divide_eprom = memory_pool_allocate_persistent( DIVIDE_PAGE_LENGTH, 1 );
    for( i = 0; i < MEMORY_PAGES_IN_8K; i++ ) {
      memory_page *page = &divide_memory_map_eprom[i];
      page->page = divide_eprom + i * MEMORY_PAGE_SIZE;
      page->offset = i * MEMORY_PAGE_SIZE;
    }

    memory_allocated = 1;
  }
}

int
divide_unittest( void )
{
  int r = 0;

  divide_set_automap( 1 );

  divide_control_write( 0x00e3, 0x80 );
  r += unittests_assert_8k_page( 0x0000, divide_memory_source_eprom, 0 );
  r += unittests_assert_8k_page( 0x2000, divide_memory_source_ram, 0 );
  r += unittests_assert_16k_ram_page( 0x4000, 5 );
  r += unittests_assert_16k_ram_page( 0x8000, 2 );
  r += unittests_assert_16k_ram_page( 0xc000, 0 );

  divide_control_write( 0x00e3, 0x83 );
  r += unittests_assert_8k_page( 0x0000, divide_memory_source_eprom, 0 );
  r += unittests_assert_8k_page( 0x2000, divide_memory_source_ram, 3 );
  r += unittests_assert_16k_ram_page( 0x4000, 5 );
  r += unittests_assert_16k_ram_page( 0x8000, 2 );
  r += unittests_assert_16k_ram_page( 0xc000, 0 );

  divide_control_write( 0x00e3, 0x40 );
  r += unittests_assert_8k_page( 0x0000, divide_memory_source_ram, 3 );
  r += unittests_assert_8k_page( 0x2000, divide_memory_source_ram, 0 );
  r += unittests_assert_16k_ram_page( 0x4000, 5 );
  r += unittests_assert_16k_ram_page( 0x8000, 2 );
  r += unittests_assert_16k_ram_page( 0xc000, 0 );

  divide_control_write( 0x00e3, 0x02 );
  r += unittests_assert_8k_page( 0x0000, divide_memory_source_ram, 3 );
  r += unittests_assert_8k_page( 0x2000, divide_memory_source_ram, 2 );
  r += unittests_assert_16k_ram_page( 0x4000, 5 );
  r += unittests_assert_16k_ram_page( 0x8000, 2 );
  r += unittests_assert_16k_ram_page( 0xc000, 0 );

  divide_set_automap( 0 );

  r += unittests_paging_test_48( 2 );

  return r;
}
