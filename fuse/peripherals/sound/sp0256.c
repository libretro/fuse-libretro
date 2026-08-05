/* sp0256.c: SP0256 speech synthesis emulation
   Copyright (c) 1998-2000 Jospeh Zbicaik, 2007-2023 Stuart Brady
   Copyright (c) 2023 Sergio Baldoví, Vic Chwe

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

/* Based on the SP0256 emulation in jzIntv (an Intellivision emulator),
 * where it is used for Intellivoice emulation.
 *
 * For information about the SP0256:
 *  * http://en.wikipedia.org/wiki/General_Instrument_SP0256-AL2
 *  * http://www.cpcwiki.eu/index.php/SP0256
 */

#include "config.h"

#include <stdio.h>

#include "libspectrum.h"

/* from jzintv-config.h */
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "compat.h"
#include "event.h"
#include "machine.h"
#include "sp0256.h"
#include "sound.h"

/*
 *  SP0256-based peripherals for the ZX Spectrum seem limited to using the
 *  interface to the "Address LoaD" (ALD) mechanism on the SP0256, and the
 *  "Load ReQuest" (LRQ) state in bit 15.  When LRQ is 1, the SP0256 is
 *  ready to receive a new command.  A new command address may then be
 *  written to using the Address LoaD mechanism to trigger the playback of
 *  a sound.  Note that command address register is actually a 1-deep FIFO,
 *  and so LRQ will go to 1 before the SP0256 is finished speaking.
 *
 *  The exact format of the SP0256 speech data, as well as the overall
 *  system view from the SP0256's perspective is documented elsewhere.
 */

#undef DEBUG_AL2_ALLOPHONE
#undef DEBUG_SP0256
#undef DEBUG_FIFO

#ifdef DEBUG_SP0256
#define debug_printf(x) printf x ; fflush(stdout)
#else
#define debug_printf(x)
#endif /* #ifdef DEBUG_SP0256 */

#define HIGH_QUALITY
#define SCBUF_SIZE   ( 4096 )             /* Must be power of 2 */
#define SCBUF_MASK   ( SCBUF_SIZE - 1 )
#define PER_PAUSE    ( 64 )               /* Equiv timing period for pauses. */
#define PER_NOISE    ( 64 )               /* Equiv timing period for noise. */

#define FIFO_ADDR    ( 0x1800 << 3 )      /* SP0256 address of speech FIFO. */

typedef struct lpc12_t {
  int rpt, cnt;                 /* Repeat counter, Period down-counter. */
  libspectrum_dword per, rng;   /* Period, Amplitude, Random Number Generator */
  int amp;
  libspectrum_signed_word f_coef[6];    /* F0 through F5. */
  libspectrum_signed_word b_coef[6];    /* B0 through B5. */
  libspectrum_signed_word z_data[6][2]; /* Time-delay data for the */
                                        /* filter stages. */
  libspectrum_byte r[16];               /* The encoded register set. */
  int interp;
} lpc12_t;


typedef struct sp0256_t {
  int silent;                       /* Flag: SP0256 is silent. */

  libspectrum_signed_word *scratch; /* Scratch buffer for audio. */
  libspectrum_dword sc_head;        /* Head/Tail pointer into scratch circular buf */
  libspectrum_dword sc_tail;        /* Head/Tail pointer into scratch circular buf */
  libspectrum_signed_dword sound_current;

  lpc12_t filt;           /* 12-pole filter */
  int lrq;                /* Load ReQuest.  == 0 if we can accept a load */
  int ald;                /* Address LoaD.  < 0 if no command pending. */
  int pc;                 /* Microcontroller's PC value. */
  int stack;              /* Microcontroller's PC stack. */
  int fifo_sel;           /* True when executing from FIFO. */
  int halted;             /* True when CPU is halted. */
  libspectrum_dword mode;          /* Mode register. */
  libspectrum_dword page;          /* Page set by SETPAGE */

  libspectrum_dword fifo_head;  /* FIFO head pointer (where new data goes). */
  libspectrum_dword fifo_tail;  /* FIFO tail pointer (where data comes from). */
  libspectrum_dword fifo_bitp;  /* FIFO bit-pointer (for partial decles). */
  libspectrum_word fifo[64];    /* The 64-decle FIFO. */

  const libspectrum_byte *rom[16]; /* 4K ROM pages. */

  libspectrum_dword clock;         /* Clock - crystal frequency */
  unsigned int clock_per_samp;     /* Sample duration in Z80 tstates */
} sp0256_t;


/* ======================================================================== */
/*  sp0256_run   -- Where the magic happens.  Generate voice data for       */
/*                  our good friend, the SP0256.                            */
/* ======================================================================== */
static libspectrum_dword sp0256_run( sp0256_t *s, libspectrum_dword len );

/* ======================================================================== */
/*  sp0256_read_lrq -- Handle reads of the SP0256 LRQ line.                 */
/* ======================================================================== */
static libspectrum_dword sp0256_read_lrq( sp0256_t *s );

/* ======================================================================== */
/*  sp0256_write_ald -- Handle writes to the SP0256's Address Load FIFO.    */
/* ======================================================================== */
static void sp0256_write_ald( sp0256_t *s, libspectrum_dword data );

/* ======================================================================== */
/*  sp0256_do_init -- Makes a new SP0256.                                   */
/* ======================================================================== */
static int sp0256_do_init( sp0256_t *s, libspectrum_byte *sp0256rom );

/* ======================================================================== */
/*  Internal function prototypes.                                           */
/* ======================================================================== */
static inline libspectrum_signed_word limit( libspectrum_signed_word s );
static inline libspectrum_dword bitrev( libspectrum_dword val );
static int lpc12_update( lpc12_t *f, int, libspectrum_signed_word *,
                         libspectrum_dword * );
static void lpc12_regdec( lpc12_t *f );
static libspectrum_dword sp0256_getb( sp0256_t *s, int len );
static void sp0256_micro( sp0256_t *s );

/* ======================================================================== */
/*  qtbl  -- Coefficient Quantization Table.  This comes from a             */
/*           SP0250 data sheet, and should be correct for SP0256.           */
/* ======================================================================== */
static const libspectrum_signed_word qtbl[128] = {
  0,      9,      17,     25,     33,     41,     49,     57,
  65,     73,     81,     89,     97,     105,    113,    121,
  129,    137,    145,    153,    161,    169,    177,    185,
  193,    201,    209,    217,    225,    233,    241,    249,
  257,    265,    273,    281,    289,    297,    301,    305,
  309,    313,    317,    321,    325,    329,    333,    337,
  341,    345,    349,    353,    357,    361,    365,    369,
  373,    377,    381,    385,    389,    393,    397,    401,
  405,    409,    413,    417,    421,    425,    427,    429,
  431,    433,    435,    437,    439,    441,    443,    445,
  447,    449,    451,    453,    455,    457,    459,    461,
  463,    465,    467,    469,    471,    473,    475,    477,
  479,    481,    482,    483,    484,    485,    486,    487,
  488,    489,    490,    491,    492,    493,    494,    495,
  496,    497,    498,    499,    500,    501,    502,    503,
  504,    505,    506,    507,    508,    509,    510,    511
};

/* ======================================================================== */
/*  limit            -- Limiter function for digital sample output.         */
/* ======================================================================== */
static inline libspectrum_signed_word
limit( libspectrum_signed_word s )
{
#ifdef HIGH_QUALITY /* Higher quality than the original, but who cares? */
  if( s >  8191 ) return 8191;
  if( s < -8192 ) return -8192;
#else
  if( s >  127 ) return 127;
  if( s < -128 ) return -128;
#endif
  return s;
}

/* ======================================================================== */
/*  amp_decode       -- Decode amplitude register                           */
/* ======================================================================== */
static int
amp_decode( libspectrum_byte a )
{
  /* -------------------------------------------------------------------- */
  /*  Amplitude has 3 bits of exponent and 5 bits of mantissa.  This      */
  /*  contradicts USP 4,296,269 but matches the SP0250 Apps Manual.       */
  /* -------------------------------------------------------------------- */
  int expn = ( a & 0xe0 ) >> 5;
  int mant = ( a & 0x1f );
  int ampl = mant << expn;

  /* -------------------------------------------------------------------- */
  /*  Careful reading of USP 4,296,279, around line 60 in column 14 on    */
  /*  page 16 of the scan suggests the LSB might be held and injected     */
  /*  into the output while the exponent gets counted down, although      */
  /*  this seems dubious.                                                 */
  /* -------------------------------------------------------------------- */
#if 0
  if( mant & 1 )
    ampl |= ( 1u << expn ) - 1;
#endif

  return ampl;
}

