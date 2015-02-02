/* debugger_internals.h: The internals of Fuse's monitor/debugger
   Copyright (c) 2002-2011 Philip Kendall

   $Id: debugger_internals.h 4696 2012-05-07 02:05:13Z fredm $

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

#ifndef FUSE_DEBUGGER_INTERNALS_H
#define FUSE_DEBUGGER_INTERNALS_H

#include "debugger.h"

/* Memory pool used by the lexer and parser */
extern int debugger_memory_pool;

/* The event type used to trigger time breakpoints */
extern int debugger_breakpoint_event;

void debugger_breakpoint_time_fn( libspectrum_dword tstates, int type, void *user_data );

int debugger_breakpoint_remove( size_t id );
int debugger_breakpoint_remove_all( void );
int debugger_breakpoint_clear( libspectrum_word address );
int debugger_breakpoint_exit( void );
int debugger_breakpoint_ignore( size_t id, size_t ignore );
int debugger_breakpoint_set_condition( size_t id,
				       debugger_expression *condition );
int debugger_breakpoint_set_commands( size_t id, const char *commands );
int debugger_breakpoint_trigger( debugger_breakpoint *bp );

int debugger_poke( libspectrum_word address, libspectrum_byte value );
int debugger_port_write( libspectrum_word address, libspectrum_byte value );

int debugger_register_hash( const char *reg );
libspectrum_word debugger_register_get( int which );
void debugger_register_set( int which, libspectrum_word value );
const char* debugger_register_text( int which );

void debugger_exit_emulator( void );

/* Utility functions called by the flex scanner */

int debugger_command_input( char *buf, int *result, int max_size );
int yylex( void );
void yyerror( const char *s );

/* The semantic values of some tokens */

typedef enum debugger_token {

  /* Chosen to match up with Unicode values */
  DEBUGGER_TOKEN_LOGICAL_AND = 0x2227,
  DEBUGGER_TOKEN_LOGICAL_OR = 0x2228,

  DEBUGGER_TOKEN_EQUAL_TO = 0x225f,
  DEBUGGER_TOKEN_NOT_EQUAL_TO = 0x2260,

  DEBUGGER_TOKEN_LESS_THAN_OR_EQUAL_TO = 0x2264,
  DEBUGGER_TOKEN_GREATER_THAN_OR_EQUAL_TO = 0x2265,

} debugger_token;

/* Numeric expression stuff */

debugger_expression*
debugger_expression_new_number( libspectrum_dword number, int pool );
debugger_expression* debugger_expression_new_register( int which, int pool );
debugger_expression*
debugger_expression_new_unaryop( int operation, debugger_expression *operand, int pool );
debugger_expression*
debugger_expression_new_binaryop( int operation, debugger_expression *operand1,
				  debugger_expression *operand2, int pool );
debugger_expression*
debugger_expression_new_variable( const char *name, int pool );

debugger_expression* debugger_expression_copy( debugger_expression *src );
void debugger_expression_delete( debugger_expression* expression );

libspectrum_dword
debugger_expression_evaluate( debugger_expression* expression );

/* Event handling */

void debugger_event_init( void );
int debugger_event_is_registered( const char *type, const char *detail );
void debugger_event_end( void );

/* Variables handling */

void debugger_variable_init( void );
void debugger_variable_end( void );
void debugger_variable_set( const char *name, libspectrum_dword value );
libspectrum_dword debugger_variable_get( const char *name );

#endif				/* #ifndef FUSE_DEBUGGER_INTERNALS_H */
