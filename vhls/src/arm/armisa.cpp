// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>


//

////////////////////////////////////////////////////////////////////////////
//
// armisa.cpp    ARM instruction sets.
//
/////////////////////////////////////////////////////////////////////////////

#include "tenos.h"
#include "argv_backdoor.h" // Needed for symbolic disassembly.
#include "io_backdoor.h"
#include "../gdbrsp/vhls_soc_debug_spr.h"

#define FATAL(X) { printf(X); exit(1); }

#define OLDTRACE(X) { printf(X); printf ("\n"); }

// The following TRACEX macros are for debuggng the ISS.  Use the Tenos trace facilities for debugging user programs.
#define TRACE5(X) 
#define TRACE4(X)
#define TRACE3(X)
#define TRACE2(X)
#define TRACE1(X) 
#define FALSE 0
#define TRUE 1
#define SKIP(X)
#define BUSIF_TRC(X)
#define FLOAT_TRACE(X)


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "tenos.h"
#include "armisa.h"

#if THREAD_COMM
extern int g_thread_comm;

addr_owners_t g_thread_comm_producers;
uint64_t** g_thread_comm_consumers;

using namespace boost::icl;
#endif

#if TRACECOMM
extern int g_tracecomm;
#endif
extern timeval g_te;

// Needed for symbolic disassembly - but this assumes there is only one type of processor core with one flat address space!
extern argv_backdoor_setup *argv_backdoor_su;
extern io_backdoor_setup *io_backdoor_su;

#define NOTDONE assert(false);

// These two macros look at the sign bit and used to generate the V flag.
#define POS(X) (~(X) >> 31)
#define NEG(X) ((X) >> 31)


#include "ptrace_v1.h"

#define   ARM32_INSTRUCTION_PTRACE(core_no, executedf, pc, ins32) BIN_INSTRUCTION_PTRACE(core_no, ARM32_SET, executedf, pc, ins32)



//--------------------------------------------------------------------------

int armisa::delete_breakpoint(u32_t addr, int specific_which /*-1 for match a breakpoint matching on addr*/)
{
  int n = specific_which;

  if (n < 0) // scan for address
    {
      for (n=0; n < breakpoints_active; n++)
	{
	  if (pc_breakpoints[n].addr == addr) break;
	}
      if (n==breakpoints_active) return -2; // not found
    }
  assert(n < breakpoints_active);

  for (int m = n; m < breakpoints_active; m++)
    {
      pc_breakpoints[m] = pc_breakpoints[m+1];
    }
  breakpoints_active --;
  return 0;
}


int armisa::set_breakpoint(u32_t addr, wp_style_t style)
{
  instruction_grace = true;
  int i;
  for(i=0;i<breakpoints_active;i++)
    {
      if (pc_breakpoints[i].addr == addr)
	{
	  pc_breakpoints[i].style = style;
	  break;
	}

    }
  if (i == breakpoints_active)
    {
      if (breakpoints_active < MAX_BREAKPOINTS)
	{
	  pc_breakpoints[breakpoints_active].style = style;
	  pc_breakpoints[breakpoints_active++].addr = addr;
	  printf ("pc_breakpoint added %i at addr %x\n", breakpoints_active, addr);
	  return 0;
	}
      return -1;
    }
  return 0;
}

int armisa::set_watchpoint(u32_t addr, wp_style_t style)
{
  int i;
  for(i=0;i<watchpoints_active;i++)
    {
      if (watchpoints[i].addr == addr)
	{
	  watchpoints[i].style = style;
	  break;
	}
    }
  if (i == watchpoints_active)
    {
      if (watchpoints_active < MAX_WATCHPOINTS)
	{
	  watchpoints[watchpoints_active].style = style;
	  watchpoints[watchpoints_active++].addr = addr;
	  return 0;
	}
      return -1;
    }
  return 0;
}

u32_t sex16(u32_t d)
{
  if (d & 0x8000) return d | 0xFFFF0000;
  else return d;
}

u32_t sex8(u32_t d)
{
  if (d & 0x80) return d | 0xFFFFFF00;
  else return d;
}

void armisa::setVectorBase(u32_t addr)
{
  vectorBase = addr;
  if (logfd) fprintf(logfd, "Set Vector Base to %x\n", addr);
}

//
// This command stub is designed to be callable by foreign threads.
// We now use the Tenos interface instead.
#if 0
int armisa::old_command_cmd(int command_code, u32_t arg)
{
  switch(command_code)
    {
    case armisa_cmd_stop:
      if (logfd) fprintf(logfd, "Command: Stop\n");
      halted = true;
      break;

    case armisa_cmd_reset:
      if (logfd) fprintf(logfd, "Command: Reset\n");
      runstate = ResettingS;
      break;

    case armisa_cmd_resume:
      if (logfd) fprintf(logfd, "Command: Resume\n");
      halted = false;
      runstate = StartingS;
      break;

    case armisa_cmd_enquire:
      return (int) runstate;
     
    default:
      printf("%s: illegal command code %i\n",  myname(), command_code);
    }
  return 0;
}
#endif


//--------------------------------------------------------------------------

const char *armisa::ArmBankToString(ArmBank a) const
{
    switch (a)
    {
    case UserBank:          return "user";
    case FiqBank:           return "fiq";
    case SupervisorBank:    return "supervisor";
    case AbortBank:         return "abort";
    case IrqBank:           return "irq";
    case UndefinedBank:     return "undefined";
    default: assert(0); return "?";
    }
}


const char *armisa::state_string(unsigned S)
{
  switch((enum ArmState) S)
    {
      //case        StoppedS: return "Stopped";
    case        ResettingS: return "Resetting";	     // restarted,	 setting PC+mode
    case        StartingS: return "Starting";	      // running,	 abandon old memory request
      //    case        LoadingS: return "Loading";	       // running,	 loading new instruction
    case        RunningS: return "Running";	       // running,	 pipelined instruction fetch issued.
    case        MidSingleS: return "MidSingle";	     // running,	 mid-instruction,	 LDR/STR
    case        MidHalfwordS: return "MidHalfword";
      
    case        MidCoproXferS: return "MidCoproXfer";	  
    case        MidMultipleS: return "MidMultiple";	   // running,	 mid-instruction,	 LDM/STM
    case        MidMultiple2S: return "MidMultiple2";	  // running,	 mid-instruction,	 LDM/STM,	 part 2
    }
  return "BadState";
}


void armisa::PrintRegs(FILE *fd, bool allf_, bool atStartOnly)
{
  if (atStartOnly && runstate != RunningS) return;
  fprintf(fd, "%s:  r0=%08X  r1=%08X  r2=%08X  r3=%08X\n", myname(), Reg(0), Reg(1), Reg(2), Reg(3));
  fprintf(fd, "%s:  r4=%08X  r5=%08X  r6=%08X  r7=%08X\n", myname(), Reg(4), Reg(5), Reg(6), Reg(7));
  fprintf(fd, "%s:  r8=%08X  r9=%08X r10=%08X r11=%08X  %c%c%c%c%c %c%c%c\n",   myname(), Reg(8), Reg(9), Reg(10), Reg(11),
                  ((Reg(CpsrReg) & 0x80000000) ? 'N' : '.'), ((Reg(CpsrReg) & 0x40000000) ? 'Z' : '.'),
                  ((Reg(CpsrReg) & 0x20000000) ? 'C' : '.'), ((Reg(CpsrReg) & 0x10000000) ? 'V' : '.'),
                  ((Reg(CpsrReg) & 0x08000000) ? 'Q' : '.'), ((Reg(CpsrReg) & 0x00000080) ? 'I' : '.'),
                  ((Reg(CpsrReg) & 0x00000040) ? 'F' : '.'), ((Reg(CpsrReg) & 0x00000020) ? 'T' : '.') );
  fprintf(fd, "%s: r12=%08X r13=%08X  lr=%08X  pc=%08X, %08X\n", myname(), Reg(12), Reg(13), Reg(14), Reg(15), Reg(CpsrReg));
}

void armisa::PrintStats()
{
   SKIP("Cycles=%i  Instructions=%i " _ Cycles _ Instructions);
}

const char *armisa::ArmModeToString(ArmMode a) const
{
    switch (a)
    {
    case UserMode:          return "user";
    case FiqMode:           return "fiq";
    case IrqMode:           return "irq";
    case SupervisorMode:    return "supervisor";
    case AbortMode:         return "abort";
    case UndefinedMode:     return "undefined";
    case SystemMode:        return "system";
    default: assert(FALSE); return "?";
    }
}


const char *armisa::ArmVectorToString(ArmVector a) const
{
    switch (a)
    {
    case ResetVector:       return "reset";
    case UndefinedVector:   return "undefined";
    case SwiVector:         return "swi";
    case PrefetchVector:    return "prefetch abort";
    case DataVector:        return "data abort";
    case ReservedVector:    return "reserved";
    case IrqVector:         return "irq";
    case FiqVector:         return "fiq";
    default: assert(FALSE); return "?";
    }
}

// --------------------------------------------------------------------------
//
extern int cmd_option(char *); 


// constructor 
armisa::armisa(int core_no, const char *myname):
  m_core_no(core_no),
  m_myname(strdup(myname)),
  wait_for_interrupt(false),
  wait_for_event(false),
  event_register(false),
  last_event(0),
  inter_core_event(NULL),
  last_shutdown_sequence(false),
  shutdown_sequence(false),
  ITSTATE(0)
{
  m_abtb = 0; // Don't have one by default.

  //This slows down simulation but speeds up real hardware:
  m_branch_predictor = new generic_branch_predictor(/*log capacity=*/9/*512 entries in btb, guess for now*/, /*ways*/2, /*local_table_size*/4096);

  new tenos_report_item(myname, "cycles", &stats.m_Cycles);
  new tenos_report_item(myname, "instructions", &stats.m_Instructions);
  new tenos_report_item(myname, "blocks", &stats.m_InstructionBlocks);
  instruction_grace = false;
  stop_after_ins_count = -1;
  watchpoints_active = 0;
  breakpoints_active = 0;
  m_tracing_enabled = false;
  logfd = NULL;
  //logfd = stdout;
  runstate = ResettingS;
  nFIQ = 1;
  nIRQ = 1;
  interrupt_wait = 0;

  // IPC analysis
  m_uses_defs_idx = 0;
  m_thumb16_only_ipc = false;
  m_thumb32 = false;

  insRt = -1;
  insRt2 = -1;
}



/////////////////////////////////////////////////////////////////////////////
//
// Low-level register access
//
/////////////////////////////////////////////////////////////////////////////


u32_t &armisa::Reg(ArmBank bank, unsigned r)
{
    //
    // Provide the specified register in this mode,
    // providing banked versions as appropriate.
    //

  if (r >= 18 || (unsigned) bank >= 16)
    {
      tenos_diagnostics.checkpoint(2, myname(), "bad reg/bank");
      assert (r != UndefinedReg);
      assert(r < 18);
      assert((unsigned) bank < 16);
    }

  if (bank == UserBank || r == 15 || r == 16)
    {
      if(r == 17) {
          printf("Executing instruction at %08x -- %08x\n", insAddress, instruction);
	  assert(0);
      }
      return raw_reg[r];
    }
  else if (bank == FiqBank)
    {
      if (r < 8) return raw_reg[r]; else return raw_reg[18 + r];
    }
  else
    {

      // System mode seems same banks as User mode.
    
      /* Registers 13 and 14 (sp and lr) are banked in modes other */
      /* than FIQ                                                  */
      if (r < 13) return raw_reg[r]; else return raw_reg[18*(unsigned)bank + r];
    }
}

void armisa::SetMode(ArmMode m, bool thumb_flag)
{
    //
    // Set processor mode, register bank, and M bits in CPSR.
    //
    prev_thumb_mode = thumb_mode;
    thumb_mode = thumb_flag;
    assert((unsigned) m < 7);

    currentMode = m;
    // Set current bank
    currentBank = (ArmBank) ("\x00\x01\x04\x02\x03\x05\x00"[(unsigned) m]);

    //printf("New mode %x and bank %x\n", m, currentBank);

    // Set relevant bits of CPSR
    u32_t cpsr = raw_reg[(unsigned) CpsrReg] & 0xFFFFFFE0UL; // clear M4..M0
    //printf("mode is %d and old cpsr is %08x\n", m, cpsr);
    cpsr |= "\x10\x11\x12\x13\x17\x1b\x1f"[(unsigned) m];
    //printf("value %08x and new cpsr is %08x\n", "\x10\x11\x12\x13\x17\x1b\x1f"[(unsigned) m], cpsr);

    // if we are serving an interrupt then we should
    // mask all other interrupts and unmask them
    // on returning from interrupt routine so
    // that we can serve next interrupts
    if(m == IrqMode && !(cpsr & 0x80)) {
      // cpsr[7] = interrupt bit is unmask
      // we are going to mask it now
      cpsr |= (1 << 7);
    }
    raw_reg[(unsigned) CpsrReg] = cpsr;
}

/////////////////////////////////////////////////////////////////////////////
//
// Decode of ARM instructions
//
/////////////////////////////////////////////////////////////////////////////

//
// This does all the real thinking for executing an ARM instruction.
// If possible it does the entire instruction. If memory cycles are
// involved then the 'state' must be set to perform follow-on portions
// of the instruction.
//

void armisa::DecodeCcode(u4_t conder)
{
  switch (conder)
    {
    case 0:     insCcodeTest =   ZFlag(); break;     // EQ
    case 1:     insCcodeTest = ! ZFlag(); break;     // NE
    case 2:     insCcodeTest =   CFlag(); break;     // CS, HS
    case 3:     insCcodeTest = ! CFlag(); break;     // CC, LO
    case 4:     insCcodeTest =   NFlag(); break;     // MI
    case 5:     insCcodeTest = ! NFlag(); break;     // PL
    case 6:     insCcodeTest =   VFlag(); break;     // VS (overflow)
    case 7:     insCcodeTest = ! VFlag(); break;     // VC
    case 8:     insCcodeTest =   CFlag() && ! ZFlag(); break; // HI (unsigned higher)
    case 9:     insCcodeTest = (! CFlag()) || ZFlag(); break; // LS
    case 10:    insCcodeTest = NFlag() == VFlag();     break; // GE (signed)
    case 11:    insCcodeTest = NFlag() != VFlag();     break; // LT
    case 12:    insCcodeTest = (!ZFlag()) && (NFlag() == VFlag());  break; // GT
    case 13:    insCcodeTest = ZFlag() || (NFlag() != VFlag());     break; // LE
    case 14:    insCcodeTest = TRUE;                                break; // always
    case 15:    assert(0);
    }
}


//
//
//
void armisa::updateFlags(fmode_t fmode, bool carry, bool overflow)    // Write back new versions of flags N, Z, C, V.
{ 
  u32_t &cpsr = Reg(CpsrReg);
  u1_t newN;
  u1_t newZ;
  if(fmode == FMODE_logic64) {
      u64_t result = (((u64_t)insAluResultHi) << 32) | insAluResult;
      newN = result < 0ll;
      newZ = result == 0llu;
  } else {
      newN = Bit(insAluResult, 31);
      newZ = insAluResult == 0;
  }

  if (fmode == FMODE_arith32)
    {
      u32_t newC = carry;
      u32_t newV = overflow;
      if (insRd != 15 || !insWBdefault)
	{
	  // Write to PC with S does something different,
	  // (ref ARM7 4.5.4 pp 58) and see below
	  u32_t &cpsr = Reg(CpsrReg);
	  cpsr &= 0x0FFFFFFFUL; // clear the flags
	  cpsr |= (newN << 31) + (newZ << 30) + (newC << 29) + (newV << 28);
	  TRACE5("ALU op sets flags %c %c %c %c" _
		 (newN ? 'N' : 'n') _
		 (newZ ? 'Z' : 'z') _
		 (newC ? 'C' : 'c') _
		 (newV ? 'V' : 'v'));
	  // Q bit needed for saturating arithmetic extensions.
	}
    }
  else if (fmode == FMODE_logic32)
    {
      u1_t newC = insShiftCarryOut; 
      cpsr &= 0x1FFFFFFFUL; // clear the N, Z and C flags and leave remainder (VQ etc) unchanged.
      cpsr |= (newN << 31) + (newZ << 30)  + (newC << 29);
      TRACE5("ALU MUL op sets flags %c %c " _
	     (newN ? 'N' : 'n') _
	     (newZ ? 'Z' : 'z') _
	     (newC ? 'C' : 'c'));
    }
  else assert(0);
}

