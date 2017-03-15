//
// $Id: orsim.cpp,v 1.2 2011-02-15 15:13:35 djg11 Exp $
//

#include <stdio.h>
#include <assert.h>
#include <tenos.h>

#define PRINTF(...) printf(__VA_ARGS__)


/* orsim.cpp -- OR1K architecture dependent simulation


   C++ CORE (C) 2010 DJ Greaves.

   Copyright (C) 1999 Damjan Lampret, lampret@opencores.org
   Copyright (C) 2005 György `nog' Jeney, nog@sdf.lonestar.org
   Copyright (C) 2008 Embecosm Limited
  
   Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>
  
   This file is part of OpenRISC 1000 Architectural Simulator.
  
   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 3 of the License, or (at your option)
   any later version.
  
   This program is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
   more details.
  
   You should have received a copy of the GNU General Public License along
   with this program.  If not, see <http://www.gnu.org/licenses/>. */

/* This program is commented throughout in a fashion suitable for processing
   with Doxygen. */


/* Most of the OR1K simulation is done in here.

   When SIMPLE_EXECUTION is defined below a file insnset.c is included!
*/

/* Autoconf and/or portability configuration */
#include "config.h"
#include "port.h"



/* System includes */
#include <stdlib.h>

#include "orsim.h"


/* Package includes */
#include "or32.h"
#include "except.h"
#include "sprs.h"
#include "branch-predict.h"



/* Includes and macros for simple execution */
#if SIMPLE_EXECUTION

#define SET_PARAM0(val) set_operand(0, val, current->insn_index, current->insn)

#define PARAM0 eval_operand(0, current->insn_index, current->insn)
#define PARAM1 eval_operand(1, current->insn_index, current->insn)
#define PARAM2 eval_operand(2, current->insn_index, current->insn)

#define INSTRUCTION(name) void name (struct iqueue_entry *current)

#endif	/* SIMPLE_EXECUTION */



//extern "C" void decode_execute (struct iqueue_entry *current);

/* Forward declaration of static functions */
#if !(DYNAMIC_EXECUTION)
//static 
void decode_execute (struct iqueue_entry *current);
#endif

/*---------------------------------------------------------------------------*/
/*!Get an actual value of a specific register

   Implementation specific. Abort if we are given a duff register. Only used
   externally to support simprintf().

   @param[in] regno  The register of interest

   @return  The value of the register                                        */
/*---------------------------------------------------------------------------*/
uorreg_t orsim::evalsim_reg (unsigned int  regno)
{
  if (regno < MAX_GPRS)
    {
#if RAW_RANGE_STATS
      int delta = (runtime.sim.cycles - raw_stats.reg[regno]);

      if ((unsigned long) delta < (unsigned long) RAW_RANGE)
	{
	  raw_stats.range[delta]++;
	}
#endif /* RAW_RANGE */

      return cpu_state.reg[regno];
    }
  else
    {
      PRINTF ("\nABORT: read out of registers\n");
      orsim_sim_done ("ABORT: read out of registers");
      return 0;
    }
}	/* evalsim_reg() */


/*---------------------------------------------------------------------------*/
/*!Set a specific register with value

   Implementation specific. Abort if we are given a duff register.

   @param[in] regno  The register of interest
   @param[in] value  The value to be set                                     */
/*---------------------------------------------------------------------------*/
void orsim::setsim_reg (unsigned int  regno,
	    uorreg_t      value)
{
  if (regno == 0)		/* gpr0 is always zero */
    {
      value = 0;
    }

  if (regno < MAX_GPRS)
    {
      cpu_state.reg[regno] = value;
    }
  else
    {
      PRINTF ("\nABORT: write out of registers\n");
      orsim_sim_done ("ABORT: write out of registers");
    }

#if RAW_RANGE_STATS
  raw_stats.reg[regno] = runtime.sim.cycles;
#endif /* RAW_RANGE */

}	/* setsim_reg() */


/*---------------------------------------------------------------------------*/
/*!Evaluates source operand operand

   Implementation specific. Declared global, although this is only actually
   required for DYNAMIC_EXECUTION,

   @param[in] insn  The instruction
   @param[in] opd   The operand

   @return  The value of the source operand                                  */
/*---------------------------------------------------------------------------*/
uorreg_t
eval_operand_val (uint32_t               insn,
		  struct insn_op_struct *opd)
{
  unsigned long  operand = 0;
  unsigned long  sbit;
  unsigned int   nbits = 0;

  while (1)
    {
      operand |=
	((insn >> (opd->type & OPTYPE_SHR)) & ((1 << opd->data) - 1)) <<
	nbits;
      nbits += opd->data;

      if (opd->type & OPTYPE_OP)
	{
	  break;
	}

      opd++;
    }

  if (opd->type & OPTYPE_SIG)
    {
      sbit = (opd->type & OPTYPE_SBIT) >> OPTYPE_SBIT_SHR;

      if (operand & (1 << sbit))
	{
	  operand |= ~REG_C (0) << sbit;
	}
    }

  return operand;

}	/* eval_operand_val() */


/*---------------------------------------------------------------------------*/
/*!Does source operand depend on computation of dest operand?

      Cycle t                 Cycle t+1
  dst: irrelevant         src: immediate                  always 0
  dst: reg1 direct        src: reg2 direct                0 if reg1 != reg2
  dst: reg1 disp          src: reg2 direct                always 0
  dst: reg1 direct        src: reg2 disp                  0 if reg1 != reg2
  dst: reg1 disp          src: reg2 disp                  always 1 (store must
                                                          finish before load)
  dst: flag               src: flag                       always 1

  @param[in] prev  Previous instruction
  @param[in] next  Next instruction

  @return  Non-zero if yes.                                                  */
