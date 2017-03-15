// Copyright (C) 2016 XPARCH, Ltd. <info@xparch.com>

#ifndef IO_BACKDOOR_H
#define IO_BACKDOOR_H

#include "tenos.h"
#include "socdam_bdoor_mapping.h"

/* FIXME: Maximum 256 can be open at any time */
#define MAX_FILES 256

class io_backdoor_setup {

 public:
  io_backdoor_setup(const char* image, u8_t psize, char e);
  /* opens file */
  int open(const char* name, const char* mode);
  /* fills buffer */
  int fill(int fid, char* buf, int max);
  /* close */
  int close(int handle);
  /* flush */
  int flush(int fid, char* buf);

 private:
  const char* image_name;
  u8_t pointer_size;

  int open_files;
  int curr_idx;
  FILE* files[MAX_FILES];
};

extern class io_backdoor_setup *io_backdoor_su;


#endif
