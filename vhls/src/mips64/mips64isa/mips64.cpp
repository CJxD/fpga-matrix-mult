// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// $Id: mips64.cpp,v 1.33 2011/08/21 10:57:33 my294 Exp $// (C) 2009-10 D J Greaves
// University of Cambridge, Computer Laboratory.
// SoC D/M Classes - CBG Prazor  


//
// Simple MIPs processor in Tenos and SystemC
// tinymips_core.cpp
//

// MIPS32 memory map:
// 0000_0000 7FFF_FFFF    2 GB Mapped kuseg 
// 8000_0000 9FFF_FFFF   .5 GB Unapped kseg0 
// A000_0000 BFFF_FFFF   .5 GB Unapped kseg1 
// C000_0000 FFFF_FFFF   1 GB Maped kseg2 
//
//
//
//

#define ISSTRC(X) if (node->traceregions_ptr && *node->traceregions_ptr && (*node->traceregions_ptr)->check(pc, TENOS_TRACE_CPU_IDECODE))  { X; }



#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "mips64.h"

extern bool wait_debugger;

#ifdef NO_BRANCH_DELAY 
#define  BRANCH_DELAY_RLA_OFFSET 0
#else
#define  BRANCH_DELAY_RLA_OFFSET 4
#endif


void mips64::barrel::setpc(u64_t w32)
{
#ifdef NO_BRANCH_DELAY
  next_ins = 0x7C0; // Kill instruction in branch delay slot (force to Largest NOP instruction).
#endif
  next_pc = w32+0;
  int trc = (node->traceregions_ptr && *node->traceregions_ptr) ? (*node->traceregions_ptr)->check(next_pc, TENOS_TRACE_CPU_IDECODE|TENOS_TRACE_CPU_IFETCH) : 0;
  if (trc) { printf("[ pc := " PFX64 "+%i ]", w32, 0); }
}

void mips64::barrel::regfile_up(u5_t d, u64_t w32)
{
  if (d != 0)
    {
      ISSTRC(printf("[ r%i := " PFX64 " ]", d, w32));
      PRESENCECHECK(if (d!= 0 && d!=26 && d!=27) set_presence(d));
      regfile[d] = w32;
    }
}

void mips64::barrel::overflow_xnn()
{ ;
}

#define POS32(X) (((((unsigned)X)>>31)) == 0LL)
#define NEG32(X) (((((unsigned)X)>>31)) == 1LL)
#define POS64(X) (((((u64_t)X)>>63LL)) == 0LL)
#define NEG64(X) (((((u64_t)X)>>63LL)) == 1LL)


//
void mips64::core_reset(bool enable_first_to_run)
{
  for (int i=0; i<N_BARREL; i++)
    {
      barrels[i]->reset(enable_first_to_run);
      enable_first_to_run = false;
    }
  way = 0;
  if (traceregions_ptr && *traceregions_ptr)
    printf("Reset way %i pc to " PFX64 "\n", way, barrels[way]->pc-PC_CORRECT);
}


//
int mips64::cpu_mem_write(u64_t d, mips_addr_t adr, u4_t size)
{
  //ISSTRC(printf("[ Store adr=" PFX64 " wd=" PFX64 " w=%i]\n", adr, d, size*8));
  if (size == 8) set_mem64(adr, d, 0);
  else if (size == 4) set_mem32(adr, d, 0);
  else if (size == 2) set_mem16(adr, d, 0);
  else if (size == 1) set_mem8(adr, d, 0);
  else assert(0);
}

//
int mips64::cpu_mem_r64(mips_addr_t a, u64_t *dp)
{
  u64_t d;
  bool ok  = eval_mem64(a, d, 0);
  *dp = d;
  //ISSTRC(printf("Read d64 adr=" PFX64 "  rd=" PFX64 "\n", a, d));
  return 0; // ok return code
}

int mips64::cpu_mem_r32(mips_addr_t a, u64_t *dp, bool linked)
{
  u32_t d;
  bool ok  = eval_mem32(a, d, 0, linked);
  *dp = d;
  //ISSTRC(printf("Read d32 adr=" PFX64 "  rd= %08X\n", a, d));
  return 0; // ok return code
}

int mips64::cpu_mem_r16(mips_addr_t a, u64_t *dp)
{
  u16_t d;
  bool ok  = eval_mem16(a, d, 0);
  *dp = d;
  //ISSTRC(printf("Read d16 adr=" PFX64 "  rd=%04x\n", a, d));
  return 0; // ok return code
}

int mips64::cpu_mem_r8(mips_addr_t a, u64_t *dp)
{
  u8_t d;
  bool ok  = eval_mem8(a, d, 0);
  *dp = d;
  //ISSTRC(printf("Read d8 adr=" PFX64 "  rd=%02X\n", a, d));
  return 0; // ok return code
}

#ifdef MAMBA
#define C_CODE_SPACE (1LLU<<56)
#else
#define C_CODE_SPACE 0
#endif

// Instruction cache.
int mips64::barrel::cpu_mem_r32i(mips_addr_t a, u32_t *dp)
{
  const int cache_index = (a >> (ICACHE_LOGSIZE + 3LLU)) & (ICACHE_LINES - 1LLU);
  while ((a & ~7) != icache[cache_index].a)
    {
      icache[cache_index].a = a & ~7;
      u64_t dr;
      bool ok = node->eval_mem64(icache[cache_index].a  | C_CODE_SPACE, dr, 0);
#define ESSS64(X)  \
   (((X)>>56)&0x00000000000000FFLLU)|   \
   (((X)>>40)&0x000000000000FF00LLU)|   \
   (((X)>>24)&0x0000000000FF0000LLU)|   \
   (((X)>>8 )&0x00000000FF000000LLU)|   \
   (((X)<<8 )&0x000000FF00000000LLU)|   \
   (((X)<<24)&0x0000FF0000000000LLU)|   \
   (((X)<<40)&0x00FF000000000000LLU)|   \
     (((X)<<56)&0xFF00000000000000LLU)

      //if (node->eflip) dr = ESSS64(dr);
      icache[cache_index].d = dr;
      if (!ok)
	{
	  printf ("Instruction memory not present proc=%s/%i a=" PFX64 "\n", node->corename, spoke, icache[cache_index].a);
	  assert(0);
	}
#define TRCI(X)
      TRCI(printf("Fill icache idx=%i adr=" PFX64 "  rd=" PFX64 "\n", cache_index, icache[cache_index].a, icache[cache_index].d));
    }
#ifdef BIG_ENDIAN
  // Big endian: fetch high data when a[2] is zero.
  u32_t ins  = ((a & 4) ? icache[cache_index].d : icache[cache_index].d >>32) & 0xFFFFffff;
#else
  u32_t ins  = (!(a & 4) ? icache[cache_index].d : icache[cache_index].d >>32) & 0xFFFFffff;
#endif
  *dp = ins;
  //printf("ifetch return " PFX64 " ins=%08X\n", a, *dp); 
  return 0;
}


