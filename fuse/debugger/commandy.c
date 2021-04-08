/* A Bison parser, made by GNU Bison 2.7.12-4996.  */

/* Bison implementation for Yacc-like parsers in C
   
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
#define YYBISON_VERSION "2.7.12-4996"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
/* Line 371 of yacc.c  */
#line 26 "commandy.y"


#include <config.h>

#include <stdio.h>		/* Needed by NetBSD yacc */
#include <stdlib.h>
#include <string.h>

#include "debugger.h"
#include "debugger_internals.h"
#include "mempool.h"
#include "ui/ui.h"
#include "z80/z80.h"
#include "z80/z80_macros.h"

#define YYDEBUG 1
#define YYERROR_VERBOSE


/* Line 371 of yacc.c  */
#line 88 "commandy.c"

# ifndef YY_NULL
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULL nullptr
#  else
#   define YY_NULL 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "y.tab.h".  */
#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
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
/* Line 387 of yacc.c  */
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



/* Line 387 of yacc.c  */
#line 223 "commandy.c"
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

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

/* Line 390 of yacc.c  */
#line 251 "commandy.c"

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
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef __attribute__
/* This feature is available in gcc versions 2.5 and later.  */
# if (! defined __GNUC__ || __GNUC__ < 2 \
      || (__GNUC__ == 2 && __GNUC_MINOR__ < 5))
#  define __attribute__(Spec) /* empty */
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif


