//
// remotecontrol_or1k - map high-level debugger commands into OR1K-specific status register writes.
//
// Should be usable both with Verilated and FastISS but currently operates on an instance of orsim - if we abstract the relevant parts of the orsim interface (reg reads and writes) we can make this portable. 
//
// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2008  Embecosm Limited <info@embecosm.com>
// (C) 2014 DJ Greaves - Univeristy of Cambridge.
//
//
//
//
#include "remotecontrol_or1k.h"
#include "spr_defs.h"
#include <iostream>


remotecontrol_or1k::remotecontrol_or1k(orsim *iss):
  iss_ptr(iss)// constructor
{
  mpHash = new MpHash();
}


int remotecontrol_or1k::RemoveMatchpoint(MpType type, u32_t addr)
{
  u32_t instr;
  // Sort out the type of matchpoint
  switch (type) 
    {
      
    case BP_MEMORY:
      // Memory breakpoint - replace the original instruction.
      if (mpHash->remove(type, addr, &instr)) 
	{
	  //cerr << "rspRemoveMatchpoint at 0x" << hex << addr << " restoring instruction: 0x" << hex << instr <<endl;
	  iss_ptr->set_mem32(addr, instr, 0);
	}
      
      return 0; //"OK";
      
    case BP_HARDWARE:
      int off;
      for (off = 0; off < 8; off++)
	if ((iss_ptr->get_spr(SPR_DCR0 + off) == (0x23)) &&
	    (iss_ptr->get_spr(SPR_DVR0 + off) == addr))
	  break;
      if (off > 7) return -2;	// Failed ot find breakpoint
      
      // Clear DCR's CT and DVR, WGB bit
      iss_ptr->set_spr(SPR_DCR0 + off, 0);
      iss_ptr->set_spr(SPR_DVR0 + off, 0);
      iss_ptr->set_spr(SPR_DMR2,
			  iss_ptr->get_spr(SPR_DMR2) & ~((1 << off)
							   <<
							   SPR_DMR2_WGB_SHIFT));
      return 0; //"OK"
      
    case WP_WRITE:
      {
	int off;
	for (off = 0; off < 8; off++)
	  {
	    if ((iss_ptr->get_spr(SPR_DCR0 + off) ==
		 (0x63))
		&& (iss_ptr->get_spr(SPR_DVR0 + off) ==
		    addr))
	      break;
	  }
	if (off > 7) return -2;	// Failed ot find breakpoint
	// Clear DCR's CT and DVR, WGB bit
	iss_ptr->set_spr(SPR_DCR0 + off, 0);
	iss_ptr->set_spr(SPR_DVR0 + off, 0);
	iss_ptr->set_spr(SPR_DMR2,
			    iss_ptr->get_spr(SPR_DMR2) &
			    ~((1 << off) <<
			      SPR_DMR2_WGB_SHIFT));
	return 0; // "OK"
    }
      
    case WP_READ:
      {
	int off;
	for (off = 0; off < 8; off++) {
	  if ((iss_ptr->get_spr(SPR_DCR0 + off) ==
	       (0x43))
	      && (iss_ptr->get_spr(SPR_DVR0 + off) ==
		  addr))
	    break;
	}
	if (off > 7) return -2;	// Failed ot find breakpoint
	// Clear DCR's CT and DVR, WGB bit
	iss_ptr->set_spr(SPR_DCR0 + off, 0);
	iss_ptr->set_spr(SPR_DVR0 + off, 0);
	iss_ptr->set_spr(SPR_DMR2,
					    iss_ptr->get_spr(SPR_DMR2) &
			    ~((1 << off) <<
			      SPR_DMR2_WGB_SHIFT));
	return 0; //"OK";
      }
      
    case WP_ACCESS:
      {
	int off;
	for (off = 0; off < 8; off++) {
	  //printf("WP_ACCESS remove check off=%d DCR=0x%.8x DVR=0x%.8x\n",
	  //off,iss_ptr->get_spr (SPR_DCR0+off),iss_ptr->get_spr (SPR_DVR0+off));
	  if ((iss_ptr->get_spr(SPR_DCR0 + off) ==
	       (0xc3))
				    && (iss_ptr->get_spr(SPR_DVR0 + off) ==
					addr))
	    break;
	}
	if (off > 7) 
	  {
	    //printf("rspRemoveWatchpoint: WP_ACCESS remove ERROR, regpair %d for 0x%.8x\n",off, addr);
	    return -2;	// Failed ot find breakpoint
	  }
	//printf("rspRemoveWatchpoint: WP_ACCESS remove, regpair %d for 0x%.8x\n",off, addr);
	
	// Clear DCR's CT and DVR, WGB bit
	iss_ptr->set_spr(SPR_DCR0 + off, 0);
	iss_ptr->set_spr(SPR_DVR0 + off, 0);
	iss_ptr->set_spr(SPR_DMR2,
			    iss_ptr->get_spr(SPR_DMR2) &
			    ~((1 << off) <<
			      SPR_DMR2_WGB_SHIFT));
	return 0; //"OK"
      }
    default:
      std::cerr << "Warning: RSP matchpoint type " << type
                << " not recognized: ignored" << std::endl;
      return -1; 
    }
} // rspRemoveMatchpoint ()



