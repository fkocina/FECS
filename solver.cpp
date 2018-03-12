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

#include "solver.h"
#include <sys/time.h>
#include <unistd.h>
using namespace std;

bool bDebug = false, bMult = false, bSuf = false, bThreaded = false;
deque<Event> events;
deque<Group*> groups;
Group *curGroup = NULL;
vector<const Number*> numbers;
map<const void*,string> pointers;
Number Cinv = 1.L/DEFAULT_C, Gi = -1.L/DEFAULT_RI, Gopen = -1.L/DEFAULT_ROPEN,
  Gclosed = -1.L/DEFAULT_RCLOSED, U = -DEFAULT_U, ONE = nanl(""),
  dt = DEFAULT_DT, mult = 0, t = DEFAULT_TMIN, tmax = DEFAULT_TMAX,
  EPS = DEFAULT_EPS, t0 = 0, totalMem = 0;
size_t TEST = DEFAULT_TEST, MAXORD = 0, nThreads = DEFAULT_THREADS,
  maxSize = DEFAULT_BUNCH, maxInputs = 0, curMult = 0, nMult = 0;
string show;
Threads threads;
vector<Assignment*> *cur_assignments = NULL;
vector<Condition*> *cur_changed = NULL;
vector<ConditionCh*> *cur_conditions = NULL;
vector<Dae*> *cur_daes = NULL;
vector<vector<Number> > *cur_mults = NULL;
vector<Number> coeff;
vector<size_t> lengths;

size_t Dae::nAlgs = 0, Dae::nODEs = 0;

void yylex_destroy();
int yyparse();

Number microtime() {
  timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec+tv.tv_usec/1000000.L;
}

inline bool prefm(const string &str) { //matches the prefix?
  static bool bEmpty = show==""; if(bEmpty) return true;
  static size_t size = show.size();
  return str.substr(0, size) == show;
}

inline string rmsuf(const string &str) { //remove suffix including index
  static int size = show.size();
  int i = 0;
  string new_str = str.substr(0, str.length()-size);
  string::reverse_iterator it, end = new_str.rend();
  for(it = new_str.rbegin(); it != end && isdigit(*it); ++it, ++i);
  return new_str.substr(0, new_str.length()-i);
}

inline bool sufm(const string &str) { //matches the suffix?
  static int size = show.size();
  int idx = (int)str.length()-size;
  if(idx < 0) return false;
  return str.substr(idx) == show;
}

void print_header() {
  bool bShow = show!="";
  bSuf = show[0]=='_';
  cout << "t";
  map<string,Arg*>::const_iterator it, end = Expr::numbers.end();
  string printed;
  size_t n = 0;
  for(it = Expr::numbers.begin(); it != end; ++it) //if var should be shown:
    if(it->second->N && (!bShow || (bSuf? sufm(it->first): prefm(it->first)))) {
      if(bSuf) { //group variable names by the suffix
        string pref = rmsuf(it->first);
        if(pref != printed) { //if the group of variables not printed yet
          cout << "\t" << pref;
          printed = pref;
          if(n) lengths.push_back(n); //push back the previous bit-length
          n = 1;
        }
        else ++n; //calculate bits
      }
      else cout << "\t" << it->first;
      numbers.push_back(it->second->N);
    }
  lengths.push_back(n);
  cout << endl;
}

void assign(vector<Assignment*> *assignments) { //sum all terms into result
  vector<Assignment*>::const_iterator it, end = assignments->end();
  for(it = assignments->begin(); it != end; ++it) (*it)->eval();
}

void print_assignments() {
  size_t i = 0;
  vector<Assignment*>::const_iterator it, end = cur_assignments->end();
  for(it = cur_assignments->begin(); it != end; ++it) {
    cerr << "Assignment " << ++i << ": ";
    (*it)->print();
    cerr << endl;
  }
}

void print_conditions() {
  if(!cur_assignments->empty() && !cur_conditions->empty()) cerr << endl;
  size_t i = 0;
  vector<ConditionCh*>::const_iterator it, end = cur_conditions->end();
  for(it = cur_conditions->begin(); it != end; ++it) {
    cerr << "Condition " << ++i << ": ";
    (*it)->print();
    cerr << endl;
  }
}

void print_numbers() {
  cerr << "Numbers:";
  vector<const Number*>::const_iterator it, end = numbers.end();
  for(it = numbers.begin(); it != end; ++it) cerr << " " << pointer(*it);
  cerr << endl;
}

