/* wd_fdc.c: Western Digital floppy disk controller emulation
   Copyright (c) 2002-2007 Stuart Brady, Fredrick Meunier, Philip Kendall,
   Dmitry Sanarin, Gergely Szasz

   $Id: wd_fdc.c 4636 2012-01-20 14:07:15Z pak21 $

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

#include <config.h>

#include <libspectrum.h>

#include "crc.h"
#include "event.h"
#include "spectrum.h"
#include "ui/ui.h"
#include "wd_fdc.h"

static void statusbar_update( int busy );
static void crc_preset( wd_fdc *f );
static void crc_add( wd_fdc *f, wd_fdc_drive *d );
static int read_id( wd_fdc *f );
static int read_datamark( wd_fdc *f );
static void wd_fdc_seek_verify( wd_fdc *f );
static void wd_fdc_type_i( wd_fdc *f );
static void wd_fdc_type_ii( wd_fdc *f );
static void wd_fdc_type_iii( wd_fdc *f );
static int wd_fdc_spinup( wd_fdc *f, libspectrum_byte b );
static void wd_fdc_event( libspectrum_dword last_tstates, int event,
			  void *user_data );

static int fdc_event, motor_off_event, timeout_event;

void
wd_fdc_init_events( void )
{
  fdc_event = event_register( wd_fdc_event, "WD FDC event" );
  motor_off_event = event_register( wd_fdc_event, "WD FDC motor off" );
  timeout_event = event_register( wd_fdc_event, "WD FDC timeout" );
}

void
wd_fdc_master_reset( wd_fdc *f )
{
  wd_fdc_drive *d = f->current_drive;

  f->spin_cycles = 0;
  f->direction = 0;
  f->head_load = 0;
  if( d ) {
    if( f->flags & WD_FLAG_BETA128 )
      fdd_motoron( &d->fdd, 0 );
    else
      fdd_head_load( &d->fdd, 0 );
  }
  f->read_id = 0;
  if( f->hlt_time > 0 ) f->hlt = 0;
  f->intrq = 0;
  f->datarq = 0;

  f->state = WD_FDC_STATE_NONE;
  f->status_type = WD_FDC_STATUS_TYPE1;

  if( d != NULL ) {
    while( !d->fdd.tr00 )
      fdd_step( &d->fdd, FDD_STEP_OUT );
  }

  f->track_register = 0;
  f->sector_register = 0;
  f->data_register = 0;
  f->status_register = WD_FDC_SR_LOST; /* track 0 */

}

wd_fdc *
wd_fdc_alloc_fdc( wd_type_t type, int hlt_time, unsigned int flags )
{
  wd_fdc *fdc = malloc( sizeof( *fdc ) );
  if( !fdc ) return NULL;

  switch( type ) {
  default:
    type = WD1770;			/* illegal type converted to wd_fdc */
  case FD1793:
  case WD1773:
  case WD1770:
    fdc->rates[ 0 ] = 6;
    fdc->rates[ 1 ] = 12;
    fdc->rates[ 2 ] = 20;
    fdc->rates[ 3 ] = 30;
    break;
  case WD1772:
    fdc->rates[ 0 ] = 2;
    fdc->rates[ 1 ] = 3;
    fdc->rates[ 2 ] = 5;
    fdc->rates[ 3 ] = 6;
    break;
  }
  fdc->type = type;
  fdc->current_drive = NULL;
  fdc->hlt_time = hlt_time;
  fdc->flags = flags;			/* Beta128 connect HLD out to READY in and MOTOR ON */
  wd_fdc_master_reset( fdc );
  return fdc;
}

static void
statusbar_update( int busy )
{
  ui_statusbar_update( UI_STATUSBAR_ITEM_DISK,
		       busy ? UI_STATUSBAR_STATE_ACTIVE :
			      UI_STATUSBAR_STATE_INACTIVE );
}

void
wd_fdc_set_intrq( wd_fdc *f )
{
  if( ( f->type == WD1770 || f->type == WD1772 ) &&
      f->status_register & WD_FDC_SR_MOTORON        ) {
    event_add_with_data( tstates + 2 * 			/* 10 rev: 10 * 200 / 1000 */
			 machine_current->timings.processor_speed,
			 motor_off_event, f );
  }

  if( ( f->type == WD1773 || f->type == FD1793 ) &&
      f->head_load				    ) {
    event_add_with_data( tstates + 3 * 			/* 15 revolution: 15 * 200 / 1000 */
			 machine_current->timings.processor_speed,
			 motor_off_event, f );
  }
  if( f->intrq != 1 ) {
    f->intrq = 1;
    if( f->set_intrq ) f->set_intrq( f );
  }
}

void
wd_fdc_reset_intrq( wd_fdc *f )
{
  if( f->intrq == 1 ) {
    f->intrq = 0;
    if( f->reset_intrq ) f->reset_intrq( f );
  }
}

void
wd_fdc_set_datarq( wd_fdc *f )
{
  if( f->datarq != 1 ) {
    f->status_register |= WD_FDC_SR_IDX_DRQ;
    f->datarq = 1;
    if( f->set_datarq ) f->set_datarq( f );
  }
}

void
wd_fdc_reset_datarq( wd_fdc *f )
{
  if( f->datarq == 1 ) {
    f->status_register &= ~WD_FDC_SR_IDX_DRQ;
    f->datarq = 0;
    if( f->reset_datarq ) f->reset_datarq( f );
  }
}

void
wd_fdc_set_hlt( wd_fdc *f, int hlt )
{
  f->hlt = hlt > 0 ? 1 : 0;
}

