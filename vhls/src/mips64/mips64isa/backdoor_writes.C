/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

// $Id: backdoor_writes.C,v 1.5 2011/06/21 11:41:40 my294 Exp $
// backdoor_writes
//
extern int activeWorkers;
#if 1
  if (memaddr == SOCDAM_BDREGS_BASE + TRACE_REG_A)
    {
      printf("%s: Enabled tracing for " PFI64 " cycles\n", name(), wdata);
      tracing = wdata;
      return;
    }

  //if (tracing > 0) { printf("[%s Writing %i " PFX64 " " PFX64 "]\n", __FILE__, procID, memaddr, wdata); tracing -= 1; }

  if (memaddr == SOCDAM_BDREGS_BASE + PID_REG_B)
    {
      printf(" Number of active workers before = %i \n", activeWorkers);
      activeWorkers = wdata;
      printf(" wdata = 0x" PFX64 " \n", wdata);
      printf(" Number of active workers afterwards = %i \n", activeWorkers);
      return;
    }

#endif
