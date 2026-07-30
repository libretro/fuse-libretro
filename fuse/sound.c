/* sound.c: Sound support
   Copyright (c) 2000-2016 Russell Marks, Matan Ziv-Av, Philip Kendall,
                           Fredrick Meunier, Patrik Rak

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

/* The AY white noise RNG algorithm is based on info from MAME's ay8910.c -
 * MAME's licence explicitly permits free use of info (even encourages it).
 */

#include <config.h>

#include "fuse.h"
#include "infrastructure/startup_manager.h"
#include "machine.h"
#include "movie.h"
#include "options.h"
#include "settings.h"
#include "sound.h"
#include "tape.h"
#include "timer/timer.h"
#include "ui/ui.h"
#include "sound/blipbuffer.h"

/* Do we have any of our sound devices available? */

/* configuration */
int sound_enabled = 0;		/* Are we currently using the sound card */

static int sound_enabled_ever = 0; /* whether sound has *ever* been in use; see
				      sound_ay_write() and sound_ay_reset() */
int sound_stereo_ay = SOUND_STEREO_AY_NONE; /* local copy of settings_current.stereo_ay */

/* assume all three tone channels together match the beeper volume (ish).
 * Must be <=127 for all channels; 50+2+(24*3) = 124.
 * (Now scaled up for 16-bit.)
 */
#define AMPL_BEEPER		( 50 * 256)
#define AMPL_TAPE		( 2 * 256 )
#define AMPL_AY_TONE		( 24 * 256 )	/* three of these */

/* max. number of sub-frame AY port writes allowed;
 * given the number of port writes theoretically possible in a
 * 50th I think this should be plenty.
 */
#define AY_CHANGE_MAX		8000

int sound_framesiz;

static int sound_channels;

static unsigned int ay_tone_levels[16];

/* TurboSound: chip index 0 is the machine's own AY chip, index 1 is
   TurboSound's second chip (only driven when ay_turbosound_enabled). */
#define TS_CHIPS 2

static unsigned int ay_tone_tick[TS_CHIPS][3], ay_tone_high[TS_CHIPS][3],
                     ay_noise_tick[TS_CHIPS];
static unsigned int ay_tone_cycles[TS_CHIPS], ay_env_cycles[TS_CHIPS];
static unsigned int ay_env_internal_tick[TS_CHIPS], ay_env_tick[TS_CHIPS];
static unsigned int ay_tone_period[TS_CHIPS][3], ay_noise_period[TS_CHIPS],
                     ay_env_period[TS_CHIPS];
static int ay_rng[TS_CHIPS];
static int ay_noise_toggle[TS_CHIPS];
static int ay_env_first[TS_CHIPS], ay_env_rev[TS_CHIPS], ay_env_counter[TS_CHIPS];

/* Local copy of the AY registers */
static libspectrum_byte sound_ay_registers[TS_CHIPS][16];

struct ay_change_tag
{
  libspectrum_dword tstates;
  unsigned char reg, val;
};

static struct ay_change_tag ay_change[TS_CHIPS][ AY_CHANGE_MAX ];
static int ay_change_count[TS_CHIPS];

Blip_Buffer *left_buf = NULL;
Blip_Buffer *right_buf = NULL;
blip_sample_t *samples = NULL;

Blip_Synth *left_beeper_synth = NULL, *right_beeper_synth = NULL;

Blip_Synth *ay_a_synth = NULL, *ay_b_synth = NULL, *ay_c_synth = NULL;
Blip_Synth *ay_a_synth_r = NULL, *ay_b_synth_r = NULL, *ay_c_synth_r = NULL;

/* TurboSound: second chip's synths, wired up identically to the above and
   only ever driven when ay_turbosound_enabled is set. */
Blip_Synth *ts_ay_a_synth = NULL, *ts_ay_b_synth = NULL, *ts_ay_c_synth = NULL;
Blip_Synth *ts_ay_a_synth_r = NULL, *ts_ay_b_synth_r = NULL, *ts_ay_c_synth_r = NULL;

