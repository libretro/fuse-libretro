/* ttx2000s.c: Routines for handling the TTX2000S teletext adapter
   Copyright (c) 2018 Alistair Cree

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

*/

#include <config.h>

#include <libspectrum.h>

#include "compat.h"
#include "debugger/debugger.h"
#include "infrastructure/startup_manager.h"
#include "machine.h"
#include "memory_pages.h"
#include "module.h"
#include "periph.h"
#include "settings.h"
#include "unittests/unittests.h"
#include "ttx2000s.h"
#include "ui/ui.h"
#include "z80/z80.h"
#include <string.h>

int ttx2000s_paged = 0;

#ifdef BUILD_TTX2000S
static memory_page ttx2000s_memory_map_romcs_rom[ MEMORY_PAGES_IN_8K ];
static memory_page ttx2000s_memory_map_romcs_ram[ MEMORY_PAGES_IN_8K ];
static libspectrum_byte ttx2000s_ram[2048];
/* FIXME: our pages are really 1k but the minimum is currently 2k 
   this is fixed as far as the z80 is concerned by masking off the address
   in ttx2000s_sram_read and ttx2000s_sram_write, but the debugger etc. will
   see holes in the RAM. */


static int ttx2000s_rom_memory_source;
static int ttx2000s_ram_memory_source;

compat_socket_t teletext_socket;
int ttx2000s_connected = 0;

int ttx2000s_line_counter = 0;

int ttx2000s_channel;

static void ttx2000s_write( libspectrum_word port, libspectrum_byte val );
static void ttx2000s_change_channel( int channel );
static void ttx2000s_reset( int hard_reset );
static void ttx2000s_memory_map( void );

static int field_event;
static void ttx2000s_field_event( libspectrum_dword last_tstates, int event,
                                  void *user_data );

static void ttx2000s_enabled_snapshot( libspectrum_snap *snap );
static void ttx2000s_from_snapshot( libspectrum_snap *snap );
static void ttx2000s_to_snapshot( libspectrum_snap *snap );

/* Debugger events */
static const char * const event_type_string = "ttx2000s";
static int page_event, unpage_event;

void
ttx2000s_page( void )
{
  if( ttx2000s_paged )
    return;

  ttx2000s_paged = 1;
  machine_current->ram.romcs = 1;
  machine_current->memory_map();
  debugger_event( page_event );
}

void
ttx2000s_unpage( void )
{
  if( !ttx2000s_paged )
    return;

  ttx2000s_paged = 0;
  machine_current->ram.romcs = 0;
  machine_current->memory_map();
  debugger_event( unpage_event );
}

static module_info_t ttx2000s_module_info = {
  /* .reset = */ ttx2000s_reset,
  /* .romcs = */ ttx2000s_memory_map,
  /* .snapshot_enabled = */ ttx2000s_enabled_snapshot,
  /* .snapshot_from = */ ttx2000s_from_snapshot,
  /* .snapshot_to = */ ttx2000s_to_snapshot,
};

static const periph_port_t ttx2000s_ports[] = {
  { 0x0080, 0x0000, NULL, ttx2000s_write },
  { 0, 0, NULL, NULL }
};

static const periph_t ttx2000s_periph = {
  /* .option = */ &settings_current.ttx2000s,
  /* .ports = */ ttx2000s_ports,
  /* .hard_reset = */ 1,
  /* .activate = */ NULL,
};

static int
ttx2000s_init( void *context )
{
  int i;

  teletext_socket = compat_socket_invalid;

  module_register( &ttx2000s_module_info );

  ttx2000s_rom_memory_source = memory_source_register( "TTX2000S ROM" );
  ttx2000s_ram_memory_source = memory_source_register( "TTX2000S RAM" );
  for( i = 0; i < MEMORY_PAGES_IN_8K; i++ )
    ttx2000s_memory_map_romcs_rom[i].source = ttx2000s_rom_memory_source;
  for( i = 0; i < MEMORY_PAGES_IN_2K; i++ )
    ttx2000s_memory_map_romcs_ram[i].source = ttx2000s_ram_memory_source;

  periph_register( PERIPH_TYPE_TTX2000S, &ttx2000s_periph );
  periph_register_paging_events( event_type_string, &page_event,
                                 &unpage_event );

  compat_socket_networking_init(); /* enable networking */

  field_event = event_register( ttx2000s_field_event, "TTX2000S field event" );

  return 0;
}

