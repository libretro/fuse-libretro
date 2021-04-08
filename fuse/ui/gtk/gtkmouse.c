/* gtkmouse.c: GTK+ routines for emulating Spectrum mice
   Copyright (c) 2004 Darren Salt

   $Id: gtkmouse.c 4723 2012-07-08 13:26:15Z fredm $

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

   E-mail: linux@youmustbejoking.demon.co.uk

*/

#include <config.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "gtkinternals.h"
#include "ui/ui.h"

/* For XWarpPointer *only* - see below */
#include <gdk/gdkx.h>
#include <X11/Xlib.h>

static GdkCursor *nullpointer = NULL;

static void
gtkmouse_reset_pointer( void )
{
  /* Ugh. GDK doesn't have its own move-pointer function :-|
   * Framebuffer users and win32 users will have to make their own
   * arrangements here.
   *
   * For Win32, use SetCursorPos() -- see sdpGtkWarpPointer() at
   * http://k3d.cvs.sourceforge.net/k3d/projects/sdplibs/sdpgtk/sdpgtkutility.cpp?view=markup
   */
  GdkWindow *window = gtk_widget_get_window( gtkui_drawing_area );

  XWarpPointer( GDK_WINDOW_XDISPLAY( window ), None, 
                GDK_WINDOW_XID( window ), 0, 0, 0, 0, 128, 128 );
}

gboolean
gtkmouse_position( GtkWidget *widget GCC_UNUSED,
                   GdkEventMotion *event, gpointer data GCC_UNUSED )
{
  if( !ui_mouse_grabbed ) return TRUE;

  if( event->x != 128 || event->y != 128 )
    gtkmouse_reset_pointer();
  ui_mouse_motion( event->x - 128, event->y - 128 );
  return TRUE;
}

gboolean
gtkmouse_button( GtkWidget *widget GCC_UNUSED, GdkEventButton *event,
		 gpointer data GCC_UNUSED )
{
  if( event->type == GDK_BUTTON_PRESS || event->type == GDK_2BUTTON_PRESS
      || event->type == GDK_3BUTTON_PRESS )
    ui_mouse_button( event->button, 1 );
  else
    ui_mouse_button( event->button, 0 );
  return TRUE;
}

int
ui_mouse_grab( int startup )
{
  GdkWindow *window;
  GdkGrabStatus status;

  if( startup ) return 0;

  if( !nullpointer ) {
    nullpointer = gdk_cursor_new( GDK_BLANK_CURSOR );
  }

  window = gtk_widget_get_window( gtkui_drawing_area );

#if !GTK_CHECK_VERSION( 3, 0, 0 )

  status = gdk_pointer_grab( window, FALSE,
                             GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK |
                             GDK_BUTTON_RELEASE_MASK,
                             window, nullpointer, GDK_CURRENT_TIME );

#else

  GdkDisplay *display;
  GdkDeviceManager *device_manager;
  GdkDevice *pointer;

  display = gdk_window_get_display( window );
  device_manager = gdk_display_get_device_manager( display );
  pointer = gdk_device_manager_get_client_pointer( device_manager );

  status = gdk_device_grab( pointer, window, GDK_OWNERSHIP_WINDOW, FALSE,
                            GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK |
                            GDK_BUTTON_RELEASE_MASK,
                            nullpointer, GDK_CURRENT_TIME );

#endif                /* #if !GTK_CHECK_VERSION( 3, 0, 0 ) */

  if( status == GDK_GRAB_SUCCESS ) {
    gtkmouse_reset_pointer();
    ui_statusbar_update( UI_STATUSBAR_ITEM_MOUSE, UI_STATUSBAR_STATE_ACTIVE );
    return 1;
  }

  ui_error( UI_ERROR_WARNING, "Mouse grab failed" );
  return 0;
}

int
ui_mouse_release( int suspend GCC_UNUSED )
{
#if !GTK_CHECK_VERSION( 3, 0, 0 )

  gdk_pointer_ungrab( GDK_CURRENT_TIME );

#else

  GdkDisplay *display;
  GdkDeviceManager *device_manager;
  GdkDevice *pointer;

  display = gtk_widget_get_display( gtkui_drawing_area );
  device_manager = gdk_display_get_device_manager( display );
  pointer = gdk_device_manager_get_client_pointer( device_manager );

  gdk_device_ungrab( pointer, GDK_CURRENT_TIME );

#endif                /* #if !GTK_CHECK_VERSION( 3, 0, 0 ) */ 

  ui_statusbar_update( UI_STATUSBAR_ITEM_MOUSE, UI_STATUSBAR_STATE_INACTIVE );
  return 0;
}
