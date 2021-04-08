/* timer.c: Speed routines for Fuse
   Copyright (c) 1999-2008 Philip Kendall, Marek Januszewski, Fredrick Meunier

   $Id: timer.c 4664 2012-02-12 11:51:01Z fredm $

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

#include "event.h"
#include "movie.h"
#include "settings.h"
#include "sound.h"
#include "tape.h"
#include "timer.h"
#include "ui/ui.h"

static void timer_frame_callback_sound( libspectrum_dword last_tstates );

/*
 * Routines for estimating emulation speed
 */

/* The actual time at the end of each of the last 10 emulated seconds */
static double stored_times[10];

/* Which is the next entry in 'stored_times' that we will update */
static size_t next_stored_time;

/* The number of frames until we next update 'stored_times' */
static int frames_until_update;

/* The number of time samples we have for estimating speed */
static int samples;

float current_speed = 100.0;

static double start_time;

static const int TEN_MS = 10;

int timer_event;

static void timer_frame( libspectrum_dword last_tstates, int event GCC_UNUSED,
			 void *user_data GCC_UNUSED );

int
timer_estimate_speed( void )
{
  double current_time;

  if( frames_until_update-- ) return 0;

  current_time = timer_get_time();
  if( current_time < 0 ) return 1;

  if( samples < 10 ) {

    /* If we don't have enough data, assume we're running at the desired
       speed :-) */
    current_speed = settings_current.emulation_speed;

  } else {
    current_speed = 10 * 100 /
                      ( current_time - stored_times[ next_stored_time ] );
  }

  ui_statusbar_update_speed( current_speed );

  stored_times[ next_stored_time ] = current_time;

  next_stored_time = ( next_stored_time + 1 ) % 10;
  frames_until_update = 
    ( machine_current->timings.processor_speed /
    machine_current->timings.tstates_per_frame ) - 1;

  samples++;

  return 0;
}

int
timer_estimate_reset( void )
{
  start_time = timer_get_time(); if( start_time < 0 ) return 1;
  samples = 0;
  next_stored_time = 0;
  frames_until_update = 0;

  return 0;
}

int
timer_init( void )
{
  start_time = timer_get_time(); if( start_time < 0 ) return 1;

  timer_event = event_register( timer_frame, "Timer" );

  event_add( 0, timer_event );

  return 0;
}

void
timer_end( void )
{
  event_remove_type( timer_event );
}

#ifdef SOUND_FIFO

/* Callback-style sound based timer */
#include "sound/sfifo.h"

extern sfifo_t sound_fifo;

static void
timer_frame_callback_sound( libspectrum_dword last_tstates )
{
  for(;;) {

    /* Sleep while fifo is full */
    if( sfifo_space( &sound_fifo ) < sound_framesiz ) {
      timer_sleep( TEN_MS );
    } else {
      break;
    }

  }

  event_add( last_tstates + machine_current->timings.tstates_per_frame,
             timer_event );
}

#else                           /* #ifdef SOUND_FIFO */

/* Blocking socket-style sound based timer */
static void
timer_frame_callback_sound( libspectrum_dword last_tstates )
{
  event_add( last_tstates + machine_current->timings.tstates_per_frame,
             timer_event );
}
  
#endif                          /* #ifdef SOUND_FIFO */

static void
timer_frame( libspectrum_dword last_tstates, int event GCC_UNUSED,
	     void *user_data GCC_UNUSED )
{
  double current_time, difference;
  long tstates;

  if( sound_enabled && settings_current.sound ) {
    timer_frame_callback_sound( last_tstates );
    return;
  }

  /* If we're fastloading, just schedule another check in a frame's time
     and do nothing else */
  if( settings_current.fastload && tape_is_playing() ) {

    libspectrum_dword next_check_time =
      last_tstates + machine_current->timings.tstates_per_frame;

    event_add( next_check_time, timer_event );

  } else {

    float speed = ( settings_current.emulation_speed < 1 ?
                    1.0                                  :
                    settings_current.emulation_speed ) / 100.0;

    while( 1 ) {

      current_time = timer_get_time(); if( current_time < 0 ) return;
      difference = current_time - start_time;

      /* Sleep while we are still 10ms ahead */
      if( difference < 0 ) {
        timer_sleep( TEN_MS );
      } else {
	break;
      }

    }

    current_time = timer_get_time(); if( current_time < 0 ) return;
    difference = current_time - start_time;

    tstates = ( ( difference + TEN_MS / 1000.0 ) *
		machine_current->timings.processor_speed
		) * speed + 0.5;
  
    event_add( last_tstates + tstates, timer_event );

    start_time = current_time + TEN_MS / 1000.0;
  }
}