/*---------------------------------------------------------------------------*/
static int
check_depend (struct iqueue_entry *prev,
	      struct iqueue_entry *next)
{
  /* Find destination type. */
  unsigned long          type = 0;
  int                    prev_dis;
  int                    next_dis;
  orreg_t                prev_reg_val = 0;
  struct insn_op_struct *opd;

  if (or32_opcodes[prev->insn_index].flags & OR32_W_FLAG
      && or32_opcodes[next->insn_index].flags & OR32_R_FLAG)
    {
      return  1;
    }

  opd      = op_start[prev->insn_index];
  prev_dis = 0;

  while (1)
    {
      if (opd->type & OPTYPE_DIS)
	{
	  prev_dis = 1;
	}

      if (opd->type & OPTYPE_DST)
	{
	  type = opd->type;

	  if (prev_dis)
	    {
	      type |= OPTYPE_DIS;
	    }

	  /* Destination is always a register */
	  prev_reg_val = eval_operand_val (prev->insn, opd);
	  break;
	}

      if (opd->type & OPTYPE_LAST)
	{
	  return 0;		/* Doesn't have a destination operand */
	}

      if (opd->type & OPTYPE_OP)
	{
	  prev_dis = 0;
	}

      opd++;
    }

  /* We search all source operands - if we find confict => return 1 */
  opd      = op_start[next->insn_index];
  next_dis = 0;

  while (1)
    {
      if (opd->type & OPTYPE_DIS)
	{
	  next_dis = 1;
	}

      /* This instruction sequence also depends on order of execution:
           l.lw r1, k(r1)
           l.sw k(r1), r4
         Here r1 is a destination in l.sw */
   
      /* FIXME: This situation is not handeld here when r1 == r2:
           l.sw k(r1), r4
           l.lw r3, k(r2) */
      if (!(opd->type & OPTYPE_DST) || (next_dis && (opd->type & OPTYPE_DST)))
	{
	  if (opd->type & OPTYPE_REG)
	    {
	      if (eval_operand_val (next->insn, opd) == prev_reg_val)
		{
		  return 1;
		}
	    }
	}

      if (opd->type & OPTYPE_LAST)
	{
	  break;
	}

      opd++;
    }

  return  0;

}	/* check_depend() */


/*---------------------------------------------------------------------------*/
/*!Should instruction NOT be executed?

   Modified by CZ 26/05/01 for new mode execution.

   @return  Nonzero if instruction should NOT be executed                    */
/*---------------------------------------------------------------------------*/
int orsim::fetch ()
{

  //  if (breakpoints)
    {
      /* MM: Check for breakpoint.  This has to be done in fetch cycle,
         because of peripheria.  
         MM1709: if we cannot access the memory entry, we could not set the
         breakpoint earlier, so just check the breakpoint list.  */
      if (has_breakpoint (peek_into_itlb (cpu_state.pc)) && !break_just_hit)
	{
	  break_just_hit = 1;
	  return 1;		/* Breakpoint set. */
	}
      break_just_hit = 0;
    }

  breakpoint                 = 0;
  cpu_state.iqueue.insn_addr = cpu_state.pc;
  if (myconfig.sim.verbose)
    {
      PRINTF ("The value of PC is 0x%" PRIxADDR "\n", cpu_state.pc);
    }

  cpu_state.iqueue.insn      = eval_insn (cpu_state.pc, &breakpoint);

  /* Fetch instruction. */
  if (!except_pending)
    {
      runtime.cpu.instructions++;
    }

  /* update_pc will be called after execution */
  return 0;

}	/* fetch() */


/*---------------------------------------------------------------------------*/
/*!This code actually updates the PC value                                   */
/*---------------------------------------------------------------------------*/
void orsim::update_pc ()
{

  if (myconfig.sim.verbose)
    {
      PRINTF ("11: Starting at 0x%" PRIxADDR "\n", pcnext);
      PRINTF ("11: Starting at 0x%" PRIxADDR "\n", pcnext);
    }

  cpu_state.delay_insn    = next_delay_insn;
  cpu_state.sprs[SPR_PPC] = cpu_state.pc;	/* Store value for later */
  cpu_state.pc            = pcnext;
  pcnext                  = cpu_state.delay_insn ? cpu_state.pc_delay :
                                                   pcnext + 4;

  if (myconfig.sim.verbose)
    {
      PRINTF ("22: Starting at 0x%" PRIxADDR "\n", cpu_state.pc);
    }

  //  printf("Updated PC=%x, %i next=%x\n", cpu_state.pc, cpu_state.delay_insn, pcnext); 
}	/* update_pc() */


/*---------------------------------------------------------------------------*/
/*!Perform analysis of the instruction being executed

   This could be static for SIMPLE_EXECUTION, but made global for general use.

   @param[in] current  The instruction being executed                        */
