//
// Created by joeba on 5/28/2019.
//

#ifndef ASGN4V3_UTILS_H
#define ASGN4V3_UTILS_H

#define MASK_CREATE 0x01
#define MASK_TABLE 0x02
#define MASK_EXTRACT 0x04
#define MASK_VERBOSE 0x08
#define MASK_FILENAME 0x10
#define MASK_STRICT 0x20

#define TYPE_FLAG_REGULAR '0'
#define TYPE_FLAG_REGULAR_ALT 0
#define TYPE_FLAG_S_LINK '2'
#define TYPE_FLAG_DIR '5'

#define SIZE_TAR_NAME 100
#define SIZE_TAR_MODE 8
#define SIZE_TAR_UID 8
#define SIZE_TAR_GID 8
#define SIZE_TAR_SIZE 12
#define SIZE_TAR_MTIME 12
#define SIZE_TAR_CHKSUM 8
#define SIZE_TAR_LINKNAME 100
#define SIZE_TAR_MAGIC 6
#define SIZE_TAR_VERSION 2
#define SIZE_TAR_UNAME 32
#define SIZE_TAR_GNAME 32
#define SIZE_TAR_DEVMAJOR 8
#define SIZE_TAR_DEVMINOR 8
#define SIZE_TAR_PREFIX 155

typedef struct __Tar{
    char name[SIZE_TAR_NAME];
    char mode[SIZE_TAR_MODE];
    char uid[SIZE_TAR_UID];
    char gid[SIZE_TAR_GID];
    char size[SIZE_TAR_SIZE];
    char mtime[SIZE_TAR_MTIME];
    char chksum[SIZE_TAR_CHKSUM];
    char typeflag;
    char linkname[SIZE_TAR_LINKNAME];
    char magic[SIZE_TAR_MAGIC];
    char version[SIZE_TAR_VERSION];
    char uname[SIZE_TAR_UNAME];
    char gname[SIZE_TAR_GNAME];
    char devmajor[SIZE_TAR_DEVMAJOR];
    char devminor[SIZE_TAR_DEVMINOR];
    char prefix[SIZE_TAR_PREFIX];
    char __buffer[12];
    char *data;
    struct __Tar **children;
    int numChildren;
} Tar;

long arrToNum(char arr[]);
void numToArr(char buf[], long val);
int numToArrSafe(char buf[], long val, int bufferSize);

char *tarGetDir(Tar *tar);
char *tarGetPath(Tar *tar);
int tarIsDir(Tar *tar);
int tarIsLink(Tar *tar);
int tarCheckValid(Tar *tar, int mask);
int tarInArgs(Tar *tar, int argc, const char **argv);
void tarPrintShort(Tar *tar);

Tar *tarCreateChild(Tar *parent, const char *path, int mask);
Tar *tarCreate(const char *path, int mask);
Tar *tarRead(int fd, int mask);
void tarFree(Tar *tar);

#endif //ASGN4V3_UTILS_H
