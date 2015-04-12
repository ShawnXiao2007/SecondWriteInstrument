#include "common.h"

#ifndef LOOPDETECTOR_H
#define LOOPDETECTOR_H
using namespace llvm;
using namespace std;

class LoopDetector{
public:
  void displayType1Loops();
  shared_ptr< unordered_set< BasicBlock const * > > getType1Loops();

  void displayType2Loops();
  shared_ptr< unordered_set< BasicBlock const * > > getType2Loops();

  shared_ptr< unordered_set< BasicBlock const * > > getLoopBBLs();

  LoopDetector(Function& F);
  
  int getNumOfBBLs();
  int getNumOfBackedges();

private:
  Function &__F;
  //a loop of a single BBL
  unordered_set< shared_ptr<vector<BasicBlock const *>> > __type1Loops;
  //a loop of several BBLs without branches
  unordered_set< shared_ptr<vector<BasicBlock const *>> > __type2Loops;
  //backedges
  vector< pair<const BasicBlock*,const BasicBlock*> > __backedges;
  //find backedges
  void __FindFunctionBackedges(const Function &F, vector<std::pair<const BasicBlock*,const BasicBlock*> > &Result); 
  void __extractType1Loops();
  void __extractType2Loops();
  void __extractBackedges();
};
#endif