void mips64::ins_opcode(int f0, int ec, const char *a, int b)
{
//printf("\n >>>>> f0 %d, ec %d, <Normal>  etable[ec] %d, inst %s, opcode %d >>>>>\n", f0, ec, etable[ec], a, b);
  opcodes0[b].f0 = f0;
#ifdef TLM_POWER3
  opcodes0[b].energy = pw_energy(etable[ec], PW_nJ);
#endif
  opcodes0[b].opcode = a;
}

void mips64::ins_special(int f0, int ec, const char *a, int b)
{
  specials0[b].f0 = f0;
#ifdef TLM_POWER3
  specials0[b].energy = pw_energy(etable[ec], PW_nJ);
//printf("\n >>>>> f0 %d, ec %d, <Special> etable[ec] %d, inst %s, opcode %d >>>>>\n", f0, ec, etable[ec], a, b);
#endif
  specials0[b].opcode = a;
}


void mips64::disinit()
{
  ins_special(0, ENERGY_1, "SLL     ", 000);
  ins_special(0, ENERGY_UNDEF, "?01     ", 001);
  ins_special(0, ENERGY_1, "SRL     ", 002);
  ins_special(0, ENERGY_1, "SRA     ", 003);
  ins_special(0, ENERGY_1, "SLLV    ", 004);
  ins_special(0, ENERGY_UNDEF, "?05     ", 005);
  ins_special(0, ENERGY_1, "SRLV    ", 006);
  ins_special(0, ENERGY_1, "SRAV    ", 007);
  ins_special(JTYPE_R, ENERGY_2, "JR      ", 010);
  ins_special(JTYPE_R, ENERGY_2, "JALR    ", 011);
  ins_special(0, ENERGY_UNDEF, "MOVZ     ", 012);
  ins_special(0, ENERGY_UNDEF, "MOVN     ", 013);
  ins_special(0, ENERGY_2, "SYSCALL ", 014);
  ins_special(0, ENERGY_2, "BREAK   ", 015);
  ins_special(0, ENERGY_UNDEF, "?16     ", 016);
  ins_special(0, ENERGY_UNDEF, "?17     ", 017);
  ins_special(0, ENERGY_1, "MFHI    ", 020);
  ins_special(0, ENERGY_1, "MTHI    ", 021);
  ins_special(0, ENERGY_1, "MFLO    ", 022);
  ins_special(0, ENERGY_1, "MTLO    ", 023);
  ins_special(0, ENERGY_1, "DSLLV   ", 024);
  ins_special(0, ENERGY_2, "MYAF    ", 025);
  ins_special(0, ENERGY_UNDEF, "?26     ", 026);
  ins_special(0, ENERGY_UNDEF, "?27     ", 027);
  ins_special(0, ENERGY_4, "MULT    ", 030);
  ins_special(0, ENERGY_4, "MULTU   ", 031);
  ins_special(0, ENERGY_4, "DIV     ", 032);
  ins_special(0, ENERGY_4, "DIVU    ", 033);
  ins_special(0, ENERGY_UNDEF, "?34     ", 034);
  ins_special(0, ENERGY_UNDEF, "?35     ", 035);
  ins_special(0, ENERGY_UNDEF, "?36     ", 036);
  ins_special(0, ENERGY_UNDEF, "?37     ", 037);
  ins_special(0, ENERGY_2, "ADD     ", 040);
  ins_special(0, ENERGY_2, "ADDU    ", 041);
  ins_special(0, ENERGY_2, "SUB     ", 042);
  ins_special(0, ENERGY_2, "SUBU    ", 043);
  ins_special(0, ENERGY_2, "AND     ", 044);
  ins_special(0, ENERGY_2, "OR      ", 045);
  ins_special(0, ENERGY_2, "XOR     ", 046);
  ins_special(0, ENERGY_2, "NOR     ", 047);
  ins_special(0, ENERGY_2, "TAS     ", 050);
  ins_special(0, ENERGY_2, "TAC     ", 051);
  ins_special(0, ENERGY_2, "SLT     ", 052);
  ins_special(0, ENERGY_2, "SLTU    ", 053);
  ins_special(0, ENERGY_2, "DADDU   ", 055);
  ins_special(0, ENERGY_2, "DSUBU   ", 057);

  ins_special(0, ENERGY_2, "TEQ     ", 064);
  ins_special(0, ENERGY_2, "SWP     ", 067);

  ins_special(0, ENERGY_2, "DSLL    ", 070);
  ins_special(0, ENERGY_2, "DSRL    ", 072);
  ins_special(0, ENERGY_2, "DSLL32  ", 074);
  ins_special(0, ENERGY_2, "SDA/SWA ", 075);
  ins_special(0, ENERGY_2, "DSRL32 ", 076);
  ins_special(0, ENERGY_2, "DSRA32 ", 077);

  ins_opcode(0, ENERGY_2, "SPECIAL ", 000);
  ins_opcode(0, ENERGY_2, "BCOND   ", 001); 
  ins_opcode(JTYPE, ENERGY_2,    "J       ", 002);
  ins_opcode(JTYPE_R, ENERGY_2,  "JAL     ", 003);
  ins_opcode(JTYPE_2R, ENERGY_2, "BEQ     ", 004);
  ins_opcode(JTYPE_2R, ENERGY_2, "BNE     ", 005);
  ins_opcode(JTYPE_2R, ENERGY_2, "BLEZ    ", 006);
  ins_opcode(JTYPE_2R, ENERGY_2, "BGTZ    ", 007);
  ins_opcode(IMED, ENERGY_1, "ADDI    ", 010);
  ins_opcode(IMED, ENERGY_1, "ADDIU   ", 011);
  ins_opcode(IMED, ENERGY_1, "SLTI    ", 012);
  ins_opcode(IMED, ENERGY_1, "SLTIU   ", 013);
  ins_opcode(IMED, ENERGY_1, "ANDI    ", 014);
  ins_opcode(IMED, ENERGY_1, "ORI     ", 015);
  ins_opcode(IMED, ENERGY_1, "XORI    ", 016);
  ins_opcode(IMED, ENERGY_1, "LUI     ", 017);
  ins_opcode(0, ENERGY_2, "COP0    ", 020);
  ins_opcode(0, ENERGY_2, "COP1    ", 021);
  ins_opcode(0, ENERGY_2, "COP2    ", 022);
  ins_opcode(0, ENERGY_2, "COP3    ", 023);
  ins_opcode(JTYPE_2R, ENERGY_2, "BEQL    ", 024);
  ins_opcode(0, ENERGY_UNDEF, "?25     ", 025);
  ins_opcode(0, ENERGY_UNDEF, "?26     ", 026);
  ins_opcode(0, ENERGY_UNDEF, "?27     ", 027);
  ins_opcode(0, ENERGY_1, "DADDI   ", 030);
  ins_opcode(0, ENERGY_1, "DADDIU  ", 031);
  ins_opcode(0, ENERGY_UNDEF, "?32     ", 032);
  ins_opcode(0, ENERGY_UNDEF, "?33     ", 033);
  ins_opcode(0, ENERGY_UNDEF, "?34     ", 034);
  ins_opcode(0, ENERGY_UNDEF, "?35     ", 035);
  ins_opcode(0, ENERGY_UNDEF, "?36     ", 036);
  ins_opcode(0, ENERGY_UNDEF, "?37     ", 037);
  ins_opcode(IDXD, 2, "LB      ", 040);
  ins_opcode(IDXD, 2, "LH      ", 041);
  ins_opcode(IDXD, 2, "LWL     ", 042);
  ins_opcode(IDXD, 2, "LW      ", 043);
  ins_opcode(IDXD, 2, "LBU     ", 044);
  ins_opcode(IDXD, 2, "LHU     ", 045);
  ins_opcode(IDXD, 2, "LWR     ", 046);
  ins_opcode(0, ENERGY_UNDEF, "?       ", 047 );
  ins_opcode(IDXD, 2, "SB      ", 050);
  ins_opcode(IDXD, 2, "SH      ", 051);
  ins_opcode(IDXD, 2, "SWL     ", 052);
  ins_opcode(IDXD, 2, "SW      ", 053);
  ins_opcode(0, ENERGY_UNDEF, "?54     ", 054);
  ins_opcode(0, ENERGY_UNDEF, "?55     ", 055);
  ins_opcode(IDXD, 2, "SWR     ", 056);
  ins_opcode(0, ENERGY_UNDEF, "?57     ", 057);
  ins_opcode(IDXD, 2, "LL      ", 060);
  ins_opcode(IDXD, 2, "LWC1    ", 061);
  ins_opcode(IDXD, 2, "LWC2    ", 062);
  ins_opcode(IDXD, 2, "LWC3    ", 063);
  ins_opcode(0, ENERGY_UNDEF, "?64     ", 064);
  ins_opcode(0, ENERGY_UNDEF, "?65     ", 065);
  ins_opcode(0, ENERGY_UNDEF, "?66     ", 066);
  ins_opcode(0, ENERGY_2, "LD      ", 067);
  ins_opcode(IDXD, 0, "SC      ", 070);
  ins_opcode(IDXD, 0, "SWC1    ", 071);
  ins_opcode(IDXD, 0, "SWC2    ", 072);
  ins_opcode(IDXD, 0, "SWC3    ", 073);
  ins_opcode(0, ENERGY_UNDEF, "?74     ", 074);
  ins_opcode(0, ENERGY_UNDEF, "?75     ", 075);
  ins_opcode(0, ENERGY_UNDEF, "?76     ", 076);
  ins_opcode(0, ENERGY_2, "SD      ", 077);
}


