/* timings.c: Timing routines
   Copyright (c) 2003 Philip Kendall

   $Id: timings.c 4866 2013-01-30 19:39:05Z pak21 $

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

/* The frame timings of a machine */
typedef struct timings_t {

  /* Processor speed in Hz */
  libspectrum_dword processor_speed;

  /* AY clock speed in Hz */
  libspectrum_dword ay_speed;

  /* Line timings in tstates */
  libspectrum_word left_border, horizontal_screen, right_border,
	           horizontal_retrace;

  /* Frame timings in lines */
  libspectrum_word top_border, vertical_screen, bottom_border,
	           vertical_retrace;

  /* How long does an interrupt last in tstates */
  libspectrum_word interrupt_length;

  /* How long after interrupt is the top-level pixel of the main screen
     displayed */
  libspectrum_dword top_left_pixel;

} timings_t;

/* The actual data from which the full timings are constructed */
static timings_t base_timings[] = {

                   /* /- Horizonal -\  /-- Vertical -\ */
  /* 48K */
  { 3500000,       0, 24, 128, 24, 48, 48, 192, 48, 24, 32, 14336 },
  /* TC2048 */
  { 3500000,       0, 24, 128, 24, 48, 48, 192, 48, 24, 32, 14321 },
  /* 128K */
  { 3546900, 1773400, 24, 128, 24, 52, 48, 192, 48, 23, 36, 14362 },
  /* +2 */
  { 3546900, 1773400, 24, 128, 24, 52, 48, 192, 48, 23, 36, 14362 },
  /* Pentagon */
  { 3584000, 1792000, 36, 128, 28, 32, 64, 192, 48, 16, 36, 17988 },
  /* +2A */
  { 3546900, 1773400, 24, 128, 24, 52, 48, 192, 48, 23, 32, 14365 },
  /* +3 */
  { 3546900, 1773400, 24, 128, 24, 52, 48, 192, 48, 23, 32, 14365 },
  /* Unknown machine */
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  /* 16K */
  { 3500000,       0, 24, 128, 24, 48, 48, 192, 48, 24, 32, 14336 },
  /* TC2068 */
  { 3500000, 1750000, 24, 128, 24, 48, 48, 192, 48, 24, 32, 14321 },
  /* Scorpion */
  { 3500000, 1750000, 24, 128, 32, 40, 48, 192, 48, 24, 36, 14336 },
  /* +3e */
  { 3546900, 1773400, 24, 128, 24, 52, 48, 192, 48, 23, 32, 14365 },
  /* SE */
  { 3500000, 1750000, 24, 128, 24, 48, 47, 192, 48, 25, 32, 14336 },
  /* TS2068 */
  { 3528000, 1764000, 24, 128, 24, 48, 24, 192, 25, 21, 32,  9169 },
  /* Pentagon 512K */
  { 3584000, 1792000, 36, 128, 28, 32, 64, 192, 48, 16, 36, 17988 },
  /* Pentagon 1024K */
  { 3584000, 1792000, 36, 128, 28, 32, 64, 192, 48, 16, 36, 17988 },
  /* 48K NTSC */
  { 3527500,       0, 24, 128, 24, 48, 24, 192, 25, 23, 32,  8960 },
  /* 128Ke */
  { 3546900, 1773400, 24, 128, 24, 52, 48, 192, 48, 23, 32, 14365 },
};

libspectrum_dword
libspectrum_timings_processor_speed( libspectrum_machine machine )
{
  return base_timings[ machine ].processor_speed;
}

libspectrum_dword
libspectrum_timings_ay_speed( libspectrum_machine machine )
{
  return base_timings[ machine ].ay_speed;
}

libspectrum_word
libspectrum_timings_left_border( libspectrum_machine machine )
{
  return base_timings[ machine ].left_border;
}

libspectrum_word
libspectrum_timings_horizontal_screen( libspectrum_machine machine )
{
  return base_timings[ machine ].horizontal_screen;
}

libspectrum_word
libspectrum_timings_right_border( libspectrum_machine machine )
{
  return base_timings[ machine ].right_border;
}

libspectrum_word
libspectrum_timings_horizontal_retrace( libspectrum_machine machine )
{
  return base_timings[ machine ].horizontal_retrace;
}

libspectrum_word
libspectrum_timings_top_border( libspectrum_machine machine )
{
  return base_timings[ machine ].top_border;
}

libspectrum_word
libspectrum_timings_vertical_screen( libspectrum_machine machine )
{
  return base_timings[ machine ].vertical_screen;
}

libspectrum_word
libspectrum_timings_bottom_border( libspectrum_machine machine )
{
  return base_timings[ machine ].bottom_border;
}

libspectrum_word
libspectrum_timings_vertical_retrace( libspectrum_machine machine )
{
  return base_timings[ machine ].vertical_retrace;
}

libspectrum_word
libspectrum_timings_interrupt_length( libspectrum_machine machine )
{
  return base_timings[ machine ].interrupt_length;
}

libspectrum_word
libspectrum_timings_top_left_pixel( libspectrum_machine machine )
{
  return base_timings[ machine ].top_left_pixel;
}

libspectrum_word
libspectrum_timings_tstates_per_line( libspectrum_machine machine )
{
  return base_timings[ machine ].left_border        +
         base_timings[ machine ].horizontal_screen  +
         base_timings[ machine ].right_border       +
	 base_timings[ machine ].horizontal_retrace;
}

libspectrum_word
libspectrum_timings_lines_per_frame( libspectrum_machine machine )
{
  return base_timings[ machine ].top_border       +
	 base_timings[ machine ].vertical_screen  +
	 base_timings[ machine ].bottom_border    +
	 base_timings[ machine ].vertical_retrace;
}

libspectrum_dword
libspectrum_timings_tstates_per_frame( libspectrum_machine machine )
{
  return libspectrum_timings_tstates_per_line( machine ) *
    ( (libspectrum_dword)libspectrum_timings_lines_per_frame( machine ) );
}