static void
ttx2000s_end( void )
{
  compat_socket_networking_end();
}

void
ttx2000s_register_startup( void )
{
  startup_manager_module dependencies[] = {
    STARTUP_MANAGER_MODULE_DEBUGGER,
    STARTUP_MANAGER_MODULE_MEMORY,
    STARTUP_MANAGER_MODULE_SETUID,
  };
  startup_manager_register( STARTUP_MANAGER_MODULE_TTX2000S, dependencies,
                            ARRAY_SIZE( dependencies ), ttx2000s_init, NULL,
                            ttx2000s_end );
}

static void
ttx2000s_reset( int hard_reset GCC_UNUSED )
{
  int i;

  ttx2000s_paged = 0;

  event_remove_type( field_event );
  if( !periph_is_active( PERIPH_TYPE_TTX2000S ) ) {
    if( teletext_socket != compat_socket_invalid ) {  /* close the socket */
      if( compat_socket_close( teletext_socket ) ) {
        /* what should we do if closing the socket fails? */
        ui_error( UI_ERROR_ERROR,
                  "ttx2000s: close returned unexpected errno %d: %s\n",
                  compat_socket_get_error(), compat_socket_get_strerror() );
      }
      teletext_socket = compat_socket_invalid;
      ttx2000s_connected = 0; /* disconnected */
    }
    return;
  }

  /* enable video field interrupt event */
  event_add_with_data( tstates + 2 *        /* 20ms delay */
                       machine_current->timings.processor_speed / 100,
                       field_event, 0 );

  ttx2000s_channel = -1; /* force the connection to be reset */
  ttx2000s_change_channel( 0 );

  if( machine_load_rom_bank( ttx2000s_memory_map_romcs_rom, 0,
                             settings_current.rom_ttx2000s,
                             settings_default.rom_ttx2000s, 0x2000 ) ) {
    settings_current.ttx2000s = 0;
    periph_activate_type( PERIPH_TYPE_TTX2000S, 0 );
    return;
  }

  periph_activate_type( PERIPH_TYPE_TTX2000S, 1 );

  for( i = 0; i < MEMORY_PAGES_IN_2K; i++ ) {
    struct memory_page *page = &ttx2000s_memory_map_romcs_ram[ i ];
    page->page = &ttx2000s_ram[ i * MEMORY_PAGE_SIZE ];
    page->offset = i * MEMORY_PAGE_SIZE;
    page->writable = 1;
  }

  ttx2000s_paged = 1;
  machine_current->memory_map();
  machine_current->ram.romcs = 1;
}

static void
ttx2000s_memory_map( void )
{
  if( !ttx2000s_paged ) return;

  memory_map_romcs_8k( 0x0000, ttx2000s_memory_map_romcs_rom );
  memory_map_romcs_2k( 0x2000, ttx2000s_memory_map_romcs_ram );
  memory_map_romcs_2k( 0x2800, ttx2000s_memory_map_romcs_ram );
  memory_map_romcs_2k( 0x3000, ttx2000s_memory_map_romcs_ram );
  memory_map_romcs_2k( 0x3800, ttx2000s_memory_map_romcs_ram );
}

static void
ttx2000s_write( libspectrum_word port GCC_UNUSED, libspectrum_byte val )
{
  /* bits 0 and 1 select channel preset */
  ttx2000s_change_channel( val & 0x03 );
  /* bit 2 enables automatic frequency control */
  if( val & 0x08 ) /* bit 3 pages out */
    ttx2000s_unpage();
  else
    ttx2000s_page();
}

libspectrum_byte
ttx2000s_sram_read( libspectrum_word address )
{
  /* reading from SRAM affects internal counter */
  ttx2000s_line_counter = ( address >> 6 ) & 0xF;
  return ttx2000s_ram[ address & 0x3FF ]; /* actual read from SRAM */
}

