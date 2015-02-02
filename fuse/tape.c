/* tape.c: tape handling routines
   Copyright (c) 1999-2011 Philip Kendall, Darren Salt, Witold Filipczyk

   $Id: tape.c 4863 2013-01-27 11:28:00Z fredm $

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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libspectrum.h>

#include "debugger/debugger.h"
#include "event.h"
#include "fuse.h"
#include "loader.h"
#include "machine.h"
#include "memory.h"
#include "peripherals/ula.h"
#include "settings.h"
#include "sound.h"
#include "settings.h"
#include "snapshot.h"
#include "tape.h"
#include "timer/timer.h"
#include "ui/ui.h"
#include "utils.h"
#include "z80/z80.h"
#include "z80/z80_macros.h"

/* The current tape */
static libspectrum_tape *tape;

/* Has the current tape been modified since it was last loaded/saved? */
int tape_modified;

/* Is the emulated tape deck playing? */
int tape_playing;

/* Was the tape playing started automatically? */
static int tape_autoplay;

/* Is there a high input to the EAR socket? */
int tape_microphone;

/* Debugger events */
static const char *event_type_string = "tape";

static const char *play_event_detail_string = "play",
  *stop_event_detail_string = "stop";
static int play_event, stop_event = -1;

/* Spectrum events */
int tape_edge_event;
static int record_event;

/* Function prototypes */

static int tape_autoload( libspectrum_machine hardware );
static int trap_load_block( libspectrum_tape_block *block );
static int tape_play( int autoplay );
static int trap_check_rom( void );
static void make_name( unsigned char *name, const unsigned char *data );
static void
tape_event_record_sample( libspectrum_dword last_tstates, int type,
			  void *user_data );

/* Function definitions */

void
tape_init( void )
{
  tape = libspectrum_tape_alloc();

  play_event = debugger_event_register( event_type_string,
					play_event_detail_string );
  stop_event = debugger_event_register( event_type_string,
					stop_event_detail_string );

  tape_edge_event = event_register( tape_next_edge, "Tape edge" );
  record_event = event_register( tape_event_record_sample,
				 "Tape sample record" );

  tape_modified = 0;

  /* Don't call tape_stop() here as the UI hasn't been initialised yet,
     so we can't update the statusbar */
  tape_playing = 0;
  tape_microphone = 0;
}

void
tape_end( void )
{
  libspectrum_tape_free( tape );
  tape = NULL;
}

int tape_open( const char *filename, int autoload )
{
  utils_file file;
  int error;

  error = utils_read_file( filename, &file );
  if( error ) return error;

  error = tape_read_buffer( file.buffer, file.length, LIBSPECTRUM_ID_UNKNOWN,
			    filename, autoload );
  if( error ) { utils_close_file( &file ); return error; }

  utils_close_file( &file );

  return 0;
}

/* Use an already open tape file as the current tape */
int
tape_read_buffer( unsigned char *buffer, size_t length, libspectrum_id_t type,
		  const char *filename, int autoload )
{
  int error;

  if( libspectrum_tape_present( tape ) ) {
    error = tape_close(); if( error ) return error;
  }

  error = libspectrum_tape_read( tape, buffer, length, type, filename );
  if( error ) return error;

  tape_modified = 0;
  ui_tape_browser_update( UI_TAPE_BROWSER_NEW_TAPE, NULL );

  if( autoload ) {
    error = tape_autoload( machine_current->machine );
    if( error ) return error;
  }

  return 0;
}