void print_daes() {
  vector<Dae*>::const_iterator it, end = cur_daes->end();
  for(it = cur_daes->begin(); it != end; ++it) (*it)->print();
}

void print_events() {
  size_t i = 0;
  deque<Event>::const_iterator it, end = events.end();
  for(it = events.begin(); it != end; ++it) {
    cerr << "Event " << ++i << ": ";
    it->print();
    cerr << endl;
  }
  if(!events.empty()) cerr << endl;
}

void print_debug() {
  deque<Group*>::const_iterator it, end = groups.end();
  size_t i = 0;
  bool bFirst = true, bPrintHeader = groups.size()>1;
  for(it = groups.begin(); it != end; ++it) {
    if(bFirst) bFirst = false;
    else cerr << endl;
    if(bPrintHeader)
      cerr << "==================Group " << ++i << "==================" << endl;
    curGroup = *it;
    cur_assignments = &curGroup->assignments;
    cur_conditions = &curGroup->conditions;
    vector<Gate*>::const_iterator it, end = curGroup->end();
    size_t i = 0;
    for(it = curGroup->begin(); it != end; ++it) {
      cerr << "Gate " << ++i << ":" << endl;
      cur_daes = &(*it)->daes;
      print_daes();
    }
    cerr << endl;
    print_assignments();
    print_conditions();
    if(bPrintHeader)
      cerr << "=============================================" << endl;
  }
  cerr << endl;
  print_events();
  print_numbers();
  cerr << endl;
}

inline void print_results() {
  if(bMult) {
    if(++curMult < nMult) return;
    curMult = 0;
  }
  cout << t;
  static vector<const Number*>::const_iterator it, end = numbers.end();
  static vector<size_t>::const_iterator rep;
  size_t n = 0, repeat = 1;
  for(it = numbers.begin(), rep = lengths.begin(); it != end; ++it)
    if(bSuf) { //print digital values
      if(++n == repeat) {
        repeat = *rep;
        ++rep;
        n = 0;
        cout << "\t";
      }
      cout << logic_cast(**it);
    }
    else cout << "\t" << **it; //print analog values
  cout << endl;
}

string hr(Number size) { //return memory usage in human-readable form
  static long pagesize = sysconf(_SC_PAGE_SIZE);
  static char prefix[] = " kMGTPEZY";
  size_t n = 0;
  size *= pagesize;
  while(size >= 1024 && n < 8) {
    size /= 1024;
    n++;
  }
  char pref = prefix[n];
  stringstream ss;
  ss.precision(2);
  ss << fixed << size << (pref==' '? "": " ") << pref << "B";
  return ss.str();
}

void mark_mem_sz() { //mark memory usage if it increases
  ifstream statm("/proc/self/statm");
  Number total = 0, resident = 0, shared = 0;
  statm >> total >> resident >> shared;
  Number size = resident-shared;
  if(size > totalMem) totalMem = size;
}

void print_stats() {
  mark_mem_sz();
  cerr << "Maximal order: " << MAXORD << endl;
  cerr << "Number of threads: " << nThreads+1 << endl;
  cerr << "Algebraic equations: " << Dae::algs() << endl;
  cerr << "Differential equations: " << Dae::odes() << endl;
  cerr << "Number of inverters: " << Term::invs() << endl;
  cerr << "Number of NANDs: " << Term::nands() << endl;
  cerr << "Number of NORs: " << Term::nors() << endl;
  cerr << "Number of gates: " << Term::gates() << endl;
  cerr << "Number of transistors: " << Term::trans() << endl;
  cerr << "Used memory: " << hr(totalMem) << endl;
  cerr << "Clock time: " << (Number)clock()/CLOCKS_PER_SEC << " s" << endl;
  cerr << "Execution time: " << microtime()-t0 << " s" << endl;
}

void perform_conditions() { //perform the transistor-input changes
  vector<Condition*>::const_iterator it, end = cur_changed->end();
  for(it = cur_changed->begin(); it != end; ++it) (*it)->eval();
  cur_changed->clear();
}

void eval_conditions(vector<ConditionCh*> *all) {
  vector<ConditionCh*>::const_iterator it, end = all->end();
  for(it = all->begin(); it != end; ++it) (*it)->eval();
}

