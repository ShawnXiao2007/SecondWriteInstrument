#ifndef BBLTRACE_H
#define BBLTRACE_H
#include "common.h"
#include "detectLoop.h"
using namespace std;
using namespace llvm;

class BBLTrace{
  public:
    BBLTrace(string fname);
    map<unsigned, unsigned> bblid2num;
    vector<unsigned> rank;
  private:
    string __fname;
    void __initEverything();
    void __initBblid2num();
    void __initRank();   
    static bool __myComp(pair<unsigned, unsigned> lhs, pair<unsigned, unsigned> rhs); 
};

class BBLAnalyzer{
  public:
    BBLAnalyzer(BBLTrace const * pBT, string bcfname);
    void write2file(string outfname);
    pair<string, unsigned> getBBLInfo(unsigned bblid);
  private:
    map<unsigned, pair<string, unsigned>> __bblinfo;
    map<unsigned, BasicBlock*> __bblid2addr;
    string __bcfname;
    BBLTrace const* __pBT;

    void __initEverything();
    unsigned __getBblid(BasicBlock const* bbl);
};
#endif
