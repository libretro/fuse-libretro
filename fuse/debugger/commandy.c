/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



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
     BASE = 263,
     BREAK = 264,
     TBREAK = 265,
     CLEAR = 266,
     COMMANDS = 267,
     CONDITION = 268,
     CONTINUE = 269,
     DEBUGGER_DELETE = 270,
     DISASSEMBLE = 271,
     DEBUGGER_END = 272,
     EVENT = 273,
     EXIT = 274,
     FINISH = 275,
     IF = 276,
     DEBUGGER_IGNORE = 277,
     NEXT = 278,
     DEBUGGER_OUT = 279,
     PORT = 280,
     DEBUGGER_PRINT = 281,
     READ = 282,
     SET = 283,
     STEP = 284,
     TIME = 285,
     WRITE = 286,
     NUMBER = 287,
     STRING = 288,
     VARIABLE = 289,
     DEBUGGER_ERROR = 290
   };
#endif
/* Tokens.  */
#define LOGICAL_OR 258
#define LOGICAL_AND 259
#define COMPARISON 260
#define EQUALITY 261
#define NEGATE 262
#define BASE 263
#define BREAK 264
#define TBREAK 265
#define CLEAR 266
#define COMMANDS 267
#define CONDITION 268
#define CONTINUE 269
#define DEBUGGER_DELETE 270
#define DISASSEMBLE 271
#define DEBUGGER_END 272
#define EVENT 273
#define EXIT 274
#define FINISH 275
#define IF 276
#define DEBUGGER_IGNORE 277
#define NEXT 278
#define DEBUGGER_OUT 279
#define PORT 280
#define DEBUGGER_PRINT 281
#define READ 282
#define SET 283
#define STEP 284
#define TIME 285
#define WRITE 286
#define NUMBER 287
#define STRING 288
#define VARIABLE 289
#define DEBUGGER_ERROR 290




/* Copy the first part of user declarations.  */
#line 25 "debugger/commandy.y"


#include <config.h>

#include <stdio.h>		/* Needed by NetBSD yacc */
#include <stdlib.h>
#include <string.h>

#include "debugger/debugger.h"
#include "debugger/debugger_internals.h"
#include "mempool.h"
#include "ui/ui.h"
#include "z80/z80.h"
#include "z80/z80_macros.h"

