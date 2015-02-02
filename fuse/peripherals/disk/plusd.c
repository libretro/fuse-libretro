/* plusd.c: Routines for handling the +D interface
   Copyright (c) 1999-2011 Stuart Brady, Fredrick Meunier, Philip Kendall,
   Dmitry Sanarin, Darren Salt

   $Id: plusd.c 4906 2013-03-10 11:21:37Z fredm $

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

#include <config.h>

#include <libspectrum.h>

#include <string.h>

#include "compat.h"
#include "machine.h"
#include "module.h"
#include "peripherals/printer.h"
#include "plusd.h"
#include "settings.h"
#include "ui/ui.h"
#include "unittests/unittests.h"
#include "utils.h"
#include "wd_fdc.h"
#include "options.h"	/* needed for get combo options */

#define DISK_TRY_MERGE(heads) ( option_enumerate_diskoptions_disk_try_merge() == 2 || \
				( option_enumerate_diskoptions_disk_try_merge() == 1 && heads == 1 ) )

/* Two 8KB memory chunks accessible by the Z80 when /ROMCS is low */
static memory_page plusd_memory_map_romcs_rom[ MEMORY_PAGES_IN_8K ];
static memory_page plusd_memory_map_romcs_ram[ MEMORY_PAGES_IN_8K ];
static int plusd_memory_source;

int plusd_available = 0;
int plusd_active = 0;

static int plusd_index_pulse;

static int index_event;

#define PLUSD_NUM_DRIVES 2

static wd_fdc *plusd_fdc;
static wd_fdc_drive plusd_drives[ PLUSD_NUM_DRIVES ];

static libspectrum_byte *plusd_ram;
static int memory_allocated = 0;

static void plusd_reset( int hard_reset );
static void plusd_memory_map( void );
static void plusd_enabled_snapshot( libspectrum_snap *snap );
static void plusd_from_snapshot( libspectrum_snap *snap );
static void plusd_to_snapshot( libspectrum_snap *snap );
static void plusd_event_index( libspectrum_dword last_tstates, int type,
			       void *user_data );
static void plusd_activate( void );

static module_info_t plusd_module_info = {

  plusd_reset,
  plusd_memory_map,
  plusd_enabled_snapshot,
  plusd_from_snapshot,
  plusd_to_snapshot,

};

static libspectrum_byte plusd_control_register;

void
plusd_page( void )
{
  plusd_active = 1;
  machine_current->ram.romcs = 1;
  machine_current->memory_map();
}

void
plusd_unpage( void )
{
  plusd_active = 0;
  machine_current->ram.romcs = 0;
  machine_current->memory_map();
}

static void
plusd_memory_map( void )
{
  if( !plusd_active ) return;

  memory_map_romcs_8k( 0x0000, plusd_memory_map_romcs_rom );
  memory_map_romcs_8k( 0x2000, plusd_memory_map_romcs_ram );
}

static const periph_port_t plusd_ports[] = {
  /* ---- ---- 1110 0011 */
  { 0x00ff, 0x00e3, plusd_sr_read, plusd_cr_write },
  /* ---- ---- 1110 1011 */
  { 0x00ff, 0x00eb, plusd_tr_read, plusd_tr_write },
  /* ---- ---- 1111 0011 */
  { 0x00ff, 0x00f3, plusd_sec_read, plusd_sec_write },
  /* ---- ---- 1111 1011 */
  { 0x00ff, 0x00fb, plusd_dr_read, plusd_dr_write },

  /* ---- ---- 1110 1111 */
  { 0x00ff, 0x00ef, NULL, plusd_cn_write },
  /* ---- ---- 1110 0111 */
  { 0x00ff, 0x00e7, plusd_patch_read, plusd_patch_write },
  /* 0000 0000 1111 0111 */
  { 0x00ff, 0x00f7, plusd_printer_read, plusd_printer_write },

  { 0, 0, NULL, NULL }
};

static const periph_t plusd_periph = {
  &settings_current.plusd,
  plusd_ports,
  1,
  plusd_activate
};