/* ======================================================================== */
/*  lpc12_update     -- Update the 12-pole filter, outputting samples.      */
/* ======================================================================== */
static int
lpc12_update( lpc12_t *f, int num_samp, libspectrum_signed_word *out,
              libspectrum_dword *optr )
{
  int i, j;
  libspectrum_signed_word samp;
  int do_int, bit;
  int oidx = *optr;

  /* -------------------------------------------------------------------- */
  /*  Iterate up to the desired number of samples.  We actually may       */
  /*  break out early if our repeat count expires.                        */
  /* -------------------------------------------------------------------- */
  for( i = 0; i < num_samp; i++ ) {
    /* ---------------------------------------------------------------- */
    /*  Generate a series of periodic impulses, or random noise.        */
    /* ---------------------------------------------------------------- */
    do_int = 0;
    samp   = 0;
    bit    = f->rng & 1;
    f->rng = ( f->rng >> 1 ) ^ ( bit ? 0x4001 : 0 );

    if( --f->cnt <= 0 ) {
      if( f->rpt-- <= 0 ) {    /* Stop if we expire the repeat counter */
        f->cnt = f->rpt = 0;
        break;
      }

      f->cnt = f->per ? f->per : PER_NOISE;
      samp   = f->amp;
      do_int = f->interp;
    }

    if( !f->per )
      samp   = bit ? -f->amp : f->amp;

    /* ---------------------------------------------------------------- */
    /*  If we need to, process the interpolation registers.             */
    /* ---------------------------------------------------------------- */
    if( do_int ) {
      f->r[0] += f->r[14];
      f->r[1] += f->r[15];

      f->amp   = amp_decode( f->r[0] );
      f->per   = f->r[1];

      do_int   = 0;
    }

    /* ---------------------------------------------------------------- */
    /*  Each 2nd order stage looks like one of these.  The App. Manual  */
    /*  gives the first form, the patent gives the second form.         */
    /*  They're equivalent except for time delay.  I implement the      */
    /*  first form.   (Note: 1/Z == 1 unit of time delay.)              */
    /*                                                                  */
    /*          ---->(+)-------->(+)----------+------->                 */
    /*                ^           ^           |                         */
    /*                |           |           |                         */
    /*                |           |           |                         */
    /*               [B]        [2*F]         |                         */
    /*                ^           ^           |                         */
    /*                |           |           |                         */
    /*                |           |           |                         */
    /*                +---[1/Z]<--+---[1/Z]<--+                         */
    /*                                                                  */
    /*                                                                  */
    /*                +---[2*F]<---+                                    */
    /*                |            |                                    */
    /*                |            |                                    */
    /*                v            |                                    */
    /*          ---->(+)-->[1/Z]-->+-->[1/Z]---+------>                 */
    /*                ^                        |                        */
    /*                |                        |                        */
    /*                |                        |                        */
    /*                +-----------[B]<---------+                        */
    /*                                                                  */
    /* ---------------------------------------------------------------- */
    for( j = 0; j < 6; j++ ) {
      samp += ( ( (int)f->b_coef[j] * (int)f->z_data[j][1] ) >> 9 );
      samp += ( ( (int)f->f_coef[j] * (int)f->z_data[j][0] ) >> 8 );

      f->z_data[j][1] = f->z_data[j][0];
      f->z_data[j][0] = samp;
    }

    /* Sergio: low quality samples sound louder as can be scaled closer to
       the limit of a signed 16 bits sample */

#ifdef HIGH_QUALITY /* Higher quality than the original, but who cares? */
    out[oidx++ & SCBUF_MASK] = limit( samp ) * 4;
#else
    out[oidx++ & SCBUF_MASK] = ( limit( samp >> 4 ) * 256 );
#endif
  }

  *optr = oidx;

  return i;
}

/*static int stage_map[6] = { 4, 2, 0, 5, 3, 1 };*/
/*static int stage_map[6] = { 3, 0, 4, 1, 5, 2 };*/
/*static int stage_map[6] = { 3, 0, 1, 4, 2, 5 };*/
static const int stage_map[6] = { 0, 1, 2, 3, 4, 5 };

/* ======================================================================== */
/*  lpc12_regdec -- Decode the register set in the filter bank.             */
/* ======================================================================== */
static void
lpc12_regdec( lpc12_t *f )
{
  int i;

  /* -------------------------------------------------------------------- */
  /*  Decode the Amplitude and Period registers.  Force cnt to 0 to get   */
  /*  the initial impulse.  (Redundant?)                                  */
  /* -------------------------------------------------------------------- */
  f->amp = amp_decode( f->r[0] );
  f->cnt = 0;
  f->per = f->r[1];

  /* -------------------------------------------------------------------- */
  /*  Decode the filter coefficients from the quant table.                */
  /* -------------------------------------------------------------------- */
  for( i = 0; i < 6; i++ ) {
    #define IQ( x ) ( ( ( x ) & 0x80 ) ? qtbl[0x7f & -( x )] : -qtbl[( x )] )

    f->b_coef[stage_map[i]] = IQ( f->r[2 + 2 * i] );
    f->f_coef[stage_map[i]] = IQ( f->r[3 + 2 * i] );
  }

  /* -------------------------------------------------------------------- */
  /*  Set the Interp flag based on whether we have interpolation parms    */
  /* -------------------------------------------------------------------- */
  f->interp = f->r[14] || f->r[15];

  return;
}

/* ======================================================================== */
/*  sp0256_datafmt   -- Data format table for the SP0256's microsequencer   */
/*                                                                          */
/*  len     4 bits      Length of field to extract                          */
/*  lshift  4 bits      Left-shift amount on field                          */
/*  param   4 bits      Parameter number being updated                      */
/*  delta   1 bit       This is a delta-update.  (Implies sign-extend)      */
/*  field   1 bit       This is a field replace.                            */
/*  clr5    1 bit       Clear F5, B5.                                       */
/*  clrall  1 bit       Clear all before doing this update                  */
/* ======================================================================== */

#define CR(l,s,p,d,f,c5,ca) \
  (                         \
    (((l)  & 15) <<  0) |   \
    (((s)  & 15) <<  4) |   \
    (((p)  & 15) <<  8) |   \
    (((d)  &  1) << 12) |   \
    (((f)  &  1) << 13) |   \
    (((c5) &  1) << 14) |   \
    (((ca) &  1) << 15)     \
  )

#define CR_DELTA  CR(0,0,0,1,0,0,0)
#define CR_FIELD  CR(0,0,0,0,1,0,0)
#define CR_CLR5   CR(0,0,0,0,0,1,0)
#define CR_CLRL   CR(0,0,0,0,0,0,1)
#define CR_LEN(x) ((x) & 15)
#define CR_SHF(x) (((x) >> 4) & 15)
#define CR_PRM(x) (((x) >> 8) & 15)

enum { AM = 0, PR, B0, F0, B1, F1, B2, F2, B3, F3, B4, F4, B5, F5, IA, IP };