void mipscore_fatal(const char *s)
{
  printf("%s", s);
  exit(1);
}




// these sex routines could be more efficient as signed lshift followed by rshift.
u64_t sex16(u16_t a)
{
  return (a & 0x8000) ? 0xFFFFffffFFFF0000LL | a : a & 0x7FFFLLU;
}

u64_t sex16to32(u16_t a)
{
  return (a & 0x8000) ? 0xFFFF0000LL | a : a & 0x7fffFFFFLLU;
}

u64_t sex24(u24_t a)
{
  return (a & 0x80) ? 0xFFFFffffFFFFFF00LL | a : a;
}

u64_t sex32(u32_t a)
{
  return (a & 0x80000000) ? 0xFFFFffff00000000LL | a : a & 0x7fffFFFFLLU;
}

void mips64::barrel::fatal(const char *s)
{
  dumpregs(stdout, 1);
  mipscore_fatal(s);
}


void mips64::barrel::dumpregs(FILE *fd, bool fullregs, int cmd)
{
  MIPS64_COUNT_INSTRUCTIONS(if (cmd > 0)  printf("Instructions executed %i = %i\n", spoke, instruction_count));
  if (fd) fprintf(fd, "<pc=" PFX64 " r1=" PFX64 " r16/t9=" PFX64 " r2=" PFX64 " r3=" PFX64 " r4=" PFX64 " gp=" PFX64 " sp=" PFX64 " r30=" PFX64 " rla=" PFX64 ">\n", pc, regfile[1], regfile[16], regfile[2], regfile[3], regfile[4], regfile[28], regfile[29], regfile[30], regfile[31]); 
}


void mips64::barrel::reset(bool enable_to_run)
{
  write_back_flag = 0;
  write_back_flag1 = 0;
  next_ins = 0;
  for (int i =0; i<ICACHE_LINES; i++)  icache[i].a = 1; 
  setpc(node->reset_vector);
  PRESENCECHECK(presencebits = (enable_to_run) ? 0xffffFFFF: 0);
  PRESENCECHECK(m_ready = enable_to_run);
  PRESENCECHECK(printf("Spoke %i reset presence %x\n",spoke, presencebits));
}


void mips64::barrel::regfile_up_xflow64(u5_t d, char op, u64_t a0, u64_t a1)
{
  u64_t v64 = (d=='-') ? a0-a1: a0+a1;
  if (d != 0)
    {
      ISSTRC(printf("[ r%i := " PFX64 " ]", d, v64));
      regfile[d] = v64;
    }
  if (
      (d=='+' && POS64(a0) && POS64(a1) && NEG64(v64)) ||
      (d=='+' && NEG64(a0) && NEG64(a1) && POS64(v64)) ||
      (d=='-' && POS64(a0) && NEG64(a1) && NEG64(v64)) ||
      (d=='-' && NEG64(a0) && POS64(a1) && POS64(v64))) overflow_xnn();
}

void mips64::barrel::regfile_up_xflow32(u5_t d, char op, u64_t a0, u64_t a1)
{
  u32_t w32 = (d=='-') ? a0-a1: a0+a1;
  if (d != 0)
    {
      ISSTRC(printf("[ r%i := 0x%08X ]", d, w32));
      regfile[d] = sex32(w32);
    }
  if (
      (d=='+' && POS32(a0) && POS32(a1) && NEG32(w32)) ||
      (d=='+' && NEG32(a0) && NEG32(a1) && POS32(w32)) ||
      (d=='-' && POS32(a0) && NEG32(a1) && NEG32(w32)) ||
      (d=='-' && NEG32(a0) && POS32(a1) && POS32(w32))) overflow_xnn();
}