static void
crc_preset( wd_fdc *f )
{
  f->crc = 0xffff;
}

static void
crc_add( wd_fdc *f, wd_fdc_drive *d )
{
  f->crc = crc_fdc( f->crc, d->fdd.data & 0xff );
}

/* return 0 if found an ID 
   return 1 if not found ID
   return 2 if found but with CRC error (READ ADDRESS command)
*/
static int
read_id( wd_fdc *f )
{
  int i = f->rev;
  wd_fdc_drive *d = f->current_drive;

  f->id_mark = WD_FDC_AM_NONE;

  if( f->rev <= 0 )
    return 1;

  while( i == f->rev ) {
    crc_preset( f );
    if( f->dden ) {	/* double density (MFM) */
      fdd_read_write_data( &d->fdd, FDD_READ );
      if( d->fdd.index ) f->rev--;
      crc_add(f, d);
      if( d->fdd.data == 0xffa1 ) {
        fdd_read_write_data( &d->fdd, FDD_READ ); crc_add(f, d);
        if( d->fdd.index ) f->rev--;
        if( d->fdd.data != 0xffa1 )
          continue;
        fdd_read_write_data( &d->fdd, FDD_READ ); crc_add(f, d);
        if( d->fdd.index ) f->rev--;
        if( d->fdd.data != 0xffa1 )
          continue;
      } else {		/* no 0xa1 with missing clock... */
        continue;
      }
    }
    fdd_read_write_data( &d->fdd, FDD_READ ); crc_add(f, d);
    if( d->fdd.index ) f->rev--;
    if( f->dden ) {	/* double density (MFM) */
      if( d->fdd.data != 0x00fe )
        continue;
    } else {		/* single density (FM) */
      if( d->fdd.data != 0xfffe )
        continue;
    }
    fdd_read_write_data( &d->fdd, FDD_READ ); crc_add(f, d);
    if( d->fdd.index ) f->rev--;
    f->id_track = d->fdd.data;
    fdd_read_write_data( &d->fdd, FDD_READ ); crc_add(f, d);
    if( d->fdd.index ) f->rev--;
    f->id_head = d->fdd.data;
    fdd_read_write_data( &d->fdd, FDD_READ ); crc_add(f, d);
    if( d->fdd.index ) f->rev--;
    f->id_sector = d->fdd.data;
    fdd_read_write_data( &d->fdd, FDD_READ ); crc_add(f, d);
    if( d->fdd.index ) f->rev--;
    f->id_length = d->fdd.data;
    f->sector_length = 0x80 << d->fdd.data; 
    fdd_read_write_data( &d->fdd, FDD_READ ); crc_add(f, d);
    if( d->fdd.index ) f->rev--;
    fdd_read_write_data( &d->fdd, FDD_READ ); crc_add(f, d);
    if( d->fdd.index ) f->rev--;
    if( f->crc != 0x0000 ) {
      f->status_register |= WD_FDC_SR_CRCERR;
      f->id_mark = WD_FDC_AM_ID;
      return 2;
    } else {
      f->status_register &= ~WD_FDC_SR_CRCERR;
      f->id_mark = WD_FDC_AM_ID;
      return 0;
    }
  }
  return 1;
}

static int
read_datamark( wd_fdc *f )
{
  wd_fdc_drive *d = f->current_drive;
  int i;

  f->id_mark = WD_FDC_AM_NONE;

  if( f->dden ) {	/* double density (MFM) */
    for( i = 40; i > 0; i-- ) {
      fdd_read_write_data( &d->fdd, FDD_READ );
      if( d->fdd.data == 0x4e )		/* read next */
	continue;

      if( d->fdd.data == 0x00 )		/* go to PLL sync */
	break;

      return 1;				/* something wrong... */
    } 
    for( ; i > 0; i-- ) {
      crc_preset( f );
      fdd_read_write_data( &d->fdd, FDD_READ ); crc_add(f, d);
      if( d->fdd.data == 0x00 )
	continue;

      if( d->fdd.data == 0xffa1 )	/* got to a1 mark */
	break;

      return 1;
    }
    for( i = d->fdd.data == 0xffa1 ? 2 : 3; i > 0; i-- ) {
      fdd_read_write_data( &d->fdd, FDD_READ ); crc_add(f, d);
      if( d->fdd.data != 0xffa1 )
	return 1;
    } 
    fdd_read_write_data( &d->fdd, FDD_READ ); crc_add(f, d);
    if( d->fdd.data < 0x00f8 || d->fdd.data > 0x00fb )	/* !fb deleted mark */
      return 1;
    if( d->fdd.data != 0x00fb )
      f->ddam = 1;
    else
      f->ddam = 0;
    f->id_mark = WD_FDC_AM_DATA;
    return 0;
  } else {		/* SD -> FM */
    for( i = 30; i > 0; i-- ) {
      fdd_read_write_data( &d->fdd, FDD_READ );
      if( d->fdd.data == 0xff )		/* read next */
	continue;

      if( d->fdd.data == 0x00 )		/* go to PLL sync */
	break;

      return 1;				/* something wrong... */
    } 
    for( ; i > 0; i-- ) {
      crc_preset( f );
      fdd_read_write_data( &d->fdd, FDD_READ ); crc_add(f, d);
      if( d->fdd.data == 0x00 )
	continue;

      if( d->fdd.data >= 0xfff8 && d->fdd.data <= 0xfffb )	/* !fb deleted mark */
	break;

      return 1;
    }
    if( i == 0 ) {
      fdd_read_write_data( &d->fdd, FDD_READ ); crc_add(f, d);
      if( d->fdd.data < 0xfff8 || d->fdd.data > 0xfffb )	/* !fb deleted mark */
	return 1;
    }
    if( d->fdd.data != 0x00fb )
      f->ddam = 1;
    else
      f->ddam = 0;
    f->id_mark = WD_FDC_AM_DATA;
    return 0;
  }
  return 1;
}