static const libspectrum_word sp0256_datafmt[] = {
  /* -------------------------------------------------------------------- */
  /*  OPCODE 1111: PAUSE                                                  */
  /* -------------------------------------------------------------------- */
  /*    0 */  CR( 0,  0,  0,  0,  0,  0,  0),     /*  Clear all   */

  /* -------------------------------------------------------------------- */
  /*  Opcode 0001: LOADALL                                                */
  /* -------------------------------------------------------------------- */
        /* Mode modes x1    */
  /*    1 */  CR( 8,  0,  AM, 0,  0,  0,  0),     /*  Amplitude   */
  /*    2 */  CR( 8,  0,  PR, 0,  0,  0,  0),     /*  Period      */
  /*    3 */  CR( 8,  0,  B0, 0,  0,  0,  0),     /*  B0          */
  /*    4 */  CR( 8,  0,  F0, 0,  0,  0,  0),     /*  F0          */
  /*    5 */  CR( 8,  0,  B1, 0,  0,  0,  0),     /*  B1          */
  /*    6 */  CR( 8,  0,  F1, 0,  0,  0,  0),     /*  F1          */
  /*    7 */  CR( 8,  0,  B2, 0,  0,  0,  0),     /*  B2          */
  /*    8 */  CR( 8,  0,  F2, 0,  0,  0,  0),     /*  F2          */
  /*    9 */  CR( 8,  0,  B3, 0,  0,  0,  0),     /*  B3          */
  /*   10 */  CR( 8,  0,  F3, 0,  0,  0,  0),     /*  F3          */
  /*   11 */  CR( 8,  0,  B4, 0,  0,  0,  0),     /*  B4          */
  /*   12 */  CR( 8,  0,  F4, 0,  0,  0,  0),     /*  F4          */
  /*   13 */  CR( 8,  0,  B5, 0,  0,  0,  0),     /*  B5          */
  /*   14 */  CR( 8,  0,  F5, 0,  0,  0,  0),     /*  F5          */
  /*   15 */  CR( 8,  0,  IA, 0,  0,  0,  0),     /*  Amp Interp  */
  /*   16 */  CR( 8,  0,  IP, 0,  0,  0,  0),     /*  Pit Interp  */

  /* -------------------------------------------------------------------- */
  /*  Opcode 0100: LOAD_4                                                 */
  /* -------------------------------------------------------------------- */
        /* Mode 00 and 01           */
  /*   17 */  CR( 6,  2,  AM, 0,  0,  0,  1),     /*  Amplitude   */
  /*   18 */  CR( 8,  0,  PR, 0,  0,  0,  0),     /*  Period      */
  /*   19 */  CR( 4,  3,  B3, 0,  0,  0,  0),     /*  B3 (S=0)    */
  /*   20 */  CR( 6,  2,  F3, 0,  0,  0,  0),     /*  F3          */
  /*   21 */  CR( 7,  1,  B4, 0,  0,  0,  0),     /*  B4          */
  /*   22 */  CR( 6,  2,  F4, 0,  0,  0,  0),     /*  F4          */
        /* Mode 01 only             */
  /*   23 */  CR( 8,  0,  B5, 0,  0,  0,  0),     /*  B5          */
  /*   24 */  CR( 8,  0,  F5, 0,  0,  0,  0),     /*  F5          */

        /* Mode 10 and 11           */
  /*   25 */  CR( 6,  2,  AM, 0,  0,  0,  1),     /*  Amplitude   */
  /*   26 */  CR( 8,  0,  PR, 0,  0,  0,  0),     /*  Period      */
  /*   27 */  CR( 6,  1,  B3, 0,  0,  0,  0),     /*  B3 (S=0)    */
  /*   28 */  CR( 7,  1,  F3, 0,  0,  0,  0),     /*  F3          */
  /*   29 */  CR( 8,  0,  B4, 0,  0,  0,  0),     /*  B4          */
  /*   30 */  CR( 8,  0,  F4, 0,  0,  0,  0),     /*  F4          */
        /* Mode 11 only             */
  /*   31 */  CR( 8,  0,  B5, 0,  0,  0,  0),     /*  B5          */
  /*   32 */  CR( 8,  0,  F5, 0,  0,  0,  0),     /*  F5          */

  /* -------------------------------------------------------------------- */
  /*  Opcode 0110: SETMSB_6                                               */
  /* -------------------------------------------------------------------- */
        /* Mode 00 only             */
  /*   33 */  CR( 0,  0,  0,  0,  0,  0,  0),
        /* Mode 00 and 01           */
  /*   34 */  CR( 6,  2,  AM, 0,  0,  0,  0),     /*  Amplitude   */
  /*   35 */  CR( 6,  2,  F3, 0,  1,  0,  0),     /*  F3 (5 MSBs) */
  /*   36 */  CR( 6,  2,  F4, 0,  1,  0,  0),     /*  F4 (5 MSBs) */
        /* Mode 01 only             */
  /*   37 */  CR( 8,  0,  F5, 0,  1,  0,  0),     /*  F5 (5 MSBs) */

        /* Mode 10 only             */
  /*   38 */  CR( 0,  0,  0,  0,  0,  0,  0),
        /* Mode 10 and 11           */
  /*   39 */  CR( 6,  2,  AM, 0,  0,  0,  0),     /*  Amplitude   */
  /*   40 */  CR( 7,  1,  F3, 0,  1,  0,  0),     /*  F3 (6 MSBs) */
  /*   41 */  CR( 8,  0,  F4, 0,  1,  0,  0),     /*  F4 (6 MSBs) */
        /* Mode 11 only             */
  /*   42 */  CR( 8,  0,  F5, 0,  1,  0,  0),     /*  F5 (6 MSBs) */

  /*   43 */  0,
  /*   44 */  0,

  /* -------------------------------------------------------------------- */
  /*  Opcode 1001: DELTA_9                                                */
  /* -------------------------------------------------------------------- */
        /* Mode 00 and 01           */
  /*   45 */  CR( 4,  2,  AM, 1,  0,  0,  0),     /*  Amplitude   */
  /*   46 */  CR( 5,  0,  PR, 1,  0,  0,  0),     /*  Period      */
  /*   47 */  CR( 3,  4,  B0, 1,  0,  0,  0),     /*  B0 4 MSBs   */
  /*   48 */  CR( 3,  3,  F0, 1,  0,  0,  0),     /*  F0 5 MSBs   */
  /*   49 */  CR( 3,  4,  B1, 1,  0,  0,  0),     /*  B1 4 MSBs   */
  /*   50 */  CR( 3,  3,  F1, 1,  0,  0,  0),     /*  F1 5 MSBs   */
  /*   51 */  CR( 3,  4,  B2, 1,  0,  0,  0),     /*  B2 4 MSBs   */
  /*   52 */  CR( 3,  3,  F2, 1,  0,  0,  0),     /*  F2 5 MSBs   */
  /*   53 */  CR( 3,  3,  B3, 1,  0,  0,  0),     /*  B3 5 MSBs   */
  /*   54 */  CR( 4,  2,  F3, 1,  0,  0,  0),     /*  F3 6 MSBs   */
  /*   55 */  CR( 4,  1,  B4, 1,  0,  0,  0),     /*  B4 7 MSBs   */
  /*   56 */  CR( 4,  2,  F4, 1,  0,  0,  0),     /*  F4 6 MSBs   */
        /* Mode 01 only             */
  /*   57 */  CR( 5,  0,  B5, 1,  0,  0,  0),     /*  B5 8 MSBs   */
  /*   58 */  CR( 5,  0,  F5, 1,  0,  0,  0),     /*  F5 8 MSBs   */

        /* Mode 10 and 11           */
  /*   59 */  CR( 4,  2,  AM, 1,  0,  0,  0),     /*  Amplitude   */
  /*   60 */  CR( 5,  0,  PR, 1,  0,  0,  0),     /*  Period      */
  /*   61 */  CR( 4,  1,  B0, 1,  0,  0,  0),     /*  B0 7 MSBs   */
  /*   62 */  CR( 4,  2,  F0, 1,  0,  0,  0),     /*  F0 6 MSBs   */
  /*   63 */  CR( 4,  1,  B1, 1,  0,  0,  0),     /*  B1 7 MSBs   */
  /*   64 */  CR( 4,  2,  F1, 1,  0,  0,  0),     /*  F1 6 MSBs   */
  /*   65 */  CR( 4,  1,  B2, 1,  0,  0,  0),     /*  B2 7 MSBs   */
  /*   66 */  CR( 4,  2,  F2, 1,  0,  0,  0),     /*  F2 6 MSBs   */
  /*   67 */  CR( 4,  1,  B3, 1,  0,  0,  0),     /*  B3 7 MSBs   */
  /*   68 */  CR( 5,  1,  F3, 1,  0,  0,  0),     /*  F3 7 MSBs   */
  /*   69 */  CR( 5,  0,  B4, 1,  0,  0,  0),     /*  B4 8 MSBs   */
  /*   70 */  CR( 5,  0,  F4, 1,  0,  0,  0),     /*  F4 8 MSBs   */
        /* Mode 11 only             */
  /*   71 */  CR( 5,  0,  B5, 1,  0,  0,  0),     /*  B5 8 MSBs   */
  /*   72 */  CR( 5,  0,  F5, 1,  0,  0,  0),     /*  F5 8 MSBs   */

  /* -------------------------------------------------------------------- */
  /*  Opcode 1010: SETMSB_A                                               */
  /* -------------------------------------------------------------------- */
        /* Mode 00 only             */
  /*   73 */  CR( 0,  0,  0,  0,  0,  0,  0),
        /* Mode 00 and 01           */
  /*   74 */  CR( 6,  2,  AM, 0,  0,  0,  0),     /*  Amplitude   */
  /*   75 */  CR( 5,  3,  F0, 0,  1,  0,  0),     /*  F0 (5 MSBs) */
  /*   76 */  CR( 5,  3,  F1, 0,  1,  0,  0),     /*  F1 (5 MSBs) */
  /*   77 */  CR( 5,  3,  F2, 0,  1,  0,  0),     /*  F2 (5 MSBs) */

        /* Mode 10 only             */
  /*   78 */  CR( 0,  0,  0,  0,  0,  0,  0),
        /* Mode 10 and 11           */
  /*   79 */  CR( 6,  2,  AM, 0,  0,  0,  0),     /*  Amplitude   */
  /*   80 */  CR( 6,  2,  F0, 0,  1,  0,  0),     /*  F0 (6 MSBs) */
  /*   81 */  CR( 6,  2,  F1, 0,  1,  0,  0),     /*  F1 (6 MSBs) */
  /*   82 */  CR( 6,  2,  F2, 0,  1,  0,  0),     /*  F2 (6 MSBs) */

  /* -------------------------------------------------------------------- */
  /*  Opcode 0010: LOAD_2  Mode 00 and 10                                 */
  /*  Opcode 1100: LOAD_C  Mode 00 and 10                                 */
  /* -------------------------------------------------------------------- */
        /* LOAD_2, LOAD_C  Mode 00  */
  /*   83 */  CR( 6,  2,  AM, 0,  0,  0,  0),     /*  Amplitude   */
  /*   84 */  CR( 8,  0,  PR, 0,  0,  0,  0),     /*  Period      */
  /*   85 */  CR( 3,  4,  B0, 0,  0,  0,  0),     /*  B0 (S=0)    */
  /*   86 */  CR( 5,  3,  F0, 0,  0,  0,  0),     /*  F0          */
  /*   87 */  CR( 3,  4,  B1, 0,  0,  0,  0),     /*  B1 (S=0)    */
  /*   88 */  CR( 5,  3,  F1, 0,  0,  0,  0),     /*  F1          */
  /*   89 */  CR( 3,  4,  B2, 0,  0,  0,  0),     /*  B2 (S=0)    */
  /*   90 */  CR( 5,  3,  F2, 0,  0,  0,  0),     /*  F2          */
  /*   91 */  CR( 4,  3,  B3, 0,  0,  0,  0),     /*  B3 (S=0)    */
  /*   92 */  CR( 6,  2,  F3, 0,  0,  0,  0),     /*  F3          */
  /*   93 */  CR( 7,  1,  B4, 0,  0,  0,  0),     /*  B4          */
  /*   94 */  CR( 6,  2,  F4, 0,  0,  0,  0),     /*  F4          */
        /* LOAD_2 only              */
  /*   95 */  CR( 5,  0,  IA, 0,  0,  0,  0),     /*  Ampl. Intr. */
  /*   96 */  CR( 5,  0,  IP, 0,  0,  0,  0),     /*  Per. Intr.  */

        /* LOAD_2, LOAD_C  Mode 10  */
  /*   97 */  CR( 6,  2,  AM, 0,  0,  0,  0),     /*  Amplitude   */
  /*   98 */  CR( 8,  0,  PR, 0,  0,  0,  0),     /*  Period      */
  /*   99 */  CR( 6,  1,  B0, 0,  0,  0,  0),     /*  B0 (S=0)    */
  /*  100 */  CR( 6,  2,  F0, 0,  0,  0,  0),     /*  F0          */
  /*  101 */  CR( 6,  1,  B1, 0,  0,  0,  0),     /*  B1 (S=0)    */
  /*  102 */  CR( 6,  2,  F1, 0,  0,  0,  0),     /*  F1          */
  /*  103 */  CR( 6,  1,  B2, 0,  0,  0,  0),     /*  B2 (S=0)    */
  /*  104 */  CR( 6,  2,  F2, 0,  0,  0,  0),     /*  F2          */
  /*  105 */  CR( 6,  1,  B3, 0,  0,  0,  0),     /*  B3 (S=0)    */
  /*  106 */  CR( 7,  1,  F3, 0,  0,  0,  0),     /*  F3          */
  /*  107 */  CR( 8,  0,  B4, 0,  0,  0,  0),     /*  B4          */
  /*  108 */  CR( 8,  0,  F4, 0,  0,  0,  0),     /*  F4          */
        /* LOAD_2 only              */
  /*  109 */  CR( 5,  0,  IA, 0,  0,  0,  0),     /*  Ampl. Intr. */
  /*  110 */  CR( 5,  0,  IP, 0,  0,  0,  0),     /*  Per. Intr.  */

  /* -------------------------------------------------------------------- */
  /*  OPCODE 1101: DELTA_D                                                */
  /* -------------------------------------------------------------------- */
        /* Mode 00 and 01           */
  /*  111 */  CR( 4,  2,  AM, 1,  0,  0,  1),     /*  Amplitude   */
  /*  112 */  CR( 5,  0,  PR, 1,  0,  0,  0),     /*  Period      */
  /*  113 */  CR( 3,  3,  B3, 1,  0,  0,  0),     /*  B3 5 MSBs   */
  /*  114 */  CR( 4,  2,  F3, 1,  0,  0,  0),     /*  F3 6 MSBs   */
  /*  115 */  CR( 4,  1,  B4, 1,  0,  0,  0),     /*  B4 7 MSBs   */
  /*  116 */  CR( 4,  2,  F4, 1,  0,  0,  0),     /*  F4 6 MSBs   */
        /* Mode 01 only             */
  /*  117 */  CR( 5,  0,  B5, 1,  0,  0,  0),     /*  B5 8 MSBs   */
  /*  118 */  CR( 5,  0,  F5, 1,  0,  0,  0),     /*  F5 8 MSBs   */

        /* Mode 10 and 11           */
  /*  119 */  CR( 4,  2,  AM, 1,  0,  0,  0),     /*  Amplitude   */
  /*  120 */  CR( 5,  0,  PR, 1,  0,  0,  0),     /*  Period      */
  /*  121 */  CR( 4,  1,  B3, 1,  0,  0,  0),     /*  B3 7 MSBs   */
  /*  122 */  CR( 5,  1,  F3, 1,  0,  0,  0),     /*  F3 7 MSBs   */
  /*  123 */  CR( 5,  0,  B4, 1,  0,  0,  0),     /*  B4 8 MSBs   */
  /*  124 */  CR( 5,  0,  F4, 1,  0,  0,  0),     /*  F4 8 MSBs   */
        /* Mode 11 only             */
  /*  125 */  CR( 5,  0,  B5, 1,  0,  0,  0),     /*  B5 8 MSBs   */
  /*  126 */  CR( 5,  0,  F5, 1,  0,  0,  0),     /*  F5 8 MSBs   */

  /* -------------------------------------------------------------------- */
  /*  OPCODE 1110: LOAD_E                                                 */
  /* -------------------------------------------------------------------- */
  /*  127 */  CR( 6,  2,  AM, 0,  0,  0,  0),     /*  Amplitude   */
  /*  128 */  CR( 8,  0,  PR, 0,  0,  0,  0),     /*  Period      */

  /* -------------------------------------------------------------------- */
  /*  Opcode 0010: LOAD_2  Mode 01 and 11                                 */
  /*  Opcode 1100: LOAD_C  Mode 01 and 11                                 */
  /* -------------------------------------------------------------------- */
        /* LOAD_2, LOAD_C  Mode 01  */
  /*  129 */  CR( 6,  2,  AM, 0,  0,  0,  0),     /*  Amplitude   */
  /*  130 */  CR( 8,  0,  PR, 0,  0,  0,  0),     /*  Period      */
  /*  131 */  CR( 3,  4,  B0, 0,  0,  0,  0),     /*  B0 (S=0)    */
  /*  132 */  CR( 5,  3,  F0, 0,  0,  0,  0),     /*  F0          */
  /*  133 */  CR( 3,  4,  B1, 0,  0,  0,  0),     /*  B1 (S=0)    */
  /*  134 */  CR( 5,  3,  F1, 0,  0,  0,  0),     /*  F1          */
  /*  135 */  CR( 3,  4,  B2, 0,  0,  0,  0),     /*  B2 (S=0)    */
  /*  136 */  CR( 5,  3,  F2, 0,  0,  0,  0),     /*  F2          */
  /*  137 */  CR( 4,  3,  B3, 0,  0,  0,  0),     /*  B3 (S=0)    */
  /*  138 */  CR( 6,  2,  F3, 0,  0,  0,  0),     /*  F3          */
  /*  139 */  CR( 7,  1,  B4, 0,  0,  0,  0),     /*  B4          */
  /*  140 */  CR( 6,  2,  F4, 0,  0,  0,  0),     /*  F4          */
  /*  141 */  CR( 8,  0,  B5, 0,  0,  0,  0),     /*  B5          */
  /*  142 */  CR( 8,  0,  F5, 0,  0,  0,  0),     /*  F5          */
        /* LOAD_2 only              */
  /*  143 */  CR( 5,  0,  IA, 0,  0,  0,  0),     /*  Ampl. Intr. */
  /*  144 */  CR( 5,  0,  IP, 0,  0,  0,  0),     /*  Per. Intr.  */

        /* LOAD_2, LOAD_C  Mode 11  */
  /*  145 */  CR( 6,  2,  AM, 0,  0,  0,  0),     /*  Amplitude   */
  /*  146 */  CR( 8,  0,  PR, 0,  0,  0,  0),     /*  Period      */
  /*  147 */  CR( 6,  1,  B0, 0,  0,  0,  0),     /*  B0 (S=0)    */
  /*  148 */  CR( 6,  2,  F0, 0,  0,  0,  0),     /*  F0          */
  /*  149 */  CR( 6,  1,  B1, 0,  0,  0,  0),     /*  B1 (S=0)    */
  /*  150 */  CR( 6,  2,  F1, 0,  0,  0,  0),     /*  F1          */
  /*  151 */  CR( 6,  1,  B2, 0,  0,  0,  0),     /*  B2 (S=0)    */
  /*  152 */  CR( 6,  2,  F2, 0,  0,  0,  0),     /*  F2          */
  /*  153 */  CR( 6,  1,  B3, 0,  0,  0,  0),     /*  B3 (S=0)    */
  /*  154 */  CR( 7,  1,  F3, 0,  0,  0,  0),     /*  F3          */
  /*  155 */  CR( 8,  0,  B4, 0,  0,  0,  0),     /*  B4          */
  /*  156 */  CR( 8,  0,  F4, 0,  0,  0,  0),     /*  F4          */
  /*  157 */  CR( 8,  0,  B5, 0,  0,  0,  0),     /*  B5          */
  /*  158 */  CR( 8,  0,  F5, 0,  0,  0,  0),     /*  F5          */
        /* LOAD_2 only              */
  /*  159 */  CR( 5,  0,  IA, 0,  0,  0,  0),     /*  Ampl. Intr. */
  /*  160 */  CR( 5,  0,  IP, 0,  0,  0,  0),     /*  Per. Intr.  */

  /* -------------------------------------------------------------------- */
  /*  Opcode 0011: SETMSB_3                                               */
  /*  Opcode 0101: SETMSB_5                                               */
  /* -------------------------------------------------------------------- */
        /* Mode 00 only             */
  /*  161 */  CR( 0,  0,  0,  0,  0,  0,  0),
        /* Mode 00 and 01           */
  /*  162 */  CR( 6,  2,  AM, 0,  0,  0,  0),     /*  Amplitude   */
  /*  163 */  CR( 8,  0,  PR, 0,  0,  0,  0),     /*  Period      */
  /*  164 */  CR( 5,  3,  F0, 0,  1,  0,  0),     /*  F0 (5 MSBs) */
  /*  165 */  CR( 5,  3,  F1, 0,  1,  0,  0),     /*  F1 (5 MSBs) */
  /*  166 */  CR( 5,  3,  F2, 0,  1,  0,  0),     /*  F2 (5 MSBs) */
        /* SETMSB_3 only            */
  /*  167 */  CR( 5,  0,  IA, 0,  0,  0,  0),     /*  Ampl. Intr. */
  /*  168 */  CR( 5,  0,  IP, 0,  0,  0,  0),     /*  Per. Intr.  */

        /* Mode 10 only             */
  /*  169 */  CR( 0,  0,  0,  0,  0,  0,  0),
        /* Mode 10 and 11           */
  /*  170 */  CR( 6,  2,  AM, 0,  0,  0,  0),     /*  Amplitude   */
  /*  171 */  CR( 8,  0,  PR, 0,  0,  0,  0),     /*  Period      */
  /*  172 */  CR( 6,  2,  F0, 0,  1,  0,  0),     /*  F0 (6 MSBs) */
  /*  173 */  CR( 6,  2,  F1, 0,  1,  0,  0),     /*  F1 (6 MSBs) */
  /*  174 */  CR( 6,  2,  F2, 0,  1,  0,  0),     /*  F2 (6 MSBs) */
        /* SETMSB_3 only            */
  /*  175 */  CR( 5,  0,  IA, 0,  0,  0,  0),     /*  Ampl. Intr. */
  /*  176 */  CR( 5,  0,  IP, 0,  0,  0,  0),     /*  Per. Intr.  */

  /* -------------------------------------------------------------------- */
  /*  Opcode 0001: LOADALL                                                */
  /* -------------------------------------------------------------------- */
        /* Mode x0    */
  /*  177 */  CR( 8,  0,  AM, 0,  0,  0,  0),     /*  Amplitude   */
  /*  178 */  CR( 8,  0,  PR, 0,  0,  0,  0),     /*  Period      */
  /*  179 */  CR( 8,  0,  B0, 0,  0,  0,  0),     /*  B0          */
  /*  180 */  CR( 8,  0,  F0, 0,  0,  0,  0),     /*  F0          */
  /*  181 */  CR( 8,  0,  B1, 0,  0,  0,  0),     /*  B1          */
  /*  182 */  CR( 8,  0,  F1, 0,  0,  0,  0),     /*  F1          */
  /*  183 */  CR( 8,  0,  B2, 0,  0,  0,  0),     /*  B2          */
  /*  184 */  CR( 8,  0,  F2, 0,  0,  0,  0),     /*  F2          */
  /*  185 */  CR( 8,  0,  B3, 0,  0,  0,  0),     /*  B3          */
  /*  186 */  CR( 8,  0,  F3, 0,  0,  0,  0),     /*  F3          */
  /*  187 */  CR( 8,  0,  B4, 0,  0,  0,  0),     /*  B4          */
  /*  188 */  CR( 8,  0,  F4, 0,  0,  0,  0),     /*  F4          */
  /*  189 */  CR( 8,  0,  IA, 0,  0,  0,  0),     /*  Amp Interp  */
  /*  190 */  CR( 8,  0,  IP, 0,  0,  0,  0),     /*  Pit Interp  */
};

