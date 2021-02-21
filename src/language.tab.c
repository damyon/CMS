/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

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
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 3 "language.y" /* yacc.c:339  */

#include <stddef.h>
#include "parser.h"
#define YYSTYPE ParseTree *
#define YYPARSE_PARAM param
#define YYLEX_PARAM param
#include "bison.h"  /* Contains definition of `parser types'        */


void yyerror (void *param, char const *msg);
extern int yylineno;

#line 79 "language.tab.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* In a future release of Bison, this section will be replaced
   by #include "language.tab.h".  */
#ifndef YY_YY_LANGUAGE_TAB_H_INCLUDED
# define YY_YY_LANGUAGE_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    FLOAT = 258,
    INT = 259,
    STRING = 260,
    ASSIGNMENT_OP = 261,
    VAR = 262,
    TYPE = 263,
    SEMICOLON = 264,
    COMMA = 265,
    ADD = 266,
    SUBTRACT = 267,
    MULTIPLY = 268,
    DIVIDE = 269,
    POWER = 270,
    MOD = 271,
    NEGATIVE = 272,
    NOT = 273,
    UNI_OPERATOR = 274,
    OPEN_SCOPE = 275,
    CLOSE_SCOPE = 276,
    OPEN_PAREN = 277,
    CLOSE_PAREN = 278,
    OPEN_ARRAY_INDEX = 279,
    CLOSE_ARRAY_INDEX = 280,
    FUNCTION_KEYWORD = 281,
    RETURN_KEYWORD = 282,
    IF_KEYWORD = 283,
    ELSE_KEYWORD = 284,
    WHILE_KEYWORD = 285,
    DO_KEYWORD = 286,
    FOR_KEYWORD = 287,
    EQUAL = 288,
    NOTEQUAL = 289,
    GREATERTHAN = 290,
    LESSTHAN = 291,
    GREATERTHANEQUAL = 292,
    LESSTHANEQUAL = 293,
    AND = 294,
    OR = 295
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void *param);

#endif /* !YY_YY_LANGUAGE_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 171 "language.tab.c" /* yacc.c:358  */

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
#else
typedef signed char yytype_int8;
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
# elif ! defined YYSIZE_T
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

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
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
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
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
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

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
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   547

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  41
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  13
/* YYNRULES -- Number of rules.  */
#define YYNRULES  63
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  135

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   295

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
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
      35,    36,    37,    38,    39,    40
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,    60,    60,    65,    72,    79,    82,    91,    97,   104,
     110,   115,   124,   135,   144,   155,   166,   177,   190,   203,
     214,   225,   234,   245,   250,   259,   264,   274,   286,   289,
     293,   301,   309,   317,   327,   331,   338,   347,   352,   362,
     367,   372,   377,   382,   389,   396,   403,   410,   417,   424,
     431,   438,   444,   450,   457,   464,   471,   478,   485,   492,
     499,   505,   511,   516
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "FLOAT", "INT", "STRING",
  "ASSIGNMENT_OP", "VAR", "TYPE", "SEMICOLON", "COMMA", "ADD", "SUBTRACT",
  "MULTIPLY", "DIVIDE", "POWER", "MOD", "NEGATIVE", "NOT", "UNI_OPERATOR",
  "OPEN_SCOPE", "CLOSE_SCOPE", "OPEN_PAREN", "CLOSE_PAREN",
  "OPEN_ARRAY_INDEX", "CLOSE_ARRAY_INDEX", "FUNCTION_KEYWORD",
  "RETURN_KEYWORD", "IF_KEYWORD", "ELSE_KEYWORD", "WHILE_KEYWORD",
  "DO_KEYWORD", "FOR_KEYWORD", "EQUAL", "NOTEQUAL", "GREATERTHAN",
  "LESSTHAN", "GREATERTHANEQUAL", "LESSTHANEQUAL", "AND", "OR", "$accept",
  "input", "block", "statement", "var_list", "var_list_ele",
  "function_decl", "param_list", "param_list_ele", "function_call",
  "exp_list", "variable", "exp", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295
};
# endif

