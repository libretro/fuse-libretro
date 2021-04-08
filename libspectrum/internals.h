/* internals.h: functions which need to be called inter-file by libspectrum
                routines, but not by user code
   Copyright (c) 2001-2008 Philip Kendall, Darren Salt

   $Id: internals.h 4695 2012-05-07 02:03:10Z fredm $

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

#ifndef LIBSPECTRUM_INTERNALS_H
#define LIBSPECTRUM_INTERNALS_H

#ifdef HAVE_LIB_GLIB		/* Only if we've got the real glib */
#include <glib.h>
#endif				/* #ifdef HAVE_LIB_GLIB */

#ifndef LIBSPECTRUM_LIBSPECTRUM_H
#include "libspectrum.h"
#endif				/* #ifndef LIBSPECTRUM_LIBSPECTRUM_H */

#ifdef __GNUC__
#define GCC_UNUSED __attribute__ ((unused))
#define GCC_PRINTF( fmtstring, args ) __attribute__ ((format( printf, fmtstring, args )))
#else				/* #ifdef __GNUC__ */
#define GCC_UNUSED
#define GCC_PRINTF( fmtstring, args )
#endif				/* #ifdef __GNUC__ */

#ifdef _MSC_VER
#if _MSC_VER > 1200		/* VC2005 or later */
#define __func__ __FUNCTION__
#else				/* #if _MSC_VER > 1200 */
#define __func__ "__func__"
#endif				/* _MSC_VER > 1200 */
#endif				/* #ifdef _MSC_VER */

/* VC6 lacks M_LN2, and VS2003+ require _USE_MATH_DEFINES defined before math.h
 */
#ifndef M_LN2
#define M_LN2 0.693147180559945309417
#endif

/* On Win32 systems, map snprintf -> _snprintf, strcasecmp -> _stricmp and
   strncasecmp -> _strnicmp */
#if !defined(HAVE_SNPRINTF) && defined(HAVE__SNPRINTF)
#define snprintf _snprintf
#endif		/* #if !defined(HAVE_SNPRINTF) && defined(HAVE__SNPRINTF) */

#if !defined(HAVE_STRCASECMP) && defined(HAVE__STRICMP)
#define strcasecmp _stricmp
#endif		/* #if !defined(HAVE_STRCASECMP) && defined(HAVE__STRICMP) */

#if !defined(HAVE_STRNCASECMP) && defined(HAVE__STRNICMP)
#define strncasecmp _strnicmp
#endif		/* #if !defined(HAVE_STRNCASECMP) && defined(HAVE__STRNICMP) */

/* Print using the user-provided error function */
libspectrum_error
libspectrum_print_error( libspectrum_error error, const char *format, ... )
     GCC_PRINTF( 2, 3 );

/* Acquire more memory for a buffer */
void libspectrum_make_room( libspectrum_byte **dest, size_t requested,
			    libspectrum_byte **ptr, size_t *allocated );

/* Read and write (d)words */
libspectrum_word libspectrum_read_word( const libspectrum_byte **buffer );
libspectrum_dword libspectrum_read_dword( const libspectrum_byte **buffer );
int libspectrum_write_word( libspectrum_byte **buffer, libspectrum_word w );
int libspectrum_write_dword( libspectrum_byte **buffer, libspectrum_dword d );

/* (de)compression routines */

libspectrum_error
libspectrum_uncompress_file( unsigned char **new_buffer, size_t *new_length,
			     char **new_filename, libspectrum_id_t type,
			     const unsigned char *old_buffer,
			     size_t old_length, const char *old_filename );

libspectrum_error
libspectrum_gzip_inflate( const libspectrum_byte *gzptr, size_t gzlength,
			  libspectrum_byte **outptr, size_t *outlength );

libspectrum_error
libspectrum_bzip2_inflate( const libspectrum_byte *bzptr, size_t bzlength,
			   libspectrum_byte **outptr, size_t *outlength );

libspectrum_error
libspectrum_zlib_inflate( const libspectrum_byte *gzptr, size_t gzlength,
        libspectrum_byte **outptr, size_t *outlength );

libspectrum_error
libspectrum_zlib_compress( const libspectrum_byte *data, size_t length,
         libspectrum_byte **gzptr, size_t *gzlength );

/* The TZX file signature */

extern const char *libspectrum_tzx_signature;

/* Convert a 48K memory dump into separate RAM pages */

libspectrum_error libspectrum_split_to_48k_pages( libspectrum_snap *snap,
				    const libspectrum_byte* data );

/* Sizes of some of the arrays in the snap structure */
#define SNAPSHOT_RAM_PAGES 16
#define SNAPSHOT_SLT_PAGES 256
#define SNAPSHOT_ZXATASP_PAGES 32
#define SNAPSHOT_ZXCF_PAGES 64
#define SNAPSHOT_DOCK_EXROM_PAGES 8
#define SNAPSHOT_JOYSTICKS 7
#define SNAPSHOT_DIVIDE_PAGES 4

/* Get memory for a snap */

libspectrum_snap* libspectrum_snap_alloc_internal( void );

/* Format specific snapshot routines */

libspectrum_error
libspectrum_plusd_read( libspectrum_snap *snap,
			const libspectrum_byte *buffer, size_t buffer_length );
libspectrum_error
internal_sna_read( libspectrum_snap *snap,
		   const libspectrum_byte *buffer, size_t buffer_length );
