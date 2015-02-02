/* szx.c: Routines for .szx snapshots
   Copyright (c) 1998-2012 Philip Kendall, Fredrick Meunier, Stuart Brady

   $Id: szx.c 4904 2013-03-08 20:21:02Z pak21 $

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

#include <string.h>

#include "internals.h"

/* Used for passing internal data around */

typedef struct szx_context {

  int swap_af;

} szx_context;

/* The machine numbers used in the .szx format */

typedef enum szx_machine_type {

  SZX_MACHINE_16 = 0,
  SZX_MACHINE_48,
  SZX_MACHINE_128,
  SZX_MACHINE_PLUS2,
  SZX_MACHINE_PLUS2A,
  SZX_MACHINE_PLUS3,
  SZX_MACHINE_PLUS3E,
  SZX_MACHINE_PENTAGON,
  SZX_MACHINE_TC2048,
  SZX_MACHINE_TC2068,
  SZX_MACHINE_SCORPION,
  SZX_MACHINE_SE,
  SZX_MACHINE_TS2068,
  SZX_MACHINE_PENTAGON512,
  SZX_MACHINE_PENTAGON1024,
  SZX_MACHINE_48_NTSC,
  SZX_MACHINE_128KE,

} szx_machine_type;

static const char *signature = "ZXST";
static const size_t signature_length = 4;

static const libspectrum_byte ZXSTMF_ALTERNATETIMINGS = 1;

static const char *libspectrum_string = "libspectrum: ";

static const libspectrum_byte SZX_VERSION_MAJOR = 1;
static const libspectrum_byte SZX_VERSION_MINOR = 5;

/* Constants etc for each chunk type */

#define ZXSTBID_CREATOR "CRTR"

#define ZXSTBID_Z80REGS "Z80R"
static const libspectrum_byte ZXSTZF_EILAST = 1;
static const libspectrum_byte ZXSTZF_HALTED = 2;
static const libspectrum_byte ZXSTZF_FSET = 4;

#define ZXSTBID_SPECREGS "SPCR"

#define ZXSTBID_RAMPAGE "RAMP"
static const libspectrum_word ZXSTRF_COMPRESSED = 1;

#define ZXSTBID_AY "AY\0\0"
static const libspectrum_byte ZXSTAYF_FULLERBOX = 1;
static const libspectrum_byte ZXSTAYF_128AY = 2;

#define ZXSTBID_MULTIFACE "MFCE"
#define ZXSTBID_USPEECH "USPE"
#define ZXSTBID_SPECDRUM "DRUM"
#define ZXSTBID_ZXTAPE "TAPE"

#define ZXSTBID_KEYBOARD "KEYB"
static const libspectrum_dword ZXSTKF_ISSUE2 = 1;

#define ZXSTBID_JOYSTICK "JOY\0"
static const libspectrum_dword ZXSTJOYF_ALWAYSPORT31 = 1;

typedef enum szx_joystick_type {

  ZXJT_KEMPSTON = 0,
  ZXJT_FULLER,
  ZXJT_CURSOR,
  ZXJT_SINCLAIR1,
  ZXJT_SINCLAIR2,
  ZXJT_SPECTRUMPLUS,
  ZXJT_TIMEX1,
  ZXJT_TIMEX2,
  ZXJT_NONE,

} szx_joystick_type;

#define ZXSTBID_IF2ROM "IF2R"

#define ZXSTBID_MOUSE "AMXM"
typedef enum szx_mouse_type {

  ZXSTM_NONE = 0,
  ZXSTM_AMX,
  ZXSTM_KEMPSTON,

} szx_mouse_type;

#define ZXSTBID_ROM "ROM\0"

#define ZXSTBID_ZXPRINTER "ZXPR"
static const libspectrum_word ZXSTPRF_ENABLED = 1;

#define ZXSTBID_IF1 "IF1\0"
static const libspectrum_word ZXSTIF1F_ENABLED = 1;
static const libspectrum_word ZXSTIF1F_COMPRESSED = 2;
static const libspectrum_word ZXSTIF1F_PAGED = 4;

#define ZXSTBID_MICRODRIVE "MDRV"
#define ZXSTBID_PLUS3DISK "+3\0\0"
#define ZXSTBID_DSKFILE "DSK\0"
#define ZXSTBID_LEC "LEC\0"
static const libspectrum_word ZXSTLECF_PAGED = 1;

#define ZXSTBID_LECRAMPAGE "LCRP"
static const libspectrum_word ZXSTLCRPF_COMPRESSED = 1;

#define ZXSTBID_TIMEXREGS "SCLD"

#define ZXSTBID_BETA128 "B128"
static const libspectrum_dword ZXSTBETAF_CONNECTED = 1;
static const libspectrum_dword ZXSTBETAF_CUSTOMROM = 2;
static const libspectrum_dword ZXSTBETAF_PAGED = 4;
static const libspectrum_dword ZXSTBETAF_AUTOBOOT = 8;
static const libspectrum_dword ZXSTBETAF_SEEKLOWER = 16;
static const libspectrum_dword ZXSTBETAF_COMPRESSED = 32;

#define ZXSTBID_BETADISK "BDSK"
#define ZXSTBID_GS "GS\0\0"
#define ZXSTBID_GSRAMPAGE "GSRP"
#define ZXSTBID_COVOX "COVX"

#define ZXSTBID_DOCK "DOCK"
static const libspectrum_word ZXSTDOCKF_RAM = 2;
static const libspectrum_word ZXSTDOCKF_EXROMDOCK = 4;

#define ZXSTBID_ZXATASP "ZXAT"
static const libspectrum_word ZXSTZXATF_UPLOAD = 1;
static const libspectrum_word ZXSTZXATF_WRITEPROTECT = 2;

#define ZXSTBID_ZXATASPRAMPAGE "ATRP"

#define ZXSTBID_ZXCF "ZXCF"
static const libspectrum_word ZXSTZXCFF_UPLOAD = 1;

#define ZXSTBID_ZXCFRAMPAGE "CFRP"

#define ZXSTBID_PLUSD "PLSD"
static const libspectrum_dword ZXSTPLUSDF_PAGED = 1;
static const libspectrum_dword ZXSTPLUSDF_COMPRESSED = 2;
static const libspectrum_dword ZXSTPLUSDF_SEEKLOWER = 4;
static const libspectrum_byte ZXSTPDRT_GDOS = 0;
static const libspectrum_byte ZXSTPDRT_UNIDOS = 1;
static const libspectrum_byte ZXSTPDRT_CUSTOM = 2;

#define ZXSTBID_PLUSDDISK "PDSK"

#define ZXSTBID_OPUS "OPUS"
static const libspectrum_dword ZXSTOPUSF_PAGED = 1;
static const libspectrum_dword ZXSTOPUSF_COMPRESSED = 2;
static const libspectrum_dword ZXSTOPUSF_SEEKLOWER = 4;
static const libspectrum_dword ZXSTOPUSF_CUSTOMROM = 8;

#define ZXSTBID_OPUSDISK "ODSK"

#define ZXSTBID_SIMPLEIDE "SIDE"
static const libspectrum_word ZXSTSIDE_ENABLED = 1;

#define ZXSTBID_DIVIDE "DIDE"
static const libspectrum_word ZXSTDIVIDE_EPROM_WRITEPROTECT = 1;
static const libspectrum_word ZXSTDIVIDE_PAGED = 2;
static const libspectrum_word ZXSTDIVIDE_COMPRESSED = 4;

#define ZXSTBID_DIVIDERAMPAGE "DIRP"

#define ZXSTBID_SPECTRANET "SNET"
static const libspectrum_word ZXSTSNET_PAGED = 1;
static const libspectrum_word ZXSTSNET_PAGED_VIA_IO = 2;
static const libspectrum_word ZXSTSNET_PROGRAMMABLE_TRAP_ACTIVE = 4;
static const libspectrum_word ZXSTSNET_PROGRAMMABLE_TRAP_MSB = 8;
static const libspectrum_word ZXSTSNET_ALL_DISABLED = 16;
static const libspectrum_word ZXSTSNET_RST8_DISABLED = 32;
static const libspectrum_word ZXSTSNET_DENY_DOWNSTREAM_A15 = 64;
static const libspectrum_word ZXSTSNET_NMI_FLIPFLOP = 128;

#define ZXSTBID_SPECTRANETFLASHPAGE "SNEF"
static const libspectrum_byte ZXSTSNEF_FLASH_COMPRESSED = 1;

#define ZXSTBID_SPECTRANETRAMPAGE "SNER"
static const libspectrum_byte ZXSTSNER_RAM_COMPRESSED = 1;

static libspectrum_error
read_chunk( libspectrum_snap *snap, libspectrum_word version,
	    const libspectrum_byte **buffer, const libspectrum_byte *end,
            szx_context *ctx );

typedef libspectrum_error (*read_chunk_fn)( libspectrum_snap *snap,
					    libspectrum_word version,
					    const libspectrum_byte **buffer,
					    const libspectrum_byte *end,
					    size_t data_length,
                                            szx_context *ctx );

static libspectrum_error
write_file_header( libspectrum_byte **buffer, libspectrum_byte **ptr,
	      size_t *length, int *out_flags, libspectrum_snap *snap );

static libspectrum_error
write_crtr_chunk( libspectrum_byte **bufer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_creator *creator );
static libspectrum_error
write_z80r_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap );
static libspectrum_error
write_spcr_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap );
static libspectrum_error
write_amxm_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap );
static libspectrum_error
write_joy_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, int *out_flags, libspectrum_snap *snap );
static libspectrum_error
write_keyb_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, int *out_flags, libspectrum_snap *snap );
static libspectrum_error
write_ram_pages( libspectrum_byte **buffer, libspectrum_byte **ptr,
		 size_t *length, libspectrum_snap *snap, int compress );
static libspectrum_error
write_ramp_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap, int page,
		  int compress );
static libspectrum_error
write_ram_page( libspectrum_byte **buffer, libspectrum_byte **ptr,
		size_t *length, const char *id, const libspectrum_byte *data,
		size_t data_length, int page, int compress, int extra_flags );
static libspectrum_error
write_rom_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		 size_t *length, int *out_flags, libspectrum_snap *snap,
                 int compress );
static libspectrum_error
write_ay_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		size_t *length, libspectrum_snap *snap );
static libspectrum_error
write_scld_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
                  size_t *length, libspectrum_snap *snap );
static libspectrum_error
write_b128_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap, int compress );
static libspectrum_error
write_opus_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap, int compress  );
static libspectrum_error
write_plsd_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap, int compress  );
static libspectrum_error
write_if1_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		 size_t *length, libspectrum_snap *snap, int compress  );
static libspectrum_error
write_zxat_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap );
static libspectrum_error
write_atrp_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap, int page,
		  int compress );
static libspectrum_error
write_zxcf_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap );
static libspectrum_error
write_cfrp_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap, int page,
		  int compress );
static libspectrum_error
write_side_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap );
static libspectrum_error
write_drum_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap );
static libspectrum_error
write_snet_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap, int compress );
static libspectrum_error
write_snef_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap, int compress );
static libspectrum_error
write_sner_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap, int compress );

#ifdef HAVE_ZLIB_H

static libspectrum_error
write_if2r_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		size_t *length, libspectrum_snap *snap );

#endif				/* #ifdef HAVE_ZLIB_H */

static libspectrum_error
write_dock_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap, int exrom_dock,
                  const libspectrum_byte *data, int page, int writeable,
                  int compress );
static libspectrum_error
write_dide_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap, int compress );
static libspectrum_error
write_dirp_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap, int page,
		  int compress );
static libspectrum_error
write_zxpr_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, int *out_flags, libspectrum_snap *snap );

static void
write_chunk_header( libspectrum_byte **buffer, libspectrum_byte **ptr,
		    size_t *length, const char *id,
		    libspectrum_dword block_length );

static libspectrum_error
read_ram_page( libspectrum_byte **data, size_t *page,
	       const libspectrum_byte **buffer, size_t data_length,
	       size_t uncompressed_length, libspectrum_word *flags )
{
#ifdef HAVE_ZLIB_H

  libspectrum_error error;

#endif			/* #ifdef HAVE_ZLIB_H */

  if( data_length < 3 ) {
    libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
			     "%s:read_ram_page: length %lu too short",
			     __FILE__, (unsigned long)data_length );
    return LIBSPECTRUM_ERROR_UNKNOWN;
  }

  *flags = libspectrum_read_word( buffer );

  *page = **buffer; (*buffer)++;

  if( *flags & ZXSTRF_COMPRESSED ) {

#ifdef HAVE_ZLIB_H

    error = libspectrum_zlib_inflate( *buffer, data_length - 3, data,
				      &uncompressed_length );
    if( error ) return error;

    *buffer += data_length - 3;

#else			/* #ifdef HAVE_ZLIB_H */

    libspectrum_print_error(
      LIBSPECTRUM_ERROR_UNKNOWN,
      "%s:read_ram_page: zlib needed for decompression\n",
      __FILE__
    );
    return LIBSPECTRUM_ERROR_UNKNOWN;

#endif			/* #ifdef HAVE_ZLIB_H */

  } else {

    if( data_length < 3 + uncompressed_length ) {
      libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
			       "%s:read_ram_page: length %lu too short",
			       __FILE__, (unsigned long)data_length );
      return LIBSPECTRUM_ERROR_UNKNOWN;
    }

    *data = libspectrum_malloc( uncompressed_length * sizeof( **data ) );
    memcpy( *data, *buffer, uncompressed_length );
    *buffer += uncompressed_length;

  }

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
read_atrp_chunk( libspectrum_snap *snap, libspectrum_word version GCC_UNUSED,
		 const libspectrum_byte **buffer,
		 const libspectrum_byte *end GCC_UNUSED, size_t data_length,
                 szx_context *ctx GCC_UNUSED )
{
  libspectrum_byte *data;
  size_t page;
  libspectrum_error error;
  libspectrum_word flags;

  error = read_ram_page( &data, &page, buffer, data_length, 0x4000, &flags );
  if( error ) return error;

  if( page >= SNAPSHOT_ZXATASP_PAGES ) {
    libspectrum_print_error( LIBSPECTRUM_ERROR_CORRUPT,
			     "%s:read_atrp_chunk: unknown page number %lu",
			     __FILE__, (unsigned long)page );
    libspectrum_free( data );
    return LIBSPECTRUM_ERROR_CORRUPT;
  }

  libspectrum_snap_set_zxatasp_ram( snap, page, data );

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
read_ay_chunk( libspectrum_snap *snap, libspectrum_word version GCC_UNUSED,
	       const libspectrum_byte **buffer,
	       const libspectrum_byte *end GCC_UNUSED, size_t data_length,
               szx_context *ctx GCC_UNUSED )
{
  size_t i;
  libspectrum_byte flags;

  if( data_length != 18 ) {
    libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
			     "szx_read_ay_chunk: unknown length %lu",
			     (unsigned long)data_length );
    return LIBSPECTRUM_ERROR_UNKNOWN;
  }

  flags = **buffer; (*buffer)++;
  libspectrum_snap_set_fuller_box_active( snap, flags & ZXSTAYF_FULLERBOX );
  libspectrum_snap_set_melodik_active( snap, !!( flags & ZXSTAYF_128AY ) );

  libspectrum_snap_set_out_ay_registerport( snap, **buffer ); (*buffer)++;

  for( i = 0; i < 16; i++ ) {
    libspectrum_snap_set_ay_registers( snap, i, **buffer ); (*buffer)++;
  }

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
read_b128_chunk( libspectrum_snap *snap, libspectrum_word version GCC_UNUSED,
		 const libspectrum_byte **buffer,
		 const libspectrum_byte *end GCC_UNUSED, size_t data_length,
                 szx_context *ctx GCC_UNUSED )
{
#ifdef HAVE_ZLIB_H
  libspectrum_error error;
#endif
  libspectrum_byte *rom_data = NULL;
  libspectrum_dword flags;
  const size_t expected_length = 0x4000;

  if( data_length < 10 ) {
    libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
			     "szx_read_b128_chunk: length %lu too short",
			     (unsigned long)data_length );
    return LIBSPECTRUM_ERROR_UNKNOWN;
  }

  flags = libspectrum_read_dword( buffer );
  libspectrum_snap_set_beta_active( snap, 1 );
  libspectrum_snap_set_beta_paged( snap, flags & ZXSTBETAF_PAGED );
  libspectrum_snap_set_beta_autoboot( snap, flags & ZXSTBETAF_AUTOBOOT );
  libspectrum_snap_set_beta_direction( snap,
				       !( flags & ZXSTBETAF_SEEKLOWER ) );

  libspectrum_snap_set_beta_custom_rom( snap,
                                        !!( flags & ZXSTBETAF_CUSTOMROM ) );

  libspectrum_snap_set_beta_drive_count( snap, **buffer ); (*buffer)++;
  libspectrum_snap_set_beta_system( snap, **buffer ); (*buffer)++;
  libspectrum_snap_set_beta_track ( snap, **buffer ); (*buffer)++;
  libspectrum_snap_set_beta_sector( snap, **buffer ); (*buffer)++;
  libspectrum_snap_set_beta_data  ( snap, **buffer ); (*buffer)++;
  libspectrum_snap_set_beta_status( snap, **buffer ); (*buffer)++;

  if( libspectrum_snap_beta_custom_rom( snap ) ) {

    if( flags & ZXSTBETAF_COMPRESSED ) {

#ifdef HAVE_ZLIB_H

      size_t uncompressed_length = 0;

      error = libspectrum_zlib_inflate( *buffer, data_length - 10, &rom_data,
					&uncompressed_length );
      if( error ) return error;

      if( uncompressed_length != expected_length ) {
	libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
				 "%s:read_b128_chunk: invalid ROM length "
				 "in compressed file, should be %lu, file "
				 "has %lu",
				 __FILE__,
				 (unsigned long)expected_length,
				 (unsigned long)uncompressed_length );
	return LIBSPECTRUM_ERROR_UNKNOWN;
      }

#else

      libspectrum_print_error(
	LIBSPECTRUM_ERROR_UNKNOWN,
	"%s:read_b128_chunk: zlib needed for decompression\n",
	__FILE__
      );
      return LIBSPECTRUM_ERROR_UNKNOWN;

#endif

    } else {

      if( data_length < 10 + expected_length ) {
        libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
				 "%s:read_b128_chunk: length %lu too short, "
				 "expected %lu",
				 __FILE__, (unsigned long)data_length,
				 (unsigned long)10 + expected_length );
	return LIBSPECTRUM_ERROR_UNKNOWN;
      }

      rom_data = libspectrum_malloc( expected_length );
      memcpy( rom_data, *buffer, expected_length );

    }

  }

  libspectrum_snap_set_beta_rom( snap, 0, rom_data );

  /* Skip any extra data (most likely a custom ROM) */
  *buffer += data_length - 10;

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
read_crtr_chunk( libspectrum_snap *snap, libspectrum_word version GCC_UNUSED,
		 const libspectrum_byte **buffer,
		 const libspectrum_byte *end GCC_UNUSED, size_t data_length,
                 szx_context *ctx )
{
  if( data_length < 36 ) {
    libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
			     "%s:read_crtr_chunk: length %lu too short",
			     __FILE__, (unsigned long)data_length );
    return LIBSPECTRUM_ERROR_UNKNOWN;
  }

  *buffer += 36;
  data_length -= 36;

  /* This is ugly, but I can't see a better way to do it */
  if( sizeof( libspectrum_byte ) == sizeof( char ) ) {
    char *custom = libspectrum_malloc( data_length + 1 );
    memcpy( custom, *buffer, data_length );
    custom[data_length] = 0;

    char *libspectrum = strstr( custom, libspectrum_string );
    if( libspectrum ) {
      int matches, v1, v2, v3;
      libspectrum += strlen( libspectrum_string );
      matches = sscanf( libspectrum, "%d.%d.%d", &v1, &v2, &v3 );
      if( matches == 3 ) {
        if( v1 == 0 ) {
          if( v2 < 5 || ( v2 == 5 && v3 == 0 ) ) {
            ctx->swap_af = 1;
          }
        }
      }
    }

    libspectrum_free( custom );
  }

  *buffer += data_length;

  return LIBSPECTRUM_ERROR_NONE;
}
     
