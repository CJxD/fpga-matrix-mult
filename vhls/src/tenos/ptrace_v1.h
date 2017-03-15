#ifndef PTRACE_V1_H
#define PTRACE_V1_H

// 
// $Id: $
// Part of prazor-virtual-platform
// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2015 DJ Greaves and Milos Puzovic
// University of Cambridge, Computer Laboratory.
//
// Binary instruction tracer.
//

// Enable and disable with this flag
// #define PTRACE_V1_ENABLE 1


#ifdef PTRACE_V1_ENABLE
#define  BIN_INSTRUCTION_PTRACE(core_no, set_no, executedf, pc, ins32)   if (g_ptracer_v1) g_ptracer_v1->instruction(core_no, set_no, executedf, pc, ins32)
#define  OPERAND_PTRACE(op32)  if (g_ptracer_v1) g_ptracer_v1->operand(op32)
#else

// Null definitions when disabled
#define  BIN_INSTRUCTION_PTRACE(core_no, set_no, executedf, pc, ins32)
#define  OPERAND_PTRACE(op32)
#endif

// AARCH ARM 32 is set no 40 Elf encoding, but we here use a more simple encoding, with values above 128 denoting 16 bit instruction sets.
#define ARM32_SET 2
#define ATHUMB2_32_SET 3
#define ATHUMB1_16_SET 130
#define ATHUMB2_16_SET 131



#define PTRACE_FMT_ID 1

class ptrace_v1
{
  const char * fn; 
  FILE *ptrace_fd = 0;
  u32_t ptrace_count = 0;
  bool ptrace_dirty = false;
  u32_t pblock[4];

  void flush();

 public:
  ptrace_v1(const char *filename);// constructor

 void instruction(int set_no, int core_no, bool executedf, u32_t pc, u32_t ins32);
 void operand(u32_t operand);
 void close();

};

extern ptrace_v1 *g_ptracer_v1;


#endif
