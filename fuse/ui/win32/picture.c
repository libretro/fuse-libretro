/* picture.c: Win32 routines to draw the keyboard picture
   Copyright (c) 2002-2019 Philip Kendall, Marek Januszewski, Stuart Brady,
                           Sergio Baldoví

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

#ifdef USE_LIBPNG
#include <png.h>
#include <zlib.h>
#endif

#include "display.h"
#include "picture.h"
#include "utils.h"
#include "win32internals.h"

/* An RGB image of the keyboard picture */
/* the memory will be allocated by Windows ( height * width * 4 bytes ) */
static libspectrum_byte *picture;
static const int picture_pitch = DISPLAY_ASPECT_WIDTH * 4;

static HWND hDialogPicture = NULL;
static HBITMAP picture_BMP = NULL;
static int min_height;
static int min_width;

static HWND create_dialog( int width, int height );

static void draw_screen( libspectrum_byte *screen, int border );

static LRESULT WINAPI picture_wnd_proc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

#ifdef PNG_SIMPLIFIED_READ_SUPPORTED

static void draw_png( libspectrum_byte *buffer, size_t size );
static int read_png_file( const char *filename, png_bytep *buffer,
                          size_t *size, int *width, int *height );

#endif			/* #ifdef PNG_SIMPLIFIED_READ_SUPPORTED */

int
win32ui_picture( const char *filename, win32ui_picture_format format )
{
  /* Dialog already open? */
  if( hDialogPicture != NULL )
    return 0;

  switch( format ) {

  case PICTURE_PNG:
  {

#ifdef PNG_SIMPLIFIED_READ_SUPPORTED
    int width, height;
    png_bytep buffer;
    size_t size;

    if( read_png_file( filename, &buffer, &size, &width, &height ) )
      return 1;

    hDialogPicture = create_dialog( width, height );
    if( hDialogPicture == NULL )
      return 1;

    draw_png( buffer, size );

    free( buffer );
    break;
#else
    /* PNG format not supported */
    return 1;
#endif			/* #ifdef PNG_SIMPLIFIED_READ_SUPPORTED */
  }

  case PICTURE_SCR:
  {
    utils_file screen;

    if( utils_read_screen( filename, &screen ) )
      return 1;

    hDialogPicture = create_dialog( DISPLAY_ASPECT_WIDTH,
                                    DISPLAY_SCREEN_HEIGHT );
    if( hDialogPicture == NULL )
      return 1;

    draw_screen( screen.buffer, 0 );

    utils_close_file( &screen );
    break;
  }

  default:
    /* Unsupported format */
    return 1;

  }

  ShowWindow( hDialogPicture, SW_SHOW );

  /* Keep focus on Fuse's main window */
  SetForegroundWindow( fuse_hWnd );

  return 0;
}

static void
resize_dialog( HWND dialog )
{
  RECT wr_dialog, cr_dialog, wr_button, wr_frame, rc;
  HWND button_hWnd, frame_hWnd;

  /* Dialog dimensions */
  GetWindowRect( dialog, &wr_dialog );
  GetClientRect( dialog, &cr_dialog );
  OffsetRect( &wr_dialog, -wr_dialog.left, -wr_dialog.top );
  OffsetRect( &cr_dialog, -cr_dialog.left, -cr_dialog.top );

  /* Move Close button */
  SetRectEmpty( &rc );
  rc.top = 7; /*  bottom spacing (DLUs) */
  MapDialogRect( dialog, &rc );

  button_hWnd = GetDlgItem( dialog, IDCLOSE );
  GetWindowRect( button_hWnd, &wr_button );
  wr_button.left = ( cr_dialog.right -
                     ( wr_button.right - wr_button.left ) ) / 2;
  wr_button.top = cr_dialog.bottom - ( wr_button.bottom - wr_button.top ) -
                  rc.top;

  SetWindowPos( button_hWnd, NULL, wr_button.left, wr_button.top, 0, 0,
                SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );

  /* Give any unused space to picture frame */
  frame_hWnd = GetDlgItem( dialog, IDC_PICTURE_FRAME );
  SetRectEmpty( &rc );
  rc.bottom = 7 + 7; /* spacing above and below button (DLUs) */
  MapDialogRect( dialog, &rc );
  GetWindowRect( button_hWnd, &wr_button );
  rc.bottom += wr_button.bottom - wr_button.top; /* button height */
  wr_frame.bottom = cr_dialog.bottom - rc.bottom;
  wr_frame.right = cr_dialog.right;

  SetWindowPos( frame_hWnd, NULL, 0, 0, wr_frame.right, wr_frame.bottom,
                SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS );
}