void eval_conditions(vector<ConditionCh*> *all, vector<Condition*> *changed) {
  vector<ConditionCh*>::const_iterator it, end = all->end();
  for(it = all->begin(); it != end; ++it) (*it)->eval(*changed);
}

inline void eval_pwl() { //evaluate piece-wise linear inputs (e.g. 1, 1, 0)
  static deque<Event>::iterator it = events.begin(), end = events.end();
  while(it != end && it->active()) {
    it->eval();
    ++it;
  }
}

void preinit_threads() {
  if(nThreads > DEFAULT_MAX_THREADS) nThreads = DEFAULT_MAX_THREADS;
  if(nThreads == 1) nThreads = sysconf(_SC_NPROCESSORS_ONLN);
  bThreaded = nThreads>1;
}

//init constant parts of Taylor polynomials (inputs x order):
void init_mults(vector<vector<Number> > &mults) {
  mults.reserve(maxInputs);
  for(size_t i = 0; i < maxInputs; ++i) {
    mults.push_back(vector<Number>());
    vector<Number> &vec = mults.back();
    vec.reserve(DEFAULT_MINCOEFF);
    vec.push_back(coeff[i]);
  }
}

void init_threads() {
  if(bThreaded && nThreads > groups.size()) {
    nThreads = groups.size();
    bThreaded = nThreads>1;
  }
  if(bThreaded) {
    threads.reserve(nThreads); //start threads:
    for(size_t i = 0; i < nThreads; i++) threads.add(new Worker);
    threads.run(); //run them (they wait for load)
    deque<Group*>::const_iterator it, end = groups.end();
    for(it = groups.begin(); it != end; ++it) { //init
      Group *group = *it;
      assign(&group->assignments);
      eval_conditions(&group->conditions);
    }
  }
  else {
    nThreads = 0; //single-threaded
    cur_mults = new vector<vector<Number> >;
    init_mults(*cur_mults); //init
    assign(cur_assignments);
    eval_conditions(cur_conditions);
  }
}

//init constant parts of Taylor polynomials for the first order:
void init_coeff() {
  coeff.reserve(maxInputs);
  for(size_t i = 1; i <= maxInputs; i++) coeff.push_back(Cinv*dt/i);
}

bool init() {
  extern int error;
  error = 0;
  yyparse();
  yylex_destroy();
  if(error) return false;
  if(isnanl(ONE)) ONE = -U/2; //default logical-one threshold (U is negative)
  if(mult > 0) { //print results only in multiplies of time
    nMult = roundl(mult/dt);
    bMult = nMult>1;
  }
  Expr::transform();
  Term::make_instr();
  sort(events.begin(), events.end());
  init_coeff();
  init_threads();
  return true;
}

inline void par_taylor() { //parallel solver
  static deque<Group*>::const_iterator it, end = groups.end();
  for(it = groups.begin(); it != end; ++it) Worker::send2any(*it);
  Worker::wait4all();
  for(it = groups.begin(); it != end; ++it) (*it)->perform_conditions();
}

inline void ser_taylor() { //serial solver
  size_t ORD = curGroup->solve(*cur_mults);
  if(ORD > MAXORD) MAXORD = ORD;
  perform_conditions();
}

size_t taylor(vector<vector<Number> > &mults, Gate *gate) { //solve one gate
  bool bCont;
  size_t n = 0, ORD = 1;
  vector<Dae*> &daes = gate->daes;
  vector<Dae*>::const_iterator it, end = daes.end();
  for(it = daes.begin(); it != end; ++it) (*it)->first_term();
  do {
    bCont = false;
    for(it = daes.begin(); it != end; ++it) {
      Dae *dae = *it;
      dae->eval_term(mults, ORD);
      if(dae->is_ode()) {
        dae->add_term();
        if(ABS(dae->term()) > EPS) { //reset counter if absolute val. is greater
          bCont = true;
          n = 0;
        }
      }
    } //continue until enough absolute values are less than or equal EPS:
    if(!bCont && ++n < TEST) bCont = true;
    ORD++;
  } while(bCont);
  return --ORD; //ORD incremented once more than it should
}

bool solve() {
  t0 = microtime();
  if(!init()) return false;
  print_header();
  print_results();
  while(t <= tmax) {
    eval_pwl(); //reflect changed piece-wise linear inputs (e.g. 1, 1, 0)
    if(bThreaded) par_taylor();
    else ser_taylor();
    t += dt;
    print_results();
  }
  if(bDebug) print_debug();
  print_stats();
  return true;
}
