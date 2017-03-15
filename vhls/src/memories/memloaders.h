// memloaders.h
// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2009-11 D J Greaves
// University of Cambridge, Computer Laboratory.
// ACS P35 SoC D/M Classes - $Id: sram64_cbg.cpp,v 1.18 2011/07/25 15:34:01 my294 Exp $
//

#ifndef MEMLOADERS_H
#define MEMLOADERS_H


#include <linux/elf.h>
#include "tenos.h"


class memloader_if
{
  // pure virtual interface. 
 public:
  virtual bool ip_write(u64_t a, u8_t d, bool es=false) = 0;
  virtual bool ip_read(u64_t a, u8_t &d) = 0;
};

// Simple loader for device tree blob
class cbgdtb 
{
 private:
  FILE* fd;
  memloader_if *par;

 public:
  u64_t reader(bool &big_endian, const char* blob, u64_t offset, u64_t mem_size_bytes);

  // Constructor
  cbgdtb(memloader_if *p);
};


// Simple loader for ELF format binary files.
class cbgelf 
{
public:
  memloader_if *par;
  // Do not use tlm::tlm_endianness in this file because we want it to be independent of SystemC.
  u64_t reader(bool &big_endian, const char *fn, u64_t mem_base, u64_t mem_size_bytes, u64_t *entrypt);
  void dump(char *p, int l);
  // constructor
  cbgelf(memloader_if *p);
};

// Alternative :  Load Ram From Intel hex file.
class cbgihex
{
  memloader_if *par;
  int a, csum, p, finished;
  FILE *fd;
  unsigned char opd[32];
  int iread(int len);
  void opfl();
  void opbyte(int x);

public:
  u64_t reader(bool &big_endian, const char *fn, u64_t mem_base, u64_t lenmax);
  void dump(char *p, int l);

  // constructor
  cbgihex(memloader_if *p);
};


#endif

// eof
