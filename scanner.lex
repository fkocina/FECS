/*
  FECS: Fast Electronic Circuits Simulator
  Copyright (C) 2017 Filip Kocina

  This file is part of FECS.

  FECS is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  FECS is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with FECS.  If not, see <http://www.gnu.org/licenses/>.
*/

%option noyywrap yylineno
%{
#include "main.h"
void scanner_error();
int yycolumn = 1; //for errors
#define YY_USER_ACTION yycolumn += yyleng;
%}

BIT             [01]
INTEGER         [0-9]+
DECIMAL         [-+]?{INTEGER}(\.{INTEGER})?([eE][-+]?{INTEGER})?

IDENTIFIER      [[:alpha:]_][[:alnum:]_]*
LINE_COMMENT    \/\/.*
COMMENT         \/\*[^*]*\*+([^*/][^*]*\*+)*\/

%%

"{"             {return LEX_BEGIN;}
"}"             {return LEX_END;}
"("             {return LEX_LEFT;}
","             {return LEX_COMMA;}
")"             {return LEX_RIGHT;}
"&"             {return LEX_AND;}
"="             {return LEX_EQUALS;}
"nand"          {return LEX_NAND;}
"nor"           {return LEX_NOR;}
"not"           {return LEX_NOT;}
"setup"         {return LEX_SETUP;}
"xor"           {return LEX_XOR;}
{BIT}           {yylval.id = yytext[0]-'0'; return LEX_BIT;}
{IDENTIFIER}    {yylval.id = symbols[yytext]; return LEX_ID;}
{DECIMAL}       {yylval.id = decimals[yytext]; return LEX_DECIMAL;}
[\n]            {yycolumn = 1;}
{LINE_COMMENT}  {}
{COMMENT}       {}
[[:space:];]    {}
.               {scanner_error();}

%%