void armisa::exec_shift_operand_instr() {
    add_use(m_uses_defs_idx, insRm);
    add_use(m_uses_defs_idx, insRn);
    add_use(m_uses_defs_idx, insRs);
  
    unsigned shift = insShiftValue;
    unsigned rm = Reg(insRm); // unshifted value

    if (insShiftComputed)
    {
        //
        // Shift by register content
        //
        shift = insShiftValue & 255; // shift amount in bottom byte of Rs
        if (insRm == 15)
        {
            //
            // PC operand and computed shift, PC now PC+12
            // (ref ARM7 4.5.5 pp 58)
            //
            rm += 4;
        }

        //
        // This has a side-effect of taking an extra tick.
        //
        //                    tickswait(1);  TODO reimplement
    }

    switch (insShiftType)
      {
      case 0:
	//
	// logical left by computed amount
	//
	if (shift == 0)
	  {
	    insShiftCarryOut = CFlag(); // unchanged carry flag if no shift specified
	    insAluOperand2 = rm;
	  }
	else if (shift > 31)
	  {
	    insShiftCarryOut = Bit(rm, 0); // 'flexible second operand'
	    insAluOperand2 = 0;
	  }
	else
	  {
	    insShiftCarryOut = Bit(rm, 32 - shift);
	    insAluOperand2 = rm << shift;
	  }
	break;
        
      case 1:
	//
	// logical right by computed amount
	//
	if (shift == 0)
	  {
	    insShiftCarryOut = CFlag();
	    insAluOperand2 = rm;
	  }
	else if (shift > 31)
	  {
	    insShiftCarryOut = Bit(rm, 31);
	    insAluOperand2 = 0;
	  }
	else
	  {
	    insShiftCarryOut = Bit(rm, shift - 1);
	    insAluOperand2 = rm >> shift;
	  }
	break;
        
      case 2:
	//
	// arithmetic right by computed amount
	//
	if (shift == 0)
	  {
	    insShiftCarryOut = CFlag();
	    insAluOperand2 = rm;
	  }
	else if (shift > 31)
	  {
	    insShiftCarryOut = Bit(rm, 31);
	    insAluOperand2 = 0 - insShiftCarryOut; // 0 or -1
	  }
	else
	  {
	    insShiftCarryOut = Bit(rm, shift - 1);
	    insAluOperand2 = rm >> shift;
	    if (Bit(rm, 31)) insAluOperand2 |= 0xFFFFFFFFUL << (32 - shift);
	  }
	break;
        
      case 3:
	//
	// rotate right by computed amount
	//
	shift &= 31;
        
	if (shift == 0)
	  {
	    insShiftCarryOut = CFlag();
	    insAluOperand2 = (rm >> 1) | (CFlag() << 31);
	  }
	else
	  {
	    insShiftCarryOut = Bit(rm, shift - 1);
	    insAluOperand2 = Ror(rm, shift);
	  }
	break;
        
      default: assert(FALSE);
      }
}

void armisa::exec_parallel_alu_operand_instr() {
    add_use(m_uses_defs_idx, insRn);
    add_use(m_uses_defs_idx, insRm);
    add_def(m_uses_defs_idx,insRd);
  
    switch(insALUOpcode) {
      case 0: {
          // UADD8
          u32_t sum1 = (Reg(insRn) & 0xFF) + (Reg(insRm) & 0xFF);
          u32_t sum2 = ((Reg(insRn) >> 8) & 0xFF) + ((Reg(insRm) >> 8) & 0xFF);
          u32_t sum3 = ((Reg(insRn) >> 16) & 0xFF) + ((Reg(insRm) >> 16) & 0xFF);
          u32_t sum4 = ((Reg(insRn) >> 24) & 0xFF) + ((Reg(insRm) >> 24) & 0xFF);
          Reg(insRd) = 
              (sum1 & 0xFF) |
              ((sum2 & 0xFF) << 8) |
              ((sum3 & 0xFF) << 16) |
              ((sum4 & 0xFF) << 24);
          
          u32_t &cpsr = Reg(CpsrReg);
          cpsr &= 0xFFF0FFFFUL; // clear flags, bits 16 to 19
          if(sum1 >= 256) cpsr |= (1 << 16);
          if(sum2 >= 256) cpsr |= (1 << 17);
          if(sum3 >= 256) cpsr |= (1 << 18);
          if(sum4 >= 256) cpsr |= (1 << 19);

          break;
      }
      default: {
          printf("Unknown parallel add/sub - %d\n", insALUOpcode);
          assert(0);
      }
    }
}

void armisa::exec_alu_operand_instr() {
    //
    // Perform ALU operation
    //

    add_use(m_uses_defs_idx, insRm);
    add_use(m_uses_defs_idx, insRn);
    add_use(m_uses_defs_idx, insRs);

    insWBdefault = FALSE;
    bool arithmetic = FALSE; // else logical
    u32_t carry = 0;
    u32_t overflow = 0;

    switch (insALUOpcode)
    {
      //AND, EOR, SUB, RSB, ADD, ADC, SBC, RSC
      //TST, TEQ, CMP, CMN, ORR, MOV, BIC, MVN
      case 0:
        //
        // AND
        //
        insAluResult = insAluOperand1 & insAluOperand2;
        insWBdefault = TRUE;
        break;
        
      case 1:
        //
        // EOR
        //
        insAluResult = insAluOperand1 ^ insAluOperand2;
        insWBdefault = TRUE;
        break;
        
      case 2:
        //
        // SUB
        //
        insAluResult = insAluOperand1 - insAluOperand2;
        arithmetic = TRUE;
        insWBdefault = TRUE;
        if (insAluOperand1 >= insAluOperand2) carry = 1;
        overflow = 
            ( (NEG(insAluOperand1) && POS(insAluOperand2) && POS(insAluResult)) ||
              (POS(insAluOperand1) && NEG(insAluOperand2) && NEG(insAluResult)) ) ;
        break;
        
        
      case 3:
        //
        // RSB
        //
        insAluResult = insAluOperand2 - insAluOperand1;
        arithmetic = TRUE;
        insWBdefault = TRUE;
        if (insAluOperand2 >= insAluOperand1) carry = 1;
        overflow = 
            ( (NEG(insAluOperand2) && POS(insAluOperand1) && POS(insAluResult)) ||
              (POS(insAluOperand2) && NEG(insAluOperand1) && NEG(insAluResult)) ) ;
        break;
        
      case 4:
        //
        // ADD
        //
        insAluResult = insAluOperand1 + insAluOperand2;
        arithmetic = TRUE;
        insWBdefault = TRUE;
        if (insAluResult < insAluOperand1) carry = 1;
        overflow = 
            ((NEG(insAluOperand1) && NEG(insAluOperand2) && POS(insAluResult)) ||
             (POS(insAluOperand1) && POS(insAluOperand2) && NEG(insAluResult)) ) ;
        break;
        
      case 5:
        //
        // ADC
        //
        insAluResult = insAluOperand1 + insAluOperand2 + CFlag();
        arithmetic = TRUE;
        insWBdefault = TRUE;
        carry = (CFlag()) ? (insAluResult <= insAluOperand1): (insAluResult < insAluOperand1);
        overflow = 
            ((NEG(insAluOperand1) && NEG(insAluOperand2) && POS(insAluResult)) ||
             (POS(insAluOperand1) && POS(insAluOperand2) && NEG(insAluResult)) ) ;
        break;
        
      case 6:
        //
        // SBC
        //
        insAluResult = insAluOperand1 - insAluOperand2 + CFlag() - 1;
        arithmetic = TRUE;
        insWBdefault = TRUE;
        carry = (CFlag()) ? (insAluOperand1 >= insAluOperand2): (insAluOperand1 > insAluOperand2);
        overflow = 
            ( (NEG(insAluOperand1) && POS(insAluOperand2) && POS(insAluResult)) ||
              (POS(insAluOperand1) && NEG(insAluOperand2) && NEG(insAluResult)) ) ;
        break;
        
      case 7:
        //
        // RSC
        //
        insAluResult = insAluOperand2 - insAluOperand1 + CFlag() - 1;
        arithmetic = TRUE;
        insWBdefault = TRUE;
        carry = (CFlag()) ? (insAluOperand2 >= insAluOperand1): (insAluOperand2 > insAluOperand1);
        overflow = 
            ( (NEG(insAluOperand2) && POS(insAluOperand1) && POS(insAluResult)) ||
              (POS(insAluOperand2) && NEG(insAluOperand1) && NEG(insAluResult)) ) ;
        break;
        
        
      case 8:
        //
        // TST
        //
        if(!thumb_mode && (Bit(instruction, 20) == 0)) {
            // there is a special case for MOVW instructions
            u32_t imm4 = Bits4(instruction, 16);
            u32_t imm12 = Bits(instruction, 0, 12);
            insAluResult = (imm4 << 12 | imm12);
            insWBdefault = TRUE;
        }
        else {
            insAluResult = insAluOperand1 & insAluOperand2;
        }
        break;
        
      case 9:
        //
        // TEQ
        //
        insAluResult = insAluOperand1 ^ insAluOperand2;
        break;
        
      case 10:
        //
        // CMP
        //
        insAluResult = insAluOperand1 - insAluOperand2;
        arithmetic = TRUE;
        if (insAluOperand1 >= insAluOperand2) carry = 1;
        overflow = 
            ( (NEG(insAluOperand1) && POS(insAluOperand2) && POS(insAluResult)) ||
              (POS(insAluOperand1) && NEG(insAluOperand2) && NEG(insAluResult)) ) ;
        break;
        
      case 11:
        //
        // CMN
        //
        insAluResult = insAluOperand1 + insAluOperand2;
        arithmetic = TRUE;
        if (insAluResult < insAluOperand1) carry = 1;
        overflow = 
            ((NEG(insAluOperand1) && NEG(insAluOperand2) && POS(insAluResult)) ||
             (POS(insAluOperand1) && POS(insAluOperand2) && NEG(insAluResult)) ) ;
        break;
        
      case 12:
        //
        // ORR
        //
        insAluResult = insAluOperand1 | insAluOperand2;
        insWBdefault = TRUE;
        break;
        
      case 13:
        //
        // MOV
        //
        insAluResult = insAluOperand2;
        insWBdefault = TRUE;
        break;
        
      case 14:
        //
        // BIC
        //
        insAluResult = insAluOperand1 & (~ insAluOperand2);
        insWBdefault = TRUE;
        break;
        
      case 15:                // MVN
        insAluResult = ~ insAluOperand2;
        insWBdefault = TRUE;
        break;
        
      default: assert(FALSE);
    }
    
    if (insS) updateFlags(arithmetic ? FMODE_arith32:FMODE_logic32, carry, overflow);
    
    else
    {
        // Neither S nor insWBdefault,
        // (ref ARM7 4.5.6) says "should always set the S flag" here
        if (!insWBdefault) TRACE2("ALU strange op, TEQ/TST/CMP/CMN with no S");
    }
    
    if (insWBdefault)
    {
      	add_def(m_uses_defs_idx,insRd);
        TRACE5("ALU op writes 0x%x to R%u" _ insAluResult _ insRd);
        Reg(insRd) = insAluResult;
	OPERAND_PTRACE(insAluResult);                      
        if (insRd == 15) // If write to PC..
        {
            // pipelineBreak = TRUE;
            
            Reg(insRd) = insAluResult + ARM_PC_CORRECTION; // Actually add on 4
            
            if (0 != (insAluResult & 3))
            {
                // Probably a programming error
                TRACE2("Warning - ALU write to PC not a word address");
            }
            
            if (insAluResult == 0)
            {
                // Probably a programming error?
                TRACE2("Warning - ALU write PC as 0");
            }
            
            if (insS)
            {
                // Write to PC and S - restores CPSR from current SPSR
                u32_t newv = Reg(SpsrReg);
                
                if(logfd) {
                    fprintf(logfd, "Returning from interrupt: 0x%08x, pc is 0x%08x, cpsr=0x%08x, spsr=0x%08x\n", newv, Reg(insRd), Reg(CpsrReg), Reg(SpsrReg));
                }
                
                
                if (currentMode == UserMode)
                {
                    // "should not" says (ref ARM7 4.5.4)
			  TRACE2("Warning - ALU write to PC and S in user mode");
                }
		
                if (Bits(Reg(CpsrReg), 8, 20) != Bits(Reg(SpsrReg), 8, 20))
                {
                    //
                    // (ref ARM7 4.6.2 pp 62), reserved CPSR bits "should" be unchanged
                    //
                    TRACE2("Warning - ALU write to PC changes CPSR reserved bits");
                }
                
                Reg(CpsrReg) = newv;
                DecodeSetModeFromCpsr();
            }
        }
    }
}