static const libspectrum_signed_word sp0256_df_idx[16 * 8] = {
  /*  OPCODE 0000 */      -1, -1,     -1, -1,     -1, -1,     -1, -1,
  /*  OPCODE 1000 */      -1, -1,     -1, -1,     -1, -1,     -1, -1,
  /*  OPCODE 0100 */      17, 22,     17, 24,     25, 30,     25, 32,
  /*  OPCODE 1100 */      83, 94,     129,142,    97, 108,    145,158,
  /*  OPCODE 0010 */      83, 96,     129,144,    97, 110,    145,160,
  /*  OPCODE 1010 */      73, 77,     74, 77,     78, 82,     79, 82,
  /*  OPCODE 0110 */      33, 36,     34, 37,     38, 41,     39, 42,
  /*  OPCODE 1110 */      127,128,    127,128,    127,128,    127,128,
  /*  OPCODE 0001 */      177,190,    1,  16,     177,190,    1,  16,
  /*  OPCODE 1001 */      45, 56,     45, 58,     59, 70,     59, 72,
  /*  OPCODE 0101 */      161,166,    162,166,    169,174,    170,174,
  /*  OPCODE 1101 */      111,116,    111,118,    119,124,    119,126,
  /*  OPCODE 0011 */      161,168,    162,168,    169,176,    170,176,
  /*  OPCODE 1011 */      -1, -1,     -1, -1,     -1, -1,     -1, -1,
  /*  OPCODE 0111 */      -1, -1,     -1, -1,     -1, -1,     -1, -1,
  /*  OPCODE 1111 */      0,  0,      0,  0,      0,  0,      0,  0
};