/*---------------------------------------------------------------------------*/
void orsim::analysis (struct iqueue_entry *current)
{
  if (myconfig.cpu.dependstats)
    {
      /* Dynamic, dependency stats. */
      adddstats (cpu_state.icomplet.insn_index, current->insn_index, 1,
		 check_depend (&cpu_state.icomplet, current));

      /* Dynamic, functional units stats. */
      addfstats (or32_opcodes[cpu_state.icomplet.insn_index].func_unit,
		 or32_opcodes[current->insn_index].func_unit, 1,
		 check_depend (&cpu_state.icomplet, current));

      /* Dynamic, single stats. */
      addsstats (current->insn_index, 1);
    }

  if (myconfig.cpu.superscalar)
    {
      if ((or32_opcodes[current->insn_index].func_unit == it_branch) ||
	  (or32_opcodes[current->insn_index].func_unit == it_jump))
	runtime.sim.storecycles += 0;
      
      if (or32_opcodes[current->insn_index].func_unit == it_store)
	runtime.sim.storecycles += 1;

      if (or32_opcodes[current->insn_index].func_unit == it_load)
	runtime.sim.loadcycles += 1;

      /* Pseudo multiple issue benchmark */
      if ((multissue[or32_opcodes[current->insn_index].func_unit] < 1) ||
	  (check_depend (&cpu_state.icomplet, current))
	  || (issued_per_cycle < 1))
	{
	  int i;
	  for (i = 0; i < 20; i++)
	    multissue[i] = 2;
	  issued_per_cycle = 2;
	  runtime.cpu.supercycles++;
	  if (check_depend (&cpu_state.icomplet, current))
	    runtime.cpu.hazardwait++;
	  multissue[it_unknown] = 2;
	  multissue[it_shift] = 2;
	  multissue[it_compare] = 1;
	  multissue[it_branch] = 1;
	  multissue[it_jump] = 1;
	  multissue[it_extend] = 2;
	  multissue[it_nop] = 2;
	  multissue[it_move] = 2;
	  multissue[it_movimm] = 2;
	  multissue[it_arith] = 2;
	  multissue[it_store] = 2;
	  multissue[it_load] = 2;
	}
      multissue[or32_opcodes[current->insn_index].func_unit]--;
      issued_per_cycle--;
    }

  if (myconfig.cpu.dependstats)
    /* Instruction waits in completition buffer until retired. */
    memcpy (&cpu_state.icomplet, current, sizeof (struct iqueue_entry));

  if (myconfig.sim.history)
    {
      /* History of execution */
      hist_exec_tail = hist_exec_tail->next;
      hist_exec_tail->addr = cpu_state.icomplet.insn_addr;
    }

  if (myconfig.sim.exe_log)
    dump_exe_log ();

}	/* analysis() */


#if !(DYNAMIC_EXECUTION)

/*---------------------------------------------------------------------------*/
/*!Store buffer analysis for store instructions

   Stores are accumulated and commited when IO is idle

   @param[in] cyc  Number of cycles being analysed                           */
/*---------------------------------------------------------------------------*/
void orsim::sbuf_store (int cyc)
{
  int delta = runtime.sim.cycles - sbuf_prev_cycles;

  sbuf_total_cyc   += cyc;
  sbuf_prev_cycles  = runtime.sim.cycles;

  /* Take stores from buffer, that occured meanwhile */
  while (sbuf_count && delta >= sbuf_buf[sbuf_tail])
    {
      delta     -= sbuf_buf[sbuf_tail];
      sbuf_tail  = (sbuf_tail + 1) % MAX_SBUF_LEN;
      sbuf_count--;
    }

  if (sbuf_count)
    {
      sbuf_buf[sbuf_tail] -= delta;
    }

  /* Store buffer is full, take one out */
  if (sbuf_count >= myconfig.cpu.sbuf_len)
    {
      sbuf_wait_cyc          += sbuf_buf[sbuf_tail];
      runtime.sim.mem_cycles += sbuf_buf[sbuf_tail];
      sbuf_prev_cycles       += sbuf_buf[sbuf_tail];
      sbuf_tail               = (sbuf_tail + 1) % MAX_SBUF_LEN;
      sbuf_count--;
    }

  /* Put newest store in the buffer */
  sbuf_buf[sbuf_head] = cyc;
  sbuf_head           = (sbuf_head + 1) % MAX_SBUF_LEN;
  sbuf_count++;

}	/* sbuf_store() */


/*---------------------------------------------------------------------------*/
/*!Store buffer analysis for load instructions

   Previous stores should commit, before any load                            */
/*---------------------------------------------------------------------------*/
void orsim::sbuf_load ()
{
  int delta = runtime.sim.cycles - sbuf_prev_cycles;
  sbuf_prev_cycles = runtime.sim.cycles;

  /* Take stores from buffer, that occured meanwhile */
  while (sbuf_count && delta >= sbuf_buf[sbuf_tail])
    {
      delta     -= sbuf_buf[sbuf_tail];
      sbuf_tail  = (sbuf_tail + 1) % MAX_SBUF_LEN;
      sbuf_count--;
    }

  if (sbuf_count)
    {
      sbuf_buf[sbuf_tail] -= delta;
    }

  /* Wait for all stores to complete */
  while (sbuf_count > 0)
    {
      sbuf_wait_cyc          += sbuf_buf[sbuf_tail];
      runtime.sim.mem_cycles += sbuf_buf[sbuf_tail];
      sbuf_prev_cycles       += sbuf_buf[sbuf_tail];
      sbuf_tail               = (sbuf_tail + 1) % MAX_SBUF_LEN;
      sbuf_count--;
    }
}	/* sbuf_load() */

#endif	/* !DYNAMIC_EXECUTION */


