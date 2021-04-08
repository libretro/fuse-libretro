/* opus.c: Routines for handling the Opus Discovery interface
   Copyright (c) 1999-2011 Stuart Brady, Fredrick Meunier, Philip Kendall,
   Dmitry Sanarin, Darren Salt, Michael D Wynne, Gergely Szasz

   $Id: opus.c 4926 2013-05-05 07:58:18Z sbaldovi $

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
#include "opus.h"
#include "peripherals/printer.h"
#include "settings.h"
#include "ui/ui.h"
#include "unittests/unittests.h"
#include "utils.h"
#include "wd_fdc.h"
#include "options.h"	/* needed for get combo options */
#include "z80/z80.h"

#define DISK_TRY_MERGE(heads) ( option_enumerate_diskoptions_disk_try_merge() == 2 || \
				( option_enumerate_diskoptions_disk_try_merge() == 1 && heads == 1 ) )

/* FIXME: this is wrong. Opus has only 2 Kb of RAM, but we can't handle
   anything less than our page size */
#define OPUS_RAM_SIZE 0x1000

#define OPUS_RAM_PAGES ( OPUS_RAM_SIZE / MEMORY_PAGE_SIZE \
  ? OPUS_RAM_SIZE / MEMORY_PAGE_SIZE : 1 )

#define TRUE_OPUS_RAM_SIZE 0x800

static int opus_rom_memory_source, opus_ram_memory_source;

/* Two memory chunks accessible by the Z80 when /ROMCS is low */
static memory_page opus_memory_map_romcs_rom[ MEMORY_PAGES_IN_8K ];
static memory_page opus_memory_map_romcs_ram[ OPUS_RAM_PAGES ];

int opus_available = 0;
int opus_active = 0;

static int opus_index_pulse;

static int index_event;

#define OPUS_NUM_DRIVES 2

static wd_fdc *opus_fdc;
static wd_fdc_drive opus_drives[ OPUS_NUM_DRIVES ];

static libspectrum_byte opus_ram[ OPUS_RAM_SIZE ];

/* 6821 PIA internal registers */
static libspectrum_byte data_reg_a, data_dir_a, control_a;
static libspectrum_byte data_reg_b, data_dir_b, control_b;

static void opus_reset( int hard_reset );
static void opus_memory_map( void );
static void opus_enabled_snapshot( libspectrum_snap *snap );
static void opus_from_snapshot( libspectrum_snap *snap );
static void opus_to_snapshot( libspectrum_snap *snap );
static void opus_event_index( libspectrum_dword last_tstates, int type,
			       void *user_data );

static module_info_t opus_module_info = {

  opus_reset,
  opus_memory_map,
  opus_enabled_snapshot,
  opus_from_snapshot,
  opus_to_snapshot,

};

static const periph_t opus_periph = {
  &settings_current.opus,
  NULL,
  1,
  NULL
};

void
opus_page( void )
{
  opus_active = 1;
  machine_current->ram.romcs = 1;
  machine_current->memory_map();
}

void
opus_unpage( void )
{
  opus_active = 0;
  machine_current->ram.romcs = 0;
  machine_current->memory_map();
}

static void
opus_memory_map( void )
{
  if( !opus_active ) return;

  memory_map_romcs_8k( 0x0000, opus_memory_map_romcs_rom );
  memory_map_romcs_4k( 0x2000, opus_memory_map_romcs_ram );
}

static void
opus_set_datarq( struct wd_fdc *f )
{
  event_add( 0, z80_nmi_event );
}