/* ======================================================================== */
/*  bitrev       -- Bit-reverse a 32-bit number.                            */
/* ======================================================================== */
static inline libspectrum_dword
bitrev( libspectrum_dword val )
{
  val = ( ( val & 0xffff0000 ) >> 16 ) | ( ( val & 0x0000ffff ) << 16 );
  val = ( ( val & 0xff00ff00 ) >>  8 ) | ( ( val & 0x00ff00ff ) <<  8 );
  val = ( ( val & 0xf0f0f0f0 ) >>  4 ) | ( ( val & 0x0f0f0f0f ) <<  4 );
  val = ( ( val & 0xcccccccc ) >>  2 ) | ( ( val & 0x33333333 ) <<  2 );
  val = ( ( val & 0xaaaaaaaa ) >>  1 ) | ( ( val & 0x55555555 ) <<  1 );

  return val;
}

/* ======================================================================== */
/*  sp0256_getb  -- Get up to 8 bits at the current PC.                     */
/* ======================================================================== */
static libspectrum_dword
sp0256_getb( sp0256_t *s, int len )
{
  libspectrum_dword data = 0;
  libspectrum_dword d0, d1;

  /* -------------------------------------------------------------------- */
  /*  Fetch data from the FIFO or from the MASK                           */
  /* -------------------------------------------------------------------- */
  if( s->fifo_sel ) {
    d0 = s->fifo[( s->fifo_tail    ) & 63];
    d1 = s->fifo[( s->fifo_tail + 1 ) & 63];

    data = ( ( d1 << 10 ) | d0 ) >> s->fifo_bitp;

#ifdef DEBUG_FIFO
    debug_printf( ( "SP0256: RD_FIFO %.3X %d.%d %d\n", data & ( ( 1u << len ) - 1 ),
                s->fifo_tail, s->fifo_bitp, s->fifo_head ) );
#endif

    /* ---------------------------------------------------------------- */
    /*  Note the PC doesn't advance when we execute from FIFO.          */
    /*  Just the FIFO's bit-pointer advances.   (That's not REALLY      */
    /*  what happens, but that's roughly how it behaves.)               */
    /* ---------------------------------------------------------------- */
    s->fifo_bitp += len;
    if( s->fifo_bitp >= 10 ) {
      s->fifo_tail++;
      s->fifo_bitp -= 10;
    }
  } else {
    /* ---------------------------------------------------------------- */
    /*  Figure out which ROMs are being fetched into, and grab two      */
    /*  adjacent bytes.  The byte we're interested in is extracted      */
    /*  from the appropriate bit-boundary between them.                 */
    /* ---------------------------------------------------------------- */
    int idx0 = ( s->pc     ) >> 3, page0 = idx0 >> 12;
    int idx1 = ( s->pc + 8 ) >> 3, page1 = idx1 >> 12;

    idx0 &= 0xfff;
    idx1 &= 0xfff;

    d0 = d1 = 0;

    if( s->rom[page0] ) d0 = s->rom[page0][idx0];
    if( s->rom[page1] ) d1 = s->rom[page1][idx1];

    data = ( ( d1 << 8 ) | d0 ) >> ( s->pc & 7 );

    s->pc += len;
  }

  /* -------------------------------------------------------------------- */
  /*  Mask data to the requested length.                                  */
  /* -------------------------------------------------------------------- */
  data &= ( ( 1u << len ) - 1 );

  return data;
}

