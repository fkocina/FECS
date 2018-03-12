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

#ifndef __TERM_H__
#define __TERM_H__

#include "main.h"

class Term {
  static std::deque<const Term*> terms;
  static size_t nTrans, nINVs, nNANDs, nNORs; //counts
  static void add_trans(size_t n) {nTrans += n;}
  static void inc_invs() {++nINVs;}
  static void inc_nands() {++nNANDs;}
  static void inc_nors() {++nNORs;}
  Arg *res; //result
  bool bIV; //initial value
  Type type;
  std::vector<Arg*> args;
  std::vector<bool> bits;
  void instr_bits() const;
  void instr_nand() const;
  void instr_nor() const;
  void instr_not() const;
  void make_par(Dae *, Number Arg::*, bool, Arg * = NULL) const;
  Dae *make_ser(Number Arg::*, bool, Arg * = NULL) const;
  void reg() {terms.push_back(this);}
  void set_current_group() const;
  friend Expr;
public:
  static size_t gates() {return nINVs+nNANDs+nNORs;}
  static size_t invs() {return nINVs;}
  static void make_instr() { //transform to differential equations
    std::deque<const Term*>::const_iterator it, end = terms.end();
    for(it = terms.begin(); it != end; ++it) (*it)->instr();
    for(it = terms.begin(); it != end; ++it) delete *it;
    mark_mem_sz(); terms = std::deque<const Term*>(); //mark memory usage
  }
  static size_t nands() {return nNANDs;}
  static size_t nors() {return nNORs;}
  static size_t trans() {return nTrans;}
  Term(Expr *);
  Term(Type type): bIV(false), res(NULL), type(type) {reg();}
  void add(Expr *);
  void add(Term *term) { //create the space for the argument on need:
    if(!term->res) term->res = new Arg;
    args.push_back(term->res);
  }
  void instr() const {
    switch(type) {
      case BITS: instr_bits(); break;
      case NAND: instr_nand(); break;
      case NOR: instr_nor(); break;
      case NOT: instr_not(); break;
    }
  }
  void set_iv(bool iv) {bIV = iv;}
  void set_res(Arg *res) {this->res = res;}
};

#endif