libspectrum_byte
wd_fdc_sr_read( wd_fdc *f )
{
  wd_fdc_drive *d = f->current_drive;

  wd_fdc_reset_intrq( f );

  if( f->status_type == WD_FDC_STATUS_TYPE1 ) {
    f->status_register &= ~WD_FDC_SR_IDX_DRQ;
    if( !d->fdd.loaded || d->index_pulse )
      f->status_register |= WD_FDC_SR_IDX_DRQ;
  }
  if( f->type == WD1773 || f->type == FD1793 ) {

/*    if( f->status_register & WD_FDC_SR_BUSY )
      f->status_register |= WD_FDC_SR_MOTORON;
    else
      f->status_register &= ~WD_FDC_SR_MOTORON; */

    if( ( ( f->flags & WD_FLAG_BETA128 ) && f->head_load ) ||
        ( !( f->flags & WD_FLAG_BETA128 ) && d->fdd.ready ) )
      f->status_register &= ~WD_FDC_SR_MOTORON;
    else
      f->status_register |= WD_FDC_SR_MOTORON;
  }

  return f->status_register;
}

static void
wd_fdc_seek_verify_read_id( wd_fdc *f )
{
  int i;
  f->read_id = 1;

  event_remove_type( fdc_event );
  if( f->id_mark == WD_FDC_AM_NONE ) {
    while( f->rev ) {
      i = f->current_drive->disk.i >= f->current_drive->disk.bpt ?
    		0 : f->current_drive->disk.i;			/* start position */
      if( !read_id( f ) ) {
	if( f->id_track != f->track_register ) {
	  f->status_register |= WD_FDC_SR_RNF;
	}
      } else
        f->id_mark = WD_FDC_AM_NONE;
      i = f->current_drive->disk.bpt ? 
	( f->current_drive->disk.i - i ) * 200 / f->current_drive->disk.bpt : 200;
      if( i > 0 ) {
        event_add_with_data( tstates + i *		/* i * 1/20 revolution */
			   machine_current->timings.processor_speed / 1000,
			   fdc_event, f );
        return;
      } else if( f->id_mark != WD_FDC_AM_NONE )
        break;
    }
    if( f->id_mark == WD_FDC_AM_NONE )
      f->status_register |= WD_FDC_SR_RNF;
  }
  f->state = WD_FDC_STATE_NONE;
  f->status_register &= ~WD_FDC_SR_BUSY;
  wd_fdc_set_intrq( f );
  f->read_id = 0;
  return;
}

static void
wd_fdc_seek_verify( wd_fdc *f )
{
  wd_fdc_drive *d = f->current_drive;

  event_remove_type( fdc_event );
  if( f->type == WD1773 || f->type == FD1793 ) {
    if( !f->hlt ) {
      event_add_with_data( tstates + 5 * 			/* sample every 5 ms */
    		    machine_current->timings.processor_speed / 1000,
			fdc_event, f );
      return;
    }
    if( f->head_load )
      f->status_register |= WD_FDC_SR_SPINUP;	
			/* when set, it indicates head is loaded and enganged.
			   This bit is logical "and" of HLD and "HLT" signals.  */
  }

  if( d->fdd.tr00 )
    f->status_register |= WD_FDC_SR_LOST;
  else
    f->status_register &= ~WD_FDC_SR_LOST;

  f->rev = 5;
  f->id_mark = WD_FDC_AM_NONE;
  wd_fdc_seek_verify_read_id( f );
}

static void
wd_fdc_type_i( wd_fdc *f )
{
  libspectrum_byte b = f->command_register;
  wd_fdc_drive *d = f->current_drive;

  if( f->state == WD_FDC_STATE_SEEK_DELAY ) {	/* after delay */
    if( ( b & 0x60 ) != 0x00 )			/* STEP/STEP-IN/STEP-OUT */
      goto type_i_verify;
    goto type_i_loop;
  } else {					/* WD_FDC_STATE_SEEK */
    f->status_register |= WD_FDC_SR_SPINUP;
  }

  if( ( b & 0x60 ) != 0x00 ) {			/* STEP/STEP-IN/STEP-OUT */
    if( b & 0x40 )
      f->direction = b & 0x20 ? FDD_STEP_OUT : FDD_STEP_IN;
    if( b & 0x10 )				/* update? */
      goto type_i_update;
    goto type_i_noupdate;
  }
						/* SEEK or RESTORE */
  if ( !( b & 0x10 ) ) {				/* RESTORE */
    f->track_register = 0xff;
    f->data_register = 0;
  }

type_i_loop:
  if( f->track_register != f->data_register ) {
    f->direction = f->track_register < f->data_register ? 
			FDD_STEP_IN : FDD_STEP_OUT;

type_i_update:
    f->track_register += f->direction == FDD_STEP_IN ? 1 : -1;

type_i_noupdate:
    if( d->fdd.tr00 && f->direction == FDD_STEP_OUT ) {
      f->track_register = 0;
    } else {
      fdd_step( &d->fdd, f->direction );
      f->state = WD_FDC_STATE_SEEK_DELAY;
      event_remove_type( fdc_event );
      event_add_with_data( tstates + f->rates[ b & 0x03 ] * 
			   machine_current->timings.processor_speed / 1000,
			   fdc_event, f );
      return;
    }
  }

type_i_verify:
  if( b & 0x04 ) {

    if( f->type == WD1773 || f->type == FD1793 ) {
      f->head_load = 1;
      event_remove_type( motor_off_event );
      if( f->flags & WD_FLAG_BETA128 )
        fdd_motoron( &f->current_drive->fdd, 1 );
      else
        fdd_head_load( &f->current_drive->fdd, 1 );
      event_remove_type( fdc_event );
      event_add_with_data( tstates + 15 * 				/* 15ms */
		    machine_current->timings.processor_speed / 1000,
			fdc_event, f );
      statusbar_update( 1 );
    }

    f->state = WD_FDC_STATE_VERIFY;

    if( ( f->type == WD1770 || f->type == WD1772 ) &&
	!( f->status_register & WD_FDC_SR_MOTORON ) ) {
      f->status_register |= WD_FDC_SR_MOTORON;
      fdd_motoron( &f->current_drive->fdd, 1 );
      statusbar_update( 1 );
      event_remove_type( fdc_event );
      event_add_with_data( tstates + 12 * 		/* 6 revolution 6 * 200 / 1000 */
		    machine_current->timings.processor_speed / 10,
			fdc_event, f );
      return;
    }

    wd_fdc_seek_verify( f );
    return;
  }

  if( d->fdd.tr00 )
    f->status_register |= WD_FDC_SR_LOST;
  else
    f->status_register &= ~WD_FDC_SR_LOST;

  f->state = WD_FDC_STATE_NONE;
  f->status_register &= ~WD_FDC_SR_BUSY;
  wd_fdc_set_intrq( f );
}

