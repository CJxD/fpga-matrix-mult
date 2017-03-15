/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

// $Id: backdoor_writes.C,v 1.1 2011/02/01 10:52:03 djg11 Exp $
//
// backdoor_writes for the ARM.
//
//
// This code implements writes to part of the address space that is internal to the CPU core
// and some of these are backdoors that do not exist in the real hardware.

// Turn this on for low-level tracing backdoor operations (but prefer to use the flag TENOS_TRACE_IO flags
// #define BDTRC(X) X

// In Zynq-7000-TRM.pdf on page 108 says that CPU1 should be running instructions from 0xFFFFFE0 to 0xFFFFFFF0 after reset and that CPU0 at one point need to write to 0xFFFFFFF0 with address of instruction to execute in order to wake up CPU1.

{
  u32_t backaddr = memaddr & 0xFFFFffff; // a nop
  
  if (backaddr >= 0xFFFFff00)
    {
      if (BACKDOOR_TRACE_ENABLED()) printf("Backdoor write %x\n", memaddr);

      if (backaddr == SOCDAM_REGS_BASE_32 + SOCDAM_CORE_CMD)
	{
	  simulator_cmd(wdata, 0, 0, 0);
	  return 1; // cmd done
	}
      else if(backaddr == 0xFFFFFFF0) {
          // According to zynq documentation CPU0
          // will active CPU1 by writing address to
          // execute at address 0xFFFFFFF0
          assert(0);
      } else {
	printf("Undefined backdoor register write %x\n", backaddr);
      }
    }
  
  else if ((backaddr & 0xFFFFFF00) == (0xFFFFff00 & SOCDAM_SPEEDO_REGS_BASE_32))
     {
       int rc = -200;
       BDTRC(printf("Backdoor spEEDO reg write %x wd=%lx\n", backaddr, wdata));
       u64_t wd = wdata;
#ifdef WITH_SPEEDO
       rc = busaccess.speedo.speedo_api(backaddr, wd, /*writef*/true, delay, procID);
#endif
	  if (BACKDOOR_TRACE_ENABLED()) printf("spEEDO reg write %x %lx rc=%i\n", backaddr, wdata, rc );
	  if (rc == 0) return 1;  // Return 1 cmd done.
     }
}

// eof
