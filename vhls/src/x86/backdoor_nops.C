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
          printf( "Stopping simulation... \n\n");
	  or1200_cpu.final(); sc_stop(); over=true;// stops simulation
	  break;



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
	  extern bool putchar_backdoor_with_handles;
	  if (putchar_backdoor_with_handles)
	    {
	      cout << "Putchar backdoor char '" << c << "'\n";
	    }
	  else std::cout << c << std::flush; // puts a character in GPR3 lower 8 bits
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
