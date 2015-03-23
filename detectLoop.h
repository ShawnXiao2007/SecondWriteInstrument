#include "common.h"

#ifndef LOOPDETECTOR_H
#define LOOPDETECTOR_H
using namespace llvm;
using namespace std;

class LoopDetector{
public:
  void displayType1Loops();
  void extractType1Loops();
  LoopDetector(Function& F);
private:
  Function &__F;
  unordered_set< shared_ptr<vector<BasicBlock*>> > __type1Loops;
};
#endif
