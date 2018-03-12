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

#ifndef __DAE_H__
#define __DAE_H__

#include "main.h"

class Dae {
  static size_t nAlgs, nODEs;
  bool bODE;
  unsigned short idx;
  std::vector<const Number*> args;
  const Number *i_val; //total current
  Number cur_val, *G, res; //term value, conductivity and result
  void fill(std::vector<std::vector<Number> > &m, size_t ORD) {
    std::vector<Number> &mults = m[idx];
    size_t size = mults.size();
    if(size < ORD) { //if a coefficient of higher order needed
      ConstNumber coeff = ::coeff[idx];
      while(size < ORD) mults.push_back(coeff/++size); //add including previous
    }
  }
  void reg() {cur_daes->push_back(this);}
public:
  static size_t algs() {return nAlgs;}
  static size_t odes() {return nODEs;}
  Dae(size_t N, Dae *i, Number *G, ConstNumber iv = 0): bODE(true),
   G(G), res(iv), i_val(&i->res), idx(N-1) {
    ++nODEs;
    i->add(&cur_val); //term value is also used for the calculation of current
    if(!G) this->G = new Number;
    reg();
  }
  Dae(): bODE(false) {++nAlgs; reg();}
  void add(const Number *num) {args.push_back(num);}
  void add_term() {res += cur_val;}
  void eval_term(std::vector<std::vector<Number> > &mults, size_t ORD) {
    if(bODE) { //evaluate the term (see Chapter 5.4)
      cur_val *= *G;
      cur_val += *i_val;
      fill(mults, ORD);
      cur_val *= mults[idx][ORD-1]; //outer coefficient
    }
    else { //expression for current
      sum(args, res);
      if(ORD == 1) res += U;
      res *= Gi;
    }
  }
  void first_term() { //init before the first term
    if(bODE) {
      cur_val = res;
      if(!args.empty()) sum(args, *G);
    }
  }
  bool is_ode() const {return bODE;}
  void labg() { //if debug, create human-readable pointer description
    if(bDebug && pointers[G] == "") {
      std::vector<const Number*>::const_iterator it, end = args.end();
      std::string name = pointer(*(it=args.begin()));
      while(++it != end) name += std::string("+")+pointer(*it);
      pointers[G] = name;
    }
  }
  void print() const { //for debugging
    std::cerr << (bODE? "ODE": "Alg") << ": res=" << pointer(&res);
    if(bODE) std::cerr << " cv=" << pointer(&cur_val);
    else if(!args.empty()) {
      std::cerr << " args=(";
      std::vector<const Number*>::const_iterator it, end = args.end();
      std::cerr << pointer(*(it=args.begin()));
      while(++it != end) std::cerr << "," << pointer(*it);
      std::cerr << ")";
    }
    if(bODE) std::cerr << " G=" << pointer(G) << " i=" << pointer(i_val);
    std::cerr << std::endl;
  }
  void reserve(size_t size) {args.reserve(size);}
  const Number *result() const {return &res;}
  void set_out(Arg *res) {new ConditionCh(&this->res, res);}
  ConstNumber term() {return cur_val;}
};

class Gate {
  std::vector<Dae*> daes;
  friend size_t taylor(std::vector<std::vector<Number> > &, Gate *);
  friend void print_debug();
  friend Term;
public:
  Gate() {cur_daes = &daes;}
  void reserve(size_t size) {daes.reserve(size);}
  size_t size() const {return daes.size();}
};

class Group {
  std::vector<Assignment*> assignments;
  std::vector<Condition*> changed;
  std::vector<ConditionCh*> conditions;
  std::vector<Gate*> gates;
  size_t sz;
  friend void init_threads();
  friend void print_debug();
public:
  Group(): sz(0) { //cur* variables are used in parser and single-threaded code
    curGroup = this;
    cur_assignments = &assignments;
    cur_changed = &changed;
    cur_conditions = &conditions;
  }
  void add() {gates.push_back(new Gate);}
  void add_size() {sz += gates.back()->size();}
  std::vector<Gate*>::iterator begin() {return gates.begin();}
  std::vector<Gate*>::iterator end() {return gates.end();}
  void perform_conditions() {cur_changed = &changed; ::perform_conditions();}
  void reserve_assignments(size_t size) {assignments.reserve(size);}
  void reserve_changed(size_t size) {changed.reserve(size);}
  void reserve_conditions(size_t size) {conditions.reserve(size);}
  void reserve_gates(size_t size) {gates.reserve(size);}
  size_t size() const {return sz;}
  size_t solve(std::vector<std::vector<Number> > &mults) {
    size_t ORD, MAXORD = 0; //solve the group of equations:
    std::vector<Gate*>::const_iterator it, end = gates.end();
    for(it = gates.begin(); it != end; ++it) {
      ORD = taylor(mults, *it);
      if(ORD > MAXORD) MAXORD = ORD;
    }
    assign(&assignments); //eval conditions locally (thread-safe):
    eval_conditions(&conditions, &changed);
    return MAXORD;
  }
};

#endif
