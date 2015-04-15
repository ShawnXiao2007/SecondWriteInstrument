#include "bblTrace.h"

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
      break;
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
