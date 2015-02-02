/* ay.c: AY-8-3912 routines
   Copyright (c) 1999-2009 Philip Kendall

   $Id: ay.c 4926 2013-05-05 07:58:18Z sbaldovi $

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

static module_info_t ay_module_info = {

  ay_reset,
  NULL,
  NULL,
  ay_from_snapshot,
  ay_to_snapshot,

};

static periph_port_t ay_ports[] = {
  { 0xc002, 0xc000, ay_registerport_read, ay_registerport_write },
  { 0xc002, 0x8000, NULL, ay_dataport_write },
  { 0, 0, NULL, NULL }
};

static periph_t ay_periph = {
  NULL,
  ay_ports,
  0,
  NULL
};

static periph_port_t ay_ports_plus3[] = {
  { 0xc002, 0xc000, ay_registerport_read, ay_registerport_write },
  { 0xc002, 0x8000, ay_registerport_read, ay_dataport_write },
  { 0, 0, NULL, NULL }
};

static periph_t ay_periph_plus3 = {
  NULL,
  ay_ports_plus3,
  0,
  NULL
};

static periph_port_t ay_ports_full_decode[] = {
  { 0xffff, 0xfffd, ay_registerport_read, ay_registerport_write },
  { 0xffff, 0xbffd, NULL, ay_dataport_write },
  { 0, 0, NULL, NULL }
};

static periph_t ay_periph_full_decode = {
  NULL,
  ay_ports_full_decode,
  0,
  NULL
};

static periph_port_t ay_ports_timex[] = {
  { 0x00ff, 0x00f5, ay_registerport_read, ay_registerport_write },
  { 0x00ff, 0x00f6, NULL, ay_dataport_write },
  { 0, 0, NULL, NULL }
};

static periph_t ay_periph_timex = {
  NULL,
  ay_ports_timex,
  0,
  NULL
};

void
ay_init( void )
{
  module_register( &ay_module_info );
  periph_register( PERIPH_TYPE_AY, &ay_periph );
  periph_register( PERIPH_TYPE_AY_PLUS3, &ay_periph_plus3 );
  periph_register( PERIPH_TYPE_AY_FULL_DECODE, &ay_periph_full_decode );
  periph_register( PERIPH_TYPE_AY_TIMEX, &ay_periph_timex );
}

static void
ay_reset( int hard_reset GCC_UNUSED )
{
  ayinfo *ay = &machine_current->ay;

  ay->current_register = 0;
  memset( ay->registers, 0, sizeof( ay->registers ) );
}

/* What happens when the AY register port (traditionally 0xfffd on the 128K
   machines) is read from */
libspectrum_byte
ay_registerport_read( libspectrum_word port GCC_UNUSED, int *attached )
{
  int current;
  const libspectrum_byte port_input = 0xbf; /* always allow serial output */

  *attached = 1;

  current = machine_current->ay.current_register;

  /* The AY I/O ports return input directly from the port when in
     input mode; but in output mode, they return an AND between the
     register value and the port input. So, allow for this when
     reading R14... */

  if( current == 14 ) {
    if(machine_current->ay.registers[7] & 0x40)
      return (port_input & machine_current->ay.registers[14]);
    else
      return port_input;
  }

  /* R15 is simpler to do, as the 8912 lacks the second I/O port, and
     the input-mode input is always 0xff */
  if( current == 15 && !( machine_current->ay.registers[7] & 0x80 ) )
    return 0xff;

  /* Otherwise return register value, appropriately masked */
  return machine_current->ay.registers[ current ] & mask[ current ];
}

/* And when it's written to */
void
ay_registerport_write( libspectrum_word port GCC_UNUSED, libspectrum_byte b )
{
  machine_current->ay.current_register = (b & 15);
}

/* What happens when the AY data port (traditionally 0xbffd on the 128K
   machines) is written to; no corresponding read function as this
   always returns 0xff */
void
ay_dataport_write( libspectrum_word port GCC_UNUSED, libspectrum_byte b )
{
  int current;

  current = machine_current->ay.current_register;

  machine_current->ay.registers[ current ] = b & mask[ current ];
  sound_ay_write( current, b, tstates );
  if( psg_recording ) psg_write_register( current, b );

  if( current == 14 ) printer_serial_write( b );
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
    sound_ay_write( i, machine_current->ay.registers[i], 0 );
  }
}

static void
ay_from_snapshot( libspectrum_snap *snap )
{
  if( machine_current->capabilities & LIBSPECTRUM_MACHINE_CAPABILITY_AY ) {
    ay_state_from_snapshot( snap );
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
}