static void
wd_fdc_type_ii_seek( wd_fdc *f )
{
  libspectrum_byte b = f->command_register;
  wd_fdc_drive *d = f->current_drive;
  int i;

  event_remove_type( fdc_event );
  if( f->id_mark == WD_FDC_AM_NONE ) {
    f->read_id = 1;
    while( f->rev ) {
      i = f->current_drive->disk.i >= f->current_drive->disk.bpt ? 
    		0 : f->current_drive->disk.i;			/* start position */
      if( !read_id( f ) ) {
        if( ( f->data_check_head != -1 && f->data_check_head != f->id_head ) ||
	    ( f->id_track != f->track_register || f->id_sector != f->sector_register ) ) {
          f->id_mark = WD_FDC_AM_NONE;
	}
      } else {
        f->id_mark = WD_FDC_AM_NONE;
      }
      i = f->current_drive->disk.bpt ? 
	  ( f->current_drive->disk.i - i ) * 200 / f->current_drive->disk.bpt : 200;
      if( i > 0 ) {
        event_add_with_data( tstates + i *		/* i * 1/20 revolution */
			     machine_current->timings.processor_speed / 1000,
			     fdc_event, f );
        return;
      } else if( f->id_mark != WD_FDC_AM_NONE ) {
	break;
      }
    }
  }

  f->read_id = 0;

  if( f->id_mark == WD_FDC_AM_NONE ) {
    f->status_register |= WD_FDC_SR_RNF;
    f->status_register &= ~WD_FDC_SR_BUSY;
    f->state = WD_FDC_STATE_NONE;
    wd_fdc_set_intrq( f );
    return;
  }

  if( f->state == WD_FDC_STATE_READ ) {
    if( f->id_mark == WD_FDC_AM_ID )
      read_datamark( f );

    if( f->id_mark == WD_FDC_AM_NONE ) {		/* not found */
      f->status_register |= WD_FDC_SR_RNF;
      f->status_register &= ~WD_FDC_SR_BUSY;
      f->state = WD_FDC_STATE_NONE;
      wd_fdc_set_intrq( f );
      return;
    }
    if( f->ddam )
      f->status_register |= WD_FDC_SR_SPINUP;	/* set deleted data mark */
    wd_fdc_set_datarq( f );
    f->data_offset = 0;

  } else {
    f->ddam = b & 0x01;
    for( i = 11; i > 0; i-- )	/* "delay" 11 GAP byte */
      fdd_read_write_data( &d->fdd, FDD_READ );
    wd_fdc_set_datarq( f );
    f->data_offset = 0;
    if( f->dden )
      for( i = 11; i > 0; i-- )	/* "delay" another 11 GAP byte */
	fdd_read_write_data( &d->fdd, FDD_READ );

    d->fdd.data = 0x00;
    for( i = f->dden ? 12 : 6; i > 0; i-- )	/* write 6/12 zero */
      fdd_read_write_data( &d->fdd, FDD_WRITE );
    crc_preset( f );
    if( f->dden ) {				/* MFM */
      d->fdd.data = 0xffa1;
      for( i = 3; i > 0; i-- ) {		/* write 3 0xa1 with clock mark */
	fdd_read_write_data( &d->fdd, FDD_WRITE ); crc_add(f, d);
      }
    }
    d->fdd.data = ( f->ddam ? 0x00f8 : 0x00fb ) |
    			( f->dden ? 0x0000 : 0xff00 );	/* write data mark */
    fdd_read_write_data( &d->fdd, FDD_WRITE ); crc_add(f, d);
  }
  event_remove_type( timeout_event );
  event_add_with_data( tstates + 			/* 5 revolutions: 5 * 200 / 1000 */
		       machine_current->timings.processor_speed,
		       timeout_event, f );
}