void
plusd_init( void )
{
  int i;
  wd_fdc_drive *d;

  plusd_fdc = wd_fdc_alloc_fdc( WD1770, 0, WD_FLAG_NONE );

  for( i = 0; i < PLUSD_NUM_DRIVES; i++ ) {
    d = &plusd_drives[ i ];
    fdd_init( &d->fdd, FDD_SHUGART, NULL, 0 );
    d->disk.flag = DISK_FLAG_NONE;
  }

  plusd_fdc->current_drive = &plusd_drives[ 0 ];
  fdd_select( &plusd_drives[ 0 ].fdd, 1 );
  plusd_fdc->dden = 1;
  plusd_fdc->set_intrq = NULL;
  plusd_fdc->reset_intrq = NULL;
  plusd_fdc->set_datarq = NULL;
  plusd_fdc->reset_datarq = NULL;
  plusd_fdc->iface = NULL;

  index_event = event_register( plusd_event_index, "+D index" );

  module_register( &plusd_module_info );

  plusd_memory_source = memory_source_register( "PlusD" );
  for( i = 0; i < MEMORY_PAGES_IN_8K; i++ )
    plusd_memory_map_romcs_rom[ i ].source = plusd_memory_source;
  for( i = 0; i < MEMORY_PAGES_IN_8K; i++ )
    plusd_memory_map_romcs_ram[ i ].source = plusd_memory_source;

  periph_register( PERIPH_TYPE_PLUSD, &plusd_periph );
}

static void
plusd_reset( int hard_reset )
{
  int i;
  wd_fdc_drive *d;
  const fdd_params_t *dt;

  plusd_active = 0;
  plusd_available = 0;

  event_remove_type( index_event );

  if( !periph_is_active( PERIPH_TYPE_PLUSD ) ) {
    return;
  }

  if( machine_load_rom_bank( plusd_memory_map_romcs_rom, 0,
			     settings_current.rom_plusd,
			     settings_default.rom_plusd, 0x2000 ) ) {
    settings_current.plusd = 0;
    periph_activate_type( PERIPH_TYPE_PLUSD, 0 );
    return;
  }

  machine_current->ram.romcs = 0;

  for( i = 0; i < MEMORY_PAGES_IN_8K; i++ ) {
    plusd_memory_map_romcs_ram[ i ].page = &plusd_ram[ i * MEMORY_PAGE_SIZE ];
    plusd_memory_map_romcs_ram[ i ].writable = 1;
  }

  plusd_available = 1;
  plusd_active = 1;
  plusd_index_pulse = 0;

  if( hard_reset )
    memset( plusd_ram, 0, 0x2000 );

  wd_fdc_master_reset( plusd_fdc );

  for( i = 0; i < PLUSD_NUM_DRIVES; i++ ) {
    d = &plusd_drives[ i ];

    d->index_pulse = 0;
    d->index_interrupt = 0;
  }

  /* We can eject disks only if they are currently present */
  dt = &fdd_params[ option_enumerate_diskoptions_drive_plusd1_type() + 1 ];
  fdd_init( &plusd_drives[ PLUSD_DRIVE_1 ].fdd, FDD_SHUGART, dt, 1 );
  ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_PLUSD_1, dt->enabled );
  ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_PLUSD_1_EJECT,
		    plusd_drives[ PLUSD_DRIVE_1 ].fdd.loaded );
  ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_PLUSD_1_FLIP_SET,
		    !plusd_drives[ PLUSD_DRIVE_1 ].fdd.upsidedown );
  ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_PLUSD_1_WP_SET,
		    !plusd_drives[ PLUSD_DRIVE_1 ].fdd.wrprot );


  dt = &fdd_params[ option_enumerate_diskoptions_drive_plusd2_type() ];
  fdd_init( &plusd_drives[ PLUSD_DRIVE_2 ].fdd, dt->enabled ? FDD_SHUGART : FDD_TYPE_NONE, dt, 1 );
  ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_PLUSD_2, dt->enabled );
  ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_PLUSD_2_EJECT,
		    plusd_drives[ PLUSD_DRIVE_2 ].fdd.loaded );
  ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_PLUSD_2_FLIP_SET,
		    !plusd_drives[ PLUSD_DRIVE_2 ].fdd.upsidedown );
  ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_PLUSD_2_WP_SET,
		    !plusd_drives[ PLUSD_DRIVE_2 ].fdd.wrprot );


  plusd_fdc->current_drive = &plusd_drives[ 0 ];
  fdd_select( &plusd_drives[ 0 ].fdd, 1 );
  machine_current->memory_map();
  plusd_event_index( 0, index_event, NULL );

  ui_statusbar_update( UI_STATUSBAR_ITEM_DISK, UI_STATUSBAR_STATE_INACTIVE );
}

