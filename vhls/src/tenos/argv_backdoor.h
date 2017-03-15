#ifndef ARGV_BACKDOOR_H
#define ARGV_BACKDOOR_H

//
// 
// $Id: argv_backdoor.h, $
// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2010-14 DJ Greaves, University of Cambridge, Computer Laboratory.
//
// A backdoor so that programs can read the command line argv argument vector
//
// Also we have in here the symbolic_dis function that helps give textual names based on debugging information in ELF files etc..

#include <vector>
#include <algorithm>    // std::sort

#include "tenos.h"
#include "socdam_bdoor_mapping.h"

extern unsigned char *argv_ram_backdoor;

#define ARGV_TRC(X)

struct tenos_symbol
{
  char * name;
  u64_t value;
  unsigned char mode;
 tenos_symbol(char * name, u64_t value, unsigned char mode) : name(name), value(value), mode(mode) { };
};

bool tenos_symbol_order_pred(tenos_symbol *l, tenos_symbol *r);

class argv_backdoor_setup
{
  unsigned int l, p;
  u64_t a0[4], a1[2];
  void put(unsigned int v, u64_t &p, bool be=false)
  {
    assert(p<l*2); // Check not too much content on the argv command line!
    ARGV_TRC(printf("Backdoor argv put byte %li %x %c\n", (p^(be?7:0)), v, v));
    argv_ram[(p ^ (be?7:0))] = v;

    p++;
  }

  std::vector<struct tenos_symbol *> vsymbols;
  bool vsymbols_sorted;
  int m_argc1;

 public:
  void add_symb(char *name, u64_t addr, unsigned char mode);

  unsigned char *argv_ram;
  unsigned char read(int a) { return argv_ram[a]; }

  // We follow the normal modelling style is that RAM models do not need endian conversion on
  // word reads.  ie they use the same model as the underlying hardware.  Hence the argv strings
  // need to be stored in both endiannesses. Also, for each endianness we have both a 32 and a 64 bit
  // index.
  u64_t read64(int a)
  {
   u64_t ans = ((u64_t *)(argv_ram))[a/8];
   ARGV_TRC(printf("Backdoor argv read %x : %lx\n", a, ans));
   return ans;
  }      

  u32_t read32(int a) // le read
  {
    u32_t ans = ((u32_t *)(argv_ram))[a/4];
    ARGV_TRC(printf("Backdoor argv read 32_le %x : %x\n", a, ans));
    return ans;
  }      

  u32_t read32_be(int a) // be read
  {
    u64_t w = ((u64_t *)(argv_ram))[a/8];
    u32_t ans = (a & 4) ? w: (w>>32l);
    ARGV_TRC(printf("Backdoor argv read 32_be %x : %x\n", a, ans));

    return ans;
  }      


  int argc()  { return m_argc1; }


  tenos_symbol *find_best_symbol(u64_t akey);

  // constructor
  // Following the principle that all memories are layed out using modelling workstation endianness this should always be littleendian when using SystemC on an x86.
  // To support heterogenous systems, we always create this backdoor in x86 (little-endian) form and
  // allow the various cores to express their endianness as they make access.
  //
  // When this backdoor is instantiated after any endian correction in the bus interface no correction
  // would be needed at access time, but we tend to make access to it within the core backdoor code that
  // has a local endianness.
  argv_backdoor_setup(const char *image_name, int argc, char *argv[]);

  // TODO: This is constructor for creating memory for stdin that
  // is passed from the command line, we might be better of renaming
  // this class to be just backdoor and than have different methods that
  // are used for all backdoors (MP - 12Feb16)
  argv_backdoor_setup(const char *image_name, FILE* input);

  // Give textual names based on debugging information in ELF files etc..
  int symbolic_dis64(char *buf, int n, u64_t a);

 private:
  void init(const char* image_name);
  void insert(const char *s1, u64_t BE, u64_t LE, bool end);

};



extern class argv_backdoor_setup *argv_backdoor_su;
extern class argv_backdoor_setup *stdin_backdoor_su;

// eof
#endif
