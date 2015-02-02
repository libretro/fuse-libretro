/* tzx_write.c: Routines for writing .tzx files
   Copyright (c) 2001-2007 Philip Kendall, Fredrick Meunier

   $Id: tzx_write.c 4441 2011-05-15 11:16:28Z fredm $

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

#include <stdio.h>
#include <string.h>

#include "tape_block.h"
#include "internals.h"

/*** Local function prototypes ***/

static void
tzx_write_rom( libspectrum_tape_block *block, libspectrum_byte **buffer,
	       libspectrum_byte **ptr, size_t *length );
static void
tzx_write_turbo( libspectrum_tape_block *block, libspectrum_byte **buffer,
		 libspectrum_byte **ptr, size_t *length );
static void
add_pure_tone_block( libspectrum_byte **buffer, libspectrum_byte **ptr,
                     size_t *length, libspectrum_dword pulse_length,
                     size_t count );
static void
tzx_write_pure_tone( libspectrum_tape_block *block, libspectrum_byte **buffer,
		     libspectrum_byte **ptr, size_t *length );
static void
tzx_write_data( libspectrum_tape_block *block, libspectrum_byte **buffer,
		libspectrum_byte **ptr, size_t *length );
static void
tzx_write_raw_data( libspectrum_tape_block *block, libspectrum_byte **buffer,
		libspectrum_byte **ptr, size_t *length );
static libspectrum_error
tzx_write_generalised_data( libspectrum_tape_block *block,
			    libspectrum_byte **buffer,
			    libspectrum_byte **ptr, size_t *length );
static void
tzx_write_pulses( libspectrum_tape_block *block, libspectrum_byte **buffer,
		  libspectrum_byte **ptr, size_t *length );
static void
tzx_write_pause( libspectrum_tape_block *block, libspectrum_byte **buffer,
		 libspectrum_byte **ptr, size_t *length );
static void
tzx_write_group_start( libspectrum_tape_block *block,
		       libspectrum_byte **buffer, libspectrum_byte **ptr,
		       size_t *length );
static void
tzx_write_jump( libspectrum_tape_block *block, libspectrum_byte **buffer,
		libspectrum_byte **ptr, size_t *length );
static void
tzx_write_loop_start( libspectrum_tape_block *block,
		      libspectrum_byte **buffer, libspectrum_byte **ptr,
		      size_t *length );
static void
tzx_write_select( libspectrum_tape_block *block, libspectrum_byte **buffer,
		  libspectrum_byte **ptr, size_t *length );
static void
tzx_write_stop( libspectrum_byte **buffer, libspectrum_byte **ptr,
		size_t *length );
static void
add_set_signal_level_block( libspectrum_byte **buffer, libspectrum_byte **ptr,
                            size_t *length, int level );
static void
tzx_write_set_signal_level( libspectrum_tape_block *block,
                            libspectrum_byte **buffer, libspectrum_byte **ptr,
                            size_t *length );
static void
tzx_write_comment( libspectrum_tape_block *block, libspectrum_byte **buffer,
		   libspectrum_byte **ptr, size_t *length );
static void
tzx_write_message( libspectrum_tape_block *block, libspectrum_byte **buffer,
		   libspectrum_byte **ptr, size_t *length );
static void
tzx_write_archive_info( libspectrum_tape_block *block,
			libspectrum_byte **buffer, libspectrum_byte **ptr,
			size_t *length );
static void
tzx_write_hardware( libspectrum_tape_block *block, libspectrum_byte **buffer,
		    libspectrum_byte **ptr, size_t *length);
static void
tzx_write_custom( libspectrum_tape_block *block, libspectrum_byte **buffer,
		  libspectrum_byte **ptr, size_t *length );
static libspectrum_error
tzx_write_rle( libspectrum_tape_block *block, libspectrum_byte **buffer,
               libspectrum_byte **ptr, size_t *length,
               libspectrum_tape *tape,
               libspectrum_tape_iterator iterator );
static void
add_pulses_block( size_t pulse_count, libspectrum_dword *lengths,
                  libspectrum_tape_block *block, libspectrum_byte **buffer,
                  libspectrum_byte **ptr, size_t *length );
static void
tzx_write_pulse_sequence( libspectrum_tape_block *block,
                          libspectrum_byte **buffer, libspectrum_byte **ptr,
                          size_t *length );
static libspectrum_error
tzx_write_data_block( libspectrum_tape_block *block, libspectrum_byte **buffer,
                      libspectrum_byte **ptr, size_t *length );

static void
tzx_write_empty_block( libspectrum_byte **buffer, libspectrum_byte **ptr,
		       size_t *length, libspectrum_tape_type id );

static void
tzx_write_bytes( libspectrum_byte **ptr, size_t length,
		 size_t length_bytes, libspectrum_byte *data );
static void
tzx_write_string( libspectrum_byte **ptr, char *string );

/*** Function definitions ***/

/* The main write function */

