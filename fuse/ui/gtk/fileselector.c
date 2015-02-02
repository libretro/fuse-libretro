/* fileselector.c: GTK+ fileselector routines
   Copyright (c) 2000-2007 Philip Kendall

   $Id: fileselector.c 4335 2011-04-02 12:41:34Z zubzero $

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

#include <gtk/gtk.h>

#include "gtkinternals.h"
#include "ui/ui.h"

static gchar *current_folder;

static char*
run_dialog( const char *title, GtkFileChooserAction action )
{
  GtkWidget *dialog;
  char *filename = NULL;
  const char *button;

  if( action == GTK_FILE_CHOOSER_ACTION_SAVE ) {
    button = GTK_STOCK_SAVE;
  } else {
    button = GTK_STOCK_OPEN;
  }

  dialog =
    gtk_file_chooser_dialog_new( title, GTK_WINDOW( gtkui_window ),
				 action, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				 button, GTK_RESPONSE_ACCEPT,
				 NULL );

  gtk_dialog_set_default_response( GTK_DIALOG( dialog ), GTK_RESPONSE_ACCEPT );

  if( current_folder )
    gtk_file_chooser_set_current_folder( GTK_FILE_CHOOSER( dialog ), current_folder );

  if( gtk_dialog_run( GTK_DIALOG( dialog ) ) == GTK_RESPONSE_ACCEPT ) {
    gchar *new_folder = gtk_file_chooser_get_current_folder( GTK_FILE_CHOOSER( dialog ) );
    if( new_folder ) {
      g_free( current_folder );
      current_folder = new_folder;
    }
    filename = gtk_file_chooser_get_filename( GTK_FILE_CHOOSER( dialog ) );
  }

  gtk_widget_destroy( dialog );

  return filename;
}


char*
ui_get_open_filename( const char *title )
{
  return run_dialog( title, GTK_FILE_CHOOSER_ACTION_OPEN );
}

char*
ui_get_save_filename( const char *title )
{
  return run_dialog( title, GTK_FILE_CHOOSER_ACTION_SAVE );
}
