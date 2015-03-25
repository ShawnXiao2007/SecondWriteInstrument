#include "common.h"

#ifndef SLIMINST_H
#define SLIMINST_H
using namespace llvm;
using namespace std;

class ModuleMeta{
public:
  ModuleMeta(Module &M):__M(M), __maxF(0), __maxB(0), __pFofMaxB(0){};
  //output the meta information
  //Function (or BBL or Loop) ID is a numerical ID
  //For BBL without name, set one
  //Function ID : Function Name
  //Function ID : {<BBL ID : BBL Name>}
  //Function ID : {<Loop ID : Loop Type>}
  void outputToFile();

  Module &__M;

  unsigned long __maxF;
  unsigned long __maxB;
  unsigned long __numLoops;
  Function const * __pFofMaxB;
  void __initMaxFandB();
  
  map<int, string> FunctionID2Name;
  map<int, map<int, string>> BBLID2Name;
  map<int, map<int, int>> LoopID2Type;


  void initFunctionID2Name();
};

class SlimInst{
public:
  SlimInst(Module &M):__M(M){};

  //output max number of functions and basic blocks
  void getNumFuncAndBBL();

  //run the instrumentation
  void run();

private:
  Module &__M;
  ModuleMeta const* __pMeta;
  //insert instructions to entry block
  //insert instructions to non-loop block
  //insert basic blocks before and after loop blocks
  void __instFunc(Function * F);

  //instrumentation
  void __instEntryBBL(Function * F);
  void __instNonLoopBBL(BasicBlock * BBL);
  void __instType1LoopBBL(BasicBlock * BBL);
};
#endif