Blip_Synth *left_specdrum_synth = NULL, *right_specdrum_synth = NULL;

Blip_Synth *left_covox_synth = NULL, *right_covox_synth = NULL;

struct speaker_type_tag
{
  int bass;
  double treble;
};

static struct speaker_type_tag speaker_type[] =
  { { 200, -37.0 }, { 1000, -67.0 }, { 0, 0.0 } };

static double
sound_get_volume( int volume )
{
  if( volume < 0 ) volume = 0;
  else if( volume > 100 ) volume = 100;

  return volume / 100.0;
}

/* Returns the emulation speed adjusted processor speed */
libspectrum_dword
sound_get_effective_processor_speed( void )
{
  return machine_current->timings.processor_speed / 100 * 
           settings_current.emulation_speed;
}

static int
sound_init_blip( Blip_Buffer **buf, Blip_Synth **synth )
{
  *buf = new_Blip_Buffer();
  blip_buffer_set_clock_rate( *buf, sound_get_effective_processor_speed() );
  /* Allow up to 1s of playback buffer - this allows us to cope with slowing
     down to 2% of speed where a single Speccy frame generates just under 1s
     of sound */
  if ( blip_buffer_set_sample_rate( *buf, settings_current.sound_freq, 1000 ) ) {
    sound_end();
    ui_error( UI_ERROR_ERROR, "out of memory at %s:%d", __FILE__, __LINE__ );
    return 0;
  }

  *synth = new_Blip_Synth();

  blip_synth_set_volume( *synth, sound_get_volume( settings_current.volume_beeper ) );
  blip_synth_set_output( *synth, *buf );

  blip_buffer_set_bass_freq( *buf, speaker_type[ option_enumerate_sound_speaker_type() ].bass );
  blip_synth_set_treble_eq( *synth, speaker_type[ option_enumerate_sound_speaker_type() ].treble );

  return 1;
}

static void
sound_ay_init( int chip )
{
  /* AY output doesn't match the claimed levels; these levels are based
   * on the measurements posted to comp.sys.sinclair in Dec 2001 by
   * Matthew Westcott, adjusted as I described in a followup to his post,
   * then scaled to 0..0xffff.
   */
  static const int levels[16] = {
    0x0000, 0x0385, 0x053D, 0x0770,
    0x0AD7, 0x0FD5, 0x15B0, 0x230C,
    0x2B4C, 0x43C1, 0x5A4B, 0x732F,
    0x9204, 0xAFF1, 0xD921, 0xFFFF
  };
  int f;

  /* scale the values down to fit */
  for( f = 0; f < 16; f++ )
    ay_tone_levels[f] = ( levels[f] * AMPL_AY_TONE + 0x8000 ) / 0xffff;

  ay_noise_tick[chip] = ay_noise_period[chip] = 0;
  ay_env_internal_tick[chip] = ay_env_tick[chip] = ay_env_period[chip] = 0;
  ay_tone_cycles[chip] = ay_env_cycles[chip] = 0;
  for( f = 0; f < 3; f++ )
    ay_tone_tick[chip][f] = ay_tone_high[chip][f] = 0, ay_tone_period[chip][f] = 1;

  ay_rng[chip] = 1;
  ay_noise_toggle[chip] = 0;
  ay_env_first[chip] = 1;
  ay_env_rev[chip] = 0;
  ay_env_counter[chip] = 15;

  ay_change_count[chip] = 0;
}

#ifndef UI_WIN32
#define MIN_SPEED_PERCENTAGE 2
#define MAX_SPEED_PERCENTAGE 500
#else                        /* #ifndef UI_WIN32 */
/* We are limiting speed until bugs in the DirectSound driver are resolved, see
   [bugs:#364] for more details */
#define MIN_SPEED_PERCENTAGE 50
#define MAX_SPEED_PERCENTAGE 300
#endif                       /* #ifndef UI_WIN32 */