libspectrum_error
internal_tzx_write( libspectrum_byte **buffer, size_t *length,
		    libspectrum_tape *tape )
{
  libspectrum_error error;
  libspectrum_tape_iterator iterator;
  libspectrum_tape_block *block;
  libspectrum_byte *ptr = *buffer;

  size_t signature_length = strlen( libspectrum_tzx_signature );

  /* First, write the .tzx signature and the version numbers */
  libspectrum_make_room( buffer, signature_length + 2, &ptr, length );

  memcpy( ptr, libspectrum_tzx_signature, signature_length );
  ptr += signature_length;

  *ptr++ = 1;		/* Major version number */
  *ptr++ = 20;		/* Minor version number */

  for( block = libspectrum_tape_iterator_init( &iterator, tape );
       block;
       block = libspectrum_tape_iterator_next( &iterator )       )
  {
    switch( libspectrum_tape_block_type( block ) ) {

    case LIBSPECTRUM_TAPE_BLOCK_ROM:
      tzx_write_rom( block, buffer, &ptr, length );
      break;

    case LIBSPECTRUM_TAPE_BLOCK_TURBO:
      tzx_write_turbo( block, buffer, &ptr, length );
      break;

    case LIBSPECTRUM_TAPE_BLOCK_PURE_TONE:
      tzx_write_pure_tone( block, buffer, &ptr, length );
      break;
    case LIBSPECTRUM_TAPE_BLOCK_PULSES:
      tzx_write_pulses( block, buffer, &ptr, length );
      break;
    case LIBSPECTRUM_TAPE_BLOCK_PURE_DATA:
      tzx_write_data( block, buffer, &ptr, length );
      break;

    case LIBSPECTRUM_TAPE_BLOCK_RAW_DATA:
      tzx_write_raw_data( block, buffer, &ptr, length );
      break;

    case LIBSPECTRUM_TAPE_BLOCK_GENERALISED_DATA:
      error = tzx_write_generalised_data( block, buffer, &ptr, length );
      if( error != LIBSPECTRUM_ERROR_NONE ) { libspectrum_free( *buffer ); return error; }
      break;

    case LIBSPECTRUM_TAPE_BLOCK_PAUSE:
      tzx_write_pause( block, buffer, &ptr, length );
      break;
    case LIBSPECTRUM_TAPE_BLOCK_GROUP_START:
      tzx_write_group_start( block, buffer, &ptr, length );
      break;

    case LIBSPECTRUM_TAPE_BLOCK_GROUP_END:
      tzx_write_empty_block( buffer, &ptr, length, libspectrum_tape_block_type( block ) );
      break;
    case LIBSPECTRUM_TAPE_BLOCK_JUMP:
      tzx_write_jump( block, buffer, &ptr, length );
      break;
    case LIBSPECTRUM_TAPE_BLOCK_LOOP_START:
      tzx_write_loop_start( block, buffer, &ptr, length );
      break;
    case LIBSPECTRUM_TAPE_BLOCK_LOOP_END:
      tzx_write_empty_block( buffer, &ptr, length, libspectrum_tape_block_type( block ) );
      break;

    case LIBSPECTRUM_TAPE_BLOCK_SELECT:
      tzx_write_select( block, buffer, &ptr, length );
      break;

    case LIBSPECTRUM_TAPE_BLOCK_STOP48:
      tzx_write_stop( buffer, &ptr, length );
      break;

    case LIBSPECTRUM_TAPE_BLOCK_SET_SIGNAL_LEVEL:
      tzx_write_set_signal_level( block, buffer, &ptr, length );
      break;

    case LIBSPECTRUM_TAPE_BLOCK_COMMENT:
      tzx_write_comment( block, buffer, &ptr, length );
      break;

    case LIBSPECTRUM_TAPE_BLOCK_MESSAGE:
      tzx_write_message( block, buffer, &ptr, length );
      break;

    case LIBSPECTRUM_TAPE_BLOCK_ARCHIVE_INFO:
      tzx_write_archive_info( block, buffer, &ptr, length );
      break;

    case LIBSPECTRUM_TAPE_BLOCK_HARDWARE:
      tzx_write_hardware( block, buffer, &ptr, length );
      break;

    case LIBSPECTRUM_TAPE_BLOCK_CUSTOM:
      tzx_write_custom( block, buffer, &ptr, length );
      break;

    case LIBSPECTRUM_TAPE_BLOCK_RLE_PULSE:
      error = tzx_write_rle( block, buffer, &ptr, length, tape, iterator );
      if( error != LIBSPECTRUM_ERROR_NONE ) { libspectrum_free( *buffer ); return error; }
      break;

    case LIBSPECTRUM_TAPE_BLOCK_PULSE_SEQUENCE:
      tzx_write_pulse_sequence( block, buffer, &ptr, length );
      break;
    case LIBSPECTRUM_TAPE_BLOCK_DATA_BLOCK:
      error = tzx_write_data_block( block, buffer, &ptr, length );
      if( error != LIBSPECTRUM_ERROR_NONE ) { libspectrum_free( *buffer ); return error; }
      break;

    default:
      libspectrum_free( *buffer );
      libspectrum_print_error(
        LIBSPECTRUM_ERROR_LOGIC,
	"libspectrum_tzx_write: unknown block type 0x%02x",
	libspectrum_tape_block_type( block )
      );
      return LIBSPECTRUM_ERROR_LOGIC;
    }
  }

  (*length) = ptr - *buffer;

  return LIBSPECTRUM_ERROR_NONE;
}

