/* ay.c: AY-3-8912 routines
   Copyright (c) 1999-2016 Philip Kendall
   Copyright (c) 2015 Stuart Brady

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

   E-mail: philip-fuse@shadowmagic.org.uk

*/

#include <config.h>

#include <string.h>

#include "compat.h"
#include "debugger/debugger.h"
#include "infrastructure/startup_manager.h"
#include "machine.h"
#include "module.h"
#include "periph.h"
#include "printer.h"
#include "psg.h"
#include "sound.h"

/* Unused bits in the AY registers are silently zeroed out; these masks
   accomplish this */
static const libspectrum_byte mask[ AY_REGISTERS ] = {

  0xff, 0x0f, 0xff, 0x0f, 0xff, 0x0f, 0x1f, 0xff,
  0x1f, 0x1f, 0x1f, 0xff, 0xff, 0x0f, 0xff, 0xff,

};

static void ay_reset( int hard_reset );
static void ay_from_snapshot( libspectrum_snap *snap );
static void ay_to_snapshot( libspectrum_snap *snap );
static libspectrum_dword get_current_register( void );
static void set_current_register( libspectrum_dword value );

#ifdef __LIBRETRO__
/* TurboSound: a second AY-3-8912 chip, selected via the classic NedoPC
   protocol - writing a value with its top 5 bits all set (0xF8-0xFF, never
   a valid register index since a single AY only has registers 0-15) to the
   register-select port selects the active chip instead of a register; bit 0
   of that value picks chip A (0) or chip B (1). Everything else (register
   read/write, port decode) is unchanged and keeps addressing whichever chip
   is now active. Confirmed against xpeccy's TS_NEDOPC handling. */
int ay_turbosound_enabled = 0;
static int ay_turbosound_active_chip = 0;

static int
current_ay_chip( void )
{
  return ( ay_turbosound_enabled && ay_turbosound_active_chip ) ? 1 : 0;
}

static ayinfo *
current_ay( void )
{
  return current_ay_chip() ? &machine_current->ay2 : &machine_current->ay;
}
#else
static ayinfo *
current_ay( void )
{
  return &machine_current->ay;
}
#endif

static module_info_t ay_module_info = {

  /* .reset = */ ay_reset,
  /* .romcs = */ NULL,
  /* .snapshot_enabled = */ NULL,
  /* .snapshot_from = */ ay_from_snapshot,
  /* .snapshot_to = */ ay_to_snapshot,

};

static periph_port_t ay_ports[] = {
  { 0xc002, 0xc000, ay_registerport_read, ay_registerport_write },
  { 0xc002, 0x8000, NULL, ay_dataport_write },
  { 0, 0, NULL, NULL }
};

static periph_t ay_periph = {
  /* .option = */ NULL,
  /* .ports = */ ay_ports,
  /* .hard_reset = */ 0,
  /* .actinate = */ NULL,
};

static periph_port_t ay_ports_plus3[] = {
  { 0xc002, 0xc000, ay_registerport_read, ay_registerport_write },
  { 0xc002, 0x8000, ay_registerport_read, ay_dataport_write },
  { 0, 0, NULL, NULL }
};

static periph_t ay_periph_plus3 = {
  /* .option = */ NULL,
  /* .ports = */ ay_ports_plus3,
  /* .hard_reset = */ 0,
  /* .activate = */ NULL,
};

static periph_port_t ay_ports_full_decode[] = {
  { 0xffff, 0xfffd, ay_registerport_read, ay_registerport_write },
  { 0xffff, 0xbffd, NULL, ay_dataport_write },
  { 0, 0, NULL, NULL }
};

static periph_t ay_periph_full_decode = {
  /* .option = */ NULL,
  /* .ports = */ ay_ports_full_decode,
  /* .hard_reset = */ 0,
  /* .activate = */ NULL,
};

static periph_port_t ay_ports_timex[] = {
  { 0x00ff, 0x00f5, ay_registerport_read, ay_registerport_write },
  { 0x00ff, 0x00f6, NULL, ay_dataport_write },
  { 0, 0, NULL, NULL }
};

static periph_t ay_periph_timex = {
  /* .option = */ NULL,
  /* .ports = */ ay_ports_timex,
  /* .hard_reset = */ 0,
  /* .activate = */ NULL,
};

/* Debugger system variables */
static const char * const debugger_type_string = "ay";
static const char * const current_register_detail_string = "current";

static int
ay_init( void *context )
{
  module_register( &ay_module_info );
  periph_register( PERIPH_TYPE_AY, &ay_periph );
  periph_register( PERIPH_TYPE_AY_PLUS3, &ay_periph_plus3 );
  periph_register( PERIPH_TYPE_AY_FULL_DECODE, &ay_periph_full_decode );
  periph_register( PERIPH_TYPE_AY_TIMEX, &ay_periph_timex );
  
  debugger_system_variable_register(
    debugger_type_string, current_register_detail_string, get_current_register,
    set_current_register );

  return 0;
}

void
ay_register_startup( void )
{
  startup_manager_module dependencies[] = {
    STARTUP_MANAGER_MODULE_DEBUGGER,
    STARTUP_MANAGER_MODULE_SETUID,
  };
  startup_manager_register( STARTUP_MANAGER_MODULE_AY, dependencies,
                            ARRAY_SIZE( dependencies ), ay_init, NULL, NULL );
}

