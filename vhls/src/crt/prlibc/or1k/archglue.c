/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

// archglue.c - OR1K version


#include "socdam_bdoor_mapping.h"

#include "archglue.h"

#define OLD 1
#define NOP_ATOMIC 0x0006

// Don't need this on all archs - eg for x86 - we have some ll/sc code in the crt.S for that.


// Actually an atomic exchange instruction, despite its name.
int _archglue_test_and_set(prlibc_mutex_t *addr, int wdata)
{

  // The appropriate prefix should go before the load, store sequence.
  // 	15 00 00 06 	l.nop 0x6
  // 	d4 03 20 00 	l.sw 0x0(r3),r4
  //	85 63 00 00 	l.lwz r11,0x0(r3)

  

#if OLD
  // Wierdly, the test and set is coded as set then test in the assembler.
  // The order of transactions is permuted in the btlm cpu_busaccess model (and if it were not for some reason we would always fail in mutex aquire since we would see ourself).
  // Clobbers RAM  Need to add. Also need to stop it re-reordering.
  asm("l.nop %0": :"K" (NOP_ATOMIC): "memory");
#else
  MAKE_ATOMIC_PREFIX_32(0);
asm(""::: "memory");
#endif

  *addr = wdata;
  int r = *addr;
  return r;
}


// eof


