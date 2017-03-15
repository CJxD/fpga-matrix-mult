/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

//
// $Id: backdoor_nops.C,v 1.2 2011/05/11 11:19:53 my294 Exp $
//
     // Do something if we have l.nop
     switch (instr)
      {
	case 0xFF15000000LLU + NOP_EXIT:
	  r3 = accessor.getGpr (3);
	  std::cout << endl<<"Exiting (" << r3 << ")" << std::endl;
          printf( "Stopping simulation... \n\n");
	  or1200.final(); sc_stop(); over=true;// stops simulation
	  break;

	case 0xFF15000000LLU + NOP_REPORT:

	  r3 = accessor.getGpr (3);
	  std::cout << " report (" << hex << r3 << ")" << std::endl; // report from GPR3 on the CPU
	  break;

      case (0xFF15000000LLU + NOP_PUTC):
	  r3 = accessor.getGpr (3);
	  std::cout << (char)r3 << std::flush; // puts a character in GPR3 lower 8 bits
	  break;

      case (0xFF15000000LLU + NOP_COREPAUSE):
	  r3 = accessor.getGpr (3);
	  corepause(r3);
	  break;

      case (0xFF15000000LLU + NOP_BUSLOCK):
      case (0xFF15000000LLU + NOP_BUSUNLOCK):
      case (0xFF15000000LLU + NOP_MEMFENCE):
	  break;
      }
