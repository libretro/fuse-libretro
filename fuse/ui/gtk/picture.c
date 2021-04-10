/* picture.c: GTK routines to draw the keyboard picture
   Copyright (c) 2002-2005 Philip Kendall

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
#include "utils.h"

static int dialog_created = 0;

static GtkWidget *dialog;
static GdkPixbuf *pixbuf = NULL;

#if GTK_CHECK_VERSION( 3, 0, 0 )

static GdkPixbuf *scaled_pixbuf = NULL;

static gint
picture_size_allocate( GtkWidget *widget GCC_UNUSED, GdkRectangle *allocation,
                       gpointer data GCC_UNUSED )
{
  float scale_x, scale_y, scale_factor;
  int scaled_width, scaled_height;

  scale_x = (float)allocation->width / gdk_pixbuf_get_width( pixbuf );
  scale_y = (float)allocation->height / gdk_pixbuf_get_height( pixbuf );
  scale_factor = scale_x < scale_y ? scale_x : scale_y;

  scaled_width = gdk_pixbuf_get_width( pixbuf ) * scale_factor;
  scaled_height = gdk_pixbuf_get_height( pixbuf ) * scale_factor;

  if( scaled_pixbuf != NULL ) g_object_unref( scaled_pixbuf );
  scaled_pixbuf = gdk_pixbuf_scale_simple( pixbuf, scaled_width, scaled_height,
                                           GDK_INTERP_BILINEAR );

  return FALSE;
}

static gboolean
picture_draw( GtkWidget *widget, cairo_t *cr, gpointer user_data GCC_UNUSED )
{
  int offset_x, offset_y;
  int scaled_width, scaled_height;
  int allocated_width, allocated_height;

  allocated_width = gtk_widget_get_allocated_width( widget );
  allocated_height = gtk_widget_get_allocated_height( widget );

  scaled_width = gdk_pixbuf_get_width( scaled_pixbuf );
  scaled_height = gdk_pixbuf_get_height( scaled_pixbuf );

  offset_x = ( allocated_width - scaled_width ) / 2;
  offset_y = ( allocated_height - scaled_height ) / 2;

  gdk_cairo_set_source_pixbuf( cr, scaled_pixbuf, offset_x, offset_y );
  cairo_paint(cr);

  return FALSE;
}

#endif                /* #if GTK_CHECK_VERSION( 3, 0, 0 ) */

int
gtkui_picture( const char *filename, int border GCC_UNUSED )
{
  GtkWidget *content_area;

  if( !dialog_created ) {

    char path[PATH_MAX];

    if( utils_find_file_path( filename, path, UTILS_AUXILIARY_LIB ) ) return 1;

    pixbuf = gdk_pixbuf_new_from_file( path, NULL );
    if( pixbuf == NULL ) {
      ui_error( UI_ERROR_ERROR, "Failed to load keyboard image" );
      return 1;
    }

    dialog = gtkstock_dialog_new( "Fuse - Keyboard",
				  G_CALLBACK( gtk_widget_hide ) );
  
    content_area = gtk_dialog_get_content_area( GTK_DIALOG( dialog ) );

#if !GTK_CHECK_VERSION( 3, 0, 0 )

    GtkWidget *image = gtk_image_new();
    gtk_image_set_from_pixbuf( GTK_IMAGE( image ), pixbuf );
    gtk_container_add( GTK_CONTAINER( content_area ), image );

    /* Stop users resizing this window */
    gtk_window_set_resizable( GTK_WINDOW( dialog ), FALSE );

#else

    GtkWidget *box = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
    gtk_container_add( GTK_CONTAINER( content_area ), box );

    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_widget_set_vexpand( drawing_area, TRUE );
    gtk_box_pack_start( GTK_BOX( box ), drawing_area, TRUE, TRUE, 0 );

    g_signal_connect( G_OBJECT( drawing_area ), "draw",
                      G_CALLBACK( picture_draw ), NULL );
    g_signal_connect( G_OBJECT( drawing_area ), "size-allocate",
                      G_CALLBACK( picture_size_allocate ), NULL );

    gtk_widget_set_size_request( GTK_WIDGET( drawing_area ),
                                 gdk_pixbuf_get_width( pixbuf ),
                                 gdk_pixbuf_get_height( pixbuf ) );

#endif                /* #if !GTK_CHECK_VERSION( 3, 0, 0 ) */

    gtkstock_create_close( dialog, NULL, G_CALLBACK( gtk_widget_hide ),
                           FALSE );

    dialog_created = 1;
  }

  gtk_widget_show_all( dialog );

  return 0;
}
