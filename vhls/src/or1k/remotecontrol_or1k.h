#ifndef REMOTECONTROL_OR1K_H
#define REMOTECONTROL_OR1K_H
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
#include "tenos.h"
#include "MpHash.h"



#include "fastiss/orsim.h"

class remotecontrol_or1k
{
  TargetSignal rsp_sigval;

  //! Trap instruction for OR32
  static const uint32_t OR1K_TRAP_INSTR = 0x21000001;
  
 
  //! Hash table for matchpoints
  MpHash *mpHash;

  orsim *iss_ptr;





	// Register numbering. Matches OR1K GDB client
#if 0
	// Older OR1K specific follows
	static const int max_gprs = 32;	//!< Max number of OR1K GPRs

	static const int PPC_REGNUM = max_gprs + 0;	//!< Previous PC
	static const int NPC_REGNUM = max_gprs + 1;	//!< Next PC
	static const int SR_REGNUM = max_gprs + 2;	//!< Supervision Register

	static const int NUM_REGS = max_gprs + 3;	//!< Total GDB registers


	// OpenRISC exception addresses. Only the ones we need to know about
	static const uint32_t EXCEPT_NONE = 0x000;	//!< No exception
	static const uint32_t EXCEPT_RESET = 0x100;	//!< Reset


	// SPR numbers
	static const uint16_t SPR_NPC = 0x0010;	//!< Next program counter
	static const uint16_t SPR_SR = 0x0011;	//!< Supervision register
	static const uint16_t SPR_PPC = 0x0012;	//!< Previous program counter
	static const uint16_t SPR_GPR0 = 0x0400;	//!< GPR 0

#endif
	//Some of these are needed...
	static const uint16_t SPR_DVR0 = 0x3000;	//!< Debug value register 0
	static const uint16_t SPR_DVR1 = 0x3001;	//!< Debug value register 1
	static const uint16_t SPR_DVR2 = 0x3002;	//!< Debug value register 2
	static const uint16_t SPR_DVR3 = 0x3003;	//!< Debug value register 3
	static const uint16_t SPR_DVR4 = 0x3004;	//!< Debug value register 4
	static const uint16_t SPR_DVR5 = 0x3005;	//!< Debug value register 5
	static const uint16_t SPR_DVR6 = 0x3006;	//!< Debug value register 6
	static const uint16_t SPR_DVR7 = 0x3007;	//!< Debug value register 7


	static const uint16_t SPR_DCR0 = 0x3008;	//!< Debug control register 0
	static const uint16_t SPR_DCR1 = 0x3009;	//!< Debug control register 1
	static const uint16_t SPR_DCR2 = 0x300a;	//!< Debug control register 2
	static const uint16_t SPR_DCR3 = 0x300b;	//!< Debug control register 3
	static const uint16_t SPR_DCR4 = 0x300c;	//!< Debug control register 4
	static const uint16_t SPR_DCR5 = 0x300d;	//!< Debug control register 5
	static const uint16_t SPR_DCR6 = 0x300e;	//!< Debug control register 6
	static const uint16_t SPR_DCR7 = 0x300f;	//!< Debug control register 7  
#if 0
	static const uint16_t SPR_DMR1 = 0x3010;	//!< Debug mode register 1
	static const uint16_t SPR_DMR2 = 0x3011;	//!< Debug mode register 2
	static const uint16_t SPR_DSR = 0x3014;	//!< Debug stop register
	static const uint16_t SPR_DRR = 0x3015;	//!< Debug reason register

	// SPR masks and offsets
	static const uint32_t SPR_DMR1_ST = 0x00400000;	//!< Single-step trace
	static const uint32_t SPR_DMR2_WGB = 0x003ff000;	//!< W/pt generating B/pt
#endif
	static const uint32_t SPR_DMR2_WBS = 0xffc00000;	//!< W/pt B/pt status
#if 0
	static const uint32_t SPR_DSR_TE = 0x00002000;	//!< Trap
	static const uint32_t SPR_DCR_DP_MASK = 0x00000001;	//!< Debug Pair Present
	static const uint32_t SPR_DCR_CC_MASK = 0x0000000e;	//!< Compare Condition
	static const uint32_t SPR_DCR_SC_MASK = 0x00000010;	//!< Signed Comparison
#endif
	static const uint32_t SPR_DCR_CT_MASK = 0x000000e0;	//!< Compare To

	static const uint32_t SPR_DMR2_WGB_SHIFT = 12;	//!< W/pt Generate B/pt
#if 0
	// DRR (Debug Reason Register) Bits
	static const uint32_t SPR_DRR_RSTE = 0x00000001;	//!< Reset
	static const uint32_t SPR_DRR_BUSEE = 0x00000002;	//!< Bus error
	static const uint32_t SPR_DRR_DPFE = 0x00000004;	//!< Data page fault
	static const uint32_t SPR_DRR_IPFE = 0x00000008;	//!< Insn page fault
	static const uint32_t SPR_DRR_TTE = 0x00000010;	//!< Tick timer
	static const uint32_t SPR_DRR_AE = 0x00000020;	//!< Alignment
	static const uint32_t SPR_DRR_IIE = 0x00000040;	//!< Illegal instruction
	static const uint32_t SPR_DRR_IE = 0x00000080;	//!< Interrupt
	static const uint32_t SPR_DRR_DME = 0x00000100;	//!< DTLB miss
	static const uint32_t SPR_DRR_IME = 0x00000200;	//!< ITLB miss
	static const uint32_t SPR_DRR_RE = 0x00000400;	//!< Range fault
	static const uint32_t SPR_DRR_SCE = 0x00000800;	//!< System call
#endif
	static const uint32_t SPR_DRR_FPE = 0x00001000;	//!< Floating point
#if 0
	static const uint32_t SPR_DRR_TE = 0x00002000;	//!< Trap



	// The bounds of flash memory
	uint32_t flashStart;	//<! Start of flash memory
	uint32_t flashEnd;	//<! End of flash memory

#endif
  // Convenience wrappers for getting particular registers, which are really
  // SPRs.
  void writeNpc(uint32_t addr);   //{ iss_ptr->set_spr(SPR_NPC, addr); }     
  u32_t readNpc() { return iss_ptr->get_spr(SPR_NPC); }

 public:
  remotecontrol_or1k(orsim *); // constructor

  int get_sigval();
  int RemoveMatchpoint(MpType type, u32_t addr);
  void rspContinue(uint32_t addr, uint32_t except);
  int InsertMatchpoint(MpType type, u32_t addr);
  void step();
};


#endif

// eof