void armisa::exec_float_alu_instr() {
  if(insFloatSingle) {
    u32_t* data = (u32_t*)(&d_reg[0]);
    
    float op1 = *((float*)(&data[insRn]));
    float op2 = *((float*)(&data[insRm]));

    float res = 0;
    
    switch(insALUOpcode) {
      case 0: { // VMLA
	float addend = op1 * op2;
	float accum = *((float*)(&data[insRd]));

	if(insU)
	  res = accum + addend;
	else
	  res = accum - addend;
	
	break;
      }

      case 1: { // VDIV
	res = op1/op2;
	FLOAT_TRACE(printf("0x%08x -- VDIV(%.4f[S%d]/%.4f[S%d])=%.4f[S%d]\n", \
			   insAddress, op1, insRn, op2, insRm, res, insRd););


	break;
      }

      case 2: { // VMUL
        res = op1 * op2;
	
	break;
      }

      case 3: { // VADD
	res = op1 + op2;

	break;
      }

      case 4: { // VNMLA
	float product = op1 * op2;
	float d = *((float*)(&data[insRd]));

	res = -d - product;

	break;
      }

      case 5: { // VNMLS
	float product = op1 * op2;
	float d = *((float*)(&data[insRd]));

	res = product - d;

	break;
      }

      case 6: { // VMOV (register)
	res = op2;

	break;
      }

      case 7: { // VCMP{E}
	float val = *((float*)(&data[insRd]));
	fpscr_val = (val == 0.0) ? 0x6 : (val > 0.0) ? 0x2 : 0x8;

	/* do not update destination register */
	return;
      }

      case 13: { // VMOV
	FLOAT_TRACE(printf("0x%08x -- S%d=MOV(0x%08x(%.4f))\n",\
			   insAddress, insRd, insAluOperand1, *((float*)(&insAluOperand1))););
	data[insRd] = insAluOperand1;
	return;
	
      }

      case 14: { // VNUM
	res = op1 * op2 * -1;
	break;
      }
	
      default: {
	assert(0);
      }
    }

    data[insRd] = *((u32_t*)(&res));
  }
  else {
    // double
    double op1 = *((double*)(&d_reg[insRn]));
    double op2 = *((double*)(&d_reg[insRm]));

    double res = 0;

    switch(insALUOpcode) {
      case 0: { // VMLA
	double addend = op1 * op2;
	double accum = *((double*)(&d_reg[insRd]));

	if(insU == 0)
	  res = accum + addend;
	else
	  res = accum - addend;

	FLOAT_TRACE(printf("0x%08x -- VMLA(%.4lf[D%d]*%.4lf[D%d]+%.4lf[D%d])=%.4lf[D%d]\n", \
			   insAddress, op1, insRn, op2, insRm, accum, insRd, res, insRd););

	
	break;
      }

      
      case 1: { // VDIV
	res = op1 / op2;
	FLOAT_TRACE(printf("0x%08x -- VDIV(%.4lf[D%d]/%.4lf[D%d])=%.4lf[D%d]\n", \
			       insAddress, op1, insRn, op2, insRm, res, insRd););
	break;
      }

      case 2: { // VMUL
        res = op1 * op2;
	FLOAT_TRACE(printf("0x%08x -- VMUL(%.4lf[D%d]*%.4lf[D%d])=%.4lf[D%d]\n", \
			   insAddress, op1, insRn, op2, insRm, res, insRd););

	break;
      }

      case 3: { // VADD
	res = op1 + op2;
	FLOAT_TRACE(printf("0x%08x -- VADD(%.4lf[D%d]+%.4lf[D%d])=%.4lf[D%d]\n", \
			   insAddress, op1, insRn, op2, insRm, res, insRd););

	break;

      }

      case 4: { // VNMLA
	double product = op1 * op2;
	double d = *((double*)(&d_reg[insRd]));

	res = -d - product;

	break;
      }

      case 5: { // VNMLS
	double product = op1 * op2;
	double d = *((double*)(&d_reg[insRd]));

	res = product - d;

	break;
      }

      case 6: { // VMOV (register)
	res = op2;

	break;
      }

      case 7: { // VCMP{E}
	double val = *((double*)(&d_reg[insRd]));
	fpscr_val = (val == 0.0) ? 0x6 : (val > 0.0) ? 0x2 : 0x8;
	FLOAT_TRACE(printf("0x%08x -- VCMP(%.4lf[D%d], 0.0)\n",	\
			       insAddress, val, insRd););

	/* do not update destination register */
	return;
      }
	
      case 8: { // VSQRT
	res = sqrt(op2);
	FLOAT_TRACE(printf("0x%08x -- VSQRT(%.4lf[D%d])=%.4lf\n",\
			   insAddress, op2, insRm, res););
	
	break;
      }

      case 9: { // VCMP (comparsin with register)
	double val = *((double*)(&d_reg[insRd]));
	FLOAT_TRACE(printf("0x%08x -- VCMP(%.4lf[D%d], %.4lf[D%d])\n",
			   insAddress, val, insRd, op2, insRm););
	fpscr_val = (val == op2) ? 0x6 : (val > op2) ? 0x2 : 0x8;

	/* do not update destination register */
	return;
      }

      case 10: { // VABS
	res = abs(op2);
	FLOAT_TRACE(printf("0x%08x -- VABS(%.4lf[D%d])=%.4lf[D%d]\n",\
			   insAddress, op2, insRm, res, insRd););
	
	break;
      }

      case 11: { // VSUB
	res = op1 -op2;
	FLOAT_TRACE(printf("0x%08x -- VSUB(%.4lf[D%d]-%.4lf[D%d])=%.4lf[D%d]\n",\
			   insAddress, op1, insRn, op2, insRm, res, insRd););

	break;
      }

	
      case 12: { // NEG
	res = -op2;
	FLOAT_TRACE(printf("0x%08x -- VNEG(%.4lf[D%d])=%.4lf[D%d]\n",\
			   insAddress, op2, insRn, res, insRd););

	break;
      }

      case 13: { // MOV
	FLOAT_TRACE(printf("0x%08x -- D%d=MOV(0x%016lx(%.4lf))\n",\
			   insAddress, insRd, insFloatOperand1, *((double*)(&insFloatOperand1))););
	d_reg[insRd] = insFloatOperand1;
	return;
      }

      case 14: { // VNUM
	res = op1 * op2 * -1;
	FLOAT_TRACE(printf("0x%08x -- VNMUL(%.4lf[D%d]*%.4lf[D%d])=%.4lf[D%d]\n", \
			 insAddress, op1, insRn, op2, insRm, res, insRd););

	break;
      }

    
      default: {
	assert(0);
      }
	
    }

    d_reg[insRd] = *((uint64_t*)(&res));

  }
}

int armisa::exec_float_memory_instr() {
  /* check if it is a load */
  if(insL) {
    u32_t* data = (u32_t*)(&d_reg[0]);
    
    for(int i = 0; i < insMultRegisters; ++i) {
      int rc = armisa_read1(insDataAddress, insSizeCode, false, false);
      if(!rc)
	return 0;
      
      if(insFloatSingle) {
	FLOAT_TRACE(printf("0x%08x -- [S%d] = 0x%08x(%.4f)[MEM[0x%08x]]\n",\
			   insAddress, insRd+i, read_data, *((float*)(&read_data)), insDataAddress););
	data[insRd+i] = read_data;
	insDataAddress += 4;
      } else {
	FLOAT_TRACE(printf("0x%08x -- [D%d] = 0x%016lx(%.4lf)[MEM[0x%08x]]\n",\
			   insAddress, insRd+i, read_data64, *((double*)(&read_data64)), insDataAddress););

	d_reg[insRd+i] = read_data64;
	insDataAddress += 8;
      }
    }
  } else {
      u32_t* data = (u32_t*)(&d_reg[0]);
      
      for(int i = 0; i < insMultRegisters; ++i) {
	if(!insFloatSingle) {
	  u64_t wdata = d_reg[insRd + i];
	  FLOAT_TRACE(printf("0x%08x -- MEM[0x%08x] = 0x%016lx(%.4lf)[D%d]\n", \
			     insAddress, insDataAddress, wdata, *((double*)(&wdata)), insRd + i););
	  int success = armisa_write1(insDataAddress, wdata, insSizeCode, NULL);
	  if(!success)
	    return 0;
	
	  insDataAddress += 8;
	}
	else {
	  u32_t wdata = data[insRd + i];
	  FLOAT_TRACE(printf("0x%08x -- MEM[0x%08x] = 0x%08x(%.4f)[S%d]\n", \
			     insAddress, insDataAddress, wdata, *((float*)(&wdata)), insRd + i););
	  int success = armisa_write1(insDataAddress, wdata, insSizeCode, NULL);
	  if(!success)
	    return 0;
	
	  insDataAddress += 4;
	}
      }
  }

  return 1;
}

int armisa::exec_float_instr() {
    u32_t in = instruction;
    u4_t type = Bits4(in, 8);
    int regs;
    switch(type) {
      case 7:
        regs = 1;
        break;
      case 10:
        regs = 2;
        break;
      case 6:
        regs = 3;
        break;
      case 2:
        regs = 4;
        break;
      default:
        assert(0);
    }
    u32_t align = Bits(in, 4, 2);
    int alignment = align == 0 ? 1 : 4 << align;
    u32_t size = Bits(in, 6, 2);
    int ebytes = 1 << size;
    int esize = 8 * ebytes;
    int elements = (int)8 / ebytes;
    int D = Bit(in, 22);
    int Vd = Bits4(in, 12);
    int d = (D << 4) | Vd;
    int n = Bits4(in, 16);
    int m = Bits4(in, 0);
    bool wback = m != 15;
    bool register_index = (m != 15 && m != 13);
    
    u32_t address = Reg(n);
    add_use(m_uses_defs_idx, n);
    // need to generate alignment exception
    assert((address % alignment) == 0);
    
    if(Bit(in, 21)) {
        for(int r = 0; r < regs; r++) {
            for(int e = 0; e < elements; e++) {
                int rc = armisa_read1(address, size, false, false);
                if(!rc)
                    return 0;
                u8_t* data = (u8_t*)(&(d_reg[d+r]));
                data += e*ebytes;
                u8_t* from;
                if(size == 3) {
                    from = (u8_t*)(&read_data64);
                } else {
                    from = (u8_t*)(&read_data);
                }
                memcpy(data, from, ebytes);
                address += ebytes;                        
            }
        }                                        
    } else {
        for(int r = 0; r < regs; r++) {
            for(int e = 0; e < elements; e++) {
                u8_t* data = (u8_t*)(&(d_reg[d+r]));
                data += e*ebytes;
                u64_t wdata = 0;
                u8_t* pwdata = (u8_t*)(&wdata);
                memcpy(pwdata, data, ebytes);
                int success = armisa_write1(address, wdata, size, NULL);
                if(!success)
                    return 0;
                address += ebytes;                        
            }
        }
    }

    if(wback) {
        if(register_index)
            Reg(n) += Reg(m);
        else
            Reg(n) += 8*regs;
	add_def(m_uses_defs_idx,n);
    }
    
    return 1;
}

int armisa::exec_mem_operand_instr() 
{
  add_use(m_uses_defs_idx, insRm);
  add_use(m_uses_defs_idx, insRn);
  add_use(m_uses_defs_idx, insRs);

#if THREAD_COMM
  int bytes = 2 << (insSizeCode + 3);
  int lower = insDataAddress;
  int upper = insDataAddress + bytes;
  
  discrete_interval<uint64_t> range = discrete_interval<uint64_t>::right_open(lower, upper);
  if(insL) {
    int total_read = 0;
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
    
    int ll = 0;
    while(g_thread_comm && true) {
      addr_owners_t::const_iterator it = g_thread_comm_producers.find(range);
      if(it == g_thread_comm_producers.end())
	break;

      int new_lower = MAX(lower, (*it).first.lower());
      int new_upper = MIN(upper, (*it).first.upper());
      int read_bytes = (new_upper - new_lower);
      lower = new_upper;
      total_read += read_bytes;
      g_thread_comm_consumers[m_core_no][(*it).second-1] += read_bytes;
      
      if(lower == upper)
	break;
      
      range = discrete_interval<uint64_t>::right_open(lower, upper);
    }
    assert(total_read >= 0 && total_read <= bytes);
  } else {
    /* interval_map does not aggregate if value is 0 */
    g_thread_comm_producers += make_pair(range, m_core_no+1);
  }
#endif

  
  if(runstate != MidMultiple2S) {
    if(insL) {
      stats.m_Loads+=1;
    }
    else {
      stats.m_Stores+=1;
    }
  }
  
  if (insL) // Generic Loads
    {
      u32_t* rd1 = NULL;
      u32_t* rd2 = NULL;
      if(insRt != -1 && insRt2 != -1) {
      	assert(!insSUser);
	rd1 = &(Reg(insRt));
	rd2 = &(Reg(insRt2));
	insRt = insRt2 = -1;
      }	else {
	rd1 = insSUser ? &(raw_reg[insRd]) : &(Reg(insRd));
	rd2 = insSUser ? &(raw_reg[insRd+1]) : &(Reg(insRd+1));
      }
      bool linkedf = insEXCF;
      //printf ("Run %x load %s  ldrex sz='%s' insSizeCode=%i addr=%x\n", in, exm, size, insSizeCode, insDataAddress);
      int rc = armisa_read1(insDataAddress, insSizeCode, FALSE, linkedf); // not opcode - load linked - ldrex.

      u32_t oldvale = *rd1;
      if (rc)
	{
	  add_def(m_uses_defs_idx,insRd);
	  switch (insSizeCode)
	    {
	    case 0: 
	      *rd1 = (insSigned) ? sex8(read_data & 0xFF): (read_data & 0xFF);
	      break;
	      
	    case 1:
	      *rd1 = (insSigned) ? sex16(read_data & 0xFFFF): (read_data & 0xFFFF);
	      break;
	      
	    case 2:
	      *rd1 = read_data;
	      break;
	      
	    case 3: // Double word load
	      *rd1 = (read_data64 << 32llu) >> 32llu;
	      *rd2 = (read_data64 >> 32llu);
	      add_def(m_uses_defs_idx,insRd + 1);
	      break;
	      
	    default:
	      assert(0);
	    }

          if(insRd == 15) { 
	    // Load to PC
	    // For the predictor we need to differentiate between pops from the stack and single data loads to r15 which are a memory indirect branch (as used for dynamic-linking trampolines).
	    int pop_style = (insDecodeT == SingleT)  ? 100:-1;
	    predictor_op((oldvale>>2)-2, insCcodeTest, read_data >> 2, pop_style);
              if((*rd1 & 1) == 1) {
                  Reg(CpsrReg) |= (1 << 5);
                  *rd1 &= ~1;
              } else if(((*rd1 >> 1) & 1) == 0) {
                  Reg(CpsrReg) &= ~(1 << 5);
              }

#if 0	      
	      if (m_abtb) {
		m_abtb->install_alt(insDataAddress, *rd1);
	      }
#endif	      

              *rd1 += ARM_PC_CORRECTION;
          }
	}
      else {
          return 0;
      }
      //printf ("Ran %s %x  ldrex rc=%i sz='%s' addr=%x ans=%x\n", in, exm, rc, size, insDataAddress, read_data);
    }

  else // Generics/misc stores or strex.
    {
      if (insEXCF)
	{
          assert(!insSUser);
	  u32_t outcome = 0;
	  //printf ("Run strex insSizeCode=%i addr=%0x wd=%x\n", insSizeCode, insDataAddress, Reg(insRd));
	  insAluResult = Reg(insRm);
	  if(insSizeCode == 3) {   // double word
	    insAluResultHi = Reg(insRm+1);
	    add_use(m_uses_defs_idx, insRm+1);
	  }
	  else insAluResultHi = 0;
	  u64_t nv = (u64_t)(u64_t)insAluResult | (((u64_t)insAluResultHi) << 32llu);

	  int rc = armisa_write1(insDataAddress, nv, insSizeCode, &outcome); // new - breaks it --- WHY IS THIS LINE WRONG
          if(outcome == 2) { 
              return 0;
          }
          else if(!rc)
              return 0;
	  //printf ("Ran %x store %s  sz='%s' addr=%x outcome=%x (zero for success)\n", in, exm, size, insDataAddress, outcome);
	  add_def(m_uses_defs_idx,insRd);
	  Reg(insRd) = outcome ? 0:1; // Zero in Rm for success so invert polarity from busaccess module.
	  
	}
      else
	{
          insAluResult = insSUser ? raw_reg[insRd] : Reg(insRd);
	  add_use(m_uses_defs_idx, insRd);
	  if(insSizeCode == 3) {
	    //double word
	    //printf ("Run store insSizeCode=%i addr=%0x wd=%x\n", insSizeCode, insSizeCode, insDataAddress, Reg(insRd));
            assert(!insSUser);
	    assert(insRt != -1 && insRt2 != -1);
	    insAluResult = Reg(insRt);
	    insAluResultHi = Reg(insRt2);
	    add_use(m_uses_defs_idx, insRt2);
	    insRt = insRt2 = -1;
	  }
          else insAluResultHi = 0;

	  u64_t nv =(u64_t)insAluResult | (((u64_t)insAluResultHi) << 32llu);

	  // update entry in the ABTB
	  if (m_abtb) {
	    m_abtb->written_address_snoop(insDataAddress, insAluResult);
	  }

	  int rc = armisa_write1(insDataAddress, nv, insSizeCode, 0);
          if(!rc)
              return 0;
	}
      
    }
  
  // do not writeback if we are in the middle of
  // multiple load/store operation, we can only write
  // back after that operations is finished and we do
  // that in DecodeMultiple2
  if ((runstate != MidMultiple2S) && (insW == 1 || insP==0)) // W==1 (writeback), write address back into Rn.  Also if not pre then default.
    {
      if (!insP)
	{
	  // P==0 (post-index), add increment after memory operation
	  insDataAddress += insDataOffset;
	}
      if (insDataOffset != 0) {
	Reg(insRn) = insDataAddress;
	add_def(m_uses_defs_idx,insRn);
      }
    }

  return 1;
}