static int
is_in_sound_enabled_range( void )
{
  return settings_current.emulation_speed >= MIN_SPEED_PERCENTAGE &&
    settings_current.emulation_speed <= MAX_SPEED_PERCENTAGE;
}

/* Set up (volume/treble/output routing) one AY chip's three tone
   Blip_Synths. Used for both the machine's own AY chip and, when
   TurboSound is enabled, the second chip. */
static void
sound_init_ay_synths( Blip_Synth **a_synth, Blip_Synth **b_synth,
                       Blip_Synth **c_synth, Blip_Synth **a_synth_r,
                       Blip_Synth **b_synth_r, Blip_Synth **c_synth_r,
                       double treble )
{
  Blip_Synth **left_synth, **mid_synth, **mid_synth_r, **right_synth;

  *a_synth = new_Blip_Synth();
  blip_synth_set_volume( *a_synth, sound_get_volume( settings_current.volume_ay ) );
  blip_synth_set_treble_eq( *a_synth, treble );

  *b_synth = new_Blip_Synth();
  blip_synth_set_volume( *b_synth, sound_get_volume( settings_current.volume_ay ) );
  blip_synth_set_treble_eq( *b_synth, treble );

  *c_synth = new_Blip_Synth();
  blip_synth_set_volume( *c_synth, sound_get_volume( settings_current.volume_ay ) );
  blip_synth_set_treble_eq( *c_synth, treble );

  /* important to override these settings if not using stereo
   * (it would probably be confusing to mess with the stereo
   * settings in settings_current though, which is why we make copies
   * rather than using the real ones).
   */
  *a_synth_r = NULL;
  *b_synth_r = NULL;
  *c_synth_r = NULL;

  if( sound_stereo_ay != SOUND_STEREO_AY_NONE ) {
    /* Attach the Blip_Synth's we've already created as appropriate, and
     * create one more Blip_Synth for the middle channel's right buffer. */
    if( sound_stereo_ay == SOUND_STEREO_AY_ACB ) {
      left_synth = a_synth;
      mid_synth = c_synth;
      mid_synth_r = c_synth_r;
      right_synth = b_synth;
    } else if ( sound_stereo_ay == SOUND_STEREO_AY_ABC ) {
      left_synth = a_synth;
      mid_synth = b_synth;
      mid_synth_r = b_synth_r;
      right_synth = c_synth;
    } else {
      ui_error( UI_ERROR_ERROR, "unknown AY stereo separation type: %d", sound_stereo_ay );
      fuse_abort();
    }

    blip_synth_set_output( *left_synth, left_buf );
    blip_synth_set_output( *mid_synth, left_buf );
    blip_synth_set_output( *right_synth, right_buf );

    *mid_synth_r = new_Blip_Synth();
    blip_synth_set_volume( *mid_synth_r,
                           sound_get_volume( settings_current.volume_ay ) );
    blip_synth_set_output( *mid_synth_r, right_buf );
    blip_synth_set_treble_eq( *mid_synth_r, treble );
  } else {
    blip_synth_set_output( *a_synth, left_buf );
    blip_synth_set_output( *b_synth, left_buf );
    blip_synth_set_output( *c_synth, left_buf );
  }
}