#define YYDEBUG 1
#define YYERROR_VERBOSE



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 45 "debugger/commandy.y"
{

  int token;

  libspectrum_dword integer;
  char *string;

  debugger_breakpoint_type bptype;
  debugger_breakpoint_life bplife;
  struct { libspectrum_word mask, value; } port;
  struct { int source; int page; int offset; } location;

  debugger_expression* exp;

}
/* Line 193 of yacc.c.  */
#line 202 "debugger/commandy.c"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 215 "debugger/commandy.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  46
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   265

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  49
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  15
/* YYNRULES -- Number of rules.  */
#define YYNRULES  68
/* YYNRULES -- Number of states.  */
#define YYNSTATES  128

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   290

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      43,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    38,     2,
      45,    46,    41,    39,     2,    40,     2,    42,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    44,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    47,     2,    48,    37,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    36,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     4,     6,     8,    12,    15,    20,    26,
      31,    38,    45,    48,    54,    58,    60,    62,    65,    68,
      71,    73,    77,    79,    83,    86,    90,    94,   100,   102,
     104,   106,   107,   109,   111,   113,   117,   119,   125,   127,
     129,   130,   133,   134,   136,   137,   139,   141,   143,   147,
     149,   153,   157,   160,   163,   166,   170,   174,   178,   182,
     186,   190,   194,   198,   202,   206,   210,   212,   215
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      50,     0,    -1,    -1,    51,    -1,     1,    -1,    50,    43,
      51,    -1,     8,    60,    -1,    52,    53,    55,    57,    -1,
      52,    25,    56,    54,    57,    -1,    52,    30,    60,    57,
      -1,    52,    18,    33,    44,    33,    57,    -1,    52,    18,
      33,    44,    41,    57,    -1,    11,    58,    -1,    12,    60,
      43,    62,    17,    -1,    13,    32,    59,    -1,    14,    -1,
      15,    -1,    15,    60,    -1,    16,    60,    -1,    19,    59,
      -1,    20,    -1,    22,    32,    60,    -1,    23,    -1,    24,
      60,    32,    -1,    26,    60,    -1,    28,    32,    60,    -1,
      28,    34,    60,    -1,    28,    33,    44,    33,    60,    -1,
      29,    -1,     9,    -1,    10,    -1,    -1,    27,    -1,    31,
      -1,    60,    -1,    60,    44,    60,    -1,    58,    -1,    33,
      44,    60,    44,    60,    -1,    27,    -1,    31,    -1,    -1,
      21,    61,    -1,    -1,    60,    -1,    -1,    61,    -1,    61,
      -1,    32,    -1,    33,    44,    33,    -1,    34,    -1,    45,
      61,    46,    -1,    47,    61,    48,    -1,    39,    61,    -1,
      40,    61,    -1,     7,    61,    -1,    61,    39,    61,    -1,
      61,    40,    61,    -1,    61,    41,    61,    -1,    61,    42,
      61,    -1,    61,     6,    61,    -1,    61,     5,    61,    -1,
      61,    38,    61,    -1,    61,    37,    61,    -1,    61,    36,
      61,    -1,    61,     4,    61,    -1,    61,     3,    61,    -1,
      63,    -1,    62,    63,    -1,    33,    43,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   142,   142,   143,   144,   145,   148,   149,   153,   158,
     162,   166,   170,   171,   172,   175,   176,   177,   178,   179,
     180,   181,   184,   185,   186,   187,   188,   189,   190,   193,
     194,   197,   198,   199,   202,   203,   206,   207,   219,   220,
     223,   224,   227,   228,   231,   232,   235,   238,   241,   244,
     247,   248,   252,   253,   257,   261,   265,   269,   273,   277,
     281,   285,   289,   293,   297,   303,   311,   312,   317
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "LOGICAL_OR", "LOGICAL_AND",
  "COMPARISON", "EQUALITY", "NEGATE", "BASE", "BREAK", "TBREAK", "CLEAR",
  "COMMANDS", "CONDITION", "CONTINUE", "DEBUGGER_DELETE", "DISASSEMBLE",
  "DEBUGGER_END", "EVENT", "EXIT", "FINISH", "IF", "DEBUGGER_IGNORE",
  "NEXT", "DEBUGGER_OUT", "PORT", "DEBUGGER_PRINT", "READ", "SET", "STEP",
  "TIME", "WRITE", "NUMBER", "STRING", "VARIABLE", "DEBUGGER_ERROR", "'|'",
  "'^'", "'&'", "'+'", "'-'", "'*'", "'/'", "'\\n'", "':'", "'('", "')'",
  "'['", "']'", "$accept", "input", "command", "breakpointlife",
  "breakpointtype", "breakpointport", "breakpointlocation",
  "portbreakpointtype", "optionalcondition", "numberorpc",
  "expressionornull", "number", "expression", "debuggercommands",
  "debuggercommand", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   124,    94,    38,    43,
      45,    42,    47,    10,    58,    40,    41,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    49,    50,    50,    50,    50,    51,    51,    51,    51,
      51,    51,    51,    51,    51,    51,    51,    51,    51,    51,
      51,    51,    51,    51,    51,    51,    51,    51,    51,    52,
      52,    53,    53,    53,    54,    54,    55,    55,    56,    56,
      57,    57,    58,    58,    59,    59,    60,    61,    61,    61,
      61,    61,    61,    61,    61,    61,    61,    61,    61,    61,
      61,    61,    61,    61,    61,    61,    62,    62,    63
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     1,     1,     3,     2,     4,     5,     4,
       6,     6,     2,     5,     3,     1,     1,     2,     2,     2,
       1,     3,     1,     3,     2,     3,     3,     5,     1,     1,
       1,     0,     1,     1,     1,     3,     1,     5,     1,     1,
       0,     2,     0,     1,     0,     1,     1,     1,     3,     1,
       3,     3,     2,     2,     2,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     1,     2,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     4,     0,    29,    30,    42,     0,     0,    15,    16,
       0,    44,    20,     0,    22,     0,     0,     0,    28,     0,
       3,    31,     0,    47,     0,    49,     0,     0,     0,     0,
       6,    46,    12,    43,     0,    44,    17,    18,    19,    45,
       0,     0,    24,     0,     0,     0,     1,     0,     0,     0,
      32,     0,    33,    42,    54,     0,    52,    53,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    14,    21,    23,    25,     0,    26,     5,     0,
      38,    39,     0,    40,     0,    40,    36,    48,    50,    51,
      65,    64,    60,    59,    63,    62,    61,    55,    56,    57,
      58,     0,     0,    66,     0,     0,    40,    34,     0,     9,
       0,     7,    68,    13,    67,    27,    40,    40,     8,     0,
      41,    48,     0,    10,    11,    35,     0,    37
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,    19,    20,    21,    53,   106,    85,    82,   109,    32,
      38,    33,    31,   102,   103
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -31
static const yytype_int16 yypact[] =
{
      77,   -31,   121,   -31,   -31,   121,   121,   -23,   -31,   121,
     121,   121,   -31,   -17,   -31,   121,   121,     0,   -31,     2,
     -31,   234,   121,   -31,   -24,   -31,   121,   121,   121,   121,
     -31,   174,   -31,   -31,   -16,   121,   -31,   -31,   -31,   174,
     121,    -8,   -31,   121,   -18,   121,   -31,   227,     4,   -21,
     -31,   121,   -31,   124,   -31,     6,   -31,   -31,   106,    25,
     121,   121,   121,   121,   121,   121,   121,   121,   121,   121,
     121,     7,   -31,   -31,   -31,   -31,     9,   -31,   -31,    31,
     -31,   -31,   121,    23,    32,    23,   -31,   -31,   -31,   -31,
     186,   167,    98,    74,   192,    30,    42,   -19,   -19,   -31,
     -31,     3,   -12,   -31,   121,   -30,    23,    50,   121,   -31,
     142,   -31,   -31,   -31,   -31,   -31,    23,    23,   -31,   121,
     174,   -24,    51,   -31,   -31,   -31,   121,   -31
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -31,   -31,    27,   -31,   -31,   -31,   -31,   -31,    19,    54,
      83,    -2,   -10,   -31,    17
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -3
static const yytype_int8 yytable[] =
{
      30,    39,    46,   116,    34,   113,    80,    36,    37,    35,
      81,   117,    54,    41,    42,    40,    56,    57,    58,    59,
      55,   101,    69,    70,    74,    39,    76,    71,    60,    61,
      62,    63,    43,    44,    45,    62,    63,    79,    73,    87,
     101,    75,   104,    77,   108,    47,   112,    62,    63,    83,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,    64,    65,    66,    67,    68,    69,    70,    66,    67,
      68,    69,    70,    89,    78,   105,   110,    -2,     1,    62,
     107,    67,    68,    69,    70,     2,     3,     4,     5,     6,
       7,     8,     9,    10,   119,   126,    11,    12,   120,    13,
      14,    15,   115,    16,   111,    17,    18,    86,   122,    60,
      61,    62,    63,    67,    68,    69,    70,   125,    72,   114,
      -2,     0,     0,     0,   127,   118,     0,     0,    22,     0,
       0,    22,     0,     0,     0,   123,   124,    67,    68,    69,
      70,     0,    64,    65,    66,    67,    68,    69,    70,    22,
       0,     0,    88,    23,    24,    25,    23,    84,    25,     0,
      26,    27,     0,    26,    27,     0,    28,     0,    29,    28,
       0,    29,    62,    63,    23,   121,    25,    60,    61,    62,
      63,    26,    27,     0,     0,     0,     0,    28,     0,    29,
      61,    62,    63,     0,     0,     0,     0,    62,    63,     0,
       0,     0,     0,    64,    65,    66,    67,    68,    69,    70,
      64,    65,    66,    67,    68,    69,    70,     0,     0,     0,
       0,     0,    64,    65,    66,    67,    68,    69,    70,    65,
      66,    67,    68,    69,    70,     2,     3,     4,     5,     6,
       7,     8,     9,    10,     0,     0,    11,    12,     0,    13,
      14,    15,    48,    16,     0,    17,    18,     0,     0,    49,
       0,    50,     0,     0,    51,    52
};

static const yytype_int8 yycheck[] =
{
       2,    11,     0,    33,     6,    17,    27,     9,    10,    32,
      31,    41,    22,    15,    16,    32,    26,    27,    28,    29,
      44,    33,    41,    42,    32,    35,    44,    43,     3,     4,
       5,     6,    32,    33,    34,     5,     6,    33,    40,    33,
      33,    43,    33,    45,    21,    43,    43,     5,     6,    51,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    36,    37,    38,    39,    40,    41,    42,    38,    39,
      40,    41,    42,    48,    47,    44,    44,     0,     1,     5,
      82,    39,    40,    41,    42,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    44,    44,    19,    20,   108,    22,
      23,    24,   104,    26,    85,    28,    29,    53,   110,     3,
       4,     5,     6,    39,    40,    41,    42,   119,    35,   102,
      43,    -1,    -1,    -1,   126,   106,    -1,    -1,     7,    -1,
      -1,     7,    -1,    -1,    -1,   116,   117,    39,    40,    41,
      42,    -1,    36,    37,    38,    39,    40,    41,    42,     7,
      -1,    -1,    46,    32,    33,    34,    32,    33,    34,    -1,
      39,    40,    -1,    39,    40,    -1,    45,    -1,    47,    45,
      -1,    47,     5,     6,    32,    33,    34,     3,     4,     5,
       6,    39,    40,    -1,    -1,    -1,    -1,    45,    -1,    47,
       4,     5,     6,    -1,    -1,    -1,    -1,     5,     6,    -1,
      -1,    -1,    -1,    36,    37,    38,    39,    40,    41,    42,
      36,    37,    38,    39,    40,    41,    42,    -1,    -1,    -1,
      -1,    -1,    36,    37,    38,    39,    40,    41,    42,    37,
      38,    39,    40,    41,    42,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    -1,    -1,    19,    20,    -1,    22,
      23,    24,    18,    26,    -1,    28,    29,    -1,    -1,    25,
      -1,    27,    -1,    -1,    30,    31
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    19,    20,    22,    23,    24,    26,    28,    29,    50,
      51,    52,     7,    32,    33,    34,    39,    40,    45,    47,
      60,    61,    58,    60,    60,    32,    60,    60,    59,    61,
      32,    60,    60,    32,    33,    34,     0,    43,    18,    25,
      27,    30,    31,    53,    61,    44,    61,    61,    61,    61,
       3,     4,     5,     6,    36,    37,    38,    39,    40,    41,
      42,    43,    59,    60,    32,    60,    44,    60,    51,    33,
      27,    31,    56,    60,    33,    55,    58,    33,    46,    48,
      61,    61,    61,    61,    61,    61,    61,    61,    61,    61,
      61,    33,    62,    63,    33,    44,    54,    60,    21,    57,
      44,    57,    43,    17,    63,    60,    33,    41,    57,    44,
      61,    33,    60,    57,    57,    60,    44,    60
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

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



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 6:
#line 148 "debugger/commandy.y"
    { debugger_output_base = (yyvsp[(2) - (2)].integer); }
    break;

  case 7:
#line 149 "debugger/commandy.y"
    {
             debugger_breakpoint_add_address( (yyvsp[(2) - (4)].bptype), (yyvsp[(3) - (4)].location).source, (yyvsp[(3) - (4)].location).page, (yyvsp[(3) - (4)].location).offset,
                                              0, (yyvsp[(1) - (4)].bplife), (yyvsp[(4) - (4)].exp) );
	   }
    break;

  case 8:
#line 153 "debugger/commandy.y"
    {
	     int mask = (yyvsp[(4) - (5)].port).mask;
	     if( mask == 0 ) mask = ( (yyvsp[(4) - (5)].port).value < 0x100 ? 0x00ff : 0xffff );
	     debugger_breakpoint_add_port( (yyvsp[(3) - (5)].bptype), (yyvsp[(4) - (5)].port).value, mask, 0, (yyvsp[(1) - (5)].bplife), (yyvsp[(5) - (5)].exp) );
           }
    break;

  case 9:
#line 158 "debugger/commandy.y"
    {
	     debugger_breakpoint_add_time( DEBUGGER_BREAKPOINT_TYPE_TIME,
					   (yyvsp[(3) - (4)].integer), 0, (yyvsp[(1) - (4)].bplife), (yyvsp[(4) - (4)].exp) );
	   }
    break;

  case 10:
#line 162 "debugger/commandy.y"
    {
	     debugger_breakpoint_add_event( DEBUGGER_BREAKPOINT_TYPE_EVENT,
					    (yyvsp[(3) - (6)].string), (yyvsp[(5) - (6)].string), 0, (yyvsp[(1) - (6)].bplife), (yyvsp[(6) - (6)].exp) );
	   }
    break;

  case 11:
#line 166 "debugger/commandy.y"
    {
	     debugger_breakpoint_add_event( DEBUGGER_BREAKPOINT_TYPE_EVENT,
					    (yyvsp[(3) - (6)].string), "*", 0, (yyvsp[(1) - (6)].bplife), (yyvsp[(6) - (6)].exp) );
	   }
    break;

  case 12:
#line 170 "debugger/commandy.y"
    { debugger_breakpoint_clear( (yyvsp[(2) - (2)].integer) ); }
    break;

  case 13:
#line 171 "debugger/commandy.y"
    { debugger_breakpoint_set_commands( (yyvsp[(2) - (5)].integer), (yyvsp[(4) - (5)].string) ); }
    break;

  case 14:
#line 172 "debugger/commandy.y"
    {
	     debugger_breakpoint_set_condition( (yyvsp[(2) - (3)].integer), (yyvsp[(3) - (3)].exp) );
           }
    break;

  case 15:
#line 175 "debugger/commandy.y"
    { debugger_run(); }
    break;

  case 16:
#line 176 "debugger/commandy.y"
    { debugger_breakpoint_remove_all(); }
    break;

  case 17:
#line 177 "debugger/commandy.y"
    { debugger_breakpoint_remove( (yyvsp[(2) - (2)].integer) ); }
    break;

  case 18:
#line 178 "debugger/commandy.y"
    { ui_debugger_disassemble( (yyvsp[(2) - (2)].integer) ); }
    break;

  case 19:
#line 179 "debugger/commandy.y"
    { debugger_exit_emulator( (yyvsp[(2) - (2)].exp) ); }
    break;

  case 20:
#line 180 "debugger/commandy.y"
    { debugger_breakpoint_exit(); }
    break;

  case 21:
#line 181 "debugger/commandy.y"
    {
	     debugger_breakpoint_ignore( (yyvsp[(2) - (3)].integer), (yyvsp[(3) - (3)].integer) );
	   }
    break;

  case 22:
#line 184 "debugger/commandy.y"
    { debugger_next(); }
    break;

  case 23:
#line 185 "debugger/commandy.y"
    { debugger_port_write( (yyvsp[(2) - (3)].integer), (yyvsp[(3) - (3)].integer) ); }
    break;

  case 24:
#line 186 "debugger/commandy.y"
    { printf( "0x%x\n", (yyvsp[(2) - (2)].integer) ); }
    break;

  case 25:
#line 187 "debugger/commandy.y"
    { debugger_poke( (yyvsp[(2) - (3)].integer), (yyvsp[(3) - (3)].integer) ); }
    break;

  case 26:
#line 188 "debugger/commandy.y"
    { debugger_variable_set( (yyvsp[(2) - (3)].string), (yyvsp[(3) - (3)].integer) ); }
    break;

  case 27:
#line 189 "debugger/commandy.y"
    { debugger_system_variable_set( (yyvsp[(2) - (5)].string), (yyvsp[(4) - (5)].string), (yyvsp[(5) - (5)].integer) ); }
    break;

  case 28:
#line 190 "debugger/commandy.y"
    { debugger_step(); }
    break;

  case 29:
#line 193 "debugger/commandy.y"
    { (yyval.bplife) = DEBUGGER_BREAKPOINT_LIFE_PERMANENT; }
    break;

  case 30:
#line 194 "debugger/commandy.y"
    { (yyval.bplife) = DEBUGGER_BREAKPOINT_LIFE_ONESHOT; }
    break;

  case 31:
#line 197 "debugger/commandy.y"
    { (yyval.bptype) = DEBUGGER_BREAKPOINT_TYPE_EXECUTE; }
    break;

  case 32:
#line 198 "debugger/commandy.y"
    { (yyval.bptype) = DEBUGGER_BREAKPOINT_TYPE_READ; }
    break;

  case 33:
#line 199 "debugger/commandy.y"
    { (yyval.bptype) = DEBUGGER_BREAKPOINT_TYPE_WRITE; }
    break;

  case 34:
#line 202 "debugger/commandy.y"
    { (yyval.port).mask = 0; (yyval.port).value = (yyvsp[(1) - (1)].integer); }
    break;

  case 35:
#line 203 "debugger/commandy.y"
    { (yyval.port).mask = (yyvsp[(1) - (3)].integer); (yyval.port).value = (yyvsp[(3) - (3)].integer); }
    break;

  case 36:
#line 206 "debugger/commandy.y"
    { (yyval.location).source = memory_source_any; (yyval.location).offset = (yyvsp[(1) - (1)].integer); }
    break;

  case 37:
#line 207 "debugger/commandy.y"
    {
                        (yyval.location).source = memory_source_find( (yyvsp[(1) - (5)].string) );
                        if( (yyval.location).source == -1 ) {
                          char buffer[80];
                          snprintf( buffer, 80, "unknown memory source \"%s\"", (yyvsp[(1) - (5)].string) );
                          yyerror( buffer );
                          YYERROR;
                        }
                        (yyval.location).page = (yyvsp[(3) - (5)].integer);
                        (yyval.location).offset = (yyvsp[(5) - (5)].integer);
                      }
    break;

  case 38:
#line 219 "debugger/commandy.y"
    { (yyval.bptype) = DEBUGGER_BREAKPOINT_TYPE_PORT_READ; }
    break;

  case 39:
#line 220 "debugger/commandy.y"
    { (yyval.bptype) = DEBUGGER_BREAKPOINT_TYPE_PORT_WRITE; }
    break;

  case 40:
#line 223 "debugger/commandy.y"
    { (yyval.exp) = NULL; }
    break;

  case 41:
#line 224 "debugger/commandy.y"
    { (yyval.exp) = (yyvsp[(2) - (2)].exp); }
    break;

  case 42:
#line 227 "debugger/commandy.y"
    { (yyval.integer) = PC; }
    break;

  case 43:
#line 228 "debugger/commandy.y"
    { (yyval.integer) = (yyvsp[(1) - (1)].integer); }
    break;

  case 44:
#line 231 "debugger/commandy.y"
    { (yyval.exp) = NULL; }
    break;

  case 45:
#line 232 "debugger/commandy.y"
    { (yyval.exp) = (yyvsp[(1) - (1)].exp); }
    break;

  case 46:
#line 235 "debugger/commandy.y"
    { (yyval.integer) = debugger_expression_evaluate( (yyvsp[(1) - (1)].exp) ); }
    break;

  case 47:
#line 238 "debugger/commandy.y"
    { (yyval.exp) = debugger_expression_new_number( (yyvsp[(1) - (1)].integer), debugger_memory_pool );
		       if( !(yyval.exp) ) YYABORT;
		     }
    break;

  case 48:
#line 241 "debugger/commandy.y"
    { (yyval.exp) = debugger_expression_new_system_variable( (yyvsp[(1) - (3)].string), (yyvsp[(3) - (3)].string), debugger_memory_pool );
                                  if( !(yyval.exp) ) YYABORT;
                                }
    break;

  case 49:
#line 244 "debugger/commandy.y"
    { (yyval.exp) = debugger_expression_new_variable( (yyvsp[(1) - (1)].string), debugger_memory_pool );
			 if( !(yyval.exp) ) YYABORT;
		       }
    break;

  case 50:
#line 247 "debugger/commandy.y"
    { (yyval.exp) = (yyvsp[(2) - (3)].exp); }
    break;

  case 51:
#line 248 "debugger/commandy.y"
    {
                (yyval.exp) = debugger_expression_new_unaryop( DEBUGGER_TOKEN_DEREFERENCE, (yyvsp[(2) - (3)].exp), debugger_memory_pool );
                if( !(yyval.exp) ) YYABORT;
              }
    break;

  case 52:
#line 252 "debugger/commandy.y"
    { (yyval.exp) = (yyvsp[(2) - (2)].exp); }
    break;

  case 53:
#line 253 "debugger/commandy.y"
    {
	        (yyval.exp) = debugger_expression_new_unaryop( '-', (yyvsp[(2) - (2)].exp), debugger_memory_pool );
		if( !(yyval.exp) ) YYABORT;
	      }
    break;

  case 54:
#line 257 "debugger/commandy.y"
    {
	        (yyval.exp) = debugger_expression_new_unaryop( (yyvsp[(1) - (2)].token), (yyvsp[(2) - (2)].exp), debugger_memory_pool );
		if( !(yyval.exp) ) YYABORT;
	      }
    break;

  case 55:
#line 261 "debugger/commandy.y"
    {
	        (yyval.exp) = debugger_expression_new_binaryop( '+', (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp), debugger_memory_pool );
		if( !(yyval.exp) ) YYABORT;
	      }
    break;

  case 56:
#line 265 "debugger/commandy.y"
    {
	        (yyval.exp) = debugger_expression_new_binaryop( '-', (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp), debugger_memory_pool );
		if( !(yyval.exp) ) YYABORT;
	      }
    break;

  case 57:
#line 269 "debugger/commandy.y"
    {
	        (yyval.exp) = debugger_expression_new_binaryop( '*', (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp), debugger_memory_pool );
		if( !(yyval.exp) ) YYABORT;
	      }
    break;

  case 58:
#line 273 "debugger/commandy.y"
    {
	        (yyval.exp) = debugger_expression_new_binaryop( '/', (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp), debugger_memory_pool );
		if( !(yyval.exp) ) YYABORT;
	      }
    break;

  case 59:
#line 277 "debugger/commandy.y"
    {
	        (yyval.exp) = debugger_expression_new_binaryop( (yyvsp[(2) - (3)].token), (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp), debugger_memory_pool );
		if( !(yyval.exp) ) YYABORT;
	      }
    break;

  case 60:
#line 281 "debugger/commandy.y"
    {
	        (yyval.exp) = debugger_expression_new_binaryop( (yyvsp[(2) - (3)].token), (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp), debugger_memory_pool );
		if( !(yyval.exp) ) YYABORT;
	      }
    break;

  case 61:
#line 285 "debugger/commandy.y"
    {
	        (yyval.exp) = debugger_expression_new_binaryop( '&', (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp), debugger_memory_pool );
		if( !(yyval.exp) ) YYABORT;
	      }
    break;

  case 62:
#line 289 "debugger/commandy.y"
    {
	        (yyval.exp) = debugger_expression_new_binaryop( '^', (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp), debugger_memory_pool );
		if( !(yyval.exp) ) YYABORT;
	      }
    break;

  case 63:
#line 293 "debugger/commandy.y"
    {
	        (yyval.exp) = debugger_expression_new_binaryop( '|', (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp), debugger_memory_pool );
		if( !(yyval.exp) ) YYABORT;
	      }
    break;

  case 64:
#line 297 "debugger/commandy.y"
    {
	        (yyval.exp) = debugger_expression_new_binaryop(
		  DEBUGGER_TOKEN_LOGICAL_AND, (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp), debugger_memory_pool
                );
		if( !(yyval.exp) ) YYABORT;
	      }
    break;

  case 65:
#line 303 "debugger/commandy.y"
    {
	        (yyval.exp) = debugger_expression_new_binaryop(
		  DEBUGGER_TOKEN_LOGICAL_OR, (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp), debugger_memory_pool
		);
		if( !(yyval.exp) ) YYABORT;
	      }
    break;

  case 66:
#line 311 "debugger/commandy.y"
    { (yyval.string) = (yyvsp[(1) - (1)].string); }
    break;

  case 67:
#line 312 "debugger/commandy.y"
    {
                      (yyval.string) = mempool_new( debugger_memory_pool, char, strlen( (yyvsp[(1) - (2)].string) ) + strlen( (yyvsp[(2) - (2)].string) ) + 2 );
                      sprintf( (yyval.string), "%s\n%s", (yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].string) );
                    }
    break;


/* Line 1267 of yacc.c.  */
#line 1942 "debugger/commandy.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 319 "debugger/commandy.y"