void
opus_init( void )
{
  int i;
  wd_fdc_drive *d;

  opus_fdc = wd_fdc_alloc_fdc( WD1770, 0, WD_FLAG_OPUS );

  for( i = 0; i < OPUS_NUM_DRIVES; i++ ) {
    d = &opus_drives[ i ];
    fdd_init( &d->fdd, FDD_SHUGART, NULL, 0 );	/* drive geometry 'autodetect' */
    d->disk.flag = DISK_FLAG_NONE;
  }

  opus_fdc->current_drive = &opus_drives[ 0 ];
  fdd_select( &opus_drives[ 0 ].fdd, 1 );
  opus_fdc->dden = 1;
  opus_fdc->set_intrq = NULL;
  opus_fdc->reset_intrq = NULL;
  opus_fdc->set_datarq = opus_set_datarq;
  opus_fdc->reset_datarq = NULL;
  opus_fdc->iface = NULL;

  index_event = event_register( opus_event_index, "Opus index" );

  module_register( &opus_module_info );

  opus_rom_memory_source = memory_source_register( "Opus ROM" );
  opus_ram_memory_source = memory_source_register( "Opus RAM" );
  for( i = 0; i < MEMORY_PAGES_IN_8K; i++ )
    opus_memory_map_romcs_rom[i].source = opus_rom_memory_source;
  for( i = 0; i < OPUS_RAM_PAGES; i++ )
    opus_memory_map_romcs_ram[i].source = opus_ram_memory_source;

  periph_register( PERIPH_TYPE_OPUS, &opus_periph );
}

static void
opus_reset( int hard_reset )
{
  int i;
  wd_fdc_drive *d;
  const fdd_params_t *dt;

  opus_active = 0;
  opus_available = 0;

  event_remove_type( index_event );

  if( !periph_is_active( PERIPH_TYPE_OPUS ) ) {
    return;
  }

  if( machine_load_rom_bank( opus_memory_map_romcs_rom, 0,
                             settings_current.rom_opus,
                             settings_default.rom_opus, 0x2000 ) ) {
    settings_current.opus = 0;
    periph_activate_type( PERIPH_TYPE_OPUS, 0 );
    return;
  }

  for( i = 0; i < OPUS_RAM_PAGES; i++ ) {
    struct memory_page *page =
      &opus_memory_map_romcs_ram[ i ];
    page->page = opus_ram + i * MEMORY_PAGE_SIZE;
    page->offset = i * MEMORY_PAGE_SIZE;
  }

  machine_current->ram.romcs = 0;

  for( i = 0; i < OPUS_RAM_PAGES; i++ )
    opus_memory_map_romcs_ram[ i ].writable = 1;

  data_reg_a = 0;
  data_dir_a = 0;
  control_a  = 0;
  data_reg_b = 0;
  data_dir_b = 0;
  control_b  = 0;

  opus_available = 1;
  opus_index_pulse = 0;

  if( hard_reset )
    memset( opus_ram, 0, TRUE_OPUS_RAM_SIZE );

  wd_fdc_master_reset( opus_fdc );

  for( i = 0; i < OPUS_NUM_DRIVES; i++ ) {
    d = &opus_drives[ i ];

    d->index_pulse = 0;
    d->index_interrupt = 0;
  }

  /* We can eject disks only if they are currently present */
  dt = &fdd_params[ option_enumerate_diskoptions_drive_opus1_type() + 1 ];	/* +1 => there is no `Disabled' */
  fdd_init( &opus_drives[ OPUS_DRIVE_1 ].fdd, FDD_SHUGART, dt, 1 );
  ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_OPUS_1, dt->enabled );
  ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_OPUS_1_EJECT,
		    opus_drives[ OPUS_DRIVE_1 ].fdd.loaded );
  ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_OPUS_1_FLIP_SET,
		    !opus_drives[ OPUS_DRIVE_1 ].fdd.upsidedown );
  ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_OPUS_1_WP_SET,
		    !opus_drives[ OPUS_DRIVE_1 ].fdd.wrprot );


  dt = &fdd_params[ option_enumerate_diskoptions_drive_opus2_type() ];
  fdd_init( &opus_drives[ OPUS_DRIVE_2 ].fdd, dt->enabled ? FDD_SHUGART : FDD_TYPE_NONE, dt, 1 );
  ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_OPUS_2, dt->enabled );
  ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_OPUS_2_EJECT,
		    opus_drives[ OPUS_DRIVE_2 ].fdd.loaded );
  ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_OPUS_2_FLIP_SET,
		    !opus_drives[ OPUS_DRIVE_2 ].fdd.upsidedown );
  ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_OPUS_2_WP_SET,
		    !opus_drives[ OPUS_DRIVE_2 ].fdd.wrprot );


  opus_fdc->current_drive = &opus_drives[ 0 ];
  fdd_select( &opus_drives[ 0 ].fdd, 1 );
  machine_current->memory_map();
  opus_event_index( 0, index_event, NULL );

  ui_statusbar_update( UI_STATUSBAR_ITEM_DISK, UI_STATUSBAR_STATE_INACTIVE );
}