void armisa::exec_mul_operand_instr() { // multiplication
    add_use(m_uses_defs_idx, insRm);
    add_use(m_uses_defs_idx, insRn);
    add_use(m_uses_defs_idx, insRs);
  
    fmode_t mode;

    switch(insMultOp) {
      case MULT_Signed: {
          int32_t operand1 = insAluOperand1;
          int32_t operand2 = insAluOperand2;
          int32_t result = operand1 * operand2 + (int32_t)insMultAccum;
          
          insAluResult = result;
          Reg(insRd) = result;

	  add_def(m_uses_defs_idx,insRd);

          mode = FMODE_logic32;
          
          break;
      }

      case MULT_SignedLong: {
          int32_t operand1 = (int32_t)insAluOperand1;
          int32_t operand2 = (int32_t)insAluOperand2;
          int64_t result = 1LL * operand1 * operand2 + (int64_t)insMultAccum;

          insAluResult = (result << 32) >> 32;
          insAluResultHi = (result >> 32);

          Reg(insRdLo) = insAluResult;
          Reg(insRdHi) = insAluResultHi;

	  if(insMultAccum != 0)
	    add_use(m_uses_defs_idx, insRd);
	  add_def(m_uses_defs_idx,insRdLo);
	  add_def(m_uses_defs_idx,insRdHi);

          mode = FMODE_logic64;

          break;
      }

      case MULT_Unsigned: {
          insAluResult = insAluOperand1 * insAluOperand2 + (u32_t)(insMultAccum);
	  Reg(insRd) = insAluResult;

	  add_def(m_uses_defs_idx,insRd);

          mode = FMODE_logic32;

          break;
      }

      case MULT_UnsignedLong: {
          u64_t result;
          u64_t operand1 = insAluOperand1;
          u64_t operand2 = insAluOperand2;
          result = operand1 * operand2 + insMultAccum;

          insAluResult = (result << 32) >> 32;
          insAluResultHi = result >> 32;

          Reg(insRdLo) = insAluResult;
          Reg(insRdHi) = insAluResultHi;

	  if(insMultAccum != 0)
	    add_use(m_uses_defs_idx, insRd);
	  add_def(m_uses_defs_idx,insRdLo);
	  add_def(m_uses_defs_idx,insRdHi);

          mode = FMODE_logic64;
          break;
      }

      default: {
          assert(0); // unknown multiplication operation
      }
    }

    insShiftCarryOut = CFlag(); // keep old value;
    if (insS) updateFlags(mode);

}