/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(N) (N)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
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
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
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
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  43
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   209

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  47
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  15
/* YYNRULES -- Number of rules.  */
#define YYNRULES  65
/* YYNRULES -- Number of states.  */
#define YYNSTATES  115

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   292

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      43,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    40,     2,
      45,    46,     2,    41,     2,    42,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    44,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    39,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    38,     2,     2,     2,     2,     2,
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
      35,    36,    37
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     4,     6,     8,    12,    15,    20,    26,
      31,    38,    41,    47,    51,    53,    55,    58,    61,    63,
      65,    69,    71,    75,    78,    82,    86,    90,    92,    94,
      96,    97,    99,   101,   103,   107,   109,   115,   117,   119,
     120,   123,   124,   126,   127,   129,   131,   133,   135,   137,
     141,   144,   147,   150,   154,   158,   162,   166,   170,   174,
     178,   182,   186,   190,   192,   195
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      48,     0,    -1,    -1,    49,    -1,     1,    -1,    48,    43,
      49,    -1,     9,    58,    -1,    50,    51,    53,    55,    -1,
      50,    26,    54,    52,    55,    -1,    50,    31,    58,    55,
      -1,    50,    19,    35,    44,    35,    55,    -1,    12,    56,
      -1,    13,    58,    43,    60,    18,    -1,    14,    34,    57,
      -1,    15,    -1,    16,    -1,    16,    58,    -1,    17,    58,
      -1,    20,    -1,    21,    -1,    23,    34,    58,    -1,    24,
      -1,    25,    58,    34,    -1,    27,    58,    -1,    29,    34,
      58,    -1,    29,    33,    58,    -1,    29,    36,    58,    -1,
      30,    -1,    10,    -1,    11,    -1,    -1,    28,    -1,    32,
      -1,    58,    -1,    58,    44,    58,    -1,    56,    -1,    35,
      44,    58,    44,    58,    -1,    28,    -1,    32,    -1,    -1,
      22,    59,    -1,    -1,    58,    -1,    -1,    59,    -1,    59,
      -1,    34,    -1,    33,    -1,    36,    -1,    45,    59,    46,
      -1,    41,    59,    -1,    42,    59,    -1,     7,    59,    -1,
      59,    41,    59,    -1,    59,    42,    59,    -1,    59,     8,
      59,    -1,    59,     6,    59,    -1,    59,     5,    59,    -1,
      59,    40,    59,    -1,    59,    39,    59,    -1,    59,    38,
      59,    -1,    59,     4,    59,    -1,    59,     3,    59,    -1,
      61,    -1,    60,    61,    -1,    35,    43,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   147,   147,   148,   149,   150,   153,   154,   158,   163,
     167,   171,   172,   173,   176,   177,   178,   179,   180,   181,
     182,   185,   186,   187,   188,   189,   190,   191,   194,   195,
     198,   199,   200,   203,   204,   207,   208,   220,   221,   224,
     225,   228,   229,   232,   233,   236,   239,   242,   245,   248,
     249,   250,   254,   258,   262,   266,   270,   274,   278,   282,
     286,   290,   296,   304,   305,   310
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "LOGICAL_OR", "LOGICAL_AND",
  "COMPARISON", "EQUALITY", "NEGATE", "TIMES_DIVIDE", "BASE", "BREAK",
  "TBREAK", "CLEAR", "COMMANDS", "CONDITION", "CONTINUE",
  "DEBUGGER_DELETE", "DISASSEMBLE", "DEBUGGER_END", "EVENT", "EXIT",
  "FINISH", "IF", "DEBUGGER_IGNORE", "NEXT", "DEBUGGER_OUT", "PORT",
  "DEBUGGER_PRINT", "READ", "SET", "STEP", "TIME", "WRITE",
  "DEBUGGER_REGISTER", "NUMBER", "STRING", "VARIABLE", "DEBUGGER_ERROR",
  "'|'", "'^'", "'&'", "'+'", "'-'", "'\\n'", "':'", "'('", "')'",
  "$accept", "input", "command", "breakpointlife", "breakpointtype",
  "breakpointport", "breakpointlocation", "portbreakpointtype",
  "optionalcondition", "numberorpc", "expressionornull", "number",
  "expression", "debuggercommands", "debuggercommand", YY_NULL
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
     285,   286,   287,   288,   289,   290,   291,   292,   124,    94,
      38,    43,    45,    10,    58,    40,    41
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    47,    48,    48,    48,    48,    49,    49,    49,    49,
      49,    49,    49,    49,    49,    49,    49,    49,    49,    49,
      49,    49,    49,    49,    49,    49,    49,    49,    50,    50,
      51,    51,    51,    52,    52,    53,    53,    54,    54,    55,
      55,    56,    56,    57,    57,    58,    59,    59,    59,    59,
      59,    59,    59,    59,    59,    59,    59,    59,    59,    59,
      59,    59,    59,    60,    60,    61
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     1,     1,     3,     2,     4,     5,     4,
       6,     2,     5,     3,     1,     1,     2,     2,     1,     1,
       3,     1,     3,     2,     3,     3,     3,     1,     1,     1,
       0,     1,     1,     1,     3,     1,     5,     1,     1,     0,
       2,     0,     1,     0,     1,     1,     1,     1,     1,     3,
       2,     2,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     1,     2,     2
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     4,     0,    28,    29,    41,     0,     0,    14,    15,
       0,    18,    19,     0,    21,     0,     0,     0,    27,     0,
       3,    30,     0,    47,    46,    48,     0,     0,     0,     6,
      45,    11,    42,     0,    43,    16,    17,     0,     0,    23,
       0,     0,     0,     1,     0,     0,     0,    31,     0,    32,
      41,    52,    50,    51,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    13,    44,    20,    22,
      25,    24,    26,     5,     0,    37,    38,     0,    39,     0,
      39,    35,    49,    62,    61,    57,    56,    55,    60,    59,
      58,    53,    54,     0,     0,    63,     0,    39,    33,     0,
       9,     0,     7,    65,    12,    64,    39,     8,     0,    40,
       0,    10,    34,     0,    36
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,    19,    20,    21,    50,    97,    80,    77,   100,    31,
      66,    32,    30,    94,    95
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -76
static const yytype_int16 yypact[] =
{
      73,   -76,    88,   -76,   -76,    88,    88,   -25,   -76,    88,
      88,   -76,   -76,   -24,   -76,    88,    88,    31,   -76,     1,
     -76,   177,    88,   -76,   -76,   -76,    88,    88,    88,   -76,
     104,   -76,   -76,   -32,    88,   -76,   -76,    88,   -18,   -76,
      88,    88,    88,   -76,   170,   -17,   -26,   -76,    88,   -76,
      35,   -76,   -76,   -76,    20,    88,    88,    88,    88,    88,
      88,    88,    88,    88,    88,    -3,   -76,   104,   -76,   -76,
     -76,   -76,   -76,   -76,   -27,   -76,   -76,    88,     8,   -11,
       8,   -76,   -76,   109,   120,    -5,   115,   -76,   126,   130,
     147,    26,    26,     0,    -6,   -76,     6,     8,     3,    88,
     -76,    88,   -76,   -76,   -76,   -76,     8,   -76,    88,   104,
      19,   -76,   -76,    88,   -76
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -76,   -76,    28,   -76,   -76,   -76,   -76,   -76,   -75,    29,
     -76,    -2,    -7,   -76,   -49
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -3
static const yytype_int8 yytable[] =
{
      29,    43,    75,    59,    33,   102,    76,    35,    36,    34,
      37,    65,   104,    38,    39,    51,    69,    96,    74,    52,
      53,    54,   107,    55,    56,    57,    58,    67,    59,    93,
      99,   111,    93,   101,    59,    68,    63,    64,    70,    71,
      72,   106,    22,   103,    44,   105,    78,   108,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    60,    61,
      62,    63,    64,   113,    40,    41,    82,    42,    23,    24,
      79,    25,    73,    -2,     1,    98,    26,    27,     0,    81,
      28,     0,     2,     3,     4,     5,     6,     7,     8,     9,
      10,     0,   109,    11,    12,    22,    13,    14,    15,   110,
      16,     0,    17,    18,     0,     0,   112,    55,    56,    57,
      58,   114,    59,    56,    57,    58,    -2,    59,     0,     0,
      57,    23,    24,    59,    25,    57,    58,     0,    59,    26,
      27,    57,    58,    28,    59,    57,    58,     0,    59,     0,
       0,     0,    60,    61,    62,    63,    64,    60,    61,    62,
      63,    64,    57,    58,     0,    59,    63,    64,    60,    61,
      62,    63,    64,     0,     0,    61,    62,    63,    64,     0,
      62,    63,    64,     0,     0,     0,     0,     0,     0,     2,
       3,     4,     5,     6,     7,     8,     9,    10,    63,    64,
      11,    12,     0,    13,    14,    15,    45,    16,     0,    17,
      18,     0,     0,    46,     0,    47,     0,     0,    48,    49
};

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-76)))

