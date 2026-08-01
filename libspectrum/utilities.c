/* utilities.c: miscellaneous utility routines
   Copyright (c) 2011 Philip Kendall

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

#include "config.h"

#ifdef HAVE_STRING_H
#include <string.h>
#endif				/* #ifdef HAVE_STRING_H */

#include "internals.h"

#define TZX_HZ 3500000

static const libspectrum_dword tstates_per_ms = TZX_HZ / 1000;

libspectrum_dword 
libspectrum_ms_to_tstates( libspectrum_dword ms )
{
  return ms * tstates_per_ms;
}

libspectrum_dword 
libspectrum_tstates_to_ms( libspectrum_dword tstates )
{
  return tstates / tstates_per_ms;
}

void
libspectrum_set_pause_ms( libspectrum_tape_block *block,
                          libspectrum_dword pause_ms )
{
  libspectrum_tape_block_set_pause( block, pause_ms );
  libspectrum_tape_block_set_pause_tstates( block,
                                        libspectrum_ms_to_tstates( pause_ms ) );
}

void
libspectrum_set_pause_tstates( libspectrum_tape_block *block,
                               libspectrum_dword pause_tstates )
{
  libspectrum_tape_block_set_pause_tstates( block, pause_tstates );
  libspectrum_tape_block_set_pause( block,
                                  libspectrum_tstates_to_ms( pause_tstates ) );
}

size_t
libspectrum_bits_to_bytes( size_t bits )
{
  return ( bits + LIBSPECTRUM_BITS_IN_BYTE - 1 ) / LIBSPECTRUM_BITS_IN_BYTE;
}

char*
libspectrum_safe_strdup( const char *src )
{
  size_t length;
  char *dest = NULL;

  if( src ) {
    length = strlen( src ) + 1;
    dest = libspectrum_new( char, length );
    memcpy( dest, src, length );
  }

  return dest;
}

/* Parse up to `count` dot-separated decimal components from `string` into
   `parts`, returning how many were found. Replaces sscanf( s, "%d.%d.%d" )
   and friends: the scanf family drags the whole formatted-input machinery
   into statically linked builds (measured at ~130K on glibc, and the
   embedded libretro targets are exactly the ones that link statically), and
   it is locale-sensitive where this is not. Components are unsigned; a
   missing or non-numeric component stops the scan. */
size_t
libspectrum_parse_dotted_version( const char *string, int *parts,
                                  size_t count )
{
  size_t found = 0;
  const char *ptr = string;

  if( !string || !parts ) return 0;

  while( found < count ) {

    int value = 0;
    int digits = 0;

    while( *ptr >= '0' && *ptr <= '9' ) {
      /* Saturate rather than overflow on an absurdly long component */
      if( value < 100000 ) value = value * 10 + ( *ptr - '0' );
      ptr++; digits++;
    }

    if( !digits ) break;

    parts[ found++ ] = value;

    if( *ptr != '.' ) break;
    ptr++;
  }

  return found;
}