bool armisa::DecodeArmInstruction(bool uncond, bool* executed)
{
  *executed = true;
  u32_t in = instruction;
  prefetchIns = 0;
  insDecodeT = UnknownT;
  insSUser = 0;

  //printf("Decode and run %x\n", in);
  if (m_abtb) m_abtb->ins32_monitor(in, Reg(12), Reg(PcReg));

  // Check condition codes    // (ref ARM7 4-5 pp 47)
  u4_t conder = (in >> 28);
  if (conder != 15) DecodeCcode(conder);
  else
    {
      // F instructions
        if(in == 0xFFFFFFFF) {
            // undefined instruction
            insDecodeT = UndefinedT;
            return true;
        }
        else if((in & 0xFB00000) == 0x4200000 ||
           (in & 0xFB00000) == 0x4000000) {
            insDecodeT = FloatT;
            return true;
        }
        else if((in & 0xE000000) == 0xA000000) {
            // BLX with mode change
            insDecodeT = BranchT;
            insOffset = Bits(in, 0, 24);
            if(Bit(in, 23)) insOffset |= 0xFF000000; // sign extend
            insOffset = (insOffset << 2) | (Bit(in, 24) << 1) | 0;
            u32_t dest = Reg(PcReg) + insOffset;
            insL = true;
	    u32_t rc = 0xFFFFffff;
	    // Check first wheter there is antry in ABTB
	    if (m_abtb) 
	      rc = m_abtb->query(dest, &Reg(CpsrReg));

	    if(rc != 0xFFFFffff)
	      rc = predictor_op((Reg(PcReg))>>2-2, insCcodeTest, dest>>2, 1);

	    if(rc != 0xFFFFffff)
	      dest = rc;

            ExecuteBL(dest);
            Reg(CpsrReg) |= (1 << 5);
            return true;
        }
        else if((in & 0x57ff01f) == 0x57ff01f) {
            // CLREX instruction, ignore for now
            return true;
        }
        else if((in & 0xFFFFF00) == 0x57FF000) {
	  // memory barriers 
	  // f57ff04e        dsb     st
	  // f57ff04f        dsb     sy
	    DecodeBarrier(Bits4(in, 4), Bits4(in, 0));
            return true;
        }

        else if((in & 0x510F000) == 0x510F000) {
            // PLD instruction, ignore for now
            return true;
        }
        
        else if ((in & 0xE80F000) == 0x480F000)
	{
	  //printf("Prefetch %x %x\n", Reg(PcReg), in);
	  prefetchIns = 1;
	}

      else if((in & 0xF000000) == 0x1000000) {
          // CPS instruction
          u2_t imod = Bits(in, 18, 2);
          bool enable = imod == 2;
          bool disable = imod == 3;
          bool changemode = !!Bit(in, 17);
          
          bool affectA = Bit(in, 8);
          bool affectI = Bit(in, 7);
          bool affectF = Bit(in, 6);
          
          u32_t cpsr_val = Reg(CpsrReg);

          //printf("old value of cpsr is 0x%08x, imod %d, enable %d, disable %d, changemode %d, affectA %d, affectI %d, affectF %d\n", 
          //     cpsr_val, imod, enable, disable, changemode, affectA, affectI, affectF);
          // we only update if we are not in the user mode
          if((cpsr_val & 0x1F) != 0x10) {

              if(enable) {
                  if(affectA) {
                      cpsr_val &= ~(1<<8); // set 8th bit to zero
                  }
                  if(affectI) {
                      cpsr_val &= ~(1<<7); // set 7th bit to zero
                  }
                  if(affectF) {
                      cpsr_val &= ~(1<<6); // set 6th bit to zero
                  }                  
              }

              if(disable) {
                  if(affectA) {
                      cpsr_val |= 1 << 8; // set 8th bit to one
                  }
                  if(affectI) {
                      cpsr_val |= 1 << 7; // set 7th bit to one
                  }
                  if(affectF) {
                      cpsr_val |= 1 << 6; // set 6th bit to one
                  }
              }

              if(changemode) {
                  u5_t new_mode = Bits(in, 0, 5);
                  cpsr_val = (cpsr_val & ~0x1F) | new_mode;
              }
          }
          //printf("CPS instruction[%d]: new value of cpsr is 0x%08x, pc = 0x%08x\n", m_core_no, cpsr_val, Reg(PcReg));
          Reg(CpsrReg) = cpsr_val;
          if(changemode)
              DecodeSetModeFromCpsr();
          
          return true;
      }
      
      else switch (in & 0xFFFFF00) 
	    {

	    default:
	      insCcodeTest = FALSE;                               break; // never
	      break;
	      // fall through - a never instruction
	    }
    }


  ARM32_INSTRUCTION_PTRACE(m_core_no, insCcodeTest, Reg(PcReg)-8, in); // ptrace_v1

    u3_t major_opcode = Bits(in, 25, 3);
    if (! insCcodeTest)
    {
      *executed = false;
        TRACE5("Condition codes fail, instruction not executed");
	if (major_opcode == 0x5) // Find not-taken branches and branch-and-links.
	  {
	    insOffset = Bits(in, 0, 24);
	    if (Bit(in, 23)) insOffset |= 0xFF000000;  // sign extend.
	    u32_t dest = Reg(PcReg) + (insOffset << 2);
	    insL        = Bit  (in, 20);
	    predictor_op((Reg(PcReg)>>2)-2, insCcodeTest, dest>>2, insL);
	  }
        return true;
    }


    // Decode to determine instruction type.
    //
    // (ref ARM7 4.1.1 fig 4-1 pp 44)
    // various bits which occur in several instruction groups
    insP        = Bit  (in, 24);
    insU        = Bit  (in, 23);
    insB        = Bit  (in, 22);
    insW        = Bit  (in, 21);
    insL        = Bit  (in, 20);
    insS        = Bit  (in, 20);
    insA        = Bit  (in, 21);
    insRn       = Bits4(in, 16);
    insRd       = Bits4(in, 12);

    insEXCF = false;

    switch (major_opcode) // top 3 bits below condition code
    {

    case 0: 

      if ((in & 0xF900010) == 0x1000000)
	{
	  if (Bits(in, 23, 5) == 2) 
            {
	      if(Bit(in, 24) && Bit(in, 22) && Bit(in, 21) && Bit(in, 7)) {
		// this is smulXX instruction
		bool n_high = !!Bit(in, 5);
		bool m_high = !!Bit(in, 6);
		
		insRd = Bits4(in, 16);
		insRm = Bits4(in, 8);
		insRn = Bits4(in, 0);

		int32_t operand1 = Reg(insRn);
		insAluOperand1 = n_high ? (operand1 >> 16) : (operand1 & 0xFFFF);
		int32_t operand2 = Reg(insRm);
		insAluOperand2 = m_high ? (operand2 >> 16) : (operand2 & 0xFFFF);
                insMultAccum = 0;
                insS = false;
                insMultOp = MULT_Signed;
                insDecodeT = MulT;

                return true;
	      }
	      else {
		// this is MSR/MRS instruction
		if (!Bit(in, 21)) 
		  { // mrs: psr to register (bit 22 set for SPSR)
		    DecodeMrs();
		  }
		else
		  {
		    // msr: register copied to psr - with mask
		    int mask = Bits4(in, 16); // mask fields: { f s x c }
		    DecodeMsr(mask);
		  }
	      }
	      break;
	    }
	  
	  else assert(FALSE); // misc - DSP arith TODO
	}

      else if (((in & 0xE000090) == 0x0000090) || // misc 3b class 
	       ((in & 0xF900090) == 0x1000010))   // misc 3c class 
      {
	if ((in & 0x0F0000F0) == 0x90) // multiply (+accumulate)
	  {
	    insS = Bit(in, 20);
	    bool A = Bit(in, 21);
	    insRm = Bits4(in, 0);
	    insRs = Bits4(in, 8);
	    insRd = Bits4(in, 16);
            insRn = Bits4(in, 12);
            insMultAccum = 0;
                  	    
            if(Bit(in,23) && Bit(in,22) && Bit(in,21)) {
                // SMLAL
                insDecodeT = MulT;
                insAluOperand1 = Reg(insRm);
                insAluOperand2 = Reg(insRs);
                insMultAccum = (((int64_t)Reg(insRd)) << 32) | ((int64_t)Reg(insRn));

                insRdHi = insRd;
                insRdLo = insRn;
                insMultOp = MULT_SignedLong;
                
            }
            
            else if (!Bit(in, 23))
	      {
                  insDecodeT = MulT;
                  insMultOp = MULT_Unsigned;
                  insAluOperand1 = Reg(insRm);
                  insAluOperand2 = Reg(insRs);
                  if (A) {
                      insMultAccum = Reg(insRn); 
                      
                      if(Bit(in, 22) != 0) {
                          insAluOperand1 *= -1;
                      }
		  } else {
		    insRn = UndefinedReg;
		  }
	      } 
            else if(Bit(in, 23) && Bit(in, 22) && !Bit(in, 21)) {
                // SMULL
                insDecodeT = MulT;
                insMultOp = MULT_SignedLong;
                insAluOperand1 = Reg(insRm);
                insAluOperand2 = Reg(insRs);
                insRdHi = insRd;
                insRdLo = insRn;
            }
	    else 
	      {
                  insDecodeT = MulT;
		  // MULL, MLAL - multiply long
                  insMultOp = MULT_UnsignedLong;
                  insAluOperand1 = Reg(insRm);
                  insAluOperand2 = Reg(insRs);
                  if (A) {
                      insMultAccum = (((u64_t)Reg(insRd)) << 32) | Reg(insRn);
		  } 

                  insRdHi = insRd;
                  insRdLo = insRn;
		  if (!A) {
		    insRn = UndefinedReg;
		  }
	      }
	  }
	else if (Bits4(in, 24) == 1 && ((in & 0x90) == 0x90)
		 // Misc loads and stores of doubles and halfwords and signed halfwords.

		 || Bits4(in, 24) == 0 && ((in & 0x90) == 0x90) // TODO check this - the ldrd bug.
		 // needed for ins=e0c3a0d8  X C`  e0c3 a0d8  ldrd r10,[r3]
		 )
	  {
            // This code should only be hit by synchronisation instructions
	    //printf("spag\n");
	    // ? insP in bit24
	    insU = Bit(in, 23);
	    insW = Bit(in, 21);
	    insDataOffset = 0;
	    insEXCF = ((in & 0xF00) == 0xF00);
	    const char *exm = (insEXCF) ? "ex":"";
	    //bool L = Bit(in, 20), H = Bit(in, 5), S = Bit(in, 6);
	    const char *size = "";
	    insSigned = false;
	    insL = false;
	    insSizeCode = 2; // Default is size 2 (int32).
	    // Further atomic decode exceptions.
	    if (insEXCF && Bits(in, 21, 3) == 7)
	      {
		if (Bit(in, 20)) insL = true;
		if (Bit(in, 22)) insDataOffset = (Bits4(in, 8) << 4) | (Bits4(in, 0) << 0);
		size = "h"; insSizeCode = 1;
	      }
	    else if (insEXCF && Bits(in, 21, 3) == 6)
	      {
		if (Bit(in, 20)) insL = true;
		size = "b";  insSizeCode = 0;
	      }
	    else if (insEXCF && Bits(in, 21, 3) == 5)
	      {
		if (Bit(in, 20)) insL = true;
		size = "d";  insSizeCode = 3;
	      }
	    
	    else  
	      switch (in & 0x100060) // indexed and reg indirect (insDataOffset=0) forms
		{
		  
		case 0x20: //	L=0, S=0, H=1 Store halfword.
		  if (Bit(in, 22)) 
		    insDataOffset = (Bits4(in, 8) << 4) | (Bits4(in, 0) << 0);
		  else {
		    insDataOffset = Reg(Bits4(in, 0));
		    insRs = Bits4(in, 0);
		  }
		  size = "h"; insSizeCode = 1;
		  break;
		  
		case 0x40:  // L=0, S=1, H=0 Load doubleword.
		  if (Bit(in, 22)) 
		    insDataOffset = (Bits4(in, 8) << 4) | (Bits4(in, 0) << 0);
		  else {
		    insDataOffset = Reg(Bits4(in, 0));
		    insRs = Bits4(in, 0);
		  }
		  insL = true; size = "d";
		  insSizeCode = 3;
		  break;
		  
		case 0x60:  // L=0, S=1, H=1 Store doubleword.
		  if (Bit(in, 22)) 
		    insDataOffset = (Bits4(in, 8) << 4) | (Bits4(in, 0) << 0);
		  else {
		    insDataOffset = Reg(Bits4(in, 0));
		    insRs = Bits4(in, 0);
		  }
		  insSizeCode = 3;
		  size = "d";
		  break;
		  
		case 0x100020:  // L=1, S=0, H=1 Load unsigned halfword.
		  if (Bit(in, 22)) 
		    insDataOffset = (Bits4(in, 8) << 4) | (Bits4(in, 0) << 0);
		  else {
		    insDataOffset = Reg(Bits4(in, 0));		    
		    insRs = Bits4(in, 0);
		  }
		  insL = true; size = "h"; insSizeCode = 1;
		  break;
		      
		case 0x100040:  // L=1, S=1, H=0 Load signed byte.
		  if (Bit(in, 22)) 
		    insDataOffset = (Bits4(in, 8) << 4) | (Bits4(in, 0) << 0);
		  else {
		    insDataOffset = Reg(Bits(in, 0, 4));
		    insRs = Bits(in, 0, 4);
		  }
		  insL = true; insSigned = true; size = "sb";  insSizeCode = 0;
		  break;
		  
		case 0x100060:  // L=1, S=1, H=1 Load signed halfword
		  if (Bit(in, 22)) 
		    insDataOffset = (Bits4(in, 8) << 4) | (Bits4(in, 0) << 0);
		  else {
		    insDataOffset = Reg(Bits4(in, 0));
		    insRs = Bits4(in, 0);
		  }
		  insL = true; insSigned = true; size = "sh"; insSizeCode = 1;
		  break;
		  
		default:
		  if (insEXCF) // not here... see above clause ? 
		    {
		      if (Bit(in, 20)) insL = true;
		      size = ""; insSizeCode = 2;
		      break;
		    }
		  printf("Bad instruction %x - extra group 3c", in);
		  assert(0);
		}
	    insDataAddress = Reg(Bits(in, 16, 4));
	    if (!insU) insDataOffset  = -insDataOffset;
	    if (insP) insDataAddress += insDataOffset;
	    insRd = Bits4(in, 12);
	    if (insEXCF) insRm = Bits4(in, 0);
            insDecodeT = SynchronisationT;
            break;

	  }

	else if ((in & 0xFFFFFD0) == 0x12fff10)
	  {
	    //
	    // BX, Branch with mode switch to/from Thumb mode (and link)
	    //
	    insL = Bit(in, 5);
	    insDecodeT = BxT;
	    insRn   = Bits4(in, 0);
            Reg(CpsrReg) = (Reg(CpsrReg) & ~(1 << 5)) | ((Reg(insRn) & 1) << 5);
	    u32_t dest = (Reg(insRn) & ~1);
	    // reg indirect jump - consider a pop if the dest is the link register. if a 'bl lr' then also a push but we ignore that for now since gcc does not make these.
	    insOffset = 0;

	    u32_t rc = 0xFFFFffff;
	    if (m_abtb) 
	      rc = m_abtb->query(dest, &Reg(CpsrReg));

	    if(rc != 0xFFFFffff)
	      rc = predictor_op((Reg(PcReg)>>2)-2, insCcodeTest, dest>>2, insRn=LrReg ? -1 : insL);

	    if(rc != 0xFFFFffff)
	      dest = rc;

	    ExecuteBL(dest);
	    break;
	  }
	else if ((in & 0xFFF0FF0) == 0x16f0f10) // count leading zeros : clz
	  {
	    insRd       = Bits4(in, 12); // already setup
	    insRm       = Bits4(in, 0);
	    u32_t r = 0, a = Reg(insRm);
	    for (r=0; r<32; r++)
	      {
		if (a & (1<<31)) break;
		a <<= 1;
	      }
	    TRACE2("clz insRd " _ insRd _ " Rm=" _ insRm _ " answer=" _ r);
	    insRn = UndefinedReg;
	    Reg(insRd) = r;
	    insDecodeT = ARM32ExecutedT;
	    stats.m_ALU+=1;
	  }

	// STRH(T)/LDRH instructions
	else if ((in & 0xB0) == 0xB0) {
          runstate = MidHalfwordS;
	}
	else if ((in & 0xB0) == 0x90) {
	  //ldrd bug - please fix
	  printf("ldrd %x\n", in);
	  NOTDONE;
	}
	else
	  {
	    //perhaps swap instruction missing?.
	    printf("ARM isa bad 3bc special %X ins=%X\n", Reg(PcReg), in);
	    tenos_diagnostics.checkpoint(2, myname(), "swap missing?");
	    FATAL("bad 3special");
	    //NOTDONE;
	  }
	break;

      }


    // remaining case 0 instructions. // ALU operations with I=0, shifted operand - fall through to case 1 for processing?
    case 1: // ALU operations with I=1, immediate operand
        {
	  if ((in & 0xFb00000) == 0x3200000) // immediate move to msr
	    {
	      // detect if instruction is WFE, WFI, SEV, NOOP
	      if(Bits(in, 16, 4) == 0 && 
		 Bits(in, 12, 4) == 0xF && 
		 Bits(in, 8, 4) == 0 && 
		 Bits(in, 4, 4) == 0) {
		
		u4_t op = Bits(in, 0, 4);
		switch(op) {
                  case 0:
                    break; // noop - do nothing

		case 1: // yield instruction
		  iss_yield(-1 );
		  break;

                  case 2: {
                      if(event_register)
                          event_register = false;
                      else {
                          //printf("Core %d has executed WFE instruction 0x%08x\n", m_core_no, insAddress);
                          last_event = inter_core_event->load();
                          wait_for_event = true;
                      }
                      break;
                  }
                  case 3: {
                      //printf("Core %d is waiting for interrupt --> %08x\n", m_core_no, insAddress);
                      wait_for_interrupt = true;
                      break; // WFI - not implemented for now
                  }
                  case 4: {
                      //cout << "Core " << m_core_no << " has executed SEV instruction " << insAddress << " @ " << sc_time_stamp() << endl;
                      u32_t value = inter_core_event->load();
                      bool success = false;
                      do {
                          u32_t new_value = (((value >> LOG_MAX_NO_OF_CORES) + 1) << LOG_MAX_NO_OF_CORES) | m_core_no;
                          success = inter_core_event->compare_exchange_strong(value, new_value);
                      } while(!success);
                      
                      break;
                  }
                    
                  default:
                    printf("Unknown instruction at address 0%08x -- %08x, minor_op=0x%x\n", insAddress, in, op);
                    assert(0); // unknown instructions
		}
	      }
	      else {
	        int mask = Bits(in, 16, 4); 
		if(!mask)
		  break;
		DecodeMsr(mask);
		break;
	      }
	    }

          if ((in & 0xFF00000) == 0x3400000) {
              // MOVT instruction
              u4_t imm4 = Bits(in, 16, 4);
              u12_t imm12 = Bits(in, 0, 12);
              u4_t Rd = Bits4(in, 12);
	      insRd = Rd;
              u16_t top = (((u16_t)imm4) << 12) | imm12;
              
              u32_t val = Reg(Rd);
              // clear the top part
              val &= 0x0000FFFF;
              // set the top part
              val |= (((u32_t)top) << 16);
              
              //printf("MOVT instruction; old value %08x, new value %08x, top %04x -- %08x, %08x\n", Reg(Rd), val, top, in, insAddress);
              Reg(Rd) = val;
	      OPERAND_PTRACE(val);
	      
              break;
          }

	  if((in & 0xFEF0FF0) == 0x1A00060) {
	    insRd = Bits4(in, 12);
	    u32_t rm = Reg(Bits4(in, 0));
	    int carry = rm & 0x1;
	    u1_t cc = CFlag();
	    insAluResult = (cc << 31) | (rm >> 1);
	    Reg(insRd) = insAluResult;
	    assert(insRd != 15);
	    if(Bit(in, 20)) { // set flags
	      updateFlags(FMODE_arith32, carry, VFlag());
	    }
	    insDecodeT = ARM32ExecutedT;
	    stats.m_ALU+=1;
	    break;
	  }


	  if (Bits(in, 25, 3) == 0
            && Bit(in, 22) == 1
            && Bit(in, 7) == 1
            && Bit(in, 4) == 1)
            {
                //
                // LDR/STR for halfword or sign-extended byte
                // (ref ARM7 4.10 pp 76)
                //
	      runstate = MidHalfwordS;
	      break;
            }
            else if (Bits(in, 23, 5) == 2
            && Bits(in, 20, 2) == 0
            && Bits(in, 4, 8) == 9)
            {
                // SWP, single data swap
                // (ref ARM7 4.12 pp 89)
                NOTDONE;
                break;
            }
    
            //
            // Main ALU decode table - (ref ARM7 4.5 pp 52)
            //
            insDecodeT = AluT;
            insShiftCarryOut = 0;

            bool shiftComputed = FALSE;

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            //
            // Determine insAluOperand2
            //
            if (Bit(in, 25))
            {
                //
                // I=1, operand 2 is an immediate operand
                //
                // (ref ARM7 4.5.3 pp 57)
                //
                u32_t imm = Bits(in, 0, 8);
                u32_t rotate = Bits4(in, 8);

                // Rotate right by twice the imm field.

                rotate *= 2;

                insAluOperand2 = Ror(imm, rotate);
            }
            else
            {
                //
                // I=0, operand 2 is a shifted register
                //
                // (ref ARM7 4.5.2 pp 54)
                //
                insRm = Bits4(in, 0);

                insShiftType = Bits(in, 5, 2);
                insShiftComputed = Bit(in, 4);
                if(insShiftComputed) {
                    assert(0 == Bit(in, 7)); // otherwise it's a multiply TODO - missing ?
                    // shift by register content
                    insShiftValue = Reg(Bits4(in, 8));
		    insRs = Bits4(in, 8);
                } else {
                    // shift by constant amount
                    insShiftValue = Bits(in, 7, 5);
                }

                exec_shift_operand_instr();
            }

            insALUOpcode = Bits4(in, 21);

            insAluOperand1 = Reg(insRn);

            if (shiftComputed && insRn == 15)
            {
                //
                // PC operand and computed shift, PC now PC+12
                // (ref ARM7 4.5.5 pp 58)
                //
                insAluOperand1 += 4;
            }


        }
        break;

    // ----------------------------------------------------------------------
    case 3:
        {
            if((in & 0xFF00FF0) == 0x6600FF0) {
                // UQSUB8
                insRd = Bits4(in, 12);
                insRm = Bits4(in, 0);
                insRn = Bits4(in, 16);

                u32_t diff1 = ((Reg(insRn) & 0xFF) > (Reg(insRm) & 0xFF)) ? ((Reg(insRn) & 0xFF) - (Reg(insRm) & 0xFF)) : 0;
                u32_t diff2 = ((Reg(insRn) >> 8) & 0xFF) > ((Reg(insRm) >> 8) & 0xFF) ? ((Reg(insRn) >> 8) & 0xFF) - ((Reg(insRm) >> 8) & 0xFF) : 0;
                u32_t diff3 = ((Reg(insRn) >> 16) & 0xFF) > ((Reg(insRm) >> 16) & 0xFF) ? ((Reg(insRn) >> 16) & 0xFF) - ((Reg(insRm) >> 16) & 0xFF) : 0;
                u32_t diff4 = ((Reg(insRn) >> 24) & 0xFF) > ((Reg(insRm) >> 24) & 0xFF) ? ((Reg(insRn) >> 24) & 0xFF) - ((Reg(insRm) >> 24) & 0xFF) : 0;
                
                Reg(insRd) =
                    (diff1) |
                    (diff2 << 8) |
                    (diff3 << 16) |
                    (diff4 << 24);

                return true;
            }
            
        else if ((in & 0xE000010) == 0x6000010) // sign extends and ubfx/sbfx - bit field extracts.
	    {
	      insRd = Bits4(in, 12);
	      insRn = Bits4(in, 0);
	      insSigned= !Bit(in, 22);
	      if ((in &0xF0) == 0x70)// sign extends and extracts
		{
		  u2_t ror = Bits(in, 10, 2);
		  bool hw = Bit(in, 20);	    
		  u32_t val = Reg(insRn) >> (8 * ror);
		  u4_t Radd = Bits4(in, 16);
		  if (!insSigned) val = val & (hw ? 0xFFff: 0xff);
		  else val = (hw) ? (((signed)val)<<16)>>16: (((signed)val)<<24)>>24;
		  if (Radd != 15) {
		    val += Reg(Radd); // With ADD of a base register
		    insRs = Radd;
		  }
		  Reg(insRd) = val;
		  //const char *n0 = (!insSigned) ? "u" : "s";
		  //const char *n1 = (hw) ? "b" : "h";
		  //writef6("%sxt%s%s %s, %s, ROR #%i",  n0, n1, maskname(in), registername(insRd), registername(insRn), ror * 8);
		}
	      else if ((in & 0x70) == 0x30)// byte reverse etc
		{
		  insRd = Bits4(in, 12);
		  insRn = Bits4(in, 0);
 		  /*
		4:   e6bf 1f32  00 b f3 rev     r1, r2
		8:   e6bf 3fb4  01 b fb rev16   r3, r4
		0:   e6ff 5f35  10 f f3 rbit    r5, r5
		34   e6ff 3fb4  11 f fb revsh   r3, r4
		c:   e6bf 1f32        rev     r1, r2
		0:   e6bf 3fb4        rev16   r3, r4
    REV        Reverse byte order in a word.
    REV16      Reverse byte order in each halfword independently.
    RBIT       Reverse the bit order in a 32-bit word.
    REVSH      Reverse byte order in the bottom halfword, and sign extend to 32 bits.

*/
		  int code = 2 * Bit(in, 22) + Bit(in, 7);
		  u32_t d = Reg(insRn);
		  switch(code)
		    {
		    case 0:  //Reverse byte order in a word.
		      d = ((d>>24) & 0xFF) | ((d>>8) & 0xFF00) | ((d<<8) & 0xFF0000) | ((d<<24) & 0xFF000000);
		      break;
		    case 1: // Reverse byte order in each halfword independently.
		      d = ((d>>8) & 0x00FF00FF) | ((d<<8) & 0xFF00FF00); 
		      break;

		    case 2://    RBIT       Reverse the bit order in a 32-bit word.
		      d = ((d>>16) & 0x0000FFFF) | ((d<<16) & 0xFFFF0000); 
		      d = ((d>>8) & 0x00FF00FF)  | ((d<<8)  & 0xFF00FF00); 
		      d = ((d>>4) & 0x0F0F0F0F0) | ((d<<4)  & 0xF0F0F0F0);
		      d = ((d>>2) & 0x333333333) | ((d<<2)  & 0xCCCCCCCC);
		      d = ((d>>1) & 0x555555555) | ((d<<1)  & 0xAAAAAAAA);
		      break;
		    case 3: //REVSH      Reverse byte order in the bottom halfword, and sign extend to 32 bits.
		      d = ((d >> 8) & 0xFF) | ((d<<8) & 0xFF00);
		      if (d & 0x8000) d |= 0xFFFF0000;
		      break;
		    }
		  TRACE4(printf("Byte reversed code=%i r%i <- r%i (ans=%x)\n", code, insRd, insRn, d));
		  Reg(insRd) = d;
		  OPERAND_PTRACE(d);
		}

              else if((in & 0x70) == 0x10 && !Bit(in, 21)) {
                  // BFI instruction
                  u4_t Rn = Bits(in, 0, 4);
                  u4_t Rd = Bits(in, 12, 4);
                  u5_t lsb = Bits(in, 7, 5);
                  u5_t msb = Bits(in, 16, 5);
                  u5_t width = msb-lsb+1;

                  u32_t new_value = ((Reg(Rn) << (32-width)) >> (32-width)) << lsb;
                  u32_t old_value = Reg(Rd);
                  //printf("BFI instruction(%08x): Reg(Rn)=%08x, Reg(Rd)=%08x, lsb=%d, msb=%d\n", insAddress, Reg(Rn), Reg(Rd), lsb, msb);
                  old_value &= ~(((1 << width) - 1) << lsb);

                  Reg(Rd) = old_value | new_value;
		  OPERAND_PTRACE(Reg(Rd));
		  insRd = Rd;
		  insRn = Rn;
                  //printf("BFI instruction(%08x): new Reg(Rd)=%08x\n", insAddress, Reg(Rd));
              }

	      else
		{
		  int base = Bits(in,  7, 5);
		  int width = Bits(in, 16, 5) + 1;  
		  //const char *nem = (unsignedf) ? "ubfx" : "sbfx";
		  if (!insSigned)
		    {
		      u32_t vale = Reg(insRn);
		      vale <<= (32-base-width);
		      vale >>= 32-width;
		      Reg(insRd) = vale;
		    }
		  else
		    {
		      int vale = Reg(insRn);
		      vale <<= (32-base-width);
		      vale >>= 32-width;
		      Reg(insRd) = vale;
		    }
		  OPERAND_PTRACE(Reg(insRd));
		}
	      return true;
	    }
          else if((in & 0xE000A10) == 0xE000A10) {
              
              printf("VMOV instruction @ %08x -- %08x\n", insAddress, in);
              assert(0);
          }
    
	  else if (1 == Bit(in, 4))
            {
                //
                // Undefined instruction
                // (ref ARM7 4.17 pp 102)
                //
                // This is mapped on top of STR with index, with another register
                // content determining shift distance of index register.
                //
                DecodeTrap(SupervisorMode, UndefinedVector);
                break;
            }
        }
        // fall through
    case 2:
        //
        // LDR and STR - single register transfers
        //
        {
            // condition code already handled
            // insRn already extracted
            // insRd already extracted

            // Other fields not yet checked.
            DecodeSingle(); 
            runstate = MidSingleS;
        }
        break;

    // ----------------------------------------------------------------------
    case 4:
        // LDM and STM - multiple register transfers
        // (ref ARM7 4.11 pp 82)
        {
            insDecodeT  = BlockT;
            insS        = Bit(in, 22); // different from default fields
            insMultRegisters = Bits(in, 0, 16);
            // all other fields are already extracted
            runstate = MidMultipleS;
            // The story continues when the current instruction prefetch returns - but this just wastes a cycle
	    // in this model. TODO.
        }
        break;

    // ----------------------------------------------------------------------
    case 5: // B and BL
      {
	insDecodeT  = BranchT;
	insL        = Bit(in, 24); // different from default above
	insOffset   = Bits(in, 0, 24);
	if (Bit(in, 23)) insOffset |= 0xFF000000;  // sign extend.
	u32_t dest = Reg(PcReg) + (insOffset << 2);

        // Linux machine halt
        if((m_core_no == 0 && insOffset == -2) ||
           (m_core_no > 0 && last_shutdown_sequence && insOffset == -3))
            shutdown_sequence = true;

	u32_t rc = 0xFFFFffff;
	if (m_abtb) 
	  rc = m_abtb->query(dest, &Reg(CpsrReg));
	
	if(rc != 0xFFFFffff)
	  rc = predictor_op((Reg(PcReg)>>2)-2, insCcodeTest, dest>>2, insL);

	if(rc != 0xFFFFffff)
	  dest = rc;
	    
	OPERAND_PTRACE(dest);
	ExecuteBL(dest);
      }
      break;

    // ----------------------------------------------------------------------
    case 6: // coprocessor data transfer
        {
            if((in & 0xF300F00) == 0xD100B00 ||
               (in & 0xF300F00) == 0xD000B00) {
	        // Floating point operations
                u1_t U = Bit(in, 23);
                u1_t D = Bit(in, 22);
                u4_t Rn = Bits4(in, 16);
                u4_t Vd = Bits4(in, 12);
                u8_t imm8 = Bits(in, 0, 8);
                
                u32_t imm32 = (u32_t)imm8 << 2;
                bool add = (U == 1);
                int d = ((u32_t)D << 4) | Vd;
                int n = Rn;

                assert(n != 15);
                u32_t base = Reg(n);
                u32_t address = add ? base + imm32 : base - imm32; 
                if(Bit(in, 20)) {
                    bool success = armisa_read1(address, 2, false, false);
                    assert(success);
                    u32_t word1 = read_data;
                    success = armisa_read1(address+4, 2, false, false);
                    assert(success);
                    u32_t word2 = read_data;
                    d_reg[d] = ((u64_t)word2 << 32) | word1;
                }
                else {
                    armisa_write1(address, (u32_t)(d_reg[d] & 0xFFFFFFFF), 2, NULL);
                    armisa_write1(address+4, (u32_t)((d_reg[d] >> 32) & 0xFFFFFFFF), 2, NULL);
                }
            } else if(((in & 0xE100F00) == 0xC000B00) ||
               ((in & 0xE100F00) == 0xC100B00)) {
                u1_t P = Bit(in, 24);
                u1_t U = Bit(in, 23);
                u1_t D = Bit(in, 22);
                u1_t W = Bit(in, 21);
                u4_t Rn = Bits4(in, 16);
                u4_t Vd = Bits4(in, 12);
                u32_t imm8 = Bits(in, 0, 8);

                u32_t imm32 = imm8 << 2;
                u32_t regs = (u32_t)(imm8 / 2);
                
                bool add = (U == 1);
                bool wback = (W == 1);
                
                int d = ((u5_t)D << 4) | Vd;
                
                u32_t address;
                if(add) {
                    address = Reg(Rn);
                } else {
                    address = Reg(Rn) - imm32;
                }
                if(wback) {
                    if(add) {
                        Reg(Rn) = Reg(Rn) + imm32;
                    } else {
                        Reg(Rn) = Reg(Rn) - imm32; 
                    }
                }

                if(Bit(in, 20)) {
                    // VLDM instruction
                    for(int r = 0; r < regs; r++) {
                        assert(d+r < 16);
                        bool success = armisa_read1(address, 2, false, false);
                        assert(success);
                        u32_t bottom = read_data;
                        success = armisa_read1(address+4, 2, false, false);
                        assert(success);
                        u64_t top = read_data;
                        d_reg[d+r] = (top << 32) | bottom;
                        address+=8;
                    }
                } else {
                    // VSTM instruction
                    for(int r = 0; r < regs; r++) {
                        assert(d+r < 16);
                        armisa_write1(address, (u32_t)(d_reg[d+r] & 0xFFFFFFFF), 2, NULL);
                        armisa_write1(address+4, (u32_t)((d_reg[d+r] >> 32) & 0xFFFFFFFF), 2, NULL);
                        address+=8;
                    }
                 }
            } else {
                u1_t c = DecodeCoprocessor_DT();
                FATAL("coprocessor data transfer not implemented yet");
                // not implemented yet
                // return c;
            }
        }
        break;

    // ----------------------------------------------------------------------
    case 7:
        {
            // TODO: For now we only have VMRS/VMSR instructions
            // implemented and hard coded, this looks to be sufficient
            // for booting the kernel but at one point floating point
            // unit should be implemented that will run floating point
            // instructions
            if((in & 0xE00A10) == 0xE00A10) {
                bool read = !!Bit(in,20);
                u4_t reg = Bits(in, 16, 4);
                u4_t dest = Bits(in, 12, 4);

                if(read && reg == 0) {
                    Reg(dest) = 0x41033094; // FPSID
                } else if(read && reg == 6) {
                    Reg(dest) = 0x10011100; // MVFR1                
                } else if(read && reg == 7) {
                    Reg(dest) = 0x00000222; // MVFR0
                } else if(read && reg == 8) {
                    Reg(dest) = 0x0; 
                } else if(!read && reg == 8) {
                    assert(Reg(dest) == 0);
                    // nooop for now
                } else {
                    printf("VMRS at instruction %08x, read=%d, reg=%d\n", insAddress, read, reg);
                    assert(0);
                }
            } else if((in & 0xBF0ED0) == 0xB00A40) {
                u32_t D = Bit(in, 22);
                int Vd = Bits4(in, 12);
                u1_t sz = Bit(in, 8);
                u1_t M = Bit(in, 5);
                u4_t Vm = Bits4(in, 0);
                bool single_register = (sz == 0);
                bool advsimd = false;
                int d, m;
                int regs = 0;
                if(single_register) {
                    d = (Vd << 1) | D;
                    m = (Vm << 1) | M;
                } else {
                    d = (D << 4) | Vd;
                    m = (M << 4) | Vm;
                    regs = 1;
                }
                    
                if(single_register) {
                    u32_t* dest = (u32_t*)(&d_reg[0]);
                    u32_t* src = (u32_t*)(&d_reg[0]);
                    dest += d;
                    src += m;
                    *dest = *src;
                } else {
                    for(int r = 0; r < regs; r++) {
                        d_reg[d+r] = d_reg[m+r];
                    }
                }
            } else {
                if (Bit(in, 24) == 0)
                {
                    // more coprocessor ops
                    if (Bit(in, 4)) DecodeCoprocessor_RT();
                    else DecodeCoprocessor_DO();
                }
                else // SWI instruction
                {
                    insDecodeT = SwiT;
                    insSWIArg = Bits(in, 0, 24);
                }
            }
        }
        break;

    default:
        assert(FALSE);
    }

    return true;
}