#define YYPACT_NINF -27

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-27)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     -27,   120,   -27,   -27,   -27,   -27,   -27,   -12,     9,     4,
       4,    12,   -27,     4,    17,     4,    10,    13,   281,    15,
     -27,   -27,   -27,   -27,    -4,   176,     4,    46,    18,   -27,
      25,    25,   -27,    29,   152,   303,    59,   185,     4,     4,
      53,    66,     4,     4,   -27,     4,   -27,     4,     4,     4,
       4,     4,     4,     4,     4,     4,     4,     4,     4,     4,
       4,     2,   257,     4,   -27,     9,   -27,   -27,   106,   -27,
     333,   363,    82,    94,   217,   494,   393,    96,    40,    40,
     128,   507,    25,    32,    63,   167,    52,   137,   -26,    78,
     -27,     4,   -27,     4,   -27,   494,   -27,   115,     8,   -27,
     281,   281,     4,     4,     4,   -27,   494,   494,   -27,   106,
     121,    97,   114,   -27,   -27,   423,   453,   226,   -27,   -27,
     281,   281,   140,   145,     4,   -27,   -27,   -27,   -27,   -27,
     -27,   483,   281,   -27,   -27
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     1,     5,    40,    42,    39,    37,     0,     0,
       0,     0,     2,     0,     0,     0,     0,     0,     0,     0,
       4,     3,    10,    62,    41,     0,    34,    25,     0,    23,
      51,    52,    37,    60,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    61,     0,     7,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     9,     0,     6,    63,    28,     8,
       0,     0,     0,     0,     0,    43,     0,    50,    55,    54,
      56,    57,    53,    44,    45,    48,    46,    49,    47,    58,
      59,     0,    32,     0,    33,    26,    24,     0,     0,    29,
       0,     0,     0,     0,     0,    38,    36,    35,    31,     0,
       0,    13,    11,    21,    22,     0,     0,     0,    30,    27,
       0,     0,     0,     0,     0,    14,    16,    15,    12,    19,
      20,     0,     0,    17,    18
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -27,   146,   -13,   -15,   -27,   116,   -27,   -27,    77,   -27,
     -27,   177,    -9
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     1,    20,    21,    28,    29,    22,    98,    99,    23,
      61,    24,    25
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      30,    31,    43,    41,    35,    40,    37,     4,     5,     6,
      26,     7,    91,    59,    60,    44,    27,    62,   109,    32,
      45,     9,    10,    11,    36,    92,    13,    64,    65,    70,
      71,   110,    38,    74,    75,    39,    76,    42,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,    88,
      89,    90,    63,    45,    95,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    54,    55,    56,    57,
      58,    59,    60,    53,    54,    55,    56,    57,    58,    59,
      60,    68,   106,    72,   107,   112,   114,   111,   113,    57,
      58,    59,    60,   115,   116,   117,    73,   119,    55,    56,
      57,    58,    59,    60,   102,   126,   128,   125,   127,    48,
      49,    50,    51,    52,    97,   131,   103,   134,    60,   133,
       2,     3,   108,     4,     5,     6,   120,     7,     8,    53,
      54,    55,    56,    57,    58,    59,    60,     9,    10,    11,
      12,    12,    13,   121,    51,    52,    14,    15,    16,   129,
      17,    18,    19,     3,   130,     4,     5,     6,    34,     7,
       8,    53,    54,    55,    56,    57,    58,    59,    60,     9,
      10,    11,    12,    66,    13,    58,    59,    60,    14,    15,
      16,    96,    17,    18,    19,    46,   118,    47,    33,    48,
      49,    50,    51,    52,    69,     0,    47,     0,    48,    49,
      50,    51,    52,    56,    57,    58,    59,    60,     0,    53,
      54,    55,    56,    57,    58,    59,    60,     0,    53,    54,
      55,    56,    57,    58,    59,    60,   104,     0,    47,     0,
      48,    49,    50,    51,    52,   124,     0,    47,     0,    48,
      49,    50,    51,    52,     0,     0,     0,     0,     0,     0,
      53,    54,    55,    56,    57,    58,    59,    60,     0,    53,
      54,    55,    56,    57,    58,    59,    60,    93,    47,     0,
      48,    49,    50,    51,    52,     0,     0,     0,     0,     0,
      94,     0,     0,     0,     4,     5,     6,     0,     7,     8,
      53,    54,    55,    56,    57,    58,    59,    60,     9,    10,
      11,    12,     0,    13,     0,     0,     0,    14,    15,    16,
       0,    17,    18,    19,    47,     0,    48,    49,    50,    51,
      52,     0,     0,     0,     0,     0,    67,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    53,    54,    55,    56,
      57,    58,    59,    60,    47,     0,    48,    49,    50,    51,
      52,     0,     0,     0,     0,     0,   100,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    53,    54,    55,    56,
      57,    58,    59,    60,    47,     0,    48,    49,    50,    51,
      52,     0,     0,     0,     0,     0,   101,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    53,    54,    55,    56,
      57,    58,    59,    60,    47,     0,    48,    49,    50,    51,
      52,     0,     0,     0,     0,     0,     0,     0,   105,     0,
       0,     0,     0,     0,     0,     0,    53,    54,    55,    56,
      57,    58,    59,    60,    47,     0,    48,    49,    50,    51,
      52,     0,     0,     0,     0,     0,   122,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    53,    54,    55,    56,
      57,    58,    59,    60,    47,     0,    48,    49,    50,    51,
      52,     0,     0,     0,     0,     0,   123,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    53,    54,    55,    56,
      57,    58,    59,    60,    47,     0,    48,    49,    50,    51,
      52,     0,     0,     0,     0,    47,   132,    48,    49,    50,
      51,    52,     0,     0,     0,     0,    53,    54,    55,    56,
      57,    58,    59,    60,    52,     0,     0,    53,    54,    55,
      56,    57,    58,    59,    60,     0,     0,     0,     0,     0,
      53,    54,    55,    56,    57,    58,    59,    60
};