void
sound_init( const char *device )
{
  float hz;
  double treble;

  /* Allow sound as long as emulation speed is greater than 2%
     (less than that and a single Speccy frame generates more
     than a seconds worth of sound which is bigger than the
     maximum Blip_Buffer of 1 second) */
  if( !( !sound_enabled && settings_current.sound &&
         is_in_sound_enabled_range() ) )
    return;

  /* only try for stereo if we need it */
  sound_stereo_ay = option_enumerate_sound_stereo_ay();

  if( settings_current.sound &&
      sound_lowlevel_init( device, &settings_current.sound_freq,
                           &sound_stereo_ay ) )
    return;

  if( !sound_init_blip(&left_buf, &left_beeper_synth) ) return;
  if( sound_stereo_ay != SOUND_STEREO_AY_NONE &&
      !sound_init_blip(&right_buf, &right_beeper_synth) )
    return;

  treble = speaker_type[ option_enumerate_sound_speaker_type() ].treble;

  sound_init_ay_synths( &ay_a_synth, &ay_b_synth, &ay_c_synth,
                         &ay_a_synth_r, &ay_b_synth_r, &ay_c_synth_r, treble );
  sound_init_ay_synths( &ts_ay_a_synth, &ts_ay_b_synth, &ts_ay_c_synth,
                         &ts_ay_a_synth_r, &ts_ay_b_synth_r, &ts_ay_c_synth_r,
                         treble );

  left_specdrum_synth = new_Blip_Synth();
  blip_synth_set_volume( left_specdrum_synth,
                         sound_get_volume( settings_current.volume_specdrum ) );
  blip_synth_set_output( left_specdrum_synth, left_buf );
  blip_synth_set_treble_eq( left_specdrum_synth, treble );

  left_covox_synth = new_Blip_Synth();
  blip_synth_set_volume( left_covox_synth,
                         sound_get_volume( settings_current.volume_covox ) );
  blip_synth_set_output( left_covox_synth, left_buf );
  blip_synth_set_treble_eq( left_covox_synth, treble );

  if( sound_stereo_ay != SOUND_STEREO_AY_NONE ) {
    right_specdrum_synth = new_Blip_Synth();
    blip_synth_set_volume( right_specdrum_synth,
                           sound_get_volume( settings_current.volume_specdrum ) );
    blip_synth_set_output( right_specdrum_synth, right_buf );
    blip_synth_set_treble_eq( right_specdrum_synth, treble );

    right_covox_synth = new_Blip_Synth();
    blip_synth_set_volume( right_covox_synth,
                           sound_get_volume( settings_current.volume_covox ) );
    blip_synth_set_output( right_covox_synth, right_buf );
    blip_synth_set_treble_eq( right_covox_synth, treble );
  }

  sound_enabled = sound_enabled_ever = 1;

  /* retroarch expects 2 channels regardless of whether the audio is mono or not */
  sound_channels = 2;

  /* Adjust relative processor speed to deal with adjusting sound generation
     frequency against emulation speed (more flexible than adjusting generated
     sample rate) */
  hz = ( float )sound_get_effective_processor_speed() /
                machine_current->timings.tstates_per_frame;

  /* Size of audio data we will get from running a single Spectrum frame */
  sound_framesiz = ( float )settings_current.sound_freq / hz;
  sound_framesiz++;

  samples = libspectrum_new0( blip_sample_t, sound_framesiz * sound_channels );
  /* initialize movie settings... */
  movie_init_sound( settings_current.sound_freq, sound_stereo_ay );

}

void
sound_pause( void )
{
  if( sound_enabled )
    sound_end();
}

void
sound_unpause( void )
{
  /* No sound if fastloading in progress */
  if( settings_current.fastload && timer_fastloading_active() )
    return;

  sound_init( settings_current.sound_device );
}

void
sound_end( void )
{
  if( sound_enabled ) {
    delete_Blip_Synth( &left_beeper_synth );
    delete_Blip_Synth( &right_beeper_synth );

    delete_Blip_Synth( &ay_a_synth );
    delete_Blip_Synth( &ay_b_synth );
    delete_Blip_Synth( &ay_c_synth );
    delete_Blip_Synth( &ay_a_synth_r );
    delete_Blip_Synth( &ay_b_synth_r );
    delete_Blip_Synth( &ay_c_synth_r );

    delete_Blip_Synth( &ts_ay_a_synth );
    delete_Blip_Synth( &ts_ay_b_synth );
    delete_Blip_Synth( &ts_ay_c_synth );
    delete_Blip_Synth( &ts_ay_a_synth_r );
    delete_Blip_Synth( &ts_ay_b_synth_r );
    delete_Blip_Synth( &ts_ay_c_synth_r );

    delete_Blip_Synth( &left_specdrum_synth );
    delete_Blip_Synth( &right_specdrum_synth );

    delete_Blip_Synth( &left_covox_synth );
    delete_Blip_Synth( &right_covox_synth );

    delete_Blip_Buffer( &left_buf );
    delete_Blip_Buffer( &right_buf );

    if( settings_current.sound ) 
      sound_lowlevel_end();
    libspectrum_free( samples );
    sound_enabled = 0;
  }
}

