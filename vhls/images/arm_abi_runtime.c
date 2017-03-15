/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */


//
// Various minor subroutines, like divide-by-zero handler and so on, needed from the O/S for a particular ABI.
//

#include "prstdio.h"


// ARM glue 
void __aeabi_unwind_cpp_pr0()
{
  printf("GCC Runtime Raise 3\n\n");
}
void __stack_chk_guard()
{
  printf("GCC Runtime Raise 2\n\n");
}
void __stack_chk_fail()
{
  printf("GCC Runtime Raise 1\n\n");
}


void raise()
{
  printf("GCC Runtime Raise 0\n\n");
}

// eof