#define yytable_value_is_error(Yytable_value) \
  YYID (0)

static const yytype_int8 yycheck[] =
{
       2,     0,    28,     8,     6,    80,    32,     9,    10,    34,
      34,    43,    18,    15,    16,    22,    34,    44,    35,    26,
      27,    28,    97,     3,     4,     5,     6,    34,     8,    35,
      22,   106,    35,    44,     8,    37,    41,    42,    40,    41,
      42,    35,     7,    43,    43,    94,    48,    44,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    38,    39,
      40,    41,    42,    44,    33,    34,    46,    36,    33,    34,
      35,    36,    44,     0,     1,    77,    41,    42,    -1,    50,
      45,    -1,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    -1,    99,    20,    21,     7,    23,    24,    25,   101,
      27,    -1,    29,    30,    -1,    -1,   108,     3,     4,     5,
       6,   113,     8,     4,     5,     6,    43,     8,    -1,    -1,
       5,    33,    34,     8,    36,     5,     6,    -1,     8,    41,
      42,     5,     6,    45,     8,     5,     6,    -1,     8,    -1,
      -1,    -1,    38,    39,    40,    41,    42,    38,    39,    40,
      41,    42,     5,     6,    -1,     8,    41,    42,    38,    39,
      40,    41,    42,    -1,    -1,    39,    40,    41,    42,    -1,
      40,    41,    42,    -1,    -1,    -1,    -1,    -1,    -1,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    41,    42,
      20,    21,    -1,    23,    24,    25,    19,    27,    -1,    29,
      30,    -1,    -1,    26,    -1,    28,    -1,    -1,    31,    32
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    20,    21,    23,    24,    25,    27,    29,    30,    48,
      49,    50,     7,    33,    34,    36,    41,    42,    45,    58,
      59,    56,    58,    58,    34,    58,    58,    34,    58,    58,
      33,    34,    36,     0,    43,    19,    26,    28,    31,    32,
      51,    59,    59,    59,    59,     3,     4,     5,     6,     8,
      38,    39,    40,    41,    42,    43,    57,    59,    58,    34,
      58,    58,    58,    49,    35,    28,    32,    54,    58,    35,
      53,    56,    46,    59,    59,    59,    59,    59,    59,    59,
      59,    59,    59,    35,    60,    61,    44,    52,    58,    22,
      55,    44,    55,    43,    18,    61,    35,    55,    44,    59,
      58,    55,    58,    44,    58
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
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))