libspectrum_error
libspectrum_sna_write( libspectrum_byte **buffer, size_t *length,
		       int *out_flags, libspectrum_snap *snap, int in_flags );
libspectrum_error
libspectrum_snp_read( libspectrum_snap *snap,
		      const libspectrum_byte *buffer, size_t buffer_length );
libspectrum_error
libspectrum_sp_read( libspectrum_snap *snap,
		     const libspectrum_byte *buffer, size_t buffer_length );
libspectrum_error
libspectrum_szx_read( libspectrum_snap *snap,
		      const libspectrum_byte *buffer, size_t buffer_length );
libspectrum_error
libspectrum_szx_write( libspectrum_byte **buffer, size_t *length,
		       int *out_flags, libspectrum_snap *snap,
		       libspectrum_creator *creator, int in_flags );
libspectrum_error
internal_z80_read( libspectrum_snap *snap,
		   const libspectrum_byte *buffer, size_t buffer_length );
libspectrum_error
libspectrum_z80_write2( libspectrum_byte **buffer, size_t *length,
			int *out_flags, libspectrum_snap *snap, int in_flags );
libspectrum_error
libspectrum_zxs_read( libspectrum_snap *snap,
		      const libspectrum_byte *buffer, size_t buffer_length );

/*** Tape constants ***/

/* The timings for the standard ROM loader */
extern const libspectrum_dword LIBSPECTRUM_TAPE_TIMING_PILOT;
extern const libspectrum_dword LIBSPECTRUM_TAPE_TIMING_SYNC1;
extern const libspectrum_dword LIBSPECTRUM_TAPE_TIMING_SYNC2;
extern const libspectrum_dword LIBSPECTRUM_TAPE_TIMING_DATA0;
extern const libspectrum_dword LIBSPECTRUM_TAPE_TIMING_DATA1;
extern const libspectrum_dword LIBSPECTRUM_TAPE_TIMING_TAIL;

/* Tape routines */

void libspectrum_tape_block_zero( libspectrum_tape_block *block );

libspectrum_error libspectrum_tape_block_read_symbol_table_parameters(
  libspectrum_tape_block *block, int pilot, const libspectrum_byte **ptr );

libspectrum_error
libspectrum_tape_block_read_symbol_table(
  libspectrum_tape_generalised_data_symbol_table *table,
  const libspectrum_byte **ptr, size_t length );

void libspectrum_init_bits_set( void );

/* Format specific tape routines */
  
libspectrum_error
internal_tap_read( libspectrum_tape *tape, const libspectrum_byte *buffer,
		   const size_t length, libspectrum_id_t type );

libspectrum_error
internal_tap_write( libspectrum_byte **buffer, size_t *length,
		    libspectrum_tape *tape, libspectrum_id_t type );

libspectrum_error
internal_tzx_read( libspectrum_tape *tape, const libspectrum_byte *buffer,
		   const size_t length );

libspectrum_error
internal_tzx_write( libspectrum_byte **buffer, size_t *length,
		    libspectrum_tape *tape );

libspectrum_error
internal_warajevo_read( libspectrum_tape *tape,
			const libspectrum_byte *buffer, size_t length );

libspectrum_error
libspectrum_z80em_read( libspectrum_tape *tape,
                        const libspectrum_byte *buffer, size_t length );

libspectrum_error
libspectrum_csw_read( libspectrum_tape *tape,
                      const libspectrum_byte *buffer, size_t length );

libspectrum_error
libspectrum_csw_write( libspectrum_byte **buffer, size_t *length,
                       libspectrum_tape *tape );

libspectrum_error
libspectrum_wav_read( libspectrum_tape *tape, const char *filename );

libspectrum_error
internal_pzx_read( libspectrum_tape *tape, const libspectrum_byte *buffer,
                   const size_t length );

libspectrum_tape_block*
libspectrum_tape_block_internal_init(
                                libspectrum_tape_block_state *iterator,
                                libspectrum_tape *tape );

libspectrum_error
libspectrum_tape_get_next_edge_internal( libspectrum_dword *tstates, int *flags,
                                         libspectrum_tape *tape,
                                         libspectrum_tape_block_state *it );

/* Crypto functions */

libspectrum_error
libspectrum_sign_data( libspectrum_byte **signature, size_t *signature_length,
		       libspectrum_byte *data, size_t data_length,
		       libspectrum_rzx_dsa_key *key );

/* Utility functions */

libspectrum_dword 
libspectrum_ms_to_tstates( libspectrum_dword ms );

libspectrum_dword 
libspectrum_tstates_to_ms( libspectrum_dword tstates );

void
libspectrum_set_pause_ms( libspectrum_tape_block *block,
                          libspectrum_dword pause_ms );

void
libspectrum_set_pause_tstates( libspectrum_tape_block *block,
                               libspectrum_dword pause_tstates );

extern const int LIBSPECTRUM_BITS_IN_BYTE;

/* glib replacement functions */

#ifndef HAVE_LIB_GLIB		/* Only if we are using glib replacement */
void
libspectrum_slist_cleanup( void );

void
libspectrum_hashtable_cleanup( void );
#endif				/* #ifndef HAVE_LIB_GLIB */

#endif				/* #ifndef LIBSPECTRUM_INTERNALS_H */