static void
tzx_write_rom( libspectrum_tape_block *block, libspectrum_byte **buffer,
	       libspectrum_byte **ptr, size_t *length )
{
  /* Make room for the ID byte, the pause, the length and the actual data */
  libspectrum_make_room( buffer, 5 + libspectrum_tape_block_data_length( block ), ptr, length);

  /* Write the ID byte and the pause */
  *(*ptr)++ = LIBSPECTRUM_TAPE_BLOCK_ROM;
  libspectrum_write_word( ptr, libspectrum_tape_block_pause( block ) );

  /* Copy the data across */
  tzx_write_bytes( ptr, libspectrum_tape_block_data_length( block ), 2,
                   libspectrum_tape_block_data( block ) );
}

static void
tzx_write_turbo( libspectrum_tape_block *block, libspectrum_byte **buffer,
		 libspectrum_byte **ptr, size_t *length )
{
  /* Make room for the ID byte, the metadata and the actual data */
  libspectrum_make_room( buffer, 19 + libspectrum_tape_block_data_length( block ), ptr, length);

  /* Write the ID byte and the metadata */
  *(*ptr)++ = LIBSPECTRUM_TAPE_BLOCK_TURBO;
  libspectrum_write_word( ptr, libspectrum_tape_block_pilot_length( block ) );
  libspectrum_write_word( ptr, libspectrum_tape_block_sync1_length( block ) );
  libspectrum_write_word( ptr, libspectrum_tape_block_sync2_length( block ) );
  libspectrum_write_word( ptr, libspectrum_tape_block_bit0_length ( block ) );
  libspectrum_write_word( ptr, libspectrum_tape_block_bit1_length ( block ) );
  libspectrum_write_word( ptr, libspectrum_tape_block_pilot_pulses( block ) );
  *(*ptr)++ = libspectrum_tape_block_bits_in_last_byte( block );
  libspectrum_write_word( ptr, libspectrum_tape_block_pause       ( block ) );

  tzx_write_bytes( ptr, libspectrum_tape_block_data_length( block ), 3,
                   libspectrum_tape_block_data( block ) );
}

static void
add_pure_tone_block( libspectrum_byte **buffer, libspectrum_byte **ptr,
                     size_t *length, libspectrum_dword pulse_length,
                     size_t count )
{
  /* Make room for the ID byte and the data */
  libspectrum_make_room( buffer, 5, ptr, length );

  *(*ptr)++ = LIBSPECTRUM_TAPE_BLOCK_PURE_TONE;
  libspectrum_write_word( ptr, pulse_length );
  libspectrum_write_word( ptr, count );
}

static void
tzx_write_pure_tone( libspectrum_tape_block *block, libspectrum_byte **buffer,
		     libspectrum_byte **ptr, size_t *length )
{
  add_pure_tone_block( buffer, ptr, length,
                       libspectrum_tape_block_pulse_length( block ),
                       libspectrum_tape_block_count( block ) );
}

static void
tzx_write_pulses( libspectrum_tape_block *block, libspectrum_byte **buffer,
		  libspectrum_byte **ptr, size_t *length )
{
  size_t i, count, block_length;

  /* ID byte, count and 2 bytes for length of each pulse */
  count = libspectrum_tape_block_count( block );
  block_length = 2 + 2 * count;

  /* Make room for the ID byte, the count and the data */
  libspectrum_make_room( buffer, block_length, ptr, length );

  *(*ptr)++ = LIBSPECTRUM_TAPE_BLOCK_PULSES;
  *(*ptr)++ = count;
  for( i = 0; i < count; i++ )
    libspectrum_write_word( ptr,
			    libspectrum_tape_block_pulse_lengths( block, i ) );
}

static void
tzx_write_data( libspectrum_tape_block *block, libspectrum_byte **buffer,
		libspectrum_byte **ptr, size_t *length )
{
  size_t data_length;

  /* Make room for the ID byte, the metadata and the actual data */
  data_length = libspectrum_tape_block_data_length( block );
  libspectrum_make_room( buffer, 11 + data_length, ptr, length );

  /* Write the ID byte and the metadata */
  *(*ptr)++ = LIBSPECTRUM_TAPE_BLOCK_PURE_DATA;
  libspectrum_write_word( ptr, libspectrum_tape_block_bit0_length( block ) );
  libspectrum_write_word( ptr, libspectrum_tape_block_bit1_length( block ) );
  *(*ptr)++ = libspectrum_tape_block_bits_in_last_byte( block );
  libspectrum_write_word( ptr, libspectrum_tape_block_pause( block ) );

  tzx_write_bytes( ptr, data_length, 3, libspectrum_tape_block_data( block ) );
}

static void
tzx_write_raw_data( libspectrum_tape_block *block, libspectrum_byte **buffer,
		libspectrum_byte **ptr, size_t *length )
{
  size_t data_length;

  /* Make room for the ID byte, the metadata and the actual data */
  data_length = libspectrum_tape_block_data_length( block );
  libspectrum_make_room( buffer, 9 + data_length, ptr, length );

  /* Write the ID byte and the metadata */
  *(*ptr)++ = LIBSPECTRUM_TAPE_BLOCK_RAW_DATA;
  libspectrum_write_word( ptr, libspectrum_tape_block_bit_length( block ) );
  libspectrum_write_word( ptr, libspectrum_tape_block_pause( block ) );
  *(*ptr)++ = libspectrum_tape_block_bits_in_last_byte( block );

  tzx_write_bytes( ptr, data_length, 3, libspectrum_tape_block_data( block ) );
}

