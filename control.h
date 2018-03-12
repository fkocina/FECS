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

#ifndef __CONTROL_H__
#define __CONTROL_H__

#include "main.h"

inline bool logic_cast(ConstNumber num) {return num>=ONE;}

class Condition {
protected:
  bool val; //logic value
  Number *Gn, *Gp; //conductivities changed according to val
public:
  Condition(Arg *a): val(false), Gn(&a->Gn), Gp(&a->Gp) {}
  Condition(bool val = false, Number *Gn = NULL, Number *Gp = NULL):
    val(val), Gn(Gn), Gp(Gp) {}
  void eval() const {
    if(val) {
      *Gn = Gopen;
      *Gp = Gclosed;
    }
    else {
      *Gp = Gopen;
      *Gn = Gclosed;
    }
  }
};

class ConditionCh: protected Condition { //conditions based on the value of res
protected:
  const Number *res;
public:
  ConditionCh(Number *res = NULL): res(res) {}
  ConditionCh(Number *res, Arg *a): res(res), Condition(a) {a->N = res; add();}
  ConditionCh(Number *res, bool val, Number *Gn, Number *Gp):
    res(res), Condition(val, Gn, Gp) {add();}
  void add() {cur_conditions->push_back(this);}
  void eval() {val = logic_cast(*res); Condition::eval();} //eval globally
  void eval(std::vector<Condition*> &changed) { //eval locally into changed
    bool tmp = logic_cast(*res);
    if(val != tmp) {
      val = tmp;
      changed.push_back(this);
    }
  }
  void print() const {
    std::cerr << "res=" << pointer(res) << " Gn=" << pointer(Gn)
              << " Gp=" << pointer(Gp);
  }
};

class Event: public Condition { //discrete events (e.g. 1, 1, 0)
  Number tn;
public:
  Event(ConstNumber t, bool val, Number *Gn, Number *Gp): tn(t),
    Condition(val, Gn, Gp) {}
  bool active() const {return tn<=t;} //is still active?
  bool operator<(const Event &event) const {return tn<event.tn;}
  void print() const {
    std::cerr << "tn=" << tn << " val=" << val << " Gn=" << pointer(Gn)
              << " Gp=" << pointer(Gp);
  }
};

class Sum { //for summing arguments
  std::vector<const Number*> args;
  Number res;
  friend Dae;
  friend Term;
public:
  void add(const Number *num) {args.push_back(num);}
  void eval() {sum(args, res);}
  void print() const {
    std::vector<const Number*>::const_iterator it, end = args.end();
    std::cerr << pointer(&res) << " = " << pointer(*(it=args.begin()));
    while(++it != end) std::cerr << "+" << pointer(*it);
  }
  void reserve(size_t size) {args.reserve(size);}
  const Number *result() {return &res;}
};

class Assignment: ConditionCh { //for sum assignments
  Sum expr;
public:
  Assignment() {res = expr.result();}
  void add(const Number *num) {expr.add(num);}
  void eval() {expr.eval();}
  void print() const {expr.print();}
  void reserve(size_t size) {expr.reserve(size);}
  const Number *result() const {return res;}
  void set_out(Arg *arg) { //bind to affected inputs
    Gn = &arg->Gn;
    Gp = &arg->Gp;
    arg->N = const_cast<Number*>(res);
    ConditionCh::add();
  }
};

#endif
