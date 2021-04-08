/* command.c: Parse a debugger command
   Copyright (c) 2002-2008 Philip Kendall

   $Id: command.c 4730 2012-09-03 12:50:19Z fredm $

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

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "debugger.h"
#include "debugger_internals.h"
#include "mempool.h"
#include "ui/ui.h"
#include "utils.h"
#include "z80/z80.h"
#include "z80/z80_macros.h"

/* The last debugger command we were given to execute */
static char *command_buffer = NULL;

/* And a pointer as to how much we've parsed */
static char *command_ptr;

int yyparse( void );
int yywrap( void );

/* Evaluate the debugger command given in 'command' */
void
debugger_command_evaluate( const char *command )
{
  if( !command ) return;

  if( command_buffer ) free( command_buffer );

  command_buffer = utils_safe_strdup( command );

  /* Start parsing at the start of the given command */
  command_ptr = command_buffer;
    
  /* Parse the command */
  yyparse();

  /* And free any memory we allocated while parsing */
  mempool_free( debugger_memory_pool );

  ui_debugger_update();
}

/* Utility functions called from the flex scanner */

int
yywrap( void )
{
  return 1;
}

/* Called to get up to 'max_size' bytes of the command to be parsed */
int
debugger_command_input( char *buf, int *result, int max_size )
{
  size_t length = strlen( command_ptr );

  if( !length ) {
    return 0;
  } else if( length < (size_t)max_size ) {
    memcpy( buf, command_ptr, length );
    *result = length; command_ptr += length;
    return 1;
  } else {
    memcpy( buf, command_ptr, max_size );
    *result = max_size; command_ptr += max_size;
    return 1;
  }
}

/* Convert a register name to a useful index value */
int
debugger_register_hash( const char *name )
{
  int hash = 0x0000, primed = 0;
  size_t length;
  const char *ptr;

  length = strlen( name );

  if( name[ length - 1 ] == '\'' ) { primed = 1; length--; }

  for( ptr = name; ptr < name + length; ptr++ ) {
    hash <<= 8; hash |= tolower( *ptr );
  }

  if( primed ) hash |= 0x8000;

  switch( hash ) {
    case 0x0061: case 0x8061:	/* A, A' */
    case 0x0066: case 0x8066:	/* F, F' */
    case 0x0062: case 0x8062:	/* B, B' */
    case 0x0063: case 0x8063:	/* C, C' */
    case 0x0064: case 0x8064:	/* D, D' */
    case 0x0065: case 0x8065:	/* E, E' */
    case 0x0068: case 0x8068:	/* H, H' */
    case 0x006c: case 0x806c:	/* L, L' */
    case 0x6166: case 0xe166:	/* AF, AF' */
    case 0x6263: case 0xe263:	/* BC, BC' */
    case 0x6465: case 0xe465:	/* DE, DE' */
    case 0x686c: case 0xe86c:	/* HL, HL' */
    case 0x7370:		/* SP */
    case 0x7063:		/* PC */
    case 0x6978:		/* IX */
    case 0x6979:		/* IY */
    case 0x696d:		/* IM */
    case 0x69666631:		/* IFF1 */
    case 0x69666632:		/* IFF2 */
      return hash;

    default: return -1;
  }
}

/* Utility functions called by the bison parser */

/* The error callback if yyparse finds an error */
void
yyerror( const char *s )
{
  ui_error( UI_ERROR_ERROR, "Invalid debugger command: %s", s );
}