void
plusd_end( void )
{
  plusd_available = 0;
  free( plusd_fdc );
}

libspectrum_byte
plusd_sr_read( libspectrum_word port GCC_UNUSED, int *attached )
{
  *attached = 1;
  return wd_fdc_sr_read( plusd_fdc );
}

void
plusd_cr_write( libspectrum_word port GCC_UNUSED, libspectrum_byte b )
{
  wd_fdc_cr_write( plusd_fdc, b );
}

libspectrum_byte
plusd_tr_read( libspectrum_word port GCC_UNUSED, int *attached )
{
  *attached = 1;
  return wd_fdc_tr_read( plusd_fdc );
}

void
plusd_tr_write( libspectrum_word port GCC_UNUSED, libspectrum_byte b )
{
  wd_fdc_tr_write( plusd_fdc, b );
}

libspectrum_byte
plusd_sec_read( libspectrum_word port GCC_UNUSED, int *attached )
{
  *attached = 1;
  return wd_fdc_sec_read( plusd_fdc );
}

void
plusd_sec_write( libspectrum_word port GCC_UNUSED, libspectrum_byte b )
{
  wd_fdc_sec_write( plusd_fdc, b );
}

libspectrum_byte
plusd_dr_read( libspectrum_word port GCC_UNUSED, int *attached )
{
  *attached = 1;
  return wd_fdc_dr_read( plusd_fdc );
}

void
plusd_dr_write( libspectrum_word port GCC_UNUSED, libspectrum_byte b )
{
  wd_fdc_dr_write( plusd_fdc, b );
}

void
plusd_cn_write( libspectrum_word port GCC_UNUSED, libspectrum_byte b )
{
  int drive, side;
  int i;

  plusd_control_register = b;

  drive = ( b & 0x03 ) == 2 ? 1 : 0;
  side = ( b & 0x80 ) ? 1 : 0;

  /* TODO: set current_drive to NULL when bits 0 and 1 of b are '00' or '11' */
  for( i = 0; i < PLUSD_NUM_DRIVES; i++ ) {
    fdd_set_head( &plusd_drives[ i ].fdd, side );
  }
  fdd_select( &plusd_drives[ (!drive) ].fdd, 0 );
  fdd_select( &plusd_drives[ drive ].fdd, 1 );

  if( plusd_fdc->current_drive != &plusd_drives[ drive ] ) {
    if( plusd_fdc->current_drive->fdd.motoron ) {            /* swap motoron */
      fdd_motoron( &plusd_drives[ (!drive) ].fdd, 0 );
      fdd_motoron( &plusd_drives[ drive ].fdd, 1 );
    }
    plusd_fdc->current_drive = &plusd_drives[ drive ];
  }

  printer_parallel_strobe_write( b & 0x40 );
}

libspectrum_byte
plusd_patch_read( libspectrum_word port GCC_UNUSED, int *attached GCC_UNUSED )
{
  /* should we set *attached = 1? */

  plusd_page();
  return 0;
}

void
plusd_patch_write( libspectrum_word port GCC_UNUSED, libspectrum_byte b GCC_UNUSED )
{
  plusd_unpage();
}

libspectrum_byte
plusd_printer_read( libspectrum_word port GCC_UNUSED, int *attached )
{
  *attached = 1;

  /* bit 7 = busy. other bits high? */

  if(!settings_current.printer)
    return(0xff); /* no printer attached */

  return(0x7f);   /* never busy */
}

