/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

// or1k backdoor_reads.C
//
// This file is included in .C form so as it can be used both in the verilated and fastiss forms - the backdoor_nops.C file is the critical one in this respect since the others can be put in the TLM wrapper.

// Turn this on for tracing.
#define BDTRC(X) 

// This code provides read results from part of the address space that is internal to the CPU core.
// and some of these are backdoors that do not exist in the real hardware.
{
  extern int backdoor_counter_ticks(lt_delay delay);
  // BDTRC(printf("Backdoor or I/O read %lx\n", memaddr));
  u32_t backaddr = memaddr & 0xFFFFffff;
  if ((backaddr & 0xFFFF0000) == (SOCDAM_ARGV_BASE_32_BE & 0xFFFF0000))
    {
      BDTRC(printf("Backdoor read %lx argv \n", memaddr));
      rdata = (argv_backdoor_su) ? argv_backdoor_su->read32_be(backaddr & 0xFFFC): 0;
      BDTRC(printf("Argv backdoor %x %lx\n", backaddr, rdata));
      if (busaccess.traceregions)
	printf("Argv backdoor %x %x\n", backaddr, rdata);
      return 1; 
    }
  else if ((backaddr & 0xFFFF0000) == (0xFFFFff00 & SOCDAM_SPEEDO_REGS_BASE_32))
    {
      u64_t r = 0x1234DEAD;
      int rc = -200;

#ifdef WITH_SPEEDO
      rc = busaccess.speedo.speedo_api(backaddr, r, /*writef*/false, delay, procID);
#endif
      rdata = r;
      if (busaccess.traceregions)
	printf("spEEDO reg read %x %x\n", backaddr, rdata);
      BDTRC(printf("Backdoor spEEDO reg read addr=%x rdata=%lx rc=%i\n", backaddr, r, rc));
      return !rc; // backdoors return 1 for ok.
    }
  else if ((backaddr & 0xFFFFff00) == (0xFFFFff00 & SOCDAM_REGS_BASE))
    {
      BDTRC(printf("Backdoor reg YES read %x\n", backaddr));
      u64_t r = 0x1234DEAD;
      if ((backaddr & 0xF8) == TICKS_REG_A)
	{
	  r = backdoor_counter_ticks(delay);
	  if (busaccess.traceregions)  printf("Returning counter ticks %li (microseconds)\n", r);
	}
      else if ((backaddr & 0xF8) == CMDLINE_ARGC)
	{
	  r = (argv_backdoor_su) ? argv_backdoor_su->argc(): 0;
	  if (busaccess.traceregions)
	    printf("Argc backdoor (%p) argc=%li\n", argv_backdoor_su, r);
	}
      else if ((backaddr & 0xF8) == PID_REG_A)
	{
	  r = procID;
	  if (busaccess.traceregions)
	    printf("%s:%s: Returning backdoor processor id %i\n", name(), kind(), procID);
	}
      else
	{
	  printf("undefined backdoor read reg %x  (pc=0x%lx)\n", backaddr, busaccess.lastfetch);
	}
      rdata = r;
      return 1;
    }
  return 0; // Not part of backdoor space.
}