static const yytype_int16 yycheck[] =
{
       9,    10,     6,    18,    13,    18,    15,     3,     4,     5,
      22,     7,    10,    39,    40,    19,     7,    26,    10,     7,
      24,    17,    18,    19,     7,    23,    22,     9,    10,    38,
      39,    23,    22,    42,    43,    22,    45,    22,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,     6,    24,    63,    15,    16,    17,    33,    34,
      35,    36,    37,    38,    39,    40,    34,    35,    36,    37,
      38,    39,    40,    33,    34,    35,    36,    37,    38,    39,
      40,    22,    91,    30,    93,   100,   101,   100,   101,    37,
      38,    39,    40,   102,   103,   104,    30,   110,    35,    36,
      37,    38,    39,    40,    22,   120,   121,   120,   121,    13,
      14,    15,    16,    17,     8,   124,    22,   132,    40,   132,
       0,     1,     7,     3,     4,     5,    29,     7,     8,    33,
      34,    35,    36,    37,    38,    39,    40,    17,    18,    19,
      20,    20,    22,    29,    16,    17,    26,    27,    28,     9,
      30,    31,    32,     1,     9,     3,     4,     5,    12,     7,
       8,    33,    34,    35,    36,    37,    38,    39,    40,    17,
      18,    19,    20,    21,    22,    38,    39,    40,    26,    27,
      28,    65,    30,    31,    32,     9,   109,    11,    11,    13,
      14,    15,    16,    17,     9,    -1,    11,    -1,    13,    14,
      15,    16,    17,    36,    37,    38,    39,    40,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    -1,    33,    34,
      35,    36,    37,    38,    39,    40,     9,    -1,    11,    -1,
      13,    14,    15,    16,    17,     9,    -1,    11,    -1,    13,
      14,    15,    16,    17,    -1,    -1,    -1,    -1,    -1,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    -1,    33,
      34,    35,    36,    37,    38,    39,    40,    10,    11,    -1,
      13,    14,    15,    16,    17,    -1,    -1,    -1,    -1,    -1,
      23,    -1,    -1,    -1,     3,     4,     5,    -1,     7,     8,
      33,    34,    35,    36,    37,    38,    39,    40,    17,    18,
      19,    20,    -1,    22,    -1,    -1,    -1,    26,    27,    28,
      -1,    30,    31,    32,    11,    -1,    13,    14,    15,    16,
      17,    -1,    -1,    -1,    -1,    -1,    23,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    11,    -1,    13,    14,    15,    16,
      17,    -1,    -1,    -1,    -1,    -1,    23,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    11,    -1,    13,    14,    15,    16,
      17,    -1,    -1,    -1,    -1,    -1,    23,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    11,    -1,    13,    14,    15,    16,
      17,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    11,    -1,    13,    14,    15,    16,
      17,    -1,    -1,    -1,    -1,    -1,    23,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    11,    -1,    13,    14,    15,    16,
      17,    -1,    -1,    -1,    -1,    -1,    23,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    11,    -1,    13,    14,    15,    16,
      17,    -1,    -1,    -1,    -1,    11,    23,    13,    14,    15,
      16,    17,    -1,    -1,    -1,    -1,    33,    34,    35,    36,
      37,    38,    39,    40,    17,    -1,    -1,    33,    34,    35,
      36,    37,    38,    39,    40,    -1,    -1,    -1,    -1,    -1,
      33,    34,    35,    36,    37,    38,    39,    40
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    42,     0,     1,     3,     4,     5,     7,     8,    17,
      18,    19,    20,    22,    26,    27,    28,    30,    31,    32,
      43,    44,    47,    50,    52,    53,    22,     7,    45,    46,
      53,    53,     7,    52,    42,    53,     7,    53,    22,    22,
      43,    44,    22,     6,    19,    24,     9,    11,    13,    14,
      15,    16,    17,    33,    34,    35,    36,    37,    38,    39,
      40,    51,    53,     6,     9,    10,    21,    23,    22,     9,
      53,    53,    30,    30,    53,    53,    53,    53,    53,    53,
      53,    53,    53,    53,    53,    53,    53,    53,    53,    53,
      53,    10,    23,    10,    23,    53,    46,     8,    48,    49,
      23,    23,    22,    22,     9,    25,    53,    53,     7,    10,
      23,    43,    44,    43,    44,    53,    53,    53,    49,    43,
      29,    29,    23,    23,     9,    43,    44,    43,    44,     9,
       9,    53,    23,    43,    44
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    41,    42,    42,    42,    42,    43,    44,    44,    44,
      44,    44,    44,    44,    44,    44,    44,    44,    44,    44,
      44,    44,    44,    45,    45,    46,    46,    47,    48,    48,
      48,    49,    50,    50,    51,    51,    51,    52,    52,    53,
      53,    53,    53,    53,    53,    53,    53,    53,    53,    53,
      53,    53,    53,    53,    53,    53,    53,    53,    53,    53,
      53,    53,    53,    53
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     2,     3,     2,     3,     3,
       1,     5,     7,     5,     7,     7,     7,     9,     9,     7,
       7,     5,     5,     1,     3,     1,     3,     6,     0,     1,
       3,     2,     4,     4,     0,     3,     3,     1,     4,     1,
       1,     1,     1,     3,     3,     3,     3,     3,     3,     3,
       3,     2,     2,     3,     3,     3,     3,     3,     3,     3,
       2,     2,     1,     3
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


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
      yyerror (param, YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, param); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, void *param)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (param);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, void *param)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, param);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule, void *param)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              , param);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, param); \
} while (0)

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
#ifndef YYINITDEPTH
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
static YYSIZE_T
yystrlen (const char *yystr)
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
static char *
yystpcpy (char *yydest, const char *yysrc)
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
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
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
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
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

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, void *param)
{
  YYUSE (yyvaluep);
  YYUSE (param);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void *param)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

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
      yychar = yylex (param);
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
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 60 "language.y" /* yacc.c:1646  */
    { 
		(yyval) = initParseTree(INPUT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		((ParseParam *) param)->ptree = (yyval);
		}
#line 1426 "language.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 65 "language.y" /* yacc.c:1646  */
    { 
		(yyval) = initParseTree(INPUT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		((ParseParam *) param)->ptree = (yyval);
		}
#line 1438 "language.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 72 "language.y" /* yacc.c:1646  */
    { 
		(yyval) = initParseTree(INPUT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		((ParseParam *) param)->ptree = (yyval);
		}
#line 1450 "language.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 79 "language.y" /* yacc.c:1646  */
    { ((ParseParam *) param)->lineno = yylineno; }
#line 1456 "language.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 82 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(BLOCK_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1468 "language.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 91 "language.y" /* yacc.c:1646  */
    { 
		(yyval) = initParseTree(STATEMENT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1479 "language.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 97 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(STATEMENT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1491 "language.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 104 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(STATEMENT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		}
#line 1502 "language.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 110 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(STATEMENT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1512 "language.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 115 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(STATEMENT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-4])); 
		addChildParseTree((yyval), (yyvsp[-3])); 
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1526 "language.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 124 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(STATEMENT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-6])); 
		addChildParseTree((yyval), (yyvsp[-5])); 
		addChildParseTree((yyval), (yyvsp[-4])); 
		addChildParseTree((yyval), (yyvsp[-3])); 
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1542 "language.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 135 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(STATEMENT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-4])); 
		addChildParseTree((yyval), (yyvsp[-3])); 
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1556 "language.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 144 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(STATEMENT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-6])); 
		addChildParseTree((yyval), (yyvsp[-5])); 
		addChildParseTree((yyval), (yyvsp[-4])); 
		addChildParseTree((yyval), (yyvsp[-3])); 
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1572 "language.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 155 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(STATEMENT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-6])); 
		addChildParseTree((yyval), (yyvsp[-5])); 
		addChildParseTree((yyval), (yyvsp[-4])); 
		addChildParseTree((yyval), (yyvsp[-3])); 
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1588 "language.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 166 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(STATEMENT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-6])); 
		addChildParseTree((yyval), (yyvsp[-5])); 
		addChildParseTree((yyval), (yyvsp[-4])); 
		addChildParseTree((yyval), (yyvsp[-3])); 
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1604 "language.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 177 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(STATEMENT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-8])); 
		addChildParseTree((yyval), (yyvsp[-7])); 
		addChildParseTree((yyval), (yyvsp[-6])); 
		addChildParseTree((yyval), (yyvsp[-5])); 
		addChildParseTree((yyval), (yyvsp[-4])); 
		addChildParseTree((yyval), (yyvsp[-3])); 
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1622 "language.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 190 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(STATEMENT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-8])); 
		addChildParseTree((yyval), (yyvsp[-7])); 
		addChildParseTree((yyval), (yyvsp[-6])); 
		addChildParseTree((yyval), (yyvsp[-5])); 
		addChildParseTree((yyval), (yyvsp[-4])); 
		addChildParseTree((yyval), (yyvsp[-3])); 
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1640 "language.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 203 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(STATEMENT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-6])); 
		addChildParseTree((yyval), (yyvsp[-5])); 
		addChildParseTree((yyval), (yyvsp[-4])); 
		addChildParseTree((yyval), (yyvsp[-3])); 
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1656 "language.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 214 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(STATEMENT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-6])); 
		addChildParseTree((yyval), (yyvsp[-5])); 
		addChildParseTree((yyval), (yyvsp[-4])); 
		addChildParseTree((yyval), (yyvsp[-3])); 
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1672 "language.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 225 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(STATEMENT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-4])); 
		addChildParseTree((yyval), (yyvsp[-3])); 
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1686 "language.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 234 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(STATEMENT_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-4])); 
		addChildParseTree((yyval), (yyvsp[-3])); 
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1700 "language.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 245 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(VAR_LIST_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1710 "language.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 250 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(VAR_LIST_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1722 "language.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 259 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(VAR_LIST_ELE_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1732 "language.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 264 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(VAR_LIST_ELE_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1744 "language.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 274 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(FUNCTION_DECL_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-5])); 
		addChildParseTree((yyval), (yyvsp[-4])); 
		addChildParseTree((yyval), (yyvsp[-3])); 
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1759 "language.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 286 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(PARAM_LIST_PARSE);
		}
#line 1767 "language.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 289 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(PARAM_LIST_PARSE);
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1776 "language.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 293 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(PARAM_LIST_PARSE); 
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1787 "language.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 301 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(PARAM_LIST_ELE_PARSE);
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-1]));
		addChildParseTree((yyval), (yyvsp[0]));
		}
#line 1798 "language.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 309 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(FUNCTION_CALL_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-3])); 
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1811 "language.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 317 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(FUNCTION_CALL_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-3])); 
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1824 "language.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 327 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(EXP_LIST_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		}
#line 1833 "language.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 331 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(EXP_LIST_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1845 "language.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 338 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(EXP_LIST_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1857 "language.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 347 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(VARIABLE_PARSE);
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[0]));
		}