void
plusd_printer_write( libspectrum_word port, libspectrum_byte b )
{
  printer_parallel_write( port, b );
}

int
plusd_disk_insert( plusd_drive_number which, const char *filename,
		   int autoload )
{
  int error;
  wd_fdc_drive *d;
  const fdd_params_t *dt;

  if( which >= PLUSD_NUM_DRIVES ) {
    ui_error( UI_ERROR_ERROR, "plusd_disk_insert: unknown drive %d",
	      which );
    fuse_abort();
  }

  d = &plusd_drives[ which ];

  /* Eject any disk already in the drive */
  if( d->fdd.loaded ) {
    /* Abort the insert if we want to keep the current disk */
    if( plusd_disk_eject( which ) ) return 0;
  }

  if( filename ) {
    error = disk_open( &d->disk, filename, 0, DISK_TRY_MERGE( d->fdd.fdd_heads ) );
    if( error != DISK_OK ) {
      ui_error( UI_ERROR_ERROR, "Failed to open disk image: %s",
				disk_strerror( error ) );
      return 1;
    }
  } else {
    switch( which ) {
    case 0:
      dt = &fdd_params[ option_enumerate_diskoptions_drive_plusd1_type() + 1 ];	/* +1 => there is no `Disabled' */
      break;
    case 1:
    default:
      dt = &fdd_params[ option_enumerate_diskoptions_drive_plusd2_type() ];
      break;
    }
    error = disk_new( &d->disk, dt->heads, dt->cylinders, DISK_DENS_AUTO, DISK_UDI );
    if( error != DISK_OK ) {
      ui_error( UI_ERROR_ERROR, "Failed to create disk image: %s",
				disk_strerror( error ) );
      return 1;
    }
  }

  fdd_load( &d->fdd, &d->disk, 0 );

  /* Set the 'eject' item active */
  switch( which ) {
  case PLUSD_DRIVE_1:
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_PLUSD_1_EJECT, 1 );
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_PLUSD_1_FLIP_SET,
		      !plusd_drives[ PLUSD_DRIVE_1 ].fdd.upsidedown );
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_PLUSD_1_WP_SET,
		      !plusd_drives[ PLUSD_DRIVE_1 ].fdd.wrprot );
    break;
  case PLUSD_DRIVE_2:
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_PLUSD_2_EJECT, 1 );
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_PLUSD_2_FLIP_SET,
		      !plusd_drives[ PLUSD_DRIVE_2 ].fdd.upsidedown );
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_PLUSD_2_WP_SET,
		      !plusd_drives[ PLUSD_DRIVE_2 ].fdd.wrprot );
    break;
  }

  if( filename && autoload ) {
    /* XXX */
  }

  return 0;
}

int
plusd_disk_eject( plusd_drive_number which )
{
  wd_fdc_drive *d;

  if( which >= PLUSD_NUM_DRIVES )
    return 1;

  d = &plusd_drives[ which ];

  if( d->disk.type == DISK_TYPE_NONE )
    return 0;

  if( d->disk.dirty ) {

    ui_confirm_save_t confirm = ui_confirm_save(
      "Disk in +D drive %c has been modified.\n"
      "Do you want to save it?",
      which == PLUSD_DRIVE_1 ? '1' : '2'
    );

    switch( confirm ) {

    case UI_CONFIRM_SAVE_SAVE:
      if( plusd_disk_save( which, 0 ) ) return 1;	/* first save */
      break;

    case UI_CONFIRM_SAVE_DONTSAVE: break;
    case UI_CONFIRM_SAVE_CANCEL: return 1;

    }
  }

  fdd_unload( &d->fdd );
  disk_close( &d->disk );

  /* Set the 'eject' item inactive */
  switch( which ) {
  case PLUSD_DRIVE_1:
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_PLUSD_1_EJECT, 0 );
    break;
  case PLUSD_DRIVE_2:
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_PLUSD_2_EJECT, 0 );
    break;
  }
  return 0;
}