static libspectrum_error
read_opus_chunk( libspectrum_snap *snap, libspectrum_word version GCC_UNUSED,
		 const libspectrum_byte **buffer,
		 const libspectrum_byte *end GCC_UNUSED, size_t data_length,
                 szx_context *ctx GCC_UNUSED )
{
#ifdef HAVE_ZLIB_H
  libspectrum_error error;
#endif
  libspectrum_byte *ram_data = NULL, *rom_data = NULL;
  libspectrum_dword flags;
  size_t disc_ram_length;
  size_t disc_rom_length;
  size_t expected_length = 0x800;

  if( data_length < 23 ) {
    libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
			     "szx_read_opus_chunk: length %lu too short",
			     (unsigned long)data_length );
    return LIBSPECTRUM_ERROR_UNKNOWN;
  }

  libspectrum_snap_set_opus_active( snap, 1 );

  flags = libspectrum_read_dword( buffer );
  libspectrum_snap_set_opus_paged( snap, flags & ZXSTOPUSF_PAGED );
  libspectrum_snap_set_opus_direction( snap,
				       !( flags & ZXSTOPUSF_SEEKLOWER ) );

  disc_ram_length = libspectrum_read_dword( buffer );
  disc_rom_length = libspectrum_read_dword( buffer );

  libspectrum_snap_set_opus_custom_rom( snap, flags & ZXSTOPUSF_CUSTOMROM );
  if( libspectrum_snap_opus_custom_rom( snap ) && !disc_rom_length ) {
    libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
			     "szx_read_opus_chunk: block flagged as custom "
                             "ROM but there is no custom ROM stored in the "
                             "snapshot" );
    return LIBSPECTRUM_ERROR_UNKNOWN;
  }

  libspectrum_snap_set_opus_control_a( snap, **buffer ); (*buffer)++;
  libspectrum_snap_set_opus_data_reg_a( snap, **buffer ); (*buffer)++;
  libspectrum_snap_set_opus_data_dir_a( snap, **buffer ); (*buffer)++;
  libspectrum_snap_set_opus_control_b( snap, **buffer ); (*buffer)++;
  libspectrum_snap_set_opus_data_reg_b( snap, **buffer ); (*buffer)++;
  libspectrum_snap_set_opus_data_dir_b( snap, **buffer ); (*buffer)++;
  libspectrum_snap_set_opus_drive_count( snap, **buffer ); (*buffer)++;
  libspectrum_snap_set_opus_track ( snap, **buffer ); (*buffer)++;
  libspectrum_snap_set_opus_sector( snap, **buffer ); (*buffer)++;
  libspectrum_snap_set_opus_data  ( snap, **buffer ); (*buffer)++;
  libspectrum_snap_set_opus_status( snap, **buffer ); (*buffer)++;

  if( flags & ZXSTOPUSF_COMPRESSED ) {

#ifdef HAVE_ZLIB_H

    size_t uncompressed_length = 0;

    if( (!libspectrum_snap_opus_custom_rom( snap ) &&
         disc_rom_length != 0 ) ||
        (libspectrum_snap_opus_custom_rom( snap ) &&
         disc_rom_length == 0 ) ) {
      libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
			       "%s:read_opus_chunk: invalid ROM length "
                               "in compressed file, should be %lu, file "
                               "has %lu",
			       __FILE__, 
                               0UL,
                               (unsigned long)disc_rom_length );
      return LIBSPECTRUM_ERROR_UNKNOWN;
    }

    if( data_length < 23 + disc_ram_length + disc_rom_length ) {
      libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
			       "%s:read_opus_chunk: length %lu too short, "
			       "expected %lu" ,
			       __FILE__, (unsigned long)data_length,
			       (unsigned long)23 + disc_ram_length +
                                 disc_rom_length );
      return LIBSPECTRUM_ERROR_UNKNOWN;
    }

    error = libspectrum_zlib_inflate( *buffer, disc_ram_length, &ram_data,
				      &uncompressed_length );
    if( error ) return error;

    if( uncompressed_length != expected_length ) {
      libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
			       "%s:read_opus_chunk: invalid RAM length "
                               "in compressed file, should be %lu, file "
                               "has %lu",
			       __FILE__, 
                               (unsigned long)expected_length,
                               (unsigned long)uncompressed_length );
      return LIBSPECTRUM_ERROR_UNKNOWN;
    }

    *buffer += disc_ram_length;

    if( libspectrum_snap_opus_custom_rom( snap ) ) {
      error = libspectrum_zlib_inflate( *buffer, disc_rom_length, &rom_data,
                                        &uncompressed_length );
      if( error ) return error;

      expected_length = 0x2000;

      if( uncompressed_length != expected_length ) {
        libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
                                 "%s:read_plsd_chunk: invalid ROM length "
                                 "in compressed file, should be %lu, file "
                                 "has %lu",
                                 __FILE__, 
                                 (unsigned long)expected_length,
                                 (unsigned long)disc_rom_length );
        return LIBSPECTRUM_ERROR_UNKNOWN;
      }

      *buffer += disc_rom_length;
    }

#else			/* #ifdef HAVE_ZLIB_H */

    libspectrum_print_error(
      LIBSPECTRUM_ERROR_UNKNOWN,
      "%s:read_opus_chunk: zlib needed for decompression\n",
      __FILE__
    );
    return LIBSPECTRUM_ERROR_UNKNOWN;

#endif			/* #ifdef HAVE_ZLIB_H */

  } else {

    if( disc_ram_length != expected_length ) {
      libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
			       "%s:read_opus_chunk: invalid RAM length "
                               "in uncompressed file, should be %lu, file "
                               "has %lu",
			       __FILE__, 
                               (unsigned long)expected_length,
                               (unsigned long)disc_rom_length );
      return LIBSPECTRUM_ERROR_UNKNOWN;
    }

    expected_length = 0x2000;

    if( (libspectrum_snap_opus_custom_rom( snap ) &&
         disc_rom_length != expected_length ) ||
        (!libspectrum_snap_opus_custom_rom( snap ) &&
         disc_rom_length != 0 ) ) {
      libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
			       "%s:read_opus_chunk: invalid ROM length "
                               "in uncompressed file, should be %lu, file "
                               "has %lu",
			       __FILE__, 
                               libspectrum_snap_opus_custom_rom( snap ) ?
                                 expected_length : 0UL,
                               (unsigned long)disc_rom_length );
      return LIBSPECTRUM_ERROR_UNKNOWN;
    }

    if( data_length < 23 + disc_ram_length + disc_rom_length ) {
      libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
			       "%s:read_opus_chunk: length %lu too short, "
			       "expected %lu" ,
			       __FILE__, (unsigned long)data_length,
			       (unsigned long)23 + disc_ram_length + disc_rom_length );
      return LIBSPECTRUM_ERROR_UNKNOWN;
    }

    expected_length = 0x800;

    ram_data = libspectrum_malloc( expected_length );
    memcpy( ram_data, *buffer, expected_length );
    *buffer += expected_length;

    if( libspectrum_snap_opus_custom_rom( snap ) ) {
      expected_length = 0x2000;
      rom_data = libspectrum_malloc( expected_length );
      memcpy( rom_data, *buffer, expected_length );
      *buffer += expected_length;
    }

  }

  libspectrum_snap_set_opus_ram( snap, 0, ram_data );
  libspectrum_snap_set_opus_rom( snap, 0, rom_data );

  return LIBSPECTRUM_ERROR_NONE;
}
     
static libspectrum_error
read_plsd_chunk( libspectrum_snap *snap, libspectrum_word version GCC_UNUSED,
		  const libspectrum_byte **buffer,
		  const libspectrum_byte *end GCC_UNUSED, size_t data_length,
                 szx_context *ctx GCC_UNUSED )
{
#ifdef HAVE_ZLIB_H
  libspectrum_error error;
#endif
  libspectrum_byte *ram_data = NULL, *rom_data = NULL;
  libspectrum_byte rom_type;
  libspectrum_dword flags;
  size_t disc_ram_length;
  size_t disc_rom_length;
  const size_t expected_length = 0x2000;

  if( data_length < 19 ) {
    libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
			     "szx_read_plusd_chunk: length %lu too short",
			     (unsigned long)data_length );
    return LIBSPECTRUM_ERROR_UNKNOWN;
  }

  libspectrum_snap_set_plusd_active( snap, 1 );

  flags = libspectrum_read_dword( buffer );
  libspectrum_snap_set_plusd_paged( snap, flags & ZXSTPLUSDF_PAGED );
  libspectrum_snap_set_plusd_direction( snap,
				       !( flags & ZXSTPLUSDF_SEEKLOWER ) );

  disc_ram_length = libspectrum_read_dword( buffer );
  disc_rom_length = libspectrum_read_dword( buffer );
  rom_type = *(*buffer)++;

  libspectrum_snap_set_plusd_custom_rom( snap, rom_type == ZXSTPDRT_CUSTOM );
  if( libspectrum_snap_plusd_custom_rom( snap ) && !disc_rom_length ) {
    libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
			     "szx_read_plusd_chunk: block flagged as custom "
                             "ROM but there is no custom ROM stored in the "
                             "snapshot" );
    return LIBSPECTRUM_ERROR_UNKNOWN;
  }

  libspectrum_snap_set_plusd_control( snap, **buffer ); (*buffer)++;
  libspectrum_snap_set_plusd_drive_count( snap, **buffer ); (*buffer)++;
  libspectrum_snap_set_plusd_track ( snap, **buffer ); (*buffer)++;
  libspectrum_snap_set_plusd_sector( snap, **buffer ); (*buffer)++;
  libspectrum_snap_set_plusd_data  ( snap, **buffer ); (*buffer)++;
  libspectrum_snap_set_plusd_status( snap, **buffer ); (*buffer)++;

  if( flags & ZXSTPLUSDF_COMPRESSED ) {

#ifdef HAVE_ZLIB_H

    size_t uncompressed_length = 0;

    if( (!libspectrum_snap_plusd_custom_rom( snap ) &&
         disc_rom_length != 0 ) ||
        (libspectrum_snap_plusd_custom_rom( snap ) &&
         disc_rom_length == 0 ) ) {
      libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
			       "%s:read_plsd_chunk: invalid ROM length "
                               "in compressed file, should be %lu, file "
                               "has %lu",
			       __FILE__, 
                               0UL,
                               (unsigned long)disc_rom_length );
      return LIBSPECTRUM_ERROR_UNKNOWN;
    }

    if( data_length < 19 + disc_ram_length + disc_rom_length ) {
      libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
			       "%s:read_plsd_chunk: length %lu too short, "
			       "expected %lu" ,
			       __FILE__, (unsigned long)data_length,
			       (unsigned long)19 + disc_ram_length +
                                 disc_rom_length );
      return LIBSPECTRUM_ERROR_UNKNOWN;
    }

    error = libspectrum_zlib_inflate( *buffer, disc_ram_length, &ram_data,
				      &uncompressed_length );
    if( error ) return error;

    if( uncompressed_length != expected_length ) {
      libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
			       "%s:read_plsd_chunk: invalid RAM length "
                               "in compressed file, should be %lu, file "
                               "has %lu",
			       __FILE__, 
                               (unsigned long)expected_length,
                               (unsigned long)uncompressed_length );
      return LIBSPECTRUM_ERROR_UNKNOWN;
    }

    *buffer += disc_ram_length;

    if( libspectrum_snap_plusd_custom_rom( snap ) ) {
      error = libspectrum_zlib_inflate( *buffer, disc_rom_length, &rom_data,
                                        &uncompressed_length );
      if( error ) return error;

      if( uncompressed_length != expected_length ) {
        libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
                                 "%s:read_plsd_chunk: invalid ROM length "
                                 "in compressed file, should be %lu, file "
                                 "has %lu",
                                 __FILE__, 
                                 (unsigned long)expected_length,
                                 (unsigned long)disc_rom_length );
        return LIBSPECTRUM_ERROR_UNKNOWN;
      }

      *buffer += disc_rom_length;
    }

#else			/* #ifdef HAVE_ZLIB_H */

    libspectrum_print_error(
      LIBSPECTRUM_ERROR_UNKNOWN,
      "%s:read_plsd_chunk: zlib needed for decompression\n",
      __FILE__
    );
    return LIBSPECTRUM_ERROR_UNKNOWN;

#endif			/* #ifdef HAVE_ZLIB_H */

  } else {

    if( disc_ram_length != expected_length ) {
      libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
			       "%s:read_plsd_chunk: invalid RAM length "
                               "in uncompressed file, should be %lu, file "
                               "has %lu",
			       __FILE__, 
                               (unsigned long)expected_length,
                               (unsigned long)disc_rom_length );
      return LIBSPECTRUM_ERROR_UNKNOWN;
    }

    if( (libspectrum_snap_plusd_custom_rom( snap ) &&
         disc_rom_length != expected_length ) ||
        (!libspectrum_snap_plusd_custom_rom( snap ) &&
         disc_rom_length != 0 ) ) {
      libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
			       "%s:read_plsd_chunk: invalid ROM length "
                               "in uncompressed file, should be %lu, file "
                               "has %lu",
			       __FILE__, 
                               libspectrum_snap_plusd_custom_rom( snap ) ?
                                 expected_length : 0UL,
                               (unsigned long)disc_rom_length );
      return LIBSPECTRUM_ERROR_UNKNOWN;
    }

    if( data_length < 19 + disc_ram_length + disc_rom_length ) {
      libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
			       "%s:read_plsd_chunk: length %lu too short, "
			       "expected %lu" ,
			       __FILE__, (unsigned long)data_length,
			       (unsigned long)19 + disc_ram_length + disc_rom_length );
      return LIBSPECTRUM_ERROR_UNKNOWN;
    }

    ram_data = libspectrum_malloc( expected_length );
    memcpy( ram_data, *buffer, expected_length );
    *buffer += expected_length;

    if( libspectrum_snap_plusd_custom_rom( snap ) ) {
      rom_data = libspectrum_malloc( expected_length );
      memcpy( rom_data, *buffer, expected_length );
      *buffer += expected_length;
    }

  }

  libspectrum_snap_set_plusd_ram( snap, 0, ram_data );
  libspectrum_snap_set_plusd_rom( snap, 0, rom_data );

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
read_cfrp_chunk( libspectrum_snap *snap, libspectrum_word version GCC_UNUSED,
		 const libspectrum_byte **buffer,
		 const libspectrum_byte *end GCC_UNUSED, size_t data_length,
                 szx_context *ctx GCC_UNUSED )
{
  libspectrum_byte *data;
  size_t page;
  libspectrum_error error;
  libspectrum_word flags;

  error = read_ram_page( &data, &page, buffer, data_length, 0x4000, &flags );
  if( error ) return error;

  if( page >= SNAPSHOT_ZXCF_PAGES ) {
    libspectrum_print_error( LIBSPECTRUM_ERROR_CORRUPT,
			     "%s:read_cfrp_chunk: unknown page number %lu",
			     __FILE__, (unsigned long)page );
    libspectrum_free( data );
    return LIBSPECTRUM_ERROR_CORRUPT;
  }

  libspectrum_snap_set_zxcf_ram( snap, page, data );

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
read_side_chunk( libspectrum_snap *snap, libspectrum_word version GCC_UNUSED,
		 const libspectrum_byte **buffer,
		 const libspectrum_byte *end GCC_UNUSED, size_t data_length,
                 szx_context *ctx GCC_UNUSED )
{
  if( data_length ) {
    libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
			     "%s:read_side_chunk: unknown length %lu",
			     __FILE__, (unsigned long)data_length );
    return LIBSPECTRUM_ERROR_UNKNOWN;
  }

  libspectrum_snap_set_simpleide_active( snap, 1 );

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
read_drum_chunk( libspectrum_snap *snap, libspectrum_word version GCC_UNUSED,
		 const libspectrum_byte **buffer,
		 const libspectrum_byte *end GCC_UNUSED, size_t data_length,
                 szx_context *ctx GCC_UNUSED )
{
  if( data_length != 1 ) {
    libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
			     "%s:read_drum_chunk: unknown length %lu",
			     __FILE__, (unsigned long)data_length );
    return LIBSPECTRUM_ERROR_UNKNOWN;
  }

  libspectrum_snap_set_specdrum_dac( snap, *(*buffer)++ );

  libspectrum_snap_set_specdrum_active( snap, 1 );

  return LIBSPECTRUM_ERROR_NONE;
}


