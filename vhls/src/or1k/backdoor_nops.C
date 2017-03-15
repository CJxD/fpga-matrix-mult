/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

//
// $Id: backdoor_nops.C,v 1.1 2011/02/01 10:52:03 djg11 Exp $
//

// Verilated only ?

// A different version of this is used between fast ISS and Verilated designs (using OrpsocAccess).
// The real hardware will treat these instructions as NOPs but the simulator attaches special functions.
//

     // Do something if we have l.nop
     switch (instr)
      {
	case 0x15000000 + NOP_EXIT:
	  r3 = accessor.getGpr (3);
	  std::cout << endl<<"Exiting (" << r3 << ")" << std::endl;
	  simulator_cmd(TENOS_CMD_SIM_DONE, r3, 0, 0);
	  break;



      case 0x15000000 + NOP_ATOMIC:  /* DJG */
	atomic_prefix();
	break;
	
      case 0x15000000 + NOP_TRACE100:  /* DJG */
	{
	  simulator_cmd(TENOS_CMD_TRACE100, 0, 0, 0);
	}
	break;
	
      case 0x15000000 + NOP_REPORT:
	  r3 = accessor.getGpr (3);
	  std::cout << " report (" << hex << r3 << ")" << std::endl; // report from GPR3 on the CPU
	  break;

      case (0x15000000 + NOP_PUTC): // write a character
	{
	  r3 = accessor.getGpr (3);
	  simulator_cmd(TENOS_CMD_WRCH, r3, 0, 0);
	  break;
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
