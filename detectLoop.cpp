#include "detectLoop.h"

LoopDetector::LoopDetector(Function& F):__F(F), __backedges(0){}

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

void LoopDetector::extractBackedges(){
  if(__F.size()){
    __FindFunctionBackedges(__F, __backedges);
  }else{
  }
}

/// FindFunctionBackedges - Analyze the specified function to find all of the
/// loop backedges in the function and return them.  This is a relatively cheap
/// (compared to computing dominators and loop info) analysis.
///
/// The output is added to Result, as pairs of <from,to> edge info.
void LoopDetector::__FindFunctionBackedges(const Function &F,
     vector<std::pair<const BasicBlock*,const BasicBlock*> > &Result) {
  const BasicBlock *BB = &F.getEntryBlock();
  if (succ_begin(BB) == succ_end(BB))
    return;

  SmallPtrSet<const BasicBlock*, 8> Visited;
  SmallVector<std::pair<const BasicBlock*, succ_const_iterator>, 8> VisitStack;
  SmallPtrSet<const BasicBlock*, 8> InStack;

  Visited.insert(BB);
  VisitStack.push_back(std::make_pair(BB, succ_begin(BB)));
  InStack.insert(BB);
  do {
    std::pair<const BasicBlock*, succ_const_iterator> &Top = VisitStack.back();
    const BasicBlock *ParentBB = Top.first;
    succ_const_iterator &I = Top.second;

    bool FoundNew = false;
    while (I != succ_end(ParentBB)) {
      BB = *I++;
      if (Visited.insert(BB)) {
        FoundNew = true;
        break;
      }
      // Successor is in VisitStack, it's a back edge.
      if (InStack.count(BB))
        Result.push_back(std::make_pair(ParentBB, BB));
    }

    if (FoundNew) {
      // Go down one level if there is a unvisited successor.
      InStack.insert(BB);
      VisitStack.push_back(std::make_pair(BB, succ_begin(BB)));
    } else {
      // Go up one level.
      InStack.erase(VisitStack.pop_back_val().first);
    }
  } while (!VisitStack.empty());
}

int LoopDetector::getNumOfBackedges(){
  return __backedges.size();
}
