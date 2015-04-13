#include "common.h"
#include "detectLoop.h"
#ifndef SLIMINST_H
#define SLIMINST_H
using namespace llvm;
using namespace std;

class ModuleMeta{
public:
  ModuleMeta(Module &M):__M(M), __maxF(0), __maxB(0), __pFofMaxB(0), __loopBBLs(new unordered_set<BasicBlock const*>()), __numBBLs(0), __numLoops(0), __numType1Loops(0), __numType2Loops(0), __numFuncs(0){
    __initEverything();
  };
  //output the meta information
  //Function (or BBL or Loop) ID is a numerical ID
  //For BBL without name, set one
  //Function ID : Function Name
  //Function ID : {<BBL ID : BBL Name>}
  //Function ID : {<Loop ID : Loop Type>}
  void outputModuleMetaToFile();

  //initilization
  void __initEverything();
  //part 1
  Module &__M;
  //part 2
  unsigned long __maxF;
  unsigned long __maxB;
  Function const * __pFofMaxB;//a pointer to the function that has the largest number of BBLs
  void __initMaxFandB();
  //part 3
  //0~9 is reserved
  //10~10+__maxF-1 is function IDs
  //10+__maxF~10+__maxF+__maxB-1 is BBLIDs
  //the BBID of the start of the loop is loop ID
  //The input Module could have BBL without name, if so, set the name as FuncName_BBL_#
  map<string, int> FunctionName2ID;
  map<string, map<int, string>> BBLID2Name;
  map<string, map<int, BasicBlock*>> BBLID2Addr;
  map<string, map<int, int>> LoopID2Type;
  void __initFunctionName2ID();
  void __initBBLID2Name();
  shared_ptr< unordered_set<BasicBlock const*> > __loopBBLs;

  //info
  unsigned int __numBBLs;
  unsigned int __numLoops;
  unsigned int __numType1Loops;
  unsigned int __numType2Loops;
  unsigned int __numFuncs;
  void displayStatInfo();
};

class ModuleMembers{
  public:
    ModuleMembers(Module& M);
    
    Module& __M;
    Type* voidTy;
    Type* shortTy;
    Type* intTy;
    Type* longTy;
    PointerType* ptr16Ty;
    PointerType* ptr32Ty;
    Function* log;
    Function* logCounter;
    Function* init;  
    Function* fork32;
    Function* fork64;
    Function* fkill;
    Function* ffflush;
    Function* eval;
    GlobalVariable* gvar_addr;
  private:
    bool checkRep();
};

class SlimInst{
public:
  SlimInst(Module &M, ModuleMeta const * pMeta, ModuleMembers const * pMbr):__M(M), __pMeta(pMeta), __pMbr(pMbr){
    assert(__pMeta);
    assert(__pMbr);
  };

  //output max number of functions and basic blocks
  void displayNumFuncAndBBL();

  //run the instrumentation
  bool run();

private:
  Module &__M;
  ModuleMeta const* __pMeta;
  ModuleMembers const* __pMbr;
  //insert instructions to entry block
  //insert instructions to non-loop block
  //insert basic blocks before and after loop blocks
  void __instFunc(Function * F);
  void __instFuncMin(Function * F);
  void __instFuncMax(Function * F);

  //instrumentation
  void __instLogBBL(BasicBlock * BBL, unsigned short BBID);
  void __instType1LoopBBL(Function& F, BasicBlock * BBL, unsigned short loopID);
  void __instMainOrStartFuncEntryBBL(Function& F);
  void __instCallInst(CallInst* callInst,Instruction* next);
};
#endif
