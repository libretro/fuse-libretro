/* disciple.c: Routines for handling the DISCiPLE interface
   Copyright (c) 1999-2011 Stuart Brady, Fredrick Meunier, Philip Kendall,
   Dmitry Sanarin, Darren Salt

   $Id: disciple.c 4906 2013-03-10 11:21:37Z fredm $

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
#include "disciple.h"
#include "machine.h"
#include "module.h"
#include "peripherals/printer.h"
#include "settings.h"
#include "ui/ui.h"
#include "unittests/unittests.h"
#include "utils.h"
#include "wd_fdc.h"
#include "options.h"	/* needed for get combo options */

#define DISK_TRY_MERGE(heads) ( option_enumerate_diskoptions_disk_try_merge() == 2 || \
				( option_enumerate_diskoptions_disk_try_merge() == 1 && heads == 1 ) )

/* Two 8 KiB memory chunks accessible by the Z80 when /ROMCS is low */
/* One 8 KiB chunk of ROM, one 8 KiB chunk of RAM */
/* TODO: add support for 16 KiB ROM images. */
/* Real hardware supports the use of a 16 KiB ROM, but all of the 16 KiB
 * 'ROM dumps' for the DISCiPLE actually contain a dump of GDOS (RAM).
 * Uni-DOS also uses an 8 KiB ROM. */
static memory_page disciple_memory_map_romcs_rom[ MEMORY_PAGES_IN_8K ];
static memory_page disciple_memory_map_romcs_ram[ MEMORY_PAGES_IN_8K ];

static int disciple_memory_source_rom;
static int disciple_memory_source_ram;

int disciple_memswap = 0;        /* Are the ROM and RAM pages swapped? */
/* TODO: add support for 16 KiB ROM images. */
/* int disciple_rombank = 0; */
int disciple_inhibited;

int disciple_available = 0;
int disciple_active = 0;

static int disciple_index_pulse;

static int index_event;

#define DISCIPLE_NUM_DRIVES 2

static wd_fdc *disciple_fdc;
static wd_fdc_drive disciple_drives[ DISCIPLE_NUM_DRIVES ];

static libspectrum_byte *disciple_ram;
static int memory_allocated = 0;

static void disciple_reset( int hard_reset );
static void disciple_memory_map( void );
static void disciple_event_index( libspectrum_dword last_tstates, int type,
				  void *user_data );
static void disciple_activate( void );

static module_info_t disciple_module_info = {

  disciple_reset,
  disciple_memory_map,
  NULL,
  NULL,
  NULL,

};

static libspectrum_byte disciple_control_register;

void
disciple_page( void )
{
  disciple_active = 1;
  machine_current->ram.romcs = 1;
  machine_current->memory_map();
}

void
disciple_unpage( void )
{
  disciple_active = 0;
  machine_current->ram.romcs = 0;
  machine_current->memory_map();
}

void
disciple_memory_map( void )
{
  struct memory_page *rom_page, *lower_page, *upper_page;

  if( !disciple_active ) return;

  /* TODO: add support for 16 KiB ROM images. */
  rom_page = disciple_memory_map_romcs_rom;

  if( !disciple_memswap ) {
    lower_page = rom_page;
    upper_page = disciple_memory_map_romcs_ram;
  } else {
    lower_page = disciple_memory_map_romcs_ram;
    upper_page = rom_page;
  }

  memory_map_romcs_8k( 0x0000, lower_page );
  memory_map_romcs_8k( 0x2000, upper_page );
}

