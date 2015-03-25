#include "common.h"

#ifndef SLIMINST_H
#define SLIMINST_H
using namespace llvm;
using namespace std;

class SlimInst{
public:
  SlimInst(Module &M):__M(M){};

  //output max number of functions and basic blocks
  void getNumFuncAndBBL();

  //

private:
  Module &__M;
};
#endif
