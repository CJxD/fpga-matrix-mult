/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */
//
// prlibc: Non arch-specific glue code.
//

//#include "socdam-bdoor-mapping.h"


#if 0
// Currently we use the one in prlibc.

int _crt0()
{
  int cpuid = READ_PID_REG(0);
  int argc = READ_ARGC(0);
  printf("Starting on core %i argc=%i\n", cpuid, argc);
  char **argv = READ_ARGV(0);
  return main(argc, argv);
}

#endif

void _isr_routine()
{

}
