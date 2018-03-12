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

#ifndef __SYMBOLS_H__
#define __SYMBOLS_H__

#include <deque>
#include <string>

class Symbols {
  std::deque<std::string> symbols;
  const std::string empty;
public:
  int operator[](const std::string &name) { //get the ID of symbol "name"
    std::deque<std::string>::const_iterator it, end = symbols.end();
    size_t i = 0;
    for(it = symbols.begin(); it != end; ++it, ++i) if(name == *it) return i;
    symbols.push_back(name); //append it if not found
    return i;
  }
  const std::string &operator[](int idx) const { //get symbol name
    if(idx < 0 || idx >= symbols.size()) return empty;
    return symbols[idx];
  }
};

#endif