static const periph_port_t disciple_ports[] = {
  /* ---- ---- 0001 1011 */
  { 0x00ff, 0x001b, disciple_sr_read, disciple_cr_write },
  /* ---- ---- 0101 1011  */
  { 0x00ff, 0x005b, disciple_tr_read, disciple_tr_write },
  /* ---- ---- 1001 1011 */
  { 0x00ff, 0x009b, disciple_sec_read, disciple_sec_write },
  /* ---- ---- 1101 1011 */
  { 0x00ff, 0x00db, disciple_dr_read, disciple_dr_write },

  /* ---- ---- 0001 1111 */
  { 0x00ff, 0x001f, disciple_joy_read, disciple_cn_write },
  /* ---- ---- 0011 1011 */
  { 0x00ff, 0x003b, NULL, disciple_net_write },
  /* ---- ---- 0111 1011 */
  { 0x00ff, 0x007b, disciple_boot_read, disciple_boot_write },
  /* ---- ---- 1011 1011 */
  { 0x00ff, 0x00bb, disciple_patch_read, disciple_patch_write },
  /* ---- ---- 1111 1011 */
  { 0x00ff, 0x00fb, NULL, disciple_printer_write },

  { 0, 0, NULL, NULL }
};

static const periph_t disciple_periph = {
  &settings_current.disciple,
  disciple_ports,
  1,
  disciple_activate
};

void
disciple_init( void )
{
  int i;
  wd_fdc_drive *d;

  disciple_fdc = wd_fdc_alloc_fdc( WD1770, 0, WD_FLAG_NONE );

  for( i = 0; i < DISCIPLE_NUM_DRIVES; i++ ) {
    d = &disciple_drives[ i ];
    fdd_init( &d->fdd, FDD_SHUGART, NULL, 0 );
    d->disk.flag = DISK_FLAG_NONE;
  }

  disciple_fdc->current_drive = &disciple_drives[ 0 ];
  fdd_select( &disciple_drives[ 0 ].fdd, 1 );
  disciple_fdc->dden = 1;
  disciple_fdc->set_intrq = NULL;
  disciple_fdc->reset_intrq = NULL;
  disciple_fdc->set_datarq = NULL;
  disciple_fdc->reset_datarq = NULL;
  disciple_fdc->iface = NULL;

  index_event = event_register( disciple_event_index, "DISCiPLE index" );

  module_register( &disciple_module_info );
  
  disciple_memory_source_rom = memory_source_register( "DISCiPLE ROM" );
  disciple_memory_source_ram = memory_source_register( "DISCiPLE RAM" );

  for( i = 0; i < MEMORY_PAGES_IN_8K; i++ ) {
    disciple_memory_map_romcs_rom[i].source = disciple_memory_source_rom;
    disciple_memory_map_romcs_rom[i].page_num = 0;
    disciple_memory_map_romcs_rom[i].writable = 0;
  }

  for( i = 0; i < MEMORY_PAGES_IN_8K; i++ ) {
    disciple_memory_map_romcs_ram[i].source = disciple_memory_source_ram;
    disciple_memory_map_romcs_ram[i].page_num = 0;
    disciple_memory_map_romcs_ram[i].writable = 1;
  }

  periph_register( PERIPH_TYPE_DISCIPLE, &disciple_periph );
}