/* Error token number */
#define YYTERROR	1
#define YYERRCODE	256


/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
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
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  YYUSE (yytype);
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
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
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
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
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

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULL, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULL;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULL, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
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

  YYUSE (yytype);
}




/* The lookahead symbol.  */
int yychar;


#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval YY_INITIAL_VALUE(yyval_default);

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
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
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
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
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

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
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
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

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
/* Line 1787 of yacc.c  */
#line 153 "commandy.y"
    { debugger_output_base = (yyvsp[(2) - (2)].integer); }
    break;

  case 7:
/* Line 1787 of yacc.c  */
#line 154 "commandy.y"
    {
             debugger_breakpoint_add_address( (yyvsp[(2) - (4)].bptype), (yyvsp[(3) - (4)].location).source, (yyvsp[(3) - (4)].location).page, (yyvsp[(3) - (4)].location).offset,
                                              0, (yyvsp[(1) - (4)].bplife), (yyvsp[(4) - (4)].exp) );
	   }
    break;

  case 8:
/* Line 1787 of yacc.c  */
#line 158 "commandy.y"
    {
	     int mask = (yyvsp[(4) - (5)].port).mask;
	     if( mask == 0 ) mask = ( (yyvsp[(4) - (5)].port).value < 0x100 ? 0x00ff : 0xffff );
	     debugger_breakpoint_add_port( (yyvsp[(3) - (5)].bptype), (yyvsp[(4) - (5)].port).value, mask, 0, (yyvsp[(1) - (5)].bplife), (yyvsp[(5) - (5)].exp) );
           }
    break;

  case 9:
/* Line 1787 of yacc.c  */
#line 163 "commandy.y"
    {
	     debugger_breakpoint_add_time( DEBUGGER_BREAKPOINT_TYPE_TIME,
					   (yyvsp[(3) - (4)].integer), 0, (yyvsp[(1) - (4)].bplife), (yyvsp[(4) - (4)].exp) );
	   }
    break;

  case 10:
/* Line 1787 of yacc.c  */
#line 167 "commandy.y"
    {
	     debugger_breakpoint_add_event( DEBUGGER_BREAKPOINT_TYPE_EVENT,
					    (yyvsp[(3) - (6)].string), (yyvsp[(5) - (6)].string), 0, (yyvsp[(1) - (6)].bplife), (yyvsp[(6) - (6)].exp) );
	   }
    break;

  case 11:
/* Line 1787 of yacc.c  */
#line 171 "commandy.y"
    { debugger_breakpoint_clear( (yyvsp[(2) - (2)].integer) ); }
    break;

  case 12:
/* Line 1787 of yacc.c  */
#line 172 "commandy.y"
    { debugger_breakpoint_set_commands( (yyvsp[(2) - (5)].integer), (yyvsp[(4) - (5)].string) ); }
    break;

  case 13:
