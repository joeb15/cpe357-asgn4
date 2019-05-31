#include <time.h>
#include <utime.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "utils.h"
#include "extract.h"

void extract(int mask, int argc, const char **argv){
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
        if(mask & MASK_VERBOSE)
          tarPrintShort(tar);

        struct stat dirpath;

        char *dir = tarGetDir(tar);

        if (stat(dir, &dirpath) == -1) {
          mkdir(dir, 0600);
        }

        char *path = tarGetPath(tar);

        int wfd = open(path, O_WRONLY|O_TRUNC|O_CREAT, 0600);
        write(wfd, tar->data, arrToNum(tar->size));
        close(wfd);

        struct stat fileStat;
        struct utimbuf new_times;

        stat(path, &fileStat);

        new_times.actime = fileStat.st_atime;
        new_times.modtime = arrToNum(tar->mtime);
        utime(path, &new_times);

        free(path);
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