static void
disciple_reset( int hard_reset )
{
  int i;
  wd_fdc_drive *d;
  const fdd_params_t *dt;

  disciple_active = 0;
  disciple_available = 0;

  event_remove_type( index_event );

  if( !periph_is_active( PERIPH_TYPE_DISCIPLE ) ) {
    return;
  }

  /* TODO: add support for 16 KiB ROM images. */
  if( machine_load_rom_bank( disciple_memory_map_romcs_rom, 0,
			     settings_current.rom_disciple,
			     settings_default.rom_disciple, 0x2000 ) ) {
    settings_current.disciple = 0;
    periph_activate_type( PERIPH_TYPE_DISCIPLE, 0 );
    return;
  }

  for( i = 0; i < MEMORY_PAGES_IN_8K; i++ )
    disciple_memory_map_romcs_ram[ i ].page = &disciple_ram[ i * MEMORY_PAGE_SIZE ];

  machine_current->ram.romcs = 1;

  for( i = 0; i < MEMORY_PAGES_IN_8K; i++ )
    disciple_memory_map_romcs_ram[ i ].writable = 1;

  disciple_available = 1;
  disciple_active = 1;
  disciple_index_pulse = 0;

  disciple_memswap = 0;
  /* TODO: add support for 16 KiB ROM images. */
  /* disciple_rombank = 0; */

  if( hard_reset )
    memset( disciple_ram, 0, 0x2000 );

  wd_fdc_master_reset( disciple_fdc );

  for( i = 0; i < DISCIPLE_NUM_DRIVES; i++ ) {
    d = &disciple_drives[ i ];

    d->index_pulse = 0;
    d->index_interrupt = 0;
  }

  /* We can eject disks only if they are currently present */
  dt = &fdd_params[ option_enumerate_diskoptions_drive_disciple1_type() + 1 ];
  fdd_init( &disciple_drives[ DISCIPLE_DRIVE_1 ].fdd, FDD_SHUGART, dt, 1 );
  ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_DISCIPLE_1, dt->enabled );
  ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_DISCIPLE_1_EJECT,
		    disciple_drives[ DISCIPLE_DRIVE_1 ].fdd.loaded );
  ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_DISCIPLE_1_FLIP_SET,
		    !disciple_drives[ DISCIPLE_DRIVE_1 ].fdd.upsidedown );
  ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_DISCIPLE_1_WP_SET,
		    !disciple_drives[ DISCIPLE_DRIVE_1 ].fdd.wrprot );


  dt = &fdd_params[ option_enumerate_diskoptions_drive_disciple2_type() ];
  fdd_init( &disciple_drives[ DISCIPLE_DRIVE_2 ].fdd, dt->enabled ? FDD_SHUGART : FDD_TYPE_NONE, dt, 1 );
  ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_DISCIPLE_2, dt->enabled );
  ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_DISCIPLE_2_EJECT,
		    disciple_drives[ DISCIPLE_DRIVE_2 ].fdd.loaded );
  ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_DISCIPLE_2_FLIP_SET,
		    !disciple_drives[ DISCIPLE_DRIVE_2 ].fdd.upsidedown );
  ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_DISCIPLE_2_WP_SET,
		    !disciple_drives[ DISCIPLE_DRIVE_2 ].fdd.wrprot );


  disciple_fdc->current_drive = &disciple_drives[ 0 ];
  fdd_select( &disciple_drives[ 0 ].fdd, 1 );
  machine_current->memory_map();
  disciple_event_index( 0, index_event, NULL );

  ui_statusbar_update( UI_STATUSBAR_ITEM_DISK, UI_STATUSBAR_STATE_INACTIVE );
}

void
disciple_inhibit( void )
{
  /* TODO: check how this affects the hardware */
  disciple_inhibited = 1;
}

void
disciple_end( void )
{
  disciple_available = 0;
  free( disciple_fdc );
}

libspectrum_byte
disciple_sr_read( libspectrum_word port GCC_UNUSED, int *attached )
{
  *attached = 1;
  return wd_fdc_sr_read( disciple_fdc );
}

void
disciple_cr_write( libspectrum_word port GCC_UNUSED, libspectrum_byte b )
{
  wd_fdc_cr_write( disciple_fdc, b );
}

libspectrum_byte
disciple_tr_read( libspectrum_word port GCC_UNUSED, int *attached )
{
  *attached = 1;
  return wd_fdc_tr_read( disciple_fdc );
}

void
disciple_tr_write( libspectrum_word port GCC_UNUSED, libspectrum_byte b )
{
  wd_fdc_tr_write( disciple_fdc, b );
}

libspectrum_byte
disciple_sec_read( libspectrum_word port GCC_UNUSED, int *attached )
{
  *attached = 1;
  return wd_fdc_sec_read( disciple_fdc );
}

void
disciple_sec_write( libspectrum_word port GCC_UNUSED, libspectrum_byte b )
{
  wd_fdc_sec_write( disciple_fdc, b );
}

libspectrum_byte
disciple_dr_read( libspectrum_word port GCC_UNUSED, int *attached )
{
  *attached = 1;
  return wd_fdc_dr_read( disciple_fdc );
}

void
disciple_dr_write( libspectrum_word port GCC_UNUSED, libspectrum_byte b )
{
  wd_fdc_dr_write( disciple_fdc, b );
}

