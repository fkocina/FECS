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

#ifndef __DEFAULTS_H__
#define __DEFAULTS_H__

typedef long double Number;
typedef Number ConstNumber;

#define str2num(s) strtold((s), NULL)

const Number DEFAULT_C = 3.851953e-9, DEFAULT_RI = 0.120792,
  DEFAULT_ROPEN = 0.601435, DEFAULT_RCLOSED = 1e10, DEFAULT_U = 3.3,
  DEFAULT_DT = 1e-10, DEFAULT_TMIN = 0, DEFAULT_TMAX = 2e-7,
  DEFAULT_EPS = 1e-20;
const unsigned DEFAULT_MAX_THREADS = 96, DEFAULT_TEST = 3, DEFAULT_THREADS = 0,
  DEFAULT_BUNCH = 0;

//internal details:
const unsigned DEFAULT_MINCOEFF = 32;

#endif
