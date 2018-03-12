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

#include "term.h"
using namespace std;

deque<const Term*> Term::terms;
size_t Term::nTrans = 0, Term::nINVs = 0, Term::nNANDs = 0, Term::nNORs = 0;

Term::Term(Expr *e): bIV(false), res(e->res), type(e->type) {
  if(type == BITS) bits = e->bits;
  else { //evaluate default initial values and add arguments:
    bool bIV = false, val = false, nval = true; //for NAND
    if(type == NOR) bIV = true, val = true, nval = false;
    vector<Expr*>::const_iterator it, end = e->args.end();
    for(it = e->args.begin(); it != end; ++it) {
      Expr *expr = *it;
      if((expr->iv>0) == val) bIV = nval;
      args.push_back(expr->out());
    }
    if(e->iv < 0) e->iv = bIV, this->bIV = bIV;
    else this->bIV = e->iv;
  }
  reg();
}

void Term::add(Expr *e) {
  args.push_back(e->out());
}

void Term::instr_bits() const { //bits -> discrete events
  vector<bool>::const_iterator it, end = bits.end();
  Number tn = t, dt = (tmax-t)/bits.size();
  for(it = bits.begin(); it != end; ++it) {
    events.push_back(Event(tn, *it, &res->Gn, &res->Gp));
    tn += dt;
  }
}

//make the parallel part of a transistor (see Chapter 5.4):
void Term::make_par(Dae *i, Number Arg::*G, bool bIV, Arg *arg) const {
  size_t size = args.size();
  Dae *uc = new Dae(size, i, NULL, bIV? -U: 0); //capacitors are merged
  if(size > maxInputs) maxInputs = size; //mark if more inputs
  vector<Arg*>::const_iterator it, end = args.end();
  uc->reserve(size); //arguments are driven by Gn or Gp:
  for(it = args.begin(); it != end; ++it) uc->add(&(*it->*G));
  if(arg) uc->set_out(arg);
  uc->labg(); //if debug, create human-readable pointer description
}

//make the serial part of a transistor (see Chapter 5.4):
Dae *Term::make_ser(Number Arg::*G, bool bIV, Arg *arg) const {
  Assignment *assignment = NULL;
  Dae *i = new Dae;
  size_t size = args.size();
  i->reserve(size+1);
  if(arg) { //NOR does not need the sum
    cur_assignments->push_back(new Assignment);
    assignment = cur_assignments->back();
    assignment->reserve(size);
  }
  vector<Arg*>::const_iterator it, end = args.end();
  Number iv = bIV? -U/size: 0; //initial values of all ser. capac. must give -U
  for(it = args.begin(); it != end; ++it) {
    Dae *uc = new Dae(1, i, &(*it->*G), iv); //an ODE for each transistor
    if(assignment) assignment->add(uc->result()); //results are summed on need
  }
  if(assignment) assignment->set_out(arg);
  return i;
}

void Term::instr_nand() const {
  if(args.empty()) error_exit("NAND has to have arguments.");
  add_trans(2*args.size()); //log number of transistors
  if(args.size() == 1) inc_invs(); //inverter can be made like NAND
  else inc_nands();
  set_current_group();
  make_par(make_ser(&Arg::Gn, bIV, res), &Arg::Gp, !bIV); //see Chapter 5.4.2
  curGroup->add_size(); //mark number of ODEs
}

void Term::instr_nor() const {
  if(args.empty()) error_exit("NOR has to have arguments.");
  add_trans(2*args.size()); //log number of transistors
  if(args.size() == 1) inc_invs(); //inverter can be made like NOR
  else inc_nors();
  set_current_group();
  make_par(make_ser(&Arg::Gp, !bIV), &Arg::Gn, bIV, res); //see Chapter 5.4.3
  curGroup->add_size(); //mark number of ODEs
}

void Term::instr_not() const {
  if(args.size() != 1) error_exit("NOT has to have exactly one argument.");
  instr_nor();
}

void Term::set_current_group() const { //maxSize can be changed by param. bunch
  if(groups.empty() || bThreaded && curGroup->size() >= maxSize)
    groups.push_back(new Group);
  curGroup->add();
}