int
plusd_disk_save( plusd_drive_number which, int saveas )
{
  wd_fdc_drive *d;

  if( which >= PLUSD_NUM_DRIVES )
    return 1;

  d = &plusd_drives[ which ];

  if( d->disk.type == DISK_TYPE_NONE )
    return 0;

  if( d->disk.filename == NULL ) saveas = 1;
  if( ui_plusd_disk_write( which, saveas ) ) return 1;
  d->disk.dirty = 0;
  return 0;
}

int
plusd_disk_flip( plusd_drive_number which, int flip )
{
  wd_fdc_drive *d;

  if( which >= PLUSD_NUM_DRIVES )
    return 1;

  d = &plusd_drives[ which ];

  if( !d->fdd.loaded )
    return 1;

  fdd_flip( &d->fdd, flip );

  /* Update the 'write flip' menu item */
  switch( which ) {
  case PLUSD_DRIVE_1:
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_PLUSD_1_FLIP_SET,
		      !plusd_drives[ PLUSD_DRIVE_1 ].fdd.upsidedown );
    break;
  case PLUSD_DRIVE_2:
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_PLUSD_2_FLIP_SET,
		      !plusd_drives[ PLUSD_DRIVE_2 ].fdd.upsidedown );
    break;
  }
  return 0;
}

int
plusd_disk_writeprotect( plusd_drive_number which, int wrprot )
{
  wd_fdc_drive *d;

  if( which >= PLUSD_NUM_DRIVES )
    return 1;

  d = &plusd_drives[ which ];

  if( !d->fdd.loaded )
    return 1;

  fdd_wrprot( &d->fdd, wrprot );

  /* Update the 'write protect' menu item */
  switch( which ) {
  case PLUSD_DRIVE_1:
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_PLUSD_1_WP_SET,
		      !plusd_drives[ PLUSD_DRIVE_1 ].fdd.wrprot );
    break;
  case PLUSD_DRIVE_2:
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_PLUSD_2_WP_SET,
		      !plusd_drives[ PLUSD_DRIVE_2 ].fdd.wrprot );
    break;
  }
  return 0;
}

/***TODO most part of the next routine could be move to a common place... */
int
plusd_disk_write( plusd_drive_number which, const char *filename )
{
  wd_fdc_drive *d = &plusd_drives[ which ];
  int error;

  d->disk.type = DISK_TYPE_NONE;
  if( filename == NULL ) filename = d->disk.filename;	/* write over original file */
  error = disk_write( &d->disk, filename );

  if( error != DISK_OK ) {
    ui_error( UI_ERROR_ERROR, "couldn't write '%s' file: %s", filename,
	      disk_strerror( error ) );
    return 1;
  }
  if( d->disk.filename && strcmp( filename, d->disk.filename ) ) {
    free( d->disk.filename );
    d->disk.filename = utils_safe_strdup( filename );
  }

  return 0;
}

fdd_t *
plusd_get_fdd( plusd_drive_number which )
{
  return &( plusd_drives[ which ].fdd );
}

static void
plusd_event_index( libspectrum_dword last_tstates, int type GCC_UNUSED,
                   void *user_data GCC_UNUSED )
{
  int next_tstates;
  int i;

  plusd_index_pulse = !plusd_index_pulse;
  for( i = 0; i < PLUSD_NUM_DRIVES; i++ ) {
    wd_fdc_drive *d = &plusd_drives[ i ];

    d->index_pulse = plusd_index_pulse;
    if( !plusd_index_pulse && d->index_interrupt ) {
      wd_fdc_set_intrq( plusd_fdc );
      d->index_interrupt = 0;
    }
  }
  next_tstates = ( plusd_index_pulse ? 10 : 190 ) *
    machine_current->timings.processor_speed / 1000;
  event_add( last_tstates + next_tstates, index_event );
}

static libspectrum_byte *
alloc_and_copy_page( libspectrum_byte* source_page )
{
  libspectrum_byte *buffer;
  buffer = malloc( MEMORY_PAGE_SIZE );
  if( !buffer ) {
    ui_error( UI_ERROR_ERROR, "Out of memory at %s:%d", __FILE__,
              __LINE__ );
    return 0;
  }

  memcpy( buffer, source_page, MEMORY_PAGE_SIZE );
  return buffer;
}

