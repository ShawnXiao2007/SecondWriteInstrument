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
    for(auto j=pBBL->begin(), j_e=pBBL->end(); j!=j_e; j++){
      Instruction* l=j;
      if(CallInst* callInst=dyn_cast<CallInst>(l)){
        BasicBlock::iterator k=j;
        k++;
        Instruction* next=k;
        __instCallInst(callInst, next);
      }
    }
  }
}

void SlimInst::__instType1LoopBBL(Function& F, BasicBlock * pBBL, unsigned short loopID){
  assert(!pBBL->isLandingPad());
  //create a basic block
  LLVMContext& context=F.getContext();
  BasicBlock* newBBL=BasicBlock::Create(context, "StraightTaint_Type1Loop", &F, pBBL);
  //log loopID, create a number, terminate
  CallInst::Create(__pMbr->log, ConstantInt::get(__pMbr->shortTy, loopID),  "inst1_logLoopID", newBBL);
  AllocaInst* inst2_var=new AllocaInst(__pMbr->intTy, nullptr, "inst2_var", newBBL);
  BranchInst* inst3_br=BranchInst::Create(pBBL, newBBL);
  //insert an instruction to pBBL to increment the inst3_load
  auto insertPt_increment=pBBL->getFirstInsertionPt();
  Instruction* insertBefore_increment=insertPt_increment;
  IRBuilder<> builder(insertBefore_increment);
  Value* counter=builder.CreateLoad(inst2_var);
  Value* CI_one=ConstantInt::get(__pMbr->intTy, 1);
  Value* add_one=builder.CreateAdd(counter, CI_one);
  builder.CreateStore(add_one, inst2_var);
  //get predecessor and change target
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
  //get successor
  TerminatorInst* termi=pBBL->getTerminator();
  int numSuccs=termi->getNumSuccessors();
  for(int i=0; i<numSuccs; i++){
    BasicBlock* succ=termi->getSuccessor(i);
    if(succ==pBBL){
      continue;
    }else{
      //create a post block
      BasicBlock* newBBL_post=BasicBlock::Create(context, "StraightTaint_Type1Loop_post", &F, succ);
      IRBuilder<> builder_postBlock(newBBL_post);
      //call a inline function and write the counter to buffer
      counter=builder_postBlock.CreateLoad(inst2_var);
      builder_postBlock.CreateCall(__pMbr->logCounter, counter);
      //insert a br inst and jump to the succ
      builder_postBlock.CreateBr(succ);
      termi->setSuccessor(i, newBBL_post);
    }
  }
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

ModuleMembers::ModuleMembers(Module& M): 
  __M(M), voidTy(NULL), shortTy(NULL), intTy(NULL), longTy(NULL), 
  ptr16Ty(NULL), ptr32Ty(NULL), 
  log(NULL), logCounter(NULL), init(NULL), fork32(NULL), fork64(NULL), fkill(NULL), ffflush(NULL), eval(NULL),
  gvar_addr(NULL)
{
  //voidTy
  voidTy = Type::getVoidTy(M.getContext());
  //shortTy
  shortTy = IntegerType::get(M.getContext(), 16);
  //intTy
  intTy = IntegerType::get(M.getContext(), 32);
  //longTy
  longTy = IntegerType::get(M.getContext(), 64);
  //ptr16Ty
  ptr16Ty = PointerType::get(IntegerType::get(M.getContext(), 16), 0); 
  //ptr32Ty
  ptr32Ty = PointerType::get(ptr16Ty, 0); 
  //log
  Constant * c = M.getOrInsertFunction("_StraightTaint_log", voidTy, shortTy, NULL);
  log = cast<Function>(c);
  //logCounter
  Constant * cCounter = M.getOrInsertFunction("_StraightTaint_logCounter", voidTy, intTy, NULL);
  logCounter = cast<Function>(cCounter);
  //init
  Constant * cInit = M.getOrInsertFunction("_StraightTaint_init", ptr16Ty, ptr32Ty, NULL);
  init = cast<Function>(cInit);
  //fork32 and fork64
  Constant * f32=M.getOrInsertFunction("_StraightTaint_fork32", voidTy, intTy, NULL);
  fork32 = cast<Function>(f32);
  Constant * f64=M.getOrInsertFunction("_StraightTaint_fork64", voidTy, longTy, NULL);
  fork64 = cast<Function>(f64);
  //kill()
  Constant* fKillConst=M.getOrInsertFunction("kill",intTy,intTy,intTy,NULL);
  fkill=cast<Function>(fKillConst);
  //fflush()
  Constant * ff=M.getOrInsertFunction("_StraightTaint_flush", voidTy, shortTy, NULL);
  ffflush = cast<Function>(ff);
  //eval()
  Constant * e=M.getOrInsertFunction("_StraightTaint_eval", voidTy, intTy, shortTy, NULL);
  eval = cast<Function>(e);
  //globals
  gvar_addr=new GlobalVariable(M,
                              ptr16Ty,
                              false,
                              GlobalValue::CommonLinkage,
                              ConstantPointerNull::get(ptr16Ty),
                              "addr");
  //check all the members
  assert(checkRep());
}

bool ModuleMembers::checkRep(){
  assert(shortTy);
  assert(voidTy);
  assert(intTy);
  assert(ptr16Ty);
  assert(ptr32Ty);

  assert(log);
  assert(logCounter);
  assert(init);
  assert(gvar_addr);
  return true;
}

void ModuleMeta::displayStatInfo(){
  errs()<<"Num of BBLs: "<<__numBBLs<<"\n";
  errs()<<"Num of Loops: "<<__numLoops<<"\n";
  errs()<<"Num of Type1Loops: "<<__numType1Loops<<"\n";
  errs()<<"Num of Funcs: "<<__numFuncs<<"\n";
}

bool SlimInst::run(){
  string fstart("start");
  string fmain("main");
  bool initDone=false;
  for(auto i=__M.begin(), i_e=__M.end(); i!=i_e; i++){
    Function& F=*i;
    if(F.size()==0){
      continue;
    }
    __instFunc(&F);
    
    string fname(F.getName().data());
    if(fname==fstart || fname==fmain){
      assert(initDone==false);
      __instMainOrStartFuncEntryBBL(F);
      initDone=true;
    }
  }
  assert(initDone==true);
  return true;
}

void SlimInst::__instMainOrStartFuncEntryBBL(Function& F){
  BasicBlock* entry=&(F.front());
  Instruction* firstInst=&(entry->front());
  IRBuilder<> builder(firstInst); 
  CallInst* callInit=builder.CreateCall(__pMbr->init, __pMbr->gvar_addr); 
  builder.CreateStore(callInit, __pMbr->gvar_addr);
}

void ModuleMeta::outputModuleMetaToFile(){
  for(auto i=FunctionName2ID.begin(), i_e=FunctionName2ID.end(); i!=i_e; i++){
    string fname=i->first;
    int fid=i->second;
    errs()<<fname.c_str()<<"\t"<<fid<<"\n";
    auto &tmp=BBLID2Name[i->first];
    auto &tmp2=LoopID2Type[i->first];
    for(auto j=tmp.begin(), j_e=tmp.end(); j!=j_e; j++){
      errs()<<"\t"<<j->first<<"\t"<<j->second.c_str()<<"\n";
    }
    for(auto k=tmp2.begin(), k_e=tmp2.end(); k!=k_e; k++){
      errs()<<"\t"<<k->first<<"\t"<<k->second<<"\n";
    }
  }
}

void SlimInst::__instCallInst(CallInst* callInst, Instruction* next){
  Function* f=callInst->getCalledFunction();
  string fname(f->getName().data());
  if(fname==string("fork") ||
      (fname==string("syscall") && ((ConstantInt*)(callInst->getArgOperand(0)))->getZExtValue()==__NR_clone )  ){
    IRBuilder<> builder(next);
    if(callInst->getType()->isIntegerTy(32)){
      builder.CreateCall(__pMbr->fork32, callInst);
    }else if(callInst->getType()->isIntegerTy(64)){
      builder.CreateCall(__pMbr->fork64, callInst);
    }else{
      assert(0);
    }
  }else if(fname==string("open") || fname==string("fopen") || fname==string("accept") || fname==string("socket") || fname==string("open64")){
    IRBuilder<> builder(next);
    Constant* pidArg=ConstantInt::get(__pMbr->intTy, 20149999);
    Constant* sigArg=ConstantInt::get(__pMbr->intTy, 0);
    builder.CreateCall2(__pMbr->fkill, pidArg, sigArg);
  }
}
