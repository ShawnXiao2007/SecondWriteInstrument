#include "slimInstrument.h"

void SlimInst::displayNumFuncAndBBL(){
  assert(__pMeta); 
  errs()<<"Num of Func: "<<__pMeta->__maxF<<"\nMax num of BBL: "<<__pMeta->__maxB<<" of function: "<<__pMeta->__pFofMaxB->getName()<<"\n";
}

void ModuleMeta::__initMaxFandB(){
  unsigned long maxF=0;
  unsigned long maxB=0;
  Function const *p=NULL;
  for(auto i=__M.begin(), i_e=__M.end(); i!=i_e; i++){
    Function & tmpF=*i;
    if(tmpF.size()==0){
      continue;
    }
    maxF++;
    unsigned long numBofF=0;
    for(auto j=tmpF.begin(), j_e=tmpF.end(); j!=j_e; j++){
      numBofF++;
    }
    if(numBofF>maxB){
      maxB=numBofF;
      p=&tmpF;
    }
  }
  __maxF=maxF;
  __maxB=maxB;
  __pFofMaxB=p;
}

void ModuleMeta::__initFunctionName2ID(){
  int id=10;
  for(auto i=__M.begin(), i_e=__M.end(); i!=i_e; i++){
    Function& F=*i;
    if(F.size()==0){
      continue;
    }
    string fname(F.getName().data());
    assert(FunctionName2ID.find(fname)==FunctionName2ID.end());
    FunctionName2ID.insert(pair<string,int>(fname, id));
    id++;
  }
  assert(id=10+__maxF);
}

void ModuleMeta::__initBBLID2Name(){
  for(auto i=__M.begin(), i_e=__M.end(); i!=i_e; i++){
    int id=10+__maxF;
    if(i->size()==0){
      continue;
    }
    string fName(i->getName().data());
    //insert the function to BBLID2Name
    assert(BBLID2Name.find(fName)==BBLID2Name.end());
    BBLID2Name.insert(pair<string, map<int, string>>(fName, map<int,string>()) );
    map<int, string> &tmp=BBLID2Name[fName];
    //insert the function to LoopID2Type
    assert(LoopID2Type.find(fName)==LoopID2Type.end());
    LoopID2Type.insert(pair<string, map<int,int>>(fName, map<int,int>()));
    map<int, int> &tmp2=LoopID2Type[fName];
    //Use a loop detector to detect the loops
    LoopDetector LD(*i);
    auto loops=LD.getType1Loops();
    
    for(auto j=i->begin(), j_e=i->end(); j!=j_e; j++){
      BasicBlock& bbl=*j;
      string bblName(j->getName().data());
      tmp.insert(pair<int,string>(id, bblName));
      if(loops->find(&bbl)!=loops->end()){
        tmp2[id]=1;
      }
      id++;
    }
  }
}

void ModuleMeta::__initEverything(){
  __initMaxFandB();
  __initFunctionName2ID();
  __initBBLID2Name();
}

void SlimInst::__instFunc(Function *  F){
  __instEntryBBL(F);
  __instType1LoopBBL(NULL);
}

void SlimInst::__instEntryBBL(Function * F){
  BasicBlock& entryBBL=F->getEntryBlock();
  auto insertPoint=entryBBL.getFirstInsertionPt();
  Instruction* first=insertPoint;
  //get function ID
  string fName(F->getName().data());
  auto tmp=__pMeta->FunctionName2ID;
  unsigned short BBID=tmp.find(fName)->second;
  //insert instruction
  CallInst::Create(__pMbr->log, ConstantInt::get(__pMbr->shortTy, BBID),  "", first);
  
}
void SlimInst::__instType1LoopBBL(BasicBlock * BBL){
}
void SlimInst::__instNonLoopBBL(BasicBlock* BBL){
}

ModuleMembers::ModuleMembers(Module& M): __M(M), voidTy(NULL), shortTy(NULL), log(NULL){
  //voidTy
  voidTy = Type::getVoidTy(M.getContext());
  //shortTy
  shortTy = IntegerType::get(M.getContext(), 16);
  //log
  Constant * c = M.getOrInsertFunction("_StraightTaint_log", voidTy, shortTy, NULL);
  log = cast<Function>(c);
  //check all the members
  assert(checkRep());
}

bool ModuleMembers::checkRep(){
  assert(shortTy);
  assert(log);
  return true;
}