void
ttx2000s_sram_write( libspectrum_word address, libspectrum_byte b )
{
  /* writing to SRAM affects internal counter */
  ttx2000s_line_counter = ( address >> 6 ) & 0xF;
  ttx2000s_ram[ address & 0x3FF ] = b; /* actual write to SRAM */
}

static void
ttx2000s_change_channel( int channel )
{
  struct addrinfo *teletext_serv_addr;
  const char *teletext_socket_addr;
  int teletext_socket_port;
  char teletext_socket_port_str[80];

  if( channel != ttx2000s_channel ) {
    /* only reconnect if channel preset changed */
    if( teletext_socket != compat_socket_invalid ) {
      if( compat_socket_close( teletext_socket ) ) {
        /* what should we do if closing the socket fails? */
        ui_error( UI_ERROR_ERROR,
                  "ttx2000s: close returned unexpected errno %d: %s\n",
                  compat_socket_get_error(), compat_socket_get_strerror() );
      }
    }

    teletext_socket = compat_socket_invalid;
    ttx2000s_channel = channel;

    switch( channel & 3 ) {
    default:
      teletext_socket_addr = settings_current.teletext_addr_1;
      teletext_socket_port = settings_current.teletext_port_1;
      break;
    case 1:
      teletext_socket_addr = settings_current.teletext_addr_2;
      teletext_socket_port = settings_current.teletext_port_2;
      break;
    case 2:
      teletext_socket_addr = settings_current.teletext_addr_3;
      teletext_socket_port = settings_current.teletext_port_3;
      break;
    case 3:
      teletext_socket_addr = settings_current.teletext_addr_4;
      teletext_socket_port = settings_current.teletext_port_4;
      break;
    }

    snprintf( teletext_socket_port_str, sizeof( teletext_socket_port_str ),
              "%d", teletext_socket_port );

    if( getaddrinfo( teletext_socket_addr, teletext_socket_port_str, 0,
                     &teletext_serv_addr ) ){
      ui_error( UI_ERROR_ERROR,
                "ttx2000s: getaddrinfo returned %d for %s: %s\n",
                compat_socket_get_error(), teletext_socket_addr,
                compat_socket_get_strerror() );
      return;
    }

    /* create a new socket */
    teletext_socket = socket( teletext_serv_addr->ai_family, SOCK_STREAM, 0 );

    /* make it non blocking */
    if( compat_socket_blocking_mode( teletext_socket, 1 ) ) {
      /* what should we do if it fails? */
      ui_error( UI_ERROR_ERROR,
                "ttx2000s: failed to set socket non-blocking errno %d: %s\n",
                compat_socket_get_error(), compat_socket_get_strerror() );
      return;
    }

    if( connect( teletext_socket, (compat_sockaddr *)teletext_serv_addr->ai_addr,
                 (int)teletext_serv_addr->ai_addrlen ) ) {
      errno = compat_socket_get_error();
      if( errno == COMPAT_ECONNREFUSED ) {
        /* the connection was refused */
        return;
      }

      if( errno == COMPAT_EWOULDBLOCK || errno == COMPAT_EINPROGRESS ) {
        /* we expect this as socket is non-blocking */
        ttx2000s_connected = 1; /* assume we are connected */
      } else {
        /* TODO: what should we do when there's an unexpected error? */
        ui_error( UI_ERROR_ERROR,
                  "ttx2000s: connect returned unexpected errno %d: %s\n", errno,
                  compat_socket_get_strerror() );
      }
    }
  }
}