/*---------------------------------------------------------------------------*/
/*!Outputs dissasembled instruction                                          */
/*---------------------------------------------------------------------------*/
void orsim::dump_exe_log ()
{
  oraddr_t      insn_addr = cpu_state.iqueue.insn_addr;
  unsigned int  i;
  unsigned int  j;
  uorreg_t      operand;

  if (insn_addr == 0xffffffff)
    {
      return;
    }

  if ((myconfig.sim.exe_log_start <= runtime.cpu.instructions) &&
      ((myconfig.sim.exe_log_end <= 0) ||
       (runtime.cpu.instructions <= myconfig.sim.exe_log_end)))
    {
      struct label_entry *entry;

      if (myconfig.sim.exe_log_marker &&
	  !(runtime.cpu.instructions % myconfig.sim.exe_log_marker))
	{
	  fprintf (runtime.sim.fexe_log,
		   "--------------------- %8i instruction "
		   "---------------------\n",
		   runtime.cpu.instructions);
	}

      switch (myconfig.sim.exe_log_type)
	{
	case EXE_LOG_HARDWARE:
	  fprintf (runtime.sim.fexe_log,
		   "\nEXECUTED(%11u): %" PRIxADDR ":  ",
		   runtime.cpu.instructions, insn_addr);
	  fprintf (runtime.sim.fexe_log, "%.2x%.2x",
		   eval_direct8 (insn_addr, 0, 0),
		   eval_direct8 (insn_addr + 1, 0, 0));
	  fprintf (runtime.sim.fexe_log, "%.2x%.2x",
		   eval_direct8 (insn_addr + 2, 0, 0),
		   eval_direct8 (insn_addr + 3, 0, 0));

	  for (i = 0; i < MAX_GPRS; i++)
	    {
	      if (i % 4 == 0)
		{
		  fprintf (runtime.sim.fexe_log, "\n");
		}

	      fprintf (runtime.sim.fexe_log, "GPR%2u: %" PRIxREG "  ", i,
		       cpu_state.reg[i]);
	    }

	  fprintf (runtime.sim.fexe_log, "\n");
	  fprintf (runtime.sim.fexe_log, "SR   : %.8" PRIx32 "  ",
		   cpu_state.sprs[SPR_SR]);
	  fprintf (runtime.sim.fexe_log, "EPCR0: %" PRIxADDR "  ",
		   cpu_state.sprs[SPR_EPCR_BASE]);
	  fprintf (runtime.sim.fexe_log, "EEAR0: %" PRIxADDR "  ",
		   cpu_state.sprs[SPR_EEAR_BASE]);
	  fprintf (runtime.sim.fexe_log, "ESR0 : %.8" PRIx32 "\n",
		   cpu_state.sprs[SPR_ESR_BASE]);
	  break;

	case EXE_LOG_SIMPLE:
	case EXE_LOG_SOFTWARE:
	  disassemble_index (cpu_state.iqueue.insn,
			     cpu_state.iqueue.insn_index);

	  entry = get_label (insn_addr);
	  if (entry)
	    {
	      fprintf (runtime.sim.fexe_log, "%s:\n", entry->name);
	    }
	  
	  if (myconfig.sim.exe_log_type == EXE_LOG_SOFTWARE)
	    {
	      struct insn_op_struct *opd =
		op_start[cpu_state.iqueue.insn_index];

	      j = 0;
	      while (1)
		{
		  operand = eval_operand_val (cpu_state.iqueue.insn, opd);
		  while (!(opd->type & OPTYPE_OP))
		    {
		      opd++;
		    }
		  if (opd->type & OPTYPE_DIS)
		    {
		      fprintf (runtime.sim.fexe_log,
			       "EA =%" PRIxADDR " PA =%" PRIxADDR " ",
			       cpu_state.insn_ea,
			       peek_into_dtlb (cpu_state.insn_ea, 0, 0));
		      opd++;	/* Skip of register operand */
		      j++;
		    }
		  else if ((opd->type & OPTYPE_REG) && operand)
		    {
		      fprintf (runtime.sim.fexe_log, "r%-2i=%" PRIxREG " ",
			       (int) operand, evalsim_reg (operand));
		    }
		  else
		    {
		      fprintf (runtime.sim.fexe_log, "             ");
		    }
		  j++;
		  if (opd->type & OPTYPE_LAST)
		    {
		      break;
		    }
		  opd++;
		}
	      if (or32_opcodes[cpu_state.iqueue.insn_index].flags & OR32_R_FLAG)
		{
		  fprintf (runtime.sim.fexe_log, "SR =%" PRIxREG " ",
			   cpu_state.sprs[SPR_SR]);
		  j++;
		}
	      while (j < 3)
		{
		  fprintf (runtime.sim.fexe_log, "             ");
		  j++;
		}
	    }
	  fprintf (runtime.sim.fexe_log, "%" PRIxADDR " ", insn_addr);
	  fprintf (runtime.sim.fexe_log, "%s\n", disassembled);
	}
    }
}	/* dump_exe_log() */


/*---------------------------------------------------------------------------*/
/*!Dump registers

   Supports the CLI 'r' and 't' commands                                     */
/*---------------------------------------------------------------------------*/
void orsim::dumpreg(FILE *fd, bool fullset)
{
  dumpreg(); // for now
}

