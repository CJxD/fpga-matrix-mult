/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

// $Id: backdoor_writes.C,v 1.1 2011/02/01 10:52:03 djg11 Exp $
//
// Prazor/vhls backdoor_writes for x86_64
//

// This code provides writeo results from part of the address space that is internal to the CPU core.
// and some of these are backdoors that do not exist in the real hardware.
{
  //  printf("Backdoor write %lx\n", memaddr);
  u32_t backaddr = memaddr & 0xFFFFffff;
  if (backaddr == (0xFFFFffffFFFF0000 & (SOCDAM_REGS_BASE_64 + TRACE_REG_A)))
    {
      //printf("%s: Enabled tracing for " PFI64 " cycles\n", name(), wdata);
      simulator_cmd(TENOS_CMD_TRACE100, 0, 0, 0);
      return 1; // cmd done
    }
#if 0
    else if (backaddr == (0xFFFFffffFFFF0000 & (SOCDAM_REGS_BASE_64 + VHLS_CORE_CMD)))
    {
      //vhls_core_cmd(wdata); in progress...
      return 1;
    }
#endif
  else if (backaddr >= 0xFFFFff00)
    {
      printf("Undefined backdoor register write %x\n", backaddr);
    }

  //if (busaccess.traceregions)
    {/// printf("[Writing %i backaddr=%x wd=" PFX64 "]\n", procID, backaddr, wdata); 
    }

  return 0; // not a backdoor address
}
