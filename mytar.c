#include <stdio.h>
#include "create.h"
#include "extract.h"
#include "table.h"
#include "utils.h"

int parse(const char *arg){
  char c;
  int flags = 0;
  int i=0;
  while((c = arg[i])){
    switch (c){
      case 'c': flags |= MASK_CREATE;break;
      case 't': flags |= MASK_TABLE;break;
      case 'x': flags |= MASK_EXTRACT;break;
      case 'v': flags |= MASK_VERBOSE;break;
      case 'f': flags |= MASK_FILENAME;break;
      case 'S': flags |= MASK_STRICT;break;
      default: return -1;
    }
    i++;
  }
  return flags;
}

void errormessage(){
  fprintf(stderr, "Usage:\n\tmytar [ctxvS]f tarfile [ path [ ...  ]  ]\n");
}

int main(int argc, char const **argv) {
  if(argc < 3){errormessage();return 1;}
  int flags = parse(argv[1]);
  if(flags == -1 || (flags & MASK_FILENAME) == 0){errormessage();   return 1;}
  if(flags & MASK_CREATE)create(flags, argc, argv);
  if(flags & MASK_EXTRACT)extract(flags, argc, argv);
  if(flags & MASK_TABLE)table(flags, argc, argv);
  return 0;
}