void
sound_register_startup( void )
{
  startup_manager_module dependencies[] = { STARTUP_MANAGER_MODULE_SETUID };
  startup_manager_register( STARTUP_MANAGER_MODULE_SOUND, dependencies,
                            ARRAY_SIZE( dependencies ), NULL, NULL, sound_end );
}

static inline void
ay_do_tone( int chip, int level, unsigned int tone_count, int *var, int chan )
{
  *var = 0;

  ay_tone_tick[ chip ][ chan ] += tone_count;

  if( ay_tone_tick[ chip ][ chan ] >= ay_tone_period[ chip ][ chan ] ) {
    ay_tone_tick[ chip ][ chan ] -= ay_tone_period[ chip ][ chan ];
    ay_tone_high[ chip ][ chan ] = !ay_tone_high[ chip ][ chan ];
  }

  if( level ) {
    if( ay_tone_high[ chip ][ chan ] )
      *var = level;
    else {
      *var = 0;
    }
  }
}

/* bitmasks for envelope */
#define AY_ENV_CONT	8
#define AY_ENV_ATTACK	4
#define AY_ENV_ALT	2
#define AY_ENV_HOLD	1

/* the AY steps down the external clock by 16 for tone and noise
   generators */
#define AY_CLOCK_DIVISOR 16
/* all Spectrum models and clones with an AY seem to count down the
   master clock by 2 to drive the AY */
#define AY_CLOCK_RATIO 2