/* ======================================================================== */
/*  sp0256_micro -- Emulate the microsequencer in the SP0256.  Executes     */
/*                  instructions either until the repeat count != 0 or      */
/*                  the sequencer gets halted by a RTS to 0.                */
/* ======================================================================== */
static void
sp0256_micro( sp0256_t *s )
{
  libspectrum_byte immed4;
  libspectrum_byte opcode;
  libspectrum_word cr;
  int ctrl_xfer = 0;
  int repeat    = 0;
  int i, idx0, idx1;

  /* -------------------------------------------------------------------- */
  /*  Only execute instructions while the filter is not busy.             */
  /* -------------------------------------------------------------------- */
  while( s->filt.rpt <= 0 && s->filt.cnt <= 0 ) {
    /* ---------------------------------------------------------------- */
    /*  If the CPU is halted, see if we have a new command pending      */
    /*  in the Address LoaD buffer.                                     */
    /* ---------------------------------------------------------------- */
    if( s->halted && !s->lrq ) {
      s->pc       = s->ald | ( 0x1000 << 3 );
      s->fifo_sel = 0;
      s->halted   = 0;
      s->lrq      = 1;
      s->ald      = 0;
      /* for( i = 0; i < 16; i++ ) */
      /*   s->filt.r[i] = 0; */
    }

    /* ---------------------------------------------------------------- */
    /*  If we're still halted, do nothing.                              */
    /* ---------------------------------------------------------------- */
    if( s->halted ) {
      s->filt.rpt = 1;
      s->filt.cnt = 0;
      s->lrq      = 1;
      s->ald      = 0;
      /* for( i = 0; i < 16; i++ ) */
      /*   s->filt.r[i] = 0; */
      return;
    }

    /* ---------------------------------------------------------------- */
    /*  Fetch the first 8 bits of the opcode, which are always in the   */
    /*  same approximate format -- immed4 followed by opcode.           */
    /* ---------------------------------------------------------------- */
    immed4 = sp0256_getb( s, 4 );
    opcode = sp0256_getb( s, 4 );
    repeat = 0;
    ctrl_xfer = 0;

    debug_printf( ( "$%.4X.%.1X: OPCODE %d%d%d%d.%d%d\n",
                    ( s->pc >> 3 ) - 1, s->pc & 7,
                    !!( opcode & 1 ), !!( opcode & 2 ),
                    !!( opcode & 4 ), !!( opcode & 8 ),
                    !!( s->mode & 4 ), !!( s->mode & 2 ) ) );

    /* ---------------------------------------------------------------- */
    /*  Handle the special cases for specific opcodes.                  */
    /* ---------------------------------------------------------------- */
    switch( opcode ) {
    /* ------------------------------------------------------------ */
    /*  OPCODE 0000:  RTS / SETPAGE                                 */
    /* ------------------------------------------------------------ */
    case 0x0:
      /* -------------------------------------------------------- */
      /*  If immed4 != 0, then this is a SETPAGE instruction.     */
      /* -------------------------------------------------------- */
      if( immed4 ) {   /* SETPAGE */
        s->page = bitrev( immed4 ) >> 13;
      } else {
        /* -------------------------------------------------------- */
        /*  Otherwise, this is an RTS / HLT.                        */
        /* -------------------------------------------------------- */
        libspectrum_dword btrg;

        /* ---------------------------------------------------- */
        /*  Figure out our branch target.                       */
        /* ---------------------------------------------------- */
        btrg = s->stack;

        s->stack = 0;

        /* ---------------------------------------------------- */
        /*  If the branch target is zero, this is a HLT.        */
        /*  Otherwise, it's an RTS, so set the PC.              */
        /* ---------------------------------------------------- */
        if( !btrg ) {
          s->halted = 1;
          s->pc     = 0;
          ctrl_xfer  = 1;
        } else {
          s->pc    = btrg;
          ctrl_xfer = 1;
        }
      }

      break;

    /* ------------------------------------------------------------ */
    /*  OPCODE 0111:  JMP          Jump to 12-bit/16-bit Abs Addr   */
    /*  OPCODE 1011:  JSR          Jump to Subroutine               */
    /* ------------------------------------------------------------ */
    case 0xe:
    case 0xd:
    {
      int btrg;

      /* -------------------------------------------------------- */
      /*  Figure out our branch target.                           */
      /* -------------------------------------------------------- */
      btrg = s->page                           |
             ( bitrev( immed4 )            >> 17 ) |
             ( bitrev( sp0256_getb( s, 8 ) ) >> 21 );
      ctrl_xfer = 1;

      /* -------------------------------------------------------- */
      /*  If this is a JSR, push our return address on the        */
      /*  stack.  Make sure it's byte aligned.                    */
      /* -------------------------------------------------------- */
      if( opcode == 0xd )
        s->stack = ( s->pc + 7 ) & ~7;

      /* -------------------------------------------------------- */
      /*  Jump to the new location!                               */
      /* -------------------------------------------------------- */
      s->pc = btrg;
      break;
    }

    /* ------------------------------------------------------------ */
    /*  OPCODE 1000:  SETMODE      Set the Mode and Repeat MSBs     */
    /* ------------------------------------------------------------ */
    case 0x1:
      s->mode = ( ( immed4 & 8 ) >> 2 ) | ( immed4 & 4 ) |
                ( ( immed4 & 3 ) << 4 );
      break;

    /* ------------------------------------------------------------ */
    /*  OPCODE 0001:  LOADALL      Load All Parameters              */
    /*  OPCODE 0010:  LOAD_2       Load Per, Ampl, Coefs, Interp.   */
    /*  OPCODE 0011:  SETMSB_3     Load Pitch, Ampl, MSBs, & Intrp  */
    /*  OPCODE 0100:  LOAD_4       Load Pitch, Ampl, Coeffs         */
    /*  OPCODE 0101:  SETMSB_5     Load Pitch, Ampl, and Coeff MSBs */
    /*  OPCODE 0110:  SETMSB_6     Load Ampl, and Coeff MSBs.       */
    /*  OPCODE 1001:  DELTA_9      Delta update Ampl, Pitch, Coeffs */
    /*  OPCODE 1010:  SETMSB_A     Load Ampl and MSBs of 3 Coeffs   */
    /*  OPCODE 1100:  LOAD_C       Load Pitch, Ampl, Coeffs         */
    /*  OPCODE 1101:  DELTA_D      Delta update Ampl, Pitch, Coeffs */
    /*  OPCODE 1110:  LOAD_E       Load Pitch, Amplitude            */
    /*  OPCODE 1111:  PAUSE        Silent pause                     */
    /* ------------------------------------------------------------ */
    default:
      repeat = immed4 | ( s->mode & 0x30 );
      break;
    }
    if( opcode != 1 ) s->mode &= 0xf;

    /* ---------------------------------------------------------------- */
    /*  If this was a control transfer, handle setting "fifo_sel"       */
    /*  and all that ugliness.                                          */
    /* ---------------------------------------------------------------- */
    if( ctrl_xfer ) {
      debug_printf( ( "jumping to $%.4X.%.1X: ", s->pc >> 3, s->pc & 7 ) );

      /* ------------------------------------------------------------ */
      /*  Set our "FIFO Selected" flag based on whether we're going   */
      /*  to the FIFO's address.                                      */
      /* ------------------------------------------------------------ */
      s->fifo_sel = s->pc == FIFO_ADDR;

      debug_printf( ( "%s ", s->fifo_sel ? "FIFO" : "ROM" ) );

      /* ------------------------------------------------------------ */
      /*  Control transfers to the FIFO cause it to discard the       */
      /*  partial decle that's at the front of the FIFO.              */
      /* ------------------------------------------------------------ */
      if( s->fifo_sel && s->fifo_bitp ) {
        debug_printf( ( "bitp = %d -> Flush", s->fifo_bitp ) );

        /* Discard partially-read decle. */
        if( s->fifo_tail < s->fifo_head ) s->fifo_tail++;
        s->fifo_bitp = 0;
      }

      debug_printf( ( "\n" ) );

      continue;
    }

    /* ---------------------------------------------------------------- */
    /*  Otherwise, if we have a repeat count, then go grab the data     */
    /*  block and feed it to the filter.                                */
    /* ---------------------------------------------------------------- */
    if( !repeat ) continue;

    s->filt.rpt = repeat;
    debug_printf( ( "repeat = %d\n", repeat ) );

    /* clear delay line on new opcode */
    for( i = 0; i < 6; i++ )
      s->filt.z_data[i][0] = s->filt.z_data[i][1] = 0;

    i = ( opcode << 3 ) | ( s->mode & 6 );
    idx0 = sp0256_df_idx[i++];
    idx1 = sp0256_df_idx[i  ];

    assert( idx0 >= 0 && idx1 >= 0 && idx1 >= idx0 );

    /* ---------------------------------------------------------------- */
    /*  If we're in one of the 10-pole modes (x0), clear F5/B5.         */
    /* ---------------------------------------------------------------- */
    if( ( s->mode & 2 ) == 0 )
      s->filt.r[F5] = s->filt.r[B5] = 0;


    /* ---------------------------------------------------------------- */
    /*  Step through control words in the description for data block.   */
    /* ---------------------------------------------------------------- */
    for( i = idx0; i <= idx1; i++ ) {
      int len, shf, delta, field, prm, clrL;
      libspectrum_signed_byte value;

      /* ------------------------------------------------------------ */
      /*  Get the control word and pull out some important fields.    */
      /* ------------------------------------------------------------ */
      cr = sp0256_datafmt[i];

      len   = CR_LEN( cr );
      shf   = CR_SHF( cr );
      prm   = CR_PRM( cr );
      clrL  = cr & CR_CLRL;
      delta = cr & CR_DELTA;
      field = cr & CR_FIELD;
      value = 0;

      debug_printf( ( "$%.4X.%.1X: len=%2d shf=%2d prm=%2d d=%d f=%d ",
                      s->pc >> 3, s->pc & 7, len, shf, prm, !!delta,
                      !!field ) );
      /* ------------------------------------------------------------ */
      /*  Clear any registers that were requested to be cleared.      */
      /* ------------------------------------------------------------ */
      if( clrL ) {
        s->filt.r[F0] = s->filt.r[B0] = 0;
        s->filt.r[F1] = s->filt.r[B1] = 0;
        s->filt.r[F2] = s->filt.r[B2] = 0;
      }

      /* ------------------------------------------------------------ */
      /*  If this entry has a bitfield with it, grab the bitfield.    */
      /* ------------------------------------------------------------ */
      if( len ) {
        value = sp0256_getb( s, len );
      } else {
        debug_printf( ( " (no update)\n" ) );
        continue;
      }

      /* ------------------------------------------------------------ */
      /*  Sign extend if this is a delta update.                      */
      /* ------------------------------------------------------------ */
      if( delta ) { /* Sign extend */
        if( value & ( 1u << ( len - 1 ) ) ) value |= -( 1u << len );
      }

      /* ------------------------------------------------------------ */
      /*  Shift the value to the appropriate precision.               */
      /* ------------------------------------------------------------ */
      if( shf ) {
        value = value < 0 ? -( -value << shf ) : ( value << shf );
      }

      debug_printf( ( "v=%.2X (%c%.2X)  ", value & 0xff,
                      value & 0x80 ? '-' : '+',
                      0xff & ( value & 0x80 ? -value : value ) ) );

      s->silent = 0;

      /* ------------------------------------------------------------ */
      /*  If this is a field-replace, insert the field.               */
      /* ------------------------------------------------------------ */
      if( field ) {
        debug_printf( ( "--field-> r[%2d] = %.2X -> ", prm,
                        s->filt.r[prm] ) );

        s->filt.r[prm] &= ~( ~0u << shf ); /* Clear the old bits.     */
        s->filt.r[prm] |= value;           /* Merge in the new bits.  */

        debug_printf( ( "%.2X\n", s->filt.r[prm] ) );

        continue;
      }

      /* ------------------------------------------------------------ */
      /*  If this is a delta update, add to the appropriate field.    */
      /* ------------------------------------------------------------ */
      if( delta ) {
        debug_printf( ( "--delta-> r[%2d] = %.2X -> ", prm,
                        s->filt.r[prm] ) );

        s->filt.r[prm] += value;

        debug_printf( ( "%.2X\n", s->filt.r[prm] ) );

        continue;
      }

      /* ------------------------------------------------------------ */
      /*  Otherwise, just write the new value.                        */
      /* ------------------------------------------------------------ */
      s->filt.r[prm] = value;
      debug_printf( ( "--value-> r[%2d] = %.2X\n", prm, s->filt.r[prm] ) );
    }

    /* ---------------------------------------------------------------- */
    /*  Most opcodes clear IA, IP.                                      */
    /* ---------------------------------------------------------------- */
    if( opcode != 0x1 && opcode != 0x2 && opcode != 0x3 ) {
      s->filt.r[IA] = 0;
      s->filt.r[IP] = 0;
    }

    /* ---------------------------------------------------------------- */
    /*  Special case:  Set PAUSE's equivalent period.                   */
    /* ---------------------------------------------------------------- */
    if( opcode == 0xf ) {
      s->silent     = 1;
      s->filt.r[AM] = 0;
      s->filt.r[PR] = PER_PAUSE;
    }

    /* ---------------------------------------------------------------- */
    /*  Now that we've updated the registers, go decode them.           */
    /* ---------------------------------------------------------------- */
    lpc12_regdec( &s->filt );

    /* ---------------------------------------------------------------- */
    /*  Break out since we now have a repeat count.                     */
    /* ---------------------------------------------------------------- */
    break;
  }
}