static void
wd_fdc_type_ii( wd_fdc *f )
{
  libspectrum_byte b = f->command_register;
  wd_fdc_drive *d = f->current_drive;

  event_remove_type( fdc_event );
  if( f->type == WD1773 || f->type == FD1793 ) {
    if( !f->hlt ) {
      event_add_with_data( tstates + 5 * 
    		    machine_current->timings.processor_speed / 1000,
			fdc_event, f );
      return;
    }
  }

  if( f->state == WD_FDC_STATE_WRITE ) {
    if( d->fdd.wrprot ) {
      f->status_register |= WD_FDC_SR_WRPROT;
      f->status_register &= ~WD_FDC_SR_BUSY;
      f->state = WD_FDC_STATE_NONE;
      wd_fdc_set_intrq( f );
      return;
    }
    f->status_register &= ~WD_FDC_SR_WRPROT;
  }

  f->data_multisector = b & 0x10 ? 1 : 0;
  f->rev = 5;
  f->id_mark = WD_FDC_AM_NONE;
  wd_fdc_type_ii_seek( f );
}

static void
wd_fdc_type_iii( wd_fdc *f )
{
  int i;
  wd_fdc_drive *d = f->current_drive;

  event_remove_type( fdc_event );
  if( !f->read_id && ( f->type == WD1773 || f->type == FD1793 ) ) {
    if( !f->hlt ) {
      event_add_with_data( tstates + 5 *
    		    machine_current->timings.processor_speed / 1000,
			fdc_event, f );
      return;
    }
  }
  if( f->state == WD_FDC_STATE_WRITETRACK ) {		/* ----WRITE TRACK---- */
    if( d->fdd.wrprot ) {
      f->status_register |= WD_FDC_SR_WRPROT;
      f->status_register &= ~WD_FDC_SR_BUSY;
      f->state = WD_FDC_STATE_NONE;
      wd_fdc_set_intrq( f );
      return;
    }
    f->status_register &= ~WD_FDC_SR_WRPROT;

    f->data_offset = 0;
    fdd_wait_index_hole( &d->fdd );
    wd_fdc_set_datarq( f );
  } else if( f->state == WD_FDC_STATE_READTRACK ) {	/* ----READ TRACK---- */
    fdd_wait_index_hole( &d->fdd );
    wd_fdc_set_datarq( f );
  } else {						/* ----READID---- */
    if( !f->read_id ) {
      f->read_id = 1;
      f->rev = 5;
      f->id_mark = WD_FDC_AM_NONE;
    }
    if( f->id_mark == WD_FDC_AM_NONE ) {
      while( f->rev ) {
        i = f->current_drive->disk.i >= f->current_drive->disk.bpt ? 
    		0 : f->current_drive->disk.i;			/* start position */
        read_id( f );
        i = f->current_drive->disk.bpt ? 
	    ( f->current_drive->disk.i - i ) * 200 / f->current_drive->disk.bpt : 200;
	if( i > 0 ) {
          event_add_with_data( tstates + i *		/* i * 1/20 revolution */
			       machine_current->timings.processor_speed / 1000,
			       fdc_event, f );
          return;
	} else if( f->id_mark != WD_FDC_AM_NONE )
	  break;
      }
      if( f->id_mark == WD_FDC_AM_NONE ) {
        f->state = WD_FDC_STATE_NONE;
        f->status_register |= WD_FDC_SR_RNF;
        f->status_register &= ~WD_FDC_SR_BUSY;
        wd_fdc_set_intrq( f );
        f->read_id = 0;
        return;
      }
    }
    f->read_id = 0;
    f->data_offset = 0;
    wd_fdc_set_datarq( f );
  }
  event_remove_type( timeout_event );
  event_add_with_data( tstates + 2 * 20 *	/* 2 revolutions: 2 * 200 / 1000  */
		       machine_current->timings.processor_speed / 100,
		       timeout_event, f );
}

static void
wd_fdc_event( libspectrum_dword last_tstates GCC_UNUSED, int event,
	      void *user_data ) 
{
  wd_fdc *f = user_data;

  if( event == timeout_event ) {
    if( f->state == WD_FDC_STATE_READ       ||
	f->state == WD_FDC_STATE_WRITE      ||
	f->state == WD_FDC_STATE_READTRACK  ||
	f->state == WD_FDC_STATE_WRITETRACK ||
	f->state == WD_FDC_STATE_READID        ) {
      f->state = WD_FDC_STATE_NONE;
      f->status_register |= WD_FDC_SR_LOST;
      f->status_register &= ~WD_FDC_SR_BUSY;
      wd_fdc_reset_datarq( f );
      wd_fdc_set_intrq( f );
    }
    return;
  }

  if( event == motor_off_event ) {
    if( f->type == WD1770 || f->type == WD1772 ) {
      f->status_register &= ~WD_FDC_SR_MOTORON;
      fdd_motoron( &f->current_drive->fdd, 0 );
    } else {						/* WD1773/FD1973 */
      f->head_load = 0;
      if( f->flags & WD_FLAG_BETA128 )
        fdd_motoron( &f->current_drive->fdd, 0 );
      else
        fdd_head_load( &f->current_drive->fdd, 0 );
    }
    statusbar_update( 0 );
    return;
  }

  if( ( f->type == WD1773 || f->type == FD1793 ) &&
        f->hlt_time > 0 && f->head_load && !f->hlt )
    f->hlt = 1;

  if( ( ( f->type == WD1770 || f->type == WD1772 ) &&
	( f->status_register & WD_FDC_SR_MOTORON ) && 
	f->status_type == WD_FDC_STATUS_TYPE1 ) ||
      ( ( f->type == WD1773 || f->type == FD1793 ) &&
	( f->state == WD_FDC_STATE_SEEK ||
	  f->state == WD_FDC_STATE_SEEK_DELAY ) &&
	f->head_load ) ) {
    f->status_register |= WD_FDC_SR_SPINUP;
  }

  if( f->read_id ) {
    if( f->state == WD_FDC_STATE_VERIFY )
      wd_fdc_seek_verify_read_id( f );
    else if( ( f->state == WD_FDC_STATE_READ || f->state == WD_FDC_STATE_WRITE ) &&
	     f->datarq )
      f->datarq = 0, wd_fdc_set_datarq( f );
    else if( f->state == WD_FDC_STATE_READ || f->state == WD_FDC_STATE_WRITE )
      wd_fdc_type_ii_seek( f );
    else if( f->state == WD_FDC_STATE_READID )
      wd_fdc_type_iii( f );
  } else if( f->state == WD_FDC_STATE_SEEK || f->state == WD_FDC_STATE_SEEK_DELAY )
    wd_fdc_type_i( f );
  else if( f->state == WD_FDC_STATE_VERIFY )
    wd_fdc_seek_verify( f );
  else if( ( f->state == WD_FDC_STATE_READ || f->state == WD_FDC_STATE_WRITE ) &&
	   f->datarq )
    f->datarq = 0, wd_fdc_set_datarq( f );
  else if( f->state == WD_FDC_STATE_READ || f->state == WD_FDC_STATE_WRITE )
    wd_fdc_type_ii( f );
  else if( ( f->state == WD_FDC_STATE_READTRACK  ||
	     f->state == WD_FDC_STATE_READID     ||
	     f->state == WD_FDC_STATE_WRITETRACK   ) &&
	   f->datarq )
      f->datarq = 0, wd_fdc_set_datarq( f );
  else if( f->state == WD_FDC_STATE_READTRACK  ||
	   f->state == WD_FDC_STATE_READID     ||
	   f->state == WD_FDC_STATE_WRITETRACK    )
    wd_fdc_type_iii( f );
}

