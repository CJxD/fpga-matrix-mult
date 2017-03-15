/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */


#ifndef ARMDIS_H
#define ARMDIS_H


// ARM32 disassembler

#include "tenos.h"


class armdis
{
  
  char printable( u32_t );
  void tabto( int );
  const char *registername( u32_t );
  const char *shiftname( u32_t );
  const char *maskname( u32_t );
  const char *opcodename( u32_t );
  const char *f4type( u32_t );
  void printregisterandshift( u32_t );
  u32_t rotateright( u32_t, u32_t );
  void printnumber( u32_t, u32_t );
  u32_t writef2start( u32_t );
  

  
  char *outputbuffer;
  
  int outputpos;
  int operandpos;
  int addresspos;
  
  int noregisterupdate;
  int movetype;
  int movewtype;

  inline u32_t Bits(u32_t a, const unsigned bitno, const unsigned width)
  {
    assert(width > 0);
    assert(bitno + width <= 32);
    
    if (bitno > 0) a >>= bitno;
    if (width < 32) a &= (1 << width) - 1;
    return a;
  }

  u32_t Bits4(u32_t a, const unsigned bitno)
  {
    assert(bitno + 4 <= 32);
    
    if (bitno > 0) a >>= bitno;
    return a & 15;
  }

  u32_t Bit(u32_t a, const unsigned bitno)
  {
    if (bitno > 0) a >>= bitno;
    return a & 1;
  }
  
  bool extra_misc_3ab(u32_t ins);
  bool extra_misc_3c(u32_t ins);
  
  bool thumb_instr(u32_t in, u32_t pc);
  u8_t thumb_instr_ITSTATE;
  bool thumb_instr_InITBlock();
  void thumb_instr_ITAdvance();
  const char* thumb_instr_ITBlockCond;
  u32_t thumb_instr_expand_imm(u12_t imm12);

  void thumb16_instr(u32_t in, u32_t pc);

  void thumb32_instr(u32_t in1, u32_t in2, u32_t pc);
  bool thumb32_instr_next;
  u32_t thumb32_instr_instr;


 public:
  // constructor
  armdis();
  
  bool dis( u32_t loc, u32_t a, char *buffer, int thumb_mode=0);
};
  

#endif