// Return 0 if ready to be called again (not halted (eg on missing presence bits)).
int mips64::barrel::step()
{
  //  assert(!regfile[0]);
  if (wait_debugger) return 1;

  u32_t ins = 0;
  u64_t tmp_1, tmp64;


  u64_t trace_pc = pc;



  if (!stalled)
    {
      MIPS64_COUNT_INSTRUCTIONS(instruction_count += 1);
      pc = next_pc;
      ins = next_ins;

      if (pc)
	{
	  cpu_mem_r32i(pc, &next_ins);
	}
      else next_ins = 0; // a nop

      if (skip_next > 0)
	{
	  skip_next -= 1;
	  next_ins = 0; // nullify current instruction
	}

      next_pc = pc + 4;

      // This write_back pipeline implements the load delay slot, that does not occur naturally
      // with anything other than the most naive processor design.
      if (write_back_flag1)
	{
	  write_back_flag1 = 0;
	  if (write_back_reg) // Cannot write to register 0
	    {
	      regfile[write_back_reg1] = write_back_data1;
	      ISSTRC(printf("Write back r%i := " PFX64 "\n", write_back_reg1, write_back_data1));
	    }
	}
      write_back_flag1 = write_back_flag;
      write_back_data1 = write_back_data;
      write_back_reg1 = write_back_reg;
      write_back_flag = 0;
    }
  else stalled = 0;

  u8_t opcode = ins >> 26;
  u8_t scode = ins & 0x3F; 
  u5_t rs = (ins >> 21) & 31; // aka base register for indexed
  u5_t rd = (ins >> 11) & 31;
  u5_t rex = (ins >> 6) & 31;
  u5_t func = (ins >> 0) & 31;
  u5_t rt = (ins >> 16) & 31;
  u64_t target = ins  & 0x3FFFFFF;
  u16_t imm = ins & 0xFFFF;
  ISSTRC(printf("%s,%i " PFX64 " RUNNING INS=%08X\n", node->corename, spoke, trace_pc, ins));

  u32_t f0, f1; const char *ops;
  if (opcode == 0)
    {
      f0 = node->specials0[scode].f0;
      POWER3(node->energy_event(node->specials0[scode].energy));
      ops = 
	(ins == 0x7C0) ? "(delay slot)": 
	(ins == 0) ? "nop": 
	node->specials0[scode].opcode;
    }
  else
    {
      f0 = node->opcodes0[opcode].f0;
      POWER3(node->energy_event(node->opcodes0[scode].energy));
      ops =  node->opcodes0[opcode].opcode;
    }


  ISSTRC(printf("   %o/%o, %s ", opcode, scode, ops));

  if (opcode==0 && scode==0 && rd==0) // some backdoors...
    {
      //printf(" nop%i %x", imm, ins);
      switch (ins)
	{
	  // Exit from the simulation (this core will enter power-down mode from now on).
	case 0x40:
          {
	    node->sim_done();
	    break;
          }

	  // Output a char to the console.
	case  0x80: // A more-full simulation is write to the UART using a device driver.
	  { bool c = 0;
	    ISSTRC(c=1; printf("Putchar backdoor '%c'\n", (char)(regfile[4])));
	    if (!c) { putchar(regfile[4]); fflush(stdout); }
	  }
	  break;
	  // Output a char to the console.

	case  0xC0: // Display R4 in hex
	  { printf("Backdoor R4:" PFX64, regfile[4]);
	    if (0) for (int i=0;i<32;i++)
	      {
		printf(PFX64 " ", regfile[i]);
		if ((i&3)==3) printf("\n"); 
	      }
	  }
	  break;
	}
    }
  else if (f0 & IMED)
  // I type
  {
    ISSTRC(printf("r%i,r%i,=0x%x", rs, rt, imm));
    //    PRESENCECHECK(if (!presreg(rs) || !presreg(rt)) return 0);
  }
  else if (f0 & JTYPE_R)  
  {
    u64_t  a = (pc & 0xFFFFffffF0000000LLU) | (target << 2);
    ISSTRC(printf(" r%i, 0x" PFX64 "", rs, a));
  }
  else if (f0 & JTYPE_2R)  
  {
    u64_t  a = (pc & 0xFFFFffffF0000000LLU) | (target << 2);
    ISSTRC(printf(" r%i,r%i 0x" PFX64 "", rs, rt, a));
  }
  else if (f0 & JTYPE)
  {
    u64_t a = pc + (sex16(imm)<<2);
    ISSTRC(printf(" 0x%llx", a - 0LL));
  }

  else if (f0 & IDXD)
  //
  {
    ISSTRC(printf("r%i,r%i[%li]", rt, rs, sex16(imm)));
    //PRESENCECHECK(if (!presreg(rs)) printf("Reg %i not present\n", rs));
    //    PRESENCECHECK(if (!presreg(rs)) return 0);
  }
  else
  // R type .. do all of them need to be present always ?
  {
    ISSTRC(printf("r%i,r%i,r%i,r%i %i", rs, rt, rd, rex, func));
    //    PRESENCECHECK(if (!presreg(rs) || !presreg(rt)) return 0);
  }


  if (opcode == 0) switch(scode)
    {
      
    case 0: /* SLL */
      PRESENCECHECK(if (!checkpres(rs)) { unready(rs); return 1; })
	PRESENCECHECK(if (!checkpres(rt)) { unready(rt); return 1; })
      regfile_up(rd, ((u64_t)regfile[rt]) << ((ins>>6) & 31));
      break;


    case 0x2: /* SRL */
      PRESENCECHECK(if (!checkpres(rs)) { unready(rs); return 1; })
      PRESENCECHECK(if (!checkpres(rt)) { unready(rt); return 1; })
      regfile_up(rd, ((u64_t)regfile[rt]) >> ((ins>>6) & 31));
      break;

    case 0x3: /* SRA */
      PRESENCECHECK(if (!checkpres(rs)) { unready(rs); return 1; })
      PRESENCECHECK(if (!checkpres(rt)) { unready(rt); return 1; })
      regfile_up(rd, ((int64_t)regfile[rt]) >> ((ins>>6) & 31));
      break;

    case 0x4: /* SLLV */
      PRESENCECHECK(if (!checkpres(rs)) { unready(rs); return 1; })
      PRESENCECHECK(if (!checkpres(rt)) { unready(rt); return 1; })
      regfile_up(rd, ((u64_t)regfile[rt]) << ((regfile[rs]) & 31));
      break;

    case 0x6: /* SRLV */
      PRESENCECHECK(if (!checkpres(rs)) { unready(rs); return 1; })
      PRESENCECHECK(if (!checkpres(rt)) { unready(rt); return 1; })
      regfile_up(rd, ((u64_t)regfile[rt]) >> ((regfile[rs]) & 31));
      break;

    case 0x7: /* SRAV */
      PRESENCECHECK(if (!checkpres(rs)) { unready(rs); return 1; })
      PRESENCECHECK(if (!checkpres(rt)) { unready(rt); return 1; })
      regfile_up(rd, ((int64_t)regfile[rt]) >> ((regfile[rs]) & 31));
      break;


    case 010: /* JR */
      setpc(regfile[rs]);
      break;

    case 011: /* JALR */
      tmp64 = regfile[rs];
      regfile_up(rd, pc + BRANCH_DELAY_RLA_OFFSET);  // Must return to instruction AFTER the one in the branch delay slot
      setpc(tmp64);
      break;

    case 012: /* MOVZ */
      // if GPR[rt] == 0, GPR[rd] <- GPR[rs]
      if (regfile[rt] == 0)  regfile_up(rd, regfile[rs]);
      break;

    case 013: /* MOVN */
      // if GPR[rt] != 0, GPR[rd] <- GPR[rs]
      if (regfile[rt] != 0)  regfile_up(rd, regfile[rs]);
      break;

    case 014: /* Syscall */
      EPC = pc; setpc(EXCEPTION_VECTOR);
      break;

    case 015: /* Break */
      EPC = pc;  setpc(EXCEPTION_VECTOR);
      break;

    case 020: /* MFHI - move from hi */
      regfile_up(rd, multhi);
      break;

    case 021: /* MTHI */
      multhi = regfile[rs];
      PRESENCECHECK(set_presence(27));
      break;

    case 022: /* MFLO move from lo. */
      regfile_up(rd, multlo);
      break;


    case 023: /* MTLO move to lo*/
      multlo = regfile[rs];
      PRESENCECHECK(set_presence(26));
      break;

    case 024: // DSLLV Doubleword Shift Left Logical Variable
      { u64_t r = regfile[rt] << (regfile[rs] & 63LLU);
	regfile_up(rd, r);
	break;
      }

    case 025:// MYAF
      // Return activation frame address
      {
	// 00041815 = myaf v1,a0 = myaf r3,r4
	u64_t k = 0;
	PRESENCECHECK(k = node->framebase + (u64_t)spoke * (u64_t)procMemSpacing);
	int d = 0; // sex16(imm);
	k += 8 * rt + d;
	printf(" [ MYAF r%i,%i(r%i); r%i = 0x" PFX64 "]\n", rd, d, rt, rd, k);
        regfile_up(rd, k);
      }
      
#define ARITHTRC(X)       
    case 030: // MULT - allowed to take several cycles on real cpu  There is also a special2 instruction called MUL!
      {
      PRESENCECHECK(if (!checkpres(rs)) { unready(rs); return 1; })
      PRESENCECHECK(if (!checkpres(rt)) { unready(rt); return 1; })
	int64_t a = (int64_t)(regfile[rs]);  // Should only take 32 bits from each reg
	int64_t b = (int64_t)(regfile[rt]);
	int64_t s = a * b;
	ARITHTRC(printf("MULT Product %lx %lx -> %lx\n", a, b, s));
	multhi = sex32(s >> 32LU);
	multlo = sex32(s & 0xFFFFffff);
	PRESENCECHECK(set_presence(26); 	set_presence(27));
	break;
      }
      
    case 031: // MULTU - allowed to take several cycles on real cpu. 
      {
	uint64_t a = (u64_t)(regfile[rs]); // Should only take 32 bits from each reg
	uint64_t b = (u64_t)(regfile[rt]);
	uint64_t s = a * b;
	ARITHTRC(printf("MULTU Product %lx %lx -> %lx\n", a, b, s));
	multhi = sex32(s >> 32LU);
	multlo = sex32(s & 0xFFFFffff);
	PRESENCECHECK(set_presence(26); 	set_presence(27));
	break;
      }

    case 032: /* DIV - allowed to take several cycles on real cpu */
      {
      PRESENCECHECK(if (!checkpres(rs)) { unready(rs); return 1; })
      PRESENCECHECK(if (!checkpres(rt)) { unready(rt); return 1; })
	int32_t a = (int32_t)(regfile[rs]); // Just a 32 bit divide, even on MIPS64.
	int32_t b = (int32_t)(regfile[rt]);
	int32_t s = a / b;
	int32_t q = a % b;
	ARITHTRC(printf("DIV %x / %x -> %x  q=%x\n", a, b, s, q));
	multhi = sex32(q);
	multlo = sex32(s);
	PRESENCECHECK(set_presence(26); 	set_presence(27));
	break;
      }
      
    case 033: /* DIVU - allowed to take several cycles on real cpu */
      {
      PRESENCECHECK(if (!checkpres(rs)) { unready(rs); return 1; })
      PRESENCECHECK(if (!checkpres(rt)) { unready(rt); return 1; })
	uint32_t a = (int32_t)(regfile[rs]);
	uint32_t b = (int32_t)(regfile[rt]);
	uint32_t s = a / b;
	uint32_t q = a % b;
	ARITHTRC(printf("DIVU %x / %x -> %x  q=%x\n", a, b, s, q));
	multhi = sex32(q);
	multlo = sex32(s);
	PRESENCECHECK(set_presence(26); 	set_presence(27));
	break;
      }
      

    case 040: /* ADD - with overflow*/
      PRESENCECHECK(if (!checkpres(rs)) { unready(rs); return 1; })
      PRESENCECHECK(if (!checkpres(rt)) { unready(rt); return 1; })
      regfile_up_xflow32(rd, '+', regfile[rs], regfile[rt]);
      break;

    case 041: /* ADDU */
      PRESENCECHECK(if (!checkpres(rs)) { unready(rs); return 1; })
      PRESENCECHECK(if (!checkpres(rt)) { unready(rt); return 1; })
      regfile_up(rd, regfile[rs] + regfile[rt]);
      break;

    case 042: /* SUB */
      PRESENCECHECK(if (!checkpres(rs)) { unready(rs); return 1; })
      PRESENCECHECK(if (!checkpres(rt)) { unready(rt); return 1; })
      regfile_up_xflow32(rd, '-', regfile[rs], regfile[rt]);
      break;

    case 043: /* SUBU */
      PRESENCECHECK(if (!checkpres(rs)) { unready(rs); return 1; })
      PRESENCECHECK(if (!checkpres(rt)) { unready(rt); return 1; })
      regfile_up(rd, sex32(regfile[rs] - regfile[rt]));
      break;

    case 044: /* AND */
      PRESENCECHECK(if (!checkpres(rs)) { unready(rs); return 1; })
      PRESENCECHECK(if (!checkpres(rt)) { unready(rt); return 1; })
      regfile_up(rd, regfile[rs] & regfile[rt]);
      break;

    case 045: /* OR */
      PRESENCECHECK(if (!checkpres(rs)) { unready(rs); return 1; })
      PRESENCECHECK(if (!checkpres(rt)) { unready(rt); return 1; })
      regfile_up(rd, regfile[rs] | regfile[rt]);
      break;

    case 046: /* XOR */
      PRESENCECHECK(if (!checkpres(rs)) { unready(rs); return 1; })
      PRESENCECHECK(if (!checkpres(rt)) { unready(rt); return 1; })
      regfile_up(rd, regfile[rs] ^ regfile[rt]);
      break;

    case 047: /* NOR */
      PRESENCECHECK(if (!checkpres(rs)) { unready(rs); return 1; })
      PRESENCECHECK(if (!checkpres(rt)) { unready(rt); return 1; })
      regfile_up(rd, ~(regfile[rs] | regfile[rt]));
      break;


    case 050: // TAS
      // Mamba special: load presence of address=rs and set presence flag
      {
	int64_t ea = regfile[rs];
	bool ok = node->eval_mem64(ea, tmp64, 0, true, true);
	//regfile_up(rd, tmp64);
	write_back_reg = rt;
	write_back_data = tmp64;
	write_back_flag = 1;
	printf("Mamba TAS ea=" PFX64 "  ov=" PFX64 "\n", ea, tmp64);
      }
      break;

    case 051: // TAC
      {
	// Mamba special: load presence of address=rs and clear presence flag
	u64_t ea = regfile[rs];
	printf("TAC rt=r%i ea = " PFX64 "\n", rt, ea);
	bool ok = node->eval_mem64(ea, tmp64, 0, true, false);
	write_back_reg = rt;
	write_back_data = tmp64;
	printf("TAC done: rt=r%i := " PFX64 " ea = " PFX64 "\n", rt, tmp64, ea);
	write_back_flag = 1;
      }
      break;


    case 052: /* SLT */
      regfile_up(rd, ((int64_t)regfile[rs]) < ((int64_t)regfile[rt]));
      break;

    case 053: /* SLTU */
      regfile_up(rd, ((u64_t)regfile[rs]) < ((u64_t)regfile[rt]));
      break;

    case 055:// DADDU
      //The 64-bit doubleword value in GPR rt is added to the 64-bit value in GPR rs to produce a 64-bit result. If the addi-
      //tion results in 64-bit s complement arithmetic overflow, then the destination register is not modified and an Integer
      //Overflow exception occurs. If it does not overflow, the 64-bit result is placed into GPR rd.
      regfile_up_xflow64(rd, '+', regfile[rs],  regfile[rt]);
      break;

    case 057:// DSUBU
      //The 64-bit doubleword value in GPR rt is added to the 64-bit value in GPR rs to produce a 64-bit result. If the addi-
      //tion results in 64-bit s complement arithmetic overflow, then the destination register is not modified and an Integer
      //Overflow exception occurs. If it does not overflow, the 64-bit result is placed into GPR rd.
      regfile_up_xflow64(rd, '-', regfile[rs],  regfile[rt]);
      break;

    case 064: // TEQ trap if equal
      {
	int trp =  ((u64_t)regfile[rs]) == ((u64_t)regfile[rt]);
	if (trp)
	  { printf("TEQ exception");
	    exit(1);
	  }
	break;
      }
    case 067: // SWP
      // Mamba : store word of presence flags
      {
	int64_t ea = regfile[rs];
	tmp64 = regfile[rt];
//	printf("swp ea=" PFX64 "  (ea)=rs=r%i, rt=(value to store)=r%i = " PFX64 "\n", ea, rs, rt, tmp64);
	node->set_mem64(ea, tmp64, 0, true);
      }
      break;

    case 070: // DSLL
      // To execute a left-shift of a doubleword by a fixed amount—0 to 31 bits
      regfile_up(rd, regfile[rt] << (0LLU+rex));
      break;

    case 072: // DSRL
      // To execute a right-shift of a doubleword by a fixed amount—0 to 31 bits
      regfile_up(rd, regfile[rt] >> (0LLU+rex));
      break;

    case 074: // DSLL32 To execute a left-shift of a doubleword by an amount fixed 32 to 63 bits
      // The 64-bit doubleword contents of GPR rt are shifted left, inserting zeros into the emptied bits; the result is placed in
      // GPR rd. The bit-shift amount in the range 0 to 31 is specified by sa.
      regfile_up(rd, regfile[rt] << (32LLU+rex));
      break;

    case 075: // SWA/SDA 
      // Mamba : store a single word and also set the presence flag enabling execution if now all set.
      {
	int64_t ea = regfile[rs];
	tmp64 = regfile[rt];
//	printf("swp ea=" PFX64 "  (ea)=rs=r%i, rt=(value to store)=r%i = " PFX64 "\n", ea, rs, rt, tmp64);
	node->cpu_mem_write(tmp64, ea, 8);
      }
      break;

   case 076: // DSRL32 To execute a logical right-shift of a doubleword by a fixed amount—32 to 63 bits
      // The 64-bit doubleword contents of GPR rt are shifted right, inserting zeros into the emptied bits; the result is placed
      // in GPR rd. The bit-shift amount in the range 32 to 63 is specified by sa+32.
     
     regfile_up(rd, regfile[rt] >> (32LLU+rex));
      break;

   case 077: // DSRA32 To execute an arithmetic right-shift of a doubleword by a fixed amount—32 to 63 bits
      // The doubleword contents of GPR rt are shifted right, duplicating the sign bit (63) into the emptied bits; the result is
      // placed in GPR rd. The bit-shift amount in the range 32 to 63 is specified by sa+32.

     regfile_up(rd, (long long int)(regfile[rt]) >> (32LLU+rex));
      break;

    default:
      fprintf(stderr, "core %s,%i, ins=%08X unimplemented scode 0%o\n", node->corename, spoke, ins, scode);
      fatal("unimplemented scode\n");
      
    }
  
  else switch(opcode)
    {
    case 001: /* BCOND */
      tmp64 = 0;
      if (rt == BLTZ) { if(regfile[rs] & (1LLU << 63)) tmp64 = 1; }
      else if (rt == BLTZAL) { regfile[31] = pc+8; if(regfile[rs] & (1LLU << 63)) tmp64 = 1; }
      else if (rt == BGEZ) { if(!(regfile[rs] & (1LLU << 63))) tmp64 = 1; }
      else if (rt == BGEZAL) { regfile[31] = pc+8; if(!(regfile[rs] & (1LLU << 63))) tmp64 = 1; }
       if (tmp64) pc += sex16(imm) << 2;
      break;

    case 002: /* J */
      setpc((pc & 0xFFFFffffF0000000LLU) | (target << 2));
      break;

    case 003: /* JAL */
      tmp64 = (pc & 0xFFFFffffF0000000LLU) | (target << 2);
      ISSTRC(printf("%i MIPS JAL " PFX64 "\n", spoke, tmp64));
      regfile_up(31, pc+BRANCH_DELAY_RLA_OFFSET);   // Must return to instruction AFTER the one in the branch delay slot
      setpc(tmp64);
      break;

    case 004: /* BEQ */
      {
	int d = sex16(imm);
	// Mamba: if branch to self then get descheduled.
	PRESENCECHECK(if (d == -1) presencebits = 0);
	if (regfile[rs] == regfile[rt])  setpc(pc + (d<<2));
      } 
      break;
      
    case 005: /* bne */
      //printf(" imm=%x  dex=" PFX64 "\n", imm, sex16(imm)<<2);
      if (regfile[rs] != regfile[rt]) setpc(pc + (sex16(imm)<<2));
      break;
      
    case 006: /* BLEZ */
      if (NEG64(regfile[rs]) || regfile[rs]==0) setpc(pc + (sex16(imm)<<2));
      break;

    case 007: /* BGTZ */
      if (POS64(regfile[rs]) && regfile[rs]!=0) setpc(pc + (sex16(imm)<<2));
      break;


    case 010: /* ADDI with overflow exn */
      regfile_up_xflow32(rt, '+', regfile[rs],  sex16to32(imm));
      break;

    case 011: /* ADDIU  32 without overflow exn */
      regfile_up(rt, sex32(regfile[rs] + sex16to32(imm)));
      break;

    case 012: /* SLTI set on lt immediate */
      tmp64 = ((int64_t)(regfile[rs])) < ((int64_t)(sex16(imm)));
      regfile_up(rt, tmp64);
      break;


    case 013: /* SLTIU set on lt immediate unsigned */
      tmp64 = ((u64_t)(regfile[rs])) < ((u64_t)(sex16(imm)));
      regfile_up(rt, tmp64);
      break;

    case 014: /* ANDI */
      regfile_up(rt, regfile[rs] & imm);
      break;


    case 015: /* ORI */
      regfile_up(rt, regfile[rs] | imm);
      break;

    case 016: /* XORI */
      regfile_up(rt, regfile[rs] ^ imm);
      break;

    case 017: /* LUI */
      regfile_up(rt,  imm << 16);
      break;

    case 020:  /* COP0 */
      {
	u3_t sel = ins & 7;
	u5_t mf = rs;
      switch (mf)
	{
	case 000:
	case 001: // DMFC0
	  regfile_up(rt, coprocessor_read(0, rd, sel));
	  break;

	case 004:
	case 005: // DMTC0 To move a doubleword from a GPR to a coprocessor 0 register.
	  coprocessor_write(0, rd, sel, regfile[rt]);
	  break;
	default:
	  printf("bad mf 0x%x\n", mf);
	  fatal("unimplemented COP0 opcode\n");
	}
      }

    case 024: // BEQL - branch likely - delay slot instruction predicated on this one.
      {
	int d = sex16(imm);
	if (regfile[rs] == regfile[rt]) setpc(pc + (d<<2));
	else skip_next = 1;
      } 
      break;


    case 030: /* DADDI */
      //The 16-bit signed immediate is added to the 64-bit value in GPR rs to produce a 64-bit result. If the addition results in
      //64-bit s complement arithmetic overflow, then the destination register is not modified and an Integer Overflow
      //exception occurs. If it does not overflow, the 64-bit result is placed into GPR rt.
      regfile_up_xflow64(rt, '+', regfile[rs],  sex16(imm));
      break;

    case 031: /* DADDIU */
      regfile_up(rt, regfile[rs] + sex16(imm));
      break;


    case 040: /* LB */
      {
	if (!node->cpu_mem_r8(regfile[rs] + sex16(imm), &tmp64))
	  {
	    write_back_reg = rt;
	    write_back_data = sex24(tmp64 & 0xFF);
	    write_back_flag = 1;
	  }
	else stalled = 1;
	break;
      }
    case 041: /* LH */
      if (!node->cpu_mem_r64(regfile[rs] + sex16(imm), &tmp64))
	{
	  write_back_data = sex16(tmp64 & 0xFFFF);
	  write_back_flag = 1;
	}
      else stalled = 1;
      break;

    case 042: /* LWL load top 3 bytes of a reg with bypassing as needed. - big endian does a leftshift 8 and */
      fatal("unimplemented LWL opcode\n");
      break;


    case 043: /* LW */
      {
	if (!node->cpu_mem_r32(regfile[rs] + sex16(imm), &tmp64))
	  {
	    write_back_reg = rt;
	    write_back_data = sex32(tmp64 &0xFFffFFff);
	    write_back_flag = 1;
	  }
	else stalled = 1;
	break;
      }
    case 044: /* LBU load byte without sign extension */
      if (!node->cpu_mem_r8(regfile[rs] + sex16(imm), &tmp64))
	{
	  write_back_reg = rt;
	  write_back_data = tmp64 & 0xFF;
	  write_back_flag = 1;
	}
      else stalled = 1;
      break;
      
    case 045: /* LHU */
      if (!node->cpu_mem_r16(regfile[rs] + sex16(imm), &tmp64))
	{
	  write_back_reg = rt;
	  write_back_data = tmp64 & 0xFFFF;
	  write_back_flag = 1;
	}
      else stalled = 1;
      break;
      

    case 046: /* LWR load bot 3 bytes of a reg with bypassing as needed.  */
      fatal("unimplemented LWR opcode\n");
      break;

    case 050: /* SB */
      node->cpu_mem_write(regfile[rt], regfile[rs] + sex16(imm), 1);
      break;
      

    case 051: /* SH */
      node->cpu_mem_write(regfile[rt], regfile[rs] + sex16(imm), 2);
      break;

    case 052: /* SWL */
      fatal("unimplemented opcode SWL\n");
      break;

    case 053: /* SW */
      {
	mips_addr_t ea = regfile[rs] + sex16(imm);
	node->cpu_mem_write(regfile[rt], ea, 4);
	break;
      }


    case 056: /* SWR */
      fatal("unimplemented opcode SWR\n");
      break;
      
    case 060:    // LL  Load Locked/Linked.
      if (!node->cpu_mem_r32(regfile[rs] + sex16(imm), &tmp64, true))
	{
	  write_back_reg = rt;
	  write_back_data = tmp64;
	  write_back_flag = 1;
	}
      else stalled = 1;
      break;

    case 061:      
    case 062:      
    case 063:  /* LWC3 Load word from memory to coprocessor */      
      if (!node->cpu_mem_r64(regfile[rs] + sex16(imm), &tmp64))
	{
	  u2_t cpr = (ins >> 26) & 3;
	  int sel = 0;
	  coprocessor_write(cpr, rt, sel, tmp64);
	}
      else stalled = 1;
      break;

    case 067: /* LD */
      if (!node->cpu_mem_r64(regfile[rs] + sex16(imm), &tmp64))
	{
	  write_back_reg = rt;
	  write_back_data = tmp64;
	  write_back_flag = 1;
	}
      else stalled = 1;
      break;


    case 070:      // SC Store Conditional
      { // A 1, indicating success, is written into GPR rt.  Otherwise, memory is not modified and a 0, indicating failure, is written into GPR rt.
	int rc = node->cpu_mem_store_conditional32(regfile[rs] + sex16(imm), regfile[rt]);
	write_back_reg = rc;
	write_back_data = tmp64;
	write_back_flag = 1;
      }
      break;

    case 071:      
    case 072:      
    case 073:  /* SWC3 Store word to memory from coprocessor */      
      {
	u3_t cpr = (ins >> 26) & 3;
	tmp_1 = coprocessor_read(cpr, rt, 0);
	node->cpu_mem_write(regfile[rt], regfile[rs] + sex16(imm), 8);
      }
      break;

    case 077: /* SD */
      node->cpu_mem_write(regfile[rt], regfile[rs] + sex16(imm), 8);
      break;

    default:
      fprintf(stderr, "core %s,%i, ins=%08X unimplemented ins 0%o\n", node->corename, spoke, ins, scode);
      fatal("unimplemented opcode\n");
    }

#ifndef USE_LOAD_DELAY_SLOT
  if (write_back_flag)
    {
      write_back_flag = 0;
      if (write_back_reg)
	{
	  regfile[write_back_reg] = write_back_data;
	  ISSTRC(printf("Load back r%i := " PFX64 "\n", write_back_reg, write_back_data));
	}
    }
#endif


  int m = node->reg_trace(trace_pc);
  if (m) dumpregs(stdout, m & TENOS_TRACE_CPU_ALL_REGS);


  ISSTRC(printf("\n"));
  return 0;

}