static void
inflate_dialog( HWND dialog, int width, int height )
{
  RECT wr_dialog, cr_dialog, wr_button, rc;
  HWND button_hWnd;

  /* Dialog dimensions */
  GetWindowRect( dialog, &wr_dialog );
  GetClientRect( dialog, &cr_dialog );
  OffsetRect( &wr_dialog, -wr_dialog.left, -wr_dialog.top );
  OffsetRect( &cr_dialog, -cr_dialog.left, -cr_dialog.top );

  /* Button dimensions */
  button_hWnd = GetDlgItem( dialog, IDCLOSE );
  GetWindowRect( button_hWnd, &wr_button );
  OffsetRect( &wr_button, -wr_button.left, -wr_button.top );

  /* Button spacing */
  SetRectEmpty( &rc );
  rc.bottom = 7 + 7; /* top spacing + bottom spacing (DLUs) */
  rc.right  = 7 + 7; /* left spacing + right spacing (DLUs) */
  MapDialogRect( dialog, &rc );
  rc.bottom += wr_button.bottom; /* pixels */
  rc.right  += wr_button.right;  /* pixels */

  /* Image dimensions */
  rc.bottom += height; /* pixels */
  if( width > rc.right )
    rc.right = width;  /* pixels */

  /* Window decorations */
  rc.bottom += wr_dialog.bottom - cr_dialog.bottom;
  rc.right  += wr_dialog.right - cr_dialog.right;

  SetWindowPos( dialog, NULL, 0, 0, rc.right, rc.bottom,
                SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );

  min_width = rc.right;
  min_height = rc.bottom;

  resize_dialog( dialog );
}

