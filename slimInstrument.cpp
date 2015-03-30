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
    //update __numFuncs
    __numFuncs++;

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
    //insert the function to BBLID2Addr
    assert(BBLID2Addr.find(fName)==BBLID2Addr.end() );
    BBLID2Addr.insert(pair<string, map<int, BasicBlock*> >(fName, map<int, BasicBlock*>() ) );
    map<int, BasicBlock*> &tmp3=BBLID2Addr[fName];
    //Use a loop detector to detect the loops
    LoopDetector LD(*i);
    auto loops=LD.getType1Loops();
    //update num Loops and Type1Loops
    __numLoops+=LD.getNumOfBackedges();
    __numType1Loops+=loops->size();

    for(auto j=i->begin(), j_e=i->end(); j!=j_e; j++){
      //update num BBLs
      __numBBLs++;

      BasicBlock& bbl=*j;
      string bblName(j->getName().data());
      tmp.insert(pair<int,string>(id, bblName));
      if(loops->find(&bbl)!=loops->end()){
        tmp2[id]=1;
      }
      
      tmp3.insert(pair<int, BasicBlock*>(id, &bbl));
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
  if(F->size()==0){
    return;
  }
  BasicBlock& entryBBL=F->getEntryBlock(); 
  //get function ID
  string fName(F->getName().data());
  auto tmp=__pMeta->FunctionName2ID;
  unsigned short FuncID=tmp.find(fName)->second;
  //get addr 2 bbid
  map<BasicBlock*, unsigned short> addr2bbid;
  auto& FBBLID2Addr=__pMeta->BBLID2Addr;
  const map<int, BasicBlock*> & bbid2addr=FBBLID2Addr.find(fName)->second;
  for(auto i=bbid2addr.begin(), i_e=bbid2addr.end(); i!=i_e; i++){
    addr2bbid.insert(make_pair(i->second, i->first));
  }
  //get LoopID 2 Type
  const map<int, int>& loopID2Type=__pMeta->LoopID2Type.find(fName)->second;
  //instrument basic blocks one by one
  for(auto i=F->begin(), i_e=F->end(); i!=i_e; i++){
    BasicBlock* pBBL=i;
    unsigned short iID=addr2bbid[pBBL];
    if(pBBL==&entryBBL){
      __instLogBBL(pBBL, FuncID);      
    }else if(loopID2Type.find(iID)==loopID2Type.end()){
      //non loop
      __instLogBBL(pBBL, iID);
    }else{
      //loop

    }
  }
}

void SlimInst::__instType1LoopBBL(Function& F, BasicBlock * pBBL, unsigned short loopID){
  assert(!pBBL->isLandingPad());
  //create a basic block
  LLVMContext& context=F.getContext();
  BasicBlock* newBBL=BasicBlock::Create(context, "", &F, pBBL);
  //log loopID, create a number, terminate
  CallInst::Create(__pMbr->log, ConstantInt::get(__pMbr->shortTy, loopID),  "inst1_logLoopID", newBBL);
  AllocaInst* inst2_var=new AllocaInst(__pMbr->intTy, nullptr, "inst2_var", newBBL);
  BranchInst* inst3_br=BranchInst::Create(pBBL, newBBL);
  //get predecessor
  for(auto i=pred_begin(pBBL), i_e=pred_end(pBBL); i!=i_e; i++){
    BasicBlock* pred=*i;
    if(pred==pBBL){
      continue;//self loop
    }else{
      TerminatorInst* ti=pBBL->getTerminator();
      if(BranchInst* bi=dyn_cast<BranchInst>(ti)){
       unsigned int numSucc=bi->getNumSuccessors();
       unsigned int j=0;
       for(; j<numSucc; j++){
        BasicBlock* jSucc=bi->getSuccessor(j);
        if(jSucc==pBBL){
          break;
        }
       }
       assert(j<numSucc);
       bi->setSuccessor(j, newBBL);
      }else if(IndirectBrInst* ibi=dyn_cast<IndirectBrInst>(ti)){
      }else if(SwitchInst* si=dyn_cast<SwitchInst>(ti)){
        errs()<<"A special terminator inst: \n"<<*ti<<"\n";
        assert(0);
      }else if(InvokeInst* ii=dyn_cast<InvokeInst>(ti)){
        errs()<<"A special terminator inst: \n"<<*ti<<"\n";
        assert(0);
      }else if(ResumeInst* ri=dyn_cast<ResumeInst>(ti)){
        errs()<<"A special terminator inst: \n"<<*ti<<"\n";
        assert(0);
      }else{
        errs()<<"A special terminator inst: \n"<<*ti<<"\n";
        assert(0);
      }
    }
  }
  //change target
  //get successor
  //create a block and jump to the successor
  //
}

void SlimInst::__instLogBBL(BasicBlock* pBBL, unsigned short BBID){
  BasicBlock& BBL=*pBBL;
  auto insertPoint=BBL.getFirstInsertionPt();
  Instruction* first=insertPoint;
  //get function ID
  //insert instruction
  CallInst::Create(__pMbr->log, ConstantInt::get(__pMbr->shortTy, BBID),  "", first);
}

ModuleMembers::ModuleMembers(Module& M): __M(M), voidTy(NULL), shortTy(NULL), log(NULL){
  //voidTy
  voidTy = Type::getVoidTy(M.getContext());
  //shortTy
  shortTy = IntegerType::get(M.getContext(), 16);
  //intTy
  intTy = IntegerType::get(M.getContext(), 32);
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

void ModuleMeta::displayStatInfo(){
  errs()<<"Num of BBLs: "<<__numBBLs<<"\n";
  errs()<<"Num of Loops: "<<__numLoops<<"\n";
  errs()<<"Num of Type1Loops: "<<__numType1Loops<<"\n";
  errs()<<"Num of Funcs: "<<__numFuncs<<"\n";
}
