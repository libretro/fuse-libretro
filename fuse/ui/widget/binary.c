/* binary.c: The binary load/save widgets
   Copyright (c) 2019 Gergely Szasz

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

   E-mail: szaszg@hu.inter.net

*/

#include <config.h>

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "fuse.h"
#include "utils.h"
#include "widget_internals.h"

typedef struct {
  int confirm;
  ui_confirm_save_t save;
  const char *title;

  char *filename;
  utils_file file;

  int load;
  char *start_str;
  char *length_str;
  libspectrum_dword start;
  libspectrum_dword length;
} widget_binary_t;

static widget_binary_t widget_binary;

struct widget_binary_entry;

/* A generic click function */
typedef void (*widget_binary_click_fn)( void );

/* A general menu */
typedef struct widget_binary_entry {
  const char *text;
  int x, y;

  widget_binary_click_fn click;
} widget_binary_entry;

static void widget_browse_click( void );
static void widget_start_click( void );
static void widget_length_click( void );
static void widget_cancel_click( void );
static void widget_ok_click( void );

static widget_binary_entry binary_entry[] = {
  { "\012B\001rowse", 208, 28,  widget_browse_click },
  { "\012S\001tart",  16,  40,  widget_start_click  },
  { "\012L\001ength", 16,  48,  widget_length_click },
  { "\012C\001ancel", 186, 68,  widget_cancel_click },
  { "\012O\001k",     228, 68,  widget_ok_click     },
  { NULL }
};

static size_t highlight_entry = 0;

static void
widget_binary_entry_draw( int index, int highlight )
{
  int w =  widget_stringwidth( binary_entry[index].text );

  widget_rectangle( binary_entry[index].x - 1, binary_entry[index].y, w + 2, 8,
                    highlight ? WIDGET_COLOUR_HIGHLIGHT : WIDGET_COLOUR_BACKGROUND );
  widget_printstring( binary_entry[index].x, binary_entry[index].y,
                      WIDGET_COLOUR_FOREGROUND, binary_entry[index].text );
  widget_display_rasters( binary_entry[index].y, 8 );
}

/* 0 - all, 1 - filename, 2 - start, 3 - length, 4 - DEC/HEX/OCT */
static void
display_values( int what )
{
  if( !what || what == 1 ) {
    const char *fn;
    fn = widget_binary.filename;
    while( widget_stringwidth( fn ) >= 136 ) fn++;
    widget_rectangle( 68, 28, 136, 8, WIDGET_COLOUR_BACKGROUND );
    widget_printstring( 68, 28, WIDGET_COLOUR_DISABLED, fn );
    widget_display_lines( 3, 2 );
  }
  if( !what || what == 2 ) {
    widget_rectangle( 68, 40, 136, 8, WIDGET_COLOUR_BACKGROUND );
    widget_printstring( 68, 40, WIDGET_COLOUR_FOREGROUND,
                        widget_binary.start_str );
    widget_display_lines( 5, 1 );
  }
  if( !what || what == 3 ) {
    widget_rectangle( 68, 48, 136, 8, WIDGET_COLOUR_BACKGROUND );
    widget_printstring( 68, 48, WIDGET_COLOUR_FOREGROUND,
                        widget_binary.length_str );
    widget_display_lines( 6, 1 );
  }
}

int
widget_binary_draw( void *data )
{
  int i;

  widget_dialog_with_border( 1, 2, 30, 8 );
  widget_printstring( 10, 16, WIDGET_COLOUR_TITLE, widget_binary.title );
  widget_printstring( 16, 28, WIDGET_COLOUR_FOREGROUND, "Filename: " );

  display_values( 0 );

  for( i = 0; binary_entry[i].text != NULL; i++ ) {
    widget_binary_entry_draw( i, ( highlight_entry == i ) );
  }

  widget_display_lines( 2, 8 );
  return 0;
}

static void
ask_filename()
{
  widget_filesel_data filesel;

  filesel.exit_all_widgets = 0;

  if( widget_binary.load ) {
    filesel.title = "Fuse - Load Binary Data";
    widget_do_fileselector( &filesel );
  } else {
    filesel.title = "Fuse - Save Binary Data";
    widget_do_fileselector_save( &filesel );
  }
  if( !widget_filesel_name ) return;

  free( widget_binary.filename );

  widget_binary.filename = utils_safe_strdup( widget_filesel_name );

  display_values( 1 );
}

/* 1 - filename, 2 - start, 3 - length */
static void
ask_value( int what )
{
  long int value;
  char *s;
  widget_text_t text_data;

  text_data.allow = WIDGET_INPUT_ALNUM;
  text_data.max_length = 9;
  if( what == 2 ) {
    text_data.title = "Enter start value";
    s = widget_binary.start_str;
  } else if( what == 3 ) {
    text_data.title = "Enter length";
    s = widget_binary.length_str;
  }
  snprintf( text_data.text, sizeof( text_data.text ), "%s", s );
  widget_do_text( &text_data );
  if( !widget_text_text ) return;

  value = strtol( widget_text_text, &s, 0 );
  if( *s != '\0' ) {
    ui_error( UI_ERROR_ERROR, "Invalid number" );
    return;
  }

  if( what == 2 ) {
    if( value < 0 || value > 0xffff ) {
      ui_error( UI_ERROR_ERROR, "Start must be between 0 and 65535" );
    } else if( value + widget_binary.length > 0x10000 ) {
      ui_error( UI_ERROR_ERROR, "Block ends after address 65535" );
    } else {
      free( widget_binary.start_str );
      widget_binary.start_str = utils_safe_strdup( widget_text_text );
      widget_binary.start = value;
    }
  } else if( what == 3 ) {
    if( value < 1 || value > 0x10000 ) {
      ui_error( UI_ERROR_ERROR, "Length must be between 1 and 65536" );
    } else if( value + widget_binary.start > 0x10000 ) {
      ui_error( UI_ERROR_ERROR, "Block ends after address 65535" );
    } else if( widget_binary.load && value > widget_binary.file.length ) {
      ui_error( UI_ERROR_ERROR, "'%s' contains only %lu bytes",
                widget_binary.filename, (unsigned long)widget_binary.file.length
                );
      return;
    } else {
      free( widget_binary.length_str );
      widget_binary.length_str = utils_safe_strdup( widget_text_text );
      widget_binary.length = value;
    }
  }

  display_values( what );
}