static int
wd_fdc_spinup( wd_fdc *f, libspectrum_byte b )
{
  libspectrum_dword delay = 0;

  if( f->state != WD_FDC_STATE_SEEK && ( b & 0x04 ) )
    delay = 30;

  if( f->type == WD1770 || f->type == WD1772 ) {
    if( !( b & 0x08 ) &&
	!( f->status_register & WD_FDC_SR_MOTORON ) ) {
      f->status_register |= WD_FDC_SR_MOTORON;
      fdd_motoron( &f->current_drive->fdd, 1 );
      statusbar_update( 1 );
      delay += 6 * 200;
    }
  } else {			/* WD1773/FD1793 */
    event_remove_type( motor_off_event );
    if( f->state == WD_FDC_STATE_SEEK ) {
      if( b & 0x08 ) {
	f->head_load = 1;
        if( f->flags & WD_FLAG_BETA128 )
          fdd_motoron( &f->current_drive->fdd, 1 );
        else
	  fdd_head_load( &f->current_drive->fdd, 1 );
	statusbar_update( 1 );
      } else {
	f->head_load = 0;
        if( f->hlt_time > 0 ) f->hlt = 0;		/* reset the trigger */
        if( f->flags & WD_FLAG_BETA128 )
          fdd_motoron( &f->current_drive->fdd, 0 );
        else
	  fdd_head_load( &f->current_drive->fdd, 0 );
	statusbar_update( 0 );
      }
      return 0;
    } else {
      f->head_load = 1;
      if( f->flags & WD_FLAG_BETA128 )
        fdd_motoron( &f->current_drive->fdd, 1 );
      else
        fdd_head_load( &f->current_drive->fdd, 1 );
      statusbar_update( 1 );
      if( f->hlt_time > 0 )
        delay += f->hlt_time;
    }
  }
  if( delay ) {
    event_remove_type( fdc_event );
    event_add_with_data( tstates + delay * 
    		    machine_current->timings.processor_speed / 1000,
			fdc_event, f );
    return 1;
  }
  return 0;
}

