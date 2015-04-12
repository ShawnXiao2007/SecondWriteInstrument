#define BUF_SIZE 512
#include "stdio.h"
void INPUT_InstTrace::convertTraceFile(string fname)
{
      string fnamenew=fname+string(".new");
      if(access(fnamenew.c_str(),F_OK)!=-1)
      {
        cout<<"Preparing steps: "<<fname<<" already converted!\n";
        return;
      }
      void *buf = malloc(BUF_SIZE);
      FILE *f = fopen(traceFileName, "r");
      if (f == NULL) {
          perror("failed to open file");
          exit(1);
      }
      FILE *fnew = fopen(fnamenew, "w+"); 
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

