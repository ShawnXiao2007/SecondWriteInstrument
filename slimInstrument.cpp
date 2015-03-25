#include "slimInstrument.h"

void SlimInst::getNumFuncAndBBL(){
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
  errs()<<"Num of Func: "<<maxF<<"\nMax num of BBL: "<<maxB<<" of function: "<<p->getName()<<"\n";
}
