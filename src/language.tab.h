/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

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