//-----------------------------------------------------------------------------
//! Generic processing of a continue request for OpenRISC.

//! The signal may be EXCEPT_NONE if there is no exception to be
//! handled. Currently the exception is ignored.

//! The single step flag is cleared in the debug registers and then the
//! processor is unstalled.

//! @param[in] addr    Address from which to step
//! @param[in] except  The exception to use (if any)                         
//-----------------------------------------------------------------------------
void remotecontrol_or1k::rspContinue(uint32_t addr, uint32_t except)
{
  // Set the address as the value of the next program counter
  writeNpc(addr);
  /*
  // If we're continuing from a breakpoint, replace that instruction in the memory
  // ... actually no, I was wrong about this.
  if (NULL != mpHash->lookup (BP_MEMORY, addr) && rsp_sigval == TARGET_SIGNAL_TRAP)
  {
  MpEntry *entry = mpHash->lookup (BP_MEMORY, addr);
  iss_ptr->set_mem32(entry->addr, entry->instr, 0);
  }
  */
  
  // Clear Debug Reason Register and watchpoint break generation in Debug Mode
  // Register 2 for watchpoints that we triggered to stop this time.
  iss_ptr->set_spr(SPR_DRR, 0);
  if (rsp_sigval == TARGET_SIGNAL_TRAP) 
    {
      /* 
	 Disable last trap generation on watchpoint if this is why we stopped last time.
      */
      uint32_t temp_dmr2 = iss_ptr->get_spr(SPR_DMR2);
      if (temp_dmr2 & SPR_DMR2_WBS) {
	/* 
	   One of these breakpoints is responsible for our stopping, so
	 disable it this time we start. GDB will send a packet re-enabling
	 it next time we continue.
	*/
	iss_ptr->set_spr(SPR_DMR2,
			 temp_dmr2 &
			 ~((temp_dmr2 & SPR_DMR2_WBS) >>
			   10));
      }
    }
  // Clear the single step trigger in Debug Mode Register 1 and set traps to
  // be handled by the debug unit in the Debug Stop Register
  iss_ptr->andSpr(SPR_DMR1, ~SPR_DMR1_ST);
  iss_ptr->orSpr(SPR_DSR, SPR_DSR_TE);
  
  // Unstall the processor. Note the GDB client is now waiting for a reply,
  // which it will get as soon as the processor stalls again.
 
} // rspContinue ()


