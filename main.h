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

#ifndef __MAIN_H__
#define __MAIN_H__

#include "defaults.h"
#include <algorithm>
#include <cmath>
#include <deque>
#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <vector>

class Assignment;
class Condition;
class ConditionCh;
class Dae;
class Event;
class Expr;
class Gate;
class Group;
class Sum;
class Symbols;
class Term;

struct Arg {
  const Number *N; //current voltage
  Number Gn, Gp; //conductiv. for n- and p-channel based on logical value of *N
  Arg(): N(NULL) {}
};

enum Type {
  VAR, ARGS, BITS, NAND, NOR, NOT, XOR
};

extern bool bDebug, bThreaded;
extern std::deque<Event> events;
extern std::deque<Group*> groups;
extern Group *curGroup;
extern std::map<const void*,std::string> pointers; //for logging
extern Number dt, mult, t, tmax, Cinv, EPS, Gi, Gclosed, Gopen, U, ONE;
extern size_t TEST, nThreads, MAXORD, maxSize, maxInputs;
extern std::string show;
extern Symbols decimals, symbols;
extern std::vector<Assignment*> *cur_assignments;
extern std::vector<Condition*> *cur_changed;
extern std::vector<ConditionCh*> *cur_conditions;
extern std::vector<Dae*> *cur_daes;
extern std::vector<std::vector<Number> > *cur_mults;
extern std::vector<Number> coeff;

#define ABS fabsl

void assign(std::vector<Assignment*> *);
void error_exit(const std::string &);
void eval_conditions(std::vector<ConditionCh*> *, std::vector<Condition*> *);
void init_mults(std::vector<std::vector<Number> > &);
void init_threads();
void mark_mem_sz();
void perform_conditions();
void preinit_threads();
size_t taylor(std::vector<std::vector<Number> > &, Gate *);

inline Arg *NULL_PTR() { //to detect cycles
  static Arg *arg = new Arg;
  return arg;
}

inline std::string num2str(ConstNumber n) {
  std::stringstream ss;
  ss << n;
  return ss.str();
}

//manages human-readable pointer descriptions:
inline const std::string &pointer(const void *ptr) {
  static size_t i = 0;
  std::string &res = pointers[ptr];
  if(res == "") {
    std::stringstream ss;
    ss << "#" << ++i;
    res = ss.str();
  }
  return res;
}

//calculates the sum of elements in from:
template<typename F, typename T> inline void sum(const F &from, T &to) {
  typename F::const_iterator it = from.begin(), end = from.end();
  to = **it; while(++it != end) to += **it;
}

#include "control.h"
#include "dae.h"
#include "expr.h"
#include "solver.h"
#include "symbols.h"
#include "term.h"
#include "threads.h"
#include "worker.h"
#include "y.tab.h"

#endif