void orsim::dumpreg()
{
  int       i;
  oraddr_t  physical_pc;

  if ((physical_pc = peek_into_itlb (cpu_state.iqueue.insn_addr)))
    {
      disassemble_memory (physical_pc, physical_pc + 4, 0);
    }
  else
    {
      PRINTF ("INTERNAL SIMULATOR ERROR:\n");
      PRINTF ("no translation for currently executed instruction\n");
    }

  // generate_time_pretty (temp, runtime.sim.cycles * myconfig.sim.clkcycle_ps);
  PRINTF (" (executed) [cycle %lld, #%d]\n", runtime.sim.cycles,
	  runtime.cpu.instructions);
  if (myconfig.cpu.superscalar)
    {
      PRINTF ("Superscalar CYCLES: %u", runtime.cpu.supercycles);
    }
  if (myconfig.cpu.hazards)
    {
      PRINTF ("  HAZARDWAIT: %u\n", runtime.cpu.hazardwait);
    }
  else if (myconfig.cpu.superscalar)
    {
      PRINTF ("\n");
    }

  if ((physical_pc = peek_into_itlb (cpu_state.pc)))
    {
      disassemble_memory (physical_pc, physical_pc + 4, 0);
    }
  else
    {
      PRINTF ("%" PRIxADDR ": : xxxxxxxx  ITLB miss follows", cpu_state.pc);
    }

  PRINTF (" (next insn) %s", (cpu_state.delay_insn ? "(delay insn)" : ""));
  PRINTF ("PC= %" PRIxADDR "\n", cpu_state.pc);
  for (i = 0; i < MAX_GPRS; i++)
    {
      if (i % 4 == 0)
	{
	  PRINTF ("\n");
	}

      PRINTF ("GPR%.2u: %" PRIxREG "  ", i, evalsim_reg (i));
    }

  PRINTF ("flag: %u\n", cpu_state.sprs[SPR_SR] & SPR_SR_F ? 1 : 0);

}	/* dumpreg() */


/*---------------------------------------------------------------------------*/
/*!Wrapper around real decode_execute function

   Some statistics here only

   @param[in] current  Instruction being executed                            */
/*---------------------------------------------------------------------------*/
void orsim::decode_execute_wrapper (struct iqueue_entry *current)
{
  breakpoint = 0;

#ifndef HAVE_EXECUTION
#error HAVE_EXECUTION has to be defined in order to execute programs.
#endif

  decoder(current);

#if SET_OV_FLAG
  /* Check for range exception */
  if ((cpu_state.sprs[SPR_SR] & SPR_SR_OVE) &&
      (cpu_state.sprs[SPR_SR] & SPR_SR_OV))
    {
      except_handle (EXCEPT_RANGE, cpu_state.sprs[SPR_EEAR_BASE]);
    }
#endif

  if (breakpoint)
    {
      except_handle (EXCEPT_TRAP, cpu_state.sprs[SPR_EEAR_BASE]);
    }
}	/* decode_execute_wrapper() */

/*---------------------------------------------------------------------------*/
/*!Reset the CPU                                                             */
/*---------------------------------------------------------------------------*/
void orsim::core_reset()
{
  int               i;
  struct hist_exec *hist_exec_head = NULL;
  struct hist_exec *hist_exec_new;

  runtime.sim.cycles       = 0;
  runtime.sim.loadcycles   = 0;
  runtime.sim.storecycles  = 0;
  runtime.cpu.instructions = 0;
  runtime.cpu.delperiods   = 0;
  runtime.cpu.supercycles  = 0;
  runtime.cpu.hazardwait   = 0;
  //printf("Reset del periods\n");
  for (i = 0; i < MAX_GPRS; i++)
    {
      setsim_reg (i, 0);
    }

  memset (&cpu_state.iqueue,   0, sizeof (cpu_state.iqueue));
  memset (&cpu_state.icomplet, 0, sizeof (cpu_state.icomplet));

  sbuf_head        = 0;
  sbuf_tail        = 0;
  sbuf_count       = 0;
  sbuf_prev_cycles = 0;

  /* Initialise execution history circular buffer */
  for (i = 0; i < HISTEXEC_LEN; i++)
    {
      hist_exec_new = (hist_exec *) malloc (sizeof (struct hist_exec));

      if (!hist_exec_new)
	{
	  fprintf (stderr, "Out-of-memory\n");
	  exit (1);
	}

      if (!hist_exec_head)
	{
	  hist_exec_head = hist_exec_new;
	}
      else
	{
	  hist_exec_tail->next = hist_exec_new;
	}

      hist_exec_new->prev = hist_exec_tail;
      hist_exec_tail = hist_exec_new;
    }

  /* Make hist_exec_tail->next point to hist_exec_head */
  hist_exec_tail->next = hist_exec_head;
  hist_exec_head->prev = hist_exec_tail;

  /* MM1409: All progs should start at reset vector entry! This sorted out by
     setting the cpu_state.pc field below. Not clear this is very good code! */

  /* Patches suggested by Shinji Wakatsuki, so that the vector address takes
     notice of the Exception Prefix High bit of the Supervision register */
  pcnext = (cpu_state.sprs[SPR_SR] & SPR_SR_EPH ? 0xf0000000 : 0x00000000);

  if (myconfig.sim.verbose)
    {
      PRINTF ("Starting at 0x%" PRIxADDR "\n", pcnext);
    }

  cpu_state.pc  = pcnext;
  pcnext       += 4;

  if (myconfig.sim.verbose)
    {
      PRINTF ("0: Starting at 0x%" PRIxADDR "\n", pcnext);
    }


  /* MM1409: All programs should set their stack pointer!  */
#if !(DYNAMIC_EXECUTION)
  except_handle (EXCEPT_RESET, 0);
  update_pc ();
#endif

  except_pending = 0;

  if (myconfig.sim.verbose)
    {
      PRINTF ("1: Starting at 0x%" PRIxADDR "\n", cpu_state.pc);
    }


  cpu_state.pc   = cpu_state.sprs[SPR_SR] & SPR_SR_EPH ?
    0xf0000000 + EXCEPT_RESET : EXCEPT_RESET;

  if (myconfig.sim.verbose)
    {
      PRINTF ("2: Starting at 0x%" PRIxADDR "\n", cpu_state.pc);
    }


}	/* cpu_reset() */


/*---------------------------------------------------------------------------*/
/*!Simulates one CPU clock cycle
 
  @return  non-zero if a breakpoint is hit, zero otherwise.                  */