/* Load a snap to start the current tape autoloading */
static int
tape_autoload( libspectrum_machine hardware )
{
  int error; const char *id;
  char filename[80];
  utils_file snap;
  libspectrum_id_t type;

  id = machine_get_id( hardware );
  if( !id ) {
    ui_error( UI_ERROR_ERROR, "Unknown machine type %d!", hardware );
    return 1;
  }

  /* Look for an autoload snap. Try .szx first, then .z80 */
  type = LIBSPECTRUM_ID_SNAPSHOT_SZX;
  snprintf( filename, sizeof(filename), "tape_%s.szx", id );
  error = utils_read_auxiliary_file( filename, &snap, UTILS_AUXILIARY_LIB );
  if( error == -1 ) {
    type = LIBSPECTRUM_ID_SNAPSHOT_Z80;
    snprintf( filename, sizeof(filename), "tape_%s.z80", id );
    error = utils_read_auxiliary_file( filename, &snap, UTILS_AUXILIARY_LIB );
  }
    
  /* If we couldn't find either, give up */
  if( error == -1 ) {
    ui_error( UI_ERROR_ERROR,
	      "Couldn't find autoload snap for machine type '%s'", id );
    return 1;
  }
  if( error ) return error;

  error = snapshot_read_buffer( snap.buffer, snap.length, type );
  if( error ) { utils_close_file( &snap ); return error; }

  utils_close_file( &snap );
    
  return 0;
}

/* Close the active tape file */
int tape_close( void )
{
  int error;
  ui_confirm_save_t confirm;

  /* If the tape has been modified, check if we want to do this */
  if( tape_modified ) {

    confirm =
      ui_confirm_save( "Tape has been modified.\nDo you want to save it?" );
    switch( confirm ) {

    case UI_CONFIRM_SAVE_SAVE:
      error = ui_tape_write(); if( error ) return error;
      break;

    case UI_CONFIRM_SAVE_DONTSAVE: break;
    case UI_CONFIRM_SAVE_CANCEL: return 1;

    }
  }

  /* Stop the tape if it's currently playing */
  if( tape_playing ) {
    error = tape_stop();
    if( error ) return error;
  }

  /* And then remove it from memory */
  error = libspectrum_tape_clear( tape );
  if( error ) return error;

  tape_modified = 0;
  ui_tape_browser_update( UI_TAPE_BROWSER_NEW_TAPE, NULL );

  return 0;
}

/* Select the nth block on the tape; 0 => 1st block */
int
tape_select_block( size_t n )
{
  int error;

  error = tape_select_block_no_update( n ); if( error ) return error;

  ui_tape_browser_update( UI_TAPE_BROWSER_SELECT_BLOCK, NULL );

  return 0;
}

/* The same, but without updating the browser display */
int
tape_select_block_no_update( size_t n )
{
  return libspectrum_tape_nth_block( tape, n );
}

/* Which block is current? */
int
tape_get_current_block( void )
{
  int n;
  libspectrum_error error;

  if( !libspectrum_tape_present( tape ) ) return -1;

  error = libspectrum_tape_position( &n, tape );
  if( error ) return -1;

  return n;
}

/* Write the current in-memory tape file out to disk */
int tape_write( const char* filename )
{
  libspectrum_id_t type;
  libspectrum_class_t class;
  libspectrum_byte *buffer; size_t length;

  int error;

  /* Work out what sort of file we want from the filename; default to
     .tzx if we couldn't guess */
  error = libspectrum_identify_file_with_class( &type, &class, filename, NULL,
						0 );
  if( error ) return error;

  if( class != LIBSPECTRUM_CLASS_TAPE || type == LIBSPECTRUM_ID_UNKNOWN )
    type = LIBSPECTRUM_ID_TAPE_TZX;

  length = 0;

  error = libspectrum_tape_write( &buffer, &length, tape, type );
  if( error != LIBSPECTRUM_ERROR_NONE ) return error;

  error = utils_write_file( filename, buffer, length );
  if( error ) { libspectrum_free( buffer ); return error; }

  tape_modified = 0;
  ui_tape_browser_update( UI_TAPE_BROWSER_MODIFIED, NULL );

  libspectrum_free( buffer );

  return 0;

}

int tape_can_autoload( void )
{
  return( settings_current.auto_load && !memory_custom_rom() );
}

/* Load the next tape block into memory; returns 0 if a block was
   loaded (even if it had an tape loading error or equivalent) or
   non-zero if there was an error at the emulator level, or tape traps
   are not active */