uint32_t swap32(uint32_t a)
{
  return ((a>>24) & 0xFF) |    ((a>>8) & 0xFF00) |    ((a<<8) & 0xFF0000) |    ((a<<24) & 0xFF000000);
}

int mips64::step()
{
  int rc = barrels[way]->step();
  return rc;
}


//constructor
mips64::barrel::barrel(mips64 *parent, int bar) : spoke(bar), node(parent)
{
  if (1)
    {
      int cu = 1; // coprocessors available
      u8_t im = 0;// interrupt mask
      // bit1 (kernel=0/user=1) and  interrupt enable lsb (disabled=0/enabled=1)
      u6_t kie_stack = 0;
      cp0_status =        (cu << 28) | (im <<8) | (kie_stack);
    }
  stalled = false;
  skip_next = 0; // move to reset code
  pv_context = 0;
  next_ins = 0;
  next_pc = 0;
  pc = 0;
  MIPS64_COUNT_INSTRUCTIONS(instruction_count = 0);
#ifdef USE_CPU_PRESENCE_BITS
  presencebits = 0;
  for (int i=0;i<32;i++) masks[i] = 1 << i;
#endif
  regfile[0] = 0;
}

u64_t mips64::barrel::coprocessor_read(u8_t cp, u5_t rd, u3_t sel)
{
  if (cp == 0) 
    switch(rd)
      {
      case 12: // status
	{
	  return cp0_status;
	}

      case 4:
	{
	  return pv_context;
	}
      }

  printf("Unsupported coprocessor read cp=%i %i %i\n", cp, rd, sel);
  fatal("bad coprocessor reg no");
  return 1;
}