static void
ay_reset( int hard_reset GCC_UNUSED )
{
  ayinfo *ay = &machine_current->ay;

  ay->current_register = 0;
  memset( ay->registers, 0, sizeof( ay->registers ) );

#ifdef __LIBRETRO__
  machine_current->ay2.current_register = 0;
  memset( machine_current->ay2.registers, 0,
          sizeof( machine_current->ay2.registers ) );
  ay_turbosound_active_chip = 0;
#endif
}

/* What happens when the AY register port (traditionally 0xfffd on the 128K
   machines) is read from */
libspectrum_byte
ay_registerport_read( libspectrum_word port GCC_UNUSED, libspectrum_byte *attached )
{
  int current;
  const libspectrum_byte port_input = 0xbf; /* always allow serial output */
  ayinfo *ay = current_ay();

  *attached = 0xff;

  current = ay->current_register;

  /* The AY I/O ports return input directly from the port when in
     input mode; but in output mode, they return an AND between the
     register value and the port input. So, allow for this when
     reading R14... */

  if( current == 14 ) {
    if(ay->registers[7] & 0x40)
      return (port_input & ay->registers[14]);
    else
      return port_input;
  }

  /* R15 is simpler to do, as the 8912 lacks the second I/O port, and
     the input-mode input is always 0xff */
  if( current == 15 && !( ay->registers[7] & 0x80 ) )
    return 0xff;

  /* Otherwise return register value, appropriately masked */
  return ay->registers[ current ] & mask[ current ];
}

/* And when it's written to */
void
ay_registerport_write( libspectrum_word port GCC_UNUSED, libspectrum_byte b )
{
#ifdef __LIBRETRO__
  if( ay_turbosound_enabled && ( b & 0xf8 ) == 0xf8 ) {
    ay_turbosound_active_chip = b & 1;
    return;
  }
#endif
  set_current_register( b );
}

/* What happens when the AY data port (traditionally 0xbffd on the 128K
   machines) is written to; no corresponding read function as this
   always returns 0xff */
void
ay_dataport_write( libspectrum_word port GCC_UNUSED, libspectrum_byte b )
{
  int current;
  ayinfo *ay = current_ay();

  current = ay->current_register;

  ay->registers[ current ] = b & mask[ current ];
#ifdef __LIBRETRO__
  sound_ay_write( current_ay_chip(), current, b, tstates );
  /* The .psg recording format and the serial-out port only cover a single
     chip; only chip A's I/O port pins are wired to anything on real
     TurboSound hardware anyway, so chip B is excluded from both. */
  if( current_ay_chip() == 0 ) {
    if( psg_recording ) psg_write_register( current, b );
    if( current == 14 ) printer_serial_write( b );
  }
#else
  sound_ay_write( current, b, tstates );
  if( psg_recording ) psg_write_register( current, b );
  if( current == 14 ) printer_serial_write( b );
#endif
}

void
ay_state_from_snapshot( libspectrum_snap *snap )
{
  size_t i;

  ay_registerport_write( 0xfffd,
                         libspectrum_snap_out_ay_registerport( snap ) );

  for( i = 0; i < AY_REGISTERS; i++ ) {
    machine_current->ay.registers[i] =
      libspectrum_snap_ay_registers( snap, i );
#ifdef __LIBRETRO__
    sound_ay_write( 0, i, machine_current->ay.registers[i], 0 );
#else
    sound_ay_write( i, machine_current->ay.registers[i], 0 );
#endif
  }
}

#ifdef __LIBRETRO__
void
ay2_state_from_snapshot( libspectrum_snap *snap )
{
  size_t i;

  machine_current->ay2.current_register =
    libspectrum_snap_out_ay2_registerport( snap ) & 0x0f;

  for( i = 0; i < AY_REGISTERS; i++ ) {
    machine_current->ay2.registers[i] =
      libspectrum_snap_ay2_registers( snap, i );
    sound_ay_write( 1, i, machine_current->ay2.registers[i], 0 );
  }
}
#endif

static void
ay_from_snapshot( libspectrum_snap *snap )
{
  if( machine_current->capabilities & LIBSPECTRUM_MACHINE_CAPABILITY_AY ) {
    ay_state_from_snapshot( snap );
#ifdef __LIBRETRO__
    /* Restored unconditionally, like chip A above, regardless of whether
       TurboSound is currently enabled: harmless if disabled (chip B's
       registers just sit unused - see current_ay_chip()), and safe for
       snapshots that predate this chunk (its accessors default to zero,
       same as any other absent chunk's fields). */
    ay2_state_from_snapshot( snap );
#endif
  }
}

static void
ay_to_snapshot( libspectrum_snap *snap )
{
  size_t i;

  libspectrum_snap_set_out_ay_registerport(
    snap, machine_current->ay.current_register
  );

  for( i = 0; i < AY_REGISTERS; i++ )
    libspectrum_snap_set_ay_registers( snap, i,
				       machine_current->ay.registers[i] );

#ifdef __LIBRETRO__
  /* Written unconditionally, like chip A above - see the write side of
     the AY2 chunk in libspectrum/szx.c for why this must not depend on
     the (live-toggleable) ay_turbosound_enabled option. */
  libspectrum_snap_set_turbosound_active( snap, 1 );
  libspectrum_snap_set_out_ay2_registerport(
    snap, machine_current->ay2.current_register
  );
  for( i = 0; i < AY_REGISTERS; i++ )
    libspectrum_snap_set_ay2_registers( snap, i,
                                        machine_current->ay2.registers[i] );
#endif
}

static libspectrum_dword
get_current_register( void )
{
  return current_ay()->current_register;
}

static void
set_current_register( libspectrum_dword value )
{
  current_ay()->current_register = (value & 0x0f);
}