libspectrum_byte
disciple_joy_read( libspectrum_word port GCC_UNUSED, int *attached )
{
  *attached = 1;

  /* bit 6 - printer busy */
  if( !settings_current.printer )
    return 0xbf; /* no printer attached */

  return 0xff;   /* never busy */
}

void
disciple_cn_write( libspectrum_word port GCC_UNUSED, libspectrum_byte b )
{
  int drive, side;
  int i;

  disciple_control_register = b;

  drive = ( b & 0x01 ) ? 0 : 1;
  side = ( b & 0x02 ) ? 1 : 0;

  for( i = 0; i < DISCIPLE_NUM_DRIVES; i++ ) {
    fdd_set_head( &disciple_drives[ i ].fdd, side );
    fdd_select( &disciple_drives[ i ].fdd, drive == i );
  }

  if( disciple_fdc->current_drive != &disciple_drives[ drive ] ) {
    if( disciple_fdc->current_drive->fdd.motoron ) {
      for (i = 0; i < DISCIPLE_NUM_DRIVES; i++ ) {
        fdd_motoron( &disciple_drives[ i ].fdd, drive == i );
      }
    }
    disciple_fdc->current_drive = &disciple_drives[ drive ];
  }

  printer_parallel_strobe_write( b & 0x40 );

  /* We only support the use of an 8 KiB ROM. */
  /* disciple_rombank = ( b & 0x08 ) ? 1 : 0; */
  machine_current->memory_map();
  if( b & 0x10 )
    disciple_inhibit();
}

void
disciple_net_write( libspectrum_word port GCC_UNUSED,
		    libspectrum_byte b GCC_UNUSED )
{
  /* TODO: implement network emulation */
}

libspectrum_byte
disciple_boot_read( libspectrum_word port GCC_UNUSED,
		    int *attached GCC_UNUSED )
{
  disciple_memswap = 0;
  machine_current->memory_map();
  return 0;
}

void
disciple_boot_write( libspectrum_word port GCC_UNUSED,
		     libspectrum_byte b GCC_UNUSED )
{
  disciple_memswap = 1;
  machine_current->memory_map();
}

libspectrum_byte
disciple_patch_read( libspectrum_word port GCC_UNUSED,
		     int *attached GCC_UNUSED )
{
  disciple_page();
  return 0;
}

void
disciple_patch_write( libspectrum_word port GCC_UNUSED,
		    libspectrum_byte b GCC_UNUSED )
{
  disciple_unpage();
}

void
disciple_printer_write( libspectrum_word port, libspectrum_byte b )
{
  printer_parallel_write( port, b );
}

int
disciple_disk_insert( disciple_drive_number which, const char *filename,
		      int autoload )
{
  int error;
  wd_fdc_drive *d;
  const fdd_params_t *dt;

  if( which >= DISCIPLE_NUM_DRIVES ) {
    ui_error( UI_ERROR_ERROR, "disciple_disk_insert: unknown drive %d",
	      which );
    fuse_abort();
  }

  d = &disciple_drives[ which ];

  /* Eject any disk already in the drive */
  if( d->fdd.loaded ) {
    /* Abort the insert if we want to keep the current disk */
    if( disciple_disk_eject( which ) ) return 0;
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
      dt = &fdd_params[ option_enumerate_diskoptions_drive_disciple1_type() + 1 ];	/* +1 => there is no 'Disabled' */
      break;
    case 1:
    default:
      dt = &fdd_params[ option_enumerate_diskoptions_drive_disciple2_type() ];
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
  case DISCIPLE_DRIVE_1:
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_DISCIPLE_1_EJECT, 1 );
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_DISCIPLE_1_FLIP_SET,
		      !disciple_drives[ DISCIPLE_DRIVE_1 ].fdd.upsidedown );
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_DISCIPLE_1_WP_SET,
		      !disciple_drives[ DISCIPLE_DRIVE_1 ].fdd.wrprot );
    break;
  case DISCIPLE_DRIVE_2:
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_DISCIPLE_2_EJECT, 1 );
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_DISCIPLE_2_FLIP_SET,
		      !disciple_drives[ DISCIPLE_DRIVE_2 ].fdd.upsidedown );
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_DISCIPLE_2_WP_SET,
		      !disciple_drives[ DISCIPLE_DRIVE_2 ].fdd.wrprot );
    break;
  }

  if( filename && autoload ) {
    /* XXX */
  }

  return 0;
}