static void
move_dialog( HWND dialog, HWND parent )
{
  RECT rc_workarea, rc_parent, wr_dialog, rc;
  int placed = 0;

  SystemParametersInfo( SPI_GETWORKAREA, 0, &rc_workarea, 0 );
  GetWindowRect( parent, &rc_parent );
  GetWindowRect( dialog, &wr_dialog );

  /* Place centered below Fuse's main window */
  if( !placed ) {
    CopyRect( &rc, &wr_dialog );
    OffsetRect( &rc, -rc.left, -rc.top );
    OffsetRect( &rc, -rc.right/2, 0 );
    OffsetRect( &rc, ( rc_parent.right - rc_parent.left ) / 2, 0 );
    OffsetRect( &rc, rc_parent.left, rc_parent.bottom );
    if( rc.left >= rc_workarea.left && rc.right <= rc_workarea.right &&
        rc.bottom <= rc_workarea.bottom )
      placed = 1;
  }

  /* Place right to Fuse's main window */
  if( !placed ) {
    CopyRect( &rc, &wr_dialog );
    OffsetRect( &rc, -rc.left, -rc.top );
    OffsetRect( &rc, rc_parent.right, rc_parent.top );
    if( rc.right <= rc_workarea.right && rc.bottom <= rc_workarea.bottom )
      placed = 1;
  }

  /* Place left to Fuse's main window */
  if( !placed ) {
    CopyRect( &rc, &wr_dialog );
    OffsetRect( &rc, -rc.left, -rc.top );
    OffsetRect( &rc, rc_parent.left - rc.right, rc_parent.top );
    if( rc.left >= rc_workarea.left && rc.bottom <= rc_workarea.bottom )
      placed = 1;
  }

  /* Move the dialog or leave at initial position */
  if( placed ) {
    SetWindowPos( dialog, NULL, rc.left, rc.top, 0, 0,
                  SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
  }
}

static HWND
create_dialog( int width, int height )
{
  HWND dialog;

  dialog = CreateDialog( fuse_hInstance, MAKEINTRESOURCE( IDD_PICTURE ),
                         fuse_hWnd, (DLGPROC)picture_wnd_proc );

  if( dialog == NULL )
    return NULL;

  inflate_dialog( dialog, width, height );
  move_dialog( dialog, fuse_hWnd );

  /* Create the picture buffer */
  BITMAPINFO picture_BMI;
  memset( &picture_BMI, 0, sizeof( picture_BMI ) );
  picture_BMI.bmiHeader.biSize = sizeof( picture_BMI.bmiHeader );
  picture_BMI.bmiHeader.biWidth = width;
  /* negative to avoid "shep-mode": */
  picture_BMI.bmiHeader.biHeight = -height;
  picture_BMI.bmiHeader.biPlanes = 1;
  picture_BMI.bmiHeader.biBitCount = 32;
  picture_BMI.bmiHeader.biCompression = BI_RGB;
  picture_BMI.bmiHeader.biSizeImage = 0;
  picture_BMI.bmiHeader.biXPelsPerMeter = 0;
  picture_BMI.bmiHeader.biYPelsPerMeter = 0;
  picture_BMI.bmiHeader.biClrUsed = 0;
  picture_BMI.bmiHeader.biClrImportant = 0;
  picture_BMI.bmiColors[0].rgbRed = 0;
  picture_BMI.bmiColors[0].rgbGreen = 0;
  picture_BMI.bmiColors[0].rgbBlue = 0;
  picture_BMI.bmiColors[0].rgbReserved = 0;

  HDC dc = GetDC( dialog );
  picture_BMP = CreateDIBSection( dc, &picture_BMI, DIB_RGB_COLORS,
                                  (void **)&picture, NULL, 0 );
  ReleaseDC( dialog, dc );

  return dialog;
}

static void
destroy_dialog( void )
{
  DestroyWindow( hDialogPicture );
  hDialogPicture = NULL;

  DeleteObject( picture_BMP );
  picture_BMP = NULL;
}

static void
resizing_dialog( WPARAM wParam, LPARAM lParam )
{
  RECT *rc;
  int width, height;

  rc = (RECT *)lParam;
  width = rc->right - rc->left;
  height = rc->bottom - rc->top;

  /* Force dialog minimum size to fit original picture size */
  if( height < min_height ) {
    if( wParam == WMSZ_TOP || wParam == WMSZ_TOPLEFT ||
        wParam == WMSZ_TOPRIGHT )
      rc->top = rc->bottom - min_height;
    else
      rc->bottom = rc->top + min_height;
  }

  if( width < min_width ) {
    if( wParam == WMSZ_LEFT ||
        wParam == WMSZ_TOPLEFT ||
        wParam == WMSZ_BOTTOMLEFT )
      rc->left = rc->right - min_width;
    else
      rc->right = rc->left + min_width;
  }
}

static int
draw_frame( LPDRAWITEMSTRUCT drawitem )
{
  float scale_x, scale_y, scale_factor;
  int offset_x, offset_y;
  int scaled_width, scaled_height;
  int allocation_width, allocation_height;
  BITMAP bitmap_info;
  RECT rc;

  if( !( drawitem->itemAction & ODA_DRAWENTIRE ) )
    return FALSE;

  HDC pic_dc = CreateCompatibleDC( drawitem->hDC );
  HBITMAP old_bmp = SelectObject( pic_dc, picture_BMP );
  GetObject( picture_BMP, sizeof( BITMAP ), &bitmap_info );

  allocation_width = drawitem->rcItem.right - drawitem->rcItem.left;
  allocation_height = drawitem->rcItem.bottom - drawitem->rcItem.top;

  scale_x = (float)allocation_width / bitmap_info.bmWidth;
  scale_y = (float)allocation_height / bitmap_info.bmHeight;
  scale_factor = scale_x < scale_y ? scale_x : scale_y;

  scaled_width = bitmap_info.bmWidth * scale_factor;
  scaled_height = bitmap_info.bmHeight * scale_factor;
  offset_x = ( allocation_width - scaled_width ) / 2;
  offset_y = ( allocation_height - scaled_height ) / 2;

  StretchBlt( drawitem->hDC, offset_x, offset_y,
              scaled_width,
              scaled_height,
              pic_dc, 0, 0, bitmap_info.bmWidth, bitmap_info.bmHeight,
              SRCCOPY );

  SelectObject( pic_dc, old_bmp );
  DeleteDC( pic_dc );

  /* Erase background (hopefully) without flickering */
  if( offset_y > 0 ) {
    rc.left = 0;
    rc.top = 0;
    rc.right = allocation_width;
    rc.bottom = offset_y;
    FillRect( drawitem->hDC, &rc, (HBRUSH) GetStockObject( BLACK_BRUSH ) );

    rc.left = 0;
    rc.top = offset_y + scaled_height;
    rc.right = allocation_width;
    rc.bottom = allocation_height;
    FillRect( drawitem->hDC, &rc, (HBRUSH) GetStockObject( BLACK_BRUSH ) );
  }

  if( offset_x > 0 ) {
    rc.left = 0;
    rc.top = 0;
    rc.right = offset_x;
    rc.bottom = allocation_height;
    FillRect( drawitem->hDC, &rc, (HBRUSH) GetStockObject( BLACK_BRUSH ) );

    rc.left = offset_x + scaled_width;
    rc.top = 0;
    rc.right = allocation_width;
    rc.bottom = allocation_height;
    FillRect( drawitem->hDC, &rc, (HBRUSH) GetStockObject( BLACK_BRUSH ) );
  }

  return TRUE;
}

static LRESULT WINAPI
picture_wnd_proc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
  switch( msg ) {
    case WM_SIZE:
      resize_dialog( hWnd );
      return 0;

    case WM_SIZING:
      resizing_dialog( wParam, lParam );
      return TRUE;

    case WM_DRAWITEM:
      if( wParam == IDC_PICTURE_FRAME )
        return draw_frame( (LPDRAWITEMSTRUCT) lParam );
      break;

    case WM_COMMAND:
      if( LOWORD( wParam ) == IDCLOSE ) {
        destroy_dialog();
        return 0;
      }
      break;

    case WM_CLOSE:
      destroy_dialog();
      return 0;
  }

  return FALSE;
}

