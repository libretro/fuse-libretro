/* A Bison parser, made by GNU Bison 2.7.12-4996.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_COMMANDY_H_INCLUDED
# define YY_YY_COMMANDY_H_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     LOGICAL_OR = 258,
     LOGICAL_AND = 259,
     COMPARISON = 260,
     EQUALITY = 261,
     NEGATE = 262,
     TIMES_DIVIDE = 263,
     BASE = 264,
     BREAK = 265,
     TBREAK = 266,
     CLEAR = 267,
     COMMANDS = 268,
     CONDITION = 269,
     CONTINUE = 270,
     DEBUGGER_DELETE = 271,
     DISASSEMBLE = 272,
     DEBUGGER_END = 273,
     EVENT = 274,
     EXIT = 275,
     FINISH = 276,
     IF = 277,
     DEBUGGER_IGNORE = 278,
     NEXT = 279,
     DEBUGGER_OUT = 280,
     PORT = 281,
     DEBUGGER_PRINT = 282,
     READ = 283,
     SET = 284,
     STEP = 285,
     TIME = 286,
     WRITE = 287,
     DEBUGGER_REGISTER = 288,
     NUMBER = 289,
     STRING = 290,
     VARIABLE = 291,
     DEBUGGER_ERROR = 292
   };
#endif
/* Tokens.  */
#define LOGICAL_OR 258
#define LOGICAL_AND 259
#define COMPARISON 260
#define EQUALITY 261
#define NEGATE 262
#define TIMES_DIVIDE 263
#define BASE 264
#define BREAK 265
#define TBREAK 266
#define CLEAR 267
#define COMMANDS 268
#define CONDITION 269
#define CONTINUE 270
#define DEBUGGER_DELETE 271
#define DISASSEMBLE 272
#define DEBUGGER_END 273
#define EVENT 274
#define EXIT 275
#define FINISH 276
#define IF 277
#define DEBUGGER_IGNORE 278
#define NEXT 279
#define DEBUGGER_OUT 280
#define PORT 281
#define DEBUGGER_PRINT 282
#define READ 283
#define SET 284
#define STEP 285
#define TIME 286
#define WRITE 287
#define DEBUGGER_REGISTER 288
#define NUMBER 289
#define STRING 290
#define VARIABLE 291
#define DEBUGGER_ERROR 292



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{
/* Line 2053 of yacc.c  */
#line 46 "commandy.y"


  int token;
  int reg;

  libspectrum_dword integer;
  char *string;

  debugger_breakpoint_type bptype;
  debugger_breakpoint_life bplife;
  struct { libspectrum_word mask, value; } port;
  struct { int source; int page; int offset; } location;

  debugger_expression* exp;



/* Line 2053 of yacc.c  */
#line 149 "commandy.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_YY_COMMANDY_H_INCLUDED  */