static void
ttx2000s_field_event( libspectrum_dword last_tstates GCC_UNUSED, int event,
                      void *user_data )
{
  int bytes_read;
  int i;
  libspectrum_byte ttx2000s_socket_buffer[672];

  /* do stuff */
  if( teletext_socket != compat_socket_invalid && ttx2000s_connected ) {
    bytes_read =
      recv( teletext_socket, (char *)ttx2000s_socket_buffer, 672, 0 );
    /* packet server sends 16 lines of 42 bytes, unused lines are padded with 0x00 */
    if( bytes_read == 672 ) {
      /* 11 line syncs occur before the first teletext line */
      ttx2000s_line_counter = ( ttx2000s_line_counter + 11 ) & 0xF;
      i = 0;
      while( 1 )
      {
        if( ttx2000s_socket_buffer[i * 42] != 0 ) /* packet isn't blank */
          ttx2000s_ram[ ttx2000s_line_counter << 6 ] = 0x27; /* framing code */
        memcpy( ttx2000s_ram + (ttx2000s_line_counter << 6) + 1,
                  ttx2000s_socket_buffer + (i * 42), 42 );
        i++;
        if( ++ttx2000s_line_counter > 15 )
          break; /* ignore packets once line counter overflows */
      }
      
      /* only generate NMI when ROM is paged in and there is signal */
      if( ttx2000s_paged )
        event_add( 0, z80_nmi_event );    /* pull /NMI */
    } else if( bytes_read == -1 ) {
      errno = compat_socket_get_error();
      if( errno == COMPAT_ECONNREFUSED ) {
        /* the connection was refused */
        ttx2000s_connected = 0;
      } else if( errno == COMPAT_ENOTCONN || errno == COMPAT_EWOULDBLOCK ) {
        /* just ignore if the socket is not connected or recv would block */
      } else {
        /* TODO: what should we do when there's an unexpected error */
        ui_error( UI_ERROR_ERROR,
                  "ttx2000s: recv returned unexpected errno %d: %s\n", errno,
                  compat_socket_get_strerror() );
        ttx2000s_connected = 0; /* the connection has failed */
      }
    }
  }

  event_remove_type( field_event );
  event_add_with_data( tstates + 2 *        /* 20ms delay */
                       machine_current->timings.processor_speed / 100,
                       field_event, 0 );
}

static void
ttx2000s_enabled_snapshot( libspectrum_snap *snap )
{
  settings_current.ttx2000s = libspectrum_snap_ttx2000s_active( snap );
}

static void
ttx2000s_from_snapshot( libspectrum_snap *snap )
{
  if( !libspectrum_snap_ttx2000s_active( snap ) ) return;
}

static void
ttx2000s_to_snapshot( libspectrum_snap *snap )
{
  if( !periph_is_active( PERIPH_TYPE_TTX2000S ) ) return;

  libspectrum_snap_set_ttx2000s_active( snap, 1 );

  /* TODO: proper snapshot support */
}

#else /* #ifdef BUILD_TTX2000S */

/* No TTX2000S support */

void
ttx2000s_register_startup( void )
{
}

libspectrum_byte
ttx2000s_sram_read( libspectrum_word address )
{
  return 0xff;
}

void
ttx2000s_sram_write( libspectrum_word address, libspectrum_byte b )
{
}

#endif /* #ifdef BUILD_TTX2000S */

int
ttx2000s_unittest( void )
{
  int r = 0;

  #ifdef BUILD_TTX2000S
  ttx2000s_paged = 1;
  ttx2000s_memory_map();
  machine_current->ram.romcs = 1;

  r += unittests_assert_8k_page( 0x0000, ttx2000s_rom_memory_source, 0 );
  r += unittests_assert_2k_page( 0x2000, ttx2000s_ram_memory_source, 0 );
  r += unittests_assert_2k_page( 0x2800, ttx2000s_ram_memory_source, 0 );
  r += unittests_assert_2k_page( 0x3000, ttx2000s_ram_memory_source, 0 );
  r += unittests_assert_2k_page( 0x3800, ttx2000s_ram_memory_source, 0 );
  r += unittests_assert_16k_ram_page( 0x4000, 5 );
  r += unittests_assert_16k_ram_page( 0x8000, 2 );
  r += unittests_assert_16k_ram_page( 0xc000, 0 );

  ttx2000s_paged = 0;
  machine_current->memory_map();
  machine_current->ram.romcs = 0;

  r += unittests_paging_test_48( 2 );
  
  #endif

  return r;
}
