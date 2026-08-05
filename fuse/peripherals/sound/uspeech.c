/* uspeech.c: Routines for handling the Currah uSpeech interface
   Copyright (c) 2007-2023 Stuart Brady
   Copyright (c) 2023 Sergio Baldoví, Vic Chwe

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

#include "config.h"

#include <string.h>

#include "libspectrum.h"

#include "compat.h"
#include "debugger/debugger.h"
#include "infrastructure/startup_manager.h"
#include "machine.h"
#include "memory_pages.h"
#include "module.h"
#include "periph.h"
#include "settings.h"
#include "sp0256.h"
#include "ui/ui.h"
#include "unittests/unittests.h"
#include "uspeech.h"
#include "utils.h"

#define SP0256_ROM_SIZE 2048

/* Intonation on uSpeech changes SP0256 clock/oscillator frequency.
   These values are based on a Kio's simulation and Thomas Busse tests. */
#define SP0256_XTAL_NORMAL 3050000
#define SP0256_XTAL_HIGH   3260000

/* A 2 KiB memory chunk accessible by the Z80 when /ROMCS is low
 * (mirrored when active) */
static memory_page uspeech_memory_map_romcs[ MEMORY_PAGES_IN_2K ];
static memory_page uspeech_empty_mapping[ MEMORY_PAGES_IN_12K ];
static int empty_mapping_allocated = 0;

static int uspeech_memory_source;

static libspectrum_byte *sp0256rom = NULL;

int uspeech_active = 0;
int uspeech_available = 0;

static void uspeech_toggle_write( libspectrum_word port,
                                  libspectrum_byte val );
static libspectrum_byte uspeech_toggle_read( libspectrum_word port,
                                             libspectrum_byte *attached );
static libspectrum_byte uspeech_port_play_read( libspectrum_word port,
                                                libspectrum_byte *attached );
static void uspeech_port_play_write( libspectrum_word port,
                                     libspectrum_byte data );
static void uspeech_port_intonation_normal( libspectrum_word port,
                                            libspectrum_byte data );
static void uspeech_port_intonation_high( libspectrum_word port,
                                          libspectrum_byte data );

static int uspeech_load_sp0256_rom( void );
static int uspeech_sp0256_reset( void );
static void uspeech_reset( int hard_reset );
static void uspeech_memory_map( void );

static void uspeech_enabled_snapshot( libspectrum_snap *snap );
static void uspeech_from_snapshot( libspectrum_snap *snap );
static void uspeech_to_snapshot( libspectrum_snap *snap );

static module_info_t uspeech_module_info = {

  uspeech_reset,
  uspeech_memory_map,
  uspeech_enabled_snapshot,
  uspeech_from_snapshot,
  uspeech_to_snapshot,

};

static const periph_port_t uspeech_ports[] = {
  /* Really?! This conflicts with the ULA! */
  /* ---- ---- 0111 1000 */
  { 0xffff, 0x0038, uspeech_toggle_read, uspeech_toggle_write },
  { 0xffff, 0x1000, uspeech_port_play_read, uspeech_port_play_write },
  { 0xffff, 0x3000, NULL, uspeech_port_intonation_normal },
  { 0xffff, 0x3001, NULL, uspeech_port_intonation_high },

  { 0, 0, NULL, NULL }
};

static const periph_t uspeech_periph = {
  /* .option = */ &settings_current.uspeech,
  /* .ports = */ uspeech_ports,
  /* .hard_reset = */ 1,
  /* .activate = */ NULL
};

/* Debugger events */
static const char * const event_type_string = "uspeech";
static int page_event, unpage_event;

static libspectrum_byte
uspeech_port_play_read( libspectrum_word port GCC_UNUSED,
                        libspectrum_byte *attached )
{
  if( !uspeech_active ) return 0xff;

  /* TODO: check if this value should be set to 0xff */
  *attached = 0xff;

  return sp0256_busy();
}

static void
uspeech_port_play_write( libspectrum_word port GCC_UNUSED,
                         libspectrum_byte data )
{
  if( !uspeech_active ) return;

  /* Address 0x1000 can also be accessed via I/O (read and write) */
  sp0256_play( data & 0x3f );
}

static void
uspeech_port_intonation_normal( libspectrum_word port GCC_UNUSED,
                                libspectrum_byte data GCC_UNUSED )
{
  if( !uspeech_active ) return;

  /* Address 0x3000 can also be accessed via I/O (only write) */
  sp0256_change_clock( SP0256_XTAL_NORMAL );
}