void armisa::DecodeBarrier(u4_t type, u4_t mask) {
    // Linux machine halt
    if(type == 0x5 && m_core_no > 0)
        shutdown_sequence = true;

    // NOOP for now
}


// --------------------------------------------------------------------------
void armisa::ExecuteBL(u32_t dest)
{
  stats.m_Branches+=1;
  
#if 0
  if (insOffset== -2UL)
    {
      if (logfd) fprintf(logfd, "Branch to self: stopping");
      PrintStats(); 
      arm_sim_command(TENOS_CMD_SIM_DONE, 0, 0, 0); 
      // A bit heavyweight - better to have a lightweight command on cores other than core0 in case they stop while useful work exists elsewhere. Also a spin waiting for interrupt might set this off!
    }
#endif
  if (insL)
    {
      // BL - save link register, minus pipeline delay.
      //
      if(!thumb_mode)
          Reg(LrReg) = Reg(PcReg) - ARM_PC_CORRECTION;
      else {
          Reg(LrReg) = ((Reg(PcReg) - 2) & ~1) | 1;
      }
              
      u32_t m = user_trace(0, Reg(LrReg));
      //printf("%s site %i - debugmask=%x\n", __FILE__, __LINE__, m);
      if (m & TENOS_TRACE_SUBROUTINE_CALL) 
	{
         if (0 && argv_backdoor_su)
	    {	      
	      char linef[132], linet[132];
	      argv_backdoor_su->symbolic_dis64(linef, 132, Reg(LrReg)-4);
	      argv_backdoor_su->symbolic_dis64(linet, 132, dest);
              printf("%s: %s Branch and link to %s (%x, %x, ...)\n", myname(), linef, linet, Reg(0), Reg(1));
	    }
	  else {
	    //printf("%s: %x Branch and link to %x\n", myname(), Reg(LrReg)-4, dest);
          }
        }
      
    }
  TRACE1(if (logfd) fprintf(logfd, "Branch %s to location 0x%0x\n", (insL ? "and link " : ""), dest));
  Reg(15) = dest + ARM_PC_CORRECTION;
  // pipelineBreak = TRUE;
}

  
// --------------------------------------------------------------------------
// Various comms with simulator (backdoors) that would not exist in the real hardware.
//
void armisa::arm_swi_backdoor(int code) // SVC or TRAP backdoor. 
{
  bool vd = false;
  if (vd) printf("swi backdoor %i\n", code);
  switch (code)
    {
    case 1:
      {
	arm_sim_command(TENOS_CMD_SIM_DONE, Reg(0), 0, 0);
	break;

      case 2:
	arm_sim_command(TENOS_CMD_WRCH, Reg(0), 0, 0);
	break;

      case 3:
	arm_sim_command(TENOS_CMD_TRACE100, Reg(0), 0, 0);
	break;

	//case 4: please use new api STATS_CHECKPOINT now
	//..arm_sim_command(TENOS_CMD_DUMP_STATS, Reg(0), 0, 0);
	//break;

      case ARM_SWI_BACKDOOR_PMU_CYCLES: // Performance monitoring is via the ARM standed PMU IP block, but here's a quick backdoor!
	{
	  u64_t cycles, instructions;
	  stats.pmu_readoff(cycles, instructions);
	  Reg(0) = cycles;
	  Reg(1) = cycles >> 32;
	}
	break;

      case ARM_SWI_BACKDOOR_PMU_INSTRUCTIONS: // Performance monitoring should be via the ARM standard PMU IP block, but here's a quick backdoor!
	{
	  u64_t cycles, instructions;
	  stats.pmu_readoff(cycles, instructions);
	  Reg(0) = instructions;
	  Reg(1) = instructions >> 32;
	}
	break;

      case ARM_SWI_BACKDOOR_STATS_CHECKPOINT:
	if (vd) printf("ARM SWI backdoor stats checkpoint r0=0x%x r1=0x%x  pc=%x\n", Reg(0), Reg(1), Reg(PcReg));
	arm_sim_command(TENOS_CMD_STATS_CHECKPOINT, Reg(0), Reg(1), &Reg(0));
	if (vd) printf("ARM backdoor stats call r=%x\n", Reg(0));
	break;

      case 10: { /* fopen */
	/* Filename */
	/* FIME: We assume file name length is no longer then 256 */
	u32_t fname_addr = Reg(0);
	char filename[256];
	char *fptr = &filename[0];

	/* Mode */
	u32_t mode_addr = Reg(1);
	char mode[8];
	char *mptr = &mode[0];

	int len;
	char buffer[1024];
	int lfid;

	
	insSizeCode = 0;
	do {
	  armisa_read1(fname_addr++,0, false, false);
	  (*fptr++) = (char)read_data;
	} while(((char)read_data) != '\0');
	

	do {
	  armisa_read1(mode_addr++, 0, false, false);
	  (*mptr++) = (char)read_data;
	} while(((char)read_data) != '\0');

	bzero(&buffer[0], 1024);
	lfid = io_backdoor_su->open(filename, mode);

	Reg(0) = lfid;
	break;
      }
	
      case 11: { /* frefill */
	int len = 0;
	
	u32_t flen_addr = Reg(1);
	u32_t fbuf_addr = Reg(2);

	int size = Reg(3);

	char* buffer = (char*)malloc(sizeof(char)*size);

	bzero(&buffer[0], size);
	len = io_backdoor_su->fill(Reg(0), buffer, size);
	if(len > 0) {
	  for(int i = 0; i < len; ++i) {
	    armisa_write1(fbuf_addr++, buffer[i], 0, NULL);
	  }
	  armisa_write1(flen_addr, len, 2, NULL);

	  Reg(0) = 0;
	} else {
	  Reg(0) = 1;
	}

	break;
      }

      case 12: { /* fflush */
	int i;
	
	u32_t fbuf_addr = Reg(1);
	int size = Reg(2);	
	char* buffer = (char*)malloc(sizeof(char)*size);

	i = 0;
	bzero(&buffer[0], size);
	while(true) {
	  armisa_read1(fbuf_addr++, 0, false, false);
	  char d = (char)read_data;
	  buffer[i++] = d;
	  if(d == 0)
	    break;
	}

	Reg(0) = io_backdoor_su->flush(Reg(0), buffer);
	break;
      }

	
      case 13: { /* fclose */
	Reg(0) = io_backdoor_su->close(Reg(0));
	break;
      }

      case 14: {
#if TRACECOMM
	g_tracecomm = !g_tracecomm;
#endif
#if THREAD_COMM
	g_thread_comm = !g_thread_comm;
#endif
	
	break;
      }

      case 15: {
	/* Time */
	unsigned long t = lrint(sc_time_stamp().to_seconds()*1e6L);

	long int s = lrint(t/(long int)1e6);
	long int us = t - s*1e6;
	
	armisa_write1(Reg(0), g_te.tv_sec + s, 2, NULL);
	armisa_write1(Reg(1), g_te.tv_usec + us, 2, NULL);

	Reg(0) = 0;
	break;
      }

      default:
	printf("ARM ISA ignored backdoor no %i\n", code);
      }
    }
}



// --------------------------------------------------------------------------
// Halfword and signed byte operations
//
void armisa::DecodeHalfword()
{
  insRd = Bits(instruction, 12, 4);
  insRn = Bits(instruction, 16, 4);
  insSigned = Bit(instruction, 6);
  u1_t H = Bit(instruction, 5);
  insL = Bit(instruction, 20);
  insW = Bit(instruction, 21);
  u1_t I = Bit(instruction, 22);
  u1_t U = Bit(instruction, 23);
  insP = Bit(instruction, 24);
  insEXCF = false;
  u32_t preaddr, postaddr, addr;
  preaddr = Reg(insRn);
  if (I)
    {
      u8_t v = Bits(instruction, 0, 4) | (Bits(instruction, 8, 4) << 4);
      if (U) postaddr = preaddr + v; else postaddr = preaddr - v;
    }
  else
    {
      u32_t v = Reg(Bits(instruction, 0, 4));
      if (U) postaddr = preaddr + v; else postaddr = preaddr - v;
    }
  insDataAddress = (insP) ? postaddr: preaddr;
  insSizeCode = (H) ? 1:0;
  insDecodeT = SingleT;
}

// --------------------------------------------------------------------------