int
disciple_disk_eject( disciple_drive_number which )
{
  wd_fdc_drive *d;

  if( which >= DISCIPLE_NUM_DRIVES )
    return 1;

  d = &disciple_drives[ which ];

  if( d->disk.type == DISK_TYPE_NONE )
    return 0;

  if( d->disk.dirty ) {

    ui_confirm_save_t confirm = ui_confirm_save(
      "Disk in DISCiPLE drive %c has been modified.\n"
      "Do you want to save it?",
      which == DISCIPLE_DRIVE_1 ? '1' : '2'
    );

    switch( confirm ) {

    case UI_CONFIRM_SAVE_SAVE:
      if( disciple_disk_save( which, 0 ) ) return 1;	/* first save */
      break;

    case UI_CONFIRM_SAVE_DONTSAVE: break;
    case UI_CONFIRM_SAVE_CANCEL: return 1;

    }
  }

  fdd_unload( &d->fdd );
  disk_close( &d->disk );

  /* Set the 'eject' item inactive */
  switch( which ) {
  case DISCIPLE_DRIVE_1:
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_DISCIPLE_1_EJECT, 0 );
    break;
  case DISCIPLE_DRIVE_2:
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_DISCIPLE_2_EJECT, 0 );
    break;
  }
  return 0;
}

int
disciple_disk_save( disciple_drive_number which, int saveas )
{
  wd_fdc_drive *d;

  if( which >= DISCIPLE_NUM_DRIVES )
    return 1;

  d = &disciple_drives[ which ];

  if( d->disk.type == DISK_TYPE_NONE )
    return 0;

  if( d->disk.filename == NULL ) saveas = 1;
  if( ui_disciple_disk_write( which, saveas ) ) return 1;
  d->disk.dirty = 0;
  return 0;
}

int
disciple_disk_flip( disciple_drive_number which, int flip )
{
  wd_fdc_drive *d;

  if( which >= DISCIPLE_NUM_DRIVES )
    return 1;

  d = &disciple_drives[ which ];

  if( !d->fdd.loaded )
    return 1;

  fdd_flip( &d->fdd, flip );

  /* Update the 'write flip' menu item */
  switch( which ) {
  case DISCIPLE_DRIVE_1:
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_DISCIPLE_1_FLIP_SET,
		      !disciple_drives[ DISCIPLE_DRIVE_1 ].fdd.upsidedown );
    break;
  case DISCIPLE_DRIVE_2:
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_DISCIPLE_2_FLIP_SET,
		      !disciple_drives[ DISCIPLE_DRIVE_2 ].fdd.upsidedown );
    break;
  }
  return 0;
}

int
disciple_disk_writeprotect( disciple_drive_number which, int wrprot )
{
  wd_fdc_drive *d;

  if( which >= DISCIPLE_NUM_DRIVES )
    return 1;

  d = &disciple_drives[ which ];

  if( !d->fdd.loaded )
    return 1;

  fdd_wrprot( &d->fdd, wrprot );

  /* Update the 'write protect' menu item */
  switch( which ) {
  case DISCIPLE_DRIVE_1:
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_DISCIPLE_1_WP_SET,
		      !disciple_drives[ DISCIPLE_DRIVE_1 ].fdd.wrprot );
    break;
  case DISCIPLE_DRIVE_2:
    ui_menu_activate( UI_MENU_ITEM_MEDIA_DISK_DISCIPLE_2_WP_SET,
		      !disciple_drives[ DISCIPLE_DRIVE_2 ].fdd.wrprot );
    break;
  }
  return 0;
}

