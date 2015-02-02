/* uijoystick.c: Joystick emulation (using libjsw)
   Copyright (c) 2003-2004 Darren Salt, Philip Kendall

   $Id: uijoystick.c 4915 2013-04-07 05:32:09Z fredm $

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

   Darren: linux@youmustbejoking.demon.co.uk

*/

/* Usage note: build this from within a specific UI unless that UI implements
 * its own joystick support using some other library.
 * Inclusion as follows:
 * #if !defined USE_JOYSTICK || defined HAVE_JSW_H
 * # include "../uijoystick.c"
 * #else
 *  // UI-specific code implementing the following (exported) functions
 * #endif
 */

#include <config.h>

#include "input.h"
#include "uijoystick.h"

#if defined USE_JOYSTICK && defined HAVE_JSW_H

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <jsw.h>

#include <libspectrum.h>

#include "fuse.h"
#include "keyboard.h"
#include "settings.h"
#include "spectrum.h"
#include "machine.h"
#include "ui/ui.h"
#include "utils.h"

static js_data_struct jsd[2];

static int js_button_states[2][10];

static void poll_joystick( int which );
static void do_axis( int which, double position, input_key negative,
		     input_key positive );

static int
init_stick( int which, const char *const device,
	    const char *const calibration )
{
  switch( JSInit( &jsd[which], device, calibration, JSFlagNonBlocking ) ) {

  case JSSuccess:
    if( JSLoadCalibrationUNIX( &jsd[which] ) && errno != ENOENT ) {
      ui_error( UI_ERROR_ERROR,
		"failed to read calibration for joystick %i: %s", which + 1,
		strerror( errno ) );
      break;
    }

    if( jsd[which].total_axises < 2 || jsd[which].total_buttons < 1 )
    {
      ui_error( UI_ERROR_ERROR, "sorry, joystick %i (%s) is inadequate!",
		which + 1, device );
      break;
    }
    return 0;

  case JSBadValue:
    ui_error( UI_ERROR_ERROR, "failed to initialise joystick %i: %s",
	      which + 1, "invalid parameter/value");
    break;

  case JSNoAccess:

    /* FIXME: why is this commented out? */
/*
    ui_error (UI_ERROR_ERROR,
	      "failed to initialise joystick %i: %s",
	      which + 1, "cannot access device");
*/
    break;

  case JSNoBuffers:
    ui_error( UI_ERROR_ERROR, "failed to initialise joystick %i: %s",
	      which + 1, "not enough memory" );
    break;

  default:
    ui_error( UI_ERROR_ERROR, "failed to initialise joystick %i", which + 1 );
    break;

  }

  JSClose( &jsd[which] );

  return 1;
}

static
int open_joystick( int which, const char *device, const char *calibration )
{
  char path[ PATH_MAX ];

  /* If we were given an explicit device to use for this joystick, try
     only that */
  if( device && device[0] ) return init_stick( which, device, calibration );

  /* Otherwise try /dev/input/js<n> and /dev/js<n> */
  snprintf( path, PATH_MAX, "/dev/input/js%d", which );
  if( !init_stick( which, path, calibration ) ) return 0;
    
  snprintf( path, PATH_MAX, "/dev/js%d", which );
  if( !init_stick( which, path, calibration ) ) return 0;

  /* Couldn't find this joystick */
  return 1;
}

int
ui_joystick_init( void )
{
  const char *home;
  char *calibration;
  int error;
  size_t i, j;

  home = compat_get_home_path(); if( !home ) return 1;

  /* Default calibration file is ~/.joystick */
  calibration = malloc( strlen( home ) + strlen( JSDefaultCalibration ) + 2 );

  if( !calibration ) {
    ui_error( UI_ERROR_ERROR, "failed to initialise joystick: %s",
	      "not enough memory" );
    return 0;
  }

  sprintf( calibration, "%s/%s", home, JSDefaultCalibration );

  for( i = 0; i<2; i++ ) {
    for( j = 0; j<10; j++ ) {
      js_button_states[i][j] = 0;
    }
  }

  /* If we can't init the first, don't try the second */
  error = open_joystick( 0, settings_current.joystick_1, calibration );
  if( error ) return 0;

  error = open_joystick( 1, settings_current.joystick_2, calibration );
  if( error ) return 1;

  return 2;
}

void
ui_joystick_end( void )
{
  int i;
  for( i = 0; i < joysticks_supported; i++ ) JSClose( &jsd[i] );
}

void
ui_joystick_poll( void )
{
  int i;

  for( i = 0; i < joysticks_supported; i++ ) poll_joystick( i );
}

static void
poll_joystick( int which )
{
  js_data_struct *joystick;
  double position;
  int fire, buttons;
  input_event_t event;
  size_t i;

  joystick = &jsd[which];

  if( JSUpdate( joystick ) != JSGotEvent ) return;

  position = JSGetAxisCoeffNZ( joystick, 0 );
  do_axis( which, position, INPUT_JOYSTICK_LEFT, INPUT_JOYSTICK_RIGHT );

  position = JSGetAxisCoeffNZ( joystick, 1 );
  do_axis( which, position, INPUT_JOYSTICK_UP,   INPUT_JOYSTICK_DOWN  );

  event.types.joystick.which = which;

  buttons = joystick->total_buttons;
  if( buttons > 15 ) buttons = 15;	/* We support 'only' 15 fire buttons */

  for( i = 0; i < buttons; i++ ) {

    fire = JSGetButtonState( joystick, i );
    if( fire == JSButtonStateOn ) {
      event.type = INPUT_EVENT_JOYSTICK_PRESS;
    } else {
      event.type = INPUT_EVENT_JOYSTICK_RELEASE;
    }

    event.types.joystick.button = INPUT_JOYSTICK_FIRE_1 + i;

    if( js_button_states[which][i] != fire ) {
      js_button_states[which][i] = fire;
      input_event( &event );
    }

  }

}

static void
do_axis( int which, double position, input_key negative, input_key positive )
{
  input_event_t event1, event2;

  event1.types.joystick.which = event2.types.joystick.which = which;

  event1.types.joystick.button = positive;
  event2.types.joystick.button = negative;

  if( position == 0.0 ) {
    event1.type = INPUT_EVENT_JOYSTICK_RELEASE;
    event2.type = INPUT_EVENT_JOYSTICK_RELEASE;
  } else if( position > 0.0 ) {
    event1.type = INPUT_EVENT_JOYSTICK_PRESS;
    event2.type = INPUT_EVENT_JOYSTICK_RELEASE;
  } else {
    event1.type = INPUT_EVENT_JOYSTICK_RELEASE;
    event2.type = INPUT_EVENT_JOYSTICK_PRESS;
  }

  input_event( &event1 );
  input_event( &event2 );
}

#else			/* #if defined USE_JOYSTICK && defined HAVE_JSW_H */

/* No joystick library */

int
ui_joystick_init( void )
{
  return 0;
}

void
ui_joystick_end( void )
{
}

void
ui_joystick_poll( void )
{
}

#endif			/* #if defined USE_JOYSTICK && defined HAVE_JSW_H */