void
opus_end( void )
{
  opus_available = 0;
  free( opus_fdc );
}

/*
 * opus_6821_access( reg, data, dir )
 *
 * reg - register to access:
 *
 * data - if dir = 1 the value being written else ignored
 *
 * dir - direction of data. 0 = read, 1 = write
 *
 * returns: value of register if dir = 0 else 0
 *
 * Mostly borrowed from EightyOne - A Windows ZX80/81/clone emulator
 */

static libspectrum_byte
opus_6821_access( libspectrum_byte reg, libspectrum_byte data,
                  libspectrum_byte dir )
{
  int drive, side;
  int i;

  switch( reg & 0x03 ) {
  case 0:
    if( dir ) {
      if( control_a & 0x04 ) {
        data_reg_a = data;

        drive = ( data & 0x02 ) == 2 ? 1 : 0;
        side = ( data & 0x10 )>>4 ? 1 : 0;

        for( i = 0; i < OPUS_NUM_DRIVES; i++ ) {
          fdd_set_head( &opus_drives[ i ].fdd, side );
        }

        fdd_select( &opus_drives[ (!drive) ].fdd, 0 );
        fdd_select( &opus_drives[ drive ].fdd, 1 );

        if( opus_fdc->current_drive != &opus_drives[ drive ] ) {
          if( opus_fdc->current_drive->fdd.motoron ) {        /* swap motoron */
            fdd_motoron( &opus_drives[ (!drive) ].fdd, 0 );
            fdd_motoron( &opus_drives[ drive ].fdd, 1 );
          }
          opus_fdc->current_drive = &opus_drives[ drive ];
        }
      } else {
        data_dir_a = data;
      }
    } else {
      if( control_a & 0x04 ) {
        /* printer never busy (bit 6) */
        data_reg_a &= ~0x40;
        return data_reg_a;
      } else {
        return data_dir_a;
      }
    }
    break;
  case 1:
    if( dir ) {
      control_a = data;
    } else {
      /* Always return bit 6 set to ACK parallel port actions */
      return control_a | 0x40;
    }
    break;
  case 2:
    if( dir ) {
      if( control_b & 0x04 ) {
        data_reg_b = data;
        printer_parallel_write( 0x00, data );
        /* Don't worry about emulating the strobes from the ROM, they are
           all bound up with checking current printer busy status which we
           don't emulate, so just send the char now */
        printer_parallel_strobe_write( 0 );
        printer_parallel_strobe_write( 1 );
        printer_parallel_strobe_write( 0 );
      } else {
        data_dir_b = data;
      }
    } else {
      if( control_b & 0x04 ) {
        return data_reg_b;
      } else {
        return data_dir_b;
      }
    }
    break;
  case 3:
    if( dir ) {
      control_b = data;
    } else {
      return control_b;
    }
    break;
  }

  return 0;
}

