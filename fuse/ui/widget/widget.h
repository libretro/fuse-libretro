/* widget.h: Simple dialog boxes for all user interfaces.
   Copyright (c) 2001-2004 Matan Ziv-Av, Philip Kendall

   $Id: widget.h 4671 2012-02-20 10:33:32Z fredm $

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

#ifndef FUSE_WIDGET_H
#define FUSE_WIDGET_H

#include "input.h"
#include "ui/scaler/scaler.h"
#include "ui/ui.h"

/* Code called at start and end of emulation */
int widget_init( void );
int widget_end( void );

/* The various widgets which are available */
typedef enum widget_type {

  WIDGET_TYPE_FILESELECTOR,	/* File selector (load) */
  WIDGET_TYPE_FILESELECTOR_SAVE,/* File selector (save) */
  WIDGET_TYPE_GENERAL,		/* General options */
  WIDGET_TYPE_PICTURE,		/* Keyboard picture */
  WIDGET_TYPE_MENU,		/* General menu */
  WIDGET_TYPE_SELECT,		/* Select machine */
  WIDGET_TYPE_SOUND,		/* Sound options */
  WIDGET_TYPE_ERROR,		/* Error report */
  WIDGET_TYPE_RZX,		/* RZX options */
  WIDGET_TYPE_MOVIE,		/* Movie options */
  WIDGET_TYPE_BROWSE,		/* Browse tape */
  WIDGET_TYPE_TEXT,		/* Text entry widget */
  WIDGET_TYPE_DEBUGGER,		/* Debugger widget */
  WIDGET_TYPE_POKEFINDER,	/* Poke finder widget */
  WIDGET_TYPE_POKEMEM,  	/* Poke memory widget */
  WIDGET_TYPE_MEMORYBROWSER,	/* Memory browser widget */
  WIDGET_TYPE_ROM,		/* ROM selector widget */
  WIDGET_TYPE_PERIPHERALS_GENERAL, /* General peripherals options */
  WIDGET_TYPE_PERIPHERALS_DISK, /* Disk peripherals options */
  WIDGET_TYPE_QUERY,		/* Query (yes/no) */
  WIDGET_TYPE_QUERY_SAVE,	/* Query (save/don't save/cancel) */
  WIDGET_TYPE_DISKOPTIONS,	/* Disk options widget */
} widget_type;

/* Activate a widget */
int widget_do( widget_type which, void *data );

/* Finish with widgets for now */
void widget_finish( void );

/* A function to handle keypresses */
typedef void (*widget_keyhandler_fn)( input_key key );

/* The current widget keyhandler */
extern widget_keyhandler_fn widget_keyhandler;

/* Widget-specific bits */

/* Menu widget */

/* A generic callback function */
typedef void (*widget_menu_callback_fn)( int action );

/* A generic menu detail callback function */
typedef const char* (*widget_menu_detail_callback_fn)( void );

/* A general menu */
typedef struct widget_menu_entry {
  const char *text;		/* Menu entry text */
  input_key key;		/* Which key to activate this widget */

  struct widget_menu_entry *submenu;
  widget_menu_callback_fn callback;
  widget_menu_detail_callback_fn detail;

  int action;
  int inactive;

} widget_menu_entry;

/* The main menu as activated with F1 */
extern widget_menu_entry widget_menu[];

/* Get the maximum menu width to use in pixels */
int widget_calculate_menu_width( widget_menu_entry *menu );

/* The name returned from the file selector */
extern char* widget_filesel_name;

/* Select a machine */
int widget_select_machine( void *data );
     
/* The error widget data type */

typedef struct widget_error_t {
  ui_error_level severity;
  const char *message;
} widget_error_t;

#endif				/* #ifndef FUSE_WIDGET_H */