static void
add_joystick( libspectrum_snap *snap, libspectrum_joystick type, int inputs )
{
  size_t i;
  size_t num_joysticks = libspectrum_snap_joystick_active_count( snap );

  for( i = 0; i < num_joysticks; i++ ) {
    if( libspectrum_snap_joystick_list( snap, i ) == type ) {
      libspectrum_snap_set_joystick_inputs( snap, i, inputs |
                                  libspectrum_snap_joystick_inputs( snap, i ) );
      return;
    }
  }

  libspectrum_snap_set_joystick_list( snap, num_joysticks, type );
  libspectrum_snap_set_joystick_inputs( snap, num_joysticks, inputs );
  libspectrum_snap_set_joystick_active_count( snap, num_joysticks + 1 );
}

static libspectrum_error
read_joy_chunk( libspectrum_snap *snap, libspectrum_word version,
		 const libspectrum_byte **buffer,
		 const libspectrum_byte *end GCC_UNUSED, size_t data_length,
                 szx_context *ctx GCC_UNUSED )
{
  libspectrum_dword flags;

  if( data_length != 6 ) {
    libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
			     "%s:read_joy_chunk: unknown length %lu",
			     __FILE__, (unsigned long)data_length );
    return LIBSPECTRUM_ERROR_UNKNOWN;
  }

  flags = libspectrum_read_dword( buffer );
  if( flags & ZXSTJOYF_ALWAYSPORT31 ) {
    add_joystick( snap, LIBSPECTRUM_JOYSTICK_KEMPSTON,
                  LIBSPECTRUM_JOYSTICK_INPUT_NONE );
  }

  switch( **buffer ) {
  case ZXJT_KEMPSTON:
    add_joystick( snap, LIBSPECTRUM_JOYSTICK_KEMPSTON,
                  LIBSPECTRUM_JOYSTICK_INPUT_JOYSTICK_1 );
    break;
  case ZXJT_FULLER:
    add_joystick( snap, LIBSPECTRUM_JOYSTICK_FULLER,
                  LIBSPECTRUM_JOYSTICK_INPUT_JOYSTICK_1 );
    break;
  case ZXJT_CURSOR:
    add_joystick( snap, LIBSPECTRUM_JOYSTICK_CURSOR,
                  LIBSPECTRUM_JOYSTICK_INPUT_JOYSTICK_1 );
    break;
  case ZXJT_SINCLAIR1:
    add_joystick( snap, LIBSPECTRUM_JOYSTICK_SINCLAIR_1,
                  LIBSPECTRUM_JOYSTICK_INPUT_JOYSTICK_1 );
    break;
  case ZXJT_SINCLAIR2:
    add_joystick( snap, LIBSPECTRUM_JOYSTICK_SINCLAIR_2,
                  LIBSPECTRUM_JOYSTICK_INPUT_JOYSTICK_1 );
    break;
  case ZXJT_TIMEX1:
    add_joystick( snap, LIBSPECTRUM_JOYSTICK_TIMEX_1,
                  LIBSPECTRUM_JOYSTICK_INPUT_JOYSTICK_1 );
    break;
  case ZXJT_TIMEX2:
    add_joystick( snap, LIBSPECTRUM_JOYSTICK_TIMEX_2,
                  LIBSPECTRUM_JOYSTICK_INPUT_JOYSTICK_1 );
    break;
  case ZXJT_NONE:
    break;
  }
  (*buffer)++;

  switch( **buffer ) {
  case ZXJT_KEMPSTON:
    add_joystick( snap, LIBSPECTRUM_JOYSTICK_KEMPSTON,
                  LIBSPECTRUM_JOYSTICK_INPUT_JOYSTICK_2 );
    break;
  case ZXJT_FULLER:
    add_joystick( snap, LIBSPECTRUM_JOYSTICK_FULLER,
                  LIBSPECTRUM_JOYSTICK_INPUT_JOYSTICK_2 );
    break;
  case ZXJT_CURSOR:
    add_joystick( snap, LIBSPECTRUM_JOYSTICK_CURSOR,
                  LIBSPECTRUM_JOYSTICK_INPUT_JOYSTICK_2 );
    break;
  case ZXJT_SINCLAIR1:
    add_joystick( snap, LIBSPECTRUM_JOYSTICK_SINCLAIR_1,
                  LIBSPECTRUM_JOYSTICK_INPUT_JOYSTICK_2 );
    break;
  case ZXJT_SINCLAIR2:
    add_joystick( snap, LIBSPECTRUM_JOYSTICK_SINCLAIR_2,
                  LIBSPECTRUM_JOYSTICK_INPUT_JOYSTICK_2 );
    break;
  case ZXJT_TIMEX1:
    add_joystick( snap, LIBSPECTRUM_JOYSTICK_TIMEX_1,
                  LIBSPECTRUM_JOYSTICK_INPUT_JOYSTICK_2 );
    break;
  case ZXJT_TIMEX2:
    add_joystick( snap, LIBSPECTRUM_JOYSTICK_TIMEX_2,
                  LIBSPECTRUM_JOYSTICK_INPUT_JOYSTICK_2 );
    break;
  case ZXJT_NONE:
    break;
  }
  (*buffer)++;

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
read_keyb_chunk( libspectrum_snap *snap, libspectrum_word version,
		 const libspectrum_byte **buffer,
		 const libspectrum_byte *end GCC_UNUSED, size_t data_length,
                 szx_context *ctx GCC_UNUSED )
{
  size_t expected_length;
  libspectrum_dword flags;

  expected_length = version >= 0x0101 ? 5 : 4;

  if( data_length != expected_length ) {
    libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
			     "%s:read_keyb_chunk: unknown length %lu",
			     __FILE__, (unsigned long)data_length );
    return LIBSPECTRUM_ERROR_UNKNOWN;
  }

  flags = libspectrum_read_dword( buffer );
  libspectrum_snap_set_issue2( snap, flags & ZXSTKF_ISSUE2 );

  if( expected_length >= 5 ) {
    switch( **buffer ) {
    case ZXJT_KEMPSTON:
      add_joystick( snap, LIBSPECTRUM_JOYSTICK_KEMPSTON,
                    LIBSPECTRUM_JOYSTICK_INPUT_KEYBOARD );
      break;
    case ZXJT_FULLER:
      add_joystick( snap, LIBSPECTRUM_JOYSTICK_FULLER,
                    LIBSPECTRUM_JOYSTICK_INPUT_KEYBOARD );
      break;
    case ZXJT_CURSOR:
      add_joystick( snap, LIBSPECTRUM_JOYSTICK_CURSOR,
                    LIBSPECTRUM_JOYSTICK_INPUT_KEYBOARD );
      break;
    case ZXJT_SINCLAIR1:
      add_joystick( snap, LIBSPECTRUM_JOYSTICK_SINCLAIR_1,
                    LIBSPECTRUM_JOYSTICK_INPUT_KEYBOARD );
      break;
    case ZXJT_SINCLAIR2:
      add_joystick( snap, LIBSPECTRUM_JOYSTICK_SINCLAIR_2,
                    LIBSPECTRUM_JOYSTICK_INPUT_KEYBOARD );
      break;
    case ZXJT_TIMEX1:
      add_joystick( snap, LIBSPECTRUM_JOYSTICK_TIMEX_1,
                    LIBSPECTRUM_JOYSTICK_INPUT_KEYBOARD );
      break;
    case ZXJT_TIMEX2:
      add_joystick( snap, LIBSPECTRUM_JOYSTICK_TIMEX_2,
                    LIBSPECTRUM_JOYSTICK_INPUT_KEYBOARD );
      break;
    case ZXJT_SPECTRUMPLUS: /* Actually, no joystick at all */
      break;
    }
    (*buffer)++;
  }

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
read_amxm_chunk( libspectrum_snap *snap, libspectrum_word version,
		 const libspectrum_byte **buffer,
		 const libspectrum_byte *end GCC_UNUSED, size_t data_length,
                 szx_context *ctx GCC_UNUSED )
{
  if( data_length != 7 ) {
    libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
			     "read_amxm_chunk: unknown length %lu",
			     (unsigned long)data_length );
    return LIBSPECTRUM_ERROR_UNKNOWN;
  }

  switch( **buffer ) {
  case ZXSTM_AMX:
    break;
  case ZXSTM_KEMPSTON:
    libspectrum_snap_set_kempston_mouse_active( snap, 1 );
    break;
  case ZXSTM_NONE:
  default:
    break;
  }
  (*buffer)++;

  /* Z80 PIO CTRLA registers for AMX mouse */
  (*buffer)++; (*buffer)++; (*buffer)++;

  /* Z80 PIO CTRLB registers for AMX mouse */
  (*buffer)++; (*buffer)++; (*buffer)++;

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
read_ramp_chunk( libspectrum_snap *snap, libspectrum_word version GCC_UNUSED,
		 const libspectrum_byte **buffer,
		 const libspectrum_byte *end GCC_UNUSED, size_t data_length,
                 szx_context *ctx GCC_UNUSED )
{
  libspectrum_byte *data;
  size_t page;
  libspectrum_error error;
  libspectrum_word flags;


  error = read_ram_page( &data, &page, buffer, data_length, 0x4000, &flags );
  if( error ) return error;

  if( page > 63 ) {
    libspectrum_print_error( LIBSPECTRUM_ERROR_CORRUPT,
			     "%s:read_ramp_chunk: unknown page number %lu",
			     __FILE__, (unsigned long)page );
    libspectrum_free( data );
    return LIBSPECTRUM_ERROR_CORRUPT;
  }

  libspectrum_snap_set_pages( snap, page, data );

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
read_scld_chunk( libspectrum_snap *snap, libspectrum_word version GCC_UNUSED,
		 const libspectrum_byte **buffer,
		 const libspectrum_byte *end GCC_UNUSED, size_t data_length,
                 szx_context *ctx GCC_UNUSED )
{
  if( data_length != 2 ) {
    libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
			     "szx_read_scld_chunk: unknown length %lu",
			     (unsigned long)data_length );
    return LIBSPECTRUM_ERROR_UNKNOWN;
  }

  libspectrum_snap_set_out_scld_hsr( snap, **buffer ); (*buffer)++;
  libspectrum_snap_set_out_scld_dec( snap, **buffer ); (*buffer)++;

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
read_spcr_chunk( libspectrum_snap *snap, libspectrum_word version,
		 const libspectrum_byte **buffer,
		 const libspectrum_byte *end GCC_UNUSED, size_t data_length,
                 szx_context *ctx GCC_UNUSED )
{
  libspectrum_byte out_ula;
  int capabilities;

  if( data_length != 8 ) {
    libspectrum_print_error(
      LIBSPECTRUM_ERROR_UNKNOWN,
      "szx_read_spcr_chunk: unknown length %lu", (unsigned long)data_length
    );
    return LIBSPECTRUM_ERROR_UNKNOWN;
  }

  capabilities =
    libspectrum_machine_capabilities( libspectrum_snap_machine( snap ) );

  out_ula = **buffer & 0x07; (*buffer)++;

  libspectrum_snap_set_out_128_memoryport( snap, **buffer ); (*buffer)++;

  if( ( capabilities & LIBSPECTRUM_MACHINE_CAPABILITY_PLUS3_MEMORY )    ||
      ( capabilities & LIBSPECTRUM_MACHINE_CAPABILITY_SCORP_MEMORY )    ||
      ( capabilities & LIBSPECTRUM_MACHINE_CAPABILITY_PENT1024_MEMORY )    )
    libspectrum_snap_set_out_plus3_memoryport( snap, **buffer );
  (*buffer)++;

  if( version >= 0x0101 ) out_ula |= **buffer & 0xf8;
  (*buffer)++;

  libspectrum_snap_set_out_ula( snap, out_ula );

  *buffer += 4;			/* Skip 'reserved' data */

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
read_z80r_chunk( libspectrum_snap *snap, libspectrum_word version,
		 const libspectrum_byte **buffer,
		 const libspectrum_byte *end GCC_UNUSED, size_t data_length,
                 szx_context *ctx )
{
  if( data_length != 37 ) {
    libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
			     "szx_read_z80r_chunk: unknown length %lu",
			     (unsigned long)data_length );
    return LIBSPECTRUM_ERROR_UNKNOWN;
  }

  if( ctx->swap_af ) {
    libspectrum_snap_set_a( snap, **buffer ); (*buffer)++;
    libspectrum_snap_set_f( snap, **buffer ); (*buffer)++;
  } else {
    libspectrum_snap_set_f( snap, **buffer ); (*buffer)++;
    libspectrum_snap_set_a( snap, **buffer ); (*buffer)++;
  }

  libspectrum_snap_set_bc  ( snap, libspectrum_read_word( buffer ) );
  libspectrum_snap_set_de  ( snap, libspectrum_read_word( buffer ) );
  libspectrum_snap_set_hl  ( snap, libspectrum_read_word( buffer ) );

  if( ctx->swap_af ) {
    libspectrum_snap_set_a_( snap, **buffer ); (*buffer)++;
    libspectrum_snap_set_f_( snap, **buffer ); (*buffer)++;
  } else {
    libspectrum_snap_set_f_( snap, **buffer ); (*buffer)++;
    libspectrum_snap_set_a_( snap, **buffer ); (*buffer)++;
  }

  libspectrum_snap_set_bc_ ( snap, libspectrum_read_word( buffer ) );
  libspectrum_snap_set_de_ ( snap, libspectrum_read_word( buffer ) );
  libspectrum_snap_set_hl_ ( snap, libspectrum_read_word( buffer ) );

  libspectrum_snap_set_ix  ( snap, libspectrum_read_word( buffer ) );
  libspectrum_snap_set_iy  ( snap, libspectrum_read_word( buffer ) );
  libspectrum_snap_set_sp  ( snap, libspectrum_read_word( buffer ) );
  libspectrum_snap_set_pc  ( snap, libspectrum_read_word( buffer ) );

  libspectrum_snap_set_i   ( snap, **buffer ); (*buffer)++;
  libspectrum_snap_set_r   ( snap, **buffer ); (*buffer)++;
  libspectrum_snap_set_iff1( snap, **buffer ); (*buffer)++;
  libspectrum_snap_set_iff2( snap, **buffer ); (*buffer)++;
  libspectrum_snap_set_im  ( snap, **buffer ); (*buffer)++;

  libspectrum_snap_set_tstates( snap, libspectrum_read_dword( buffer ) );

  if( version >= 0x0101 ) {
    (*buffer)++;		/* Skip chHoldIntReqCycles */
    
    /* Flags */
    libspectrum_snap_set_last_instruction_ei( snap, **buffer & ZXSTZF_EILAST );
    libspectrum_snap_set_halted( snap, **buffer & ZXSTZF_HALTED );
    libspectrum_snap_set_last_instruction_set_f( snap, **buffer & ZXSTZF_FSET );
    (*buffer)++;

    if( version >= 0x0104 ) {
      libspectrum_snap_set_memptr( snap, libspectrum_read_word( buffer ) );
    } else {
      (*buffer)++;		/* Skip the hidden register */
      (*buffer)++;		/* Skip the reserved byte */
    }

  } else {
    *buffer += 4;		/* Skip the reserved dword */
  }

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
read_zxat_chunk( libspectrum_snap *snap, libspectrum_word version GCC_UNUSED,
		 const libspectrum_byte **buffer,
		 const libspectrum_byte *end GCC_UNUSED, size_t data_length,
                 szx_context *ctx GCC_UNUSED )
{
  libspectrum_word flags;

  if( data_length != 8 ) {
    libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
			     "%s:read_zxat_chunk: unknown length %lu",
			     __FILE__, (unsigned long)data_length );
    return LIBSPECTRUM_ERROR_UNKNOWN;
  }

  libspectrum_snap_set_zxatasp_active( snap, 1 );

  flags = libspectrum_read_word( buffer );
  libspectrum_snap_set_zxatasp_upload( snap, flags & ZXSTZXATF_UPLOAD );
  libspectrum_snap_set_zxatasp_writeprotect( snap,
					     flags & ZXSTZXATF_WRITEPROTECT );

  libspectrum_snap_set_zxatasp_port_a( snap, **buffer ); (*buffer)++;
  libspectrum_snap_set_zxatasp_port_b( snap, **buffer ); (*buffer)++;
  libspectrum_snap_set_zxatasp_port_c( snap, **buffer ); (*buffer)++;
  libspectrum_snap_set_zxatasp_control( snap, **buffer ); (*buffer)++;
  libspectrum_snap_set_zxatasp_pages( snap, **buffer ); (*buffer)++;
  libspectrum_snap_set_zxatasp_current_page( snap, **buffer ); (*buffer)++;

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
read_zxcf_chunk( libspectrum_snap *snap, libspectrum_word version GCC_UNUSED,
		 const libspectrum_byte **buffer,
		 const libspectrum_byte *end GCC_UNUSED, size_t data_length,
                 szx_context *ctx GCC_UNUSED )
{
  libspectrum_word flags;

  if( data_length != 4 ) {
    libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
			     "read_zxcf_chunk: unknown length %lu",
			     (unsigned long)data_length );
    return LIBSPECTRUM_ERROR_UNKNOWN;
  }

  libspectrum_snap_set_zxcf_active( snap, 1 );

  flags = libspectrum_read_word( buffer );
  libspectrum_snap_set_zxcf_upload( snap, flags & ZXSTZXCFF_UPLOAD );

  libspectrum_snap_set_zxcf_memctl( snap, **buffer ); (*buffer)++;
  libspectrum_snap_set_zxcf_pages( snap, **buffer ); (*buffer)++;

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
read_if1_chunk( libspectrum_snap *snap, libspectrum_word version GCC_UNUSED,
		const libspectrum_byte **buffer,
		const libspectrum_byte *end GCC_UNUSED, size_t data_length,
                szx_context *ctx GCC_UNUSED )
{
  libspectrum_word flags;
  libspectrum_word expected_length;
  libspectrum_byte *rom_data = NULL; 

  if( data_length < 40 ) {
    libspectrum_print_error(
      LIBSPECTRUM_ERROR_UNKNOWN,
      "read_if1_chunk: length %lu too short", (unsigned long)data_length
    );
    return LIBSPECTRUM_ERROR_UNKNOWN;
  }

  flags = libspectrum_read_word( buffer );
  libspectrum_snap_set_interface1_drive_count( snap, **buffer ); (*buffer)++;
  *buffer += 3;		/* Skip reserved byte space */
  *buffer += sizeof( libspectrum_dword ) * 8; /* Skip reserved dword space */
  expected_length = libspectrum_read_word( buffer );

  libspectrum_snap_set_interface1_active( snap, flags & ZXSTIF1F_ENABLED );

  libspectrum_snap_set_interface1_paged( snap, flags & ZXSTIF1F_PAGED );

  if( expected_length ) {
    if( expected_length != 0x2000 && expected_length != 0x4000 ) {
        libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
                                 "%s:read_if1_chunk: invalid ROM length "
                                 "in file, should be 8192 or 16384 bytes, "
                                 "file has %lu",
                                 __FILE__, 
                                 (unsigned long)expected_length );
      return LIBSPECTRUM_ERROR_UNKNOWN;
    }

    libspectrum_snap_set_interface1_custom_rom( snap, 1 );

    if( flags & ZXSTIF1F_COMPRESSED ) {

#ifdef HAVE_ZLIB_H

      size_t uncompressed_length = 0;

      libspectrum_error error =
              libspectrum_zlib_inflate( *buffer, data_length - 40, &rom_data,
                                        &uncompressed_length );
      if( error ) return error;

      if( uncompressed_length != expected_length ) {
        libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
                                 "%s:read_if1_chunk: invalid ROM length "
                                 "in compressed file, should be %lu, file "
                                 "has %lu",
                                 __FILE__, 
                                 (unsigned long)expected_length,
                                 (unsigned long)uncompressed_length );
        return LIBSPECTRUM_ERROR_UNKNOWN;
      }

      libspectrum_snap_set_interface1_rom( snap, 0, rom_data );
      libspectrum_snap_set_interface1_rom_length( snap, 0,
                                                  uncompressed_length );

      *buffer += data_length - 40;

#else			/* #ifdef HAVE_ZLIB_H */

      libspectrum_print_error(
        LIBSPECTRUM_ERROR_UNKNOWN,
        "%s:read_if1_chunk: zlib needed for decompression\n",
        __FILE__
      );
      return LIBSPECTRUM_ERROR_UNKNOWN;

#endif			/* #ifdef HAVE_ZLIB_H */

    } else {

      if( data_length < 40 + expected_length ) {
        libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
                                 "%s:read_if1_chunk: length %lu too short, "
                                 "expected %lu" ,
                                 __FILE__, (unsigned long)data_length,
                                 (unsigned long)40 + expected_length );
        return LIBSPECTRUM_ERROR_UNKNOWN;
      }

      rom_data = libspectrum_malloc( expected_length );
      memcpy( rom_data, *buffer, expected_length );

      libspectrum_snap_set_interface1_rom_length( snap, 0, expected_length );

      *buffer += expected_length;

    }
  }

  return LIBSPECTRUM_ERROR_NONE;
}