// Coprocessor Data transfers. need to implement properly
// using MidCoproXferS
u1_t armisa::DecodeCoprocessor_DT()
{

  u8_t offset = Bits(instruction, 0, 8);
  u4_t cp = Bits(instruction, 8, 4);
  u4_t cpr = Bits(instruction, 12, 4);
  u4_t reg = Bits(instruction, 16, 4);

  int op1 = 0, op2 = 0; // Cannot be set in this instruction?
  // Bit 20 is a 0 for store to memory
  if (Bit(instruction, 20))
    {
      // load from mem
      u1_t c = armisa_read1(insDataAddress, 2, FALSE);
      if (c) 
	{
	  u32_t data = read_data;
	  copro_write(cp, cpr, data, op1, op2);
	}
      else
          assert(0);
      return c;
    }
  else
  {
    // store to memory
    u32_t d =  copro_read(cp, cpr, op1, op2);
    u1_t c = armisa_write1(insDataAddress, d, 2);
    assert(c != 0);
    return c;
  }
}



// Data ops - none provided.
void armisa::DecodeCoprocessor_DO()
{

  // none implemented.


}

// Register transfers
void armisa::DecodeCoprocessor_RT()
{
  u4_t rm = Bits(instruction, 0, 4);
  u4_t cpn = Bits(instruction, 8, 4);
  u4_t Rd = Bits(instruction, 12, 4);
  u4_t crn = Bits(instruction, 16, 4);
  const char *dir;
  const char *nem = Bit(instruction, 20) ? "mrc": "mcr"; 
  u3_t op1 = Bits(instruction, 21, 3);
  u3_t op2 = Bits(instruction, 5, 3);

  if (Bit(instruction, 20))
    {
      dir = "from";
      Reg(Rd) = copro_read(cpn, crn, rm, op1, op2);
      OPERAND_PTRACE(Reg(Rd));
    }
  else
    {
      dir = "to";
      copro_write(cpn, crn, rm, Reg(Rd), op1, op2);
      OPERAND_PTRACE(Reg(Rd));
    }
  if (logfd) fprintf(logfd, "Coprocessor transfer Reg=%i  %s CP%i, Reg %i\n", Rd, dir, cpn, crn);
}



void armisa::DecodeIllegal()
{
    DecodeTrap(UndefinedMode, UndefinedVector);
}



void armisa::DecodeTrap(ArmMode mode, ArmVector vector)
{
    //
    // Take a trap - change to the given mode, jump to given vector.
    // Used also for hardware interrupts.
    if (mode == currentMode)
    {
        //
        // This is rather suspicious as state is definitely lost.
        //
      if (logfd) fprintf(logfd, "Taking recursive trap to unchanged mode '%s', vector '%s'" , 
            ArmModeToString(mode) , ArmVectorToString(vector));
    }
    else
    {
      if (logfd) fprintf(logfd, "Taking trap to vector '%s'" , ArmVectorToString(vector));
    }

    u32_t newpc = vector + vectorBase + ARM_PC_CORRECTION;
    int m = user_trace(TENOS_TRACE_SUBROUTINE_CALL, 0);
    //printf("Trapper %s\n", ArmVectorToString(vector));
    if (m) 
      {
	printf("%s: Trap from %x to mode=%X pc=%x vector=%s\n", myname(),  Reg(PcReg), mode, newpc, ArmVectorToString(vector));
      }
    u32_t prev_cpsr = Reg(CpsrReg);              // save previous CPSR before its low bits updated to reflect the mode change.
    // if (logfd) fprintf(logfd, "Old CPSR to save is %08X" , prev);
    u32_t prev_pc = Reg(PcReg); 
    // check to see if it is thumb instruction and if difference
    // between next instruction to be executed and current instruction is 2
    if(thumb_mode && (((prev_pc - ARM_PC_CORRECTION) - insAddress) == 0x2)) {
        // The exception was raised from thumb instruction that is 16 bits,
        // when we are returning from serving exception we subtract ARM_PC_CORRECTION
        // (that is 4) from the value of program counter so in order to redo this thumb16 instruction
        // we need to add 2 to program counter
        prev_pc += 2;
    }
    u32_t prev_lr = Reg(LrReg);
    u32_t prev_sp = Reg(13);

    //printf("Trap/Interrupt/Exception Save old: CPSR is %08X and the PC is %08X\n" , prev_cpsr, prev_pc);
    
    if(logfd) fprintf(logfd, "%s: Trap from %x to mode=%X pc=%x vector=%s\n", myname(),  Reg(PcReg), mode, newpc, ArmVectorToString(vector));
    if(thumb_mode) {
        if(InITBlock())
            assert(0); // TODO
        Reg(CpsrReg) &= ~(1 << 5); 
    }

    SetMode(mode, false);
    Reg(LrReg) = prev_pc;  // Save PC in LR of new mode.

    Reg(SpsrReg) = prev_cpsr;

    Reg(PcReg) = newpc;
    // pipelineBreak = TRUE;
}

void armisa::DecodeSetModeFromCpsr()
{
    //
    // Examine a new CPSR, and set mode appropriately
    //
    unsigned cpsr = Reg(CpsrReg);

    //printf("New mode cpsr %x\n", cpsr);
    switch (cpsr & 31) // bits M[4:0], (ref ARM7 3.8.2 pp 35)
    {
    case 0x10: SetMode(UserMode); break;
    case 0x11: SetMode(FiqMode); break;
    case 0x12: SetMode(IrqMode); break;
    case 0x13: SetMode(SupervisorMode); break;
    case 0x17: SetMode(AbortMode); break;
    case 0x1b: SetMode(UndefinedMode); break;
    case 0x1f: SetMode(SystemMode); break;
    default:
      printf("Bad cpsr %x\n", cpsr);
      TRACE2("Bad CPSR 0x%x, M bits wrong" _ cpsr);
    }
}

// --------------------------------------------------------------------------

// Decoding MRS and MSR instructions

void armisa::DecodeMrs()
{
    //
    // MRS (transfer PSR contents to a register)
    //
    // (ref ARM7 fig 4-11 pp 61)
    //
    unsigned Ps = Bit(instruction, 22);
    unsigned rd = Bits4(instruction, 12);

    insAluResult = Reg((Ps ? SpsrReg : CpsrReg)); 

    Reg(rd) = insAluResult;
    OPERAND_PTRACE(insAluResult);
}



void armisa::iss_reset()
{
  fiqSet = FALSE;
  irqSet = FALSE;
  resetPending = FALSE;
  instruction = 0;

  midInstruction = FALSE;
  memset(raw_reg, 0, sizeof(raw_reg));
  SetMode(SystemMode);  
  Reg(CpsrReg) |= 0xC0 + 0x1F;  // Set both interrupt masks, Fiq and Irq, and System Mode.
  Reg(PcReg) = 0x20000;
  runstate = ResettingS;
  
}


void armisa::DecodeMsr(int mask) // mask fields: { f s x c }
{
    //
    // MSR (transfer register contents to PSR)
    //
    // (ref ARM7 fig 4-11 pp 61)
    //
    unsigned Pd = Bit(instruction, 22);
    unsigned Rm = Bits4(instruction, 0);
    unsigned i = Bit(instruction, 25);
    

    u32_t bin_mask = 
      ((mask & 1) ? 0xFF: 0) | 
      ((mask & 2) ? 0xFF00: 0) | 
      ((mask & 4) ? 0xFF0000: 0) | 
      ((mask & 8) ? 0xFF000000: 0);

    if (i == 0)
    {
      //
      // source operand is in a register
        //
      insAluResult = Reg(Rm);
    }
    else
      {
        //
        // source operand is an immediate value
        //
        // NOTE - I assume that the format of immediate/rotate
        // is the same as in ALU operations with a constant.
        // However, this is not explicit in the text.
        //
        u32_t imm = Bits(instruction, 0, 8);
        u32_t rotate = Bits4(instruction, 8);
        // Rotate right by twice the imm field.
        rotate *= 2;
        insAluResult = (imm >> rotate) | (imm << (32 - rotate));
    }
    
    u32_t &psr = Reg((Pd == 1 ? SpsrReg : CpsrReg)); // TODO check Privileged mode to write SPSR.
    psr = (insAluResult & bin_mask) | (psr & ~bin_mask);
    if (Pd == 0) {
        //printf("Value of CPSR[%d] is %08x\n", m_core_no, psr);        
        DecodeSetModeFromCpsr();
    }
    OPERAND_PTRACE(insAluResult);
}


// --------------------------------------------------------------------------

// Single data transfers, LDR and STR
//
// Must be idempotent in terms of writebacks and so on since may be reissued.
// Returns 1 if completed.
//
void armisa::DecodeSingle()
{
  int c;
    //
    // LDR and STR - single register transfers
    //
    // (ref ARM7 4.9 pp 70)
    //
    // 'instruction' holds the current instruction.
    //
    // insRn and insRd have been set.
    // insP, insU, insB, insW, insL are set to various flags
    //
    u32_t in = instruction;

    assert(Bits(in, 26, 2) == 1); // a LDR or STR
    assert(insRn == Bits4(in, 16));
    assert(insRd == Bits4(in, 12));

    insDataAddress = Reg(insRn); // base register

    if (0 == Bit(in, 25)) // the I bit
    {
        //
        // Offset is an immediate value
        //
        insDataOffset = Bits(in, 0, 12);
    }
    else
    {
      //
      // Offset is a shifted register
      //
      if (Bit(in, 4) == 1)
        {
	  TRACE2("LD/ST with register shift, not valid (ARM7 4.9.2 pp 71)");
        }

      unsigned insRm = Bits4(in, 0);  
      unsigned shiftType = Bits(in, 5, 2);
      unsigned shift = Bits(in, 7, 5);
      unsigned rm = Reg(insRm); // unshifted value
      unsigned shiftComputed = Bit(in, 4);
      assert(shiftComputed == 0);

      //
      // Shift by constant amount
      //
      shift = Bits(in, 7, 5); // shift by constant 5-bit field
  
      switch (shiftType)
	{
	case 0:
	  //
	  // logical shift left by constant amount
	  //
	  insDataOffset = rm << shift;
	  break;

	case 1:
	  //
	  // logical shift right by constant amount
	  //
	  if (shift == 0) // LSR #0
	    {
	      insDataOffset = 0;
	    }
	  else
	    {
	      insDataOffset = rm >> shift;
	    }
	  break;

	case 2:
	  //
	  // arithmetic shift right by constant amount
	  //
	  if (shift == 0) // ASR #0
	    {
	      insDataOffset = 0 - insShiftCarryOut; // 0 or -1
	    }
	  else
	    {
	      insDataOffset = rm >> shift;
	      if (Bit(rm, 31)) insDataOffset |= 0xFFFFFFFFUL << (32 - shift);
	    }
	  break;

	case 3:
	  //
	  // rotate right by constant amount
	  //
	  if (shift == 0) // RRX, 33 bit rotate
	    {
	      u32_t c = CFlag();
	      insDataOffset = (rm >> 1) | (c << 31);
	    }
	  else
	    {
	      insDataOffset = Ror(rm, shift);
	    }
	  break;

	default: assert(FALSE);
	}
  
    }

    if (insW) // W==1 (writeback) additional checks
    {
        if (insP == 0) // P==0 (post-index) && W==1 (writeback) not valid
        {
            TRACE2("LD/ST with W && ~P not valid (ARM7 4.9.1 pp 71");
            // (ref ARM7 4.9.1 pp 71) says this is valid in priviledged
            // modes, and will be interpreted by memory management hardware
            // to force user-level operations. Not explicitly supported here.
        }

        if (insRn == 15)
        {
            // Writeback and R15 as base register, not allowed
            // (ref ARM7 4.9.4 pp 73)
            TRACE2("LD/ST with W and Rn=15 not valid (ARM7 4.9.4 pp 73)");
        }

        if (Bit(in, 25) == 1 && insRn == insRm) // I==1, so Rm present
        {
            // Writeback and Rn == Rm, not allowed
            // (ref ARM7 4.9.5 pp 73)
            TRACE2("LD/ST with W and Rn==Rm not valid (ARM7 4.9.5 pp 73");
        }


        if (insRn == insRd && 1 == insL)
        {
            // Writeback, and loading the same register - surely wrong
            TRACE2("LD with W and Rd==Rn, not valid");
        }
    }

    if (insU == 0) insDataOffset = 0 - insDataOffset; // U bit
    if (insP == 1) insDataAddress += insDataOffset; // P bit
    insSizeCode = (insB) ? 0: 2; // This does unsigned bytes and words
    insSigned = false;
    insEXCF = false;
    insDecodeT = SingleT;
    OPERAND_PTRACE(insDataAddress);
}


// --------------------------------------------------------------------------

// Multiple data transfers, LDM and STM

bool armisa::DecodeMultiple()
{
    // LDM and STM - multiple register transfers
    //
    // (ref ARM7 4.11 pp 82)
    //
    // The bus is available for use.
    // 'instruction' holds the current instruction.
    // All relevant instruction field variables have been set.
    //
    // We always do the transfers starting with the lowest memory
    // address, and working upwards. This is what a real ARM does.
    insMultCount = 0; // count of registers to be transferred.
    u32_t j;
    for (j = 0; j < 16; j++) if (Bit(insMultRegisters, j)) insMultCount++;

    if (insMultCount == 0)
    {
        TRACE2("LDM/STM with empty transfer list, not valid (ARM7 4.11.1 pp 82)");
        assert(0);
    }

    if (insRn == 15)
    {
        TRACE2("LDM/STM with PC as index, not valid (ARM7 4.11.5 pp 86)");
    }

    if (insS == 1)
    {
        if (currentMode == UserMode)
        {
            TRACE2("LDM/STM with S in user mode, not valid (ARM7 4.11.4 pp 85)");
        }

        if (Bit(insMultRegisters, 15) == 0 || (insL == 0  && Bit(insMultRegisters, 15) == 1))
        {
            if (1 == insW)
            {
                // STM with R15 in transfer list and S bit set,
                // or R15 not in list and S bit set:
                // writeback "should not" be used.
                // (ref ARM7 4.11.4 pp 85)
                TRACE2("LDM/STM use of W and S not valid (ARM7 4.11.4 pp 85");
            }
        }
    }

    insDataAddress = Reg(insRn);
    OPERAND_PTRACE(insDataAddress);
    if (insU == 0)
    {
        // U==0, decrementing addresses
        insMultWriteback = insDataAddress - insMultCount*4;  // value to write back
        insDataAddress = insMultWriteback + 4; 
    }
    else
    {
        // U==1, incrementing addresses
        insMultWriteback = insDataAddress + 4*insMultCount;
    }

    // if (logfd)    fprintf(logfd, "multipl insW=%i regs=%i insP=%i insU=%i  insMultWriteback=%x, insDataAddress=%x\n",  insW, insMultCount,  insP, insU, insMultWriteback, insDataAddress);
    if (insP == 1) // P==1, pre-inc/dec (rather than post-inc/dec)
    {
        if (insU == 1) insDataAddress += 4; else insDataAddress -= 4;
    }

    // insDataAddress is now the lowest address that we will use.
    insRd = 0;
    while (0 == Bit(insMultRegisters, insRd)) insRd++; // find lowest reg
    // Initiate a load or store for this register to/from this location.
    insSUser = (insL && insS && Bit(insMultRegisters, 15) != 1) || (!insL && insS);
    insSizeCode = 2;
    insEXCF = 0;
    insSigned = 0;
    return true;
}


int armisa::DecodeMultiple2() // LDM/STM
{
  int rc = 0;

  if(insL && (insS == 1) && (Bit(insMultRegisters, 15) == 1) && (insRd == 15)) {
    // LDM with R15 in transfer list and S bit set (Mode changes)
    // (ref ARM7 4.11.4 pp 85)
      Reg(CpsrReg) = Reg(SpsrReg);
      DecodeSetModeFromCpsr();        
  }

  if(insRd == 15 && (Reg(insRd) & 1) == 1) {
    Reg(insRd) &= ~1;
    Reg(CpsrReg) |= (1 << 5);
  }


  if (insMultCount == 1) rc = 1; // Finished load/store
  if (rc && insW) Reg(insRn) = insMultWriteback;  // Update index register/sp.
  if (!rc)
    {
      //
      // Move on to next reg
      //
      insDataAddress += 4;
      insRd++;
      insMultCount--;
      while (0 == Bit(insMultRegisters, insRd)) insRd++; // find next register in list
      assert(insRd <= 15);
    }

  return rc;
}

