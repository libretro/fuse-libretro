/* binary.c: GTK routines to load/save chunks of binary data
   Copyright (c) 2003-2013 Philip Kendall
   Copyright (c) 2015 Stuart Brady

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

#include <gtk/gtk.h>
#include <libspectrum.h>

#include "fuse.h"
#include "gtkinternals.h"
#include "memory_pages.h"
#include "menu.h"
#include "ui/ui.h"
#include "utils.h"

struct binary_info {

  char *filename;
  utils_file file;

  GCallback activate_data;
  GCallback change_filename;

  GtkWidget *dialog;
  GtkWidget *filename_widget, *start_widget, *length_widget;

  long start;
  long length;
};

static struct binary_info save_info = { 0 };
static struct binary_info load_info = { 0 };

static void change_load_filename( GtkButton *button, gpointer user_data );
static void load_data( GtkEntry *entry, gpointer user_data );

static void change_save_filename( GtkButton *button, gpointer user_data );
static void save_data( GtkEntry *entry, gpointer user_data );


/*
 * This is the main dialog for the load/save binary data interface. It's
 * shared between the 2 functions.
 *
 * It's passed the binary_info structure, which contains the start address
 * and length values of the block to either load or save. These are populated
 * into the GUI.
 */

typedef enum
{
  BINARY_DIALOG_MODE_LOAD,
  BINARY_DIALOG_MODE_SAVE
} BINARY_DIALOG_MODE;

static void
create_shared_binary_dialog( struct binary_info *info, const char *title,
                             BINARY_DIALOG_MODE mode )
{
  GtkWidget *table, *button, *content_area;
  GtkWidget *label_filename, *label_start, *label_length;

  char buffer[80];

  info->dialog = gtkstock_dialog_new( title, NULL );

  content_area = gtk_dialog_get_content_area( GTK_DIALOG( info->dialog ) );

  label_filename = gtk_label_new( "Filename" );

  info->filename_widget = gtk_label_new( info->filename );
#if GTK_CHECK_VERSION( 3, 16, 0 )
  gtk_label_set_xalign( GTK_LABEL(info->filename_widget), 0.0);
#else
  gtk_misc_set_alignment( GTK_MISC(info->filename_widget), 0.0, 0.5 );
#endif

  button = gtk_button_new_with_label( "Browse..." );
  g_signal_connect( G_OBJECT( button ), "clicked", info->change_filename,
                    info );

  label_start = gtk_label_new( "Start" );

  info->start_widget = gtk_entry_new();
  snprintf( buffer, 80, "%lu", info->start );
  gtk_entry_set_text( GTK_ENTRY( info->start_widget ), buffer );
  g_signal_connect( G_OBJECT( info->start_widget ), "activate",
                    info->activate_data, info );

  label_length = gtk_label_new( "Length" );

  info->length_widget = gtk_entry_new();
  if( mode == BINARY_DIALOG_MODE_LOAD ) {
    snprintf( buffer, 80, "%lu", (unsigned long)info->file.length );
  } else {
    snprintf( buffer, 80, "%lu", info->length );
  }
  gtk_entry_set_text( GTK_ENTRY( info->length_widget ), buffer );

  g_signal_connect( G_OBJECT( info->length_widget ), "activate",
                    info->activate_data, info );

#if GTK_CHECK_VERSION( 3, 0, 0 )

  table = gtk_grid_new();
  gtk_grid_set_column_homogeneous( GTK_GRID( table ), FALSE );
  gtk_grid_set_column_spacing( GTK_GRID( table ), 6 );
  gtk_grid_set_row_spacing( GTK_GRID( table ), 6 );
  gtk_container_set_border_width( GTK_CONTAINER( table ), 6 );
  gtk_container_add( GTK_CONTAINER( content_area ), table );

  gtk_grid_attach( GTK_GRID( table ), label_filename, 0, 0, 1, 1 );
  gtk_widget_set_hexpand( info->filename_widget, TRUE );
  gtk_grid_attach( GTK_GRID( table ), info->filename_widget,
                   1, 0, 1, 1 );
  gtk_grid_attach( GTK_GRID( table ), button, 2, 0, 1, 1 );
  gtk_grid_attach( GTK_GRID( table ), label_start, 0, 1, 1, 1 );
  gtk_grid_attach( GTK_GRID( table ), info->start_widget, 1, 1, 2, 1 );
  gtk_grid_attach( GTK_GRID( table ), label_length, 0, 2, 1, 1 );
  gtk_grid_attach( GTK_GRID( table ), info->length_widget, 1, 2, 2, 1 );

#else                /* #if GTK_CHECK_VERSION( 3, 0, 0 ) */

  table = gtk_table_new( 3, 3, FALSE );
  gtk_box_pack_start( GTK_BOX( content_area ), table, TRUE, TRUE, 0 );

  gtk_table_attach( GTK_TABLE( table ), label_filename, 0, 1, 0, 1,
                    GTK_FILL, GTK_FILL, 3, 3 );

  gtk_table_attach( GTK_TABLE( table ), info->filename_widget, 1, 2, 0, 1,
                    GTK_FILL, GTK_FILL, 3, 3 );

  gtk_table_attach( GTK_TABLE( table ), button, 2, 3, 0, 1,
                    GTK_FILL, GTK_FILL, 3, 3 );

  gtk_table_attach( GTK_TABLE( table ), label_start, 0, 1, 1, 2, 0, 0, 3, 3 );

  gtk_table_attach( GTK_TABLE( table ), info->start_widget, 1, 3, 1, 2,
                    GTK_FILL, GTK_FILL, 3, 3 );

  gtk_table_attach( GTK_TABLE( table ), label_length, 0, 1, 2, 3,
                    GTK_FILL, GTK_FILL, 3, 3 );

  gtk_table_attach( GTK_TABLE( table ), info->length_widget, 1, 3, 2, 3,
                    GTK_FILL, GTK_FILL, 3, 3 );

#endif

  GtkAccelGroup *accel_group;
  accel_group = gtk_accel_group_new();
  gtk_window_add_accel_group( GTK_WINDOW( info->dialog ), accel_group );

  /* Command buttons */
  gtkstock_create_ok_cancel( info->dialog, NULL, info->activate_data, info,
                             DEFAULT_DESTROY, DEFAULT_DESTROY );

  gtk_widget_grab_focus( info->start_widget );
}