int tape_load_trap( void )
{
  libspectrum_tape_block *block, *next_block;
  int error;

  /* Do nothing if tape traps aren't active, or the tape is already playing */
  if( !settings_current.tape_traps || tape_playing ) return 2;

  /* Do nothing if we're not in the correct ROM */
  if( ! trap_check_rom() ) return 3;

  /* Return with error if no tape file loaded */
  if( !libspectrum_tape_present( tape ) ) return 1;

  block = libspectrum_tape_current_block( tape );

  /* Skip over any meta-data blocks */
  while( libspectrum_tape_block_metadata( block ) ) {
    block = libspectrum_tape_select_next_block( tape );
    if( !block ) return 1;
  }
  
  /* If this block isn't a ROM loader, start the block playing. After
     that, return with `error' so that we actually do whichever
     instruction it was that caused the trap to hit */
  if( libspectrum_tape_block_type( block ) != LIBSPECTRUM_TAPE_BLOCK_ROM ||
      libspectrum_tape_state( tape ) != LIBSPECTRUM_TAPE_STATE_PILOT ) {
    tape_play( 1 );
    return -1;
  }

  /* We don't properly handle the case of partial loading, so don't run
     the traps in that situation */
  if( libspectrum_tape_block_data_length( block ) != DE + 2 ) {
    tape_play( 1 );
    return -1;
  }

  /* All returns made via the RET at #05E2, except on Timex 2068 at #0136 */
  if ( machine_current->machine == LIBSPECTRUM_MACHINE_TC2068 ||
       machine_current->machine == LIBSPECTRUM_MACHINE_TS2068 ) {
    PC = 0x0136;
  } else {
    PC = 0x05e2;
  }

  error = trap_load_block( block );
  if( error ) return error;

  /* Peek at the next block. If it's a ROM block, move along, initialise
     the block, and return */
  next_block = libspectrum_tape_peek_next_block( tape );

  if( libspectrum_tape_block_type(next_block) == LIBSPECTRUM_TAPE_BLOCK_ROM ) {

    next_block = libspectrum_tape_select_next_block( tape );
    if( !next_block ) return 1;

    ui_tape_browser_update( UI_TAPE_BROWSER_SELECT_BLOCK, NULL );

    return 0;
  }

  /* If the next block isn't a ROM block, set ourselves up such that the
     next thing to occur is the pause at the end of the current block */
  libspectrum_tape_set_state( tape, LIBSPECTRUM_TAPE_STATE_PAUSE );

  return 0;
}

static int
trap_load_block( libspectrum_tape_block *block )
{
  libspectrum_byte parity, *data;
  int i = 0, length, read;

  /* On exit:
   *  A = calculated parity byte if parity checked, else 0 (CHECKME)
   *  F : if parity checked, all flags are modified
   *      else carry only is modified (FIXME)
   *  B = 0xB0 (success) or 0x00 (failure)
   *  C = 0x01 (confirmed), 0x21, 0xFE or 0xDE (CHECKME)
   * DE : decremented by number of bytes loaded or verified
   *  H = calculated parity byte or undefined
   *  L = last byte read, or 1 if none
   * IX : incremented by number of bytes loaded or verified
   * A' = unchanged on error + no flag byte, else 0x01
   * F' = 0x01      on error + no flag byte, else 0x45
   *  R = no point in altering it :-)
   * Other registers unchanged.
   */

  data = libspectrum_tape_block_data( block );
  length = libspectrum_tape_block_data_length( block );

  /* Number of bytes to load or verify */
  read = length - 1;
  if( read > DE )
    read = DE;

  /* If there's no data in the block (!), set L then error exit.
   * We don't need to alter H, IX or DE here */
  if( !length ) {
    L = F_ = 1;
    F &= ~FLAG_C;
    return 0;
  }

  i = A_; /* i = A' (flag byte) */
  AF_ = 0x0145;
  A = 0;

  /* Initialise the parity check and L to the block ID byte */
  L = parity = *data++;

  /* If the block ID byte != the flag byte, clear carry and return */
  if( parity != i )
    goto error_ret;

  /* Now set L to the *last* byte in the block */
  L = data[read - 1];

  /* Loading or verifying determined by the carry flag of F' */
  if( F_ & FLAG_C ) {
    for( i = 0; i < read; i++ ) {
      parity ^= data[i];
      writebyte_internal( IX+i, data[i] );
    }
  } else {		/* verifying */
    for( i = 0; i < read; i++ ) {
      parity ^= data[i];
      if( data[i] != readbyte_internal(IX+i) ) {
        /* Verification failure */
        L = data[i];
	goto error_ret;
      }
    }
  }

  /* At this point, i == number of bytes actually read or verified */

  /* If |DE| bytes have been read and there's more data, do the parity check */
  if( DE == i && read + 1 < length ) {
    parity ^= data[read];
    A = parity;
    CP( 1 ); /* parity check is successful if A==0 */
    B = 0xB0;
  } else {
    /* Failure to read first bit of the next byte (ref. 48K ROM, 0x5EC) */
    B = 255;
    L = 1;
    INC( B );
error_ret:
    F &= ~FLAG_C;
  }

  /* At this point, AF, AF', B and L are already modified */
  C = 1;
  H = parity;
  DE -= i;
  IX += i;
  return 0;
}

