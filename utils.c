//
// Created by joeba on 5/28/2019.
//
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

#include "utils.h"

extern int lstat(const char *, struct stat*);
extern void realpath(const char *, char *);

long arrToNum(char arr[]){
  long num = 0;
  char c;
  int size = 0;
  while ((c = arr[size++]) && c>='0' && c<='7'){
    num <<= 3;
    num += c-'0';
  }
  return num;
}
void numToArr(char buf[], long val){
  numToArrSafe(buf, val, 0);
}
int numToArrSafe(char buf[], long val, int bufferSize){
  int size = 0;
  while(val){
    if(bufferSize && size > bufferSize){
      fprintf(stderr, "%ld could not be converted to array of size %d", val, bufferSize);
      return 0;
    }
    buf[size++] = (val & 7) + '0';
    val >>= 3;
  }
  for(int i=0;i<size>>1;i++){
    char tmp = buf[i];
    buf[i] = buf[size-1-i];
    buf[size-1-i]=tmp;
  }
  return 1;
}

int tarIsDir(Tar *tar){
  return tar->typeflag==TYPE_FLAG_DIR;
}
int tarIsLink(Tar *tar){
  return tar->typeflag==TYPE_FLAG_S_LINK;
}
void tarPrintShort(Tar *tar){
  long secssinceepoch = arrToNum(tar->mtime);
  struct tm *time = localtime((time_t*) (&secssinceepoch));

  char sizeStr[256];
  char timeStr[256];
  char accessStr[256];
  char permStr[256];

  sprintf(sizeStr, "%8ld", arrToNum(tar->size));
  sprintf(timeStr, "%04d-%02d-%02d %02d:%02d", time->tm_year+1900, time->tm_mon+1, time->tm_mday, time->tm_hour, time->tm_min);

  sprintf(accessStr, "%s/%s                      ", tar->gname, tar->uname);

  permStr[0] = (tarIsDir(tar)?'d':tarIsLink(tar)?'l':'-');
  int perms = arrToNum(tar->mode);

  for(int i=0;i<3;i++){
    permStr[(2-i)*3+1] = perms & 4 ? 'r':'-';
    permStr[(2-i)*3+2] = perms & 2 ? 'w':'-';
    permStr[(2-i)*3+3] = perms & 1 ? 'x':'-';
    perms>>=3;
  }


  sizeStr[8]=0;
  timeStr[16]=0;
  permStr[10]=0;
  accessStr[17]=0;

  char *path = tarGetPath(tar);
  printf("%s %s %s %s %s\n", permStr, accessStr, sizeStr, timeStr, path);
  free(path);
}

/*
 * Tar Helper Methods
 */
char *tarGetPath(Tar *tar){
  int pathMaxSize = SIZE_TAR_PREFIX+SIZE_TAR_NAME+2;
  char *path = malloc(pathMaxSize);

  int index = 0;
  if(tar->prefix[0]){
    for(int i=0;i<SIZE_TAR_PREFIX && tar->prefix[i];i++)
      path[index++] = tar->prefix[i];
    path[index++]= '/';
  }

  for(int i=0;i<SIZE_TAR_NAME && tar->name[i];i++)
    path[index++] = tar->name[i];

  if(tarIsDir(tar) && path[index-1]!='/')
    path[index++]='/';

  path[index++]=0;

  return path;
}

char *tarGetDir(Tar *tar){
  char *dir = tarGetPath(tar);
  int slash = 0;
  for(int i=0;i<strlen(dir);i++){
    if(dir[i]=='/')slash=i;
  }
  dir[slash]=0;
  return dir;
}