/*
 * This is the entry point for the load binary data command. It's called from
 * the GUI menu. It pops the file name selector dialog if the user hasn't
 * already specified a filename, then pops the dialog requesting start address
 * and length.
 */
void
menu_file_loadbinarydata( GtkAction *gtk_action GCC_UNUSED,
                          gpointer data GCC_UNUSED )
{
  int error;

  fuse_emulation_pause();

  /*
   * If the filename is blank it's not been selected previously.
   * Pop up the file selector.
   */
  if( !load_info.filename )
  {
    load_info.filename = ui_get_open_filename( "Fuse - Load Binary Data" );
    if( !load_info.filename )
    {
      fuse_emulation_unpause();
      return;
    }
  }

  error = utils_read_file( load_info.filename, &load_info.file );
  if( error )
  {
    fuse_emulation_unpause();
    return;
  }

  /* Information display */
  load_info.activate_data = G_CALLBACK( load_data );
  load_info.change_filename = G_CALLBACK( change_load_filename );

  create_shared_binary_dialog( &load_info, "Fuse - Load Binary Data",
                               BINARY_DIALOG_MODE_LOAD );

  /* Process the dialog */
  gtk_widget_show_all( load_info.dialog );
  gtk_main();

  utils_close_file( &load_info.file );

  fuse_emulation_unpause();
}


/*
 * Change the name of the file the user is going to load into the Spectrum's
 * memory. This pops a file selector dialog and stores the name of the file
 * chosen (releasing the memory of the previously chosen name).
 */
static void
change_load_filename( GtkButton *button GCC_UNUSED, gpointer user_data )
{
  struct binary_info *info = user_data;
  
  char *new_filename;
  utils_file new_file;

  char buffer[80];
  int error;

  new_filename = ui_get_open_filename( "Fuse - Load Binary Data" );
  if( !new_filename ) return;

  error = utils_read_file( new_filename, &new_file );
  if( error ) { free( new_filename ); return; }

  /* Remove the data for the old file */
  utils_close_file( &info->file );

  free( info->filename );

  /* Put the new data in */
  info->filename = new_filename; info->file = new_file;

  /* And update the displayed information */
  gtk_label_set_text( GTK_LABEL( info->filename_widget ), new_filename );
  
  snprintf( buffer, 80, "%lu", (unsigned long)info->file.length );
  gtk_entry_set_text( GTK_ENTRY( info->length_widget ), buffer );
}


/*
 * Load a chunk of binary data into the Spectrum's memory. It's called as a
 * callback from the GUI when the user hits the OK button.
 *
 * The given user_data is a binary_info structure which contains the user's
 * selected filename, plus the start/length widgets the user will have filled
 * in.
 *
 * This is called in a separate GTK loop, so it returns to code which tidies
 * up and unpauses the Spectrum.
 */