static size_t
generalised_data_length( libspectrum_tape_block *block )
{
  libspectrum_tape_generalised_data_symbol_table *table;
  size_t symbol_count;

  size_t data_length = 14;	/* Minimum if no tables or symbols present */

  table = libspectrum_tape_block_pilot_table( block );
  
  symbol_count = libspectrum_tape_generalised_data_symbol_table_symbols_in_block( table );
  if( symbol_count ) {

    size_t max_pulses = libspectrum_tape_generalised_data_symbol_table_max_pulses( table );
    size_t symbols_in_table = libspectrum_tape_generalised_data_symbol_table_symbols_in_table( table );

    data_length += ( 2 * max_pulses + 1 ) * symbols_in_table;
    data_length += 3 * symbol_count;

  }

  table = libspectrum_tape_block_data_table( block );

  symbol_count = libspectrum_tape_generalised_data_symbol_table_symbols_in_block( table );
  if( symbol_count ) {

    size_t max_pulses = libspectrum_tape_generalised_data_symbol_table_max_pulses( table );
    size_t symbols_in_table = libspectrum_tape_generalised_data_symbol_table_symbols_in_table( table );

    size_t bits_per_symbol;

    data_length += ( 2 * max_pulses + 1 ) * symbols_in_table;

    bits_per_symbol = libspectrum_tape_block_bits_per_data_symbol( block );
    
    data_length += ( ( bits_per_symbol * symbol_count ) + 7 ) / 8;

  }

  return data_length;
}

static libspectrum_error
serialise_generalised_data_table( libspectrum_byte **ptr, libspectrum_tape_generalised_data_symbol_table *table )
{
  libspectrum_dword symbols_in_block;
  libspectrum_word symbols_in_table;

  symbols_in_block = libspectrum_tape_generalised_data_symbol_table_symbols_in_block( table );

  if( !symbols_in_block ) return LIBSPECTRUM_ERROR_NONE;

  libspectrum_write_dword( ptr, symbols_in_block );
  *(*ptr)++ = libspectrum_tape_generalised_data_symbol_table_max_pulses( table );

  symbols_in_table = libspectrum_tape_generalised_data_symbol_table_symbols_in_table( table );

  if( symbols_in_table == 0 || symbols_in_table > 256 ) {
    libspectrum_print_error( LIBSPECTRUM_ERROR_INVALID, "%s: invalid number of symbols in table: %d", __func__, symbols_in_table );
    return LIBSPECTRUM_ERROR_INVALID;
  } else if( symbols_in_table == 256 ) {
    symbols_in_table = 0;
  }

  *(*ptr)++ = symbols_in_table;

  return LIBSPECTRUM_ERROR_NONE;
}

static void
serialise_generalised_data_symbols( libspectrum_byte **ptr, libspectrum_tape_generalised_data_symbol_table *table )
{
  libspectrum_word symbols_in_table = libspectrum_tape_generalised_data_symbol_table_symbols_in_table( table );
  libspectrum_byte max_pulses = libspectrum_tape_generalised_data_symbol_table_max_pulses( table );

  libspectrum_word i;
  libspectrum_byte j;

  if( !libspectrum_tape_generalised_data_symbol_table_symbols_in_block( table ) ) return;

  for( i = 0; i < symbols_in_table; i++ ) {

    libspectrum_tape_generalised_data_symbol *symbol = libspectrum_tape_generalised_data_symbol_table_symbol( table, i );

    *(*ptr)++ = libspectrum_tape_generalised_data_symbol_type( symbol );

    for( j = 0; j < max_pulses; j++ )
      libspectrum_write_word( ptr, libspectrum_tape_generalised_data_symbol_pulse( symbol, j ) );

  }
}

static libspectrum_error
tzx_write_generalised_data( libspectrum_tape_block *block,
			    libspectrum_byte **buffer, libspectrum_byte **ptr,
			    size_t *length )
{
  size_t data_length, bits_per_symbol;
  libspectrum_error error;
  libspectrum_tape_generalised_data_symbol_table *pilot_table, *data_table;
  libspectrum_dword pilot_symbol_count, data_symbol_count, i;

  data_length = generalised_data_length( block );

  libspectrum_make_room( buffer, 5 + data_length, ptr, length );

  *(*ptr)++ = LIBSPECTRUM_TAPE_BLOCK_GENERALISED_DATA;
  libspectrum_write_dword( ptr, data_length );

  libspectrum_write_word( ptr, libspectrum_tape_block_pause( block ) );

  pilot_table = libspectrum_tape_block_pilot_table( block );
  data_table = libspectrum_tape_block_data_table( block );

  error = serialise_generalised_data_table( ptr, pilot_table );
  if( error != LIBSPECTRUM_ERROR_NONE ) return error;

  error = serialise_generalised_data_table( ptr, data_table );
  if( error != LIBSPECTRUM_ERROR_NONE ) return error;

  serialise_generalised_data_symbols( ptr, pilot_table );

  pilot_symbol_count = libspectrum_tape_generalised_data_symbol_table_symbols_in_block( pilot_table );

  for( i = 0; i < pilot_symbol_count; i++ ) {
    *(*ptr)++ = libspectrum_tape_block_pilot_symbols( block, i );
    libspectrum_write_word( ptr, libspectrum_tape_block_pilot_repeats( block, i ) );
  }

  serialise_generalised_data_symbols( ptr, data_table );

  bits_per_symbol = libspectrum_tape_block_bits_per_data_symbol( block );
  data_symbol_count = libspectrum_tape_generalised_data_symbol_table_symbols_in_block( data_table );

  data_length = ( ( bits_per_symbol * data_symbol_count ) + 7 ) / 8;

  memcpy( *ptr, libspectrum_tape_block_data( block ), data_length );
  (*ptr) += data_length;

  return LIBSPECTRUM_ERROR_NONE;
}

