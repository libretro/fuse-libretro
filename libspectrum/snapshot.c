/* snapshot.c: Snapshot handling routines
   Copyright (c) 2001-2009 Philip Kendall, Darren Salt

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

#include "config.h"

#include <string.h>

#include "internals.h"

/* Some flags which may be given to libspectrum_snap_write() */
const int LIBSPECTRUM_FLAG_SNAPSHOT_NO_COMPRESSION = 1 << 0;
const int LIBSPECTRUM_FLAG_SNAPSHOT_ALWAYS_COMPRESS = 1 << 1;

/* Some flags which may be returned from libspectrum_snap_write() */
const int LIBSPECTRUM_FLAG_SNAPSHOT_MINOR_INFO_LOSS = 1 << 0;
const int LIBSPECTRUM_FLAG_SNAPSHOT_MAJOR_INFO_LOSS = 1 << 1;

/* Read in a snapshot, optionally guessing what type it is */
libspectrum_error
libspectrum_snap_read( libspectrum_snap *snap, const libspectrum_byte *buffer,
		       size_t length, libspectrum_id_t type,
		       const char *filename )
{
  libspectrum_id_t raw_type;
  libspectrum_class_t class;
  libspectrum_byte *new_buffer;
  libspectrum_error error;

  /* If we don't know what sort of file this is, make a best guess */
  if( type == LIBSPECTRUM_ID_UNKNOWN ) {
    error = libspectrum_identify_file( &type, filename, buffer, length );
    if( error ) return error;

    /* If we still can't identify it, give up */
    if( type == LIBSPECTRUM_ID_UNKNOWN ) {
      libspectrum_print_error(
        LIBSPECTRUM_ERROR_UNKNOWN,
	"libspectrum_snap_read: couldn't identify file"
      );
      return LIBSPECTRUM_ERROR_UNKNOWN;
    }
  }

  error = libspectrum_identify_class( &class, type );
  if( error ) return error;

  if( class != LIBSPECTRUM_CLASS_SNAPSHOT ) {
    libspectrum_print_error( LIBSPECTRUM_ERROR_CORRUPT,
			     "libspectrum_snap_read: not a snapshot file" );
    return LIBSPECTRUM_ERROR_CORRUPT;
  }

  /* Find out if this file needs decompression */
  new_buffer = NULL;

  error = libspectrum_identify_file_raw( &raw_type, filename, buffer, length );
  if( error ) return error;

  error = libspectrum_identify_class( &class, raw_type );
  if( error ) return error;

  if( class == LIBSPECTRUM_CLASS_COMPRESSED ) {

    size_t new_length;

    error = libspectrum_uncompress_file( &new_buffer, &new_length, NULL,
					 raw_type, buffer, length, NULL );
    if( error ) return error;
    buffer = new_buffer; length = new_length;
  }

  switch( type ) {

  case LIBSPECTRUM_ID_SNAPSHOT_PLUSD:
    error = libspectrum_plusd_read( snap, buffer, length ); break;

  case LIBSPECTRUM_ID_SNAPSHOT_SNA:
    error = internal_sna_read( snap, buffer, length ); break;

  case LIBSPECTRUM_ID_SNAPSHOT_SNP:
    error = libspectrum_snp_read( snap, buffer, length ); break;

  case LIBSPECTRUM_ID_SNAPSHOT_SP:
    error = libspectrum_sp_read( snap, buffer, length ); break;

  case LIBSPECTRUM_ID_SNAPSHOT_SZX:
    error = libspectrum_szx_read( snap, buffer, length ); break;

  case LIBSPECTRUM_ID_SNAPSHOT_Z80:
    error = internal_z80_read( snap, buffer, length ); break;

  case LIBSPECTRUM_ID_SNAPSHOT_ZXS:
    error = libspectrum_zxs_read( snap, buffer, length ); break;

  default:
    libspectrum_print_error( LIBSPECTRUM_ERROR_LOGIC,
			     "libspectrum_snap_read: unknown snapshot type %d",
			     type );
    libspectrum_free( new_buffer );
    return LIBSPECTRUM_ERROR_LOGIC;
  }

  libspectrum_free( new_buffer );
  return error;
}

libspectrum_error
libspectrum_snap_write( libspectrum_byte **buffer, size_t *length,
			int *out_flags, libspectrum_snap *snap,
			libspectrum_id_t type, libspectrum_creator *creator,
			int in_flags )
{
  libspectrum_byte *ptr = *buffer;
  libspectrum_buffer *new_buffer = libspectrum_buffer_alloc();
  libspectrum_error error =
    libspectrum_snap_write_buffer( new_buffer, out_flags, snap, type, creator,
                                   in_flags );
  libspectrum_buffer_append( buffer, length, &ptr, new_buffer );
  libspectrum_buffer_free( new_buffer );
  return error;
}

libspectrum_error
libspectrum_snap_write_buffer( libspectrum_buffer *buffer, int *out_flags,
                               libspectrum_snap *snap, libspectrum_id_t type,
                               libspectrum_creator *creator, int in_flags )
{
  libspectrum_class_t class;
  libspectrum_error error;

  error = libspectrum_identify_class( &class, type );
  if( error ) return error;

  if( class != LIBSPECTRUM_CLASS_SNAPSHOT ) {
    libspectrum_print_error( LIBSPECTRUM_ERROR_INVALID,
			     "libspectrum_snap_write: not a snapshot format" );
    return LIBSPECTRUM_ERROR_INVALID;
  }

  switch( type ) {

  case LIBSPECTRUM_ID_SNAPSHOT_SNA:
    error = libspectrum_sna_write( buffer, out_flags, snap, in_flags );
    break;

  case LIBSPECTRUM_ID_SNAPSHOT_SZX:
    error = libspectrum_szx_write( buffer, out_flags, snap, creator, in_flags );
    break;

  case LIBSPECTRUM_ID_SNAPSHOT_Z80:
    error = libspectrum_z80_write2( buffer, out_flags, snap, in_flags );
    break;

  default:
    libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
			     "libspectrum_snap_write: format not supported" );
    error = LIBSPECTRUM_ERROR_UNKNOWN;

  }

  return error;
}

/* Given a 48K memory dump `data', place it into the
   appropriate bits of `snap' for a 48K machine */
libspectrum_error
libspectrum_split_to_48k_pages( libspectrum_snap *snap,
				const libspectrum_byte* data )
{
  libspectrum_byte *buffer[3];
  size_t i;

  /* If any of the three pages are already occupied, barf */
  if( libspectrum_snap_pages( snap, 5 ) ||
      libspectrum_snap_pages( snap, 2 ) ||
      libspectrum_snap_pages( snap, 0 )    ) {
    libspectrum_print_error(
      LIBSPECTRUM_ERROR_LOGIC,
      "libspectrum_split_to_48k_pages: RAM page already in use"
    );
    return LIBSPECTRUM_ERROR_LOGIC;
  }

  for( i = 0; i < 3; i++ )
    buffer[i] = libspectrum_new( libspectrum_byte, 0x4000 );

  libspectrum_snap_set_pages( snap, 5, buffer[0] );
  libspectrum_snap_set_pages( snap, 2, buffer[1] );
  libspectrum_snap_set_pages( snap, 0, buffer[2] );

  /* Finally, do the copies... */
  memcpy( libspectrum_snap_pages( snap, 5 ), &data[0x0000], 0x4000 );
  memcpy( libspectrum_snap_pages( snap, 2 ), &data[0x4000], 0x4000 );
  memcpy( libspectrum_snap_pages( snap, 0 ), &data[0x8000], 0x4000 );

  return LIBSPECTRUM_ERROR_NONE;
}