static void
load_data( GtkEntry *entry GCC_UNUSED, gpointer user_data )
{
  struct binary_info *info = user_data;

  long start, length; size_t i;
  const gchar *nptr;
  char *endptr;
  int base;

  errno = 0;
  nptr = gtk_entry_get_text( GTK_ENTRY( info->length_widget ) );
  base = ( g_str_has_prefix( nptr, "0x" ) )? 16 : 10;
  length = strtol( nptr, &endptr, base );

  if( errno || length < 1 || length > 0x10000 || endptr == nptr ) {
    ui_error( UI_ERROR_ERROR, "Length must be between 1 and 65536" );
    return;
  }

  if( length > info->file.length ) {
    ui_error( UI_ERROR_ERROR,
	      "'%s' contains only %lu bytes",
	      info->filename, (unsigned long)info->file.length );
    return;
  }

  errno = 0;
  nptr = gtk_entry_get_text( GTK_ENTRY( info->start_widget ) );
  base = ( g_str_has_prefix( nptr, "0x" ) )? 16 : 10;
  start = strtol( nptr, &endptr, base );

  if( errno || start < 0 || start > 0xffff || endptr == nptr ) {
    ui_error( UI_ERROR_ERROR, "Start must be between 0 and 65535" );
    return;
  }

  if( start + length > 0x10000 ) {
    ui_error( UI_ERROR_ERROR, "Block ends after address 65535" );
    return;
  }

  for( i = 0; i < length; i++ )
    writebyte_internal( start + i, info->file.buffer[ i ] );

  gtkui_destroy_widget_and_quit( info->dialog, NULL );
}
  

/*
 * This is the start point for the save binary data GUI command. It pops up
 * the file selector if a selected file isn't already in the static structure,
 * then pops up the filename/start/length dialog.
 */
void
menu_file_savebinarydata( GtkAction *gtk_action GCC_UNUSED,
                          gpointer data GCC_UNUSED )
{
  fuse_emulation_pause();

  /*
   * If the filename is blank it's not been selected previously.
   * Pop up the file selector.
   */
  if( !save_info.filename )
  {
    save_info.filename = ui_get_save_filename( "Fuse - Save Binary Data" );
    if( !save_info.filename )
    {
      fuse_emulation_unpause();
      return;
    }
  }

  save_info.activate_data = G_CALLBACK( save_data );
  save_info.change_filename = G_CALLBACK( change_save_filename );

  create_shared_binary_dialog( &save_info, "Fuse - Save Binary Data",
                               BINARY_DIALOG_MODE_SAVE );

  /* Process the dialog */
  gtk_widget_show_all( save_info.dialog );
  gtk_main();

  fuse_emulation_unpause();
}


/*
 * Change the name of the file the save binary command will write
 * out to. The previously specified filename is removed (and the memory
 * for it reclaimed) and a new filename is stored in its place. The
 * GUI entry field is updated.
 *
 * This is only called after the user has already specified a filename,
 * so it's safe to free the old name.
 */
static void
change_save_filename( GtkButton *button GCC_UNUSED, gpointer user_data )
{
  struct binary_info *info = user_data;
  char *new_filename;

  new_filename = ui_get_save_filename( "Fuse - Save Binary Data" );
  if( !new_filename ) return;

  free( info->filename );

  info->filename = new_filename;

  gtk_label_set_text( GTK_LABEL( info->filename_widget ), new_filename );
}


/*
 * Write out a chunk of binary data, as described by the given binary_info
 * structure passed in.
 *
 * This writes the given number of data bytes from the given start point in
 * the Spectrum's memory out to the given file.
 */
static void
save_data( GtkEntry *entry GCC_UNUSED, gpointer user_data )
{
  struct binary_info *info = user_data;

  const gchar *nptr;
  char *endptr;
  int base;

  int error;

  errno = 0;
  nptr = gtk_entry_get_text( GTK_ENTRY( info->length_widget ) );
  base = ( g_str_has_prefix( nptr, "0x" ) )? 16 : 10;
  info->length = strtol( nptr, &endptr, base );

  if( errno || info->length < 1 || info->length > 0x10000 || endptr == nptr ) {
    ui_error( UI_ERROR_ERROR, "Length must be between 1 and 65536" );
    return;
  }

  errno = 0;
  nptr = gtk_entry_get_text( GTK_ENTRY( info->start_widget ) );
  base = ( g_str_has_prefix( nptr, "0x" ) )? 16 : 10;
  info->start = strtol( nptr, &endptr, base );

  if( errno || info->start < 0 || info->start > 0xffff || endptr == nptr ) {
    ui_error( UI_ERROR_ERROR, "Start must be between 0 and 65535" );
    return;
  }

  if( info->start + info->length > 0x10000 ) {
    ui_error( UI_ERROR_ERROR, "Block ends after address 65535" );
    return;
  }

  error = utils_save_binary( info->start, info->length, info->filename );
  if( error ) return;

  gtkui_destroy_widget_and_quit( info->dialog, NULL );
}