int
opus_disk_insert( opus_drive_number which, const char *filename,
		   int autoload )
{
  int error;
  wd_fdc_drive *d;
  const fdd_params_t *dt;

  if( which >= OPUS_NUM_DRIVES ) {
    ui_error( UI_ERROR_ERROR, "opus_disk_insert: unknown drive %d",
	      which );
    fuse_abort();
  }

  d = &opus_drives[ which ];

  /* Eject any disk already in the drive */
  if( d->fdd.loaded ) {
    /* Abort the insert if we want to keep the current disk */
    if( opus_disk_eject( which ) ) return 0;
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
      /* +1 => there is no `Disabled' */
      dt = &fdd_params[ option_enumerate_diskoptions_drive_opus1_type() + 1 ];
      break;
    case 1:
    default:
      dt = &fdd_params[ option_enumerate_diskoptions_drive_opus2_type() ];
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
  case OPUS_DRIVE_1:
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_OPUS_1_EJECT, 1 );
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_OPUS_1_FLIP_SET,
		      !opus_drives[ OPUS_DRIVE_1 ].fdd.upsidedown );
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_OPUS_1_WP_SET,
		      !opus_drives[ OPUS_DRIVE_1 ].fdd.wrprot );
    break;
  case OPUS_DRIVE_2:
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_OPUS_2_EJECT, 1 );
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_OPUS_2_FLIP_SET,
		      !opus_drives[ OPUS_DRIVE_2 ].fdd.upsidedown );
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_OPUS_2_WP_SET,
		      !opus_drives[ OPUS_DRIVE_2 ].fdd.wrprot );
    break;
  }

  if( filename && autoload ) {
    /* XXX */
  }

  return 0;
}

int
opus_disk_eject( opus_drive_number which )
{
  wd_fdc_drive *d;

  if( which >= OPUS_NUM_DRIVES )
    return 1;

  d = &opus_drives[ which ];

  if( d->disk.type == DISK_TYPE_NONE )
    return 0;

  if( d->disk.dirty ) {

    ui_confirm_save_t confirm = ui_confirm_save(
      "Disk in Opus Discovery drive %c has been modified.\n"
      "Do you want to save it?",
      which == OPUS_DRIVE_1 ? '1' : '2'
    );

    switch( confirm ) {

    case UI_CONFIRM_SAVE_SAVE:
      if( opus_disk_save( which, 0 ) ) return 1;	/* first save */
      break;

    case UI_CONFIRM_SAVE_DONTSAVE: break;
    case UI_CONFIRM_SAVE_CANCEL: return 1;

    }
  }

  fdd_unload( &d->fdd );
  disk_close( &d->disk );

  /* Set the 'eject' item inactive */
  switch( which ) {
  case OPUS_DRIVE_1:
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_OPUS_1_EJECT, 0 );
    break;
  case OPUS_DRIVE_2:
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_OPUS_2_EJECT, 0 );
    break;
  }
  return 0;
}

int
opus_disk_save( opus_drive_number which, int saveas )
{
  wd_fdc_drive *d;

  if( which >= OPUS_NUM_DRIVES )
    return 1;

  d = &opus_drives[ which ];

  if( d->disk.type == DISK_TYPE_NONE )
    return 0;

  if( d->disk.filename == NULL ) saveas = 1;
  if( ui_opus_disk_write( which, saveas ) ) return 1;
  d->disk.dirty = 0;
  return 0;
}

int
opus_disk_flip( opus_drive_number which, int flip )
{
  wd_fdc_drive *d;

  if( which >= OPUS_NUM_DRIVES )
    return 1;

  d = &opus_drives[ which ];

  if( !d->fdd.loaded )
    return 1;

  fdd_flip( &d->fdd, flip );

  /* Update the 'write flip' menu item */
  switch( which ) {
  case OPUS_DRIVE_1:
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_OPUS_1_FLIP_SET,
		      !opus_drives[ OPUS_DRIVE_1 ].fdd.upsidedown );
    break;
  case OPUS_DRIVE_2:
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_OPUS_2_FLIP_SET,
		      !opus_drives[ OPUS_DRIVE_2 ].fdd.upsidedown );
    break;
  }
  return 0;
}