static void
tzx_write_pause( libspectrum_tape_block *block, libspectrum_byte **buffer,
		 libspectrum_byte **ptr, size_t *length )
{
  /* High pause when represented in the TZX format is really a set signal level
     1 and then a pulse as TZX format says that all pauses are low, a don't care
     pause is a pulse too */
  if( libspectrum_tape_block_level( block ) != 0 ) {
    if( libspectrum_tape_block_level( block ) == 1 ) {
      add_set_signal_level_block( buffer, ptr, length, 1 );
    }

    add_pure_tone_block( buffer, ptr, length,
                         libspectrum_tape_block_pause_tstates( block ), 1 );

    return;
  }

  libspectrum_make_room( buffer, 3, ptr, length );
  *(*ptr)++ = LIBSPECTRUM_TAPE_BLOCK_PAUSE;
  libspectrum_write_word( ptr, libspectrum_tape_block_pause( block ) );
}

static void
tzx_write_group_start( libspectrum_tape_block *block,
		       libspectrum_byte **buffer, libspectrum_byte **ptr,
		       size_t *length )
{
  char *name; size_t name_length;

  name = libspectrum_tape_block_text( block );
  name_length = strlen( (char*)name );

  /* Make room for the ID byte, the length byte and the name */
  libspectrum_make_room( buffer, 2 + name_length, ptr, length );

  *(*ptr)++ = LIBSPECTRUM_TAPE_BLOCK_GROUP_START;
  
  tzx_write_string( ptr, name );
}

static void
tzx_write_jump( libspectrum_tape_block *block, libspectrum_byte **buffer,
		libspectrum_byte **ptr, size_t *length )
{
  int u_offset;

  /* Make room for the ID byte and the offset */
  libspectrum_make_room( buffer, 3, ptr, length );

  *(*ptr)++ = LIBSPECTRUM_TAPE_BLOCK_JUMP;

  u_offset = libspectrum_tape_block_offset( block );
  if( u_offset < 0 ) u_offset += 65536;
  libspectrum_write_word( ptr, u_offset );
}

static void
tzx_write_loop_start( libspectrum_tape_block *block, libspectrum_byte **buffer,
		      libspectrum_byte **ptr, size_t *length )
{
  libspectrum_make_room( buffer, 3, ptr, length );
  *(*ptr)++ = LIBSPECTRUM_TAPE_BLOCK_LOOP_START;
  libspectrum_write_word( ptr, libspectrum_tape_block_count( block ) );
}

static void
tzx_write_select( libspectrum_tape_block *block, libspectrum_byte **buffer,
		  libspectrum_byte **ptr, size_t *length )
{
  size_t count, total_length, i;

  /* The id byte, the total length (2 bytes), the count byte,
     and ( 2 offset bytes and 1 length byte ) per selection */
  count = libspectrum_tape_block_count( block );
  total_length = 4 + 3 * count;

  for( i = 0; i < count; i++ )
    total_length += strlen( (char*)libspectrum_tape_block_texts( block, i ) );

  libspectrum_make_room( buffer, total_length, ptr, length );

  *(*ptr)++ = LIBSPECTRUM_TAPE_BLOCK_SELECT;
  libspectrum_write_word( ptr, total_length );
  *(*ptr)++ = count;

  for( i = 0; i < count; i++ ) {
    libspectrum_write_word( ptr, libspectrum_tape_block_offsets( block, i ) );
    tzx_write_string( ptr, libspectrum_tape_block_texts( block, i ) );
  }
}

static void
tzx_write_stop( libspectrum_byte **buffer, libspectrum_byte **ptr,
		size_t *length )
{
  /* Make room for the ID byte and four length bytes */
  libspectrum_make_room( buffer, 5, ptr, length );

  *(*ptr)++ = LIBSPECTRUM_TAPE_BLOCK_STOP48;
  *(*ptr)++ = '\0'; *(*ptr)++ = '\0'; *(*ptr)++ = '\0'; *(*ptr)++ = '\0';
}

