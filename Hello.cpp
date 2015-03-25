#define DEBUG_TYPE "hello"

#include "common.h"
#include "detectLoop.h"
#include "slimInstrument.h"
using namespace llvm;
using namespace std;
const int BUF_SIZE=10*1024*1024;
const int objOffsetStart=0;
const int wholeObjSize=-2;  
const int logSysCallArgs=0;

//#define _HACK_LOG //use fprintf instead of shared memory to log BBID
//#define _EVAL_PTR
#define DEBUG_CallRet
#define DEBUG_DrawGraph
//#define DEBUG_DataSink
#define DEBUG_DataSource
//#define DEBUG_ShowGraph
namespace {

  struct StraightDFA_Inline : public ModulePass {
    static char ID;
    StraightDFA_Inline() : ModulePass(ID) {}

    virtual bool runOnModule(Module &M) {
      Type * VoidTy = Type::getVoidTy(M.getContext());
      Type * ShortTy = IntegerType::get(M.getContext(), 16);

      Constant * c = M.getOrInsertFunction("_StraightTaint_log", VoidTy, ShortTy, NULL);
      Function * log = cast<Function>(c);
      log->addFnAttr(llvm::Attribute::AlwaysInline);
      return true;
    }
  };
}
char StraightDFA_Inline::ID = 0;
static RegisterPass<StraightDFA_Inline> C("inline-log", "inline logging instructions");

namespace{
  struct StraightDFA_SlimInst : public ModulePass{
    static char ID;
    StraightDFA_SlimInst() : ModulePass(ID){};
    virtual bool runOnModule(Module &M){
      for(Module::iterator i=M.begin(), e=M.end(); i!=e; i++){
        Function& F=*i;
        LoopDetector LD(F);
        LD.extractType1Loops();
        //LD.displayType1Loops();
        LD.extractBackedges();
        //outs()<<"Number of backedges: "<<LD.getNumOfBackedges()<<"\n\n";
      }

      SlimInst SI(M);
      SI.getNumFuncAndBBL();

      return true;
    };
  };
}
char StraightDFA_SlimInst::ID = 0;
static RegisterPass<StraightDFA_SlimInst> B("slim-inst", "inline logging instructions");


namespace {

  struct StraightDFA_Inst : public ModulePass {
    static char ID;
    StraightDFA_Inst() : ModulePass(ID) {}