/* Line 1787 of yacc.c  */
#line 173 "commandy.y"
    {
	     debugger_breakpoint_set_condition( (yyvsp[(2) - (3)].integer), (yyvsp[(3) - (3)].exp) );
           }
    break;

  case 14:
/* Line 1787 of yacc.c  */
#line 176 "commandy.y"
    { debugger_run(); }
    break;

  case 15:
/* Line 1787 of yacc.c  */
#line 177 "commandy.y"
    { debugger_breakpoint_remove_all(); }
    break;

  case 16:
/* Line 1787 of yacc.c  */
#line 178 "commandy.y"
    { debugger_breakpoint_remove( (yyvsp[(2) - (2)].integer) ); }
    break;

  case 17:
/* Line 1787 of yacc.c  */
#line 179 "commandy.y"
    { ui_debugger_disassemble( (yyvsp[(2) - (2)].integer) ); }
    break;

  case 18:
/* Line 1787 of yacc.c  */
#line 180 "commandy.y"
    { debugger_exit_emulator(); }
    break;

  case 19:
/* Line 1787 of yacc.c  */
#line 181 "commandy.y"
    { debugger_breakpoint_exit(); }
    break;

  case 20:
/* Line 1787 of yacc.c  */
#line 182 "commandy.y"
    {
	     debugger_breakpoint_ignore( (yyvsp[(2) - (3)].integer), (yyvsp[(3) - (3)].integer) );
	   }
    break;

  case 21:
/* Line 1787 of yacc.c  */
#line 185 "commandy.y"
    { debugger_next(); }
    break;

  case 22:
/* Line 1787 of yacc.c  */
#line 186 "commandy.y"
    { debugger_port_write( (yyvsp[(2) - (3)].integer), (yyvsp[(3) - (3)].integer) ); }
    break;

  case 23:
/* Line 1787 of yacc.c  */
#line 187 "commandy.y"
    { printf( "0x%x\n", (yyvsp[(2) - (2)].integer) ); }
    break;

  case 24:
/* Line 1787 of yacc.c  */
#line 188 "commandy.y"
    { debugger_poke( (yyvsp[(2) - (3)].integer), (yyvsp[(3) - (3)].integer) ); }
    break;

  case 25:
/* Line 1787 of yacc.c  */
#line 189 "commandy.y"
    { debugger_register_set( (yyvsp[(2) - (3)].reg), (yyvsp[(3) - (3)].integer) ); }
    break;

  case 26:
/* Line 1787 of yacc.c  */
#line 190 "commandy.y"
    { debugger_variable_set( (yyvsp[(2) - (3)].string), (yyvsp[(3) - (3)].integer) ); }
    break;

  case 27:
/* Line 1787 of yacc.c  */
#line 191 "commandy.y"
    { debugger_step(); }
    break;

  case 28:
/* Line 1787 of yacc.c  */
#line 194 "commandy.y"
    { (yyval.bplife) = DEBUGGER_BREAKPOINT_LIFE_PERMANENT; }
    break;

  case 29:
/* Line 1787 of yacc.c  */
#line 195 "commandy.y"
    { (yyval.bplife) = DEBUGGER_BREAKPOINT_LIFE_ONESHOT; }
    break;

  case 30:
/* Line 1787 of yacc.c  */
#line 198 "commandy.y"
    { (yyval.bptype) = DEBUGGER_BREAKPOINT_TYPE_EXECUTE; }
    break;

  case 31:
/* Line 1787 of yacc.c  */
#line 199 "commandy.y"
    { (yyval.bptype) = DEBUGGER_BREAKPOINT_TYPE_READ; }
    break;

  case 32:
/* Line 1787 of yacc.c  */
#line 200 "commandy.y"
    { (yyval.bptype) = DEBUGGER_BREAKPOINT_TYPE_WRITE; }
    break;

  case 33:
/* Line 1787 of yacc.c  */
#line 203 "commandy.y"
    { (yyval.port).mask = 0; (yyval.port).value = (yyvsp[(1) - (1)].integer); }
    break;

  case 34:
