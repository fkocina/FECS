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

#ifndef __THREAD_H__
#define __THREAD_H__

#include <pthread.h>
#include <vector>

//universal abstract class:
class Thread { //called in each thread (arg=this):
  static void *run2(void *arg) {static_cast<Thread*>(arg)->run(); return NULL;}
  pthread_t id;
  virtual void run() = 0;
public: //create the thread identified by id and call static run2 for it:
  void start() {pthread_create(&id, NULL, run2, this);}
};

//threads container:
class Threads {
  std::vector<Thread*> threads; //all threads
public:
  void add(Thread *thread) {threads.push_back(thread);}
  void reserve(size_t size) {threads.reserve(size);} //allocate memory
  void run() { //create threads:
    std::vector<Thread*>::const_iterator it, end = threads.end();
    for(it = threads.begin(); it != end; ++it) (*it)->start();
  }
  ~Threads() { //the class owns the pointers
    std::vector<Thread*>::const_reverse_iterator it, end = threads.rend();
    for(it = threads.rbegin(); it != end; ++it) delete *it;
    threads = std::vector<Thread*>(); //free memory
  }
};

#endif