static void
draw_screen( libspectrum_byte *screen, int border )
{
  int i, x, y, ink, paper;
  libspectrum_byte attr, data;

  GdiFlush();

  for( y=0; y < DISPLAY_BORDER_HEIGHT; y++ ) {
    for( x=0; x < DISPLAY_ASPECT_WIDTH; x++ ) {
      *(libspectrum_dword*)( picture + y * picture_pitch + 4 * x ) =
        win32display_colours[border];
      *(libspectrum_dword*)(
          picture +
          ( y + DISPLAY_BORDER_HEIGHT + DISPLAY_HEIGHT ) * picture_pitch +
          4 * x
        ) = win32display_colours[ border ];
    }
  }

  for( y=0; y<DISPLAY_HEIGHT; y++ ) {

    for( x=0; x < DISPLAY_BORDER_ASPECT_WIDTH; x++ ) {
      *(libspectrum_dword*)
        (picture + ( y + DISPLAY_BORDER_HEIGHT) * picture_pitch + 4 * x) =
        win32display_colours[ border ];
      *(libspectrum_dword*)(
          picture +
          ( y + DISPLAY_BORDER_HEIGHT ) * picture_pitch +
          4 * ( x+DISPLAY_ASPECT_WIDTH-DISPLAY_BORDER_ASPECT_WIDTH )
        ) = win32display_colours[ border ];
    }

    for( x=0; x < DISPLAY_WIDTH_COLS; x++ ) {

      attr = screen[ display_attr_start[y] + x ];

      ink = ( attr & 0x07 ) + ( ( attr & 0x40 ) >> 3 );
      paper = ( attr & ( 0x0f << 3 ) ) >> 3;

      data = screen[ display_line_start[y]+x ];

      for( i=0; i<8; i++ ) {
        libspectrum_dword pix =
          win32display_colours[ ( data & 0x80 ) ? ink : paper ];

        /* rearrange pixel components */
        pix = ( pix & 0x0000ff00 ) |
              ( ( pix & 0x000000ff ) << 16 ) |
              ( ( pix & 0x00ff0000 ) >> 16 );

        *(libspectrum_dword*)(
            picture +
            ( y + DISPLAY_BORDER_HEIGHT ) * picture_pitch +
            4 * ( 8 * x + DISPLAY_BORDER_ASPECT_WIDTH + i )
          ) = pix;
        data <<= 1;
      }
    }

  }
}

#ifdef PNG_SIMPLIFIED_READ_SUPPORTED

static void
draw_png( libspectrum_byte *buffer, size_t size )
{
  GdiFlush();
  memcpy( picture, buffer, size );
}

static int
read_png_file( const char *filename, png_bytep *buffer, size_t *size,
               int *width, int *height )
{
  utils_file png_file;
  int error = utils_read_auxiliary_file( filename, &png_file,
                                         UTILS_AUXILIARY_LIB );
  if( error == -1 ) {
    return 1;
  }
  if( error ) return 1;

  /* Initialize the 'png_image' structure */
  png_image image;
  memset( &image, 0, sizeof( image ) );
  image.version = PNG_IMAGE_VERSION;
  image.opaque = NULL;

  if( png_image_begin_read_from_memory( &image, png_file.buffer,
                                        png_file.length ) == 0 ) {
    utils_close_file( &png_file );
    return 1;
  }

  image.format = PNG_FORMAT_BGRA;
  *buffer = malloc( PNG_IMAGE_SIZE( image ) );

  if( *buffer == NULL ) {
    png_image_free( &image );
    utils_close_file( &png_file );
    return 1;
  }
  *size = PNG_IMAGE_SIZE( image );

  if( !png_image_finish_read( &image, NULL, *buffer, 0, NULL ) ) {
    png_image_free( &image );
    free( *buffer );
    utils_close_file( &png_file );
    return 1;
  }

  *width = PNG_IMAGE_ROW_STRIDE( image ) /
           PNG_IMAGE_PIXEL_SIZE( PNG_FORMAT_BGRA );
  *height = PNG_IMAGE_SIZE( image ) / PNG_IMAGE_ROW_STRIDE( image );
  png_image_free( &image );
  utils_close_file( &png_file );

  return 0;
}

#endif			/* #ifdef PNG_SIMPLIFIED_READ_SUPPORTED */