/* Line 1787 of yacc.c  */
#line 204 "commandy.y"
    { (yyval.port).mask = (yyvsp[(1) - (3)].integer); (yyval.port).value = (yyvsp[(3) - (3)].integer); }
    break;

  case 35:
/* Line 1787 of yacc.c  */
#line 207 "commandy.y"
    { (yyval.location).source = memory_source_any; (yyval.location).offset = (yyvsp[(1) - (1)].integer); }
    break;

  case 36:
/* Line 1787 of yacc.c  */
#line 208 "commandy.y"
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

  case 37:
/* Line 1787 of yacc.c  */
#line 220 "commandy.y"
    { (yyval.bptype) = DEBUGGER_BREAKPOINT_TYPE_PORT_READ; }
    break;

  case 38:
/* Line 1787 of yacc.c  */
#line 221 "commandy.y"
    { (yyval.bptype) = DEBUGGER_BREAKPOINT_TYPE_PORT_WRITE; }
    break;

  case 39:
/* Line 1787 of yacc.c  */
#line 224 "commandy.y"
    { (yyval.exp) = NULL; }
    break;

  case 40:
/* Line 1787 of yacc.c  */
#line 225 "commandy.y"
    { (yyval.exp) = (yyvsp[(2) - (2)].exp); }
    break;

  case 41:
/* Line 1787 of yacc.c  */
#line 228 "commandy.y"
    { (yyval.integer) = PC; }
    break;

  case 42:
/* Line 1787 of yacc.c  */
#line 229 "commandy.y"
    { (yyval.integer) = (yyvsp[(1) - (1)].integer); }
    break;

  case 43:
/* Line 1787 of yacc.c  */
#line 232 "commandy.y"
    { (yyval.exp) = NULL; }
    break;

  case 44:
/* Line 1787 of yacc.c  */
#line 233 "commandy.y"
    { (yyval.exp) = (yyvsp[(1) - (1)].exp); }
    break;

  case 45:
/* Line 1787 of yacc.c  */
#line 236 "commandy.y"
    { (yyval.integer) = debugger_expression_evaluate( (yyvsp[(1) - (1)].exp) ); }
    break;

  case 46:
/* Line 1787 of yacc.c  */
#line 239 "commandy.y"
    { (yyval.exp) = debugger_expression_new_number( (yyvsp[(1) - (1)].integer), debugger_memory_pool );
		       if( !(yyval.exp) ) YYABORT;
		     }
    break;

  case 47:
/* Line 1787 of yacc.c  */
#line 242 "commandy.y"
    { (yyval.exp) = debugger_expression_new_register( (yyvsp[(1) - (1)].reg), debugger_memory_pool );
			 if( !(yyval.exp) ) YYABORT;
		       }
    break;

  case 48:
/* Line 1787 of yacc.c  */
#line 245 "commandy.y"
    { (yyval.exp) = debugger_expression_new_variable( (yyvsp[(1) - (1)].string), debugger_memory_pool );
			 if( !(yyval.exp) ) YYABORT;
		       }
    break;

  case 49:
/* Line 1787 of yacc.c  */
#line 248 "commandy.y"
    { (yyval.exp) = (yyvsp[(2) - (3)].exp); }
    break;

  case 50:
/* Line 1787 of yacc.c  */
#line 249 "commandy.y"
    { (yyval.exp) = (yyvsp[(2) - (2)].exp); }
    break;

  case 51:
/* Line 1787 of yacc.c  */
#line 250 "commandy.y"
    {
	        (yyval.exp) = debugger_expression_new_unaryop( '-', (yyvsp[(2) - (2)].exp), debugger_memory_pool );
		if( !(yyval.exp) ) YYABORT;
	      }
    break;

  case 52:
/* Line 1787 of yacc.c  */
#line 254 "commandy.y"
    {
	        (yyval.exp) = debugger_expression_new_unaryop( (yyvsp[(1) - (2)].token), (yyvsp[(2) - (2)].exp), debugger_memory_pool );
		if( !(yyval.exp) ) YYABORT;
	      }
    break;

  case 53:
/* Line 1787 of yacc.c  */
#line 258 "commandy.y"
    {
	        (yyval.exp) = debugger_expression_new_binaryop( '+', (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp), debugger_memory_pool );
		if( !(yyval.exp) ) YYABORT;
	      }
    break;

  case 54:
/* Line 1787 of yacc.c  */
#line 262 "commandy.y"
    {
	        (yyval.exp) = debugger_expression_new_binaryop( '-', (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp), debugger_memory_pool );
		if( !(yyval.exp) ) YYABORT;
	      }
    break;

  case 55:
/* Line 1787 of yacc.c  */
#line 266 "commandy.y"
    {
	        (yyval.exp) = debugger_expression_new_binaryop( (yyvsp[(2) - (3)].token), (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp), debugger_memory_pool );
		if( !(yyval.exp) ) YYABORT;
	      }
    break;

  case 56:
/* Line 1787 of yacc.c  */
#line 270 "commandy.y"
    {
	        (yyval.exp) = debugger_expression_new_binaryop( (yyvsp[(2) - (3)].token), (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp), debugger_memory_pool );
		if( !(yyval.exp) ) YYABORT;
	      }
    break;

  case 57:
/* Line 1787 of yacc.c  */
#line 274 "commandy.y"
    {
	        (yyval.exp) = debugger_expression_new_binaryop( (yyvsp[(2) - (3)].token), (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp), debugger_memory_pool );
		if( !(yyval.exp) ) YYABORT;
	      }
    break;

  case 58:
/* Line 1787 of yacc.c  */
#line 278 "commandy.y"
    {
	        (yyval.exp) = debugger_expression_new_binaryop( '&', (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp), debugger_memory_pool );
		if( !(yyval.exp) ) YYABORT;
	      }
    break;

  case 59:
/* Line 1787 of yacc.c  */
#line 282 "commandy.y"
    {
	        (yyval.exp) = debugger_expression_new_binaryop( '^', (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp), debugger_memory_pool );
		if( !(yyval.exp) ) YYABORT;
	      }
    break;

  case 60:
/* Line 1787 of yacc.c  */
#line 286 "commandy.y"
    {
	        (yyval.exp) = debugger_expression_new_binaryop( '|', (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp), debugger_memory_pool );
		if( !(yyval.exp) ) YYABORT;
	      }
    break;

  case 61:
/* Line 1787 of yacc.c  */
#line 290 "commandy.y"
    {
	        (yyval.exp) = debugger_expression_new_binaryop(
		  DEBUGGER_TOKEN_LOGICAL_AND, (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp), debugger_memory_pool
                );
		if( !(yyval.exp) ) YYABORT;
	      }
    break;

  case 62:
/* Line 1787 of yacc.c  */
#line 296 "commandy.y"
    {
	        (yyval.exp) = debugger_expression_new_binaryop(
		  DEBUGGER_TOKEN_LOGICAL_OR, (yyvsp[(1) - (3)].exp), (yyvsp[(3) - (3)].exp), debugger_memory_pool
		);
		if( !(yyval.exp) ) YYABORT;
	      }
    break;

  case 63:
/* Line 1787 of yacc.c  */
#line 304 "commandy.y"
    { (yyval.string) = (yyvsp[(1) - (1)].string); }
    break;

  case 64:
/* Line 1787 of yacc.c  */
#line 305 "commandy.y"
    {
                      (yyval.string) = mempool_alloc( debugger_memory_pool, strlen( (yyvsp[(1) - (2)].string) ) + strlen( (yyvsp[(2) - (2)].string) ) + 2 );
                      sprintf( (yyval.string), "%s\n%s", (yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].string) );
                    }
    break;


/* Line 1787 of yacc.c  */
#line 1992 "commandy.c"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
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
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
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

  /* Else will try to reuse lookahead token after shifting the error
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
      if (!yypact_value_is_default (yyn))
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

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


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

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
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


/* Line 2050 of yacc.c  */
#line 312 "commandy.y"