static void
plusd_enabled_snapshot( libspectrum_snap *snap )
{
  if( libspectrum_snap_plusd_active( snap ) )
    settings_current.plusd = 1;
}

static void
plusd_from_snapshot( libspectrum_snap *snap )
{
  if( !libspectrum_snap_plusd_active( snap ) ) return;

  if( libspectrum_snap_plusd_custom_rom( snap ) &&
      libspectrum_snap_plusd_rom( snap, 0 ) &&
      machine_load_rom_bank_from_buffer(
                             plusd_memory_map_romcs_rom, 0,
                             libspectrum_snap_plusd_rom( snap, 0 ),
                             0x2000, 1 ) )
    return;

  if( libspectrum_snap_plusd_ram( snap, 0 ) ) {
    memcpy( plusd_ram,
            libspectrum_snap_plusd_ram( snap, 0 ), 0x2000 );
  }

  /* ignore drive count for now, there will be an issue with loading snaps where
     drives have been disabled
  libspectrum_snap_plusd_drive_count( snap )
   */

  plusd_fdc->direction = libspectrum_snap_plusd_direction( snap );

  plusd_cr_write ( 0x00e3, libspectrum_snap_plusd_status ( snap ) );
  plusd_tr_write ( 0x00eb, libspectrum_snap_plusd_track  ( snap ) );
  plusd_sec_write( 0x00f3, libspectrum_snap_plusd_sector ( snap ) );
  plusd_dr_write ( 0x00fb, libspectrum_snap_plusd_data   ( snap ) );
  plusd_cn_write ( 0x00ef, libspectrum_snap_plusd_control( snap ) );

  if( libspectrum_snap_plusd_paged( snap ) ) {
    plusd_page();
  } else {
    plusd_unpage();
  }
}

static void
plusd_to_snapshot( libspectrum_snap *snap )
{
  libspectrum_byte *buffer;
  int drive_count = 0;

  if( !periph_is_active( PERIPH_TYPE_PLUSD ) ) return;

  libspectrum_snap_set_plusd_active( snap, 1 );

  buffer = alloc_and_copy_page( plusd_memory_map_romcs_rom[ 0 ].page );
  if( !buffer ) return;
  libspectrum_snap_set_plusd_rom( snap, 0, buffer );
  if( plusd_memory_map_romcs_rom[ 0 ].save_to_snapshot )
    libspectrum_snap_set_plusd_custom_rom( snap, 1 );

  buffer = alloc_and_copy_page( plusd_ram );
  if( !buffer ) return;
  libspectrum_snap_set_plusd_ram( snap, 0, buffer );

  drive_count++; /* Drive 1 is not removable */
  if( option_enumerate_diskoptions_drive_plusd2_type() > 0 ) drive_count++;
  libspectrum_snap_set_plusd_drive_count( snap, drive_count );

  libspectrum_snap_set_plusd_paged ( snap, plusd_active );
  libspectrum_snap_set_plusd_direction( snap, plusd_fdc->direction );
  libspectrum_snap_set_plusd_status( snap, plusd_fdc->status_register );
  libspectrum_snap_set_plusd_track ( snap, plusd_fdc->track_register );
  libspectrum_snap_set_plusd_sector( snap, plusd_fdc->sector_register );
  libspectrum_snap_set_plusd_data  ( snap, plusd_fdc->data_register );
  libspectrum_snap_set_plusd_control( snap, plusd_control_register );
}

static void
plusd_activate( void )
{
  if( !memory_allocated ) {
    plusd_ram = memory_pool_allocate_persistent( 0x2000, 1 );
    memory_allocated = 1;
  }
}

int
plusd_unittest( void )
{
  int r = 0;

  plusd_page();

  r += unittests_assert_16k_page( 0x0000, plusd_memory_source, 0 );
  r += unittests_assert_16k_ram_page( 0x4000, 5 );
  r += unittests_assert_16k_ram_page( 0x8000, 2 );
  r += unittests_assert_16k_ram_page( 0xc000, 0 );

  plusd_unpage();

  r += unittests_paging_test_48( 2 );

  return r;
}