int
opus_disk_writeprotect( opus_drive_number which, int wrprot )
{
  wd_fdc_drive *d;

  if( which >= OPUS_NUM_DRIVES )
    return 1;

  d = &opus_drives[ which ];

  if( !d->fdd.loaded )
    return 1;

  fdd_wrprot( &d->fdd, wrprot );

  /* Update the 'write protect' menu item */
  switch( which ) {
  case OPUS_DRIVE_1:
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_OPUS_1_WP_SET,
		      !opus_drives[ OPUS_DRIVE_1 ].fdd.wrprot );
    break;
  case OPUS_DRIVE_2:
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_OPUS_2_WP_SET,
		      !opus_drives[ OPUS_DRIVE_2 ].fdd.wrprot );
    break;
  }
  return 0;
}

int
opus_disk_write( opus_drive_number which, const char *filename )
{
  wd_fdc_drive *d = &opus_drives[ which ];
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
opus_get_fdd( opus_drive_number which )
{
  return &( opus_drives[ which ].fdd );
}

static void
opus_event_index( libspectrum_dword last_tstates, int type GCC_UNUSED,
                   void *user_data GCC_UNUSED )
{
  int next_tstates;
  int i;

  opus_index_pulse = !opus_index_pulse;
  for( i = 0; i < OPUS_NUM_DRIVES; i++ ) {
    wd_fdc_drive *d = &opus_drives[ i ];

    d->index_pulse = opus_index_pulse;
    if( !opus_index_pulse && d->index_interrupt ) {
      wd_fdc_set_intrq( opus_fdc );
      d->index_interrupt = 0;
    }
  }
  next_tstates = ( opus_index_pulse ? 10 : 190 ) *
    machine_current->timings.processor_speed / 1000;
  event_add( last_tstates + next_tstates, index_event );
}

libspectrum_byte
opus_read( libspectrum_word address )
{
  libspectrum_byte data = 0xff;

  if( address >= 0x3800 ) data = 0xff; /* Undefined on Opus */
  else if( address >= 0x3000 )         /* 6821 PIA */
    data = opus_6821_access( address, 0, 0 );
  else if( address >= 0x2800 ) {       /* WD1770 FDC */
    switch( address & 0x03 ) {
    case 0:
      data = wd_fdc_sr_read( opus_fdc );
      break;
    case 1:
      data = wd_fdc_tr_read( opus_fdc );
      break;
    case 2:
      data = wd_fdc_sec_read( opus_fdc );
      break;
    case 3:
      data = wd_fdc_dr_read( opus_fdc );
      break;
    }
  }

  return data;
}

void
opus_write( libspectrum_word address, libspectrum_byte b )
{
  if( address < 0x2000 ) return;
  if( address >= 0x3800 ) return;

  if( address >= 0x3000 ) {
    opus_6821_access( address, b, 1 );
  } else if( address >= 0x2800 ) {
    switch( address & 0x03 ) {
    case 0:
      wd_fdc_cr_write( opus_fdc, b );
      break;
    case 1:
      wd_fdc_tr_write( opus_fdc, b );
      break;
    case 2:
      wd_fdc_sec_write( opus_fdc, b );
      break;
    case 3:
      wd_fdc_dr_write( opus_fdc, b );
      break;
    }
  }
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
opus_enabled_snapshot( libspectrum_snap *snap )
{
  if( libspectrum_snap_opus_active( snap ) )
    settings_current.opus = 1;
}

static void
opus_from_snapshot( libspectrum_snap *snap )
{
  if( !libspectrum_snap_opus_active( snap ) ) return;

  if( libspectrum_snap_opus_custom_rom( snap ) &&
      libspectrum_snap_opus_rom( snap, 0 ) &&
      machine_load_rom_bank_from_buffer(
                             opus_memory_map_romcs_rom, 0,
                             libspectrum_snap_opus_rom( snap, 0 ),
                             0x2000, 1 ) )
    return;

  if( libspectrum_snap_opus_ram( snap, 0 ) ) {
    memcpy( opus_ram,
            libspectrum_snap_opus_ram( snap, 0 ), TRUE_OPUS_RAM_SIZE );
  }

  /* ignore drive count for now, there will be an issue with loading snaps where
     drives have been disabled
  libspectrum_snap_opus_drive_count( snap )
   */

  opus_fdc->direction = libspectrum_snap_opus_direction( snap );

  wd_fdc_cr_write ( opus_fdc, libspectrum_snap_opus_status ( snap ) );
  wd_fdc_tr_write ( opus_fdc, libspectrum_snap_opus_track  ( snap ) );
  wd_fdc_sec_write( opus_fdc, libspectrum_snap_opus_sector ( snap ) );
  wd_fdc_dr_write ( opus_fdc, libspectrum_snap_opus_data   ( snap ) );
  data_reg_a = libspectrum_snap_opus_data_reg_a( snap );
  data_dir_a = libspectrum_snap_opus_data_dir_a( snap );
  control_a  = libspectrum_snap_opus_control_a ( snap );
  data_reg_b = libspectrum_snap_opus_data_reg_b( snap );
  data_dir_b = libspectrum_snap_opus_data_dir_b( snap );
  control_b  = libspectrum_snap_opus_control_b ( snap );

  if( libspectrum_snap_opus_paged( snap ) ) {
    opus_page();
  } else {
    opus_unpage();
  }
}

static void
opus_to_snapshot( libspectrum_snap *snap )
{
  libspectrum_byte *buffer;
  int drive_count = 0;

  if( !periph_is_active( PERIPH_TYPE_OPUS ) ) return;

  libspectrum_snap_set_opus_active( snap, 1 );

  buffer = alloc_and_copy_page( opus_memory_map_romcs_rom[0].page );
  if( !buffer ) return;
  libspectrum_snap_set_opus_rom( snap, 0, buffer );
  if( opus_memory_map_romcs_rom[0].save_to_snapshot )
    libspectrum_snap_set_opus_custom_rom( snap, 1 );

  buffer = alloc_and_copy_page( opus_ram );
  if( !buffer ) return;
  libspectrum_snap_set_opus_ram( snap, 0, buffer );

  drive_count++; /* Drive 1 is not removable */
  if( option_enumerate_diskoptions_drive_opus2_type() > 0 ) drive_count++;
  libspectrum_snap_set_opus_drive_count( snap, drive_count );

  libspectrum_snap_set_opus_paged     ( snap, opus_active );
  libspectrum_snap_set_opus_direction ( snap, opus_fdc->direction );
  libspectrum_snap_set_opus_status    ( snap, opus_fdc->status_register );
  libspectrum_snap_set_opus_track     ( snap, opus_fdc->track_register );
  libspectrum_snap_set_opus_sector    ( snap, opus_fdc->sector_register );
  libspectrum_snap_set_opus_data      ( snap, opus_fdc->data_register );
  libspectrum_snap_set_opus_data_reg_a( snap, data_reg_a );
  libspectrum_snap_set_opus_data_dir_a( snap, data_dir_a );
  libspectrum_snap_set_opus_control_a ( snap, control_a );
  libspectrum_snap_set_opus_data_reg_b( snap, data_reg_b );
  libspectrum_snap_set_opus_data_dir_b( snap, data_dir_b );
  libspectrum_snap_set_opus_control_b ( snap, control_b );
}

int
opus_unittest( void )
{
  int r = 0;

  opus_page();

  r += unittests_assert_8k_page( 0x0000, opus_rom_memory_source, 0 );
  r += unittests_assert_4k_page( 0x2000, opus_ram_memory_source, 0 );
  r += unittests_assert_4k_page( 0x3000, memory_source_rom, 0 );
  r += unittests_assert_16k_ram_page( 0x4000, 5 );
  r += unittests_assert_16k_ram_page( 0x8000, 2 );
  r += unittests_assert_16k_ram_page( 0xc000, 0 );

  opus_unpage();

  r += unittests_paging_test_48( 2 );

  return r;
}