int remotecontrol_or1k::InsertMatchpoint(MpType type, u32_t addr) // or1k
{


  if (type == BP_MEMORY) // DJG: i dont know how to make gdb issue hardware breakpoints at the moment so here I kludge it to the sort I want.
    {
      type = BP_HARDWARE;
      printf("Adjusted BP to be of type h/w for address %x\n", addr);
    }
  

  // Sort out the type of matchpoint
  switch (type) {
  case BP_MEMORY:
    // Memory breakpoint - substitute a TRAP instruction
    mpHash->add(type, addr, iss_ptr->eval_mem32(addr, 0));
    iss_ptr->set_mem32(addr, OR1K_TRAP_INSTR, 0);
    return 0; //"OK"
    
  case BP_HARDWARE:
    {
      int off;
      for (off = 0; off < 8; off++)
	if (!
	    (iss_ptr->get_spr(SPR_DCR0 + off) &
	     SPR_DCR_CT_MASK))
	  break;
      if (off > 7) return -2;	// No room
      // CC = equal, CT = Instruction fetch EA, set WGB bit
      iss_ptr->set_spr(SPR_DCR0 + off, 0x22);
      iss_ptr->set_spr(SPR_DVR0 + off, addr);
      iss_ptr->set_spr(SPR_DMR2,
			  iss_ptr->get_spr(SPR_DMR2) |
			  ((1 << off) << SPR_DMR2_WGB_SHIFT));
      return 0; //"OK"
    }

  case WP_WRITE:
    {
      int off;
      for (off = 0; off < 8; off++)
	if (!
	    (iss_ptr->get_spr(SPR_DCR0 + off) &
	     SPR_DCR_CT_MASK))
	  break;
      //printf("rspInsertWatchpoint: WP_WRITE, regpair %d for 0x%.8x\n",off, addr);
      if (off > 7) return -2;	// No room
      // CC = equal, CT = Store EA, set WGB bit
      iss_ptr->set_spr(SPR_DCR0 + off, 0x62);
      iss_ptr->set_spr(SPR_DVR0 + off, addr);
      iss_ptr->set_spr(SPR_DMR2,
			  iss_ptr->get_spr(SPR_DMR2) |
			  ((1 << off) << SPR_DMR2_WGB_SHIFT));
      return 0;//"OK"
    }

  case WP_READ:
    {
      int off;
      for (off = 0; off < 8; off++)
	if (!
	    (iss_ptr->get_spr(SPR_DCR0 + off) &
	     SPR_DCR_CT_MASK))
	  break;
      //printf("rspInsertWatchpoint: WP_WRITE, regpair %d for 0x%.8x\n",off, addr);
      if (off > 7) return -2;	// No room
      // CC = equal, CT = Load EA, set WGB bit
      iss_ptr->set_spr(SPR_DCR0 + off, 0x42);
      iss_ptr->set_spr(SPR_DVR0 + off, addr);
      iss_ptr->set_spr(SPR_DMR2,
			  iss_ptr->get_spr(SPR_DMR2) |
			  ((1 << off) << SPR_DMR2_WGB_SHIFT));
      return 0; // "OK"
    }
    
    
  case WP_ACCESS:
    {
      int off;
      for (off = 0; off < 8; off++)
	if (!
	    (iss_ptr->get_spr(SPR_DCR0 + off) &
	     SPR_DCR_CT_MASK))
	  break;
      //printf("rspInsertWatchpoint: WP_ACCESS, regpair %d for 0x%.8x\n",off, addr);
      if (off > 7) return  -2;	// No room
      // CC = equal, CT = Load/Store EA, set WGB bit
      iss_ptr->set_spr(SPR_DCR0 + off, 0xc2);
      iss_ptr->set_spr(SPR_DVR0 + off, addr);
      iss_ptr->set_spr(SPR_DMR2,
			  iss_ptr->get_spr(SPR_DMR2) |
			  ((1 << off) << SPR_DMR2_WGB_SHIFT));
      return 0; // "OK"
    }
  default:
    std::cerr << "Warning: RSP matchpoint type " << type
              << "not recognized: ignored" << std::endl;
    return -1;
  }	
}// rspInsertMatchpoint ()

//-----------------------------------------------------------------------------
//! Write the value of the Next Program Counter (a SPR)

//! A convenience function.

//! Setting the NPC flushes the pipeline, so subsequent reads will return
//! zero until the processor has refilled the pipeline. This will not be
//! happening if the processor is stalled (as it is when GDB had control).

//! However for debugging we always want to know what the effective value of
//! the NPC will be (i.e. the value that will be used once the pipeline has
//! refilled). Fortunately SPR cacheing in the debug unit silently solves this
//! for us.

