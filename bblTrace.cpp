#include "bblTrace.h"

BBLTrace::BBLTrace(string fname){
  __fname=fname;
  __initEverything();
}
void BBLTrace::__initEverything(){
  __initBblid2num();
  __initRank();   
}
void BBLTrace::__initBblid2num(){
  ifstream infile(__fname); 
  string line;
  while(getline(infile, line)){
    unsigned bblid;
    istringstream iss(line);
    if(!(iss>>bblid)){
      continue;
    }
    if(bblid2num.find(bblid)==bblid2num.end()){
      bblid2num[bblid]=0;
    }
    bblid2num[bblid]+=1;
  }
}
void BBLTrace::__initRank(){
  vector<pair<unsigned, unsigned>> tmp;
  for(auto i=bblid2num.begin(), i_e=bblid2num.end(); i!=i_e; i++){
    tmp.push_back(pair<unsigned, unsigned>(i->first, i->second));  
  }
  sort(tmp.begin(), tmp.end(), BBLTrace::__myComp);
  for(auto i=tmp.begin(), i_e=tmp.end(); i!=i_e; i++){
    rank.push_back(i->first);
  }
}
bool BBLTrace::__myComp(pair<unsigned, unsigned> lhs, pair<unsigned, unsigned> rhs){
  return lhs.second>rhs.second;
}

BBLAnalyzer::BBLAnalyzer(Module& M, BBLTrace const* pBT, string bcfname):__bcfname(bcfname), __pBT(pBT), __M(M){
  __initEverything(); 
}

void  BBLAnalyzer::__initEverything(){
  for(auto i=__M.begin(), i_e=__M.end(); i!=i_e; i++){
    Function& F=*i;
    LoopDetector LD(F);
    auto loopStarts=LD.getLoopStarts();
    auto type1loopBBLs=LD.getType1Loops();
    auto type2loopBBLs=LD.getType2Loops();
    string funcName(F.getName().data());
    for(auto j=F.begin(), j_e=F.end(); j!=j_e; j++){
      BasicBlock* bbl=j;
      unsigned id=__getBblid(bbl);
      unsigned type=0;
      if(loopStarts->find(bbl)!=loopStarts->end()){
        if(type1loopBBLs->find(bbl)!=type1loopBBLs->end()){
          type=1;
        }else if(type2loopBBLs->find(bbl)!=type2loopBBLs->end()){
          type=2;
        }else{
          type=3;
        }
      }
      assert(__bblinfo.find(id)==__bblinfo.end());
      __bblinfo[id]=pair<string, unsigned>(funcName, type);
    }
  }
}

void BBLAnalyzer::write2file(string outfname){
  ofstream ouf;
  ouf.open(outfname.c_str());
  for(auto i :  __pBT->rank){
    ouf<<i<<"\t";
    ouf<<__bblinfo[i].first.c_str()<<"\t";
    ouf<<__bblinfo[i].second<<"\n";
  }
  ouf.close();
}

unsigned BBLAnalyzer::__getBblid(BasicBlock const* bbl){
  unsigned retVal=65536;
  string s1("_StraightTaint_log");
  for(auto i=bbl->begin(), i_e=bbl->end(); i!=i_e; i++){
    Instruction const * k=&(*i);
    if(CallInst const* ci=dyn_cast<CallInst>(k)){
      Function* f=ci->getCalledFunction();
      string s(f->getName().data());
      if(s==s1){
        assert(ci->getNumArgOperands()==1);
        Value* idval=ci->getArgOperand(0);
        if(ConstantInt* idvalci=dyn_cast<ConstantInt>(idval)){
          unsigned id=idvalci->getZExtValue();
          retVal=id;
          break;
        }else{
          assert(0);
        }
      }
    }
  }
  return retVal;
}
