#include "detectLoop.h"

LoopDetector::LoopDetector(Function& F):__F(F){}

void LoopDetector::displayType1Loops(){
  errs()<<__F.getName()<<":\t\tNumber of Type 1 loops: "<<__type1Loops.size()<<"  out of "<<this->getNumOfBBLs()<<" BBLs\n";
}

int LoopDetector::getNumOfBBLs(){
  int retVal=0;
  for(Function::iterator i=__F.begin(), e=__F.end(); i!=e; i++){
    retVal++;
  }
  return retVal;
}

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