    virtual bool runOnModule(Module &M) {
      Type * VoidTy = Type::getVoidTy(M.getContext());
      Type * IntTy = IntegerType::get(M.getContext(), 32);
      Type * LongTy = IntegerType::get(M.getContext(), 64);
      Type * ShortTy = IntegerType::get(M.getContext(), 16);
      PointerType * Ptr16Ty = PointerType::get(IntegerType::get(M.getContext(), 16), 0);
      //PointerType * Ptr32Ty = PointerType::get(IntegerType::get(M.getContext(), 32), 0);
      PointerType * Ptr32Ty = PointerType::get(Ptr16Ty, 0);
      //declare global variable addr
      GlobalVariable * gvar_addr =
        new GlobalVariable(M,
            Ptr16Ty,
            false,
            GlobalValue::CommonLinkage,
            ConstantPointerNull::get(Ptr16Ty),
            "addr");
      //declare init function: short * __StraightTaint_init(short **)
      Constant * c = M.getOrInsertFunction("_StraightTaint_init", Ptr16Ty, Ptr32Ty, NULL);
      Function * init = cast<Function>(c);
      errs() << *init << "\n";
      //declare eval
      Constant * e=M.getOrInsertFunction("_StraightTaint_eval", VoidTy, IntTy, ShortTy, NULL);
      Function * eval = cast<Function>(e);
      errs() << *eval <<"\n";
      //declare clone/fork handling function
      Constant * ff=M.getOrInsertFunction("_StraightTaint_flush", VoidTy, ShortTy, NULL);
      Function * ffl = cast<Function>(ff);
      errs() << *ffl <<"\n";      
      Constant * f32=M.getOrInsertFunction("_StraightTaint_fork32", VoidTy, IntTy, NULL);
      Function * fork32 = cast<Function>(f32);
      errs() << *fork32 <<"\n";
      Constant * f64=M.getOrInsertFunction("_StraightTaint_fork64", VoidTy, LongTy, NULL);
      Function * fork64 = cast<Function>(f64);
      errs() << *fork64 <<"\n";
      //declare kill()
      Constant* fKillConst=M.getOrInsertFunction("kill",IntTy,IntTy,IntTy,NULL);
      Function* fKill=cast<Function>(fKillConst);
      errs()<< *fKill<<"\n";

#ifdef _HACK_LOG
      //declare log function: void _StraightTaint_log(void)
      c = M.getOrInsertFunction("_StraightTaint_log", VoidTy, ShortTy, NULL);
      Function * log = cast<Function>(c);
      errs() << *log << "\n";
#else
      //declare log function: void _StraightTaint_log(void)
      c = M.getOrInsertFunction("_StraightTaint_log", VoidTy, ShortTy, NULL);
      Function * log = cast<Function>(c);
#endif
      unsigned short BBID = 1; //starting from 1, reserve 0 for error report
      bool init_done = false;

      //DataLayout
//      DataLayout* DL=new DataLayout(&M);

      for (Module::iterator i = M.begin(), e = M.end(); i != e; ++i)
      {
        // insert logging logic for every basic block
         
        for (Function::iterator j = (*i).begin(), f = (*i).end(); j != f; ++j)
        {
          //insert _StraightTint_log()
          BasicBlock * BB = j;
          BasicBlock::iterator insertPoint = BB->getFirstInsertionPt();
          Instruction * first = insertPoint;
          CallInst::Create(log, ConstantInt::get(ShortTy, BBID), "", first);

          //insert _StraightTint_fork()
          for (BasicBlock::iterator k = (*BB).begin(), h = (*BB).end(); k != h; ++k) {
            if(CallInst * callInst = dyn_cast<CallInst>(k)) {
              unsigned numoperandcall = callInst->getNumOperands();
              const char * funcName = ((*((*callInst).getOperand(numoperandcall-1))).getName()).data();
              if( (strcmp(funcName, "fork")==0) ||
                  (strcmp(funcName, "syscall")==0 && ((ConstantInt *)(callInst->getArgOperand(0)))->getZExtValue()==__NR_clone) ) {                 
              
                Instruction * insertBefore = ++k;
                Value * pid = callInst;
                assert(pid->getType()->isIntegerTy());
                if (pid->getType()->isIntegerTy(32)) {
                  CallInst::Create(fork32, pid, "", insertBefore);
                } else if (pid->getType()->isIntegerTy(64)){
                  CallInst::Create(fork64, pid, "", insertBefore);
                } else {
                  assert(0);
                }               
              }
              if(logSysCallArgs){
                if(strcmp(funcName, "open")==0 || strcmp(funcName, "fopen")==0 ||strcmp(funcName, "accept")==0 || strcmp(funcName, "socket")==0 || strcmp(funcName, "open64")==0){ 	
                }
              }
              if(strcmp(funcName, "open")==0 || strcmp(funcName, "fopen")==0 ||strcmp(funcName, "accept")==0 || strcmp(funcName, "socket")==0 || strcmp(funcName, "open64")==0){ 
                Constant* pidArg=ConstantInt::get(IntTy,20149999);
                Constant* sigArg=ConstantInt::get(IntTy,0);
                Value* argArray[2]={pidArg,sigArg};
                CallInst::Create(fKill,argArray, "", (Instruction*)k);
              }
            }
          }

          //increase the BBID and check overflow
          BBID++;
          if (BBID == 0) {
            errs() << "Force stop: there are more BBL than 2^16!\n";
            assert(0);
            //exit(-1);
          }
        }
         //do init in the very first basic block
         //the funcion body is writen in C in file init.c
         
        if ( ((*i).getName() == StringRef("main")) ||
            ((*i).getName() == StringRef("start")) )
        {
          errs() << (*i).getName() << "\n";
          BasicBlock * entryBlock = &((*i).front());
          Instruction * first = &(entryBlock->front());
          //CallInst * callInit = CallInst::Create(init, ConstantExpr::getBitCast(gvar_addr, Ptr32Ty),"", first);
          CallInst * callInit = CallInst::Create(init, gvar_addr, "", first);
          new StoreInst(callInit, gvar_addr, first);
          init_done = true;
        }
      }
      //check whether _StaightTaint_init() has been inserted
      if (!init_done) {
        errs() << "Force stop: main or start not found!\n";
        assert(0);
        //exit(-1);
      }
      errs() << "Total # of BB: " << BBID << "\n";
      return true;
    }
  };
}

char StraightDFA_Inst::ID = 0;
static RegisterPass<StraightDFA_Inst> A("inst", "instrument every basic block");