/* Append to the current tape file in memory; returns 0 if a block was
   saved or non-zero if there was an error at the emulator level, or tape
   traps are not active */
int tape_save_trap( void )
{
  libspectrum_tape_block *block;
  libspectrum_byte parity, *data;
  size_t length;

  int i;

  /* Do nothing if tape traps aren't active */
  if( !settings_current.tape_traps || tape_recording ) return 2;

  /* Check we're in the right ROM */
  if( ! trap_check_rom() ) return 3;

  block = libspectrum_tape_block_alloc( LIBSPECTRUM_TAPE_BLOCK_ROM );
  
  /* The +2 here is for the flag and parity bytes */
  length = DE + 2;
  libspectrum_tape_block_set_data_length( block, length );

  data = libspectrum_malloc( length * sizeof(libspectrum_byte) );
  libspectrum_tape_block_set_data( block, data );

  /* First, store the flag byte (and initialise the parity counter) */
  data[0] = parity = A;

  /* then the main body of the data, counting parity along the way */
  for( i=0; i<DE; i++) {
    libspectrum_byte b = readbyte_internal( IX+i );
    parity ^= b;
    data[i+1] = b;
  }

  /* And finally the parity byte */
  data[ DE+1 ] = parity;

  /* Give a 1 second pause after this block */
  libspectrum_tape_block_set_pause( block, 1000 );

  libspectrum_tape_append_block( tape, block );

  tape_modified = 1;
  ui_tape_browser_update( UI_TAPE_BROWSER_NEW_BLOCK, block );

  /* And then return via the RET at #053E, except on Timex 2068 at #00E4 */
  if ( machine_current->machine == LIBSPECTRUM_MACHINE_TC2068 ||
       machine_current->machine == LIBSPECTRUM_MACHINE_TS2068 ) {
    PC = 0x00e4;
  } else {
    PC = 0x053e;
  }

  return 0;

}

