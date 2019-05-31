#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include "utils.h"
#include "create.h"

void addFile(Tar *tar, int mask, int fd){
  write(fd, tar, 512);
  long size = arrToNum(tar->size);
  int round = size % 512;
  write(fd, tar->data, size);
  if(round){
    int num = 512-round;
    char buf[num];
    for(int i=0;i<num;i++)
      buf[i]=0;
    write(fd, buf, num);
  }
  if(tarIsDir(tar)){
    for(int i=0;i<tar->numChildren;i++){
      addFile(tar->children[i], mask, fd);
    }
    char buf[512];
    for(int i=0;i<512;i++)
      buf[i]=0;
    write(fd, buf, 512);
    write(fd, buf, 512);
  }
}

void create(int mask, int argc, const char **argv) {
  int fd = open(argv[2], O_WRONLY|O_TRUNC|O_CREAT, 0600);
  for(int i=3;i<argc;i++){
    Tar *tar = tarCreate(argv[i], mask);
    addFile(tar, mask, fd);
    tarFree(tar);
  }

  char buf[512];
  for(int i=0;i<512;i++)
  buf[i]=0;
  write(fd, buf, 512);
  write(fd, buf, 512);

  close(fd);
}