void
wd_fdc_cr_write( wd_fdc *f, libspectrum_byte b )
{
  wd_fdc_drive *d = f->current_drive;

  wd_fdc_reset_intrq( f );

  if( ( b & 0xf0 ) == 0xd0 ) {			/* Type IV - Force Interrupt */
    event_remove_type( fdc_event );
    f->status_register &= ~( WD_FDC_SR_BUSY   | WD_FDC_SR_WRPROT |
			     WD_FDC_SR_CRCERR | WD_FDC_SR_IDX_DRQ );
    f->state = WD_FDC_STATE_NONE;
    f->status_type = WD_FDC_STATUS_TYPE1;
    wd_fdc_reset_datarq( f );

    if( b & 0x08 )
      wd_fdc_set_intrq( f );
    else if( b & 0x04 )
      d->index_interrupt = 1;

    if( d->fdd.tr00 )
      f->status_register |= WD_FDC_SR_LOST;
    else
      f->status_register &= ~WD_FDC_SR_LOST;

    return;
  }

  if( f->status_register & WD_FDC_SR_BUSY )
    return;

  f->command_register = b;
  f->status_register |= WD_FDC_SR_BUSY;

  /* keep spindle motor on: */
  event_remove_type( motor_off_event );

  if( !( b & 0x80 ) ) {                         /* Type I */
    f->state = WD_FDC_STATE_SEEK;
    f->status_type = WD_FDC_STATUS_TYPE1;
    f->status_register &= ~( WD_FDC_SR_CRCERR | WD_FDC_SR_RNF |
			     WD_FDC_SR_IDX_DRQ );
    wd_fdc_reset_datarq( f );

    f->rev = 5;
    if( wd_fdc_spinup( f, b ) )
      return;
    wd_fdc_type_i( f );
  } else if( !( b & 0x40 ) ) {                  /* Type II */
    if( f->type == WD1773 || f->type == FD1793 ) {
      if( ( ( f->flags & WD_FLAG_BETA128 ) && !f->head_load ) ||
        ( !( f->flags & WD_FLAG_BETA128 ) && !d->fdd.ready ) ) {
        f->status_register &= ~WD_FDC_SR_BUSY;
        f->state = WD_FDC_STATE_NONE;
        wd_fdc_set_intrq( f );
        return;
      }
    }
    if( f->type == WD1773 && b & 0x02 )
      f->data_check_head = b & 0x08 ? 1 : 0;
    else
      f->data_check_head = -1;

    f->state = b & 0x20 ? WD_FDC_STATE_WRITE : WD_FDC_STATE_READ;
    f->status_type = WD_FDC_STATUS_TYPE2;
    f->status_register &= ~( WD_FDC_SR_WRPROT | WD_FDC_SR_RNF |
			     WD_FDC_SR_IDX_DRQ| WD_FDC_SR_LOST|
			     WD_FDC_SR_SPINUP );
    f->rev = 5;
    if( wd_fdc_spinup( f, b ) )
      return;
    wd_fdc_type_ii( f );
  } else if( ( b & 0x30 ) != 0x10 ) {           /* Type III */
    if( f->type == WD1773 || f->type == FD1793 ) {
      if( ( ( f->flags & WD_FLAG_BETA128 ) && !f->head_load ) ||
        ( !( f->flags & WD_FLAG_BETA128 ) && !d->fdd.ready ) ) {
        f->status_register &= ~WD_FDC_SR_BUSY;
        f->state = WD_FDC_STATE_NONE;
        wd_fdc_set_intrq( f );
        return;
      }
    }
    f->state = b & 0x20 ? ( b & 0x10 ? 
			    WD_FDC_STATE_WRITETRACK : WD_FDC_STATE_READTRACK ) :
		WD_FDC_STATE_READID;
    f->status_type = WD_FDC_STATUS_TYPE2;
    f->status_register &= ~( WD_FDC_SR_SPINUP | WD_FDC_SR_RNF |
			     WD_FDC_SR_IDX_DRQ| WD_FDC_SR_LOST );

    f->rev = 5;
    if( wd_fdc_spinup( f, b ) )
      return;
    wd_fdc_type_iii( f );
  }
}

libspectrum_byte
wd_fdc_tr_read( wd_fdc *f )
{
  return f->track_register;
}

void
wd_fdc_tr_write( wd_fdc *f, libspectrum_byte b )
{
  f->track_register = b;
}

libspectrum_byte
wd_fdc_sec_read( wd_fdc *f )
{
  return f->sector_register;
}

void
wd_fdc_sec_write( wd_fdc *f, libspectrum_byte b )
{
  f->sector_register = b;
}

libspectrum_byte
wd_fdc_dr_read( wd_fdc *f )
{
  wd_fdc_drive *d = f->current_drive;

  if( f->flags & WD_FLAG_OPUS &&
	f->status_register & WD_FDC_SR_BUSY )
    event_remove_type( fdc_event );

  if( f->state == WD_FDC_STATE_READ ) {
    f->data_offset++;				/* count read bytes */
    fdd_read_write_data( &d->fdd, FDD_READ ); crc_add(f, d); /* read a byte */
    if( d->fdd.data > 0xff ) {			/* no data */
      f->status_register |= WD_FDC_SR_RNF;
      f->status_register &= ~WD_FDC_SR_BUSY;
      f->status_type = WD_FDC_STATUS_TYPE2;
      f->state = WD_FDC_STATE_NONE;
      wd_fdc_set_intrq( f );
      wd_fdc_reset_datarq( f );
    } else {
      f->data_register = d->fdd.data;
      if( f->data_offset == f->sector_length ) {	/* read the CRC */
	fdd_read_write_data( &d->fdd, FDD_READ ); crc_add(f, d);
	fdd_read_write_data( &d->fdd, FDD_READ ); crc_add(f, d);

	/* FIXME: make this per-FDC */
	event_remove_type( timeout_event );	/* clear the timeout */

	if( f->crc == 0x0000 && f->data_multisector ) {
	  f->sector_register++;
	  f->rev = 5;
	  wd_fdc_reset_datarq( f );
	  event_add_with_data( tstates +	 	/* 5 revolutions: 5 * 200 / 1000 */
			       machine_current->timings.processor_speed,
			       timeout_event, f );
	  event_add_with_data( tstates + 2 * 		/* 20 ms delay */
			       machine_current->timings.processor_speed / 100,
			       fdc_event, f );
	} else {
	  f->status_register &= ~WD_FDC_SR_BUSY;
	  if( f->crc == 0x0000 )
	    f->status_register &= ~WD_FDC_SR_CRCERR;
	  else
	    f->status_register |= WD_FDC_SR_CRCERR;
	  f->status_type = WD_FDC_STATUS_TYPE2;
	  f->state = WD_FDC_STATE_NONE;
	  wd_fdc_set_intrq( f );
	  wd_fdc_reset_datarq( f );
	}
      }
    }
  } else if( f->state == WD_FDC_STATE_READID ) {
    switch( f->data_offset ) {
    case 0:		/* track */
      f->data_register = f->id_track;
      break;
    case 1:		/* head */
      f->data_register = f->id_head;
      break;
    case 2:		/* sector */
      f->data_register = f->id_sector;
      break;
    case 3:		/* length */
      f->data_register = f->id_length;
      break;
    case 4:		/* crc1 */
      f->data_register = f->crc >> 8;
      break;
    case 5:		/* crc2 */
      f->sector_register = f->id_track;
      f->data_register = f->crc & 0xff;
      f->status_register &= ~WD_FDC_SR_BUSY;
      f->status_type = WD_FDC_STATUS_TYPE2;
      f->state = WD_FDC_STATE_NONE;
      event_remove_type( timeout_event );	/* clear the timeout */
      wd_fdc_set_intrq( f );
      wd_fdc_reset_datarq( f );
      break;
    default:
      break;
    }
    f->data_offset++;
  } else if( f->state == WD_FDC_STATE_READTRACK ) {
						/* unformatted/out of track looks like 1x 0x00 */
    fdd_read_write_data( &d->fdd, FDD_READ );	/* read a byte and give to host */
    f->data_register = d->fdd.data & 0x00ff;	/* drop clock marks */
    if( d->fdd.index ) {
      event_remove_type( timeout_event );	/* clear the timeout */
      f->status_register &= ~WD_FDC_SR_BUSY;
      f->status_type = WD_FDC_STATUS_TYPE2;
      f->state = WD_FDC_STATE_NONE;
      wd_fdc_set_intrq( f );
      wd_fdc_reset_datarq( f );
    }
  }
  if( ( f->flags & WD_FLAG_OPUS ) &&
	( f->status_register & WD_FDC_SR_BUSY ) ) {	/* we need a next datarq */
    event_add_with_data( tstates + 30 * 		/* 30 us delay */
			       machine_current->timings.processor_speed / 1000000,
			       fdc_event, f );
  }
  return f->data_register;
}

