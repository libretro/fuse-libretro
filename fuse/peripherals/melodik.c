/* melodik.c: Routines for handling the Melodik interface
   Copyright (c) 2009-2011 Fredrick Meunier, Philip Kendall

   $Id: melodik.c 4926 2013-05-05 07:58:18Z sbaldovi $

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

*/

#include <config.h>

#include <libspectrum.h>

#include "ay.h"
#include "compat.h"
#include "melodik.h"
#include "module.h"
#include "periph.h"
#include "settings.h"

static void melodik_enabled_snapshot( libspectrum_snap *snap );
static void melodik_from_snapshot( libspectrum_snap *snap );
static void melodik_to_snapshot( libspectrum_snap *snap );

static module_info_t melodik_module_info = {

  NULL,
  NULL,
  melodik_enabled_snapshot,
  melodik_from_snapshot,
  melodik_to_snapshot,

};

static const periph_port_t melodik_ports[] = {
  { 0xc002, 0xc000, ay_registerport_read, ay_registerport_write },
  { 0xc002, 0x8000, NULL, ay_dataport_write },
  { 0, 0, NULL, NULL }
};

static const periph_t melodik_periph = {
  &settings_current.melodik,
  melodik_ports,
  1,
  NULL
};

static void
melodik_enabled_snapshot( libspectrum_snap *snap )
{
  if( libspectrum_snap_melodik_active( snap ) )
    settings_current.melodik = 1;
}

static void
melodik_from_snapshot( libspectrum_snap *snap )
{
  if( periph_is_active( PERIPH_TYPE_MELODIK ) )
    ay_state_from_snapshot( snap );
}

static void
melodik_to_snapshot( libspectrum_snap *snap )
{
  int active = periph_is_active( PERIPH_TYPE_MELODIK );
  libspectrum_snap_set_melodik_active( snap, active );
}

void
melodik_init( void )
{
  module_register( &melodik_module_info );
  periph_register( PERIPH_TYPE_MELODIK, &melodik_periph );
}