static void
szx_set_custom_rom( libspectrum_snap *snap, int page_no,
                    libspectrum_byte *rom_data, libspectrum_word length )
{
  if( length ) {
    libspectrum_byte *page = libspectrum_malloc( length );
    memcpy( page, rom_data, length );

    libspectrum_snap_set_roms( snap, page_no, page );
    libspectrum_snap_set_rom_length( snap, page_no, length );
  }
}

static libspectrum_error
szx_extract_roms( libspectrum_snap *snap, libspectrum_byte *rom_data,
                  libspectrum_dword length, libspectrum_dword expected_length )
{
  int i;
  const size_t standard_rom_length = 0x4000;
  size_t num_16k_roms, additional_rom_length;

  if( length != expected_length ) {
    libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
                             "%s:szx_extract_roms: invalid ROM length %u, "
                             "expected %u",
                             __FILE__, length, expected_length );
    return LIBSPECTRUM_ERROR_UNKNOWN;
  }

  num_16k_roms = length / standard_rom_length;
  additional_rom_length = length % standard_rom_length;

  for( i = 0; i < num_16k_roms; i++ )
    szx_set_custom_rom( snap, i, rom_data + (i * standard_rom_length), standard_rom_length );

  /* Timex 2068 machines have a 16k and an 8k ROM, all other machines just have
     multiples of 16k ROMs */
  if( additional_rom_length )
    szx_set_custom_rom( snap, i, rom_data + (i * standard_rom_length), additional_rom_length );

  libspectrum_snap_set_custom_rom_pages( snap, num_16k_roms +
                                         !!additional_rom_length );

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
read_rom_chunk( libspectrum_snap *snap, libspectrum_word version GCC_UNUSED,
		const libspectrum_byte **buffer,
		const libspectrum_byte *end GCC_UNUSED, size_t data_length,
                 szx_context *ctx GCC_UNUSED )
{
  libspectrum_word flags;
  libspectrum_dword expected_length;
  libspectrum_byte *rom_data = NULL; 
  libspectrum_error retval = LIBSPECTRUM_ERROR_NONE;

  if( data_length < 6 ) {
    libspectrum_print_error(
      LIBSPECTRUM_ERROR_UNKNOWN,
      "read_rom_chunk: length %lu too short", (unsigned long)data_length
    );
    return LIBSPECTRUM_ERROR_UNKNOWN;
  }

  flags = libspectrum_read_word( buffer );
  expected_length = libspectrum_read_dword( buffer );

  if( flags & ZXSTRF_COMPRESSED ) {

#ifdef HAVE_ZLIB_H

    size_t uncompressed_length = 0;

    libspectrum_error error =
            libspectrum_zlib_inflate( *buffer, data_length - 6, &rom_data,
                                      &uncompressed_length );
    if( error ) return error;

    if( uncompressed_length != expected_length ) {
      libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
                               "%s:read_rom_chunk: invalid ROM length "
                               "in compressed file, should be %lu, file "
                               "has %lu",
                               __FILE__, 
                               (unsigned long)expected_length,
                               (unsigned long)uncompressed_length );
      return LIBSPECTRUM_ERROR_UNKNOWN;
    }

    *buffer += data_length - 6;

#else			/* #ifdef HAVE_ZLIB_H */

    libspectrum_print_error(
      LIBSPECTRUM_ERROR_UNKNOWN,
      "%s:read_rom_chunk: zlib needed for decompression\n",
      __FILE__
    );
    return LIBSPECTRUM_ERROR_UNKNOWN;

#endif			/* #ifdef HAVE_ZLIB_H */

  } else {

    if( data_length < 6 + expected_length ) {
      libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
                               "%s:read_rom_chunk: length %lu too short, "
                               "expected %lu" ,
                               __FILE__, (unsigned long)data_length,
                               (unsigned long)6 + expected_length );
      return LIBSPECTRUM_ERROR_UNKNOWN;
    }

    rom_data = libspectrum_malloc( expected_length );
    memcpy( rom_data, *buffer, expected_length );
    *buffer += expected_length;
  }

  libspectrum_snap_set_custom_rom( snap, 1 );

  switch ( libspectrum_snap_machine( snap ) ) {
  case LIBSPECTRUM_MACHINE_16:
  case LIBSPECTRUM_MACHINE_48:
  case LIBSPECTRUM_MACHINE_TC2048:
    retval = szx_extract_roms( snap, rom_data, expected_length, 0x4000 );
    break;
  case LIBSPECTRUM_MACHINE_128:
  case LIBSPECTRUM_MACHINE_PLUS2:
  case LIBSPECTRUM_MACHINE_SE:
    retval = szx_extract_roms( snap, rom_data, expected_length, 0x8000 );
    break;
  case LIBSPECTRUM_MACHINE_PLUS2A:
  case LIBSPECTRUM_MACHINE_PLUS3:
  case LIBSPECTRUM_MACHINE_PLUS3E:
    retval = szx_extract_roms( snap, rom_data, expected_length, 0x10000 );
    break;
  case LIBSPECTRUM_MACHINE_PENT:
    /* FIXME: This is a conflict with Fuse - szx specs say Pentagon 128k snaps
       will total 32k, Fuse also has the 'gluck.rom' */
    retval = szx_extract_roms( snap, rom_data, expected_length, 0x8000 );
    break;
  case LIBSPECTRUM_MACHINE_TC2068:
  case LIBSPECTRUM_MACHINE_TS2068:
    retval = szx_extract_roms( snap, rom_data, expected_length, 0x6000 );
    break;
  case LIBSPECTRUM_MACHINE_SCORP:
  case LIBSPECTRUM_MACHINE_PENT512:
  case LIBSPECTRUM_MACHINE_PENT1024:
    retval = szx_extract_roms( snap, rom_data, expected_length, 0x10000 );
    break;
  default:
    libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
                             "%s:read_rom_chunk: don't know correct custom ROM "
                             "length for this machine",
                             __FILE__ );
    retval = LIBSPECTRUM_ERROR_UNKNOWN;
    break;
  }

  libspectrum_free( rom_data );

  return retval;
}

static libspectrum_error
read_zxpr_chunk( libspectrum_snap *snap, libspectrum_word version,
		 const libspectrum_byte **buffer,
		 const libspectrum_byte *end GCC_UNUSED, size_t data_length,
                 szx_context *ctx GCC_UNUSED )
{
  libspectrum_word flags;

  if( data_length != 2 ) {
    libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
			     "%s:read_zxpr_chunk: unknown length %lu",
			     __FILE__, (unsigned long)data_length );
    return LIBSPECTRUM_ERROR_UNKNOWN;
  }

  flags = libspectrum_read_word( buffer );
  libspectrum_snap_set_zx_printer_active( snap, flags & ZXSTPRF_ENABLED );

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
read_if2r_chunk( libspectrum_snap *snap, libspectrum_word version GCC_UNUSED,
		 const libspectrum_byte **buffer,
		 const libspectrum_byte *end GCC_UNUSED, size_t data_length,
                 szx_context *ctx GCC_UNUSED )
{

#ifdef HAVE_ZLIB_H

  libspectrum_byte *buffer2;

  size_t uncompressed_length;

  libspectrum_error error;

  if( data_length < 4 ) {
    libspectrum_print_error(
      LIBSPECTRUM_ERROR_UNKNOWN,
      "read_if2r_chunk: length %lu too short", (unsigned long)data_length
    );
    return LIBSPECTRUM_ERROR_UNKNOWN;
  }

  /* Skip the compressed length as we never actually use it - bug? */
  libspectrum_read_dword( buffer );

  uncompressed_length = 0x4000;

  error = libspectrum_zlib_inflate( *buffer, data_length - 4, &buffer2,
                                    &uncompressed_length );
  if( error ) return error;

  *buffer += data_length - 4;

  libspectrum_snap_set_interface2_active( snap, 1 );

  libspectrum_snap_set_interface2_rom( snap, 0, buffer2 );

  return LIBSPECTRUM_ERROR_NONE;

#else			/* #ifdef HAVE_ZLIB_H */

  libspectrum_print_error(
    LIBSPECTRUM_ERROR_UNKNOWN,
    "%s:read_if2r_chunk: zlib needed for decompression\n",
    __FILE__
  );
  return LIBSPECTRUM_ERROR_UNKNOWN;

#endif			/* #ifdef HAVE_ZLIB_H */

}

static libspectrum_error
read_dock_chunk( libspectrum_snap *snap, libspectrum_word version GCC_UNUSED,
		 const libspectrum_byte **buffer,
		 const libspectrum_byte *end GCC_UNUSED, size_t data_length,
                 szx_context *ctx GCC_UNUSED )
{
  libspectrum_byte *data;
  size_t page;
  libspectrum_error error;
  libspectrum_word flags;
  libspectrum_byte writeable;

  error = read_ram_page( &data, &page, buffer, data_length, 0x2000, &flags );
  if( error ) return error;

  if( page > 7 ) {
    libspectrum_print_error( LIBSPECTRUM_ERROR_CORRUPT,
			     "%s:read_dock_chunk: unknown page number %ld",
			     __FILE__, (unsigned long)page );
    libspectrum_free( data );
    return LIBSPECTRUM_ERROR_CORRUPT;
  }

  libspectrum_snap_set_dock_active( snap, 1 );

  writeable = flags & ZXSTDOCKF_RAM;

  if( flags & ZXSTDOCKF_EXROMDOCK ) {
    libspectrum_snap_set_dock_ram( snap, page, writeable );
    libspectrum_snap_set_dock_cart( snap, page, data );
  } else {
    libspectrum_snap_set_exrom_ram( snap, page, writeable );
    libspectrum_snap_set_exrom_cart( snap, page, data );
  }

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
read_dide_chunk( libspectrum_snap *snap, libspectrum_word version GCC_UNUSED,
		 const libspectrum_byte **buffer,
		 const libspectrum_byte *end GCC_UNUSED, size_t data_length,
                 szx_context *ctx GCC_UNUSED )
{
#ifdef HAVE_ZLIB_H
  libspectrum_error error;
#endif
  libspectrum_word flags;
  libspectrum_byte *eprom_data = NULL;
  const size_t expected_length = 0x2000;

  if( data_length < 4 ) {
    libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
			     "%s:read_dide_chunk: unknown length %lu",
			     __FILE__, (unsigned long)data_length );
    return LIBSPECTRUM_ERROR_UNKNOWN;
  }

  flags = libspectrum_read_word( buffer );
  libspectrum_snap_set_divide_active( snap, 1 );
  libspectrum_snap_set_divide_eprom_writeprotect(
                                      snap,
                                      !!(flags & ZXSTDIVIDE_EPROM_WRITEPROTECT)
                                    );
  libspectrum_snap_set_divide_paged( snap, !!(flags & ZXSTDIVIDE_PAGED) );

  libspectrum_snap_set_divide_control( snap, **buffer ); (*buffer)++;
  libspectrum_snap_set_divide_pages( snap, **buffer ); (*buffer)++;

  if( flags & ZXSTDIVIDE_COMPRESSED ) {

#ifdef HAVE_ZLIB_H

    size_t uncompressed_length = 0;

    error = libspectrum_zlib_inflate( *buffer, data_length - 4, &eprom_data,
                                      &uncompressed_length );
    if( error ) return error;

    if( uncompressed_length != expected_length ) {
      libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
                               "%s:read_dide_chunk: invalid EPROM length "
                               "in compressed file, should be %lu, file "
                               "has %lu",
                               __FILE__,
                               (unsigned long)expected_length,
                               (unsigned long)uncompressed_length );
      return LIBSPECTRUM_ERROR_UNKNOWN;
    }

    *buffer += data_length - 4;

#else

    libspectrum_print_error(
      LIBSPECTRUM_ERROR_UNKNOWN,
      "%s:read_dide_chunk: zlib needed for decompression\n",
      __FILE__
    );
    return LIBSPECTRUM_ERROR_UNKNOWN;

#endif

  } else {

    if( data_length < 4 + expected_length ) {
      libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
                               "%s:read_dide_chunk: length %lu too short, "
                               "expected %lu",
                               __FILE__, (unsigned long)data_length,
                               (unsigned long)4 + expected_length );
      return LIBSPECTRUM_ERROR_UNKNOWN;
    }

    eprom_data = libspectrum_malloc( expected_length );
    memcpy( eprom_data, *buffer, expected_length );

    *buffer += expected_length;

  }

  libspectrum_snap_set_divide_eprom( snap, 0, eprom_data );

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
read_dirp_chunk( libspectrum_snap *snap, libspectrum_word version GCC_UNUSED,
		 const libspectrum_byte **buffer,
		 const libspectrum_byte *end GCC_UNUSED, size_t data_length,
                 szx_context *ctx GCC_UNUSED )
{
  libspectrum_byte *data;
  size_t page;
  libspectrum_error error;
  libspectrum_word flags;

  error = read_ram_page( &data, &page, buffer, data_length, 0x2000, &flags );
  if( error ) return error;

  if( page >= SNAPSHOT_DIVIDE_PAGES ) {
    libspectrum_print_error( LIBSPECTRUM_ERROR_CORRUPT,
			     "%s:read_dirp_chunk: unknown page number %lu",
			     __FILE__, (unsigned long)page );
    libspectrum_free( data );
    return LIBSPECTRUM_ERROR_CORRUPT;
  }

  libspectrum_snap_set_divide_ram( snap, page, data );

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
read_snet_memory( libspectrum_snap *snap, const libspectrum_byte **buffer,
  int compressed, size_t *data_remaining,
  void (*setter)(libspectrum_snap*, int, libspectrum_byte*) )
{
  size_t data_length;
  libspectrum_byte *data_out;
  const libspectrum_byte *data;

  if( *data_remaining < 4 ) {
    libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
      "%s:read_snet_memory: not enough data for length", __FILE__ );
    return LIBSPECTRUM_ERROR_UNKNOWN;
  }

  data_length = libspectrum_read_dword( buffer );
  *data_remaining -= 4;

  if( *data_remaining < data_length ) {
    libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
      "%s:read_snet_memory: not enough data", __FILE__ );
    return LIBSPECTRUM_ERROR_UNKNOWN;
  }
  *data_remaining -= data_length;

  if( compressed ) {

#ifdef HAVE_ZLIB_H
    libspectrum_error error;
    size_t uncompressed_length = 0;
    libspectrum_byte *uncompressed_data;

    error = libspectrum_zlib_inflate( *buffer, data_length, &uncompressed_data,
        &uncompressed_length );
    if( error ) return error;

    *buffer += data_length;

    if( uncompressed_length != 0x20000 ) {
      libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
          "%s:read_snet_memory: data decompressed to %lu but should be 0x20000",
          __FILE__, (unsigned long)uncompressed_length );
      libspectrum_free( uncompressed_data );
      return LIBSPECTRUM_ERROR_UNKNOWN;
    }

    data = uncompressed_data;