void tarAddPrefix(const char *path, Tar *child){
    int slash = 0;
  for(int i=0;i<strlen(path)-1;i++){
    if(path[i]=='/')
      slash=i;
  }
  strncat(child->prefix, path, slash);
}
void tarAddName(Tar *tar, const char *path){
  if(tar->typeflag != TYPE_FLAG_S_LINK)
    return;
  int i=0;
  int lastSlash = -1;
  int nameLen = 0;
  while(path[i]){
    if(path[i] == '/' || path[i] == '\\'){
      lastSlash = i;
      nameLen=0;
    }else{
      nameLen++;
    }
    i++;
  }
  if(nameLen > SIZE_TAR_NAME){
    fprintf(stderr, "Tar link name is %d characters and can only be %d\n", nameLen, SIZE_TAR_NAME);
    return;
  }
  strcpy(tar->name, path+lastSlash+1);
}
void tarStat(Tar *tar, const char *path){
  struct stat fileStat;
  struct stat fileLStat;
  if(stat(path, &fileStat) == -1){
    perror("Stat Error: ");
    return;
  }
  if(lstat(path, &fileLStat) == -1){
    perror("LStat Error: ");
    return;
  }
  if(S_ISLNK(fileLStat.st_mode)){
    tar->typeflag=TYPE_FLAG_S_LINK;
    numToArr(tar->size, 0);
  }else if(S_ISDIR(fileLStat.st_mode)) {
    tar->typeflag=TYPE_FLAG_DIR;
    numToArr(tar->size, 0);
  }else{
    tar->typeflag=TYPE_FLAG_REGULAR;
    if(!numToArrSafe(tar->size, fileStat.st_size, SIZE_TAR_SIZE)){
      fprintf(stderr, "Error writing Tar size\n");
    }
  }

  if(!numToArrSafe(tar->uid, fileStat.st_uid, SIZE_TAR_UID)){
    fprintf(stderr, "Error writing UID\n");
  }
  if(!numToArrSafe(tar->gid, fileStat.st_gid, SIZE_TAR_GID)){
    fprintf(stderr, "Error writing GID\n");
  }
  if(!numToArrSafe(tar->mode, fileStat.st_mode, SIZE_TAR_MODE)){
    fprintf(stderr, "Error writing mode\n");
  }
  if(!numToArrSafe(tar->devmajor, major(fileStat.st_dev), SIZE_TAR_DEVMAJOR)){
    fprintf(stderr, "Error writing Device Major Version\n");
  }
  if(!numToArrSafe(tar->devminor, minor(fileStat.st_dev), SIZE_TAR_DEVMINOR)){
    fprintf(stderr, "Error writing Device Minor Version\n");
  }
  if(!numToArrSafe(tar->mtime, fileStat.st_mtime, SIZE_TAR_MTIME)){
    fprintf(stderr, "Error writing Modified Time\n");
  }

  char *uname = getpwuid(fileStat.st_uid)->pw_name;
  if(strlen(uname) > SIZE_TAR_UNAME){
    fprintf(stderr, "Users name is %ld characters, max is %d\n", strlen(uname), SIZE_TAR_UNAME);
    return;
  }
  strcpy(tar->uname, uname);
  char *gname = getgrgid(fileStat.st_gid)->gr_name;
  if(strlen(gname) > SIZE_TAR_GNAME){
    fprintf(stderr, "Group name is %ld characters, max is %d\n", strlen(gname), SIZE_TAR_GNAME);
    return;
  }
  strcpy(tar->gname, gname);
}
void tarAddEncoding(Tar *tar){
  strncpy(tar->version, "00", 2);
  strcpy(tar->magic, "ustar");
}
void tarAddLinkname(Tar *tar, const char *path){
  int i=0;
  int lastSlash = -1;
  int nameLen = 0;
  char *fullpath = malloc(SIZE_TAR_NAME+SIZE_TAR_PREFIX);
  realpath(path, fullpath);
  while(fullpath[i]){
    if(fullpath[i] == '/' || fullpath[i] == '\\'){
      lastSlash = i;
      nameLen=0;
    }else{
      nameLen++;
    }
    i++;
  }
  if(nameLen > SIZE_TAR_NAME){
    fprintf(stderr, "Tar name is %d characters and can only be %d\n", nameLen, SIZE_TAR_NAME);
    return;
  }
  if(tar->typeflag == TYPE_FLAG_S_LINK){
    strcpy(tar->linkname, fullpath+lastSlash+1);
  }else{
    strcpy(tar->name, fullpath+lastSlash+1);
  }
  free(fullpath);
}
void tarGetFileData(Tar *tar, const char *path){
  if(tar->typeflag == TYPE_FLAG_DIR || tar->typeflag == TYPE_FLAG_S_LINK)return;
  int size;
  int currsize = 0;
  char buffer[512];
  int fd = open(path, O_RDONLY);
  if(!fd){
    perror("File read Error:");
    return;
  }
  tar->data = malloc(512);
  while((size = read(fd, &buffer, 512))){
    int ptr = currsize;
    currsize+=size;
    tar->data = realloc(tar->data, currsize);
    for(int i=0;i<size;i++){
      tar->data[ptr+i] = buffer[i];
    }
  }
  close(fd);
}
void tarGetCheckSum(Tar *tar){
  for(int i=0;i<8;i++)
    tar->chksum[i] = ' ';
  char *tarC = (char*)tar;
  long chksum = 0;
  for(int i=0;i<512;i++)
    chksum += tarC[i];
  if(!numToArrSafe(tar->chksum, chksum, SIZE_TAR_CHKSUM)){
    fprintf(stderr, "Error Writing checksum\n");
  }
}
int tarCheckValid(Tar *tar, int mask){
  long expected = arrToNum(tar->chksum);
  for(int i=0;i<8;i++)
    tar->chksum[i] = ' ';
  char *tarC = (char*)tar;
  long chksum = 0;
  for(int i=0;i<512;i++)
    chksum += tarC[i];
  int equal = (chksum == expected);
  if(!equal){
    fprintf(stderr, "Checksum of %ld does not match expected %ld\n", chksum, expected);
    return 0;
  }

  char *validMagic = "ustar\0";
  for(int i=0;i<5+(mask&MASK_STRICT?1:0);i++){
    if(tar->magic[i] != validMagic[i]){
      fprintf(stderr, "Magic does not match expected\n");
      return 0;
    }
  }

  if(mask & MASK_STRICT && !tar->version){
    fprintf(stderr, "Magic does not have a version\n");
    return 0;
  }

  return equal;
}
void tarAddChildren(Tar *tar, int mask){
  tar->numChildren = 0;
  if(!tarIsDir(tar))
    return;

  char *directory = tarGetPath(tar);
  struct dirent *dir;
  DIR *d = opendir(directory);
  int maxsize = 4;
  tar->children = malloc(maxsize);
  if (d){
    while ((dir = readdir(d))){
      if(strcmp(".", dir->d_name) && strcmp("..", dir->d_name)){
        char childName[300];
        strcpy(childName, directory);
        strcat(childName, dir->d_name);
        Tar *child = tarCreateChild(tar, childName, mask);
        tar->children[tar->numChildren] = child;
        tar->numChildren++;
        if(tar->numChildren >= maxsize){
          tar->children = realloc(tar->children, maxsize<<=1);
        }
      }
    }
    closedir(d);
  }
}
int strpos(char *haystack, const char *needle){
  char *c = strstr(haystack, needle);
  if(c)
    return c - haystack;
  return -1;
}
int tarInArgs(Tar *tar, int argc, const char **argv){
  if(argc <= 0)return 1;
  for(int i=0;i<argc;i++){
    char *path = tarGetPath(tar);
    int arglen = strlen(argv[i]);
    int pathlen = strlen(path);
    if(strpos(path, argv[i]) == 0 || (pathlen-arglen >=0 && strpos(path, argv[i]) == pathlen-arglen))
      return 1;
  }
  return 0;
}