static void
sound_ay_overlay( int chip )
{
  int tone_level[3];
  int mixer, envshape;
  int g, level;
  libspectrum_dword f;
  struct ay_change_tag *change_ptr = ay_change[ chip ];
  int changes_left = ay_change_count[ chip ];
  int reg, r;
  int chan1, chan2, chan3;
  int last_chan1 = 0, last_chan2 = 0, last_chan3 = 0;
  unsigned int tone_count, noise_count;
  Blip_Synth *a_synth, *b_synth, *c_synth;
  Blip_Synth *a_synth_r, *b_synth_r, *c_synth_r;

  /* If no AY chip, don't produce any AY sound (!) */
  if( !( periph_is_active( PERIPH_TYPE_FULLER) ||
         periph_is_active( PERIPH_TYPE_MELODIK ) ||
         machine_current->capabilities & LIBSPECTRUM_MACHINE_CAPABILITY_AY ) )
    return;

  if( chip == 0 ) {
    a_synth = ay_a_synth; b_synth = ay_b_synth; c_synth = ay_c_synth;
    a_synth_r = ay_a_synth_r; b_synth_r = ay_b_synth_r; c_synth_r = ay_c_synth_r;
  } else {
    a_synth = ts_ay_a_synth; b_synth = ts_ay_b_synth; c_synth = ts_ay_c_synth;
    a_synth_r = ts_ay_a_synth_r; b_synth_r = ts_ay_b_synth_r; c_synth_r = ts_ay_c_synth_r;
  }

  for( f = 0; f < machine_current->timings.tstates_per_frame;
       f+= AY_CLOCK_DIVISOR * AY_CLOCK_RATIO ) {
    /* update ay registers. */
    while( changes_left && f >= change_ptr->tstates ) {
      sound_ay_registers[ chip ][ reg = change_ptr->reg ] = change_ptr->val;
      change_ptr++;
      changes_left--;

      /* fix things as needed for some register changes */
      switch ( reg ) {
      case 0: case 1: case 2: case 3: case 4: case 5:
        r = reg >> 1;
        /* a zero-len period is the same as 1 */
        ay_tone_period[ chip ][r] = ( sound_ay_registers[ chip ][ reg & ~1 ] |
                              ( sound_ay_registers[ chip ][ reg | 1 ] & 15 ) << 8 );
        if( !ay_tone_period[ chip ][r] )
          ay_tone_period[ chip ][r]++;

        /* important to get this right, otherwise e.g. Ghouls 'n' Ghosts
         * has really scratchy, horrible-sounding vibrato.
         */
        if( ay_tone_tick[ chip ][r] >= ay_tone_period[ chip ][r] * 2 )
          ay_tone_tick[ chip ][r] %= ay_tone_period[ chip ][r] * 2;
        break;
      case 6:
        ay_noise_tick[ chip ] = 0;
        ay_noise_period[ chip ] = ( sound_ay_registers[ chip ][ reg ] & 31 );
        break;
      case 11: case 12:
        ay_env_period[ chip ] =
          sound_ay_registers[ chip ][11] | ( sound_ay_registers[ chip ][12] << 8 );
        break;
      case 13:
        ay_env_internal_tick[ chip ] = ay_env_tick[ chip ] = ay_env_cycles[ chip ] = 0;
        ay_env_first[ chip ] = 1;
        ay_env_rev[ chip ] = 0;
        ay_env_counter[ chip ] = ( sound_ay_registers[ chip ][13] & AY_ENV_ATTACK ) ? 0 : 15;
        break;
      }
    }

    /* the tone level if no enveloping is being used */
    for( g = 0; g < 3; g++ )
      tone_level[g] = ay_tone_levels[ sound_ay_registers[ chip ][ 8 + g ] & 15 ];

    /* envelope */
    envshape = sound_ay_registers[ chip ][13];
    level = ay_tone_levels[ ay_env_counter[ chip ] ];

    for( g = 0; g < 3; g++ )
      if( sound_ay_registers[ chip ][ 8 + g ] & 16 )
        tone_level[g] = level;

    /* envelope output counter gets incr'd every 16 AY cycles. */
    ay_env_cycles[ chip ] += AY_CLOCK_DIVISOR;
    noise_count = 0;
    while( ay_env_cycles[ chip ] >= 16 ) {
      ay_env_cycles[ chip ] -= 16;
      noise_count++;
      ay_env_tick[ chip ]++;
      while( ay_env_tick[ chip ] >= ay_env_period[ chip ] ) {
        ay_env_tick[ chip ] -= ay_env_period[ chip ];

        /* do a 1/16th-of-period incr/decr if needed */
        if( ay_env_first[ chip ] ||
            ( ( envshape & AY_ENV_CONT ) && !( envshape & AY_ENV_HOLD ) ) ) {
          if( ay_env_rev[ chip ] )
            ay_env_counter[ chip ] -= ( envshape & AY_ENV_ATTACK ) ? 1 : -1;
          else
            ay_env_counter[ chip ] += ( envshape & AY_ENV_ATTACK ) ? 1 : -1;
          if( ay_env_counter[ chip ] < 0 )
            ay_env_counter[ chip ] = 0;
          if( ay_env_counter[ chip ] > 15 )
            ay_env_counter[ chip ] = 15;
        }

        ay_env_internal_tick[ chip ]++;
        while( ay_env_internal_tick[ chip ] >= 16 ) {
          ay_env_internal_tick[ chip ] -= 16;

          /* end of cycle */
          if( !( envshape & AY_ENV_CONT ) )
            ay_env_counter[ chip ] = 0;
          else {
            if( envshape & AY_ENV_HOLD ) {
              if( ay_env_first[ chip ] && ( envshape & AY_ENV_ALT ) )
                ay_env_counter[ chip ] = ( ay_env_counter[ chip ] ? 0 : 15 );
            } else {
              /* non-hold */
              if( envshape & AY_ENV_ALT )
                ay_env_rev[ chip ] = !ay_env_rev[ chip ];
              else
                ay_env_counter[ chip ] = ( envshape & AY_ENV_ATTACK ) ? 0 : 15;
            }
          }

          ay_env_first[ chip ] = 0;
        }

        /* don't keep trying if period is zero */
        if( !ay_env_period[ chip ] )
          break;
      }
    }

    /* generate tone+noise... or neither.
     * (if no tone/noise is selected, the chip just shoves the
     * level out unmodified. This is used by some sample-playing
     * stuff.)
     */
    chan1 = tone_level[0];
    chan2 = tone_level[1];
    chan3 = tone_level[2];
    mixer = sound_ay_registers[ chip ][7];

    ay_tone_cycles[ chip ] += AY_CLOCK_DIVISOR;
    tone_count = ay_tone_cycles[ chip ] >> 3;
    ay_tone_cycles[ chip ] &= 7;

    if( ( mixer & 1 ) == 0 ) {
      level = chan1;
      ay_do_tone( chip, level, tone_count, &chan1, 0 );
    }
    if( ( mixer & 0x08 ) == 0 && ay_noise_toggle[ chip ] )
      chan1 = 0;

    if( ( mixer & 2 ) == 0 ) {
      level = chan2;
      ay_do_tone( chip, level, tone_count, &chan2, 1 );
    }
    if( ( mixer & 0x10 ) == 0 && ay_noise_toggle[ chip ] )
      chan2 = 0;

    if( ( mixer & 4 ) == 0 ) {
      level = chan3;
      ay_do_tone( chip, level, tone_count, &chan3, 2 );
    }
    if( ( mixer & 0x20 ) == 0 && ay_noise_toggle[ chip ] )
      chan3 = 0;

    if( last_chan1 != chan1 ) {
      blip_synth_update( a_synth, f, chan1 );
      if( a_synth_r ) blip_synth_update( a_synth_r, f, chan1 );
      last_chan1 = chan1;
    }
    if( last_chan2 != chan2 ) {
      blip_synth_update( b_synth, f, chan2 );
      if( b_synth_r ) blip_synth_update( b_synth_r, f, chan2 );
      last_chan2 = chan2;
    }
    if( last_chan3 != chan3 ) {
      blip_synth_update( c_synth, f, chan3 );
      if( c_synth_r ) blip_synth_update( c_synth_r, f, chan3 );
      last_chan3 = chan3;
    }

    /* update noise RNG/filter */
    ay_noise_tick[ chip ] += noise_count;
    while( ay_noise_tick[ chip ] >= ay_noise_period[ chip ] ) {
      ay_noise_tick[ chip ] -= ay_noise_period[ chip ];

      if( ( ay_rng[ chip ] & 1 ) ^ ( ( ay_rng[ chip ] & 2 ) ? 1 : 0 ) )
        ay_noise_toggle[ chip ] = !ay_noise_toggle[ chip ];

      /* rng is 17-bit shift reg, bit 0 is output.
       * input is bit 0 xor bit 3.
       */
      if( ay_rng[ chip ] & 1 ) {
        ay_rng[ chip ] ^= 0x24000;
      }
      ay_rng[ chip ] >>= 1;

      /* don't keep trying if period is zero */
      if( !ay_noise_period[ chip ] )
        break;
    }
  }
}