/* Check whether we're actually in the right ROM when a tape trap hit */
static int
trap_check_rom( void )
{
  if( plusd_available && plusd_active )
    return 0;		/* +D must not be active */

  if( disciple_available && disciple_active )
    return 0;		/* DISCiPLE must not be active */

  if( opus_available && opus_active )
    return 0;		/* Opus must not be active */

  if( memory_custom_rom() )
    return 0;           /* and we can't be using a custom ROM */

  switch( machine_current->machine ) {
  case LIBSPECTRUM_MACHINE_16:
  case LIBSPECTRUM_MACHINE_48:
  case LIBSPECTRUM_MACHINE_48_NTSC:
  case LIBSPECTRUM_MACHINE_SE:
  case LIBSPECTRUM_MACHINE_TC2048:
    return 1;		/* Always OK here */

  case LIBSPECTRUM_MACHINE_TC2068:
  case LIBSPECTRUM_MACHINE_TS2068:
    /* OK if we're in the EXROM (location of the tape routines) */
    return( memory_map_read[0].source == memory_source_exrom );

  case LIBSPECTRUM_MACHINE_128:
  case LIBSPECTRUM_MACHINE_PLUS2:
    /* OK if we're in ROM 1 */
    return( machine_current->ram.current_rom == 1 );

  case LIBSPECTRUM_MACHINE_PLUS2A:
  case LIBSPECTRUM_MACHINE_PLUS3:
  case LIBSPECTRUM_MACHINE_PLUS3E:
    /* OK if we're not in a 64Kb RAM configuration and we're in
       ROM 3 */
    return( ! machine_current->ram.special &&
	    machine_current->ram.current_rom == 3 );

  case LIBSPECTRUM_MACHINE_128E:
    /* OK if we're not in a 64Kb RAM configuration and we're in
       either ROM 1 or ROM 3 (which are the same) */
    return( ! machine_current->ram.special &&
	    ( machine_current->ram.current_rom == 1 ||
              machine_current->ram.current_rom == 3    ));

  case LIBSPECTRUM_MACHINE_PENT:
  case LIBSPECTRUM_MACHINE_PENT512:
  case LIBSPECTRUM_MACHINE_PENT1024:
  case LIBSPECTRUM_MACHINE_SCORP:
    /* OK if we're in ROM 1 and the Beta disk interface is not active */
    return( machine_current->ram.current_rom == 1 && !beta_active );

  case LIBSPECTRUM_MACHINE_UNKNOWN:	/* should never happen */
    ui_error( UI_ERROR_ERROR,
	      "trap_check_rom: machine type is LIBSPECTRUM_MACHINE_UNKNOWN" );
    fuse_abort();

  }

  ui_error( UI_ERROR_ERROR, "trap_check_rom: unknown machine type %d",
	    machine_current->machine );
  fuse_abort();
}

static int
tape_play( int autoplay )
{
  if( !libspectrum_tape_present( tape ) ) return 1;
  
  /* Otherwise, start the tape going */
  tape_playing = 1;
  tape_autoplay = autoplay;
  tape_microphone = 0;

  /* Update the status bar */
  ui_statusbar_update( UI_STATUSBAR_ITEM_TAPE, UI_STATUSBAR_STATE_ACTIVE );

  /* If we're fastloading, turn sound off */
  if( settings_current.fastload ) sound_pause();

  loader_tape_play();

  tape_next_edge( tstates, 0, NULL );

  debugger_event( play_event );

  return 0;
}

int
tape_do_play( int autoplay )
{
  if( !tape_playing ) {
    return tape_play( autoplay );
  } else {
    return 0;
  }
}

int tape_toggle_play( int autoplay )
{
  if( tape_playing ) {
    return tape_stop();
  } else {
    return tape_play( autoplay );
  }
}

int tape_stop( void )
{
  if( tape_playing ) {

    tape_playing = 0;
    ui_statusbar_update( UI_STATUSBAR_ITEM_TAPE, UI_STATUSBAR_STATE_INACTIVE );
    loader_tape_stop();

    /* If we were fastloading, sound was off, so turn it back on, and
       reset the speed counter */
    if( settings_current.fastload ) {
      sound_unpause();
      timer_estimate_reset();
    }

    event_remove_type( tape_edge_event );
  }

  if( stop_event != -1 ) debugger_event( stop_event );

  return 0;
}

int
tape_is_playing( void )
{
  return tape_playing;
}

int
tape_present( void )
{
  return libspectrum_tape_present( tape );
}

typedef struct
{
  libspectrum_byte *tape_buffer;
  libspectrum_dword tape_buffer_size;
  libspectrum_dword tape_buffer_used;
  int tstates_per_sample;
  int last_level;
  int last_level_count;
} tape_rec_state;

int tape_recording = 0;

static tape_rec_state rec_state;

