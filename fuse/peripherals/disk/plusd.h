/* plusd.h: Routines for handling the +D interface
   Copyright (c) 2005-2007 Stuart Brady

   $Id: plusd.h 4896 2013-02-23 17:44:30Z zubzero $

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

   Philip: philip-fuse@shadowmagic.org.uk

   Stuart: stuart.brady@gmail.com

*/

#ifndef FUSE_PLUSD_H
#define FUSE_PLUSD_H

#include <config.h>

typedef enum plusd_drive_number {
  PLUSD_DRIVE_1 = 0,
  PLUSD_DRIVE_2,
} plusd_drive_number;

#include <libspectrum.h>

#include "fdd.h"

extern int plusd_available;  /* Is the +D available for use? */
extern int plusd_active;     /* +D enabled? */

void plusd_init( void );
void plusd_end( void );

void plusd_page( void );
void plusd_unpage( void );

libspectrum_byte plusd_sr_read( libspectrum_word port, int *attached );
void plusd_cr_write( libspectrum_word port, libspectrum_byte b );

libspectrum_byte plusd_tr_read( libspectrum_word port, int *attached );
void plusd_tr_write( libspectrum_word port, libspectrum_byte b );

libspectrum_byte plusd_sec_read( libspectrum_word port, int *attached );
void plusd_sec_write( libspectrum_word port, libspectrum_byte b );

libspectrum_byte plusd_dr_read( libspectrum_word port, int *attached );
void plusd_dr_write( libspectrum_word port, libspectrum_byte b );

libspectrum_byte plusd_joy_read( libspectrum_word port, int *attached );
void plusd_cn_write( libspectrum_word port, libspectrum_byte b );

libspectrum_byte plusd_patch_read( libspectrum_word port, int *attached );
void plusd_patch_write( libspectrum_word port, libspectrum_byte b );

libspectrum_byte plusd_printer_read( libspectrum_word port, int *attached );
void plusd_printer_write( libspectrum_word port, libspectrum_byte b );

int plusd_disk_insert( plusd_drive_number which, const char *filename,
		       int autoload );
int plusd_disk_eject( plusd_drive_number which );
int plusd_disk_save( plusd_drive_number which, int saveas );
int plusd_disk_write( plusd_drive_number which, const char *filename );
int plusd_disk_flip( plusd_drive_number which, int flip );
int plusd_disk_writeprotect( plusd_drive_number which, int wrprot );
fdd_t *plusd_get_fdd( plusd_drive_number which );

int plusd_unittest( void );

#endif                  /* #ifndef FUSE_PLUSD_H */