/* ======================================================================== */
/*  sp0256_run   -- Where the magic happens.  Generate voice data for       */
/*                  our good friend, the SP0256.                            */
/* ======================================================================== */
static libspectrum_dword
sp0256_run( sp0256_t *s, libspectrum_dword len )
{
  libspectrum_signed_dword sp0256_now = s->sound_current;
  libspectrum_signed_dword until = sp0256_now + len;
  int samples, did_samp, old_idx;

  /* -------------------------------------------------------------------- */
  /*  If the rest of the machine hasn't caught up to us, just return.     */
  /* -------------------------------------------------------------------- */
  if( until <= s->sound_current )
    return 0;

  /* -------------------------------------------------------------------- */
  /*  Iterate the sound engine.                                           */
  /* -------------------------------------------------------------------- */
  while( s->sound_current < until ) {
    /* ---------------------------------------------------------------- */
    /*  Renormalize our sc_head and sc_tail.                            */
    /* ---------------------------------------------------------------- */
    while( s->sc_head > SCBUF_SIZE && s->sc_tail > SCBUF_SIZE ) {
      s->sc_head -= SCBUF_SIZE;
      s->sc_tail -= SCBUF_SIZE;
    }

    s->sc_tail = s->sc_head;

    /* ---------------------------------------------------------------- */
    /*  Calculate the number of samples required at ~10kHz.             */
    /*  (Actually, on NTSC this is 3579545 / 358, or 9998.73 Hz).       */
    /* ---------------------------------------------------------------- */
    samples = ( (int)( until - s->sound_current + s->clock_per_samp - 1 ) )
            / s->clock_per_samp;

    /* ---------------------------------------------------------------- */
    /*  Process the current set of filter coefficients as long as the   */
    /*  repeat count holds up and we have room in our scratch buffer.   */
    /* ---------------------------------------------------------------- */
    did_samp = 0;
    old_idx  = s->sc_head;
    if( samples > 0 ) {
      do {
        int do_samp;

        /* ------------------------------------------------------------ */
        /*  If our repeat count expired, emulate the microsequencer.    */
        /* ------------------------------------------------------------ */
        if( s->filt.rpt <= 0 && s->filt.cnt <= 0 )
          sp0256_micro( s );

        /* ------------------------------------------------------------ */
        /*  Do as many samples as we can.                               */
        /* ------------------------------------------------------------ */
        do_samp = samples - did_samp;
        if( s->sc_head + do_samp - s->sc_tail > SCBUF_SIZE )
          do_samp = s->sc_tail + SCBUF_SIZE - s->sc_head;

        if( do_samp == 0 ) break;

        if( s->silent &&
            s->filt.rpt <= 0 && s->filt.cnt <= 0 ) {
          int x, y = s->sc_head;

          for( x = 0; x < do_samp; x++ )
            s->scratch[y++ & SCBUF_MASK] = 0;
          s->sc_head += do_samp;
          did_samp        += do_samp;
        } else {
          did_samp += lpc12_update( &s->filt, do_samp,
                                    s->scratch, &s->sc_head );
        }
      } while ( ( s->filt.rpt > 0 || s->filt.cnt > 0 ) &&
                samples > did_samp );
    }

    if( did_samp ) {
      int i;
      for( i = 0; i < did_samp; i++ ) {
        sound_sp0256_write( s->sound_current + ( i * s->clock_per_samp ),
                            s->scratch[( i + old_idx ) & SCBUF_MASK] );
      }
    }

    s->sound_current += did_samp * s->clock_per_samp;
  }

  if( s->sound_current < sp0256_now ) {
    return 0;
  }

  return s->sound_current - sp0256_now;
}


/* ======================================================================== */
/*  sp0256_read_lrq -- Handle reads from the SP0256.                        */
/* ======================================================================== */
static libspectrum_dword
sp0256_read_lrq( sp0256_t *s )
{
  return s->lrq;
}

/* ======================================================================== */
/*  sp0256_write_ald -- Handle writes to the SP0256's Address LoaD FIFO.    */
/* ======================================================================== */
static void
sp0256_write_ald( sp0256_t *s, libspectrum_dword data )
{
  if( !s->lrq )
    return;

  /* ---------------------------------------------------------------- */
  /*  Set LRQ to "busy" and load the 8 LSBs of the data into the ALD  */
  /*  reg.  We take the command address, and multiply by 2 bytes to   */
  /*  get the new PC address.                                         */
  /* ---------------------------------------------------------------- */
  s->lrq = 0;
  s->ald = ( 0xff & data ) << 4;

  return;
}

/* ======================================================================== */
/*  sp0256_reset -- Resets the SP0256.                                      */
/* ======================================================================== */
static void
sp0256_do_reset( sp0256_t *s )
{
  /* -------------------------------------------------------------------- */
  /*  Do a software-style reset of the SP0256.                            */
  /* -------------------------------------------------------------------- */
  s->fifo_head = s->fifo_tail = s->fifo_bitp = 0;

  memset( &s->filt, 0, sizeof( s->filt ) );
  s->halted   = 1;
  s->filt.rpt = -1;
  s->filt.rng = 1;
  s->lrq      = 1;
  s->ald      = 0x0000;
  s->pc       = 0x0000;
  s->stack    = 0x0000;
  s->fifo_sel = 0;
  s->mode     = 0;
  s->page     = 0x1000 << 3;
  s->silent   = 1;
}

