/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

// $Id: backdoor_writes.C,v 1.1 2011/02/01 10:52:03 djg11 Exp $
//
// backdoor_writes for or1k
//
// This file is included in .C form so as it can be used both in the verilated and fastiss forms - the backdoor_nops.C file is the critical one in this respect since the others can be put in the TLM wrapper.

// This code implemetns writes results from part of the address space that is internal to the CPU core.
// Some of these are backdoors that do not exist in the real hardware.
{
  //  printf("Backdoor write %lx\n", memaddr);
  u32_t backaddr = memaddr & 0xFFFFfff0;

  if (backaddr == SOCDAM_REGS_BASE_32 + SOCDAM_ATOMIC_PREFIX)
    {
      atomic_prefix();
      return 1;
    }
  if (backaddr == SOCDAM_REGS_BASE_32 + SOCDAM_CORE_CMD)
    {
      simulator_cmd(wdata, 0, 0, 0);
      return 1; // cmd done
    }

  else if ((backaddr & 0xFFFF0000) == (0xFFFFff00 & SOCDAM_SPEEDO_REGS_BASE_32))
    {
      int rc = -200;
      BDTRC(printf("Backdoor spEEDO reg write %x wd=%lx\n", backaddr, wdata));
#ifdef WITH_SPEEDO
      u64_t wdata64 = wdata;
      rc = busaccess.speedo.speedo_api(backaddr, wdata64, /*writef*/true, delay, procID);
#endif
      if (busaccess.traceregions) printf("spEEDO reg write %x %x rc=%i\n", backaddr, wdata, rc );
      return 1;
    }

  else if (backaddr >= 0xFFFFff00)
    {
      printf("Undefined backdoor register write %x\n", backaddr);
    }
  if (0 && busaccess.traceregions)
    { printf("[Writing %i backaddr=%x wd=%x]\n", procID, backaddr, wdata); 
    }

  return 0; // Not part of backdoor space.
}

//eof
