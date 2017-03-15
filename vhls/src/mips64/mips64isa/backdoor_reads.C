/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

extern int activeWorkers;

#if 1
  if ((memaddr  & SOCDAM_BDREGS_MASK) == SOCDAM_BDARGV_BASE)

    {
      rdata = (argv_backdoor) ? argv_backdoor->read32(memaddr & 0xFFFFLLU): 0;
      //      printf("Argc backdoor %x %x\n", memaddr, r);
      return true; 
    }

  else if ((memaddr  & SOCDAM_BDREGS_MASK) == (SOCDAM_BDREGS_MASK & SOCDAM_BDREGS_BASE))

    {
      //      printf("Backdoor read %x\n", memaddr);
      if (memaddr == SOCDAM_BDREGS_BASE + TICKS_REG_A)
	{
	  rdata = counter_ticks;
	  return true;
	}
      else if (memaddr == SOCDAM_BDREGS_BASE + GETENV_ARGC)
	{
	  rdata = (argv_backdoor) ? argv_backdoor->argc(): 0;
	  //     printf("Argc backdoor argc=%i\n", r);
	  return true;
	}
      else if (memaddr == SOCDAM_BDREGS_BASE + PID_REG_A)
	{
	  //      printf("Returning procid %i\n", procID);
	  rdata = procID; // Really need to get barrel spoke number as well!
	  return true;
	}
      else if (memaddr == SOCDAM_BDREGS_BASE + PID_REG_B)
	{
	  //		increment actworkers
	  //      printf("Returning activeWorkers %i\n", actWorkers);
	  rdata = ++activeWorkers; // Really need to get barrel spoke number as well!
	  return true;
	}
      else if (memaddr == SOCDAM_BDREGS_BASE + PID_REG_C)
	{
	  //		decrement actworkers
	  //      printf("Returning activeWorkers %i\n", actWorkers);
	  rdata = --activeWorkers; // Really need to get barrel spoke number as well!
	  return true;
	}
      else return 0;
    }
#endif

