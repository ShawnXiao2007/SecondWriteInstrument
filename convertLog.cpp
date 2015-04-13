#define BUF_SIZE 1024*1024
#include "stdlib.h"
#include "stdio.h"
#include <string>
#include <iostream>
using namespace std;
void convertTraceFile(string infname, string outfname)
{
      if(access(outfname.c_str(),F_OK)!=-1)
      {
        cout<<outfname.c_str()<<" already exist!\n";
        return;
      }
      void *buf = malloc(BUF_SIZE);
      FILE *f = fopen(infname.c_str(), "r");
      if (f == NULL) {
          cout<<"failed to open file"<<"\n";
          exit(1);
      }
      FILE *fnew = fopen(outfname.c_str(), "w+"); 
      if (fnew == NULL) {
          perror("failed to open file");
          exit(1);
      }
      for(;;) {
          int ret = fread(buf, 1, BUF_SIZE, f);
          if (ret == 0) {
              break;
          }
          short *p;
          for (p = (short *)buf; (char *)p < (char *)buf + ret; p++) {
              fprintf(fnew, "%hu\n", *p);
          }
      }
      fclose(f);
      fclose(fnew);
}

int main(int argc, char** argv){
  if(argc!=3 || argv[1]==0 || argv[2]==0){
    cout<<"Usage: convert inputfilename outputfilename\n";
    return 0;
  }
  
  convertTraceFile(string(argv[1]), string(argv[2]));
  return 0;
}