/* Get the value of a register */
libspectrum_word
debugger_register_get( int which )
{
  switch( which ) {

    /* 8-bit registers */
  case 0x0061: return A;
  case 0x8061: return A_;
  case 0x0066: return F;
  case 0x8066: return F_;
  case 0x0062: return B;
  case 0x8062: return B_;
  case 0x0063: return C;
  case 0x8063: return C_;
  case 0x0064: return D;
  case 0x8064: return D_;
  case 0x0065: return E;
  case 0x8065: return E_;
  case 0x0068: return H;
  case 0x8068: return H_;
  case 0x006c: return L;
  case 0x806c: return L_;
    
    /* 16-bit registers */
  case 0x6166: return AF;
  case 0xe166: return AF_;
  case 0x6263: return BC;
  case 0xe263: return BC_;
  case 0x6465: return DE;
  case 0xe465: return DE_;
  case 0x686c: return HL;
  case 0xe86c: return HL_;

  case 0x7370: return SP;
  case 0x7063: return PC;
  case 0x6978: return IX;
  case 0x6979: return IY;

   /* interrupt flags */
  case 0x696d: return IM;
  case 0x69666631: return IFF1;
  case 0x69666632: return IFF2;

  default:
    ui_error( UI_ERROR_ERROR, "attempt to get unknown register '%d'", which );
    return 0;
  }
}

/* Set the value of a register */
void
debugger_register_set( int which, libspectrum_word value )
{
  switch( which ) {

    /* 8-bit registers */
    case 0x0061: A = value; break;
    case 0x8061: A_ = value; break;
    case 0x0066: F = value; break;
    case 0x8066: F_ = value; break;
    case 0x0062: B = value; break;
    case 0x8062: B_ = value; break;
    case 0x0063: C = value; break;
    case 0x8063: C_ = value; break;
    case 0x0064: D = value; break;
    case 0x8064: D_ = value; break;
    case 0x0065: E = value; break;
    case 0x8065: E_ = value; break;
    case 0x0068: H = value; break;
    case 0x8068: H_ = value; break;
    case 0x006c: L = value; break;
    case 0x806c: L_ = value; break;

    /* 16-bit registers */
    case 0x6166: AF = value; break;
    case 0xe166: AF_ = value; break;
    case 0x6263: BC = value; break;
    case 0xe263: BC_ = value; break;
    case 0x6465: DE = value; break;
    case 0xe465: DE_ = value; break;
    case 0x686c: HL = value; break;
    case 0xe86c: HL_ = value; break;

    case 0x7370: SP = value; break;
    case 0x7063: PC = value; break;
    case 0x6978: IX = value; break;
    case 0x6979: IY = value; break;

     /* interrupt flags */
    case 0x696d: if( value >= 0 && value <=2 ) IM = value; break;
    case 0x69666631: IFF1 = !!value; break;
    case 0x69666632: IFF2 = !!value; break;

  default:
    ui_error( UI_ERROR_ERROR, "attempt to set unknown register '%d'", which );
    break;
  }
}

/* Get the textual representation of a register */
const char *
debugger_register_text( int which )
{
  switch( which ) {

    /* 8-bit registers */
  case 0x0061: return "A";
  case 0x8061: return "A'";
  case 0x0066: return "F";
  case 0x8066: return "F'";
  case 0x0062: return "B";
  case 0x8062: return "B'";
  case 0x0063: return "C";
  case 0x8063: return "C'";
  case 0x0064: return "D";
  case 0x8064: return "D'";
  case 0x0065: return "E";
  case 0x8065: return "E'";
  case 0x0068: return "H";
  case 0x8068: return "H'";
  case 0x006c: return "L";
  case 0x806c: return "L'";
    
    /* 16-bit registers */
  case 0x6166: return "AF";
  case 0xe166: return "AF'";
  case 0x6263: return "BC";
  case 0xe263: return "BC'";
  case 0x6465: return "DE";
  case 0xe465: return "DE'";
  case 0x686c: return "HL";
  case 0xe86c: return "HL'";

  case 0x7370: return "SP";
  case 0x7063: return "PC";
  case 0x6978: return "IX";
  case 0x6979: return "IY";

   /* interrupt flags */
  case 0x696d: return "IM";
  case 0x69666631: return "IFF1";
  case 0x69666632: return "IFF2";

  default:
    ui_error( UI_ERROR_ERROR, "attempt to get unknown register '%d'", which );
    return "(invalid)";
  }
}
