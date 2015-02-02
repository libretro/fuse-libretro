/* input.c: generalised input events layer for Fuse
   Copyright (c) 2004 Philip Kendall

   $Id: input.c 4915 2013-04-07 05:32:09Z fredm $

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

#include "fuse.h"
#include "input.h"
#include "keyboard.h"
#include "peripherals/joystick.h"
#include "settings.h"
#include "snapshot.h"
#include "tape.h"
#include "ui/ui.h"
#include "utils.h"

static int keypress( const input_event_key_t *event );
static int keyrelease( const input_event_key_t *event );
static int do_joystick( const input_event_joystick_t *joystick_event,
			int press );

int
input_event( const input_event_t *event )
{

  switch( event->type ) {

  case INPUT_EVENT_KEYPRESS: return keypress( &( event->types.key ) );
  case INPUT_EVENT_KEYRELEASE: return keyrelease( &( event->types.key ) );

  case INPUT_EVENT_JOYSTICK_PRESS:
    return do_joystick( &( event->types.joystick ), 1 );
  case INPUT_EVENT_JOYSTICK_RELEASE:
    return do_joystick( &( event->types.joystick ), 0 );

  }

  ui_error( UI_ERROR_ERROR, "unknown input event type %d", event->type );
  return 1;

}

static int
keypress( const input_event_key_t *event )
{
  int swallow;
  const keyboard_spectrum_keys_t *ptr;

  if( ui_widget_level >= 0 ) {
    ui_widget_keyhandler( event->native_key );
    return 0;
  }

  /* Escape => ask UI to end mouse grab, return if grab ended */
  if( event->native_key == INPUT_KEY_Escape && ui_mouse_grabbed ) {
    ui_mouse_grabbed = ui_mouse_release( 0 );
    if( !ui_mouse_grabbed ) return 0;
  }

  swallow = 0;
  /* Joystick emulation via keyboard keys */
  if ( event->spectrum_key == settings_current.joystick_keyboard_up ) {
    swallow = joystick_press( JOYSTICK_KEYBOARD, JOYSTICK_BUTTON_UP   , 1 );
  }
  else if( event->spectrum_key == settings_current.joystick_keyboard_down ) {
    swallow = joystick_press( JOYSTICK_KEYBOARD, JOYSTICK_BUTTON_DOWN , 1 );
  }
  else if( event->spectrum_key == settings_current.joystick_keyboard_left ) {
    swallow = joystick_press( JOYSTICK_KEYBOARD, JOYSTICK_BUTTON_LEFT , 1 );
  }
  else if( event->spectrum_key == settings_current.joystick_keyboard_right ) {
    swallow = joystick_press( JOYSTICK_KEYBOARD, JOYSTICK_BUTTON_RIGHT, 1 );
  }
  else if( event->spectrum_key == settings_current.joystick_keyboard_fire ) {
    swallow = joystick_press( JOYSTICK_KEYBOARD, JOYSTICK_BUTTON_FIRE , 1 );
  }

  if( swallow ) return 0;

  ptr = keyboard_get_spectrum_keys( event->spectrum_key );

  if( ptr ) {
    keyboard_press( ptr->key1 );
    keyboard_press( ptr->key2 );
  }

  ui_popup_menu( event->native_key );

  return 0;
}

static int
keyrelease( const input_event_key_t *event )
{
  const keyboard_spectrum_keys_t *ptr;

  ptr = keyboard_get_spectrum_keys( event->spectrum_key );

  if( ptr ) {
    keyboard_release( ptr->key1 );
    keyboard_release( ptr->key2 );
  }

  /* Joystick emulation via keyboard keys */
  if( event->spectrum_key == settings_current.joystick_keyboard_up ) {
    joystick_press( JOYSTICK_KEYBOARD, JOYSTICK_BUTTON_UP   , 0 );
  }
  else if( event->spectrum_key == settings_current.joystick_keyboard_down ) {
    joystick_press( JOYSTICK_KEYBOARD, JOYSTICK_BUTTON_DOWN , 0 );
  }
  else if( event->spectrum_key == settings_current.joystick_keyboard_left ) {
    joystick_press( JOYSTICK_KEYBOARD, JOYSTICK_BUTTON_LEFT , 0 );
  }
  else if( event->spectrum_key == settings_current.joystick_keyboard_right ) {
    joystick_press( JOYSTICK_KEYBOARD, JOYSTICK_BUTTON_RIGHT, 0 );
  }
  else if( event->spectrum_key == settings_current.joystick_keyboard_fire ) {
    joystick_press( JOYSTICK_KEYBOARD, JOYSTICK_BUTTON_FIRE , 0 );
  }

  return 0;
}