void mips64::barrel::coprocessor_write(u8_t cp, u5_t rd, u3_t sel, u64_t data)
{
  switch(rd)
    {
    case 12: // status
      {
	cp0_status = data;
	return;
      }
    case 4:
      pv_context = data;
      return;

    }
  printf("Unsupported coprocessor write cp=%i %i %i  data=" PFX64 "\n", cp, rd, sel, data);
  fatal("bad coprocessor reg no");

}

#ifdef USE_CPU_PRESENCE_BITS
bool mips64::barrel::regfile_snoop(u5_t reg, bool write, u64_t &data)
{
  if (write) printf("%s %i remote presence write update reg=%i write=%i " PFX64 "", node->corename, spoke, reg, write, data);
  if (reg == 0) // PC operation
    {
      if (write) setpc(data);
      else data = pc; // +/- correct TODO
    }
  else if (reg >=1 && reg <=25 || reg >=27 && reg <=31)
    {
      if (write) regfile[reg] = data; else data = regfile[reg];
    }
  else if (reg == 26) 
    {
      if (write) multlo = data; else data = multlo;
    }
  else if (reg == 27) 
    {
      if (write) multhi = data; else data = multhi;
    }
  else assert(0);
  if (write) presencebits |= (1<<reg); // THIS ALWAYS SETS - TAC works by ?
  if (!write) printf("%s %i remote presence read reg=%i write=%i " PFX64 "", node->corename, spoke, reg, write, data);
  return true;
}