/***TODO most part of the next routine could be move to a common place... */
int
disciple_disk_write( disciple_drive_number which, const char *filename )
{
  wd_fdc_drive *d = &disciple_drives[ which ];
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
disciple_get_fdd( disciple_drive_number which )
{
  return &( disciple_drives[ which ].fdd );
}

static void
disciple_event_index( libspectrum_dword last_tstates, int type GCC_UNUSED,
		      void *user_data GCC_UNUSED )
{
  int next_tstates;
  int i;

  disciple_index_pulse = !disciple_index_pulse;
  for( i = 0; i < DISCIPLE_NUM_DRIVES; i++ ) {
    wd_fdc_drive *d = &disciple_drives[ i ];

    d->index_pulse = disciple_index_pulse;
    if( !disciple_index_pulse && d->index_interrupt ) {
      wd_fdc_set_intrq( disciple_fdc );
      d->index_interrupt = 0;
    }
  }
  next_tstates = ( disciple_index_pulse ? 10 : 190 ) *
    machine_current->timings.processor_speed / 1000;
  event_add( last_tstates + next_tstates, index_event );
}

static void
disciple_activate( void )
{
  if( !memory_allocated ) {
    disciple_ram = memory_pool_allocate_persistent( 0x2000, 1 );
    memory_allocated = 1;
  }
}

int
disciple_unittest( void )
{
  int r = 0;
  /* We only support the use of an 8 KiB ROM.  Change this to 1 if adding
   * support for 16 KiB ROMs. */
  const int upper_rombank = 0;

  disciple_page();

  r += unittests_assert_8k_page( 0x0000, disciple_memory_source_rom, 0 );
  r += unittests_assert_8k_page( 0x2000, disciple_memory_source_ram, 0 );
  r += unittests_assert_16k_ram_page( 0x4000, 5 );
  r += unittests_assert_16k_ram_page( 0x8000, 2 );
  r += unittests_assert_16k_ram_page( 0xc000, 0 );

  disciple_cn_write( 0x001f, 0x08 );
  r += unittests_assert_8k_page( 0x0000, disciple_memory_source_rom,
				 upper_rombank );
  r += unittests_assert_8k_page( 0x2000, disciple_memory_source_ram, 0 );
  r += unittests_assert_16k_ram_page( 0x4000, 5 );
  r += unittests_assert_16k_ram_page( 0x8000, 2 );
  r += unittests_assert_16k_ram_page( 0xc000, 0 );

  disciple_boot_write( 0x007b, 0x00 );
  r += unittests_assert_8k_page( 0x0000, disciple_memory_source_ram, 0 );
  r += unittests_assert_8k_page( 0x2000, disciple_memory_source_rom,
				 upper_rombank );
  r += unittests_assert_16k_ram_page( 0x4000, 5 );
  r += unittests_assert_16k_ram_page( 0x8000, 2 );
  r += unittests_assert_16k_ram_page( 0xc000, 0 );

  disciple_cn_write( 0x001f, 0x00 );
  r += unittests_assert_8k_page( 0x0000, disciple_memory_source_ram, 0 );
  r += unittests_assert_8k_page( 0x2000, disciple_memory_source_rom, 0 );
  r += unittests_assert_16k_ram_page( 0x4000, 5 );
  r += unittests_assert_16k_ram_page( 0x8000, 2 );
  r += unittests_assert_16k_ram_page( 0xc000, 0 );

  disciple_boot_read( 0x007b, NULL );
  r += unittests_assert_8k_page( 0x0000, disciple_memory_source_rom, 0 );
  r += unittests_assert_8k_page( 0x2000, disciple_memory_source_ram, 0 );
  r += unittests_assert_16k_ram_page( 0x4000, 5 );
  r += unittests_assert_16k_ram_page( 0x8000, 2 );
  r += unittests_assert_16k_ram_page( 0xc000, 0 );

  disciple_unpage();

  r += unittests_paging_test_48( 2 );

  return r;
}