void
tape_record_start( void )
{
  /* sample rate will be 44.1KHz */
  rec_state.tstates_per_sample =
    machine_current->timings.processor_speed/44100;

  rec_state.tape_buffer_size = 8192;
  rec_state.tape_buffer = libspectrum_malloc(rec_state.tape_buffer_size);
  rec_state.tape_buffer_used = 0;

  /* start scheduling events that record into a buffer that we
     start allocating here */
  event_add( tstates + rec_state.tstates_per_sample, record_event );

  rec_state.last_level = ula_tape_level();
  rec_state.last_level_count = 1;

  tape_recording = 1;

  /* Also want to disable other tape actions */
  ui_menu_activate( UI_MENU_ITEM_TAPE_RECORDING, 1 );
}

static int
write_rec_buffer( libspectrum_byte *tape_buffer,
                  libspectrum_dword tape_buffer_used,
                  int last_level_count )
{
  if( last_level_count <= 0xff ) {
    tape_buffer[ tape_buffer_used++ ] = last_level_count;
  } else {
    tape_buffer[ tape_buffer_used++ ] = 0;
    tape_buffer[ tape_buffer_used++ ] = ( last_level_count & 0x000000ff )      ;
    tape_buffer[ tape_buffer_used++ ] = ( last_level_count & 0x0000ff00 ) >>  8;
    tape_buffer[ tape_buffer_used++ ] = ( last_level_count & 0x00ff0000 ) >> 16;
    tape_buffer[ tape_buffer_used++ ] = ( last_level_count & 0xff000000 ) >> 24;
  }

  return tape_buffer_used;
}

void
tape_event_record_sample( libspectrum_dword last_tstates, int type,
			  void *user_data )
{
  if( rec_state.last_level != (ula_tape_level()) ) {
    /* put a sample into the recording buffer */
    rec_state.tape_buffer_used =
      write_rec_buffer( rec_state.tape_buffer,
                        rec_state.tape_buffer_used,
                        rec_state.last_level_count );

    rec_state.last_level_count = 0;
    rec_state.last_level = ula_tape_level();
    /* make sure we can still fit a dword and a flag byte in the buffer */
    if( rec_state.tape_buffer_used+5 >= rec_state.tape_buffer_size ) {
      rec_state.tape_buffer_size = rec_state.tape_buffer_size*2;
      rec_state.tape_buffer = libspectrum_realloc( rec_state.tape_buffer,
                                                   rec_state.tape_buffer_size );
    }
  }

  rec_state.last_level_count++;

  /* schedule next timer */
  event_add( last_tstates + rec_state.tstates_per_sample, record_event );
}

int
tape_record_stop( void )
{
  libspectrum_tape_block* block;

  /* put last sample into the recording buffer */
  rec_state.tape_buffer_used = write_rec_buffer( rec_state.tape_buffer,
                                                 rec_state.tape_buffer_used,
                                                 rec_state.last_level_count );

  /* stop scheduling events and turn buffer into a block and
     pop into the current tape */
  event_remove_type( record_event );

  block = libspectrum_tape_block_alloc( LIBSPECTRUM_TAPE_BLOCK_RLE_PULSE );

  libspectrum_tape_block_set_scale( block, rec_state.tstates_per_sample );
  libspectrum_tape_block_set_data_length( block, rec_state.tape_buffer_used );
  libspectrum_tape_block_set_data( block, rec_state.tape_buffer );

  libspectrum_tape_append_block( tape, block );

  rec_state.tape_buffer = NULL;
  rec_state.tape_buffer_size = 0;
  rec_state.tape_buffer_used = 0;

  tape_modified = 1;
  ui_tape_browser_update( UI_TAPE_BROWSER_NEW_BLOCK, block );

  tape_recording = 0;

  /* Also want to reenable other tape actions */
  ui_menu_activate( UI_MENU_ITEM_TAPE_RECORDING, 0 );

  return 0;
}