/* ======================================================================== */
/*  sp0256_do_init -- Makes a new SP0256.                                   */
/* ======================================================================== */
static int
sp0256_do_init( sp0256_t *s, libspectrum_byte *sp0256rom )
{
  /* -------------------------------------------------------------------- */
  /*  First, lets zero out the structure to be safe.                      */
  /* -------------------------------------------------------------------- */
  memset( s, 0, sizeof( *s ) );

  /* -------------------------------------------------------------------- */
  /*  Configure our internal variables.                                   */
  /* -------------------------------------------------------------------- */
  s->filt.rng = 1;

  /* -------------------------------------------------------------------- */
  /*  Allocate a scratch buffer for generating 10kHz samples.             */
  /* -------------------------------------------------------------------- */
  s->scratch = calloc( SCBUF_SIZE, sizeof( libspectrum_signed_word ) );
  s->sc_head = s->sc_tail = 0;

  if( !s->scratch )
    return -1;

  /* -------------------------------------------------------------------- */
  /*  Set up the microsequencer's initial state.                          */
  /* -------------------------------------------------------------------- */
  s->halted   = 1;
  s->filt.rpt = -1;
  s->lrq      = 1;
  s->page     = 0x1000 << 3;
  s->silent   = 1;

  /* -------------------------------------------------------------------- */
  /*  Attempt to read SP0256 ROM files.  This needs re-architecting if    */
  /*  you're going to have multiple SP0256's in a system, or use ROMs     */
  /*  from various places, but it'll do for the moment.                   */
  /* -------------------------------------------------------------------- */

  /* Original code was iterating over 16 "pages", but we always provided
     ROM for "page" 1 only */
  s->rom[1] = sp0256rom;

  /* Set nominal crystal clock at 3.12 MHz */
  s->clock = 3120000;

  /* Nominal sample duration of 350 tstates on a Z80 3.50 MHz */
  s->clock_per_samp = 350;

  return 0;
}

#ifdef DEBUG_AL2_ALLOPHONE
struct allophone_t {
  const char *name;
  int length;
};

static const struct allophone_t al2_allophones[ 64 ] = {

  /* pauses */
  { "PA1",    10 },
  { "PA2",    30 },
  { "PA3",    50 },
  { "PA4",   100 },
  { "PA5",   200 },

  /* allophones */
  { "/OY/",  420 }, /* oy */
  { "/AY/",  260 }, /* (ii) */
  { "/EH/",   70 }, /* e */
  { "/KK3/", 120 }, /* c */
  { "/PP/",  210 }, /* p */
  { "/JH/",  140 }, /* j */
  { "/NN1/", 140 }, /* n */
  { "/IH/",   70 }, /* i */
  { "/TT2/", 140 }, /* (tt) */
  { "/RR1/", 170 }, /* (rr) */
  { "/AX/",   70 }, /* u */
  { "/MM/",  180 }, /* m */
  { "/TT1/", 100 }, /* t */
  { "/DH1/", 290 }, /* dth */
  { "/IY/",  250 }, /* (ee) */
  { "/EY/",  280 }, /* (aa), (ay) */
  { "/DD1/",  70 }, /* d */
  { "/UW1/", 100 }, /* ou */
  { "/AO/",  100 }, /* o */
  { "/AA/",  100 }, /* a */
  { "/YY2/", 180 }, /* (yy) */
  { "/AE/",  120 }, /* eh */
  { "/HH1/", 130 }, /* h */
  { "/BB1/",  80 }, /* b */
  { "/TH/",  180 }, /* th */
  { "/UH/",  100 }, /* uh */
  { "/UW2/", 260 }, /* ouu */
  { "/AW/",  370 }, /* ow */
  { "/DD2/", 160 }, /* (dd) */
  { "/GG3/", 140 }, /* (ggg) */
  { "/VV/",  190 }, /* v */
  { "/GG1/",  80 }, /* g */
  { "/SH/",  160 }, /* sh */
  { "/ZH/",  190 }, /* zh */
  { "/RR2/", 120 }, /* r */
  { "/FF/",  150 }, /* f */
  { "/KK2/", 190 }, /* ck? (gg)? */
  { "/KK1/", 160 }, /* k */
  { "/ZZ/",  210 }, /* z */
  { "/NG/",  220 }, /* ng */
  { "/LL/",  110 }, /* l */
  { "/WW/",  180 }, /* w */
  { "/XR/",  360 }, /* aer */
  { "/WH/",  200 }, /* wh */
  { "/YY1/", 130 }, /* y */
  { "/CH/",  190 }, /* ch */
  { "/ER1/", 160 }, /* er */
  { "/ER2/", 300 }, /* err */
  { "/OW/",  240 }, /* (oo), (eau) */
  { "/DH2/", 240 }, /* ? */
  { "/SS/",   90 }, /* s */
  { "/NN2/", 190 }, /* (nn) */
  { "/HH2/", 180 }, /* (hh) */
  { "/OR/",  330 }, /* or */
  { "/AR/",  290 }, /* ar */
  { "/YR/",  350 }, /* ear */
  { "/GG2/",  40 }, /* ? */
  { "/EL/",  190 }, /* (ll) */
  { "/BB2/",  50 }, /* (bb) */

};
#endif /* #ifdef DEBUG_AL2_ALLOPHONE */

static sp0256_t sp0256;

int
sp0256_init( libspectrum_byte *sp0256rom )
{
  return sp0256_do_init( &sp0256, sp0256rom );
}

int
sp0256_reset( libspectrum_byte *sp0256rom )
{
  /* Lazy init, if necessary */
  if( !sp0256.scratch ) {
    if( sp0256_do_init( &sp0256, sp0256rom ) ) {
      return -1;
    }
  } else {
    sp0256_do_reset( &sp0256 );
  }
  return 0;
}

void
sp0256_end()
{
  if( sp0256.scratch ) {
    free( sp0256.scratch );
  }
  memset( &sp0256, 0, sizeof( sp0256 ) );
}

static void
sp0256_run_to( sp0256_t *s, libspectrum_dword t )
{
  libspectrum_dword periph_step;

  while( s->sound_current < (libspectrum_signed_dword) t ) {
    libspectrum_signed_dword len = t - s->sound_current;
    if( len > 14934 ) len = 14934;

    periph_step = sp0256_run( s, len );

    if( !periph_step ) {
      return;
    }
  }
}

void
sp0256_do_frame( void )
{
  /* No op if it wasn't initialised yet */
  if( !sp0256.scratch ) return;
  sp0256_run_to( &sp0256, machine_current->timings.tstates_per_frame );

  sp0256.sound_current -= machine_current->timings.tstates_per_frame;
}

void
sp0256_play( int a )
{
#ifdef DEBUG_AL2_ALLOPHONE
  if( a >= 5 && a < 64 ) {
    if( al2_allophones[ a ].name ) {
      printf( "sp0256: allophone written: 0x%02x, %s\n", a,
              al2_allophones[a].name );
    } else {
      printf( "sp0256: allophone written: 0x%02x\n", a );
    }
    fflush( stdout );
  }
#endif /* #ifdef DEBUG_AL2_ALLOPHONE */

  sp0256_run_to( &sp0256, tstates );
  sp0256_write_ald( &sp0256, a );
}

void
sp0256_set_clock( libspectrum_dword clock )
{
  unsigned int samples;
  libspectrum_dword processor_speed;

  /* CPC wiki states:
     The SP0256 is (usually) driven by a 3.12MHz oscillator, and it uses
     7bit PWM output, which is clocked at 3.12MHz/2, to obtain a 10kHz
     sample rate, the chip issues some dummy steps with constant LOW level
     additionally to the 128 steps needed for 7bit PWM, making it a total
     number of 156 steps per sample.
  */
  sp0256.clock = clock;

  /* SP0256 sample rate:
     A nominal 3.12 MHz XTAL would produce 10000 samples (10 kHz).
     With uSpeech (normal intonation), a 3.05 MHz XTAL would produce 9775 samples.
     With uSpeech (high intonation), a 3.26 MHz XTAL would produce 10448 samples.
   */
  samples = sp0256.clock / 312.0;

  processor_speed = ( machine_current == NULL ) ? 3500000 :
                    machine_current->timings.processor_speed;

  /* Z80 sample rate (examples with Z80 clocked at 3.5 MHz):
     - A SP0256 sample at 3.12 MHz would last 350 (Z80) t-states.
     - A SP0256 sample at 3.05 MHz would last 358 (Z80) t-states.
     - A SP0256 sample at 3.26 MHz would last 334 (Z80) t-states.
   */
  sp0256.clock_per_samp = processor_speed / samples;
}

void
sp0256_change_clock( libspectrum_dword clock )
{
  if( sp0256.clock != clock ) {
    sp0256_run_to( &sp0256, tstates );
    sp0256_set_clock( clock );
  }
}

int
sp0256_busy( void )
{
  sp0256_run_to( &sp0256, tstates );
  return !sp0256_read_lrq( &sp0256 );
}
