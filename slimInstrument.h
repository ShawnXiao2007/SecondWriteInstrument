#include "common.h"

#ifndef SLIMINST_H
#define SLIMINST_H
using namespace llvm;
using namespace std;

class SlimInst{
public:
  SlimInst(Module &M):__M(M){};
private:
  Module &__M;
};
#endif