bool mips64::node_snoop_callback(u64_t addr, bool write, u64_t &data)
{
  if (addr >= framebase && addr < framebase + procMemSpacing * (u64_t)N_BARREL)
    {
      printf("Snoop callback " PFX64 "\n", addr);
      return barrels[(addr-framebase)/procMemSpacing]->regfile_snoop((addr >> 3) & 31, write, data);
    }
  return false;
}
#endif


static u64_t bitpos[64];

void mips64::dumpregs(FILE *fd, bool fullregs, int cmd)
{
  for (int i =0; i < N_BARREL; i++) barrels[i]->dumpregs(fd, fullregs, cmd);
}

//constructor
mips64::mips64(const char *name, u8_t pID,  traceregion **ptr):
  traceregions_ptr(ptr)
{
  eflip = false; // Set in wrapper if needed.
  reset_vector = RESET_VECTOR;
  corename = strdup(name);
  for (int i =0; i < 64; i++) bitpos[i] = (1LLU<<i);
  for (int i =0; i < N_BARREL; i++) barrels[i] = new barrel(this, i);
  //  rsprsp = new gdbrsp(name, 10000 + pID);
  VCC = 1.4; // supply voltage - read from Prazor instead ?

#ifdef TLM_POWER3
// The following tables gives energy consumed in executing the corresponding instruction per bit per volt of VCC
// to obtain energy consumed in executing the corresponding instruction, multiply this number by number of bits and VCC squared

// do we multiply by v^2/2 before or after storing in etable ?  Need to minimise floating point ops really.

// This is energy per instruction ignoring current core voltage: a better fundamental unit would be capacitance disharged per instruction ?  
  etable[ENERGY_1] =  100; // picojoules per Volt per bit
  etable[ENERGY_2] =  200; // picojoules per Volt per bit
  etable[ENERGY_3] =  400; // picojoules per Volt per bit
  etable[ENERGY_4] =  1600; // picojoules per Volt per bit
  etable[ENERGY_UNDEF] =  100; // picojoules per Volt per bit
#endif
 disinit();

}

//TENOS_KIND_DEFINITION(mips64)

// EOF