static void
add_set_signal_level_block( libspectrum_byte **buffer, libspectrum_byte **ptr,
                            size_t *length, int level )
{
  /* Make room for the ID byte, four length bytes and the level byte */
  libspectrum_make_room( buffer, 6, ptr, length );

  *(*ptr)++ = LIBSPECTRUM_TAPE_BLOCK_SET_SIGNAL_LEVEL;

  libspectrum_write_dword( ptr, 1 );
  *(*ptr)++ = level;
}

static void
tzx_write_set_signal_level( libspectrum_tape_block *block,
                            libspectrum_byte **buffer, libspectrum_byte **ptr,
                            size_t *length )
{
  add_set_signal_level_block( buffer, ptr, length,
                              libspectrum_tape_block_level( block ) );
}

static void
tzx_write_comment( libspectrum_tape_block *block, libspectrum_byte **buffer,
		   libspectrum_byte **ptr, size_t *length )
{
  char *comment; size_t comment_length;

  comment = libspectrum_tape_block_text( block );
  comment_length = strlen( (char*)comment );

  /* Make room for the ID byte, the length byte and the text */
  libspectrum_make_room( buffer, 2 + comment_length, ptr, length );

  *(*ptr)++ = LIBSPECTRUM_TAPE_BLOCK_COMMENT;

  tzx_write_string( ptr, comment );
}

static void
tzx_write_message( libspectrum_tape_block *block, libspectrum_byte **buffer,
		   libspectrum_byte **ptr, size_t *length )
{
  char *message; size_t text_length;

  message = libspectrum_tape_block_text( block );
  text_length = strlen( (char*)message );

  /* Make room for the ID byte, the time byte, length byte and the text */
  libspectrum_make_room( buffer, 3 + text_length, ptr, length );

  *(*ptr)++ = LIBSPECTRUM_TAPE_BLOCK_MESSAGE;
  *(*ptr)++ = libspectrum_tape_block_pause( block );

  tzx_write_string( ptr, message );
}

static void
tzx_write_archive_info( libspectrum_tape_block *block,
			libspectrum_byte **buffer, libspectrum_byte **ptr,
			size_t *length )
{
  size_t i, count, total_length;

  count = libspectrum_tape_block_count( block );

  /* 1 count byte, 2 bytes (ID and length) for every string */
  total_length = 1 + 2 * count;
  /* And then the length of all the strings */
  for( i = 0; i < count; i++ )
    total_length += strlen( (char*)libspectrum_tape_block_texts( block, i ) );

  /* Make room for all that, along with ID byte and two bytes storing the
     total length */
  libspectrum_make_room( buffer, total_length + 3, ptr, length );

  /* Write out the metadata */
  *(*ptr)++ = LIBSPECTRUM_TAPE_BLOCK_ARCHIVE_INFO;
  libspectrum_write_word( ptr, total_length );
  *(*ptr)++ = count;

  /* And the strings */
  for( i = 0; i < count; i++ ) {
    *(*ptr)++ = libspectrum_tape_block_ids( block, i );
    tzx_write_string( ptr, libspectrum_tape_block_texts( block, i ) );
  }
}

static void
tzx_write_hardware( libspectrum_tape_block *block, libspectrum_byte **buffer,
		    libspectrum_byte **ptr, size_t *length )
{
  size_t i, count;

  /* We need one ID byte, one count byte and then three bytes for every
     entry */
  count = libspectrum_tape_block_count( block );
  libspectrum_make_room( buffer, 2 + 3 * count, ptr, length );

  /* Write out the metadata */
  *(*ptr)++ = LIBSPECTRUM_TAPE_BLOCK_HARDWARE;
  *(*ptr)++ = count;

  /* And the info */
  for( i = 0; i < count; i++ ) {
    *(*ptr)++ = libspectrum_tape_block_types( block, i );
    *(*ptr)++ = libspectrum_tape_block_ids  ( block, i );
    *(*ptr)++ = libspectrum_tape_block_values( block, i );
  }
}

static void
tzx_write_custom( libspectrum_tape_block *block, libspectrum_byte **buffer,
		  libspectrum_byte **ptr, size_t *length )
{
  size_t data_length;

  data_length = libspectrum_tape_block_data_length( block );

  /* An ID byte, 16 description bytes, 4 length bytes and the data
     itself */
  libspectrum_make_room( buffer, 1 + 16 + 4 + data_length, ptr, length );

  *(*ptr)++ = LIBSPECTRUM_TAPE_BLOCK_CUSTOM;
  memcpy( *ptr, libspectrum_tape_block_text( block ), 16 ); *ptr += 16;

  tzx_write_bytes( ptr, data_length, 4,
		   libspectrum_tape_block_data( block ) );
}

typedef struct {
  short bits_used; /* The bits used in the current byte in progress */
  short level; /* The last level output to this block */
  libspectrum_byte *tape_buffer; /* The buffer we are writing into */
  size_t tape_length; /* size of the buffer allocated so far */
  size_t length; /* size of the buffer used so far */
} rle_write_state;

static rle_write_state rle_state;