Tar *tarCreateChild(Tar *parent, const char *path, int mask){
  Tar *tar = malloc(sizeof(Tar));
  char *tarChar = (char*)tar;
  for(int i=0;i<512;i++)
    tarChar[i]=0;
  tarAddPrefix(path, tar);
  tarStat(tar, path);
  tarAddName(tar, path);
  tarAddEncoding(tar);
  tarAddLinkname(tar, path);
  tarGetCheckSum(tar);
  tarGetFileData(tar, path);
  if(mask & MASK_VERBOSE)
    tarPrintShort(tar);
  tarAddChildren(tar, mask);
  return tar;
}
Tar *tarCreate(const char *path, int mask){
  return tarCreateChild(NULL, path, mask);
}
Tar *tarRead(int fd, int mask){
  Tar *tar = malloc(sizeof(Tar));
  read(fd, tar, 512);
  char *tarC = (char*)tar;
  int empty = 1;
  for(int i=0;i<512;i++){
    if(tarC[i]){
      empty = 0;
      break;
    }
  }
  if(empty)
    return NULL;

  long size = arrToNum(tar->size);
  tar->data = malloc(size);
  tar->numChildren=0;
  read(fd, tar->data, size);
  int round = size % 512;
  if(round){
    round = 512-round;
    char buffer[round];
    read(fd, buffer, round);
  }
  return tar;
}

void tarFree(Tar *tar){
  if(tar->data)
    free(tar->data);
  if(tarIsDir(tar)){
    for(int i=0;i<tar->numChildren;i++){
      tarFree(tar->children[i]);
    }
    free(tar->children);
  }
  free(tar);
}