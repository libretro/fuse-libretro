/* fileselector.c: Win32 fileselector routines
   Copyright (c) 2008 Marek Januszewski

   $Id: fileselector.c 4643 2012-01-21 16:12:10Z pak21 $

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

#include <windows.h>

#include "utils.h"
#include "win32internals.h"

/* FIXME: remember the last directory when opening/saving
static char *current_folder;
*/

static char*
run_dialog( const char *title, int is_saving )
{
  OPENFILENAME ofn;
  char szFile[512];
  int result;

  memset( &ofn, 0, sizeof( ofn ) );
  szFile[0] = '\0';

  ofn.lStructSize = sizeof( ofn );
  ofn.hwndOwner = fuse_hWnd;
  ofn.lpstrFilter = "All Files\0*.*\0\0";
  ofn.lpstrCustomFilter = NULL;
  ofn.nFilterIndex = 0;
  ofn.lpstrFile = szFile;
  ofn.nMaxFile = sizeof( szFile );
  ofn.lpstrFileTitle = NULL;
  ofn.lpstrInitialDir = NULL;
  ofn.lpstrTitle = title;
  ofn.Flags = /* OFN_DONTADDTORECENT | */ OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
  if( is_saving ) {
    ofn.Flags |= OFN_OVERWRITEPROMPT | OFN_NOREADONLYRETURN;
  } else {
    ofn.Flags |= OFN_FILEMUSTEXIST;
  }
  ofn.nFileOffset = 0;
  ofn.nFileExtension = 0;
  ofn.lpstrDefExt = NULL;
/* ofn.pvReserved = NULL; */
/* ofn.FlagsEx = 0; */

  if( is_saving ) {
    result = GetSaveFileName( &ofn );
  } else {
    result = GetOpenFileName( &ofn );
  }

  if( !result ) {
    return NULL;
  } else {
    return utils_safe_strdup( ofn.lpstrFile );
  }
}

char*
ui_get_open_filename( const char *title )
{
  return run_dialog( title, 0 );
}

char*
ui_get_save_filename( const char *title )
{
  return run_dialog( title, 1 );
}