static keyboard_key_name
get_fire_button_key( int which, input_key button )
{
  switch( which ) {

  case 0:
    switch( button ) {
    case INPUT_JOYSTICK_FIRE_1 : return settings_current.joystick_1_fire_1;
    case INPUT_JOYSTICK_FIRE_2 : return settings_current.joystick_1_fire_2;
    case INPUT_JOYSTICK_FIRE_3 : return settings_current.joystick_1_fire_3;
    case INPUT_JOYSTICK_FIRE_4 : return settings_current.joystick_1_fire_4;
    case INPUT_JOYSTICK_FIRE_5 : return settings_current.joystick_1_fire_5;
    case INPUT_JOYSTICK_FIRE_6 : return settings_current.joystick_1_fire_6;
    case INPUT_JOYSTICK_FIRE_7 : return settings_current.joystick_1_fire_7;
    case INPUT_JOYSTICK_FIRE_8 : return settings_current.joystick_1_fire_8;
    case INPUT_JOYSTICK_FIRE_9 : return settings_current.joystick_1_fire_9;
    case INPUT_JOYSTICK_FIRE_10: return settings_current.joystick_1_fire_10;
    case INPUT_JOYSTICK_FIRE_11: return settings_current.joystick_1_fire_11;
    case INPUT_JOYSTICK_FIRE_12: return settings_current.joystick_1_fire_12;
    case INPUT_JOYSTICK_FIRE_13: return settings_current.joystick_1_fire_13;
    case INPUT_JOYSTICK_FIRE_14: return settings_current.joystick_1_fire_14;
    case INPUT_JOYSTICK_FIRE_15: return settings_current.joystick_1_fire_15;
    default: break;
    }
    break;

  case 1:
    switch( button ) {
    case INPUT_JOYSTICK_FIRE_1 : return settings_current.joystick_2_fire_1;
    case INPUT_JOYSTICK_FIRE_2 : return settings_current.joystick_2_fire_2;
    case INPUT_JOYSTICK_FIRE_3 : return settings_current.joystick_2_fire_3;
    case INPUT_JOYSTICK_FIRE_4 : return settings_current.joystick_2_fire_4;
    case INPUT_JOYSTICK_FIRE_5 : return settings_current.joystick_2_fire_5;
    case INPUT_JOYSTICK_FIRE_6 : return settings_current.joystick_2_fire_6;
    case INPUT_JOYSTICK_FIRE_7 : return settings_current.joystick_2_fire_7;
    case INPUT_JOYSTICK_FIRE_8 : return settings_current.joystick_2_fire_8;
    case INPUT_JOYSTICK_FIRE_9 : return settings_current.joystick_2_fire_9;
    case INPUT_JOYSTICK_FIRE_10: return settings_current.joystick_2_fire_10;
    case INPUT_JOYSTICK_FIRE_11: return settings_current.joystick_2_fire_11;
    case INPUT_JOYSTICK_FIRE_12: return settings_current.joystick_2_fire_12;
    case INPUT_JOYSTICK_FIRE_13: return settings_current.joystick_2_fire_13;
    case INPUT_JOYSTICK_FIRE_14: return settings_current.joystick_2_fire_14;
    case INPUT_JOYSTICK_FIRE_15: return settings_current.joystick_2_fire_15;
    default: break;
    }
    break;

  }

  ui_error( UI_ERROR_ERROR, "get_fire_button_key: which = %d, button = %d",
	    which, button );
  fuse_abort();
}

static int
do_joystick( const input_event_joystick_t *joystick_event, int press )
{
  int which;

#ifdef USE_WIDGET
  if( ui_widget_level >= 0 ) {
    if( press ) ui_widget_keyhandler( joystick_event->button );
    return 0;
  }

#ifndef GEKKO /* Home button opens the menu on Wii */
  switch( joystick_event->button ) {
  case INPUT_JOYSTICK_FIRE_2:
    if( press ) ui_popup_menu( INPUT_KEY_F1 );
    break;

  default: break;		/* Remove gcc warning */

  }
#endif  /* #ifndef GEKKO */

#endif				/* #ifdef USE_WIDGET */

  which = joystick_event->which;

  if( joystick_event->button < INPUT_JOYSTICK_FIRE_1 ) {

    joystick_button button;

    switch( joystick_event->button ) {
    case INPUT_JOYSTICK_UP   : button = JOYSTICK_BUTTON_UP;    break;
    case INPUT_JOYSTICK_DOWN : button = JOYSTICK_BUTTON_DOWN;  break;
    case INPUT_JOYSTICK_LEFT : button = JOYSTICK_BUTTON_LEFT;  break;
    case INPUT_JOYSTICK_RIGHT: button = JOYSTICK_BUTTON_RIGHT; break;

    default:
      ui_error( UI_ERROR_ERROR, "do_joystick: unknown button %d",
		joystick_event->button );
      fuse_abort();
    }

    joystick_press( which, button, press );

  } else {

    keyboard_key_name key;

    key = get_fire_button_key( which, joystick_event->button );

    if( key == KEYBOARD_JOYSTICK_FIRE ) {
      joystick_press( which, JOYSTICK_BUTTON_FIRE, press );
    } else {
      if( press ) {
	keyboard_press( key );
      } else {
	keyboard_release( key );
      }
    }

  }

  return 0;
}
