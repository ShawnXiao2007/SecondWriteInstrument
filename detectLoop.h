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
  int getNumOfBBLs();
private:
  Function &__F;
  //a loop of a single BBL
  unordered_set< shared_ptr<vector<BasicBlock*>> > __type1Loops;
  //a loop of several BBLs without branches
  unordered_set< shared_ptr<vector<BasicBlock*>> > __type2Loops;
};
#endif