void
tape_next_edge( libspectrum_dword last_tstates, int type, void *user_data )
{
  libspectrum_error libspec_error;
  libspectrum_tape_block *block;

  libspectrum_dword edge_tstates;
  int flags;

  /* If the tape's not playing, just return */
  if( ! tape_playing ) return;

  /* Get the time until the next edge */
  libspec_error = libspectrum_tape_get_next_edge( &edge_tstates, &flags,
						  tape );
  if( libspec_error != LIBSPECTRUM_ERROR_NONE ) return;

  /* Invert the microphone state */
  if( edge_tstates ||
      ( flags & ( LIBSPECTRUM_TAPE_FLAGS_STOP |
                  LIBSPECTRUM_TAPE_FLAGS_LEVEL_LOW |
                  LIBSPECTRUM_TAPE_FLAGS_LEVEL_HIGH ) ) ) {

    if( flags & LIBSPECTRUM_TAPE_FLAGS_NO_EDGE ) {
      /* Do nothing */
    } else if( flags & LIBSPECTRUM_TAPE_FLAGS_LEVEL_LOW ) {
      tape_microphone = 0;
    } else if( flags & LIBSPECTRUM_TAPE_FLAGS_LEVEL_HIGH ) {
      tape_microphone = 1;
    } else {
      tape_microphone = !tape_microphone;
    }
  }

  /* If we've been requested to stop the tape, do so and then
     return without stacking another edge */
  if( ( flags & LIBSPECTRUM_TAPE_FLAGS_STOP ) ||
      ( ( flags & LIBSPECTRUM_TAPE_FLAGS_STOP48 ) && 
	( !( libspectrum_machine_capabilities( machine_current->machine ) &
	     LIBSPECTRUM_MACHINE_CAPABILITY_128_MEMORY
	   )
	)
      )
    )
  {
    tape_stop();
    return;
  }

  /* If that was the end of a block, update the browser */
  if( flags & LIBSPECTRUM_TAPE_FLAGS_BLOCK ) {

    ui_tape_browser_update( UI_TAPE_BROWSER_SELECT_BLOCK, NULL );

    /* If the tape was started automatically, tape traps are active
       and the new block is a ROM loader, stop the tape and return
       without putting another event into the queue */
    block = libspectrum_tape_current_block( tape );
    if( tape_autoplay && settings_current.tape_traps &&
	libspectrum_tape_block_type( block ) == LIBSPECTRUM_TAPE_BLOCK_ROM
      ) {
      tape_stop();
      return;
    }
  }

  /* Otherwise, put this into the event queue; remember that this edge
     should occur 'edge_tstates' after the last edge, not after the
     current time (these will be slightly different as we only process
     events between instructions). */
  event_add( last_tstates + edge_tstates, tape_edge_event );

  /* Store length flags for acceleration purposes */
  loader_set_acceleration_flags( flags );
}

/* Call a user-supplied function for every block in the current tape */
int
tape_foreach( void (*function)( libspectrum_tape_block *block,
				void *user_data),
	      void *user_data )
{
  libspectrum_tape_block *block;
  libspectrum_tape_iterator iterator;

  for( block = libspectrum_tape_iterator_init( &iterator, tape );
       block;
       block = libspectrum_tape_iterator_next( &iterator ) )
    function( block, user_data );

  return 0;
}