void
wd_fdc_dr_write( wd_fdc *f, libspectrum_byte b )
{
  wd_fdc_drive *d = f->current_drive;

  f->data_register = b;
  if( f->state == WD_FDC_STATE_WRITE ) {
    d->fdd.data = b;
    f->data_offset++;				/* count bytes read */
    fdd_read_write_data( &d->fdd, FDD_WRITE ); crc_add(f, d);
    if( f->data_offset == f->sector_length ) {	/* write the CRC */
      d->fdd.data = f->crc >> 8;
      fdd_read_write_data( &d->fdd, FDD_WRITE );	/* write crc1 */
      d->fdd.data = f->crc & 0xff;
      fdd_read_write_data( &d->fdd, FDD_WRITE );	/* write crc2 */
      d->fdd.data = 0xff;
      fdd_read_write_data( &d->fdd, FDD_WRITE );	/* write 1 byte of ff? */

      event_remove_type( timeout_event );	/* clear the timeout */

      if( f->data_multisector ) {
	f->sector_register++;
	f->rev = 5;
	wd_fdc_reset_datarq( f );
	event_add_with_data( tstates +		/* 5 revolutions: 5 * 200 / 1000 */
			     machine_current->timings.processor_speed,
			     timeout_event, f );
	event_add_with_data( tstates + 2 * 		/* 20ms delay */
			     machine_current->timings.processor_speed / 100,
			     fdc_event, f );
      } else {
	f->status_register &= ~WD_FDC_SR_BUSY;
	f->status_type = WD_FDC_STATUS_TYPE2;
	f->state = WD_FDC_STATE_NONE;
	wd_fdc_set_intrq( f );
	wd_fdc_reset_datarq( f );
      }
    }
  } else if( f->state == WD_FDC_STATE_WRITETRACK ) {
    d->fdd.data = b;
    if( f->dden ) {			/* MFM */
      if( b == 0xf7 ) {			/* CRC */
	d->fdd.data = f->crc >> 8;
	fdd_read_write_data( &d->fdd, FDD_WRITE );	/* write crc1 */
	d->fdd.data = f->crc & 0xff;
      } else if ( b == 0xf5 ) {
	d->fdd.data = 0xffa1;		/* and preset CRC */
	f->crc = 0xcdb4;		/* 3x crc = crc_fdc( crc, 0xa1 )???? */
      } else if ( b == 0xf6 ) {
	d->fdd.data = 0xffc2;
      } else {
	crc_add(f, d);
      }
    } else {				/* FM */
      if( b == 0xf7 ) {			/* CRC */
	d->fdd.data = f->crc >> 8;
	fdd_read_write_data( &d->fdd, FDD_WRITE );	/* write crc1 */
	d->fdd.data = f->crc & 0xff;
      } else if ( b == 0xfe || ( b >= 0xf8 && b <= 0xfb ) ) {
	crc_preset( f );		/* preset CRC */
	crc_add(f, d);
	d->fdd.data |= 0xff00;
      } else if ( b == 0xfc ) {
	d->fdd.data |= 0xff00;
      } else {
	crc_add(f, d);
      }
    }
    fdd_read_write_data( &d->fdd, FDD_WRITE );	/* write a byte */

    if( d->fdd.index ) {
      event_remove_type( timeout_event );	/* clear the timeout */
      f->status_register &= ~WD_FDC_SR_BUSY;
      f->state = WD_FDC_STATE_NONE;
      wd_fdc_set_intrq( f );
      wd_fdc_reset_datarq( f );
    }
  }
  if( ( f->flags & WD_FLAG_OPUS ) &&
	f->status_register & WD_FDC_SR_BUSY ) {	/* we need a next datarq */
//    wd_fdc_reset_datarq( f );
    event_add_with_data( tstates + 30 * 		/* 30 us delay */
			       machine_current->timings.processor_speed / 1000000,
			       fdc_event, f );
  }
}