/* write a pulse of pulse_length bits into the tape_buffer */
static void
write_pulse( libspectrum_dword pulse_length )
{
  int i;
  size_t target_size = rle_state.length + pulse_length/8;

  if( rle_state.tape_length <= target_size ) {
    rle_state.tape_length = target_size * 2;
    rle_state.tape_buffer = libspectrum_realloc( rle_state.tape_buffer,
                                                 rle_state.tape_length );
  }

  for( i = pulse_length; i > 0; i-- ) {
    if( rle_state.level ) 
      *(rle_state.tape_buffer + rle_state.length) |=
        1 << (7 - rle_state.bits_used);
    rle_state.bits_used++;

    if( rle_state.bits_used == 8 ) {
      rle_state.length++;
      *(rle_state.tape_buffer + rle_state.length) = 0;
      rle_state.bits_used = 0;
    }
  }

  rle_state.level = !rle_state.level;
}

/* Convert RLE block to a TZX DRB as TZX CSW block support is limited :/ */
static libspectrum_error
tzx_write_rle( libspectrum_tape_block *block, libspectrum_byte **buffer,
               libspectrum_byte **ptr, size_t *length,
               libspectrum_tape *tape,
               libspectrum_tape_iterator iterator )
{
  libspectrum_error error;
  libspectrum_tape_block_state it;
  libspectrum_dword scale = libspectrum_tape_block_scale( block );
  libspectrum_dword pulse_tstates = 0;
  libspectrum_dword balance_tstates = 0;
  int flags = 0;

  libspectrum_tape_block *raw_block = 
    libspectrum_tape_block_alloc( LIBSPECTRUM_TAPE_BLOCK_RAW_DATA );

  libspectrum_tape_block_set_bit_length( raw_block, scale );
  libspectrum_set_pause_ms( raw_block, 0 );

  rle_state.bits_used = 0;
  rle_state.level = 0;
  rle_state.length = 0;
  rle_state.tape_length = 8192;
  rle_state.tape_buffer = libspectrum_malloc( rle_state.tape_length );

  *rle_state.tape_buffer = 0;

  it.current_block = iterator;
  error = libspectrum_tape_block_init( block, &it );
  if( error != LIBSPECTRUM_ERROR_NONE ) {
    libspectrum_free( rle_state.tape_buffer );
    libspectrum_tape_block_free( raw_block );
    return error;
  }

  while( !(flags & LIBSPECTRUM_TAPE_FLAGS_BLOCK) ) {
    libspectrum_dword pulse_length = 0;

    /* Use internal version of this that doesn't bugger up the
       external tape status */
    error = libspectrum_tape_get_next_edge_internal( &pulse_tstates, &flags,
                                                     tape, &it );
    if( error != LIBSPECTRUM_ERROR_NONE ) {
      libspectrum_free( rle_state.tape_buffer );
      libspectrum_tape_block_free( raw_block );
      return error;
    }

    balance_tstates += pulse_tstates;

    /* next set of pulses is: balance_tstates / scale; */
    pulse_length = balance_tstates / scale;
    balance_tstates = balance_tstates % scale;

    /* write pulse_length bits of the current level into the buffer */
    write_pulse( pulse_length );
  }

  if( rle_state.length || rle_state.bits_used ) {
    if( rle_state.bits_used ) {
      rle_state.length++;
    } else {
      rle_state.bits_used = 8;
    }

    error = libspectrum_tape_block_set_bits_in_last_byte( raw_block,
                                                          rle_state.bits_used );
    if( error != LIBSPECTRUM_ERROR_NONE ) {
      libspectrum_free( rle_state.tape_buffer );
      libspectrum_tape_block_free( raw_block );
      return error;
    }

    error = libspectrum_tape_block_set_data_length( raw_block,
                                                    rle_state.length );
    if( error != LIBSPECTRUM_ERROR_NONE ) {
      libspectrum_free( rle_state.tape_buffer );
      libspectrum_tape_block_free( raw_block );
      return error;
    }

    error = libspectrum_tape_block_set_data( raw_block, rle_state.tape_buffer );
    if( error != LIBSPECTRUM_ERROR_NONE ) {
      libspectrum_free( rle_state.tape_buffer );
      libspectrum_tape_block_free( raw_block );
      return error;
    }

    /* now have tzx_write_raw_data finish the job */
    tzx_write_raw_data( raw_block, buffer, ptr, length );
  } else {
    libspectrum_free( rle_state.tape_buffer );
  }

  return libspectrum_tape_block_free( raw_block );
}

static void
add_pulses_block( size_t pulse_count, libspectrum_dword *lengths,
                  libspectrum_tape_block *block, libspectrum_byte **buffer,
                  libspectrum_byte **ptr, size_t *length )
{
  libspectrum_tape_block *pulses = 
              libspectrum_tape_block_alloc( LIBSPECTRUM_TAPE_BLOCK_PULSES );

  libspectrum_tape_block_set_count( pulses, pulse_count );
  libspectrum_tape_block_set_pulse_lengths( pulses, lengths );

  tzx_write_pulses( pulses, buffer, ptr, length );

  libspectrum_tape_block_free( pulses );
}

/* Use ID 2B to set initial signal level, ID 12 Pure Tone for repeating pulses
   and ID 13 Pulse Sequence for non-repeating */
