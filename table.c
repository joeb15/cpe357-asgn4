#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "utils.h"
#include "table.h"

void table(int mask, int argc, const char **argv){
  int in = open(argv[2], O_RDONLY, 0600);
  int depth = 1;
  int nulls = 0;
  while (1){
    Tar *tar = tarRead(in, mask & ~MASK_VERBOSE);
    if(tar){
      if(!tarCheckValid(tar, mask)){
        fprintf(stderr, "Invalid or corrupt tar file.\n");
        return;
      }
      nulls=0;
      if(tarIsDir(tar))
        depth++;
      if(tarInArgs(tar, argc-3, argv+3)){
        if(mask & MASK_VERBOSE){
          tarPrintShort(tar);
        }else{
          printf("%s\n", tarGetPath(tar));
        }
      }
      tarFree(tar);
    }else{
      nulls++;
      if(nulls==2){
        depth--;
        nulls=0;
      }
      if(depth==0)
        break;
    }
  }
  close(in);
}