#line 1867 "language.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 352 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(VARIABLE_PARSE);
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-3]));
		addChildParseTree((yyval), (yyvsp[-2]));
		addChildParseTree((yyval), (yyvsp[-1]));
		addChildParseTree((yyval), (yyvsp[0]));
		}
#line 1880 "language.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 362 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1890 "language.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 367 "language.y" /* yacc.c:1646  */
    { 
		(yyval) = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1900 "language.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 372 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(EXP_PARSE);
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[0]));
		}
#line 1910 "language.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 377 "language.y" /* yacc.c:1646  */
    { 
		(yyval) = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1920 "language.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 382 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1932 "language.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 389 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1944 "language.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 396 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1956 "language.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 403 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1968 "language.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 410 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1980 "language.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 417 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 1992 "language.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 424 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 2004 "language.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 431 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 2016 "language.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 438 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 2027 "language.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 444 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 2038 "language.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 450 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 2050 "language.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 457 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 2062 "language.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 464 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 2074 "language.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 471 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 2086 "language.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 478 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 2098 "language.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 485 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 2110 "language.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 492 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 2122 "language.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 499 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 2133 "language.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 505 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(EXP_PARSE); 
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 2144 "language.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 511 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(EXP_PARSE);
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 2154 "language.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 516 "language.y" /* yacc.c:1646  */
    {
		(yyval) = initParseTree(EXP_PARSE);	
		appendLinkedList(&((ParseParam *) param)->cleanup, (yyval));
		addChildParseTree((yyval), (yyvsp[-2])); 
		addChildParseTree((yyval), (yyvsp[-1])); 
		addChildParseTree((yyval), (yyvsp[0])); 
		}
#line 2166 "language.tab.c" /* yacc.c:1646  */
    break;


#line 2170 "language.tab.c" /* yacc.c:1646  */
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

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (param, YY_("syntax error"));
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
        yyerror (param, yymsgp);
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
                      yytoken, &yylval, param);
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

  /* Do not reclaim the symbols of the rule whose action triggered
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
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

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
                  yystos[yystate], yyvsp, param);
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
  yyerror (param, YY_("memory exhausted"));
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
                  yytoken, &yylval, param);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, param);
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
  return yyresult;
}
#line 526 "language.y" /* yacc.c:1906  */



#include <stdio.h>

void generateerror (void *param, char const *msg) {
  snprintf (((ParseParam *) param)->errormessage, 255, "%s, Line: %d\n", msg, yylineno);
}

void yyerror (void *param, char const *msg) {
  generateerror(param, msg);
}
