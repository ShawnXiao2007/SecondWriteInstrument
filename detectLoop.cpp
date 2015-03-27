#include "detectLoop.h"

LoopDetector::LoopDetector(Function& F):__F(F), __backedges(0){}

void LoopDetector::displayType1Loops(){
  errs()<<__F.getName()<<":\t\tNumber of Type 1 loops: "<<__type1Loops.size()<<"  out of "<<this->getNumOfBBLs()<<" BBLs\n";
  for(auto i = __type1Loops.begin(), e=__type1Loops.end(); i!=e; i++){
    auto tmp=*i;
    assert(tmp->size()==1);
    errs()<<*((*tmp)[0])<<"\n";
  }
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
      BasicBlock const * succ=TInst->getSuccessor(j);
      if(succ==&bb){
        __type1Loops.insert( shared_ptr<vector<BasicBlock const*>>(new vector<BasicBlock const*>(1,succ)) );
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

void LoopDetector::displayType2Loops(){
  errs()<<__F.getName()<<":\t\tNumber of Type 2 loops: "<<__type2Loops.size()<<"  out of "<<this->getNumOfBBLs()<<" BBLs\n";
}

void LoopDetector::extractType2Loops(){
  for(auto it=__backedges.begin(), e=__backedges.end(); it!=e; it++){
    BasicBlock const * loopEnd=it->first;
    BasicBlock const * loopStart=it->second;
    if(loopEnd==loopStart){
      //type 1 loop
      continue;
    }
    BasicBlock const * tmp=loopStart;
    while(tmp->getTerminator()->getNumSuccessors()==1){
      tmp=tmp->getTerminator()->getSuccessor(0);
    }
    if(tmp==loopEnd){
      shared_ptr< vector<BasicBlock const*> > tmp2(new vector<BasicBlock const*>() );
      tmp2->push_back(loopStart);
      tmp2->push_back(loopEnd);
      __type2Loops.insert(tmp2);
    }
  }
}

shared_ptr< unordered_set< BasicBlock const *  >> LoopDetector::getType1Loops(){
  shared_ptr< unordered_set< BasicBlock const *> > retVal(new unordered_set< BasicBlock const *>() );
  for(auto i=__type1Loops.begin(), i_e=__type1Loops.end(); i!=i_e; i++){
    auto v=*(*i);
    assert(v.size()!=0);
    retVal->insert(v[0]);
  }
  return retVal;
}