#else

    libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
        "%s:read_snet_memory: zlib needed for decompression\n", __FILE__ );
    return LIBSPECTRUM_ERROR_UNKNOWN;

#endif

  } else {
    if( data_length != 0x20000 ) {
      libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
          "%s:read_snet_memory: data has length %lu but should be 0x20000",
          __FILE__, (unsigned long)data_length );
      return LIBSPECTRUM_ERROR_NONE;
    }

    data = *buffer;
    *buffer += data_length;
  }

  data_out = libspectrum_malloc( 0x20000 );
  memcpy( data_out, data, 0x20000 );
  setter( snap, 0, data_out );

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
read_snet_chunk( libspectrum_snap *snap, libspectrum_word version GCC_UNUSED,
		 const libspectrum_byte **buffer,
		 const libspectrum_byte *end GCC_UNUSED, size_t data_length,
                 szx_context *ctx GCC_UNUSED )
{
  libspectrum_word flags;
  libspectrum_byte *w5100;

  if( data_length < 54 ) {
    libspectrum_print_error(
      LIBSPECTRUM_ERROR_UNKNOWN,
      "read_snet_chunk: length %lu too short", (unsigned long)data_length
    );
    return LIBSPECTRUM_ERROR_UNKNOWN;
  }

  libspectrum_snap_set_spectranet_active( snap, 1 );

  flags = libspectrum_read_word( buffer );
  libspectrum_snap_set_spectranet_paged( snap, flags & ZXSTSNET_PAGED );
  libspectrum_snap_set_spectranet_paged_via_io( snap, flags & ZXSTSNET_PAGED_VIA_IO );
  libspectrum_snap_set_spectranet_programmable_trap_active( snap,
    flags & ZXSTSNET_PROGRAMMABLE_TRAP_ACTIVE );
  libspectrum_snap_set_spectranet_programmable_trap_msb( snap,
    flags & ZXSTSNET_PROGRAMMABLE_TRAP_MSB );
  libspectrum_snap_set_spectranet_all_traps_disabled( snap, flags & ZXSTSNET_ALL_DISABLED );
  libspectrum_snap_set_spectranet_rst8_trap_disabled( snap, flags & ZXSTSNET_RST8_DISABLED );
  libspectrum_snap_set_spectranet_deny_downstream_a15( snap, flags & ZXSTSNET_DENY_DOWNSTREAM_A15 );
  libspectrum_snap_set_spectranet_nmi_flipflop( snap, flags & ZXSTSNET_NMI_FLIPFLOP );

  libspectrum_snap_set_spectranet_page_a( snap, **buffer ); (*buffer)++;
  libspectrum_snap_set_spectranet_page_b( snap, **buffer ); (*buffer)++;

  libspectrum_snap_set_spectranet_programmable_trap( snap,
    libspectrum_read_word( buffer ) );

  w5100 = libspectrum_malloc( 0x30 * sizeof( libspectrum_byte ) );
  libspectrum_snap_set_spectranet_w5100( snap, 0, w5100 );
  memcpy( w5100, *buffer, 0x30 );
  (*buffer) += 0x30;

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
read_snef_chunk( libspectrum_snap *snap, libspectrum_word version GCC_UNUSED,
		 const libspectrum_byte **buffer,
		 const libspectrum_byte *end GCC_UNUSED, size_t data_length,
                 szx_context *ctx GCC_UNUSED )
{
  libspectrum_byte flags;
  int flash_compressed;
  libspectrum_error error;
  size_t data_remaining;

  if( data_length < 5 ) {
    libspectrum_print_error(
      LIBSPECTRUM_ERROR_UNKNOWN,
      "read_snef_chunk: length %lu too short", (unsigned long)data_length
    );
    return LIBSPECTRUM_ERROR_UNKNOWN;
  }

  flags = **buffer; (*buffer)++;
  flash_compressed = flags & ZXSTSNEF_FLASH_COMPRESSED;

  data_remaining = data_length - 1;

  error = read_snet_memory( snap, buffer, flash_compressed, &data_remaining,
    libspectrum_snap_set_spectranet_flash );
  if( error )
    return error;

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
read_sner_chunk( libspectrum_snap *snap, libspectrum_word version GCC_UNUSED,
		 const libspectrum_byte **buffer,
		 const libspectrum_byte *end GCC_UNUSED, size_t data_length,
                 szx_context *ctx GCC_UNUSED )
{
  libspectrum_byte flags;
  int ram_compressed;
  libspectrum_error error;
  size_t data_remaining;

  if( data_length < 5 ) {
    libspectrum_print_error(
      LIBSPECTRUM_ERROR_UNKNOWN,
      "read_sner_chunk: length %lu too short", (unsigned long)data_length
    );
    return LIBSPECTRUM_ERROR_UNKNOWN;
  }

  flags = **buffer; (*buffer)++;
  ram_compressed = flags & ZXSTSNER_RAM_COMPRESSED;

  data_remaining = data_length - 1;

  error = read_snet_memory( snap, buffer, ram_compressed, &data_remaining,
    libspectrum_snap_set_spectranet_ram );
  if( error )
    return error;

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
skip_chunk( libspectrum_snap *snap GCC_UNUSED,
	    libspectrum_word version GCC_UNUSED,
	    const libspectrum_byte **buffer,
	    const libspectrum_byte *end GCC_UNUSED,
	    size_t data_length,
            szx_context *ctx GCC_UNUSED )
{
  *buffer += data_length;
  return LIBSPECTRUM_ERROR_NONE;
}

struct read_chunk_t {

  const char *id;
  read_chunk_fn function;

};

static struct read_chunk_t read_chunks[] = {

  { ZXSTBID_AY,		         read_ay_chunk   },
  { ZXSTBID_BETA128,	         read_b128_chunk },
  { ZXSTBID_BETADISK,	         skip_chunk      },
  { ZXSTBID_COVOX,	         skip_chunk      },
  { ZXSTBID_CREATOR,	         read_crtr_chunk },
  { ZXSTBID_DIVIDE,	         read_dide_chunk },
  { ZXSTBID_DIVIDERAMPAGE,       read_dirp_chunk },
  { ZXSTBID_DOCK,	         read_dock_chunk },
  { ZXSTBID_DSKFILE,	         skip_chunk      },
  { ZXSTBID_LEC,                 skip_chunk      },
  { ZXSTBID_LECRAMPAGE,          skip_chunk      },
  { ZXSTBID_GS,		         skip_chunk      },
  { ZXSTBID_GSRAMPAGE,	         skip_chunk      },
  { ZXSTBID_IF1,	         read_if1_chunk  },
  { ZXSTBID_IF2ROM,	         read_if2r_chunk },
  { ZXSTBID_JOYSTICK,	         read_joy_chunk  },
  { ZXSTBID_KEYBOARD,	         read_keyb_chunk },
  { ZXSTBID_MICRODRIVE,	         skip_chunk      },
  { ZXSTBID_MOUSE,	         read_amxm_chunk },
  { ZXSTBID_MULTIFACE,	         skip_chunk      },
  { ZXSTBID_OPUS,	         read_opus_chunk },
  { ZXSTBID_OPUSDISK,	         skip_chunk      },
  { ZXSTBID_PLUS3DISK,	         skip_chunk      },
  { ZXSTBID_PLUSD,	         read_plsd_chunk },
  { ZXSTBID_PLUSDDISK,	         skip_chunk      },
  { ZXSTBID_RAMPAGE,	         read_ramp_chunk },
  { ZXSTBID_ROM,	         read_rom_chunk  },
  { ZXSTBID_SIMPLEIDE,	         read_side_chunk },
  { ZXSTBID_SPECDRUM,	         read_drum_chunk },
  { ZXSTBID_SPECREGS,	         read_spcr_chunk },
  { ZXSTBID_SPECTRANET,          read_snet_chunk },
  { ZXSTBID_SPECTRANETFLASHPAGE, read_snef_chunk },
  { ZXSTBID_SPECTRANETRAMPAGE,   read_sner_chunk },
  { ZXSTBID_TIMEXREGS,	         read_scld_chunk },
  { ZXSTBID_USPEECH,	         skip_chunk      },
  { ZXSTBID_Z80REGS,	         read_z80r_chunk },
  { ZXSTBID_ZXATASPRAMPAGE,      read_atrp_chunk },
  { ZXSTBID_ZXATASP,	         read_zxat_chunk },
  { ZXSTBID_ZXCF,	         read_zxcf_chunk },
  { ZXSTBID_ZXCFRAMPAGE,         read_cfrp_chunk },
  { ZXSTBID_ZXPRINTER,	         read_zxpr_chunk },
  { ZXSTBID_ZXTAPE,	         skip_chunk      },

};

static size_t read_chunks_count =
  sizeof( read_chunks ) / sizeof( struct read_chunk_t );

static libspectrum_error
read_chunk_header( char *id, libspectrum_dword *data_length, 
		   const libspectrum_byte **buffer,
		   const libspectrum_byte *end )
{
  if( end - *buffer < 8 ) {
    libspectrum_print_error(
      LIBSPECTRUM_ERROR_CORRUPT,
      "szx_read_chunk_header: not enough data for chunk header"
    );
    return LIBSPECTRUM_ERROR_CORRUPT;
  }

  memcpy( id, *buffer, 4 ); id[4] = '\0'; *buffer += 4;
  *data_length = libspectrum_read_dword( buffer );

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
read_chunk( libspectrum_snap *snap, libspectrum_word version,
	    const libspectrum_byte **buffer, const libspectrum_byte *end,
            szx_context *ctx )
{
  char id[5];
  libspectrum_dword data_length;
  libspectrum_error error;
  size_t i; int done;

  error = read_chunk_header( id, &data_length, buffer, end );
  if( error ) return error;

  if( *buffer + data_length > end || *buffer + data_length < *buffer ) {
    libspectrum_print_error(
      LIBSPECTRUM_ERROR_CORRUPT,
      "szx_read_chunk: chunk length goes beyond end of file"
    );
    return LIBSPECTRUM_ERROR_CORRUPT;
  }

  done = 0;

  for( i = 0; !done && i < read_chunks_count; i++ ) {

    if( !memcmp( id, read_chunks[i].id, 4 ) ) {
      error = read_chunks[i].function( snap, version, buffer, end,
				       data_length, ctx );
      if( error ) return error;
      done = 1;
    }

  }

  if( !done ) {
    libspectrum_print_error( LIBSPECTRUM_ERROR_UNKNOWN,
			     "szx_read_chunk: unknown chunk id '%s'", id );
    *buffer += data_length;
  }

  return LIBSPECTRUM_ERROR_NONE;
}

libspectrum_error
libspectrum_szx_read( libspectrum_snap *snap, const libspectrum_byte *buffer,
		      size_t length )
{
  libspectrum_word version;
  libspectrum_byte machine;
  libspectrum_byte flags;

  libspectrum_error error;
  const libspectrum_byte *end = buffer + length;
  szx_context *ctx;

  if( end - buffer < 8 ) {
    libspectrum_print_error(
      LIBSPECTRUM_ERROR_CORRUPT,
      "libspectrum_szx_read: not enough data for SZX header"
    );
    return LIBSPECTRUM_ERROR_CORRUPT;
  }

  if( memcmp( buffer, signature, signature_length ) ) {
    libspectrum_print_error(
      LIBSPECTRUM_ERROR_SIGNATURE,
      "libspectrum_szx_read: wrong signature"
    );
    return LIBSPECTRUM_ERROR_SIGNATURE;
  }
  buffer += signature_length;

  version = (*buffer++) << 8; version |= *buffer++;

  machine = *buffer++;

  switch( machine ) {

  case SZX_MACHINE_16:
    libspectrum_snap_set_machine( snap, LIBSPECTRUM_MACHINE_16 );
    break;

  case SZX_MACHINE_48:
    libspectrum_snap_set_machine( snap, LIBSPECTRUM_MACHINE_48 );
    break;

  case SZX_MACHINE_48_NTSC:
    libspectrum_snap_set_machine( snap, LIBSPECTRUM_MACHINE_48_NTSC );
    break;

  case SZX_MACHINE_128:
    libspectrum_snap_set_machine( snap, LIBSPECTRUM_MACHINE_128 );
    break;

  case SZX_MACHINE_PLUS2:
    libspectrum_snap_set_machine( snap, LIBSPECTRUM_MACHINE_PLUS2 );
    break;

  case SZX_MACHINE_PLUS2A:
    libspectrum_snap_set_machine( snap, LIBSPECTRUM_MACHINE_PLUS2A );
    break;

  case SZX_MACHINE_PLUS3:
    libspectrum_snap_set_machine( snap, LIBSPECTRUM_MACHINE_PLUS3 );
    break;

  case SZX_MACHINE_PLUS3E:
    libspectrum_snap_set_machine( snap, LIBSPECTRUM_MACHINE_PLUS3E );
    break;

  case SZX_MACHINE_PENTAGON:
    libspectrum_snap_set_machine( snap, LIBSPECTRUM_MACHINE_PENT );
    break;

  case SZX_MACHINE_TC2048:
    libspectrum_snap_set_machine( snap, LIBSPECTRUM_MACHINE_TC2048 );
    break;

  case SZX_MACHINE_TC2068:
    libspectrum_snap_set_machine( snap, LIBSPECTRUM_MACHINE_TC2068 );
    break;

  case SZX_MACHINE_TS2068:
    libspectrum_snap_set_machine( snap, LIBSPECTRUM_MACHINE_TS2068 );
    break;

  case SZX_MACHINE_SCORPION:
    libspectrum_snap_set_machine( snap, LIBSPECTRUM_MACHINE_SCORP );
    break;

  case SZX_MACHINE_SE:
    libspectrum_snap_set_machine( snap, LIBSPECTRUM_MACHINE_SE );
    break;

  case SZX_MACHINE_PENTAGON512:
    libspectrum_snap_set_machine( snap, LIBSPECTRUM_MACHINE_PENT512 );
    break;

  case SZX_MACHINE_PENTAGON1024:
    libspectrum_snap_set_machine( snap, LIBSPECTRUM_MACHINE_PENT1024 );
    break;

  case SZX_MACHINE_128KE:
    libspectrum_snap_set_machine( snap, LIBSPECTRUM_MACHINE_128E );
    break;

  default:
    libspectrum_print_error(
      LIBSPECTRUM_ERROR_UNKNOWN,
      "libspectrum_szx_read: unknown machine type %d", (int)*buffer
    );
    return LIBSPECTRUM_ERROR_UNKNOWN;
  }

  flags = *buffer++;

  switch( machine ) {

  case SZX_MACHINE_16:
  case SZX_MACHINE_48:
  case SZX_MACHINE_48_NTSC:
  case SZX_MACHINE_128:
    libspectrum_snap_set_late_timings( snap, flags & ZXSTMF_ALTERNATETIMINGS );
    break;

  default:
    break;
  }

  ctx = libspectrum_malloc( sizeof( *ctx ) );
  ctx->swap_af = 0;

  while( buffer < end ) {
    error = read_chunk( snap, version, &buffer, end, ctx );
    if( error ) {
      libspectrum_free( ctx );
      return error;
    }
  }

  libspectrum_free( ctx );
  return LIBSPECTRUM_ERROR_NONE;
}

libspectrum_error
libspectrum_szx_write( libspectrum_byte **buffer, size_t *length,
		       int *out_flags, libspectrum_snap *snap,
		       libspectrum_creator *creator, int in_flags )
{
  libspectrum_byte *ptr = *buffer;
  int capabilities, compress;
  libspectrum_error error;
  size_t i;

  *out_flags = 0;

  capabilities =
    libspectrum_machine_capabilities( libspectrum_snap_machine( snap ) );

  compress = !( in_flags & LIBSPECTRUM_FLAG_SNAPSHOT_NO_COMPRESSION );

  error = write_file_header( buffer, &ptr, length, out_flags, snap );
  if( error ) return error;

  if( creator ) {
    error = write_crtr_chunk( buffer, &ptr, length, creator );
    if( error ) return error;
  }

  error = write_z80r_chunk( buffer, &ptr, length, snap );
  if( error ) return error;

  error = write_spcr_chunk( buffer, &ptr, length, snap );
  if( error ) return error;

  error = write_joy_chunk( buffer, &ptr, length, out_flags, snap );
  if( error ) return error;

  error = write_keyb_chunk( buffer, &ptr, length, out_flags, snap );
  if( error ) return error;

  if( libspectrum_snap_custom_rom( snap ) ) {
    error = write_rom_chunk( buffer, &ptr, length, out_flags, snap, compress );
    if( error ) return error;
  }


  error = write_ram_pages( buffer, &ptr, length, snap, compress );
  if( error ) return error;

  if( libspectrum_snap_fuller_box_active( snap ) ||
      libspectrum_snap_melodik_active( snap ) ||
      capabilities & LIBSPECTRUM_MACHINE_CAPABILITY_AY ) {
    error = write_ay_chunk( buffer, &ptr, length, snap );
    if( error ) return error;
  }

  if( capabilities & ( LIBSPECTRUM_MACHINE_CAPABILITY_TIMEX_MEMORY |
                       LIBSPECTRUM_MACHINE_CAPABILITY_SE_MEMORY ) ) {
    error = write_scld_chunk( buffer, &ptr, length, snap );
    if( error ) return error;
  }

  if( libspectrum_snap_beta_active( snap ) ) {
    error = write_b128_chunk( buffer, &ptr, length, snap, compress );
    if( error ) return error;
  }

  if( libspectrum_snap_zxatasp_active( snap ) ) {
    error = write_zxat_chunk( buffer, &ptr, length, snap );
    if( error ) return error;

    for( i = 0; i < libspectrum_snap_zxatasp_pages( snap ); i++ ) {
      error = write_atrp_chunk( buffer, &ptr, length, snap, i, compress );
      if( error ) return error;
    }
  }

  if( libspectrum_snap_zxcf_active( snap ) ) {
    error = write_zxcf_chunk( buffer, &ptr, length, snap );
    if( error ) return error;

    for( i = 0; i < libspectrum_snap_zxcf_pages( snap ); i++ ) {
      error = write_cfrp_chunk( buffer, &ptr, length, snap, i, compress );
      if( error ) return error;
    }
  }

  if( libspectrum_snap_interface2_active( snap ) ) {
#ifdef HAVE_ZLIB_H
    error = write_if2r_chunk( buffer, &ptr, length, snap );
    if( error ) return error;
#else
    /* IF2R blocks only support writing compressed images */
    *out_flags |= LIBSPECTRUM_FLAG_SNAPSHOT_MAJOR_INFO_LOSS;
#endif                         /* #ifdef HAVE_ZLIB_H */
  }

  if( libspectrum_snap_dock_active( snap ) ) {
    for( i = 0; i < 8; i++ ) {
      if( libspectrum_snap_exrom_cart( snap, i ) ) {
        error = write_dock_chunk( buffer, &ptr, length, snap, 0,
                                  libspectrum_snap_exrom_cart( snap, i ), i,
                                  libspectrum_snap_exrom_ram( snap, i ),
                                  compress );
        if( error ) return error;
      }
      if( libspectrum_snap_dock_cart( snap, i ) ) {
        error = write_dock_chunk( buffer, &ptr, length, snap, 1,
                                  libspectrum_snap_dock_cart( snap, i ), i,
                                  libspectrum_snap_dock_ram( snap, i ),
                                  compress );
        if( error ) return error;
      }
    }
  }

  if( libspectrum_snap_interface1_active( snap ) ) {
    error = write_if1_chunk( buffer, &ptr, length, snap, compress );
    if( error ) return error;
  }

  if( libspectrum_snap_opus_active( snap ) ) {
    error = write_opus_chunk( buffer, &ptr, length, snap, compress );
    if( error ) return error;
  }

  if( libspectrum_snap_plusd_active( snap ) ) {
    error = write_plsd_chunk( buffer, &ptr, length, snap, compress );
    if( error ) return error;
  }

  if( libspectrum_snap_kempston_mouse_active( snap ) ) {
    error = write_amxm_chunk( buffer, &ptr, length, snap );
    if( error ) return error;
  }

  if( libspectrum_snap_simpleide_active( snap ) ) {
    error = write_side_chunk( buffer, &ptr, length, snap );
    if( error ) return error;
  }

  if( libspectrum_snap_specdrum_active( snap ) ) {
    error = write_drum_chunk( buffer, &ptr, length, snap );
    if( error ) return error;
  }

  if( libspectrum_snap_divide_active( snap ) ) {
    error = write_dide_chunk( buffer, &ptr, length, snap, compress );
    if( error ) return error;

    for( i = 0; i < libspectrum_snap_divide_pages( snap ); i++ ) {
      error = write_dirp_chunk( buffer, &ptr, length, snap, i, compress );
      if( error ) return error;
    }
  }

  if( libspectrum_snap_spectranet_active( snap ) ) {
    error = write_snet_chunk( buffer, &ptr, length, snap, compress );
    if( error ) return error;
    error = write_snef_chunk( buffer, &ptr, length, snap, compress );
    if( error ) return error;
    error = write_sner_chunk( buffer, &ptr, length, snap, compress );
    if( error ) return error;
  }

  error = write_zxpr_chunk( buffer, &ptr, length, out_flags, snap );
  if( error ) return error;

  /* Set length to be actual length, not allocated length */
  *length = ptr - *buffer;

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
write_file_header( libspectrum_byte **buffer, libspectrum_byte **ptr,
		   size_t *length, int *out_flags, libspectrum_snap *snap )
{
  libspectrum_byte flags;

  libspectrum_make_room( buffer, 8, ptr, length );

  memcpy( *ptr, signature, 4 ); *ptr += 4;
  
  *(*ptr)++ = SZX_VERSION_MAJOR; *(*ptr)++ = SZX_VERSION_MINOR;

  switch( libspectrum_snap_machine( snap ) ) {

  case LIBSPECTRUM_MACHINE_16:     **ptr = SZX_MACHINE_16; break;
  case LIBSPECTRUM_MACHINE_48:     **ptr = SZX_MACHINE_48; break;
  case LIBSPECTRUM_MACHINE_48_NTSC: **ptr = SZX_MACHINE_48_NTSC; break;
  case LIBSPECTRUM_MACHINE_128:    **ptr = SZX_MACHINE_128; break;
  case LIBSPECTRUM_MACHINE_128E:    **ptr = SZX_MACHINE_128KE; break;
  case LIBSPECTRUM_MACHINE_PLUS2:  **ptr = SZX_MACHINE_PLUS2; break;
  case LIBSPECTRUM_MACHINE_PLUS2A: **ptr = SZX_MACHINE_PLUS2A; break;
  case LIBSPECTRUM_MACHINE_PLUS3:  **ptr = SZX_MACHINE_PLUS3; break;
  case LIBSPECTRUM_MACHINE_PLUS3E: **ptr = SZX_MACHINE_PLUS3E; break;
  case LIBSPECTRUM_MACHINE_PENT:   **ptr = SZX_MACHINE_PENTAGON; break;
  case LIBSPECTRUM_MACHINE_TC2048: **ptr = SZX_MACHINE_TC2048; break;
  case LIBSPECTRUM_MACHINE_TC2068: **ptr = SZX_MACHINE_TC2068; break;
  case LIBSPECTRUM_MACHINE_TS2068: **ptr = SZX_MACHINE_TS2068; break;
  case LIBSPECTRUM_MACHINE_SCORP:  **ptr = SZX_MACHINE_SCORPION; break;
  case LIBSPECTRUM_MACHINE_SE:     **ptr = SZX_MACHINE_SE; break;
  case LIBSPECTRUM_MACHINE_PENT512: **ptr = SZX_MACHINE_PENTAGON512; break;
  case LIBSPECTRUM_MACHINE_PENT1024: **ptr = SZX_MACHINE_PENTAGON1024; break;

  case LIBSPECTRUM_MACHINE_UNKNOWN:
    libspectrum_print_error( LIBSPECTRUM_ERROR_LOGIC,
			     "Emulated machine type is set to 'unknown'!" );
    return LIBSPECTRUM_ERROR_LOGIC;
  }
  (*ptr)++;

  /* Flags byte */
  flags = 0;
  if( libspectrum_snap_late_timings( snap ) ) flags |= ZXSTMF_ALTERNATETIMINGS;
  *(*ptr)++ = flags;

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
write_crtr_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_creator *creator )
{
  size_t custom_length;

  custom_length = libspectrum_creator_custom_length( creator );

  write_chunk_header( buffer, ptr, length, ZXSTBID_CREATOR, 36 + custom_length );

  memcpy( *ptr, libspectrum_creator_program( creator ), 32 ); *ptr += 32;
  libspectrum_write_word( ptr, libspectrum_creator_major( creator ) );
  libspectrum_write_word( ptr, libspectrum_creator_minor( creator ) );

  if( custom_length ) {
    memcpy( *ptr, libspectrum_creator_custom( creator ), custom_length );
    *ptr += custom_length;
  }

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
write_z80r_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap )
{
  libspectrum_dword tstates;
  libspectrum_byte flags;

  write_chunk_header( buffer, ptr, length, ZXSTBID_Z80REGS, 37 );

  *(*ptr)++ = libspectrum_snap_f ( snap );
  *(*ptr)++ = libspectrum_snap_a ( snap );
  libspectrum_write_word( ptr, libspectrum_snap_bc  ( snap ) );
  libspectrum_write_word( ptr, libspectrum_snap_de  ( snap ) );
  libspectrum_write_word( ptr, libspectrum_snap_hl  ( snap ) );

  *(*ptr)++ = libspectrum_snap_f_( snap );
  *(*ptr)++ = libspectrum_snap_a_( snap );
  libspectrum_write_word( ptr, libspectrum_snap_bc_ ( snap ) );
  libspectrum_write_word( ptr, libspectrum_snap_de_ ( snap ) );
  libspectrum_write_word( ptr, libspectrum_snap_hl_ ( snap ) );

  libspectrum_write_word( ptr, libspectrum_snap_ix  ( snap ) );
  libspectrum_write_word( ptr, libspectrum_snap_iy  ( snap ) );
  libspectrum_write_word( ptr, libspectrum_snap_sp  ( snap ) );
  libspectrum_write_word( ptr, libspectrum_snap_pc  ( snap ) );

  *(*ptr)++ = libspectrum_snap_i   ( snap );
  *(*ptr)++ = libspectrum_snap_r   ( snap );
  *(*ptr)++ = libspectrum_snap_iff1( snap );
  *(*ptr)++ = libspectrum_snap_iff2( snap );
  *(*ptr)++ = libspectrum_snap_im  ( snap );

  tstates = libspectrum_snap_tstates( snap );

  libspectrum_write_dword( ptr, tstates );

  /* Number of tstates remaining in which an interrupt can occur */
  if( tstates < 48 ) {
    *(*ptr)++ = (unsigned char)(48 - tstates);
  } else {
    *(*ptr)++ = '\0';
  }

  flags = '\0';
  if( libspectrum_snap_last_instruction_ei( snap ) ) flags |= ZXSTZF_EILAST;
  if( libspectrum_snap_halted( snap ) ) flags |= ZXSTZF_HALTED;
  if( libspectrum_snap_last_instruction_set_f( snap ) ) flags |= ZXSTZF_FSET;
  *(*ptr)++ = flags;

  libspectrum_write_word( ptr, libspectrum_snap_memptr( snap ) );

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
write_spcr_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap )
{
  int capabilities;

  write_chunk_header( buffer, ptr, length, ZXSTBID_SPECREGS, 8 );

  capabilities =
    libspectrum_machine_capabilities( libspectrum_snap_machine( snap ) );

  /* Border colour */
  *(*ptr)++ = libspectrum_snap_out_ula( snap ) & 0x07;

  if( capabilities & LIBSPECTRUM_MACHINE_CAPABILITY_128_MEMORY ) {
    *(*ptr)++ = libspectrum_snap_out_128_memoryport( snap );
  } else {
    *(*ptr)++ = '\0';
  }
  
  if( capabilities & LIBSPECTRUM_MACHINE_CAPABILITY_PLUS3_MEMORY    || 
      capabilities & LIBSPECTRUM_MACHINE_CAPABILITY_SCORP_MEMORY    ||
      capabilities & LIBSPECTRUM_MACHINE_CAPABILITY_PENT1024_MEMORY    ) {
    *(*ptr)++ = libspectrum_snap_out_plus3_memoryport( snap );
  } else {
    *(*ptr)++ = '\0';
  }

  *(*ptr)++ = libspectrum_snap_out_ula( snap );

  /* Reserved bytes */
  libspectrum_write_dword( ptr, 0 );

  return LIBSPECTRUM_ERROR_NONE;
}

static void
write_joystick( libspectrum_byte **ptr, int *out_flags, libspectrum_snap *snap,
                const int connection )
{
  size_t num_joysticks = libspectrum_snap_joystick_active_count( snap );
  int found = 0;
  int i;

  for( i = 0; i < num_joysticks; i++ ) {
    if( libspectrum_snap_joystick_inputs( snap, i ) & connection ) {
      switch( libspectrum_snap_joystick_list( snap, i ) ) {
      case LIBSPECTRUM_JOYSTICK_CURSOR:
        if( !found ) { *(*ptr)++ = ZXJT_CURSOR; found = 1; }
        else *out_flags |= LIBSPECTRUM_FLAG_SNAPSHOT_MINOR_INFO_LOSS;
        break;
      case LIBSPECTRUM_JOYSTICK_KEMPSTON:
        if( !found ) { *(*ptr)++ = ZXJT_KEMPSTON; found = 1; }
        else *out_flags |= LIBSPECTRUM_FLAG_SNAPSHOT_MINOR_INFO_LOSS;
        break;
      case LIBSPECTRUM_JOYSTICK_SINCLAIR_1:
        if( !found ) { *(*ptr)++ = ZXJT_SINCLAIR1; found = 1; }
        else *out_flags |= LIBSPECTRUM_FLAG_SNAPSHOT_MINOR_INFO_LOSS;
        break;
      case LIBSPECTRUM_JOYSTICK_SINCLAIR_2:
        if( !found ) { *(*ptr)++ = ZXJT_SINCLAIR2; found = 1; }
        else *out_flags |= LIBSPECTRUM_FLAG_SNAPSHOT_MINOR_INFO_LOSS;
        break;
      case LIBSPECTRUM_JOYSTICK_TIMEX_1:
        if( !found ) { *(*ptr)++ = ZXJT_TIMEX1; found = 1; }
        else *out_flags |= LIBSPECTRUM_FLAG_SNAPSHOT_MINOR_INFO_LOSS;
        break;
      case LIBSPECTRUM_JOYSTICK_TIMEX_2:
        if( !found ) { *(*ptr)++ = ZXJT_TIMEX2; found = 1; }
        else *out_flags |= LIBSPECTRUM_FLAG_SNAPSHOT_MINOR_INFO_LOSS;
        break;
      case LIBSPECTRUM_JOYSTICK_FULLER:
        if( !found ) { *(*ptr)++ = ZXJT_FULLER; found = 1; }
        else *out_flags |= LIBSPECTRUM_FLAG_SNAPSHOT_MINOR_INFO_LOSS;
        break;
      case LIBSPECTRUM_JOYSTICK_NONE: /* Shouldn't happen */
      default:
        *(*ptr)++ = ZXJT_NONE;
        break;
      }
    }
  }

  if( !found ) *(*ptr)++ = ZXJT_NONE;
}

static libspectrum_error
write_joy_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, int *out_flags, libspectrum_snap *snap )
{
  libspectrum_dword flags;
  size_t num_joysticks = libspectrum_snap_joystick_active_count( snap );
  int i;

  write_chunk_header( buffer, ptr, length, ZXSTBID_JOYSTICK, 6 );

  flags = 0;
  for( i = 0; i < num_joysticks; i++ ) {
    if( libspectrum_snap_joystick_list( snap, i ) ==
        LIBSPECTRUM_JOYSTICK_KEMPSTON )
      flags |= ZXSTJOYF_ALWAYSPORT31;
  }
  libspectrum_write_dword( ptr, flags );

  write_joystick( ptr, out_flags, snap, LIBSPECTRUM_JOYSTICK_INPUT_JOYSTICK_1 );
  write_joystick( ptr, out_flags, snap, LIBSPECTRUM_JOYSTICK_INPUT_JOYSTICK_2 );

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
write_amxm_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap )
{
  write_chunk_header( buffer, ptr, length, ZXSTBID_MOUSE, 7 );

  if( libspectrum_snap_kempston_mouse_active( snap ) )
    *(*ptr)++ = ZXSTM_KEMPSTON;
  else
    *(*ptr)++ = ZXSTM_NONE;

  /* Z80 PIO CTRLA registers for AMX mouse */
  *(*ptr)++ = '\0'; *(*ptr)++ = '\0'; *(*ptr)++ = '\0';

  /* Z80 PIO CTRLB registers for AMX mouse */
  *(*ptr)++ = '\0'; *(*ptr)++ = '\0'; *(*ptr)++ = '\0';

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
write_keyb_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, int *out_flags, libspectrum_snap *snap )
{
  libspectrum_dword flags;

  write_chunk_header( buffer, ptr, length, ZXSTBID_KEYBOARD, 5 );

  flags = 0;
  if( libspectrum_snap_issue2( snap ) ) flags |= ZXSTKF_ISSUE2;

  libspectrum_write_dword( ptr, flags );

  write_joystick( ptr, out_flags, snap, LIBSPECTRUM_JOYSTICK_INPUT_KEYBOARD );

  return LIBSPECTRUM_ERROR_NONE;
}
  
static libspectrum_error
write_zxpr_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, int *out_flags, libspectrum_snap *snap )
{
  libspectrum_word flags;

  write_chunk_header( buffer, ptr, length, ZXSTBID_ZXPRINTER, 2 );

  flags = 0;
  if( libspectrum_snap_zx_printer_active( snap ) ) flags |= ZXSTPRF_ENABLED;

  libspectrum_write_word( ptr, flags );

  return LIBSPECTRUM_ERROR_NONE;
}
  
static libspectrum_error
write_rom_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr, size_t *length,
                 int *out_flags, libspectrum_snap *snap, int compress )
{
#ifdef HAVE_ZLIB_H
  libspectrum_error error;
#endif
  size_t i, data_length = 0;
  size_t uncompressed_data_length = 0;
  libspectrum_byte *data, *rom_base;
  int flags = 0;

  for( i = 0; i< libspectrum_snap_custom_rom_pages( snap ); i++ ) {
    data_length += libspectrum_snap_rom_length( snap, i );
  }

  /* Check that we have the expected number of ROMs per the machine type */
  switch( libspectrum_snap_machine( snap ) ) {

  case LIBSPECTRUM_MACHINE_16:
  case LIBSPECTRUM_MACHINE_48:
  case LIBSPECTRUM_MACHINE_48_NTSC:
  case LIBSPECTRUM_MACHINE_TC2048:
    /* 1 ROM = 16k */
    if( ( libspectrum_snap_custom_rom_pages( snap ) != 1 ||
          data_length != 0x4000 ) ) {
      *out_flags |= LIBSPECTRUM_FLAG_SNAPSHOT_MAJOR_INFO_LOSS;
      return LIBSPECTRUM_ERROR_NONE;
    }
    break;
  case LIBSPECTRUM_MACHINE_128:
  case LIBSPECTRUM_MACHINE_128E:
  case LIBSPECTRUM_MACHINE_PENT:
  case LIBSPECTRUM_MACHINE_PLUS2:
  case LIBSPECTRUM_MACHINE_SE:
    /* 2 ROMs = 32k */
    if( ( libspectrum_snap_custom_rom_pages( snap ) != 2 ||
          data_length != 0x8000 ) ) {
      *out_flags |= LIBSPECTRUM_FLAG_SNAPSHOT_MAJOR_INFO_LOSS;
      return LIBSPECTRUM_ERROR_NONE;
    }
    break;
  case LIBSPECTRUM_MACHINE_PLUS2A:
  case LIBSPECTRUM_MACHINE_PLUS3:
  case LIBSPECTRUM_MACHINE_PLUS3E:
    /* 4 ROMs = 64k */
    if( ( libspectrum_snap_custom_rom_pages( snap ) != 4 ||
          data_length != 0x10000 ) ) {
      *out_flags |= LIBSPECTRUM_FLAG_SNAPSHOT_MAJOR_INFO_LOSS;
      return LIBSPECTRUM_ERROR_NONE;
    }
    break;
  case LIBSPECTRUM_MACHINE_PENT512:
  case LIBSPECTRUM_MACHINE_PENT1024:
    /* 3 ROMs = 48k */
    if( ( libspectrum_snap_custom_rom_pages( snap ) != 3 ||
          data_length != 0xc000 ) ) {
      *out_flags |= LIBSPECTRUM_FLAG_SNAPSHOT_MAJOR_INFO_LOSS;
      return LIBSPECTRUM_ERROR_NONE;
    }
    break;
  case LIBSPECTRUM_MACHINE_TC2068:
  case LIBSPECTRUM_MACHINE_TS2068:
    /* 2 ROMs = 24k */
    if( ( libspectrum_snap_custom_rom_pages( snap ) != 2 ||
          data_length != 0x6000 ) ) {
      *out_flags |= LIBSPECTRUM_FLAG_SNAPSHOT_MAJOR_INFO_LOSS;
      return LIBSPECTRUM_ERROR_NONE;
    }
    break;
  case LIBSPECTRUM_MACHINE_SCORP:
    /* 4 ROMs = 64k */
    if( ( libspectrum_snap_custom_rom_pages( snap ) != 4 ||
          data_length != 0x10000 ) ) {
      *out_flags |= LIBSPECTRUM_FLAG_SNAPSHOT_MAJOR_INFO_LOSS;
      return LIBSPECTRUM_ERROR_NONE;
    }
    break;

  case LIBSPECTRUM_MACHINE_UNKNOWN:
    libspectrum_print_error( LIBSPECTRUM_ERROR_LOGIC,
			     "Emulated machine type is set to 'unknown'!" );
    return LIBSPECTRUM_ERROR_LOGIC;
  }

  uncompressed_data_length = data_length;

  data = libspectrum_malloc( data_length );
  rom_base = data;

  /* Copy the rom data into a single block ready for putting in the szx */
  for( i = 0; i< libspectrum_snap_custom_rom_pages( snap ); i++ ) {
    memcpy( rom_base, libspectrum_snap_roms( snap, i ),
            libspectrum_snap_rom_length( snap, i ) );
    rom_base += libspectrum_snap_rom_length( snap, i );
  }

#ifdef HAVE_ZLIB_H

  if( compress ) {

    libspectrum_byte *compressed_data;
    size_t compressed_length;

    error = libspectrum_zlib_compress( data, data_length,
				       &compressed_data, &compressed_length );
    if( error ) return error;

    if( compress & LIBSPECTRUM_FLAG_SNAPSHOT_ALWAYS_COMPRESS ||
        compressed_length < data_length ) {
      libspectrum_byte *old_data = data;
      flags |= ZXSTRF_COMPRESSED;
      data = compressed_data;
      data_length = compressed_length;
      libspectrum_free( old_data );
    } else {
      libspectrum_free( compressed_data );
    }

  }

#endif				/* #ifdef HAVE_ZLIB_H */

  write_chunk_header( buffer, ptr, length, ZXSTBID_ROM, 6+data_length );

  libspectrum_write_word( ptr, flags );
  libspectrum_write_dword( ptr, uncompressed_data_length );

  memcpy( *ptr, data, data_length ); *ptr += data_length;

  libspectrum_free( data );

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
write_ram_pages( libspectrum_byte **buffer, libspectrum_byte **ptr,
		 size_t *length, libspectrum_snap *snap, int compress )
{
  libspectrum_machine machine;
  int i, capabilities; 
  libspectrum_error error;

  machine = libspectrum_snap_machine( snap );
  capabilities = libspectrum_machine_capabilities( machine );

  error = write_ramp_chunk( buffer, ptr, length, snap, 5, compress );
  if( error ) return error;

  if( machine != LIBSPECTRUM_MACHINE_16 ) {
    error = write_ramp_chunk( buffer, ptr, length, snap, 2, compress );
    if( error ) return error;
    error = write_ramp_chunk( buffer, ptr, length, snap, 0, compress );
    if( error ) return error;
  }

  if( capabilities & LIBSPECTRUM_MACHINE_CAPABILITY_128_MEMORY ) {
    error = write_ramp_chunk( buffer, ptr, length, snap, 1, compress );
    if( error ) return error;
    error = write_ramp_chunk( buffer, ptr, length, snap, 3, compress );
    if( error ) return error;
    error = write_ramp_chunk( buffer, ptr, length, snap, 4, compress );
    if( error ) return error;
    error = write_ramp_chunk( buffer, ptr, length, snap, 6, compress );
    if( error ) return error;
    error = write_ramp_chunk( buffer, ptr, length, snap, 7, compress );
    if( error ) return error;

    if( capabilities & LIBSPECTRUM_MACHINE_CAPABILITY_SCORP_MEMORY ) {
      for( i = 8; i < 16; i++ ) {
        error = write_ramp_chunk( buffer, ptr, length, snap, i, compress );
        if( error ) return error;
      }
    } else if( capabilities & LIBSPECTRUM_MACHINE_CAPABILITY_PENT512_MEMORY ) {
      for( i = 8; i < 32; i++ ) {
        error = write_ramp_chunk( buffer, ptr, length, snap, i, compress );
        if( error ) return error;
      }

      if( capabilities & LIBSPECTRUM_MACHINE_CAPABILITY_PENT1024_MEMORY ) {
	for( i = 32; i < 64; i++ ) {
	  error = write_ramp_chunk( buffer, ptr, length, snap, i, compress );
	  if( error ) return error;
	}
      }
    }

  }

  if( capabilities & LIBSPECTRUM_MACHINE_CAPABILITY_SE_MEMORY ) {
    error = write_ramp_chunk( buffer, ptr, length, snap, 8, compress );
    if( error ) return error;
  }

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
write_ramp_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap, int page,
		  int compress )
{
  libspectrum_error error;
  const libspectrum_byte *data;

  data = libspectrum_snap_pages( snap, page );

  error = write_ram_page( buffer, ptr, length, ZXSTBID_RAMPAGE, data, 0x4000,
			  page, compress, 0x00 );
  if( error ) return error;

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
write_ram_page( libspectrum_byte **buffer, libspectrum_byte **ptr,
		size_t *length, const char *id, const libspectrum_byte *data,
		size_t data_length, int page, int compress, int extra_flags )
{
#ifdef HAVE_ZLIB_H
  libspectrum_error error;
#endif
  libspectrum_byte *block_length, *flags, *compressed_data;
  int use_compression;

  if( !data ) return LIBSPECTRUM_ERROR_NONE;

  /* 8 for the chunk header, 3 for the flags and the page number */
  libspectrum_make_room( buffer, 8 + 3, ptr, length );

  memcpy( *ptr, id, 4 ); (*ptr) += 4;

  /* Store this location for later */
  block_length = *ptr; *ptr += 4;

  /* And this one */
  flags = *ptr; *ptr += 2;

  *(*ptr)++ = (libspectrum_byte)page;

  use_compression = 0;
  compressed_data = NULL;

#ifdef HAVE_ZLIB_H

  if( compress ) {

    size_t compressed_length;

    error = libspectrum_zlib_compress( data, data_length,
				       &compressed_data, &compressed_length );
    if( error ) return error;

    if( compress & LIBSPECTRUM_FLAG_SNAPSHOT_ALWAYS_COMPRESS ||
        compressed_length < data_length ) {
      use_compression = 1;
      data = compressed_data;
      data_length = compressed_length;
    }
  }

#endif				/* #ifdef HAVE_ZLIB_H */

  if( use_compression ) extra_flags |= ZXSTRF_COMPRESSED;

  libspectrum_write_dword( &block_length, 3 + data_length );
  libspectrum_write_word( &flags, extra_flags );

  libspectrum_make_room( buffer, data_length, ptr, length );

  memcpy( *ptr, data, data_length ); *ptr += data_length;

  if( compressed_data ) libspectrum_free( compressed_data );

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
write_ay_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		size_t *length, libspectrum_snap *snap )
{
  size_t i;
  libspectrum_byte flags;

  write_chunk_header( buffer, ptr, length, ZXSTBID_AY, 18 );

  flags = 0;
  if( libspectrum_snap_fuller_box_active( snap ) ) flags |= ZXSTAYF_FULLERBOX;
  if( libspectrum_snap_melodik_active( snap ) ) flags |= ZXSTAYF_128AY;
  *(*ptr)++ = flags;

  *(*ptr)++ = libspectrum_snap_out_ay_registerport( snap );

  for( i = 0; i < 16; i++ )
    *(*ptr)++ = libspectrum_snap_ay_registers( snap, i );

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
write_scld_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		size_t *length, libspectrum_snap *snap )
{
  write_chunk_header( buffer, ptr, length, ZXSTBID_TIMEXREGS, 2 );

  *(*ptr)++ = libspectrum_snap_out_scld_hsr( snap );
  *(*ptr)++ = libspectrum_snap_out_scld_dec( snap );

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
write_b128_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap, int compress )
{
#ifdef HAVE_ZLIB_H
  libspectrum_error error;
#endif
  libspectrum_byte *rom_data = NULL;
  libspectrum_byte *compressed_rom_data = NULL;
  size_t block_size;
  libspectrum_word beta_rom_length = 0;
  libspectrum_word uncompressed_rom_length = 0;
  libspectrum_dword flags;
  int use_compression = 0;

  if( libspectrum_snap_beta_custom_rom( snap ) ) {
    rom_data = libspectrum_snap_beta_rom( snap, 0 );
    uncompressed_rom_length = beta_rom_length = 0x4000;

#ifdef HAVE_ZLIB_H

    if( rom_data && compress ) {

      size_t compressed_rom_length;

      error = libspectrum_zlib_compress( rom_data, uncompressed_rom_length,
                                         &compressed_rom_data,
                                         &compressed_rom_length );
      if( error ) return error;

      if( compress & LIBSPECTRUM_FLAG_SNAPSHOT_ALWAYS_COMPRESS ||
          compressed_rom_length < uncompressed_rom_length ) {
        use_compression = 1;
        rom_data = compressed_rom_data;
        beta_rom_length = compressed_rom_length;
      }

    }

#endif

  }

  block_size = 10 + beta_rom_length;

  write_chunk_header( buffer, ptr, length, ZXSTBID_BETA128, block_size );

  flags = ZXSTBETAF_CONNECTED;	/* Betadisk interface connected */
  if( libspectrum_snap_beta_paged( snap ) ) flags |= ZXSTBETAF_PAGED;
  if( libspectrum_snap_beta_autoboot( snap ) ) flags |= ZXSTBETAF_AUTOBOOT;
  if( !libspectrum_snap_beta_direction( snap ) ) flags |= ZXSTBETAF_SEEKLOWER;
  if( libspectrum_snap_beta_custom_rom( snap ) ) flags |= ZXSTBETAF_CUSTOMROM;
  if( use_compression ) flags |= ZXSTBETAF_COMPRESSED;
  libspectrum_write_dword( ptr, flags );

  *(*ptr)++ = libspectrum_snap_beta_drive_count( snap );
  *(*ptr)++ = libspectrum_snap_beta_system( snap );
  *(*ptr)++ = libspectrum_snap_beta_track ( snap );
  *(*ptr)++ = libspectrum_snap_beta_sector( snap );
  *(*ptr)++ = libspectrum_snap_beta_data  ( snap );
  *(*ptr)++ = libspectrum_snap_beta_status( snap );

  if( libspectrum_snap_beta_custom_rom( snap ) && rom_data ) {
    memcpy( *ptr, rom_data, beta_rom_length ); *ptr += beta_rom_length;
  }

  if( compressed_rom_data ) libspectrum_free( compressed_rom_data );

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
write_if1_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		 size_t *length, libspectrum_snap *snap, int compress  )
{
#ifdef HAVE_ZLIB_H
  libspectrum_error error;
#endif
  libspectrum_byte *rom_data = NULL; 
  libspectrum_byte *compressed_rom_data = NULL;
  size_t block_size;
  libspectrum_word disk_rom_length = 0;
  libspectrum_word uncompressed_rom_length = 0;
  libspectrum_word flags = 0;
  int use_compression = 0;

  if( libspectrum_snap_interface1_custom_rom( snap ) ) {
    if( !(libspectrum_snap_interface1_rom_length( snap, 0 ) == 0x2000 ||
          libspectrum_snap_interface1_rom_length( snap, 0 ) == 0x4000 )) {
      libspectrum_print_error( LIBSPECTRUM_ERROR_LOGIC,
                               "Interface 1 custom ROM must be 8192 or 16384 "
                               "bytes, supplied ROM is %lu bytes",
                               (unsigned long)
			       libspectrum_snap_interface1_rom_length(
                                 snap, 0 ) );
      return LIBSPECTRUM_ERROR_LOGIC;
    }
    rom_data = libspectrum_snap_interface1_rom( snap, 0 );
    uncompressed_rom_length = disk_rom_length =
      libspectrum_snap_interface1_rom_length( snap, 0 );
  }

  compressed_rom_data = NULL;

#ifdef HAVE_ZLIB_H

  if( rom_data && compress ) {

    size_t compressed_rom_length;

    error = libspectrum_zlib_compress( rom_data, disk_rom_length,
				       &compressed_rom_data, &compressed_rom_length );
    if( error ) return error;

    if( compress & LIBSPECTRUM_FLAG_SNAPSHOT_ALWAYS_COMPRESS ||
        compressed_rom_length < disk_rom_length ) {
      use_compression = 1;
      rom_data = compressed_rom_data;
      disk_rom_length = compressed_rom_length;
    }

  }

#endif				/* #ifdef HAVE_ZLIB_H */

  block_size = 40;
  if( libspectrum_snap_interface1_custom_rom( snap ) ) {
    block_size += disk_rom_length;
  }

  write_chunk_header( buffer, ptr, length, ZXSTBID_IF1, block_size );

  flags |= ZXSTIF1F_ENABLED;
  if( libspectrum_snap_interface1_paged( snap ) ) flags |= ZXSTIF1F_PAGED;
  if( use_compression ) flags |= ZXSTIF1F_COMPRESSED;
  libspectrum_write_word( ptr, flags );

  if( libspectrum_snap_interface1_drive_count( snap ) )
    *(*ptr)++ = libspectrum_snap_interface1_drive_count( snap );
  else
    *(*ptr)++ = 8;				/* guess 8 drives connected */
  *ptr += 3;					/* Skip 'reserved' data */
  *ptr += sizeof(libspectrum_dword) * 8;	/* Skip 'reserved' data */
  libspectrum_write_word( ptr, uncompressed_rom_length );

  if( libspectrum_snap_interface1_custom_rom( snap ) && disk_rom_length ) {
    memcpy( *ptr, rom_data, disk_rom_length ); *ptr += disk_rom_length;
  }

  if( compressed_rom_data ) libspectrum_free( compressed_rom_data );

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
write_opus_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap, int compress  )
{
#ifdef HAVE_ZLIB_H
  libspectrum_error error;
#endif
  libspectrum_byte *rom_data, *ram_data; 
  libspectrum_byte *compressed_rom_data = NULL, *compressed_ram_data = NULL;
  size_t disk_rom_length, disk_ram_length, block_size;
  libspectrum_dword flags = 0;
  int use_compression = 0;

  rom_data = libspectrum_snap_opus_rom( snap, 0 );
  disk_rom_length = 0x2000;
  ram_data = libspectrum_snap_opus_ram( snap, 0 );
  disk_ram_length = 0x800;

  compressed_ram_data = compressed_rom_data = NULL;

#ifdef HAVE_ZLIB_H

  if( compress ) {

    size_t compressed_rom_length, compressed_ram_length;

    error = libspectrum_zlib_compress( rom_data, disk_rom_length,
				       &compressed_rom_data, &compressed_rom_length );
    if( error ) return error;

    error = libspectrum_zlib_compress( ram_data, disk_ram_length,
				       &compressed_ram_data, &compressed_ram_length );
    if( error ) {
      if( compressed_rom_data ) libspectrum_free( compressed_rom_data );
      return error;
    }

    if( compress & LIBSPECTRUM_FLAG_SNAPSHOT_ALWAYS_COMPRESS ||
        (compressed_rom_length + compressed_ram_length) <
        (disk_rom_length + disk_ram_length ) ) {
      use_compression = 1;
      rom_data = compressed_rom_data;
      disk_rom_length = compressed_rom_length;
      ram_data = compressed_ram_data;
      disk_ram_length = compressed_ram_length;
    }

  }

#endif				/* #ifdef HAVE_ZLIB_H */

  block_size = 23 + disk_ram_length;
  if( libspectrum_snap_opus_custom_rom( snap ) ) {
    block_size += disk_rom_length;
  }

  write_chunk_header( buffer, ptr, length, ZXSTBID_OPUS, block_size );

  if( libspectrum_snap_opus_paged( snap ) ) flags |= ZXSTPLUSDF_PAGED;
  if( use_compression ) flags |= ZXSTPLUSDF_COMPRESSED;
  if( !libspectrum_snap_opus_direction( snap ) ) flags |= ZXSTPLUSDF_SEEKLOWER;
  if( libspectrum_snap_opus_custom_rom( snap ) ) flags |= ZXSTOPUSF_CUSTOMROM;
  libspectrum_write_dword( ptr, flags );

  libspectrum_write_dword( ptr, disk_ram_length );
  if( libspectrum_snap_opus_custom_rom( snap ) ) {
    libspectrum_write_dword( ptr, disk_rom_length );
  } else {
    libspectrum_write_dword( ptr, 0 );
  }
  *(*ptr)++ = libspectrum_snap_opus_control_a( snap );
  *(*ptr)++ = libspectrum_snap_opus_data_reg_a( snap );
  *(*ptr)++ = libspectrum_snap_opus_data_dir_a( snap );
  *(*ptr)++ = libspectrum_snap_opus_control_b( snap );
  *(*ptr)++ = libspectrum_snap_opus_data_reg_b( snap );
  *(*ptr)++ = libspectrum_snap_opus_data_dir_b( snap );
  *(*ptr)++ = libspectrum_snap_opus_drive_count( snap );
  *(*ptr)++ = libspectrum_snap_opus_track  ( snap );
  *(*ptr)++ = libspectrum_snap_opus_sector ( snap );
  *(*ptr)++ = libspectrum_snap_opus_data   ( snap );
  *(*ptr)++ = libspectrum_snap_opus_status ( snap );

  memcpy( *ptr, ram_data, disk_ram_length ); *ptr += disk_ram_length;

  if( libspectrum_snap_opus_custom_rom( snap ) ) {
    memcpy( *ptr, rom_data, disk_rom_length ); *ptr += disk_rom_length;
  }

  if( compressed_rom_data ) libspectrum_free( compressed_rom_data );
  if( compressed_ram_data ) libspectrum_free( compressed_ram_data );

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
write_plsd_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap, int compress  )
{
#ifdef HAVE_ZLIB_H
  libspectrum_error error;
#endif
  libspectrum_byte *rom_data, *ram_data; 
  libspectrum_byte *compressed_rom_data = NULL, *compressed_ram_data = NULL;
  size_t disk_rom_length, disk_ram_length, block_size;
  libspectrum_dword flags = 0;
  int use_compression = 0;

  rom_data = libspectrum_snap_plusd_rom( snap, 0 );
  disk_rom_length = 0x2000;
  ram_data = libspectrum_snap_plusd_ram( snap, 0 );
  disk_ram_length = 0x2000;

  compressed_ram_data = compressed_rom_data = NULL;

#ifdef HAVE_ZLIB_H

  if( compress ) {

    size_t compressed_rom_length, compressed_ram_length;

    error = libspectrum_zlib_compress( rom_data, disk_rom_length,
				       &compressed_rom_data, &compressed_rom_length );
    if( error ) return error;

    error = libspectrum_zlib_compress( ram_data, disk_ram_length,
				       &compressed_ram_data, &compressed_ram_length );
    if( error ) {
      if( compressed_rom_data ) libspectrum_free( compressed_rom_data );
      return error;
    }

    if( compress & LIBSPECTRUM_FLAG_SNAPSHOT_ALWAYS_COMPRESS ||
        (compressed_rom_length + compressed_ram_length) <
        (disk_rom_length + disk_ram_length ) ) {
      use_compression = 1;
      rom_data = compressed_rom_data;
      disk_rom_length = compressed_rom_length;
      ram_data = compressed_ram_data;
      disk_ram_length = compressed_ram_length;
    }

  }

#endif				/* #ifdef HAVE_ZLIB_H */

  block_size = 19 + disk_ram_length;
  if( libspectrum_snap_plusd_custom_rom( snap ) ) {
    block_size += disk_rom_length;
  }

  write_chunk_header( buffer, ptr, length, ZXSTBID_PLUSD, block_size );

  if( libspectrum_snap_plusd_paged( snap ) ) flags |= ZXSTPLUSDF_PAGED;
  if( use_compression ) flags |= ZXSTPLUSDF_COMPRESSED;
  if( !libspectrum_snap_plusd_direction( snap ) ) flags |= ZXSTPLUSDF_SEEKLOWER;
  libspectrum_write_dword( ptr, flags );

  libspectrum_write_dword( ptr, disk_ram_length );
  if( libspectrum_snap_plusd_custom_rom( snap ) ) {
    libspectrum_write_dword( ptr, disk_rom_length );
    *(*ptr)++ = ZXSTPDRT_CUSTOM;
  } else {
    libspectrum_write_dword( ptr, 0 );
    *(*ptr)++ = ZXSTPDRT_GDOS;
  }
  *(*ptr)++ = libspectrum_snap_plusd_control( snap );
  *(*ptr)++ = libspectrum_snap_plusd_drive_count( snap );
  *(*ptr)++ = libspectrum_snap_plusd_track  ( snap );
  *(*ptr)++ = libspectrum_snap_plusd_sector ( snap );
  *(*ptr)++ = libspectrum_snap_plusd_data   ( snap );
  *(*ptr)++ = libspectrum_snap_plusd_status ( snap );

  memcpy( *ptr, ram_data, disk_ram_length ); *ptr += disk_ram_length;

  if( libspectrum_snap_plusd_custom_rom( snap ) ) {
    memcpy( *ptr, rom_data, disk_rom_length ); *ptr += disk_rom_length;
  }

  if( compressed_rom_data ) libspectrum_free( compressed_rom_data );
  if( compressed_ram_data ) libspectrum_free( compressed_ram_data );

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
write_zxat_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap )
{
  libspectrum_word flags;

  write_chunk_header( buffer, ptr, length, ZXSTBID_ZXATASP, 8 );

  flags = 0;
  if( libspectrum_snap_zxatasp_upload ( snap ) ) flags |= ZXSTZXATF_UPLOAD;
  if( libspectrum_snap_zxatasp_writeprotect( snap ) )
    flags |= ZXSTZXATF_WRITEPROTECT;
  libspectrum_write_word( ptr, flags );

  *(*ptr)++ = libspectrum_snap_zxatasp_port_a( snap );
  *(*ptr)++ = libspectrum_snap_zxatasp_port_b( snap );
  *(*ptr)++ = libspectrum_snap_zxatasp_port_c( snap );
  *(*ptr)++ = libspectrum_snap_zxatasp_control( snap );
  *(*ptr)++ = libspectrum_snap_zxatasp_pages( snap );
  *(*ptr)++ = libspectrum_snap_zxatasp_current_page( snap );

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
write_atrp_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap, int page,
		  int compress )
{
  libspectrum_error error;
  const libspectrum_byte *data;

  data = libspectrum_snap_zxatasp_ram( snap, page );

  error = write_ram_page( buffer, ptr, length, ZXSTBID_ZXATASPRAMPAGE, data,
			  0x4000, page, compress, 0x00 );
  if( error ) return error;

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
write_zxcf_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap )
{
  libspectrum_word flags;

  write_chunk_header( buffer, ptr, length, ZXSTBID_ZXCF, 4 );

  flags = 0;
  if( libspectrum_snap_zxcf_upload( snap ) ) flags |= ZXSTZXCFF_UPLOAD;
  libspectrum_write_word( ptr, flags );

  *(*ptr)++ = libspectrum_snap_zxcf_memctl( snap );
  *(*ptr)++ = libspectrum_snap_zxcf_pages( snap );

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
write_cfrp_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap, int page,
		  int compress )
{
  libspectrum_error error;
  const libspectrum_byte *data;

  data = libspectrum_snap_zxcf_ram( snap, page );

  error = write_ram_page( buffer, ptr, length, ZXSTBID_ZXCFRAMPAGE, data,
			  0x4000, page, compress, 0x00 );
  if( error ) return error;

  return LIBSPECTRUM_ERROR_NONE;
}

#ifdef HAVE_ZLIB_H

static libspectrum_error
write_if2r_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		size_t *length, libspectrum_snap *snap )
{
  libspectrum_error error;
  libspectrum_byte *block_length, *data, *cart_size, *compressed_data;
  size_t data_length, compressed_length;

  /* 8 for the chunk header, 4 for the compressed cart size */
  libspectrum_make_room( buffer, 8 + 4, ptr, length );

  memcpy( *ptr, ZXSTBID_IF2ROM, 4 ); (*ptr) += 4;

  /* Store this location for later */
  block_length = *ptr; *ptr += 4;

  /* And this one */
  cart_size = *ptr; *ptr += 4;

  data = libspectrum_snap_interface2_rom( snap, 0 ); data_length = 0x4000;
  compressed_data = NULL;

  error = libspectrum_zlib_compress( data, data_length,
                                     &compressed_data, &compressed_length );
  if( error ) return error;

  libspectrum_write_dword( &block_length, 4 + compressed_length );
  libspectrum_write_dword( &cart_size, compressed_length );

  libspectrum_make_room( buffer, compressed_length, ptr, length );

  memcpy( *ptr, compressed_data, compressed_length ); *ptr += compressed_length;

  if( compressed_data ) libspectrum_free( compressed_data );

  return LIBSPECTRUM_ERROR_NONE;
}

#endif                         /* #ifdef HAVE_ZLIB_H */

static libspectrum_error
write_dock_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap, int exrom_dock,
                  const libspectrum_byte *data, int page, int writeable,
                  int compress )
{
  libspectrum_error error;
  libspectrum_byte extra_flags;

  extra_flags = 0;
  if( writeable  ) extra_flags |= ZXSTDOCKF_RAM;
  if( exrom_dock ) extra_flags |= ZXSTDOCKF_EXROMDOCK;

  error = write_ram_page( buffer, ptr, length, ZXSTBID_DOCK, data, 0x2000,
			  page, compress, extra_flags );
  if( error ) return error;

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
write_side_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap )
{
  write_chunk_header( buffer, ptr, length, ZXSTBID_SIMPLEIDE, 0 );
  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
write_drum_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap )
{
  write_chunk_header( buffer, ptr, length, ZXSTBID_SPECDRUM, 1 );

  *(*ptr)++ = libspectrum_snap_specdrum_dac( snap );

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
write_dide_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap, int compress )
{
#ifdef HAVE_ZLIB_H
  libspectrum_error error;
#endif
  libspectrum_byte *eprom_data = NULL;
  libspectrum_byte *compressed_eprom_data = NULL;
  size_t block_size;
  libspectrum_word flags = 0;
  libspectrum_word divide_eprom_length = 0;
  libspectrum_word uncompressed_eprom_length = 0;
  int use_compression = 0;

  eprom_data = libspectrum_snap_divide_eprom( snap, 0 );
  if( !eprom_data ) {
    libspectrum_print_error( LIBSPECTRUM_ERROR_LOGIC,
                             "DivIDE EPROM data is missing" );
    return LIBSPECTRUM_ERROR_LOGIC;
  }
  uncompressed_eprom_length = divide_eprom_length = 0x2000;

#ifdef HAVE_ZLIB_H

  if( eprom_data && compress ) {

    size_t compressed_eprom_length;

    error = libspectrum_zlib_compress( eprom_data, uncompressed_eprom_length,
                                       &compressed_eprom_data,
                                       &compressed_eprom_length );
    if( error ) return error;

    if( compress & LIBSPECTRUM_FLAG_SNAPSHOT_ALWAYS_COMPRESS ||
        compressed_eprom_length < uncompressed_eprom_length ) {
      use_compression = 1;
      eprom_data = compressed_eprom_data;
      divide_eprom_length = compressed_eprom_length;
    }

  }

#endif

  block_size = 4 + divide_eprom_length;

  write_chunk_header( buffer, ptr, length, ZXSTBID_DIVIDE, block_size );

  if( libspectrum_snap_divide_eprom_writeprotect( snap ) )
    flags |= ZXSTDIVIDE_EPROM_WRITEPROTECT;
  if( libspectrum_snap_divide_paged( snap ) ) flags |= ZXSTDIVIDE_PAGED;
  if( use_compression ) flags |= ZXSTDIVIDE_COMPRESSED;
  libspectrum_write_word( ptr, flags );

  *(*ptr)++ = libspectrum_snap_divide_control( snap );
  *(*ptr)++ = libspectrum_snap_divide_pages( snap );

  memcpy( *ptr, eprom_data, divide_eprom_length ); *ptr += divide_eprom_length;

  if( compressed_eprom_data ) libspectrum_free( compressed_eprom_data );

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
write_dirp_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap, int page,
		  int compress )
{
  libspectrum_error error;
  const libspectrum_byte *data;

  data = libspectrum_snap_divide_ram( snap, page );

  error = write_ram_page( buffer, ptr, length, ZXSTBID_DIVIDERAMPAGE, data,
			  0x2000, page, compress, 0x00 );
  if( error ) return error;

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
write_snet_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap, int compress )
{
  libspectrum_word flags = 0;

  write_chunk_header( buffer, ptr, length, ZXSTBID_SPECTRANET, 54 );

  if( libspectrum_snap_spectranet_paged( snap ) )
    flags |= ZXSTSNET_PAGED;
  if( libspectrum_snap_spectranet_paged_via_io( snap ) )
    flags |= ZXSTSNET_PAGED_VIA_IO;
  if( libspectrum_snap_spectranet_programmable_trap_active( snap ) )
    flags |= ZXSTSNET_PROGRAMMABLE_TRAP_ACTIVE;
  if( libspectrum_snap_spectranet_programmable_trap_msb( snap ) )
    flags |= ZXSTSNET_PROGRAMMABLE_TRAP_MSB;
  if( libspectrum_snap_spectranet_all_traps_disabled( snap ) )
    flags |= ZXSTSNET_ALL_DISABLED;
  if( libspectrum_snap_spectranet_rst8_trap_disabled( snap ) )
    flags |= ZXSTSNET_RST8_DISABLED;
  if( libspectrum_snap_spectranet_deny_downstream_a15( snap ) )
    flags |= ZXSTSNET_DENY_DOWNSTREAM_A15;
  if( libspectrum_snap_spectranet_nmi_flipflop( snap ) )
    flags |= ZXSTSNET_NMI_FLIPFLOP;
  libspectrum_write_word( ptr, flags );

  *(*ptr)++ = libspectrum_snap_spectranet_page_a( snap );
  *(*ptr)++ = libspectrum_snap_spectranet_page_b( snap );

  libspectrum_write_word( ptr,
    libspectrum_snap_spectranet_programmable_trap( snap ) );

  memcpy( *ptr, libspectrum_snap_spectranet_w5100( snap, 0 ), 0x30 );
  (*ptr) += 0x30;

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
write_snef_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap, int compress )
{
#ifdef HAVE_ZLIB_H
  libspectrum_error error;
#endif

  size_t flash_length;
  libspectrum_byte *flash_data;
  libspectrum_byte *compressed_flash_data = NULL;
  int flash_compressed = 0;

  size_t block_size;
  libspectrum_byte flags = 0;

  flash_data = libspectrum_snap_spectranet_flash( snap, 0 );
  flash_length = 0x20000;

#ifdef HAVE_ZLIB_H

  if( compress ) {
    size_t compressed_length;

    error = libspectrum_zlib_compress( flash_data, flash_length,
      &compressed_flash_data, &compressed_length );
    if( error ) return error;

    if( compress & LIBSPECTRUM_FLAG_SNAPSHOT_ALWAYS_COMPRESS ||
      compressed_length < flash_length ) {
      flash_compressed = 1;
      flash_data = compressed_flash_data;
      flash_length = compressed_length;
    }
  }

#endif

  block_size = 5 + flash_length;

  write_chunk_header( buffer, ptr, length, ZXSTBID_SPECTRANETFLASHPAGE,
                      block_size );

  if( flash_compressed )
    flags |= ZXSTSNEF_FLASH_COMPRESSED;
  *(*ptr)++ = flags;

  libspectrum_write_dword( ptr, flash_length );
  memcpy( *ptr, flash_data, flash_length ); *ptr += flash_length;

  if( flash_compressed )
    libspectrum_free( compressed_flash_data );

  return LIBSPECTRUM_ERROR_NONE;
}

static libspectrum_error
write_sner_chunk( libspectrum_byte **buffer, libspectrum_byte **ptr,
		  size_t *length, libspectrum_snap *snap, int compress )
{
#ifdef HAVE_ZLIB_H
  libspectrum_error error;
#endif

  size_t ram_length;
  libspectrum_byte *ram_data;
  libspectrum_byte *compressed_ram_data = NULL;
  int ram_compressed = 0;

  size_t block_size;
  libspectrum_byte flags = 0;

  ram_data = libspectrum_snap_spectranet_ram( snap, 0 );
  ram_length = 0x20000;

#ifdef HAVE_ZLIB_H

  if( compress ) {
    size_t compressed_length;

    error = libspectrum_zlib_compress( ram_data, ram_length,
      &compressed_ram_data, &compressed_length );
    if( error ) return error;

    if( compress & LIBSPECTRUM_FLAG_SNAPSHOT_ALWAYS_COMPRESS ||
      compressed_length < ram_length ) {
      ram_compressed = 1;
      ram_data = compressed_ram_data;
      ram_length = compressed_length;
    }
  }

#endif

  block_size = 5 + ram_length;

  write_chunk_header( buffer, ptr, length, ZXSTBID_SPECTRANETRAMPAGE,
                      block_size );

  if( ram_compressed )
    flags |= ZXSTSNER_RAM_COMPRESSED;
  *(*ptr)++ = flags;

  libspectrum_write_dword( ptr, ram_length );
  memcpy( *ptr, ram_data, ram_length ); *ptr += ram_length;

  if( ram_compressed )
    libspectrum_free( compressed_ram_data );

  return LIBSPECTRUM_ERROR_NONE;
}

static void
write_chunk_header( libspectrum_byte **buffer, libspectrum_byte **ptr,
		    size_t *length, const char *id,
		    libspectrum_dword block_length )
{
  libspectrum_make_room( buffer, 8 + block_length, ptr, length );
  memcpy( *ptr, id, 4 ); *ptr += 4;
  libspectrum_write_dword( ptr, block_length );
}
