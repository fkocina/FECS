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

%{
#define YY_NO_UNPUT
#define YYERROR_VERBOSE
#include "main.h"
using namespace std;
int yyerror(const char *);
int yylex();
int error;
void set_const(const string &, const string &); //set number const
void set_error(int id) {
  error = id;
}
void set_par(const string &, const string &); //set non-number const
%}

%union {
  unsigned int id;
  class Expr *node;
}

%token <id> LEX_AND LEX_BEGIN LEX_BIT LEX_COMMA LEX_DECIMAL LEX_END LEX_EQUALS
            LEX_ID LEX_LEFT LEX_RIGHT LEX_SETUP LEX_NAND LEX_NOR LEX_NOT LEX_XOR
%type <node> arg args bits expr gate input iv setup setupLine setupLines source

%%

source: setup input
      | input

setup: LEX_SETUP LEX_BEGIN setupLines LEX_END {}

setupLines: setupLines setupLine
          | setupLine

//setting simulation parameters
setupLine: LEX_ID LEX_EQUALS LEX_BIT {set_const(symbols[$1], $3? "1": "0");}
         | LEX_ID LEX_EQUALS LEX_DECIMAL {set_const(symbols[$1], decimals[$3]);}
         | LEX_ID LEX_EQUALS LEX_ID {set_par(symbols[$1], symbols[$3]);}

input: input expr
      | expr

//e.g. x = 1, 1, 0
expr: LEX_ID LEX_EQUALS LEX_ID {new Expr(symbols[$1], symbols[$3]);}
    | LEX_ID LEX_EQUALS bits {$3->set_var(symbols[$1]);}
    | LEX_ID LEX_EQUALS gate {$3->set_var(symbols[$1]);}
    | gate

bits: bits LEX_COMMA LEX_BIT {$$ = $1; $$->add($3);}
    | LEX_BIT {$$ = new Expr($1);}

//e.g. nand(nor(x, not(y)), xor(x, y), not(y))
gate: LEX_NAND LEX_LEFT args iv {$$ = $3; $$->set_type(NAND); $$->set_iv($4);}
    | LEX_NOR LEX_LEFT args iv {$$ = $3; $$->set_type(NOR); $$->set_iv($4);}
    | LEX_NOT LEX_LEFT args iv {$$ = $3; $$->set_type(NOT); $$->set_iv($4);}
    | LEX_XOR LEX_LEFT args iv {$$ = $3; $$->set_type(XOR); $$->set_iv($4);}

iv: LEX_AND LEX_BIT LEX_RIGHT {$$ = new Expr($2);} //initial value
  | LEX_RIGHT {$$ = new Expr(~0U);} //without initial value

args: args LEX_COMMA arg {$$ = $1; $$->add($3);}
    | arg {$$ = new Expr($1);}

arg: LEX_ID {$$ = new Expr(symbols[$1]);}
   | gate {$$ = $1;}

%%

Symbols decimals, symbols;

extern char *yytext;
extern int yycolumn, yyleng, yylineno;

void error_exit(const string &str) {
  cerr << "Error: " << str << endl;
  exit(2);
}

void scanner_error() {
  int col = yycolumn>1? yycolumn-yyleng: 1;
  cerr << "Error: stdin:" << yylineno << ":" << col
       << ": lexical error, unexpected symbol \"" << yytext << "\"." << endl;
  set_error(1);
}

int yyerror(const char *s) {
  int col = yycolumn>1? yycolumn-yyleng: 1;
  cerr << "Error: stdin:" << yylineno << ":" << col << ": " << s << "." << endl;
  set_error(2);
  return 2;
}

inline char lower(char c) {
  return tolower(c);
}

inline void tolower(const string &name, string &lc) {
  lc = ""; transform(name.begin(), name.end(), back_inserter(lc), lower);
}

void set_const(const string &name, const string &value) { //set number const
  static Number val; val = str2num(value.c_str());
  static string lc; tolower(name, lc);
  if(lc == "tmax") tmax = val; //ending simulation time
  else if(lc == "threads") { //0 ~ single-threaded, 1 ~ number of HW threads
    nThreads = roundl(val);
    preinit_threads();
  }
  else if(lc == "bunch") maxSize = roundl(val); //number of eq. in each thread
  else if(lc == "mult") mult = val; //print results only in multiplies of time
  else if(lc == "u") U = -val; //unit voltage, minus to avoid subtraction
  else if(lc == "one") ONE = val; //voltage threshold for logical one
  else if(lc == "c") Cinv = 1/val; //capacity
  else if(lc == "ri") Gi = -1/val; //resistance
  else if(lc == "ropen") Gopen = -1/val; //resistance of open channel
  else if(lc == "rclosed") Gclosed = -1/val; //resistance of closed channel
  else if(lc == "dt") dt = val; //step size
  else if(lc == "eps") EPS = val; //precision
  else if(lc == "test") TEST = roundl(val); //nr. of tested Taylor polynomials
  else if(lc == "tmin") t = val; //starting simulation time
  else cerr << "Warning: Unknown parameter \"" << name << "\"." << endl;
}

bool get_bool(const string &val) {
  static string lc; tolower(val, lc);
  if(lc == "on" || lc == "true") return true;
  else if(lc == "off" || lc == "false") return false;
  else error_exit("Boolean parameters accept only on/off or true/false.");
}

void set_par(const string &name, const string &value) { //set non-number const
  static string lc; tolower(name, lc);
  //if value begins with '_', show digit. values of variables with given suffix;
  //show variables with prefix value otherwise
  if(lc == "show") show = value;
  else if(lc == "debug") bDebug = get_bool(value);
  else cerr << "Warning: Unknown parameter \"" << name << "\"." << endl;
}
