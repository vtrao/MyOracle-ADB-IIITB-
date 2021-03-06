/* A Bison parser, made by GNU Bison 1.875d.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.

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
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     SELECT = 258,
     INSERT = 259,
     UPDATE = 260,
     DELETE = 261,
     CREATE = 262,
     DROP = 263,
     ALTER = 264,
     COMMIT = 265,
     EXITST = 266,
     GET = 267,
     TO = 268,
     INTNUM = 269,
     APPROXNUM = 270,
     IDENTIFIER = 271,
     NEWLINE = 272,
     TABLE = 273,
     LEFTPARANTHESIS = 274,
     RIGHTPARANTHESIS = 275,
     INTO = 276,
     DATABASE = 277,
     SHOW = 278,
     CLEAR = 279,
     FROM = 280,
     DOT = 281,
     COMMA = 282,
     CHARACTER = 283,
     INTEGER = 284,
     DATE = 285,
     NOT = 286,
     NULLX = 287,
     UNIQUE = 288,
     PRIMARY = 289,
     KEY = 290,
     DEFAULT = 291,
     ALL = 292,
     DISTINCT = 293,
     STAR = 294,
     WHERE = 295,
     OR = 296,
     AND = 297,
     VALUES = 298,
     ISEQUALTO = 299,
     SET = 300,
     ADD = 301,
     STRING = 302,
     DATELITERAL = 303,
     GREATERTHAN = 304,
     LESSTHAN = 305,
     GREATERTHANE = 306,
     LESSTHANE = 307,
     NOTEQUALTO = 308,
     USE = 309,
     DESC = 310
   };
#endif
#define SELECT 258
#define INSERT 259
#define UPDATE 260
#define DELETE 261
#define CREATE 262
#define DROP 263
#define ALTER 264
#define COMMIT 265
#define EXITST 266
#define GET 267
#define TO 268
#define INTNUM 269
#define APPROXNUM 270
#define IDENTIFIER 271
#define NEWLINE 272
#define TABLE 273
#define LEFTPARANTHESIS 274
#define RIGHTPARANTHESIS 275
#define INTO 276
#define DATABASE 277
#define SHOW 278
#define CLEAR 279
#define FROM 280
#define DOT 281
#define COMMA 282
#define CHARACTER 283
#define INTEGER 284
#define DATE 285
#define NOT 286
#define NULLX 287
#define UNIQUE 288
#define PRIMARY 289
#define KEY 290
#define DEFAULT 291
#define ALL 292
#define DISTINCT 293
#define STAR 294
#define WHERE 295
#define OR 296
#define AND 297
#define VALUES 298
#define ISEQUALTO 299
#define SET 300
#define ADD 301
#define STRING 302
#define DATELITERAL 303
#define GREATERTHAN 304
#define LESSTHAN 305
#define GREATERTHANE 306
#define LESSTHANE 307
#define NOTEQUALTO 308
#define USE 309
#define DESC 310




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



  