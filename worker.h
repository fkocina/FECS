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

#ifndef __WORKER_H__
#define __WORKER_H__

#include "main.h"
#include "threads.h"

class Worker: public Thread {
  static bool bCanSend, bCanRecv, bNextStep;
  static pthread_cond_t cond, cond2, cond3;
  static pthread_mutex_t mutex;
  static unsigned nWorkers, nReaders;
  static void *data;
  std::vector<std::vector<Number> > mults; //each thread has to have own mults
  unsigned workerId;
  void run();
public:
  static void send2any(void *data) {
    pthread_mutex_lock(&mutex); //CS begin
    while(!bCanSend) pthread_cond_wait(&cond, &mutex);
    bNextStep = false;
    bCanSend = false;
    Worker::data = data; //data are delivered safely to a thread
    bCanRecv = true;
    pthread_cond_signal(&cond2); //signal any thread
    pthread_mutex_unlock(&mutex); //CS end
  }
  static void wait4all() { //wait for next step
    pthread_mutex_lock(&mutex); //CS begin
    while(!bNextStep) pthread_cond_wait(&cond3, &mutex);
    pthread_mutex_unlock(&mutex); //CS end
  }
  Worker(): workerId(nWorkers++) {}
};

#endif