int
tape_block_details( char *buffer, size_t length,
		    libspectrum_tape_block *block )
{
  libspectrum_byte *data;
  const char *type; unsigned char name[11];
  int offset;

  buffer[0] = '\0';

  switch( libspectrum_tape_block_type( block ) ) {

  case LIBSPECTRUM_TAPE_BLOCK_ROM:
    /* See if this looks like a standard Spectrum header and if so
       display some extra data */
    if( libspectrum_tape_block_data_length( block ) != 19 ) goto normal;

    data = libspectrum_tape_block_data( block );

    /* Flag byte is 0x00 for headers */
    if( data[0] != 0x00 ) goto normal;

    switch( data[1] ) {
    case 0x00: type = "Program"; break;
    case 0x01: type = "Number array"; break;
    case 0x02: type = "Character array"; break;
    case 0x03: type = "Bytes"; break;
    default: goto normal;
    }
    
    make_name( name, &data[2] );

    snprintf( buffer, length, "%s: \"%s\"", type, name );

    break;

  normal:
    snprintf( buffer, length, "%lu bytes",
	      (unsigned long)libspectrum_tape_block_data_length( block ) );
    break;

  case LIBSPECTRUM_TAPE_BLOCK_TURBO:
  case LIBSPECTRUM_TAPE_BLOCK_PURE_DATA:
  case LIBSPECTRUM_TAPE_BLOCK_RAW_DATA:
  case LIBSPECTRUM_TAPE_BLOCK_DATA_BLOCK:
    snprintf( buffer, length, "%lu bytes",
	      (unsigned long)libspectrum_tape_block_data_length( block ) );
    break;

  case LIBSPECTRUM_TAPE_BLOCK_PURE_TONE:
    snprintf( buffer, length, "%lu tstates",
	      (unsigned long)libspectrum_tape_block_pulse_length( block ) );
    break;

  case LIBSPECTRUM_TAPE_BLOCK_PULSES:
  case LIBSPECTRUM_TAPE_BLOCK_PULSE_SEQUENCE:
    snprintf( buffer, length, "%lu pulses",
	      (unsigned long)libspectrum_tape_block_count( block ) );
    break;

  case LIBSPECTRUM_TAPE_BLOCK_PAUSE:
    snprintf( buffer, length, "%lu ms",
	      (unsigned long)libspectrum_tape_block_pause( block ) );
    break;

  case LIBSPECTRUM_TAPE_BLOCK_GROUP_START:
  case LIBSPECTRUM_TAPE_BLOCK_COMMENT:
  case LIBSPECTRUM_TAPE_BLOCK_MESSAGE:
  case LIBSPECTRUM_TAPE_BLOCK_CUSTOM:
    snprintf( buffer, length, "%s", libspectrum_tape_block_text( block ) );
    break;

  case LIBSPECTRUM_TAPE_BLOCK_JUMP:
    offset = libspectrum_tape_block_offset( block );
    if( offset > 0 ) {
      snprintf( buffer, length, "Forward %d blocks", offset );
    } else {
      snprintf( buffer, length, "Backward %d blocks", -offset );
    }
    break;

  case LIBSPECTRUM_TAPE_BLOCK_LOOP_START:
    snprintf( buffer, length, "%lu iterations",
	      (unsigned long)libspectrum_tape_block_count( block ) );
    break;

  case LIBSPECTRUM_TAPE_BLOCK_SELECT:
    snprintf( buffer, length, "%lu options",
	      (unsigned long)libspectrum_tape_block_count( block ) );
    break;

  case LIBSPECTRUM_TAPE_BLOCK_GENERALISED_DATA:
    snprintf( buffer, length, "%lu data symbols",
	      (unsigned long)libspectrum_tape_generalised_data_symbol_table_symbols_in_block( libspectrum_tape_block_data_table( block ) ) );
    break;

  case LIBSPECTRUM_TAPE_BLOCK_RLE_PULSE:
    /* Could do something better with this one */
    break;

  case LIBSPECTRUM_TAPE_BLOCK_GROUP_END:
  case LIBSPECTRUM_TAPE_BLOCK_LOOP_END:
  case LIBSPECTRUM_TAPE_BLOCK_STOP48:
  case LIBSPECTRUM_TAPE_BLOCK_SET_SIGNAL_LEVEL:
  case LIBSPECTRUM_TAPE_BLOCK_ARCHIVE_INFO:
  case LIBSPECTRUM_TAPE_BLOCK_HARDWARE:
  case LIBSPECTRUM_TAPE_BLOCK_CONCAT:
    break;

  }

  return 0;
}

static void
make_name( unsigned char *name, const unsigned char *data )
{
  size_t i;

  for( i = 0; i < 10; i++, name++, data++ ) {
    if( *data >= 32 && *data < 127 ) {
      *name = *data;
    } else {
      *name = '?';
    }
  }

  *name = '\0';
}