/* don't make the change immediately; record it for later,
 * to be made by sound_frame() (via sound_ay_overlay()).
 */
void
sound_ay_write( int chip, int reg, int val, libspectrum_dword now )
{
  if( ay_change_count[ chip ] < AY_CHANGE_MAX ) {
    ay_change[ chip ][ ay_change_count[ chip ] ].tstates = now;
    ay_change[ chip ][ ay_change_count[ chip ] ].reg = ( reg & 15 );
    ay_change[ chip ][ ay_change_count[ chip ] ].val = val;
    ay_change_count[ chip ]++;
  }
}

/* no need to call this initially, but should be called
 * on reset otherwise.
 */
void
sound_ay_reset( void )
{
  int f, chip;

  for( chip = 0; chip < TS_CHIPS; chip++ ) {
    /* recalculate timings based on new machines ay clock */
    sound_ay_init( chip );

    ay_change_count[ chip ] = 0;
    for( f = 0; f < 16; f++ )
      sound_ay_write( chip, f, 0, 0 );
    for( f = 0; f < 3; f++ )
      ay_tone_high[ chip ][f] = 0;
    ay_tone_cycles[ chip ] = ay_env_cycles[ chip ] = 0;
  }
}

/*
 * sound_specdrum_write - very simple routine
 * as the output is already a digitized waveform
 */
