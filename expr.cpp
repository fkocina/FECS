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

#include "expr.h"
using namespace std;

deque<Expr*> Expr::exprs;
map<string,Arg*,num_greater> Expr::numbers;

void Expr::transform() { //transform to class Term to lower memory usage
  deque<Expr*>::const_iterator it, end = exprs.end();
  for(it = exprs.begin(); it != end; ++it) {
    Expr *expr = *it;
    expr->set_res(); //create the space for the result if not reserved yet
    switch(expr->type) {
      case ARGS: break; //not needed
      case BITS: if(expr->var != "") new Term(expr); break; //only if assigned
      case XOR: expr->tran_xor(); break; //use basic gates
      case VAR: break; //not needed
      default: new Term(expr); //basic gates
    }
  }
  for(it = exprs.begin(); it != end; ++it) delete *it;
  mark_mem_sz(); exprs = deque<Expr*>(); //mark memory usage if higher
}

void Expr::tran_xor() { //transform two- or three-input XOR to basic gates
  size_t size = args.size();
  if(size == 2) {
    Expr *a = args.front(), *b = args.back();
    Term *na = new Term(NOT); na->add(a); //not(a)
    Term *A = new Term(NAND); A->add(na); A->add(b); //nand(not(a), b)
    Term *nb = new Term(NOT); nb->add(b); //not(b)
    Term *B = new Term(NAND); B->add(a); B->add(nb); //nand(a, not(b))
    Term *R = new Term(NAND); R->add(A); R->add(B); //nand(A, B)
    if(iv >= 0) R->set_iv(iv);
    R->set_res(res);
  }
  else if(size == 3) {
    Expr *a = args[0], *b = args[1], *c = args[2];
    Term *ab = new Term(NAND); ab->add(a); ab->add(b); //nand(a, b)
    Term *A = new Term(NOR); A->add(ab); A->add(c); //nor(nand(a,b), c)
    Term *bc = new Term(NAND); bc->add(b); bc->add(c); //nand(b, c)
    Term *B = new Term(NOR); B->add(bc); B->add(a); //nor(nand(b,c), a)
    Term *ac = new Term(NAND); ac->add(a); ac->add(c); //nand(a, c)
    Term *C = new Term(NOR); C->add(ac); C->add(b); //nor(nand(a,c), b)
    Term *D = new Term(NOR); D->add(a); D->add(b); D->add(c); //nor(a, b, c)
    Term *R = new Term(NOR); R->add(A); R->add(B); R->add(C); R->add(D);
    if(iv >= 0) R->set_iv(iv);
    R->set_res(res);
  }
  else error_exit("XOR can have only two or three arguments.");
}
