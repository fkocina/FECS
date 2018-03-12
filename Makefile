# FECS: Fast Electronic Circuits Simulator
# Copyright (C) 2017 Filip Kocina
#
# This file is part of FECS.
#
# FECS is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# FECS is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with FECS.  If not, see <http://www.gnu.org/licenses/>.

.PHONY: clean

PROJ=vlsi
CC=$(CXX)
CFLAGS=-O2 -lpthread
CXXFLAGS=$(CFLAGS)
LEX=lex
YACC=yacc

$(PROJ): y.tab.o lex.yy.o expr.o main.o solver.o term.o worker.o
	$(CXX) $(CXXFLAGS) -o $@ $^

lex.yy.c: scanner.lex
	$(LEX) $^

y.tab.c: parser.y
	$(YACC) -d $^

clean:
	rm -f -- $(PROJ) *.o lex.yy.c y.tab.?
