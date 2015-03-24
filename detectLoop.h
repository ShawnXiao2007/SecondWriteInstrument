#include "common.h"

#ifndef LOOPDETECTOR_H
#define LOOPDETECTOR_H
using namespace llvm;
using namespace std;

class LoopDetector{
public:
  void displayType1Loops();
  void extractType1Loops();

  void displayType2Loops();
  void extractType2Loops();

  LoopDetector(Function& F);
  
  int getNumOfBBLs();
  
  void extractBackedges();
  int getNumOfBackedges();

private:
  Function &__F;
  //a loop of a single BBL
  unordered_set< shared_ptr<vector<BasicBlock const *>> > __type1Loops;
  //a loop of several BBLs without branches
  unordered_set< shared_ptr<vector<BasicBlock const *>> > __type2Loops;
  //backedges
  vector<std::pair<const BasicBlock*,const BasicBlock*> > __backedges;
  //find backedges
  void __FindFunctionBackedges(const Function &F, vector<std::pair<const BasicBlock*,const BasicBlock*> > &Result); 
};
#endif