static void
tzx_write_pulse_sequence( libspectrum_tape_block *block,
                          libspectrum_byte **buffer, libspectrum_byte **ptr,
                          size_t *length )
{
  size_t count = libspectrum_tape_block_count( block );
  size_t i;
  size_t uncommitted_pulse_count = 0;
  size_t max_pulse_count = 0;
  libspectrum_dword *lengths = NULL;

  add_set_signal_level_block( buffer, ptr, length, 0 );

  for( i = 0; i<count; i++ ) {
    size_t pulse_repeats = libspectrum_tape_block_pulse_repeats( block, i );
    if( pulse_repeats > 1 ) {
      /* Close off any outstanding pulse blocks */
      if( uncommitted_pulse_count > 0 ) {
        add_pulses_block( uncommitted_pulse_count, lengths, block, buffer, ptr,
                          length );
        uncommitted_pulse_count = 0;
        lengths = NULL;
      }

      add_pure_tone_block( buffer, ptr, length,
                           libspectrum_tape_block_pulse_lengths( block, i ),
                           pulse_repeats );
    } else {
      if( uncommitted_pulse_count == max_pulse_count ) {
        max_pulse_count = uncommitted_pulse_count + 64;
        lengths =
          libspectrum_realloc( lengths, max_pulse_count * sizeof( *lengths ) );
      }
      /* Queue up pulse */
      lengths[uncommitted_pulse_count++] =
        libspectrum_tape_block_pulse_lengths( block, i );
    }
  }

  /* Close off any outstanding pulse blocks */
  if( uncommitted_pulse_count > 0 ) {
    add_pulses_block( uncommitted_pulse_count, lengths, block, buffer, ptr,
                      length );
  }
}

/* Use ID 2B to set initial signal level, ID 14 Pure Data Block and a ID 12
   Pure Tone for the tail pulse */
static libspectrum_error
tzx_write_data_block( libspectrum_tape_block *block, libspectrum_byte **buffer,
                      libspectrum_byte **ptr, size_t *length )
{
  libspectrum_tape_block *pure_data;
  size_t data_length;

  /* Pure data block can only have two identical pulses for bit 0 and bit 1 */
  if( libspectrum_tape_block_bit0_pulse_count( block ) != 2 ||
      ( libspectrum_tape_block_bit0_pulses( block, 0 ) !=
        libspectrum_tape_block_bit0_pulses( block, 1 ) ) ||
      libspectrum_tape_block_bit1_pulse_count( block ) != 2 ||
      ( libspectrum_tape_block_bit1_pulses( block, 0 ) !=
        libspectrum_tape_block_bit1_pulses( block, 1 ) ) ) {
    /* Could use generalised data block for these cases if a suitable test file
       is located, right now I haven't seen any */
    return LIBSPECTRUM_ERROR_UNKNOWN;
  }

  tzx_write_set_signal_level( block, buffer, ptr, length );

  pure_data = libspectrum_tape_block_alloc( LIBSPECTRUM_TAPE_BLOCK_PURE_DATA );

  libspectrum_tape_block_set_bit0_length( pure_data,
                              libspectrum_tape_block_bit0_pulses( block, 0 ) );
  libspectrum_tape_block_set_bit1_length( pure_data,
                              libspectrum_tape_block_bit1_pulses( block, 0 ) );
  libspectrum_tape_block_set_bits_in_last_byte( pure_data, 
                          libspectrum_tape_block_bits_in_last_byte( block ) );
  libspectrum_set_pause_tstates( pure_data, 0 );

  /* And the actual data */
  data_length = libspectrum_tape_block_data_length( block );
  libspectrum_tape_block_set_data_length( pure_data, data_length );
  libspectrum_byte *data = libspectrum_malloc( data_length * sizeof( *data ) );
  memcpy( data, libspectrum_tape_block_data( block ), data_length );
  libspectrum_tape_block_set_data( pure_data, data );

  tzx_write_data( pure_data, buffer, ptr, length );

  libspectrum_tape_block_free( pure_data );

  if( libspectrum_tape_block_tail_length( block ) ) {
    add_pure_tone_block( buffer, ptr, length,
                         libspectrum_tape_block_tail_length( block ), 1 );
  }

  return LIBSPECTRUM_ERROR_NONE;
}

static void
tzx_write_empty_block( libspectrum_byte **buffer, libspectrum_byte **ptr,
		       size_t *length, libspectrum_tape_type id )
{
  /* Make room for the ID byte */
  libspectrum_make_room( buffer, 1, ptr, length );

  *(*ptr)++ = id;
}

static void
tzx_write_bytes( libspectrum_byte **ptr, size_t length,
		 size_t length_bytes, libspectrum_byte *data )
{
  size_t i, shifter;

  /* Write out the appropriate number of length bytes */
  for( i=0, shifter = length; i<length_bytes; i++, shifter >>= 8 )
    *(*ptr)++ = shifter & 0xff;

  /* And then the actual data */
  memcpy( *ptr, data, length ); (*ptr) += length;
}

static void
tzx_write_string( libspectrum_byte **ptr, char *string )
{
  size_t i, length = strlen( (char*)string );

  tzx_write_bytes( ptr, length, 1, (libspectrum_byte*)string );

  /* Fix up line endings */
  (*ptr) -= length;
  for( i=0; i<length; i++, (*ptr)++ ) if( **ptr == '\x0a' ) **ptr = '\x0d';
}
