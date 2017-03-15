/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

//
// ARM32 backdoor reads. 
//


// This code provides read results from part of the address space that is internal to the CPU core.
// and some of these are backdoors that do not exist in the real hardware.
{

  u32_t backaddr = memaddr & 0xFFFFffff; // a nop
  if(((backaddr & 0xFFFF0000) == (SOCDAM_STDIN_BASE_32_LE & 0xFFFF0000)) &&
     (backaddr & 0xFFFF) < 0x4000 /* otherwise spEEDO */ ) {
    read_data = (!stdin_backdoor_su) ? 0 : stdin_backdoor_su->read32(backaddr & 0xFFFF);
    switch(insSizeCode) {
      case 0: {
	u8_t shift = (backaddr & 3) * 8;
	read_data = (read_data >> shift) & 0xFF;
	break;
      }
      case 1: {
	assert(0); // TODO
	break;
      }
      case 2: {
	break;
      }
      case 3: {
	assert(0); // TODO
	break;
      }
    }

    return 1;
  }
  
  else if ((backaddr & 0xFFFF0000) == (SOCDAM_ARGV_BASE_32_LE & 0xFFFF0000))
    {

      BDTRC(printf("Backdoor read a=%x argv \n", memaddr));
      read_data = (!argv_backdoor_su) ? 0:
	argv_backdoor_su->read32(backaddr & 0xFFFF);
      switch(insSizeCode) {
      case 0: {
	// this is byte read 
	// and if it is NOT word aligned
	// we should shift the value 
	// correctly
	u8_t shift = (backaddr & 3) * 8;
	read_data = (read_data >> shift) & 0xFF;
	break;
      }
      case 1:
	assert(0); // TODO
      case 2: {
	break;
      }
      case 3: 
	assert(0); // TODO
      }
      BDTRC(printf("Argv backdoor read a=%x d=%lx\n", backaddr, read_data));
      if (BACKDOOR_TRACE_ENABLED()) printf("Argv backdoor read a=%x d=%x\n", backaddr, read_data);
      return 1;  // Return 1 for ok.  Field read_data contains the answer for ARM32.
    }
  else if ((backaddr & 0xFFFFF000) == (0xFFFFf000 & SOCDAM_SPEEDO_REGS_BASE_32))
    {
      BDTRC(printf("Backdoor spEEDO-arm reg read %lx\n", memaddr));
      u64_t r = 0x1234DEAD;
      int rc = -200;
#ifdef WITH_SPEEDO
      rc = busaccess.speedo.speedo_api(backaddr, r, /*writef*/false, delay, procID);
#endif
      read_data = r;
      if (BACKDOOR_TRACE_ENABLED()) printf("spEEDO reg read %x %x\n", backaddr, read_data);
      BDTRC(printf("Backdoor spEEDO reg read %x r=%lx\n", backaddr, r));
      return !rc; // backdoors return 1 for ok.
    }
  else if ((backaddr & 0xFFFFff00) == (0xFFFFff00 & SOCDAM_REGS_BASE_32))
    {
      BDTRC(printf("Backdoor reg file read %x\n", backaddr));
      u64_t r = 0x1234DEAD;
      if ((backaddr & 0xF8) == TICKS_REG_A)
	{
	  // Add on the loosely timed advance.
	  u32_t current_time = static_cast<u32_t>((lt_d_delay+sc_time_stamp()).to_seconds()*1e6);
	  r = current_time;
	  // You can read the ticks from the local timer register on ARM 
	  if (BACKDOOR_TRACE_ENABLED())  printf("Returning counter ticks %li (microseconds)\n", r);
	}
      else if ((backaddr & 0xF8) == CMDLINE_ARGC)
	{
	  r = (argv_backdoor_su) ? argv_backdoor_su->argc(): 0;
	  if (BACKDOOR_TRACE_ENABLED()) printf("Argc backdoor (%p) argc=%li\n", argv_backdoor_su, r);
	}
      else if ((backaddr & 0xF8) == PID_REG_A) // We do not need this PRAZOR backdoor for ARM systems now the info is in the CP15 file somewhere.
	{
	  r = procID;
	  if (BACKDOOR_TRACE_ENABLED()) printf("%s:%s: Returning backdoor processor id %i\n", name(), kind(), procID);
	}
      else if((backaddr & 0xF0 == 0xF0)) {
          // According to zynq documentation CPU0
          // will active CPU1 by writing address to
          // execute at address 0xFFFFFFF0
          assert(0);
      }
      else 
	{
            printf("undefined backdoor read reg %x  (pc=0x%x)\n", backaddr, insAddress);
	}
      read_data = r;
      return 1;
    }
}

// eof

