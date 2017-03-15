// Copyright (C) 2016 XPARCH, Ltd. <info@xparch.com>

#include "io_backdoor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

io_backdoor_setup::io_backdoor_setup(const char* image, u8_t psz, char e)
  : image_name(image),
    open_files(0),
    curr_idx(0),
    pointer_size(psz) {
  for(int i = 0; i < MAX_FILES; ++i)
    files[i] = NULL;
}

int io_backdoor_setup::open(const char *name, const char* mode) {
  FILE* f;
  int r, c;
  char buff[8];

  if(open_files >= MAX_FILES) {
    printf("Simulator only supports up to %d simultanously opened files\n", MAX_FILES);
    return -1;
  }
  
  f = fopen(name, mode);
  if(f == NULL) {
    /*printf("io_backdoor_setup::open() -> couldn't open file %s with mode %s\n", name, mode);*/
    return -1;
  }

  if(curr_idx == MAX_FILES)
    curr_idx = 0;
  files[curr_idx] = f;
  r = curr_idx++;
  open_files++;

  return r;
}

int io_backdoor_setup::fill(int fid, char* buf, int max) {
  FILE* f;
  int len = 0;
  int c;
  
  f = files[fid];

  do {
    c = fgetc(f);
    if(c == EOF) {
      buf[len] = EOF;
      break;
    }
    buf[len++] = (char)c;
  } while(len < max);

  return len;
}

int io_backdoor_setup::flush(int fid, char* buf) {
  FILE *f;

  f = files[fid];
  fputs(buf, f);

  return strlen(buf);
}

int io_backdoor_setup::close(int handle) {
  FILE *f;
  int ret;
  
  f = files[handle];
  ret = fclose(f);
  if(!ret) {
    open_files--;
    files[handle] = NULL;
  }

  return ret;
}
  



