#include "detectLoop.h"

LoopDetector::LoopDetector(Function& F):__F(F){}

void LoopDetector::displayType1Loops(){}

void LoopDetector::extractType1Loops(){
  for(Function::iterator i=__F.begin(), e=__F.end(); i!=e; i++){
    BasicBlock& bb=*i;
    TerminatorInst const * TInst=bb.getTerminator();
    for(int j=0, j_n=TInst->getNumSuccessors(); j<j_n; j++){
      BasicBlock* succ=TInst->getSuccessor(j);
      if(succ==&bb){
        __type1Loops.insert( shared_ptr<vector<BasicBlock*>>(new vector<BasicBlock*>(1,succ)) );
        break;
      }
    }
  }
}
