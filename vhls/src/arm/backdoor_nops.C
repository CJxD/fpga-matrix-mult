/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

//
// ARM version - unused code. reimplemented in swi_backdoor or arm_backdoor.
//
// $Id: backdoor_nops.C, $
//

#error never used now

// A different version of this is used between fast ISS and Verilated designs (using OrpsocAccess).
// The real hardware will treat these instructions as NOPs but the simulator attaches special functions.
//

     // Do something if we have l.nop
     switch (instr)
      {
	case 0x15000000 + NOP_EXIT:



      case 0x15000000 + NOP_ATOMIC:  /* DJG */
	atomic_prefix();
	break;
	
      case 0x15000000 + NOP_TRACE100:  /* DJG */
	{
	  trace100("trace100 nop backdoor");
	}
	break;
	
      case 0x15000000 + NOP_REPORT:
	  r3 = accessor.getGpr (3);
	  std::cout << " report (" << hex << r3 << ")" << std::endl; // report from GPR3 on the CPU
	  break;

      case (0x15000000 + NOP_PUTC):
	{
	  r3 = accessor.getGpr (3);
	  char c = (char) r3;
	}
	
      case (0x15000000 + NOP_COREPAUSE):
	r3 = accessor.getGpr (3);
	corepause(r3);
	  break;

      case (0x15000000 + NOP_BUSLOCK):
      case (0x15000000 + NOP_BUSUNLOCK):
      case (0x15000000 + NOP_MEMFENCE):
	  break;
      }