static void
uspeech_port_intonation_high( libspectrum_word port GCC_UNUSED,
                              libspectrum_byte data GCC_UNUSED )
{
  if( !uspeech_active ) return;

  /* Address 0x3001 can also be accessed via I/O (only write) */
  sp0256_change_clock( SP0256_XTAL_HIGH );
}

static void
ensure_empty_mapping( void )
{
  int i;
  libspectrum_byte *empty_chunk;

  if( empty_mapping_allocated ) return;

  empty_chunk = memory_pool_allocate_persistent( 0x3000, 1 );
  memset( empty_chunk, 0xff, 0x3000 );

  for( i = 0; i < MEMORY_PAGES_IN_12K; i++ ) {
    memory_page *page = &uspeech_empty_mapping[i];
    page->page = empty_chunk + i * MEMORY_PAGE_SIZE;
    page->offset = i * MEMORY_PAGE_SIZE;
    page->writable = 0;
    page->contended = 0;
    page->source = memory_source_none;
  }

  empty_mapping_allocated = 1;
}

static int
uspeech_init( void *context GCC_UNUSED )
{
  int i;

  module_register( &uspeech_module_info );

  uspeech_memory_source = memory_source_register( "uSpeech" );
  for( i = 0; i < MEMORY_PAGES_IN_2K; i++ )
    uspeech_memory_map_romcs[ i ].source = uspeech_memory_source;

  ensure_empty_mapping();

  periph_register( PERIPH_TYPE_USPEECH, &uspeech_periph );

  periph_register_paging_events( event_type_string, &page_event,
                                 &unpage_event );

  sp0256_set_clock( SP0256_XTAL_NORMAL );

  return 0;
}

static void
uspeech_end( void )
{
  uspeech_available = 0;
  sp0256_end();
  if( sp0256rom ) {
    libspectrum_free( sp0256rom );
  }
}

void
uspeech_register_startup( void )
{
  startup_manager_module dependencies[] = {
    STARTUP_MANAGER_MODULE_DEBUGGER,
    STARTUP_MANAGER_MODULE_MEMORY,
    STARTUP_MANAGER_MODULE_SETUID,
  };
  startup_manager_register( STARTUP_MANAGER_MODULE_USPEECH, dependencies,
                            ARRAY_SIZE( dependencies ), uspeech_init, NULL,
                            uspeech_end );
}

static int
uspeech_load_sp0256_rom( void )
{
  int error;
  char *filename;
  utils_file rom;

  filename = settings_current.rom_sp0256;
  error = utils_read_auxiliary_file( filename, &rom, UTILS_AUXILIARY_ROM );
  if( error ) {
    filename = settings_default.rom_sp0256;
    error = utils_read_auxiliary_file( filename, &rom, UTILS_AUXILIARY_ROM );
    if( error ) {
      ui_error( UI_ERROR_ERROR, "couldn't find ROM '%s'", filename );
      return -1;
    }
  }

  if( rom.length != SP0256_ROM_SIZE ) {
    ui_error( UI_ERROR_ERROR,
              "ROM '%s' is %ld bytes long; expected %ld bytes",
              filename, (unsigned long)rom.length,
              (unsigned long)SP0256_ROM_SIZE );
    utils_close_file( &rom );
    return -1;
  }

  sp0256rom = libspectrum_new( libspectrum_byte, SP0256_ROM_SIZE * 2 );

  if( !sp0256rom ) {
    utils_close_file( &rom );
    ui_error( UI_ERROR_ERROR, "SP0256: Out of memory in rdrom" );
    return -1;
  }

  memcpy( sp0256rom, rom.buffer, SP0256_ROM_SIZE );
  memcpy( sp0256rom + SP0256_ROM_SIZE, rom.buffer, SP0256_ROM_SIZE );

  utils_close_file( &rom );

  return 0;
}

static int
uspeech_sp0256_reset( void )
{
  if( !sp0256rom && uspeech_load_sp0256_rom() ) {
    return -1;
  }

  if( sp0256_reset( sp0256rom ) ) {
    return -1;
  }

  return 0;
}

