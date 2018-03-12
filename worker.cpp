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

#include "worker.h"
#include <iostream>
using namespace std;

bool Worker::bCanSend = true, Worker::bCanRecv = false;
bool Worker::bNextStep = true;
pthread_cond_t Worker::cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t Worker::cond2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t Worker::cond3 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t Worker::mutex = PTHREAD_MUTEX_INITIALIZER;
unsigned Worker::nWorkers = 0, Worker::nReaders = 0;
void *Worker::data = NULL;


void Worker::run() {
  init_mults(mults);
  size_t ORD;
  while(true) {
    pthread_mutex_lock(&mutex); //CS begin
    while(!bCanRecv) pthread_cond_wait(&cond2, &mutex); //wait for load
    bCanRecv = false;
    bCanSend = true;
    pthread_cond_signal(&cond); //signal the sender (main thread)
    Group &group = *static_cast<Group*>(data);
    nReaders++;
    pthread_mutex_unlock(&mutex); //CS end

    //the main part of the thread (this line should take longest):
    ORD = group.solve(mults);

    pthread_mutex_lock(&mutex); //CS begin
    if(ORD > MAXORD) MAXORD = ORD;
    nReaders--;
    if(nReaders == 0 && bCanSend && !bCanRecv) { //can make next step?
      bNextStep = true;
      pthread_cond_signal(&cond3);
    }
    pthread_mutex_unlock(&mutex); //CS end
  }
}