static void
load_data()
{
  libspectrum_dword i;

  for( i = 0; i < widget_binary.length; i++ )
    writebyte_internal( widget_binary.start + i,
                        widget_binary.file.buffer[ i ] );
}

static void
save_data()
{
  int error;
  error = utils_save_binary( widget_binary.start, widget_binary.length,
                             widget_binary.filename );
  /* TODO: some error reporting */
  if( error ) return;
}

static void
widget_browse_click( void )
{
  ask_filename();
}

static void
widget_start_click( void )
{
  ask_value( 2 );
}

static void
widget_length_click( void )
{
  ask_value( 3 );
}

static void
widget_cancel_click( void )
{
  widget_end_widget( WIDGET_FINISHED_CANCEL );
}

static void
widget_ok_click( void )
{
  if( widget_binary.load ) {
    load_data();
  } else {
    save_data();
  }
  widget_end_all( WIDGET_FINISHED_OK );
  display_refresh_all();
}

void
widget_binary_keyhandler( input_key key )
{
  int new_highlight_entry = highlight_entry;
  int clicked = 0;

  switch( key ) {

#if 0
  case INPUT_KEY_Resize:	/* Fake keypress used on window resize */
    widget_dialog_with_border( 1, 2, 30, 2 + 20 );
    widget_general_show_all( &widget_options_settings );
    break;
#endif

  case INPUT_KEY_c:
  case INPUT_KEY_C:
  case INPUT_KEY_Escape:
  case INPUT_KEY_F10:
  case INPUT_JOYSTICK_FIRE_2:
    new_highlight_entry = 3;
    clicked = 1;
    break;

  case INPUT_KEY_b:
  case INPUT_KEY_B:
    new_highlight_entry = 0;
    clicked = 1;
    break;

  case INPUT_KEY_s:
  case INPUT_KEY_S:
    new_highlight_entry = 1;
    clicked = 1;
    break;

  case INPUT_KEY_l:
  case INPUT_KEY_L:
    new_highlight_entry = 2;
    clicked = 1;
    break;

  case INPUT_KEY_o:
  case INPUT_KEY_O:
    new_highlight_entry = 4;
    clicked = 1;
    break;

  case INPUT_KEY_Up:
  case INPUT_KEY_7:
  case INPUT_JOYSTICK_UP:
    new_highlight_entry = highlight_entry - 1;
    if( new_highlight_entry < 0 )
      while( binary_entry[new_highlight_entry + 1].text != NULL )
        new_highlight_entry++;
    break;

  case INPUT_KEY_Down:
  case INPUT_KEY_6:
  case INPUT_JOYSTICK_DOWN:
    new_highlight_entry = highlight_entry + 1;
    if( binary_entry[new_highlight_entry].text == NULL )
      new_highlight_entry = 0;
    break;

  case INPUT_KEY_Return:
  case INPUT_KEY_KP_Enter:
  case INPUT_JOYSTICK_FIRE_1:
    clicked = 1;

  default:	/* Keep gcc happy */
    break;

  }

  if( highlight_entry != new_highlight_entry ) {
    widget_binary_entry_draw( highlight_entry, 0 );
    widget_binary_entry_draw( new_highlight_entry, 1 );
    highlight_entry = new_highlight_entry;
  }
  if( clicked ) binary_entry[highlight_entry].click();
}

int
widget_binary_finish( widget_finish_state finished )
{
  free( widget_binary.start_str );
  free( widget_binary.length_str );
  free( widget_binary.filename );

  return 0;
}

static void
init_widget_binary()
{
  char str[8];

  highlight_entry = 0;
  widget_binary.start = 0;
  widget_binary.length = widget_binary.load &&
    widget_binary.file.length < 65536 ? widget_binary.file.length : 65536;

  snprintf( str, 8, "%d", widget_binary.length );
  widget_binary.start_str = utils_safe_strdup( "0" );
  widget_binary.length_str = utils_safe_strdup( str );
}

int
menu_file_loadbinarydata( int action )
{
  int error;

  fuse_emulation_pause();

  widget_binary.filename = ui_get_open_filename( "Fuse - Load Binary Data" );
  if( !widget_binary.filename ) {
    fuse_emulation_unpause();
    return 1;
  }

  error = utils_read_file( widget_binary.filename, &widget_binary.file );
  if( error ) {
    free( widget_binary.filename );
    fuse_emulation_unpause();
    return 1;
  }

  widget_binary.load = 1;
  widget_binary.title = "Fuse - Load Binary Data";
  init_widget_binary();
  /* Run dialog */
  widget_do_binary();

  utils_close_file( &widget_binary.file );

  fuse_emulation_unpause();
  return 0;
}

int
menu_file_savebinarydata( int action )
{
  fuse_emulation_pause();

  widget_binary.filename = ui_get_save_filename( "Fuse - Save Binary Data" );
  if( !widget_binary.filename ) {
    fuse_emulation_unpause();
    return 1;
  }

  widget_binary.load = 0;
  widget_binary.title = "Fuse - Save Binary Data";
  init_widget_binary();
  /* Run dialog */
  widget_do_binary();

  fuse_emulation_unpause();
  return 0;
}
