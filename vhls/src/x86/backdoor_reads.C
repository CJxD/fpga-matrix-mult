/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

//
// Prazor/vhls backdoor_reads for x86_64
//
// This code provides read results from part of the address space that is internal to the CPU core.
// and some of these are backdoors that do not exist in the real hardware.

#define BDTRC(X)

{
  //printf("Backdoor read %lx\n", memaddr);

  u64_t backaddr = memaddr & 0xFFFFffffFFFFffff; // a nop.

  if ((backaddr & 0xFFFFffffFFFC0000) == (0xFFFFffffFFFC0000 & SOCDAM_ARGV_BASE_64_LE))
    {
      rdata = (argv_backdoor_su) ? argv_backdoor_su->read64(backaddr & 0xFFFF): 0;
      if (parent->busaccess.traceregions)
	printf("Argv backdoor %lx %lx\n", backaddr, rdata);
      return 1; 
    }
  else if ((backaddr & 0xFFFFffffFFFF0000) == (0xFFFFffffFFFFff00 & SOCDAM_SPEEDO_REGS_BASE_64))
    {
      u64_t r = 0x1234DEAD;
      int rc = -200;
#ifdef WITH_SPEEDO
      rc = parent->busaccess.speedo.speedo_api(backaddr, r, /*writef*/false, delay, parent->procID);
#endif
      rdata = r;
      if (parent->busaccess.traceregions)
	printf("spEEDO reg read %lx %lx\n", backaddr, rdata);
      BDTRC(printf("Backdoor spEEDO reg read %x r=%lx\n", backaddr, r));
      return !rc; // backdoors return 1 for ok.
    }

  else if ((backaddr  & 0xFFFFffffFFFF0000) == (0xFFFFffffFFFF0000 & SOCDAM_REGS_BASE_64))
    {
      //printf("Backdoor reg YES read %x\n", backaddr);
      int r = 0x1234DEAD;
      if ((backaddr & 0xF8) == TICKS_REG_A)
	{
	  extern int backdoor_counter_ticks();
	  r = backdoor_counter_ticks();
	  if (parent->busaccess.traceregions)  printf("Returning counter ticks %i\n", r);
	}
      else if ((backaddr & 0xF8) == CMDLINE_ARGC)
	{
	  r = (argv_backdoor_su) ? argv_backdoor_su->argc(): 0;
	  if (parent->busaccess.traceregions)
	    printf("Argc backdoor (%p) argc=%i\n", argv_backdoor_su, r);
	}
      else if ((backaddr & 0xF8) == PID_REG_A)
	{
	  r = parent->procID;

	  //if (parent->busaccess.traceregions)
	  //  printf("%s:%s: Returning backdoor processor id %i\n", name(), "x86_64", r);
	}
      else
	{
	  //printf("undefined backdoor read reg %x  (pc=0x%lx)\n", backaddr, lastfetch);
	}
      rdata = r;
      return 1;
    }

  return 0; // not a backdoor address
}

// eof

