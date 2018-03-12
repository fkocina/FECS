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

#ifndef __EXPR_H__
#define __EXPR_H__

#include "main.h"

class num_greater { //split str into a and b, b starts with the 1st digit if any
  static void split(const std::string &str, std::string &a, std::string &b) {
    std::string::const_iterator it = str.begin(), end = str.end();
    for(size_t i = 0; it != end; ++it, ++i) {
      char c = *it;
      if(c >= '0' && c <= '9') {
        a = str.substr(0, i);
        b = str.substr(i);
        return;
      }
    }
    a = str;
    b = "";
  }
public: //sort not only literally but also by the first number
  bool operator()(const std::string &a, const std::string &b) {
    static std::string a1, a2, b1, b2;
    static long l, l2;
    split(a, a1, a2);
    split(b, b1, b2);
    if(a1 > b1) return true;
    else if(a1 == b1) { //the first number decides
      l = atol(a2.c_str());
      l2 = atol(b2.c_str());
      if(l > l2) return true;
      else if(l == l2) return a2>b2;
    }
    return false;
  }
};

class Expr {
  static std::deque<Expr*> exprs;
  Arg *res;
  short iv; //initial value
  std::vector<bool> bits;
  std::vector<Expr*> args;
  std::string var; //assigned variable name
  Type type;
  void assign() {iv = -1; res = NULL; reg();} //assign default values
  void reg() {exprs.push_back(this);}
  void set_res() {
    if(res == NULL_PTR()) error_exit("Cycle detected.");
    if(!res) {
      if(var == "") res = new Arg;
      else {
        Arg *&nres = numbers[var];
        if(!nres)  {
          if(type == VAR && args.size() == 1) {
            res = NULL_PTR(); //to detect cycles
            nres = args.front()->out(); //bind the variables
          }
          else nres = new Arg;
        }
        res = nres;
      } //if debug, mark human-readable pointer descriptions:
      if(bDebug && pointers[&res->Gn] == "") {
        pointers[&res->Gn] = var+".N";
        pointers[&res->Gp] = var+".P";
      }
    }
  }
  void tran_xor();
  friend Term;
public:
  static std::map<std::string,Arg*,num_greater> numbers;
  static void transform();
  Expr(Expr *expr): type(ARGS) {assign(); add(expr);} //the first argument
  Expr(const std::string &var): type(VAR), var(var) {assign();} //named variable
  Expr(const std::string &var, const std::string &var2): type(VAR), var(var) {
    assign(); add(new Expr(var2)); set_res(); //assignment from a variable
  }
  Expr(unsigned bit): type(BITS) {assign(); if(bit < 2) add(bit);} //the 1st bit
  void add(Expr *arg) {args.push_back(arg);}
  void add(unsigned bit) {bits.push_back(bit);}
  Arg *out() {set_res(); return res;}
  void set_iv(const Expr *e) {if(!e->bits.empty()) iv = e->bits.front();}
  void set_type(Type type) {this->type = type;}
  void set_var(const std::string &name) {var = name; set_res();}
};

#endif