/*---------------------------------------------------------------------------*/
int orsim::step()
{
  except_pending  = 0;
  next_delay_insn = 0;

  if (fetch ())
    {
      PRINTF ("Breakpoint hit.\n");
      return  1; // Wrapper will halt and wait until debugger sends continue or reset.
    }

  if (except_pending)
    {
      update_pc ();
      except_pending = 0;
      return  0;
    }

  if (breakpoint) // not being used at the moment
    {
      except_handle (EXCEPT_TRAP, cpu_state.sprs[SPR_EEAR_BASE]);
      update_pc ();
      except_pending = 0;
      return  0;
    }

  decode_execute_wrapper (&cpu_state.iqueue);
  //  if (  cpu_state.iqueue.insn & 0xFFFF
  update_pc ();


  return  0;
}	/* cpu_clock() */


void orsim::hw_interrupt(const char *msg)
{
  if (cpu_state.sprs[SPR_SR] & SPR_SR_IEE_BIT) //  supervision_register bit2 must hold for an interrupt to be recognised
    except_handle (EXCEPT_INT, cpu_state.pc);
}


/*---------------------------------------------------------------------------*/
/*!If decoding cannot be found, call this function                           */
/*---------------------------------------------------------------------------*/
#if SIMPLE_EXECUTION
void orsim::l_invalid (struct iqueue_entry *current)
{
#else
void orsim::l_invalid ()
{
#endif
  except_handle (EXCEPT_ILLEGAL, cpu_state.iqueue.insn_addr);

}	/* l_invalid() */




#if COMPLEX_EXECUTION

/* Include generated/built in decode_execute function */
#define config myconfig
#define simprintf(X, Y) printf("simprintf %x %x\n", X, Y)
#include "iss.C"

#elif SIMPLE_EXECUTION


/*---------------------------------------------------------------------------*/
/*!Evaluates source operand

   Implementation specific.

   @param[in] op_no       The operand
   @param[in] insn_index  Address of the instruction
   @param[in] insn        The instruction

   @return  The value of the operand                                         */
/*---------------------------------------------------------------------------*/
static uorreg_t
eval_operand (int            op_no,
	      unsigned long  insn_index,
	      uint32_t       insn)
{
  struct insn_op_struct *opd = op_start[insn_index];
  uorreg_t               ret;

  while (op_no)
    {
      if (opd->type & OPTYPE_LAST)
	{
	  fprintf (stderr,
		   "Instruction requested more operands than it has\n");
	  exit (1);
	}

      if ((opd->type & OPTYPE_OP) && !(opd->type & OPTYPE_DIS))
	{
	  op_no--;
	}

      opd++;
    }

  if (opd->type & OPTYPE_DIS)
    {
      ret = eval_operand_val (insn, opd);

      while (!(opd->type & OPTYPE_OP))
	{
	  opd++;
	}

      opd++;
      ret               += evalsim_reg (eval_operand_val (insn, opd));
      cpu_state.insn_ea  = ret;

      return  ret;
    }

  if (opd->type & OPTYPE_REG)
    {
      return  evalsim_reg (eval_operand_val (insn, opd));
    }

  return  eval_operand_val (insn, opd);

}	/* eval_operand() */



/*---------------------------------------------------------------------------*/
/*!Set destination operand (register direct) with value.

   Implementation specific.

   @param[in] op_no       The operand
   @param[in] value       The value to set
   @param[in] insn_index  Address of the instruction
   @param[in] insn        The instruction                                    */
/*---------------------------------------------------------------------------*/
static void
set_operand (int            op_no,
	     orreg_t        value,
	     unsigned long  insn_index,
	     uint32_t       insn)
{
  struct insn_op_struct *opd = op_start[insn_index];

  while (op_no)
    {
      if (opd->type & OPTYPE_LAST)
	{
	  fprintf (stderr,
		   "Instruction requested more operands than it has\n");
	  exit (1);
	}

      if ((opd->type & OPTYPE_OP) && !(opd->type & OPTYPE_DIS))
	{
	  op_no--;
	}

      opd++;
    }

  if (!(opd->type & OPTYPE_REG))
    {
      fprintf (stderr, "Trying to set a non-register operand\n");
      exit (1);
    }

  setsim_reg (eval_operand_val (insn, opd), value);

}	/* set_operand() */


/*---------------------------------------------------------------------------*/
/*!Simple and rather slow decoding function

   Based on built automata.

   @param[in] current  The current instruction to execute                    */
/*---------------------------------------------------------------------------*/
static void
decode_execute (struct iqueue_entry *current)
{
  int insn_index;

  printf("Running ins cin\n");
  current->insn_index = insn_index = insn_decode (current->insn);

  if (insn_index < 0)
    {
      l_invalid (current);
    }
  else
    {
      or32_opcodes[insn_index].exec (current);
    }

  if (do_stats)
    analysis (&cpu_state.iqueue);
}





#include "insnset.c"


#elif defined(DYNAMIC_EXECUTION)



#else
# error "Must define SIMPLE_EXECUTION, COMPLEX_EXECUTION or DYNAMIC_EXECUTION"
#endif


extern struct dev_memarea *verify_memoryarea (oraddr_t addr)
{
  assert(0);

}


void orsim::l_none(struct iqueue_entry *current)
{
  assert(0);
}


void orsim::l_none(struct op_queue *opq, int *param_t, orreg_t *param, int delay_slot)
{
  assert(0);
}


void orsim::l_none (void)
{

  assert(0);
}



uorreg_t  orsim::mfspr (const uint16_t  regno) // uorreg_t &a)
{
  assert(regno >= 0 && regno < MAX_SPRS);
  return cpu_state.sprs[regno];
}


uorreg_t  orsim::mtspr (const uint16_t  regno, uorreg_t &a)
{
  assert(regno >= 0 && regno < MAX_SPRS);
  cpu_state.sprs[regno] = a;
  printf("Set spr %i with 0x%x\n", regno, a);
  return a;
}

void orsim::core_stat_report(const char *msg, FILE *fd, bool resetf)
{

  //fprintf(fd, "%s: superscalar instructions : %d\n", msg, runtime.cpu.supercycles);
  //fprintf(fd, "%s: hazardwait  instructions : %d\n", msg, runtime.cpu.hazardwait);
  fprintf(fd, "@reset :  insn #%d\n",	  runtime.cpu.reset_instructions);
  fprintf(fd, "delperiods     #" PFI64 "\n",	  runtime.cpu.delperiods);
  fprintf(fd, "@exit  :  insn #%d\n",   runtime.cpu.instructions);
  fprintf(fd, " diff  :  insn #%d\n",  runtime.cpu.instructions - runtime.cpu.reset_instructions);

  if (resetf) runtime.cpu.reset_instructions = 0;
  if (resetf) runtime.cpu.instructions = 0;
  if (resetf) runtime.cpu.delperiods = 0;
  if (resetf) runtime.cpu.supercycles = 0;
  if (resetf) runtime.cpu.hazardwait = 0;
}

void  orsim::set_stall_state (int state)
{
  assert(0);//unimplemented

}

void  orsim::bpb_update (oraddr_t addr, int taken)
{
  assert(0);
}

void  orsim::btic_update (oraddr_t targetaddr)
{
  //  assert(0);
}

struct dev_memarea *orsim::verify_memoryarea (oraddr_t addr)
{
  assert(0);

}

void orsim::except_handle  (oraddr_t except, oraddr_t ea)
{ 
  //printf("Core has an exception to  handle\n");
  oraddr_t except_vector;
  //  if (debug_ignore_exception (except))  return;

#if !(DYNAMIC_EXECUTION)
  /* In the dynamic recompiler, this function never returns, so this is not
   * needed.  Ofcourse we could set it anyway, but then all code that checks
   * this variable would break, since it is never reset */
  except_pending = 1;
#endif

  except_vector = except + (cpu_state.sprs[SPR_SR] & SPR_SR_EPH ? 0xf0000000 : 0x00000000);

  //printf("%s: except_handler %x\n", __FILE__, except_vector);

  // If we are hosting an OS we do not want this diverted to the backdoor! TODO add a control flag.
  if (except_vector == 0xC00) exit_backdoor("l.sys 0xC00 ");

#if !(DYNAMIC_EXECUTION)
  pcnext = except_vector;
  //  PRINTF ("pc next is in excepate handle 0x%" PRIxADDR "\n", pcnext);
#endif
  cpu_state.sprs[SPR_EEAR_BASE] = ea;
  cpu_state.sprs[SPR_ESR_BASE] = cpu_state.sprs[SPR_SR];

  cpu_state.sprs[SPR_SR] &= ~SPR_SR_OVE;	/* Disable overflow flag exception. */

  cpu_state.sprs[SPR_SR] |= SPR_SR_SM;	/* SUPV mode */
  cpu_state.sprs[SPR_SR] &= ~(SPR_SR_IEE | SPR_SR_TEE);	/* Disable interrupts. */

  /* Address translation is always disabled when starting exception. */
  cpu_state.sprs[SPR_SR] &= ~SPR_SR_DME;

#if DYNAMIC_EXECUTION
  /* If we were called from do_scheduler and there were more jobs scheduled to
   * run after this, they won't run unless the following call is made since this
   * function never returns.  (If we weren't called from do_scheduler, then the
   * job at the head of the queue will still have some time remaining) */
  if (scheduler.job_queue->time <= 0)
    do_scheduler ();
#endif

  switch (except)
    {
      /* EPCR is irrelevent */
    case EXCEPT_RESET:
      break;
      /* EPCR is loaded with address of instruction that caused the exception */
    case EXCEPT_ITLBMISS:
    case EXCEPT_IPF:
      cpu_state.sprs[SPR_EPCR_BASE] = ea - (cpu_state.delay_insn ? 4 : 0);
#if DYNAMIC_EXECUTION
      op_join_mem_cycles ();
#endif
      break;
    case EXCEPT_BUSERR:
    case EXCEPT_DPF:
    case EXCEPT_ALIGN:
    case EXCEPT_ILLEGAL:
    case EXCEPT_DTLBMISS:
    case EXCEPT_RANGE:
    case EXCEPT_TRAP:
      /* All these exceptions happen during a simulated instruction */
#if DYNAMIC_EXECUTION
      /* Since these exceptions happen during a simulated instruction and this
       * function jumps out to the exception vector the scheduler would never have
       * a chance to run, therefore run it now */
      run_sched_out_of_line ();
#endif
      cpu_state.sprs[SPR_EPCR_BASE] =
	cpu_state.pc - (cpu_state.delay_insn ? 4 : 0);
      break;
      /* EPCR is loaded with address of next not-yet-executed instruction */
    case EXCEPT_SYSCALL:
      cpu_state.sprs[SPR_EPCR_BASE] =
	(cpu_state.pc + 4) - (cpu_state.delay_insn ? 4 : 0);
      break;
      /* These exceptions happen AFTER (or before) an instruction has been
       * simulated, therefore the pc already points to the *next* instruction */
    case EXCEPT_TICK:
    case EXCEPT_INT:
      cpu_state.sprs[SPR_EPCR_BASE] =
	cpu_state.pc - (cpu_state.delay_insn ? 4 : 0);
#if !(DYNAMIC_EXECUTION)
      /* If we don't update the pc now, then it will only happen *after* the next
       * instruction (There would be serious problems if the next instruction just
       * happens to be a branch), when it should happen NOW. */
      cpu_state.pc = pcnext;
      pcnext += 4;
#endif
      break;
    }

  /* Address translation is here because run_sched_out_of_line calls
   * eval_insn_direct which checks out the immu for the address translation but
   * if it would be disabled above then there would be not much point... */
  cpu_state.sprs[SPR_SR] &= ~SPR_SR_IME;

  /* Complex/simple execution strictly don't need this because of the
   * next_delay_insn thingy but in the dynamic execution modell that doesn't
   * exist and thus cpu_state.delay_insn would stick in the exception handler
   * causeing grief if the first instruction of the exception handler is also in
   * the delay slot of the previous instruction */
  cpu_state.delay_insn = 0;

#if DYNAMIC_EXECUTION
  do_jump (except_vector);
#endif

};


//
//
//
void decode_execute(iqueue_entry* a)
{


}



extern void                disassemble_memory (oraddr_t  from,
					       oraddr_t  to,
					       int       nl)
{

}

 struct label_entry *orsim::get_label (oraddr_t addr)
{
  return 0;
}


void orsim::disassemble_memory(oraddr_t a, oraddr_t x, int d)
{


}


int orsim::disassemble_index(unsigned long a, int d)
{


}


oraddr_t orsim::peek_into_itlb (oraddr_t virtaddr) 
{
  //  printf("Consult itlb va=%x\n", virtaddr);
  return virtaddr;


};

oraddr_t orsim::peek_into_dtlb (oraddr_t virtaddr, int, int) 
{
  //  printf("Consult dtlb va=%x\n", virtaddr);
  return virtaddr;
}

bool orsim::has_breakpoint(oraddr_t virtaddr) 
{

  // The spr_defs seems to define the lsb of WGB (Watchpoint Generating Breakpoint) as bit 13 but our GDB implementation has it as bit 12.
  // We use bit 12 here too and ignore the value in spr_defs.

  if ((cpu_state.sprs[SPR_DMR1] & /*SPR_DMR1_ST*/0x004000000))
    {
      return 1;
      // single step break.
    }

  if ((cpu_state.sprs[SPR_DMR2] & 0xFFF000/*SPR_DMR2_WGB*/) == 0) return 0; // Fast path: if nothing enabled, return.
  int off;


  for (off = 0; off< 8; off ++)
    {
      if (virtaddr == 0x2890) printf("comparing %x  with %x dmr2=%x\n", virtaddr, cpu_state.sprs[SPR_DVR(off)], cpu_state.sprs[SPR_DMR2]);
      if (cpu_state.sprs[SPR_DVR(off)] == virtaddr && cpu_state.sprs[SPR_DMR2] & (0x1000 << off)) break;
    }


  if (off < 8) 
    {
      printf("Breakpoint reg %i hit\n", off);
      return 1;
    }
  return 0; 
}


void orsim::addsstats (int item, int cnt_dynamic) {};
void orsim::adddstats (int item1, int item2, int cnt_dynamic, int depend) {};
void orsim::addfstats (enum insn_type item1, enum insn_type item2,
		       int cnt_dynamic, int depend) {};



orsim::orsim(const char *sname): sname(sname) // constructor
{
  except_pending = 0;
  break_just_hit = 0;//DJG was a silly static
  //breakpoints = 0;
  pcnext = 0;
  next_delay_insn = 0;// DJG
  myconfig.sim.verbose = 0;

      /*! Num cycles waiting for stores to complete. Globally available */
      sbuf_wait_cyc = 0;
      
      /*! Number of total store cycles. Globally available */
      sbuf_total_cyc = 0;
      
      /*! Whether we are doing statistical analysis. Globally available */
      do_stats = 0;
      
      /*! History of execution. Globally available */
      hist_exec_tail = NULL;
      
      /* Benchmark multi issue execution. This class only */
      multissue[20];
      issued_per_cycle = 4;
      
      /* Store buffer analysis - stores are accumulated and commited when IO is
	 idle. This class only */
      sbuf_head              = 0;
      sbuf_tail              = 0;
      sbuf_count             = 0;
      
#if !(DYNAMIC_EXECUTION)
      for (int i =0; i< MAX_SBUF_LEN; i++) sbuf_buf[i] = 0;
#endif

      sbuf_prev_cycles = 0;
      new tenos_report_item(sname, "instructions", &runtime.cpu.instructions);
      new tenos_report_item(sname, "delperiods", &runtime.cpu.delperiods);
}


void orsim::log_delperiods(u64_t new_delperiods, u32_t addr)
{
  //printf("pc = 0x%x increment delperiods " PFI64 " by " PFI64 "\n", runtime.cpu.delperiods, new_delperiods);
  runtime.cpu.delperiods += new_delperiods;
}


#if 0
//Want to be really sure is inlined.
oraddr_t orsim::get_reg(int x) 
{
  return cpu_state.reg[x]; 
}
#endif

oraddr_t orsim::get_spr(int sprNum)
{ 

  #define SPR_GPR0 0x400 
  //#define SPR_NPC  0x010
  if (sprNum >= SPR_GPR0 && sprNum < SPR_GPR0 + MAX_GPRS)
    {
      //printf("Reading GRP via spr reg spr=$%x\n", sprNum);
      return get_reg(sprNum-SPR_GPR0);
    }
  if (sprNum == SPR_NPC)  return get_pc(); // or nextpc?
  return cpu_state.sprs[sprNum]; 

}
// eof


