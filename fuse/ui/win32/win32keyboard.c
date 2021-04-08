/* win32keyboard.c: routines for dealing with the Win32 keyboard
   Copyright (c) 2003 Marek Januszewski, Philip Kendall

   $Id: win32keyboard.c 3726 2008-07-23 20:07:22Z specu $

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
#include <stdlib.h>
#include <windows.h>

#include "display.h"
#include "fuse.h"
#include "keyboard.h"
#include "machine.h"
#include "settings.h"
#include "snapshot.h"
#include "spectrum.h"
#include "tape.h"
#include "utils.h"
#include "win32internals.h"

void
win32keyboard_keypress( WPARAM wParam, LPARAM lParam )
{
  input_key fuse_keysym;
  input_event_t fuse_event;

  fuse_keysym = keysyms_remap( wParam );

  if( fuse_keysym == INPUT_KEY_NONE ) return;

  fuse_event.type = INPUT_EVENT_KEYPRESS;
  fuse_event.types.key.native_key = fuse_keysym;
  fuse_event.types.key.spectrum_key = fuse_keysym;

  input_event( &fuse_event );
}

void
win32keyboard_keyrelease( WPARAM wParam, LPARAM lParam )
{
  input_key fuse_keysym;
  input_event_t fuse_event;

  fuse_keysym = keysyms_remap( wParam );

  if( fuse_keysym == INPUT_KEY_NONE ) return;

  fuse_event.type = INPUT_EVENT_KEYRELEASE;
  fuse_event.types.key.native_key = fuse_keysym;
  fuse_event.types.key.spectrum_key = fuse_keysym;

  input_event( &fuse_event );
}