//! There is one other caveat for the NPC. We do not wish to write it (whether
//! or not it is cached) if it has not changed. So unlike all other SPRs we
//! always read it first before writing.

//! @param[in]  The address to write into the NPC
//-----------------------------------------------------------------------------
void remotecontrol_or1k::writeNpc(uint32_t addr)
{
  if (addr != readNpc()) {
    iss_ptr->set_spr(SPR_NPC, addr);
  }
} // writeNpc ()


//-----------------------------------------------------------------------------
//! Check if processor is stalled. If it is, read the DRR and return the
//! target signal code.
//-----------------------------------------------------------------------------

int remotecontrol_or1k::get_sigval()
{
  switch ((iss_ptr->get_spr(SPR_DRR) & 0xffffffff)) {
  case SPR_DRR_RSTE:
    rsp_sigval = TARGET_SIGNAL_PWR;
    break;
  case SPR_DRR_BUSEE:
    rsp_sigval = TARGET_SIGNAL_BUS;
    break;
  case SPR_DRR_DPFE:
    rsp_sigval = TARGET_SIGNAL_SEGV;
    break;
  case SPR_DRR_IPFE:
    rsp_sigval = TARGET_SIGNAL_SEGV;
    break;
  case SPR_DRR_TTE:
    rsp_sigval = TARGET_SIGNAL_ALRM;
    break;
  case SPR_DRR_AE:
    rsp_sigval = TARGET_SIGNAL_BUS;
    break;
  case SPR_DRR_IIE:
    rsp_sigval = TARGET_SIGNAL_ILL;
    break;
  case SPR_DRR_IE:
    rsp_sigval = TARGET_SIGNAL_INT;
    break;
  case SPR_DRR_DME:
    rsp_sigval = TARGET_SIGNAL_SEGV;
    break;
  case SPR_DRR_IME:
    rsp_sigval = TARGET_SIGNAL_SEGV;
    break;
  case SPR_DRR_RE:
    rsp_sigval = TARGET_SIGNAL_FPE;
    break;
  case SPR_DRR_SCE:
    rsp_sigval = TARGET_SIGNAL_USR2;
    break;
    //case SPR_DRR_FPE:
    //rsp_sigval = TARGET_SIGNAL_FPE;
    //break;
  case SPR_DRR_TE:
    rsp_sigval = TARGET_SIGNAL_TRAP;
    break;
    
  default:
    // This must be the case of single step (which does not set DRR)
    rsp_sigval = TARGET_SIGNAL_TRAP;
    break;
  }
  return rsp_sigval;
}


void remotecontrol_or1k::step()
{
  // Clear Debug Reason Register and watchpoint break generation in Debug Mode
  // Register 2 for watchpoints that we triggered to stop this time.
  iss_ptr->set_spr(SPR_DRR, 0);
  if (rsp_sigval == TARGET_SIGNAL_TRAP) {
    /* 
       Disable last trap generation on watchpoint if this is why we stopped
		   last time.
    */
    uint32_t temp_dmr2 = iss_ptr->get_spr(SPR_DMR2);
    if (temp_dmr2 & SPR_DMR2_WBS) {
      /* 
	 One of these breakpoints is responsible for our stopping, so
	 disable it this time we start. GDB will send a packet re-enabling
			   it next time we continue.
      */
      iss_ptr->set_spr(SPR_DMR2,
		       temp_dmr2 &
		       ~((temp_dmr2 & SPR_DMR2_WBS) >>
			 10));
    }
  }
  // Set the single step trigger in Debug Mode Register 1 and set traps to be
  // handled by the debug unit in the Debug Stop Register
  iss_ptr->orSpr(SPR_DMR1, SPR_DMR1_ST);
  iss_ptr->orSpr(SPR_DSR, SPR_DSR_TE);
  
  // Unstall the processor. Note the GDB client is now waiting for a reply,
  // which it will get as soon as the processor stalls again.
  iss_ptr->stall(/*unstall=*/true);
}

  
// eof