static void
uspeech_reset( int hard_reset GCC_UNUSED )
{
  uspeech_active = 0;
  uspeech_available = 0;

  if( !periph_is_active( PERIPH_TYPE_USPEECH ) )
    return;

  if( machine_load_rom_bank( uspeech_memory_map_romcs, 0,
                             settings_current.rom_uspeech,
                             settings_default.rom_uspeech, 0x0800 ) ) {
    settings_current.uspeech = 0;
    periph_activate_type( PERIPH_TYPE_USPEECH, 0 );
    return;
  }

  if( uspeech_sp0256_reset() ) {
    settings_current.uspeech = 0;
    periph_activate_type( PERIPH_TYPE_USPEECH, 0 );
    return;
  }

  sp0256_set_clock( SP0256_XTAL_NORMAL );

  machine_current->ram.romcs = 0;

  uspeech_available = 1;
}

void
uspeech_toggle( void )
{
  uspeech_active = !uspeech_active;
  machine_current->ram.romcs = uspeech_active;
  machine_current->memory_map();

  if( uspeech_active )
    debugger_event( page_event );
  else
    debugger_event( unpage_event );
}

static void
uspeech_memory_map( void )
{
  if( !uspeech_active ) return;

  /* https://maziac.github.io/currah_uspeech_tests/
     says only the lower 4k should be mapped. 0x1000-0x3fff do not contain
     ZX Spectrum ROM when read */
  memory_map_romcs_2k( 0x0000, uspeech_memory_map_romcs );
  memory_map_romcs_2k( 0x0800, uspeech_memory_map_romcs );
  memory_map_romcs_4k( 0x1000, uspeech_empty_mapping );
  memory_map_romcs_8k( 0x2000, uspeech_empty_mapping + MEMORY_PAGES_IN_4K );
}

static libspectrum_byte
uspeech_toggle_read( libspectrum_word port GCC_UNUSED,
                     libspectrum_byte *attached GCC_UNUSED )
{
  if( !uspeech_available ) return 0xff;

  uspeech_toggle();

  return 0xff;
}

static void
uspeech_toggle_write( libspectrum_word port GCC_UNUSED,
                      libspectrum_byte val GCC_UNUSED )
{
  if( !uspeech_available ) return;

  uspeech_toggle();
}

void
uspeech_write( libspectrum_word address, libspectrum_byte b )
{
  switch( address ) {
  case 0x1000:
    /* This address is mirrored at 0011XXXX XXXXXXXX */
    sp0256_play( b & 0x3f );
    break;
  case 0x3000:
    /* This address is mirrored at 0011XXXX XXXXXXX0 */
    sp0256_change_clock( SP0256_XTAL_NORMAL );
    break;
  case 0x3001:
    /* This address is mirrored at 0011XXXX XXXXXXX1 */
    sp0256_change_clock( SP0256_XTAL_HIGH );
    break;
  }
}

libspectrum_byte
uspeech_busy( void )
{
  /* Thomas Busse tests claims:
     - The bits are not floating, there seems to be some deterministic behaviour.
     - Bits 0, 1 and 5 contain different values.
     - Bits 2, 3 and 4 might be equal and bit 6 and 7 might be equal.
     We only return/known bit 0 (busy), though.
  */

  return sp0256_busy();
}

int
uspeech_unittest( void )
{
  int r = 0;

  uspeech_active = 1;
  uspeech_memory_map();

  r += unittests_assert_2k_page( 0x0000, uspeech_memory_source, 0 );
  r += unittests_assert_2k_page( 0x0800, uspeech_memory_source, 0 );
  r += unittests_assert_4k_page( 0x1000, memory_source_none, 0 );
  r += unittests_assert_8k_page( 0x2000, memory_source_none, 0 );
  r += unittests_assert_16k_ram_page( 0x4000, 5 );
  r += unittests_assert_16k_ram_page( 0x8000, 2 );
  r += unittests_assert_16k_ram_page( 0xc000, 0 );

  uspeech_active = 0;
  machine_current->memory_map();

  r += unittests_paging_test_48( 2 );

  return r;
}

static void
uspeech_enabled_snapshot( libspectrum_snap *snap )
{
  settings_current.uspeech = libspectrum_snap_uspeech_active( snap );
}

static void
uspeech_from_snapshot( libspectrum_snap *snap )
{
  if( !libspectrum_snap_uspeech_active( snap ) ) return;

  if( libspectrum_snap_uspeech_paged( snap ) ) {
    uspeech_active = 0; /* Will be toggled to active next */
    uspeech_toggle();
  }
}

static void
uspeech_to_snapshot( libspectrum_snap *snap )
{
  if( !periph_is_active( PERIPH_TYPE_USPEECH ) ) return;

  libspectrum_snap_set_uspeech_active( snap, 1 );
  libspectrum_snap_set_uspeech_paged( snap, uspeech_active );
}