void
sound_specdrum_write( libspectrum_word port GCC_UNUSED, libspectrum_byte val )
{
  if( periph_is_active( PERIPH_TYPE_SPECDRUM ) ) {
    blip_synth_update( left_specdrum_synth, tstates, ( val - 128) * 128);
    if( right_specdrum_synth ) {
      blip_synth_update( right_specdrum_synth, tstates, ( val - 128) * 128);
    }
    machine_current->specdrum.specdrum_dac = val - 128;
  }
}

/*
 * sound_covox_write - very simple routine
 * as the output is already a digitized waveform
 */
void
sound_covox_write( libspectrum_word port GCC_UNUSED, libspectrum_byte val )
{
  if( periph_is_active( PERIPH_TYPE_COVOX_FB ) ||
      periph_is_active( PERIPH_TYPE_COVOX_DD ) ) {
    blip_synth_update( left_covox_synth, tstates, val * 128);
    if( right_covox_synth ) {
      blip_synth_update( right_covox_synth, tstates, val * 128);
    }
    machine_current->covox.covox_dac = val;
  }
}

void
sound_frame( void )
{
  long count;

  if( !sound_enabled )
    return;

  /* overlay AY sound */
  sound_ay_overlay( 0 );
  if( ay_turbosound_enabled )
    sound_ay_overlay( 1 );

  blip_buffer_end_frame( left_buf, machine_current->timings.tstates_per_frame );

  if( sound_stereo_ay != SOUND_STEREO_AY_NONE ) {
    blip_buffer_end_frame( right_buf, machine_current->timings.tstates_per_frame );

    /* Read left channel into even samples, right channel into odd samples:
       LRLRLRLRLR... */
    count = blip_buffer_read_samples( left_buf, samples, sound_framesiz, 1 );
    blip_buffer_read_samples( right_buf, samples + 1, count, 1 );
    count <<= 1;
  } else {
    long i;
    count = blip_buffer_read_samples( left_buf, samples, sound_framesiz, 1 );
    for (i = 0; i < count; ++i)
    {
      samples[i*2+1] = samples[i*2];
    }
    count <<= 1;
  }

  if( settings_current.sound ) 
    sound_lowlevel_frame( samples, count );

  if( movie_recording )
      movie_add_sound( samples, count );
  ay_change_count[0] = ay_change_count[1] = 0;
}

void
sound_beeper( libspectrum_dword at_tstates, int on )
{
  static int beeper_ampl[] = { 0, AMPL_TAPE, AMPL_BEEPER,
                               AMPL_BEEPER+AMPL_TAPE };
  int val;

  if( !sound_enabled ) return;

  if( tape_is_playing() ) {
    /* Timex machines have no loading noise */
    if( !settings_current.sound_load || machine_current->timex ) on = on & 0x02;
  } else {
    /* ULA book says that MIC only isn't enough to drive the speaker as output
       voltage is below the 1.4v threshold */
    if( on == 1 ) on = 0;
  }

  val = beeper_ampl[on];

  blip_synth_update( left_beeper_synth, at_tstates, val );
  if( sound_stereo_ay != SOUND_STEREO_AY_NONE )
    blip_synth_update( right_beeper_synth, at_tstates, val );
}
