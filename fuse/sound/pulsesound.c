/* pulsesound.c: pulseaudio (Linux) sound I/O
   Copyright (c) 2010-2019 Grzegorz Jablonski, Sergio Baldoví

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

*/

#include <config.h>

#include <stdio.h>
#include <string.h>

#include <libspectrum.h>
#include <pulse/error.h>
#include <pulse/simple.h>
#include <pulse/timeval.h>

#include "sound.h"

#define PULSEAUDIO_DEBUG 0

static pa_simple *pulse_s;
static int verbose = 0;

void
sound_lowlevel_end( void )
{
  pa_simple_free( pulse_s );
}

int
sound_lowlevel_init( const char *device, int *freqptr, int *stereoptr )
{
  pa_sample_spec ss;
  pa_buffer_attr buf;
  unsigned int n;
  unsigned int val;
  const char *option;
  char tmp;
  int err;
  int bsize = 0;
  int bdelay = 30; /* default to 30ms */

  /* scan explicitly given parameters */
  option = device;
  while( option && *option ) {
    tmp = '*';
    if( ( err = sscanf( option, " tlength=%ims %n%c", &val, &n, &tmp ) > 0 ) &&
        ( tmp == ',' || strlen( option ) == n ) ) {
      if( val < 1 ) {
        fprintf( stderr, "Bad value for PULSEAUDIO tlength, using default\n" );
      } else {
        bdelay = val;
      }
    } else if( ( err = sscanf( option, " tlength=%i %n%c", &val, &n, &tmp ) > 0 ) &&
        ( tmp == ',' || strlen( option ) == n ) ) {
      if( val < 1 ) {
        fprintf( stderr, "Bad value for PULSEAUDIO tlength, using default\n" );
      } else {
        bsize = val;
      }
    } else if( ( err = sscanf( option, " verbose %n%c", &n, &tmp ) == 1 ) &&
              ( tmp == ',' || ( strlen( option ) == n ) ) ) {
      verbose = 1;
    }

    option += n + ( tmp == ',' );
  }

#if defined WORDS_BIGENDIAN
  ss.format = PA_SAMPLE_S16BE;
#else
  ss.format = PA_SAMPLE_S16LE;
#endif

  ss.channels = ( *stereoptr )? 2 : 1;
  ss.rate = *freqptr;

  /* Reduce latency to 30ms or the user-defined value. pulseaudio does
     not guarantee that */
  buf.tlength = (bsize)? bsize :
                         pa_usec_to_bytes( bdelay * PA_USEC_PER_MSEC, &ss );

  buf.maxlength = buf.tlength * 4;
  buf.minreq = buf.tlength / 4;
  buf.prebuf = (uint32_t) -1;
  buf.fragsize = (uint32_t) -1;

  if( verbose ) {
    fprintf( stdout, "buffer requested: maxlength=%lu, tlength=%lu, prebuf=%lu,"
             " minreq=%lu\n",
             (unsigned long) buf.maxlength,
             (unsigned long) buf.tlength,
             (unsigned long) buf.prebuf,
             (unsigned long) buf.minreq );
  }

  pulse_s = pa_simple_new( NULL, PACKAGE, PA_STREAM_PLAYBACK, NULL,
                           "Spectrum", &ss, NULL, &buf, &err );
  if( pulse_s == NULL ) {
    fprintf( stderr, "pulseaudio: pa_simple_new() failed: %s\n",
             pa_strerror( err ) );
    return 1;
  }

  return 0;
}

void
sound_lowlevel_frame( libspectrum_signed_word *data, int len )
{
  int retval, error;

  /* Measure sound lag */
  if( PULSEAUDIO_DEBUG ) {
    pa_usec_t latency;
    latency = pa_simple_get_latency( pulse_s, &error );
    if( latency == (pa_usec_t) - 1 ) {
      fprintf( stderr, "pulseaudio: pa_simple_get_latency() failed: %s\n",
               pa_strerror( error ) );
    } else {
      fprintf( stderr, "%0.0f ", (float)latency / PA_USEC_PER_MSEC );
    }
  }

  retval = pa_simple_write( pulse_s, data, 2 * len, &error );

  if( verbose && retval < 0 ) {
    fprintf( stderr, "pulseaudio: pa_simple_write() failed: %s\n",
             pa_strerror( error ) );
  }
}
