#ifndef BBLTRACE_H
#define BBLTRACE_H
#include "common.h"
#include "detectLoop.h"
using namespace std;
using namespace llvm;

class BBLTrace{
  public:
    BBLTrace(string fname):__fname(fname){};
    map<unsigned, unsigned> bblid2num;
    vector<unsigned> rank;
  private:
    string __fname;
    void __initEverything();
    void __initBblid2num();
    void __initRank();   
    static bool __myComp(pair<unsigned, unsigned> lhs, pair<unsigned, unsigned> rhs); 
};
#endif