//
//
//
void armisa::bodystate_clock() // Do not call this when halted.
{

again:
  stats.m_Cycles += 1;
  bool uncond = false;
  prev_thumb_mode = thumb_mode;
  thumb_mode = (Reg(CpsrReg) >> 5) & 1;
  switch ((ArmState) runstate)
    {
	
    case ResettingS:
      {
	  //
	  // We want to execute from the reset vector.
	  //
	  thumb_mode = 0;
	  lastAddress = 0xFFFFffff; // an invalid address
	  prevAddress = 0xFFFFffff; // an invalid address
	  nextAddress = 0xFFFFffff; // an invalid address
	  m_uses_defs_idx = 0;
	  SetMode(SystemMode);
	  Reg(PcReg) = ResetVector + vectorBase + ARM_PC_CORRECTION + 0x20000; // bodystate
	  printf("%s: set reset pc %x\n", myname(), Reg(PcReg));
	  Reg(CpsrReg) |= 0xC0 + 0x1F;  // Set both interrupt masks, Fiq and Irq, and System Mode.         
	}
      break;
      

      case StartingS: // bodystate
	{
	  insAddress = Reg(PcReg)-ARM_PC_CORRECTION; // bodystate	  
	  int c = armisa_read1(insAddress, 2, TRUE);
          assert(c == 1);
	  runstate = RunningS;
	}
      break;

      case RunningS: // bodystate - fetch.
	{
          u32_t oldInsAddress = insAddress;
	  insAddress = Reg(PcReg)-ARM_PC_CORRECTION; // bodystate
	  //printf("insAddress is %08x\n", insAddress);

	  if (stop_after_ins_count >= 0)
	    {
	      stop_after_ins_count -= 1;
	      if (stop_after_ins_count == -1)
		{
		  printf("%s: Stop_after limit hit at %x\n", myname(), insAddress);
		  sigval = TARGET_SIGNAL_TRAP;
		  arm_sim_command(TENOS_CMD_CORE_HALT, 0, 0, 0);
		  return;
		}
	    }

	  if (instruction_grace)
	    {
	      instruction_grace = false;
	    }
	  else
	    for (int w=0; w<breakpoints_active; w++)
	      {
		//printf("Break check %x cf %x\n", pc_breakpoints[w].addr, insAddress);
		if (pc_breakpoints[w].addr == insAddress)
		  {
		    printf("%s: Break hit %i at %x\n", myname(), w, pc_breakpoints[w].addr);
		    sigval = TARGET_SIGNAL_TRAP;
		    arm_sim_command(TENOS_CMD_CORE_HALT, 0, 0, 0);
		    if (pc_breakpoints[w].style == SingleStep) delete_breakpoint(0, w);
		    return;
		  }
	      }
          u32_t instrSize = thumb_mode ? 1 : 2;          
          int c = armisa_read1(insAddress, instrSize, TRUE);              
          if (c) {
              instruction = read_data;
          }
          
          if (!c) {
              // For trap, we always add 4 to program counter
              // regardless of the mode we are in
              Reg(PcReg) += 4;
              DecodeTrap(AbortMode,DataVector);
              return; // Return if the current instruction is not to hand
          }
          
          Reg(PcReg) += (thumb_mode) ? 2: 4;  // Advance the PC. PC increment.  User model sees it 8 more than current instruction.

	  if (0) // abort sequence not implemented here
	      {
		//
		// Our attempt to load an instruction lead to a bad
		// memory access. This will lead to a prefetch abort.
		//

		DecodeTrap(AbortMode, PrefetchVector);
		runstate = StartingS;
		break;
	      }
	    
	  

	  //	  fprintf(logfd, "bodystate Collected ins %08X from the bus allegedly from address %08X\n", instruction, insAddress);

          last_shutdown_sequence = shutdown_sequence;
          shutdown_sequence = false;

	  //
	  // Break out information about the new instruction
	  //

	  
	  //
	  // Decode and execute the fetched instruction
	  //
	  // bodystate: PC has advanced by 8 (ARM32 mode) at this point.
	  //

	  // IPC analysis
	  
	  do_ipc_analysis();

	  //	  printf("t32=%i idx=%i i=%llu b=%llu\n", m_thumb32, m_uses_defs_idx, stats.m_Instructions, stats.m_InstructionBlocks);

	  // Set all registers to be undefined
	  insRn = UndefinedReg;
	  insRd = UndefinedReg;
	  insRs = UndefinedReg;
	  insRm = UndefinedReg;
	  insRdHi = UndefinedReg;
	  insRdLo = UndefinedReg;

	  bool executed = true;
	  if (thumb_mode)
	    {
	      m_thumb32 = DecodeThumbInstruction(&executed);
	    }
	  else
	    {
	      m_thumb32 = false;
	      // we should never return false from decoding	     
	      assert(DecodeArmInstruction(uncond, &executed));

	    }

	  if(executed) {
	    stats.m_Instructions += 1;
	  }


          // check to see what is type of decoded
          // instruction and then based on that 
          // execute instruction
          // TODO: Consider whether we want to
          // have a separate state for execution
          switch(insDecodeT) {
            case SynchronisationT: {
                if(!(exec_mem_operand_instr())) {
                    // we are having data abort exception
                    DecodeTrap(AbortMode, DataVector);
                    runstate = RunningS;
                    return;
                }
                break;
            }

            case FloatT: {
	        stats.m_Floats+=1;
                if(!(exec_float_instr())) {
                    // data abort exception
                    DecodeTrap(AbortMode, DataVector);
                    runstate = RunningS;
                    return;
                }
                break;
            }

	    case FloatMemoryT: {
	      stats.m_Floats += 1;
	      if(!(exec_float_memory_instr())) {
		// data abort exception
		DecodeTrap(AbortMode, DataVector);
		runstate = RunningS;
		return;
	      }
	      break;
	    }

	    case FloatALUT: {
	      stats.m_Floats += 1;
	      exec_float_alu_instr();
	      break;
	    }
              
            case AluT: {
	        stats.m_ALU+=1;
                exec_alu_operand_instr();
                break;
            }

            case ParallelAluT: {
	        stats.m_ALU+=1;
                exec_parallel_alu_operand_instr();
                break;
            }

            case MulT: {
	        stats.m_ALU+=1;
                exec_mul_operand_instr();
                break;
            }

            case SwiT: {
	      // printf("SWI  swi-arg=%i pc=0x%x\n", insSWIArg, Reg(PcReg));
	      stats.m_Unknowns+=1;
                TRACE5("Software Interrupt 0x%0x" _ insSWIArg);
                // Some backdoor SWIs
                if (insSWIArg >= ARM_SWI_BACKDOOR_BASE && insSWIArg < ARM_SWI_BACKDOOR_BASE+100) arm_swi_backdoor(insSWIArg - ARM_SWI_BACKDOOR_BASE);
                else {
                    // PC here must be PC of the executed instruction and not next
                    // instruction to execute
                    Reg(PcReg) = Reg(PcReg) - 4;
                    DecodeTrap(SupervisorMode, SwiVector);
                }
                
                break;
            }

            case TableBranchT: {
 	        add_use(m_uses_defs_idx, insRn);
	        add_use(m_uses_defs_idx, insRm);
	        add_use(m_uses_defs_idx, insRs);
	      
                int rc = armisa_read1(insDataAddress, insSizeCode, false, false);
                if(!rc) {
                    // we are having data abort exception
                    DecodeTrap(AbortMode, DataVector);
                    runstate = RunningS;
                    return;
                }

                u32_t offset = read_data;
                switch(insSizeCode) {
                  case 0: 
                    offset &= 0xFF;
                    break;
                  case 1:
                    offset &= 0xFFFF;
                    break;
                  default:
                    assert(0);
                }

                u32_t dest = Reg(PcReg) - ARM_PC_CORRECTION + 2*offset;
		add_use(m_uses_defs_idx, PcReg);
                dest &= ~1;
                insL = false;
		// TODO predictor munge missing
                ExecuteBL(dest);

                break;
            }
              
            case UndefinedT: {
                DecodeTrap(UndefinedMode, UndefinedVector);
                runstate = RunningS;
                return;
            }
              
            default:
              // do nothing for now, but we will be adding new instruction types
	      if((insDecodeT == UnknownT || insDecodeT == CoprocRegT) && executed)
		stats.m_Unknowns+=1;
	      
              break; 
          }



	  if (logfd)
	    {
	      dis.dis(insAddress, instruction, insDisasm, thumb_mode);
	      char line[132];
	      if (argv_backdoor_su)
		argv_backdoor_su->symbolic_dis64(line, 132, insAddress);
	      else
		snprintf(line, 132, " %X ", insAddress);

		fprintf(logfd, "%s, Instruction: %s: %s\n", myname(), line, insDisasm);
		PrintRegs(logfd);
	    }
	  if (runstate == (unsigned) MidSingleS) goto again; 
	  if (runstate == (unsigned) MidHalfwordS) goto again; 
	}
      break;
      
      case MidSingleS: // bodystate
	{
	  // We are half-way through a LDR/STR instruction.
	  // The prefetched memory request has completed.
	  //
          // after executing above method the decoded
          // instruction must be of type SingleT
          assert(insDecodeT == SingleT);
          // execute instruction
          if(!(exec_mem_operand_instr())) {
              // we are having data abort exception
              DecodeTrap(AbortMode, DataVector);
              runstate = RunningS;
              return;
          }
	  runstate = RunningS;
	}
      break;

      case MidHalfwordS: // bodystate
	{
	  //
	  // We are half-way through a LDR/STR instruction.
	  // The prefetched memory request has completed.
	  //
          DecodeHalfword(); 
          // after executing above method the decoded
          // instruction must be of type SingleT
          assert(insDecodeT == SingleT);
          // execute instruction
          if(!(exec_mem_operand_instr())) {
              // we are having data abort exception
              DecodeTrap(AbortMode, DataVector);
              runstate = RunningS;
              return;
          }
	  runstate = RunningS;
	}
      break;


      //-------------------------------------------------------------
      
      case MidMultipleS:
	{
	  // We are half-way through a LDM/STM instruction.
          if(!DecodeMultiple()) {
              DecodeTrap(AbortMode, DataVector);
              runstate = RunningS;
              return;  // Return if memory cycle not complete: we will reissue at no cost next cycle.
            }

	  // decode instruction, issue memory cycle
	  runstate = MidMultiple2S;
	}
      break;
      
      case MidMultiple2S:
	{
	  // We are performing the LDM/STM bus access.
	  // The pipelined instruction is saved in pipelined_data_ready.

          // we enter this state after being in MidMultipleS
          // or continuation of this state
          // therefore decoded instruction must be of type BlockT
          assert(insDecodeT == BlockT);
          // execute instruction
          if(!(exec_mem_operand_instr())) {
              // we are having data abort exception
              DecodeTrap(AbortMode, DataVector);
              runstate = RunningS;
              return;
          }

	  int c = DecodeMultiple2();
	  // Continue execution of instruction
          if(!c)
	    {
	      // The LDM/STM is not finished.
	      // Another memory cycle has been issued.
	      // Stay in this state.
	      // OR (if pipelineBreak) we have a data abort
	    }
	  else
	    { // The LDM/STM is complete.
	      runstate = RunningS;
	      if(insL)
		stats.m_Loads+=1;
	      else
	      	stats.m_Stores+=1;
	    }
	}
      break;
      
      default: // bodystate
	printf("Bad bodystate %i\n", runstate);
	assert(FALSE);
	break;
      }
    }


bool armisa::hw_interrupt(const char *msg)
{
  //if (!nFIQ && currentMode != FiqMode && !FIQFlag()) DecodeTrap(FiqMode, FiqVector);
  if (currentMode != IrqMode && !IRQFlag() && (currentMode != FiqMode)) {
      if(runstate != RunningS) {
          // we are in the middle of the pipeline so we are not going
          // to serve interrupt at this point and are going to complete
          // instruction execution, once we have completed the instructions
          // execution we will then serve interrupt
          interrupt_wait++;
          return false;
        }

      if(interrupt_wait > 0) {
          interrupt_wait = 0;
      }
    DecodeTrap(IrqMode, IrqVector);
    return true;
  }

  return false;
}


void armisa::execute()
{
  if (logfd)
    {
    }
  
  if (runstate == ResettingS) runstate = StartingS;


  if (0) // another trace site
    {
      PrintRegs(stdout);
      fprintf(stdout, "   state %i %s\n", runstate, state_string(runstate));
    }

  bodystate_clock();
}
  

void armisa::copro_write(u4_t cpn, u4_t cp, u4_t cpr, u32_t data, u3_t op1, u3_t op2) 
{
  // unimplemented copro
}

u32_t armisa::copro_read(u4_t cpn, u4_t cp, u4_t cpr, u3_t op1, u3_t op2)
{

  return 0;
}

u32_t armisa::get_spr(int x) 
{
  if (x >= VHLS_SPR_GPR0 && x < VHLS_SPR_GPR0+16) return Reg(x - VHLS_SPR_GPR0); // Read general purpose regs of current mode.
  if (x == VHLS_SPR_PC) return Reg(PcReg);
  if (x == VHLS_SPR_SR) return Reg(CpsrReg); // current, not supervisor always.

  // Not sure why 0x19 is used here. Part of RSP for ARM?
  if (x == 0x19) return Reg(CpsrReg); // current, not supervisor always.

  // See:  http://www.cems.uwe.ac.uk/~rwilliam/FSSE_ufeETV-20-1/gnu_gdb.pdf
  printf ("%s: armisa: +++ UNIMPLEMENTED SPR: Get spr $%x\n", myname(), x);
  return 0; /* for now */    
}

void armisa::set_stop_after_count(int n)
{
  printf("%s: Set stop after %i instructions\n", myname(), n);
  stop_after_ins_count = n;
}

 
TargetSignal armisa::get_sigval()
{
  return sigval;
}

void armisa::set_reg(int x, int d) // used by the debugger
{
  if (x >=0 && x <= 15)
    {
      printf("Set GPR 0x%x, d=0x%x\n", x, d); 
      Reg(x) = d;
    }
  else
    printf("Ignored set GPR 0x%x, d=0x%x\n", x, d); 
}



void armisa:: set_spr(int x, int d) // used by the debugger
{ 
  if (x >= VHLS_SPR_GPR0 && x < VHLS_SPR_GPR0+16) return set_reg(x - VHLS_SPR_GPR0, d); // Write general purpose regs of current mode.
  printf ("Ignored set SPR 0x%x d=0x%x\n", x, d); 
}

void armisa::do_ipc_analysis() {
  
  if(stats.m_Instructions > 0) {

    if(!m_thumb16_only_ipc
       || (m_thumb16_only_ipc
	   && prev_thumb_mode
	   && !m_thumb32)) {
      switch(m_uses_defs_idx) {
	
        case 0: {
	  m_uses_defs_idx++;
	  break;
	}
		
        case 1: {
	  // compare if there is a true dependency
	  if((uses[1] & defs[0]) == 0) {
	    // no true dependency
	    clear_uses_and_defs();
	    m_uses_defs_idx = 0;
	  } else {
	    // there is a true dependency
	    uses[0] = uses[1];
	    defs[0] = defs[1];
	    uses[1] = 0;
	    defs[1] = 0;
	    m_uses_defs_idx = 1;
	  }
	  stats.m_InstructionBlocks++;
	  break;
	}
	  
        default: {
	  // We should never hit this as at the
	  // moment our IPC measurement is hard coded
	  // to be at most 2
	  assert(0);
	}
      }
    } else {
      if(m_uses_defs_idx == 1)
	stats.m_InstructionBlocks++;
		  
      clear_uses_and_defs();
      m_uses_defs_idx = 0;
      stats.m_InstructionBlocks++;
    }
  }

}




// end of file



