// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>


/*
 *  ARM disassembler.  This code was originally written in BCPL by Harry
 *  Meekings, and has since been translated to C.
 *
 *  Version for tenosarm.
 *
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "tenos.h"
#include "armdis.h"
#include "ctype.h"

#define f_add           0x00800000u
#define f_sub           0x00400000u
#define f_rsb           0x00600000u
#define f_adc           0x00a00000u
#define f_sbc           0x00c00000u
#define f_rsc           0x00e00000u
#define f_and           0x00000000u
#define f_eor           0x00200000u
#define f_orr           0x01800000u
#define f_bic           0x01c00000u
#define f_cmp           0x01400000u
#define f_cmn           0x01600000u
#define f_teq           0x01200000u
#define f_tst           0x01000000u
#define f_subs          0x00500000u
#define f_cmps          0x01500000u
#define f_cmns          0x01700000u
#define f_teqs          0x01300000u
#define f_tsts          0x01100000u
#define f_mov           0x01a00000u
#define f_mvn           0x01e00000u
#define f_movs          0x01b00000u
#define f_cond          0x00100000u

#define f_ldr           0x04100000u
#define f_str           0x04000000u
#define f_ldrb          0x04500000u
#define f_strb          0x04400000u
#define f_ld            0x00100000u
#define f_st            0x00000000u

#define f_ldm           0x08100000u
#define f_stm           0x08000000u
#define f_pc            0x00400000u

#define f_pre           0x01000000u
#define f_post          0x00000000u
#define f_wb            0x00200000u
#define f_up            0x00800000u
#define f_down          0x00000000u

#define f_preup         (f_pre + f_up)
#define f_postup        (f_post + f_up)
#define f_predown       (f_pre + f_down)
#define f_postdown      (f_post + f_down)

#define f_b             0x0a000000u
#define f_bl            0x0b000000u

#define m_eq            0x00000000u
#define m_ne            0x10000000u
#define m_geu           0x20000000u
#define m_ltu           0x30000000u
#define m_mi            0x40000000u
#define m_pl            0x50000000u
#define m_vs            0x60000000u
#define m_vc            0x70000000u
#define m_gtu           0x80000000u
#define m_leu           0x90000000u
#define m_ge            0xa0000000u
#define m_lt            0xb0000000u
#define m_gt            0xc0000000u
#define m_le            0xd0000000u
#define m_always        0xe0000000u
#define m_never         0xf0000000u



#define wrch( C )\
        {\
                outputbuffer[outputpos++]  =  C;\
                outputbuffer[outputpos]  =  (char) 0;\
        }



#define writes( S )\
        {\
                sprintf( &outputbuffer[outputpos], "%s", S );\
                \
                outputpos  =  strlen( outputbuffer );\
        }



#define writef1( F, A1 )\
        {\
                sprintf( &outputbuffer[outputpos], F, A1 );\
                \
                outputpos  =  strlen( outputbuffer );\
        }



#define writef2( F, A1, A2 )\
        {\
                sprintf( &outputbuffer[outputpos], F, A1, A2 );\
                \
                outputpos  =  strlen( outputbuffer );\
        }



#define writef3( F, A1, A2, A3 )\
        {\
                sprintf( &outputbuffer[outputpos], F, A1, A2, A3 );\
                \
                outputpos  =  strlen( outputbuffer );\
        }



#define writef4( F, A1, A2, A3, A4 )\
        {\
                sprintf( &outputbuffer[outputpos], F, A1, A2, A3, A4 );\
                \
                outputpos  =  strlen( outputbuffer );\
        }

#define writef5( F, A1, A2, A3, A4, A5 )		\
        {\
 	        sprintf( &outputbuffer[outputpos], F, A1, A2, A3, A4, A5 );	\
                \
                outputpos  =  strlen( outputbuffer );\
        }

#define writef6( F, A1, A2, A3, A4, A5, A6 )	\
        {\
 	        sprintf( &outputbuffer[outputpos], F, A1, A2, A3, A4, A5, A6 );	\
                \
                outputpos  =  strlen( outputbuffer );\
        }

#define writef7( F, A1, A2, A3, A4, A5, A6, A7 )	\
        {\
 	        sprintf( &outputbuffer[outputpos], F, A1, A2, A3, A4, A5, A6, A7 ); \
                \
                outputpos  =  strlen( outputbuffer );\
        }

#define writef8( F, A1, A2, A3, A4, A5, A6, A7, A8 )	\
        {\
	        sprintf( &outputbuffer[outputpos], F, A1, A2, A3, A4, A5, A6, A7, A8 ); \
                \
                outputpos  =  strlen( outputbuffer );\
        }

#define writef9( F, A1, A2, A3, A4, A5, A6, A7, A8, A9 )	\
        {\
    	        sprintf( &outputbuffer[outputpos], F, A1, A2, A3, A4, A5, A6, A7, A8, A9 ); \
                \
                outputpos  =  strlen( outputbuffer );\
        }



bool armdis::extra_misc_3ab(u32_t in) 
{
  //if (Bits(a, 4, 24) == 0x12fff1UL)

  if ((in & 0xFFFFFD0) == 0x12fff10) // This code not in the right place? 
    {
      const char *nem = (Bit(in, 5)) ? "blx": "bx";
      writef3("%s%s %s", nem, maskname(in), registername(Bits(in, 0, 4)));
    }
  
  else if ((in & 0xF9000F0) == 0x1000000)
    {
      if (!Bit(in, 21)) // when set we move to msr with a mask
	
	{ // psr to register - bit 22 set for SPSR
	  u4_t insRd = Bits4(in, 12);
	  writef3("mrs%s %s,%s",  maskname(in), registername(insRd), Bit(in, 22) ? "SPSR": "CPSR");
		  
	}
      else 
	{ // register to psr - bit 22 set for SPSR
	  int mask = Bits4(in, 16);
	  u4_t insRm = Bits4(in, 0);
	  writef4("msr%s %s,%s (mask=0x%X)",  maskname(in), Bit(in, 22) ? "SPSR": "CPSR", registername(insRm), mask);
	}
    }

  else if ((in & 0xFFF0FF0) == 0x16f0f10) // count leading zeros : clz
    {
      u4_t insRd       = Bits4(in, 12);
      u4_t insRm       = Bits4(in, 0);
      writef3("clz%s %s, %s",  maskname(in), registername(insRd), registername(insRm));
    }
  else if ((in & 0x1600080) == 0x1600080) {
    bool n_high = !!Bit(in, 5);
    bool m_high = !!Bit(in, 6);
    
    u4_t insRd = Bits4(in, 16);
    u4_t insRm = Bits4(in, 8);
    u4_t insRn = Bits4(in, 0);

    writef5("smul%c%c %s, %s, %s", 
	   n_high ? 't' : 'b',
	   m_high ? 't' : 'b',
	   registername(insRd), 
	   registername(insRn),
	   registername(insRm));

  }

  // DSP add/sub/mult is missing
  // Software break is missing
  else writef1("Bad 3ab ins %x", in);

  return true;
}

bool armdis::extra_misc_3c(u32_t in) 
{
  // Section 3.1.1 of ddi0100e_arm_arm.pdf
  //printf("special3c\n"); 
  if ((in & 0x0F0000F0) == 0x90) // multiply (+accumulate)
    {
      u4_t Rd = Bits4(in, 16);
      u4_t Rn = Bits4(in, 12);
      u4_t Rm = Bits4(in, 0);
      u4_t Rs = Bits4(in, 8);
      bool S = Bit(in, 20);
      bool A = Bit(in, 21);
      //bad disassembly
      //	     e0832490 umull r2, r3, r0, r4
      //comes out as e0832490  mulL r3, r2, r0 which does not mention r4!

      const char *mde = !Bit(in, 23) ? "": Bit(in, 22) ? "LU": "L";      
      const char *s = (S) ? "s" : "";
      if (A)
	{
	  if(Bit(in, 22) == 0) {
	    writef7("mla%s%s%s %s, %s, %s, %s",  s, maskname(in), mde, registername(Rd), registername(Rn), registername(Rm), registername(Rs)); 
	  }
	  else if(Bit(in, 22) == 1) {
	    writef7("mls%s%s%s %s, %s, %s, %s", s, maskname(in), mde, registername(Rd), registername(Rm), registername(Rs), registername(Rn));
	  }
	}
      else
	{
	  writef6("mul%s%s%s %s, %s, %s",  s, maskname(in), mde, registername(Rd), registername(Rn), registername(Rm)); 
        }
    }

  else if ((in & 0x0FB000F0) == 0x1000090) // swap
    {
      bool B = Bit(in, 22);
      int Rm = Bits(in, 0, 4);
      int Rd = Bits(in, 12, 4);
      int Rn = Bits(in, 16, 4);
      const char *mde = (B) ? "b": "";
      writef5("swap%s%s %s,%s,%s (order of regs?)",  mde, maskname(in), 
	      registername(Rd),
	      registername(Rn),
	      registername(Rm));
    }
  else // Extra loads and stores
    {
      int index = 0;
      bool excf = ((in & 0xF00) == 0xF00);
      const char *exm = (excf) ? "ex":"";
      bool U = Bit(in, 23);
      bool W = Bit(in, 21);
      bool P = Bit(in, 24);
      //bool L = Bit(in, 20), H = Bit(in, 5), S = Bit(in, 6);
      const char *size = "";
      bool signedf = false;
      bool loadf = false;

      // Further atomic decode exceptions - where are these documented?
      if (exm[0] && Bits(in, 21, 3) == 7)
	{
	  if (Bit(in, 20)) loadf = true;
	  if (Bit(in, 22)) index = (Bits(in, 8, 4) << 4) | (Bits(in, 0, 4) << 0);
	  size = "h";
	}
      else if (exm[0] && Bits(in, 21, 3) == 6)
	{
	  if (Bit(in, 20)) loadf = true;
	  size = "b";
	}
      else if (exm[0] && Bits(in, 21, 3) == 5)
	{
	  if (Bit(in, 20)) loadf = true;
	  size = "d";
	}

      else  switch (in & 0x100060) // indexed and reg indirect (index=0) forms
	{
	  
	case 0x20: //	L=0, S=0, H=1 Store halfword.
	  if (Bit(in, 22)) 
	    index = (Bits(in, 8, 4) << 4) | (Bits(in, 0, 4) << 0);
	  else
	    index = Bits(in, 0, 4);
	  size = "h";
	  break;

	case 0x40:  // L=0, S=1, H=0 Load doubleword.
	  if (Bit(in, 22)) 
	    index = (Bits(in, 8, 4) << 4) | (Bits(in, 0, 4) << 0);
	  else
	    index = Bits(in, 0, 4);
	  loadf = true; size = "d";
	  break;

	case 0x60:  // L=0, S=1, H=1 Store doubleword.
	  if (Bit(in, 22)) 
	    index = (Bits(in, 8, 4) << 4) | (Bits(in, 0, 4) << 0);
	  else
	    index = Bits(in, 0, 4);
	  size = "d";
	  break;

	case 0x100020:  // L=1, S=0, H=1 Load unsigned halfword.
	  if (Bit(in, 22)) 
	    index = (Bits(in, 8, 4) << 4) | (Bits(in, 0, 4) << 0);
	  else
	    index = Bits(in, 0, 4);
	  loadf = true; size = "h";
	  break;

	case 0x100040:  // L=1, S=1, H=0 Load signed byte.
	  if (Bit(in, 22)) 
	    index = (Bits(in, 8, 4) << 4) | (Bits(in, 0, 4) << 0);
	  else
	    index = Bits(in, 0, 4);
	  loadf = true; signedf = true; size = "sb"; 
	  break;

	case 0x100060:  // L=1, S=1, H=1 Load signed halfword
	  if (Bit(in, 22)) 
	    index = (Bits(in, 8, 4) << 4) | (Bits(in, 0, 4) << 0);
	  else
	    index = Bits(in, 0, 4);
	  loadf = true; signedf = true; size = "sh"; 
	  break;

	default:
	  if (exm[0]) 
	    {
	      if (Bit(in, 20)) loadf = true;
	      size = "";
	      break;
	    }
	  writef1("Bad instruction %x - extra group 3c", in);
	  return true;
	}

      if (!U) index = -index;
      char offsets[132];
      if (P) {
	if(Bit(in, 22)) 
	  snprintf(offsets, 132, ", #%i", index); 
	else
	  //snprintf(offsets, 132, ",%c%s", (U) ? '+' : '-', registername(index));
	  //	e1802f93 	strex	r2, r3, [r0]
	  // was  wrongly disassembling : (_archglue_test_and_set+0x8) ins=e1802f93  ./.a  e1802f93  strex r2,r3,[r0,+r0]
	  offsets[0] = (char) 0;
      }
      else 
	offsets[0] = (char) 0;
      int Rd = Bits(in, 12, 4); 
      int Rn = Bits(in, 16, 4); 
      // TODO: mp and djg to go over this logic
      const char *wb = (!P) ? "": (W) ? "!": ""; 
      // Store exclusives have an extra leading reg for the return code.
      // Double word stores have an extra implied reg (odd of an odd even pair).
      if (loadf)
	{
	  writef9("ldr%s%s%s %s,[%s%s%s%s%s",  exm, size, maskname(in), registername(Rd), registername(Rn), (!P) ? "]" : "",  offsets, (P) ? "]" : "", wb);
	}
      else 
	{
	  if (exm[0])
	    {
	      int Rm = Bits(in, 0, 4); 
	      writef7("strex%s%s %s,%s,[%s%s]%s",  size, maskname(in), registername(Rd), registername(Rm), registername(Rn), offsets, wb);
	    }
	  else
	    {
	      writef8("str%s%s %s,[%s%s%s%s%s",  size, maskname(in), registername(Rd), registername(Rn), (!P) ? "]" : "", offsets, (P) ? "]" : "", wb);
	    }
	}
      return true;
    }

  return true;
}

bool armdis::thumb_instr_InITBlock() {
    return (thumb_instr_ITSTATE & 0xF) != 0;
}

void armdis::thumb_instr_ITAdvance() {
    if((thumb_instr_ITSTATE & 0x7) == 0)
        thumb_instr_ITSTATE = 0;
    else
        thumb_instr_ITSTATE = (thumb_instr_ITSTATE & 0xE0) | ((thumb_instr_ITSTATE & 0x1F) << 1);
}

u32_t armdis::thumb_instr_expand_imm(u12_t imm12) {
    u32_t imm32 = 0;

    if((imm12 >> 10) == 0) {
        u2_t op = (imm12 >> 8) & 3;
        u8_t imm8 = (imm12 & 0xFF);
        switch(op) {
          case 0: {
              imm32 = (u32_t)(imm12 & 0xFF);
              break;
          }
          case 1: {
              assert(imm8 != 0);
              imm32 |= (imm8 << 16) | imm8;
              break;
          }
          case 2: {
              assert(imm8 != 0);
              imm32 |= (imm8 << 24) | (imm8 << 8);
              break;
          }                        
          case 3: {
              assert(imm8 != 0);
              imm32 = (imm8 << 24) | (imm8 << 16) | (imm8 << 8) | imm8;
              break;
          }        
        }
    } else {
        u32_t unrotated_value = (1 << 7) | (imm12 & 0x7F);
        u8_t rotation = (imm12 >> 7) & 0x1F;
        if(rotation != 0)
            imm32 = (unrotated_value >> rotation) 
                | (unrotated_value << (32 - rotation));
    }

    return imm32;
}


void armdis::thumb32_instr(u32_t in1, u32_t in2, u32_t pc) {
    u32_t in = (in1 << 16) | in2;
    writef4( " %c%c%c%c", 
	     printable( (in >> 0) & 0xff ), 
	     printable( (in >> 8) & 0xff ), 
             printable( (in >> 16) & 0xff ), 
             printable( (in >> 24) & 0xff ) );
    writef2( "  %04x %04x  ", (in >> 16), (in & 0xFFFF) );

    thumb_instr_ITBlockCond = "";
    if(thumb_instr_InITBlock()) {
        u32_t condition = (thumb_instr_ITSTATE >> 4) & 0xF;
        thumb_instr_ITBlockCond = maskname(condition << 28);
        thumb_instr_ITAdvance();
    }

    u2_t op1 = Bits(in1, 11, 2);
    u7_t op2 = Bits(in1, 4, 7);
    u1_t op = Bits(in2, 15, 1);

    const char* cond = thumb_instr_ITBlockCond;

    switch(op1) {
      case 1: {
          if((op2 & 0x64) == 0) {
              // Load/store multiple
              u2_t op = Bits(in1, 7, 2);
              u5_t W = Bit(in1, 5);
              u1_t L = Bit(in1, 4);
              u4_t Rn = Bits(in1, 0, 4);
              
              switch(op) {                  
                case 1: {
                    switch(L) {
		      case 0: {
			writes("stmia.w");
			break;
		      }
                      case 1: {
                          writes("ldmia.w");
			  break;
                      }
                    }
                    break;
                }
                  
                case 2: {
                    switch(L) {
                      case 0: {
                          writes("stmdb");
                          break;
                      }
                    }
                    break;
                }
              }

              writef3("%s %s%s, {",
                      cond,
                      registername(Rn),
                      W ? "!" : "");
              u16_t registers = Bits(in2, 0, 16);
              bool previous = false;
              for(int i = 0; i < 16; i++) {
                  if(Bit(registers, i)) {
                      if(previous) {
                          writes(", ");
                      }
                      else {
                          previous = true;
                      }
                      writef1("%s", registername(i));
                  }
              }
              writes("}");
          } else if((op2 & 0x64) == 0x04) {
              // Load/store dual, load/store exclusive, table branch
              u2_t op1 = Bits(in1, 7, 2);
              u2_t op2 = Bits(in1, 4, 2);
              u2_t op3 = Bits(in2, 4, 4);
              u4_t Rn = Bits4(in1, 0);
              bool index = !!Bit(in1, 8);
              bool wback = !!Bit(in1, 5);
              u32_t imm32 = ((u32_t)Bits(in2, 0, 8)) << 2;
              if(!Bit(in1, 7)) imm32 *= -1;

	      if(op1 == 0 && op2 == 0) {
		writef5("strex%s, %s, %s, [%s, #%d]",
			cond,
			registername(Bits(in2, 8, 4)),
			registername(Bits(in2, 12, 4)),
			registername(Bits(in1, 0, 4)), 
			imm32);
		return;
	      }
	      else if(op1 == 0 && op2 == 1) {
		writes("ldrex");
		index = true;
		wback = false;
	      }
              else if(((op1 >> 1) == 0 && op2 == 2) ||
                 ((op1 >> 1) == 1 && ((op2 & 1) == 0))) {
                  writes("strd");
              } else if((((op1 >> 1) == 0 && op2 == 3) 
                         || ((op1 >> 1) == 1 && (op2 & 1) == 1)) 
                        && Rn != 15) {
                  writes("ldrd");
              } else if(op1 == 1 && op2 == 1) {
                  switch(op3) {
                    case 0:
                      writes("tbb");
                      break;
                    case 1:
                      writes("tbh");
                      break;
                  }

                  writef4("%s [%s, %s%s",
                          cond,
                          registername(Rn),
                          registername(Bits4(in2, 0)),
                          op3 == 0 ? "]" : ", lsl #1]");

                  return;
                  
              }
              writef7("%s, %s, [%s%s, #%d%s%s",
                      cond,
                      registername(Bits(in2, 12, 4)),
                      registername(Bits(in1, 0, 4)),
                      index ? "" : "]",
                      imm32,
                      index ? "]" : "",
                      (index && wback) ? "!" : "");
          } else if((op2 & 0x60) == 0x20) {
              // Data-processing (shifted register)
              u4_t op = Bits(in1, 5, 4);
              u32_t shiftty = Bits(in2, 4, 2) << 5;
              u32_t imm3 = Bits(in2, 12, 3);
              u32_t imm2 = Bits(in2, 6, 2);
              u32_t shiftval = (imm3 << 2) | imm2;
              u4_t Rn = Bits4(in1, 0);
              u5_t RdS = (((u5_t)(Bits(in2, 8, 4))) << 1) | Bit(in1, 4);

              bool dest = true;
              
              switch(op) {
                case 0: {
                    if(RdS != 0x1F) {
                        writes("and");
                    }
                    else {
                        writes("tst");
                    }
                    break;
                }
                case 1: {
                    writes("bic");
                    break;
                }
                case 2: {
                    if(Rn != 15) {
                        writes("orr");
                    } else {
                        if(shiftty == 0 && shiftval == 0) {
                            writef4("mov%s%s.w %s, %s",
                                    (Bit(in1, 4)) ? "s" : "",
                                    cond,
                                    registername(Bits4(in2, 8)),
                                    registername(Bits4(in2, 0)));
                            return;
                        } else {
                            writef1("%s", shiftname(shiftty));
                        }
                    }
                    break;
                }

                case 3: {
                    if(Rn != 15) {
                        writes("orn");
                    } else {
                        writes("mvn");
                    }
                    break;
                }

                case 4: {
                    if(RdS != 0x1F) {
                        writes("eor");
                    } else {
                        writes("teq");
                    }
                    break;
                }

                case 8: {
                    if(RdS != 0x1F) {
                        writes("add");
                    }
                    break;
                }
                case 10: {
                    writes("adc");
                    break;
                }
                case 11: {
                    writes("sbc");
                    break;
                }
                case 13: {
                    if(RdS != 0x1F) {
                        writes("sub");
                    } else {
                        writes("cmp");
                        dest = false;
                    }
                    break;
                }

                case 14: {
                    writes("rsb");
                    break;
                }
              }              
              if(Rn != 15) {
                  writef7("%s%s.w %s, %s, %s, %s #%d",
                          cond,
                          !(Bit(in1, 4)) ? "" : (dest ? "s" : ""),
                          dest ? registername(Bits(in2, 8, 4)) : "",
                          registername(Bits(in1, 0, 4)),
                          registername(Bits(in2, 0, 4)),
                          shiftname(shiftty),
                          shiftval);
              } else {
                  writef5("%s%s.w %s, %s, #%d",
                          cond,
                          !(Bit(in1, 4)) ? "" : "s",
                          registername(Bits4(in2, 8)),
                          registername(Bits4(in2, 0)),
                          shiftval);
              }
          } else if((op2 & 0x40) == 0x40) {
              // Coprocessor, Advanced SIMD, and Floating-point instructions
	      u6_t op1 = Bits(in1, 4, 6);
	      u4_t coproc = Bits(in2, 8, 4);
	      u1_t op = Bit(in2, 4);
	      u4_t Rn = Bits4(in1, 0);
	      
	      if((coproc & 0xE) != 10) {
		  if((op1 & 0x31) == 0x20 && op == 1) {
		    writef6("mcr %d, %d, %s, cr%d, cr%d, {%d}",
			    coproc,
			    Bits(in1, 5, 3),
			    registername(Bits4(in2, 12)),
			    Bits4(in1, 0),
			    Bits4(in2, 0),
			    Bits(in2, 5, 4));
		  }
	      }
          }
          break;
      }
      case 2: {
          if((op2 & 0x20) == 0x0 && op == 0) {
              u4_t op = Bits4(in1, 5);
              u4_t Rn = Bits4(in1, 0);
              u1_t S = Bit(in1, 4);
              u4_t Rd = Bits4(in2, 8);
              u5_t RdS = ((u5_t)Rd << 1) | S;
              
              u12_t imm3 = Bits(in2, 12, 3);
              u12_t imm8 = Bits(in2, 0, 8);
              u12_t imm12 = (Bit(in1, 10) << 11) | (imm3 << 8) | imm8;
              u32_t imm32 = thumb_instr_expand_imm(imm12);

              switch(op) {
                case 0: {
                    if(RdS != 0x1F) {
                        writes("and");
                    } else {
                        writes("tst");
                    }
                    break;
                }
                case 1: {
                    writes("bic");
                    break;
                }
                case 2: {
                    if(Rn == 15) {
                        writes("mov");
                    } else {
                        writes("orr");
                    } 
                    break;
                }
                case 3: {
                    if(Rn == 15) {
                        writes("mvn");
                    }
                    else {
                        writes("orn");
                    }
                    break;
                }
                case 4: {
                    if(RdS != 0x1F) {
                        writes("eor");
                    } else {
                        writes("teq");
                    }
                    break;
                }
                case 8: {
                    if(RdS != 0x1F) {
                        writes("add");
                    }
                    break;
                }

                case 10: {
                    writes("adc");
                    break;
                }                  

                case 11: {
                    writes("sbc");
                    break;
                }

                case 13: {
                    if(RdS != 0x1F) {
                        writes("sub");
                    } else {
                        writes("cmp");
                    }
                    break;
                }
                case 14: {
                    writes("rsb");
                    break;
                }
              }
              if(Rn != 15) {
                  if(RdS == 0x1F) {
                      writef3("%s.w %s, #%d",
                              cond,
                              registername(Rn),
                              imm32);
                  } else {
                      writef5("%s%s.w %s, %s, #%d",
                              S ? "s" : "",
                              cond,
                              registername(Rd),
                              registername(Rn),
                              imm32);
                  }
              } else {
                  writef4("%s%s.w %s, #%d",
                          S ? "s" : "",
                          cond,
                          registername(Rd),
                          imm32);                  
              }                       
          } else if((op2 & 0x20) == 0x20 && op == 0) {
              // Data-processing (plain binary immediate)
              u5_t op = Bits(in1, 4, 5);
              u4_t Rd = Bits4(in2, 8);
              u4_t Rn = Bits4(in1, 0);
              u32_t imm32 = 0;
              imm32 = 
                  (Bits(in1, 0, 4) << 12) | 
                  (Bit(in1, 10) << 11) |
                  (Bits(in2, 12, 3)) << 8 |
                  (Bits(in2, 0, 8));
              u32_t imm12 = 
                  (Bit(in1, 10) << 11) | 
                  (Bits(in2, 12, 3) << 8) |
                  (Bits(in2, 0, 8));
              
              switch(op) {
                case 0: {
                    if(Rn != 15) {
                        writef4("addw%s %s %s, #%d",
                                cond,
                                registername(Rd),
                                registername(Rn),
                                imm12);
                        return;
                    }
                    break;
                }
                case 4: {
                    writes("movw");
                    break;
                }

                case 10: {
                    if(Rn != 15) {
                        writef4("subw%s %s %s, #%d",
                                cond,
                                registername(Rd),
                                registername(Rn),
                                imm12);
                        return;
                    }

                    break;                    
                }
                  
                case 12: {
                    writes("movt");
                    break;
                }

                case 0x1C: {
                    writef5("ubfx%s %s, %s, #%d, #%d",
                            cond,
                            registername(Bits4(in2, 8)),
                            registername(Bits4(in1, 0)),
                            (Bits(in2, 12, 3) << 2) | Bits(in2, 6, 2),
                            Bits(in2, 0, 5) + 1);
                    return;
                }
              }

              writef3("%s %s, #%d",
                      cond,
                      registername(Rd),
                      imm32);

          } else if(op == 1) {
              // Branches and miscellaneous control
              u7_t op = Bits(in1, 4, 7);
              u3_t op1 = Bits(in2, 12, 3);
              u4_t op2 = Bits(in2, 8, 4);
              
              if(((op1 & 0x5) == 0) && (op & 0x38) != 0x38) {
                  u32_t cond = Bits(in1, 6, 4) << 28;
                  u32_t S = Bit(in1, 10);
                  u32_t J2 = Bit(in2, 11);
                  u32_t J1 = Bit(in2, 13);
                  u32_t imm6 = Bits(in1, 0, 6);
                  u32_t imm11 = Bits(in2, 0, 11);
                  
                  u32_t offset = 
                      (S << 20) |
                      (J2 << 19) |
                      (J1 << 18) |
                      (imm6 << 12) |
                      (imm11 << 1) |
                      0;
                  if(Bit(offset, 20)) offset |= 0xFFE00000; // sign extend
                  u32_t dest = pc + offset + 2;
                  writef2("b%s.w 0x%x",
                          maskname(cond),
                          dest);
              }
              else if((op1 & 5) == 5 || (op1 & 5) == 4 || (op1 & 5) == 1) {
                  bool s = Bits(in1, 10, 1);
                  bool j1 = Bits(in2, 13, 1);
                  bool j2 = Bits(in2, 11, 1);
                  bool i1 = !(j1 ^ s);
                  bool i2 = !(j2 ^ s);
                  u10_t imm10 = Bits(in1, 0, 10);
                  u11_t imm11 = 
                      Bits(in2, 12, 1) ? 
                      Bits(in2, 0, 11) : 
                      (Bits(in2, 1, 10) << 1);
                  u32_t offset = 0;
                  offset = 
                      (s << 24) |
                      (i1 << 23) |
                      (i2 << 22) |
                      (imm10 << 12) |
                      (imm11 << 1) |
                      0;
                  if(s) offset |= 0xFE000000;
                  bool thumb_is_next = !!Bits(in2, 12, 1);
                  u32_t base = pc + 2;
                  if(!thumb_is_next)
                      base &= ~3; // align
                  u32_t dest = base + offset;
		  if(Bit(in2, 14)) {
		    writef3("bl%s%s 0x%x",
			    !Bit(in2, 12) ? "x" : "",
			    cond,
			    dest);
		  }
		  else {
		    writef2("b%s.w 0x%x",
			    cond,
			    dest);
		  }
              }              
          }
          break;
      }
      case 3: {
          if((op2 & 0x71) == 0x0) {
              // Store single data item
              u3_t op1 = Bits(in1, 5, 3);
              u6_t op2 = Bits(in2, 6, 6);

              bool index = false;
              bool wback = false;
              u32_t offset = Bits(in2, 0, 8);
    
              if((op1 == 2 && ((op2 & 0x24) == 0x24 || (op2 & 0x3C) == 0x30))
		 || op1 == 6
		 || (op1 == 0 && ((op2 & 0x24) == 0x24 || (op2 & 0x3C) == 0x30))
		 || op1 == 4) {
		if(op1 == 2 || op1 == 6) {
		  writes("str");
		}
		else if(op1 == 0 || op1 == 4) {
		  writes("strb");
		}
		
		if(Bit(in1, 7)) {
		  index = true;
		  wback = false;
		} else {
		  index = Bit(in2, 10);
		  wback = Bit(in2, 8);
		  if(!(Bit(in2, 9)))
		    offset = 0 - offset;
		}
              }
              else if(op1 == 2 && op2 == 0) {
                  writef5("str%s.w, %s, [%s, %s, lsl#%d]",
                          cond,
                          registername(Bits(in2, 12, 4)),
                          registername(Bits(in1, 0, 4)),
                          registername(Bits(in2, 0, 4)),
                          Bits(in2, 4, 2));
                  return;
              } else if(op1 == 5 || 
			(op1 == 1 && ((op2 & 0x24) == 0x24 || (op2 & 0x3C) == 0x30))) {
 		  writes("strh");
		  if(Bit(in1, 7)) {
		      index = true;
		      wback = false;
		      offset = Bits(in2, 0, 12);
		  }
		  else {
		      index = Bit(in2, 10);
		      wback = Bit(in2, 8);
		      offset = Bits(in2, 0, 8);
		  }
	      } else if(op1 == 1 && op2 == 0) {
		writef5("strh%s.w, %s, [%s, %s, lsl#%d]",
			cond,
			registername(Bits(in2, 12, 4)),
			registername(Bits(in1, 0, 4)),
			registername(Bits(in2, 0, 4)),
			Bits(in2, 4, 2));
		return;
	      }
                            
              writef7("%s.w, %s, [%s%s, #%d%s%s",
                      cond,
                      registername(Bits(in2, 12, 4)),
                      registername(Bits(in1, 0, 4)),
                      index ? "" : "]",
                      offset, 
                      index ? "]" : "",
                      (index && wback) ? "!" : "");

          } else if((op2 & 0x67) == 0x1) {
              // Load byte, memory hints
	      u2_t op1 = Bits(in1, 7, 2);
	      u6_t op2 = Bits(in2, 6, 6);
	      u4_t Rn = Bits4(in1, 0);
	      u4_t Rt = Bits4(in2, 12);

	      if(op1 == 0 && op2 == 0 && Rn != 15 && Rt != 15) {
		  writef5("ldrb%s.w %s, [%s, %s, lsl #%d]",
			  cond,
			  registername(Bits4(in2, 12)),
			  registername(Rn),
			  registername(Bits4(in2, 0)),
			  Bits(in2, 4, 2));
		  return;
	      } else if((op1 == 0 && (op2 & 0x24) == 0x24 && Rn != 15)
			|| (op1 == 1 && Rn != 15 && Rt != 15) 
			|| (op1 == 0 && (op2 & 0x3C) == 0x30 && Rn != 15 && Rt != 15)) {
		  bool index = true;
		  bool wback = false;
		  u32_t offset;
		  if(!Bit(in1, 7)) {
		      index = Bit(in2, 10);
		      wback = Bit(in2, 8);
		      offset = Bits(in2, 0, 8);
		  } else {
		      offset = Bits(in2, 0, 12);
		  }
		
		  writef7("ldrb%s.w, %s, [%s%s, #%d%s%s",
			  cond,
			  registername(Bits4(in2, 12)),
			  registername(Rn),
			  index ? "" : "]",
			  offset,
			  index ? "]" : "",
			  (index && wback) ? "!" : "");
	      }
          } else if((op2 & 0x67) == 0x3) {
	    // Load halfword, memory hints
              u2_t op1 = Bits(in1, 7, 2);
              u6_t op2 = Bits(in2, 6, 6);
              u4_t Rn = Bits4(in1, 0);
              u4_t Rt = Bits4(in2, 12);
	      u32_t offset = 0;
	      bool index = true;
	      bool wback = false;

	      bool imm = true;
	      
              if(op1 == 3 && Rn != 15 && Rt != 15) {
		  writes("ldrsh");
		  offset = Bits(in2, 0, 12);
              } else if((op1 == 1 && Rn != 15 && Rt != 15) ||
			(op1 == 0 && 
			 (((op2 & 0x24) == 0x24 && Rn != 15) ||
			  ((op2 & 0x3C) == 0x30 && Rn != 15 && Rt != 15)))) {
		  writes("ldrh");
		  if(!(Bit(in1, 7))) {
		      index = Bit(in2, 10);
		      wback = Bit(in2, 8);
		      offset = Bits(in2, 0, 8);
		  } else {
		      offset = Bits(in2, 0, 12);
		  }
	      } else if(op1 == 2 && op2 == 0 && Rn != 15 && Rt != 15) {
		writes("ldrsh");
		imm = false;
	      }

	      if(imm) {
		writef7("%s.w, %s, [%s%s, #%d%s%s",
			cond,
			registername(Bits(in2, 12, 4)),
			registername(Bits(in1, 0, 4)),
			index ? "" : "]",
			offset, 
			index ? "]" : "",
			(index && wback) ? "!" : "");
	      } else {
		writef7("%s.w, %s, [%s%s, %s%s%s",
			cond,
			registername(Bits(in2, 12, 4)),
			registername(Bits(in1, 0, 4)),
			index ? "" : "]",
			registername(Bits(in2, 0, 4)),
			index ? "]" : "",
			(index && wback) ? "!" : "");
			
	      }

          } else if((op2 & 0x67) == 0x5) {
              // Load word
              u2_t op1 = Bits(in1, 7, 2);
              u6_t op2 = Bits(in2, 6, 6);
              u4_t Rn = Bits4(in1, 0);
              
              if(op1 == 0 && op2 == 0 && Rn != 15) {
                  writef5("ldr%s.w %s, [%s, %s, lsl #%d]",
                          cond,
                          registername(Bits4(in2, 12)),
                          registername(Rn),
                          registername(Bits4(in2, 0)),
                          Bits(in2, 4, 2));
              }
              else if((op1 == 0 && (((op2 & 0x24) == 0x24) || ((op2 & 0x3C) == 0x30)) && Rn != 15) ||
                        (op1 == 1 && Rn != 15)) {
                  // LDR (immediate, Thumb)
                  if(Bit(in1, 7)) {
                      writef4("ldr%s.w %s, [%s, #%d]",
                              cond,
                              registername(Bits(in2, 12, 4)),
                              registername(Bits(in1, 0, 4)),
                              Bits(in2, 0, 12));
                  } else {
                      bool index = Bit(in2, 10);
                      bool wback = Bit(in2, 9);
                      writef7("ldr%s.w %s, [%s%s, #%d%s%s",
                              cond,
                              registername(Bits4(in2, 12)),
                              registername(Bits4(in1, 0)),
                              index ? "" : "]",
                              Bits(in2, 0, 8),
                              index ? "]" : "",
                              (index && wback) ? "!" : "");
                  }
              } else if((op1 >> 1) == 0 && Rn == 15) {
		writef3("ldr%s.w %s, [pc, #%d]",
			cond,
			registername(Bits4(in2, 12)),
			Bits(in2, 0, 12));
	      }
          } else if((op2 & 0x67) == 0x7) {
              // UNDEFINED
          } else if((op2 & 0x71) == 0x10) {
              // Advanced SIMD element on structure load/store instructions
          } else if((op2 & 0x70) == 0x20) {
              // Data-processing (register)
              u4_t op1 = Bits(in1, 4, 4);
              u4_t op2 = Bits(in2, 4, 4);
              u4_t Rn = Bits(in1, 0, 4);
              u4_t Rd = Bits4(in2, 8);
              u4_t Rm = Bits4(in2, 0);
              
              if(((op1 >> 1) >= 0 && (op1 >> 1) <= 2) && op2 == 0) {
                  // logical shift left, logical shift right, arithmetic shift right
                  u32_t shiftty = ((op1 >> 1) & 3) << 5;
                  writef6("%s%s%s.w %s, %s, %s",
                          shiftname(shiftty),
                          Bit(in1, 4) ? "s" : "",
                          cond,
                          registername(Rd),
                          registername(Rn),
                          registername(Rm));
              } else if((op1 >> 3) == 0 && (op2 >> 3) == 1) {
                  u8_t rotation = Bits(in2, 4, 2) << 3;
                  switch(op1) {
                    case 0: {
                        if(Rn == 15) {
                            writes("sxth");
                        } else {
                            writes("sxath");
                        }
                        break;
                    }
                    case 1: {
                        if(Rn == 15) {
                            // UXTH
                            writes("uxth");
                        }
                        
                        break;
                    }
		    case 5: {
		        if(Rn == 15) {
			    writes("uxtb");
		        }
			break;
		    }
                  }
                  if(Rn == 15) {
                      writef4("%s %s, %s, #%d",                          
                              cond,
                              registername(Rd),
                              registername(Rm),
                              rotation);       
                  } else {
                      writef5("%s %s, %s, %s, #%d",                          
                              cond,
                              registername(Rd),
                              registername(Rn),
                              registername(Rm),
                              rotation);       
                  }           
              } else if((op1 >> 3) == 1 && (op2 >> 2) == 1) {
                  // parallel addition and subtraction, unsigned
                  u3_t op1 = Bits(in1, 4, 3);
                  u2_t op2 = Bits(in2, 4, 2);
                  if(op1 == 0 && op2 == 0) {
                      writef4("uadd8%s %s, %s, %s",
                              cond, 
                              registername(Bits4(in2, 8)),
                              registername(Bits4(in1, 0)),
                              registername(Bits4(in2, 0)));
                  }
                  else 
                      assert(0); // TODO: need to add the rest of instructions
              } else if((op1 >> 2) == 2 && (op2 >> 2) == 2) {
                  // Miscellaneous operations
                  u2_t op1 = Bits(in1, 4, 2);
                  u2_t op2 = Bits(in2, 4, 2);
                  
                  switch(op1) {
                    case 2: {
                        writef4("sel%s %s, %s, %s",
                                cond,
                                registername(Bits4(in2, 8)),
                                registername(Bits4(in1, 0)),
                                registername(Bits4(in2, 0)));
                        break;
                    }
                    case 3: {
                        writef3("clz%s %s, %s",
                                cond,
                                registername(Bits4(in2, 8)),
                                registername(Bits4(in2, 0)));
                        break;
                    }
                      
                    default: 
                      assert(0); // we should never reach this
                  }
              }              
          } else if((op2 & 0x78) == 0x30) {
              // Multiply, multiply accumulate and absolute difference
              u3_t op1 = Bits(in1, 4, 3);
              u2_t op2 = Bits(in2, 4, 2);
              u4_t Ra = Bits(in2, 12, 4);
              
              if(op1 == 0 && op2 == 0) {
                  writef1("%s", Ra != 15 ? "mla" : "mul");
              } else if(op1 == 0 && op2 == 1) {
                  writes("mls");
              }
              writef4("%s %s, %s, %s",
                      cond,
                      registername(Bits4(in2, 8)),
                      registername(Bits4(in1, 0)),
                      registername(Bits4(in2, 0)));
              if(Ra != 15) {
                  writef1(", %s",
                          registername(Ra));

              }
          } else if((op2 & 0x78) == 0x38) {
              // Long multiply, long multiply accumulate and divide
              u3_t op1 = Bits(in1, 4, 3);
              u4_t op2 = Bits(in2, 4, 4);

              switch(op1) {
                case 0: {
                    if(op2 == 0)
                        writes("smull");
                    break;
                }
                case 2: {
                    writes("umull");
                    break;
                }
                case 6: {
                    if(op2 == 0)
                        writes("umlal");
                    break;
                }
              }
              
              writef5("%s %s, %s, %s, %s",
                      cond,
                      registername(Bits4(in2, 12)),
                      registername(Bits4(in2, 8)),
                      registername(Bits4(in1, 0)),
                      registername(Bits4(in2, 0)));
          } else if((op2 & 0x20) == 0x40) {
              // Coprocessor, Advanced SIMD, Floating-point instruction
          }
          break;
      }
    }
}

void armdis::thumb16_instr(u32_t in, u32_t pc) {
    writef4( " %c%c%c%c", 
	     printable( (in >> 0) & 0xff ), 
	     printable( (in >> 8) & 0xff ), 
             printable( (in >> 16) & 0xff ), 
             printable( (in >> 24) & 0xff ) );
    writef1( "  %04x  ", in );

    thumb_instr_ITBlockCond = "";
    if(thumb_instr_InITBlock()) {
        u32_t condition = (thumb_instr_ITSTATE >> 4) & 0xF;
        thumb_instr_ITBlockCond = maskname(condition << 28);
        thumb_instr_ITAdvance();
    }

    u6_t opcode = Bits(in, 10, 6);
        
    if((opcode & 0x30) == 0x0) {
        // shift (immediate), add, subtract, move, and compare
        u3_t mask = (Bits(in, 9, 5) >> 2) & 7;
        switch(mask) {
          case 0:
          case 1:
          case 2: {
            if(opcode == 0 && Bits(in, 6, 3) == 0) {
                writef2("movs %s, %s",
                        registername(Bits(in, 0, 3)),
                        registername(Bits(in, 3, 3)));
            }
            else {
                writef5("%s%s %s, %s, #%d", 
                        shiftname(mask << 5),
                        strlen(thumb_instr_ITBlockCond) > 0 ? thumb_instr_ITBlockCond : "s", 
                        registername(Bits(in, 0, 3)),
                        registername(Bits(in, 3, 3)),
                        Bits(in, 6, 5));
            }
            return;
          }
          case 3: {
            u2_t op = Bits(in, 9, 2);
            writef4("%s%s %s, %s, ",
                    op == 0 || op == 2 ? "add" : "sub",
                    strlen(thumb_instr_ITBlockCond) > 0 ? thumb_instr_ITBlockCond : "s", 
                    registername(Bits(in, 0, 3)),
                    registername(Bits(in, 3, 3)));
            
            switch(op) {
              case 0:
              case 1:
                writef1("%s", registername(Bits(in, 6, 3)));
                break;
              case 2:
              case 3:
                writef1("#%d", Bits(in, 6, 3));
                break;
            }
            
            return;
          }

          case 4: {
            writef3("mov%s %s, #%d",
                    strlen(thumb_instr_ITBlockCond) > 0 ? thumb_instr_ITBlockCond : "s", 
                    registername(Bits(in, 8, 3)),
                    Bits(in, 0, 8));
            return;
          }

          case 5:
            writes("cmp");
            break;
          case 6:
            writef1("add%s", strlen(thumb_instr_ITBlockCond) > 0 ? thumb_instr_ITBlockCond : "s");
            break;
          case 7:
            writef1("sub%s", strlen(thumb_instr_ITBlockCond) > 0 ? thumb_instr_ITBlockCond : "s");
            break;
        }
         
        writef2(" %s, #%d",
                registername(Bits(in, 8, 3)),
                Bits(in, 0, 8));    
    } else if((opcode & 0x3F) == 0x10) {
        // data processing
        u4_t opcode = Bits(in, 6, 4);
        bool flags = true;
        switch(opcode) {
          case 0:
            writes("and");
            break;
          case 1:
            writes("eor");
            break;
          case 2:
            writes("lsl");
            break;
          case 3:
            writes("lsr");
            break;
          case 4:
            writes("asr");
            break;
          case 5:
            writes("adc");
            break;
          case 6:
            writes("sbc");
            break;
          case 7:
            writes("ror");
            break;
          case 8:
            writes("tst");
            flags = false;
            break;
          case 9:
            writes("neg");
            break;
          case 10:
            writes("cmp");
            flags = false;
            break;
          case 11:
            writes("cmn");
            break;
          case 12:
            writes("orr");
            break;
          case 13:
            writes("mul");
            break;
          case 14:
            writes("bic");
            break;
          case 15:
            writes("mvn");
            break;
        }
        writef3("%s %s, %s",
                strlen(thumb_instr_ITBlockCond) > 0 ? thumb_instr_ITBlockCond : (flags ? "s" : ""), 
                registername(Bits(in, 0, 3)),
                registername(Bits(in, 3, 3))); 
    } else if((opcode & 0x3F) == 0x11) {
        u4_t opcode = Bits(in, 6, 4);
        switch(opcode) {
          case 0: 
          case 1:
          case 2: 
          case 3: {
              writes("add");

              u4_t dest = (Bits(in, 7, 1) << 3) | Bits(in, 0, 3);
              writef1(" %s, ", registername(dest));
              
              u3_t Rdn = Bits(in, 0, 3);
              u4_t Rm = Bits(in, 3, 4);
              
              if(Rm == 13) {
                  writef2("%s, %s",
                          registername(13), 
                          registername(dest));
              } else {          
                  writef2("%s, %s",
                          registername(dest),
                          registername(Rm));                  
              }
          
              break;
          }

          case 4: 
          case 5: 
          case 6: 
          case 7: {
              u4_t N = Bits(in, 7, 1);
              u3_t Rn = Bits(in, 0, 3);
              u4_t Rm = Bits(in, 3, 4);
              u4_t n = (N << 3) | Rn;
              writef2("cmp %s, %s",
                      registername(n),
                      registername(Rm));
              
              break;
          }
            
          case 8:
          case 9:
          case 10: 
          case 11: {
              u4_t D = Bits(in, 7, 1);
              u3_t Rd = Bits(in, 0, 3);
              u4_t Rm = Bits(in, 3, 4);
              
              u4_t dest = (Bits(in, 7, 1) << 3) | Bits(in, 0, 3);
              writef2("mov %s, %s", 
                      registername(dest),
                      registername(Bits(in, 3, 4)));
              
              break;
          }
            
          case 12:
          case 13: 
          case 14:
          case 15: {
              if(opcode == 12 || opcode == 13) {
                  writes("bx");
              } else {
                  writes("blx");
              }
              writef2("%s %s", 
                      thumb_instr_ITBlockCond,
                      registername(Bits(in, 3, 4)));

              break;
          } 
        }
    } else if((opcode & 0x3E) == 0x12) {
        // load register literal
        writef3("ldr %s, [%s, #%d]",
                registername(Bits(in, 8, 3)),
                registername(15),
                (Bits(in, 0, 8) << 2));                
    } else if((opcode & 0x34) == 0x14 ||
              (opcode & 0x38) == 0x18 ||
              (opcode & 0x38) == 0x20) {
        // load/store single data item
        u4_t opA = Bits(in, 12, 4);
        u3_t opB = Bits(in, 9, 3);

        switch(opA) {
          case 5: { 
              u4_t Rm = Bits(in, 6, 3);
              u4_t Rn = Bits(in, 3, 3);
              u4_t Rd = Bits(in, 0, 3);
          
              // check for the special case of loading signed byte/halfword
              if(Bits(opB, 0, 2) == 0x3) {
                  if(Bits(opB, 2, 1) == 0) {
                      writes("ldrsb");
                  } else if(Bits(opB, 2, 1) == 1) {
                      writes("ldrsh");
                  }
              } else {
                  writef1("%s", Bits(opB, 2, 1) == 1 ? "ldr" :  "str");
                  switch(Bits(opB, 0, 2)) {
                    case 1:
                      writes("h")
                          break;
                    case 2:
                      writes("b");
                      break;
                  }
              }
              writef3(" %s, [%s, %s]",
                      registername(Rd),
                      registername(Rn),
                      registername(Rm));
          break;
      }
          case 6:
          case 7:
          case 8: {
              writef1("%s", Bits(opB, 2, 1) == 1 ? "ldr" : "str");
              u32_t imm5 = Bits(in, 6, 5);
              switch(opA) {
                case 6:
                  imm5 <<= 2;
                  break;
                case 7:
                  writes("b");
                  break;
                case 8:
                  imm5 <<= 1;
                  writes("h");
                  break;
              }
              
              writef3(" %s, [%s, #%d]",
                      registername(Bits(in, 0, 3)),
                      registername(Bits(in, 3, 3)),
                      imm5);
              
              break;
          }
          case 9: {
              writef1("%s", Bits(opB, 2, 1) == 1 ? "ldr" : "str");
              u32_t imm8 = (Bits(in, 0, 8) << 2);
              writef3(" %s, [%s, #%d]",
                      registername(Bits(in, 8, 3)),
                      registername(13),
                      imm8);
              break;
          }
        }
    } else if((opcode & 0x3E) == 0x28) {
        // generate pc-relative address
        writef3("add %s, %s, #%d",
                registername(Bits(in, 8, 3)),
                registername(15),
                (Bits(in, 0, 8) << 2));
    } else if((opcode & 0x3E) == 0x2A) {
        // generate sp-relative address
        writef3("add %s, %s, #%d",
                registername(Bits(in, 8, 3)),
                registername(13),
                Bits(in, 0, 8) << 2);        
    } else if((opcode & 0x3C) == 0x2C) {
        // miscellaneous 16-bit instructions
        u7_t opcode = Bits(in, 5, 7);
        if((opcode & 0x7C) == 0x0 || 
           (opcode & 0x7C) == 0x4) {
            writef4("%s %s, %s, #%d ", 
                    Bits(in, 7, 1) ? "sub" : "add",
                    registername(13),
                    registername(13),
                    Bits(in, 0, 7) << 2);
        } else if((opcode & 0x7E) == 0x10) {
            // signed extend halfword
            writef2("sxth %s, %s",
                    registername(Bits(in, 0, 3)),
                    registername(Bits(in, 3, 3)));
        } else if((opcode & 0x7E) == 0x12) {
            writef2("sxtb %s, %s",
                    registername(Bits(in, 0, 3)),
                    registername(Bits(in, 3, 3)));
        } else if((opcode & 0x7E) == 0x14) {
            writef2("uxth %s, %s",
                    registername(Bits(in, 0, 3)),
                    registername(Bits(in, 3, 3)));
        } else if((opcode & 0x7E) == 0x16) {
            writef2("uxtb %s, %s",
                    registername(Bits(in, 0, 3)),
                    registername(Bits(in, 3, 3)));

        } else if((opcode & 0x70) == 0x20 || (opcode & 0x70) == 0x60) {
            // push and pop multiple registers
            bool load = Bits(in, 11, 1) ? true : false;
            writef1("%s {", load ? "pop" : "push");
            u16_t registers = 0;
            if(!load) {
                registers = (Bits(in, 8, 1) << 14) | Bits(in, 0, 8);
            } else {
                registers = (Bits(in, 8, 1) << 15)  | Bits(in, 0, 8);
            }
            bool previous = false;
            for(int i = 0; i < 16; i++) {
                if(Bit(registers, i)) {
                    if(previous) {
                        writes(", ");
                    }
                    else {
                        previous = true;
                    }
                    writef1("%s", registername(i));
                }
            }
            writes("}");
        }
        else if((opcode & 0x78) == 0x48 
                || (opcode & 0x78) == 0x08 
                || (opcode & 0x78) == 0x58
                || (opcode & 0x78) == 0x18) {
            if(Bit(in, 11)) writes("cbnz") else writes("cbz");
            writef2(" %s, 0x%x",
                    registername(Bits(in, 0, 3)),
                    pc + ((Bits(in, 9, 1) << 6) | (Bits(in, 3, 5) << 1) | 0) + 4);
        }
        else if((opcode & 0x78) == 0x78) {
          u4_t opA = Bits4(in, 4);
          u4_t opB = Bits4(in, 0);
          if(opB != 0) {
              u32_t mask = opA << 28;
              writef1("it %s",
                      maskname(mask));
              thumb_instr_ITSTATE = opA << 4 | opB;
          } else {
              switch(opA) {
                case 0: 
                    writes("nop");
                    break;
                case 1: 
                    writes("yield");
                    break;
                case 2: 
                    writes("wfe");
                    break;
                case 3: 
                    writes("wfi");
                    break;
                case 4: 
                    writes("sev");
                    break;
              }
          }
        }          
    } else if((opcode & 0x3E) == 0x30) {
        // store multiple registers
        u8_t registers = Bits(in, 0, 8);
        writef2("stmia%s %s!, {",
                thumb_instr_ITBlockCond,
                registername(Bits(in, 8, 3)));
        bool previous = false;
        for(int i = 0; i < 8; i++) {
            if(Bit(registers, i)) {
                if(!previous)
                    previous = true;
                else
                    writes(", ");
                writef1("%s", registername(i));
            }
        }
        writes("}");
    } else if((opcode & 0x3E) == 0x32) {
        // load multiple registers
        u8_t registers = Bits(in, 0, 8);
        writef3("ldm%s %s%s, {",
                thumb_instr_ITBlockCond,
                registername(Bits(in, 8, 3)),
                !(Bit(registers, Bits(in, 8, 3))) ? "!" : "");
        bool previous = false;
        for(int i = 0; i < 8; i++) {
            if(Bit(registers, i)) {
                if(!previous)
                    previous = true;
                else
                    writes(", ");
                writef1("%s", registername(i));
            }
        }
        writes("}");
    } else if((opcode & 0x3C) == 0x34) {
      // conditional branch, and Supervisor Call
      u4_t opcode = Bits(in, 8, 4);
      switch(opcode) {
        case 0xF: {
          writef1("svc #%d", Bits(in, 0, 8));
          break;
        }
        case 0xE: {
          writef1("udf #%d", Bits(in, 0, 8));
          break;

        }

        default: {
          u32_t offset = Bits(in, 0, 8) << 1;
          if(Bits(offset, 8, 1)) offset |= 0xFFFFFE00;
          writef2("b%s.n 0x%x",
                  maskname(Bits(in, 8, 4) << 28),
                  pc + offset + 4);
          break;
        }
      }
    } else if((opcode & 0x3E) == 0x38) {
        // 11100x
        // uncoditional branch
        u32_t offset = Bits(in, 0, 11) << 1;
        if(Bits(offset, 11, 1)) offset |= 0xFFFFF000;
        writef1("b.n 0x%x", pc + offset + 4);
    } else {
        printf("Unknown Thumb16 instruction %08x\n", in);
    }

}

bool armdis::thumb_instr(u32_t in, u32_t pc) {
    u5_t opcode = Bits(in, 11, 5);

    // check to see if this is second part 
    if(thumb32_instr_next) {
        thumb32_instr_next = false;
        thumb32_instr(thumb32_instr_instr, (in & 0xFFFF), pc);
    } else {
        switch(opcode) {
          case 0x1D:
          case 0x1E:
          case 0x1F:
            // it is 32-bit thumb instruction
            thumb32_instr_next = true;
            thumb32_instr_instr = in;
            return false;
          default:
            // it is 16-bit thumb instruction
            thumb16_instr(in, pc);
        } 
    }

    return true;
}

armdis::armdis()
    : thumb_instr_ITSTATE(0),
      thumb_instr_ITBlockCond(NULL),
      thumb32_instr_instr(0x0),
      thumb32_instr_next(false)
{}

// blx to thumb mode://This is encoding T2 of the BLX instruction. Breaking this into the fields identified in the ARM: 
//
//11110  0  0000000010  11  1  0  1  1011011001  0
//       S    imm10H        J1    J2   imm10L 
bool armdis::dis(u32_t loc, u32_t in, char *buffer, int thumb_mode)
{

    noregisterupdate  =  0;
    movetype          =  0;
    movewtype         =  0;

    outputbuffer  =  buffer;
    outputpos     =  0;
    
    if (thumb_mode)
    {
        return thumb_instr(in, loc);
    } else {
        writef4( " %c%c%c%c", 
	     printable( (in >> 0) & 0xff ), 
	     printable( (in >> 8) & 0xff ), 
             printable( (in >> 16) & 0xff ), 
             printable( (in >> 24) & 0xff ) );
        writef1( "  %08x  ", in );
    }
    operandpos  =  outputpos + 8;
    addresspos  =  outputpos + 26;

    if ((in & 0xf0000000u) == 0xf0000000u)
      {
	if ((in & 0x0E000000u) == 0x0a000000u)
	  {
	    printf("Special X mode change to thumb\n");
	  }
      }
    
    
    
    if ((in & 0xF0000000) == 0xF0000000) // F instructions
      {

        if((in & 0xE000000) == 0xA000000) {
	  // BLX with mode change
	  u32_t insOffset = Bits(in, 0, 24);
	  if(Bit(in, 23)) insOffset |= 0xFF000000; // sign extend
	  insOffset = (insOffset << 2) | (Bit(in, 24) << 1) | 0;
	  u32_t dest = loc + insOffset + 0x8; 
	  writef1(" blx 0x%x", dest);
	  return true;
	}
	else switch (in & 0xFFFFFF0) 
	  {
	  case 0x57FF040: // memory barriers
	    writef1(" dsb m=%i", Bits(in, 0, 4));
	    return true;
	  case 0x57FF050:
	    writef1(" dmb m=%i", Bits(in, 0, 4));
	    return true;
	  case 0x57FF060:
	    writef1(" isb m=%i", Bits(in, 0, 4));
	    return true;

	    
	  default:
	    break;
	    // fall through
	  }
      }
    /*
      http://www.altera.com/literature/third-party/archives/ddi0100e_arm_arm.pdf page 68
      
      misc  in 000   24 is 1, 23 is 0, 20 is 0, and 4 is 0                                      0xF90010 == 0x1000000
      extra in 000   24 is 1, 23 is 0, 20 is 0, and 4 is 1 and 7 is 0                           0xF90090 == 0x1000010
      extra in 000   4 is 1 and 7 is 1 - multiplies - extra loads and stores.                   0xE00090 == 0x0000090
    */
    // Special decoding
    else if ((in & 0xF900010) == 0x1000000) return extra_misc_3ab(in);
    else if ((in & 0xF900090) == 0x1000010) return extra_misc_3ab(in);
    else if ((in & 0xE000090) == 0x0000090) return extra_misc_3c(in);
    
    else if ((in & 0xE000010) == 0x6000010) // extends and ubfx/sbfx - bit field extracts.
      {
	u4_t Rd = Bits4(in, 12);
	u4_t Rn = Bits4(in, 0);
	bool unsignedf = Bit(in, 22);



	if ((in &0xF0) == 0x70)// sign extends and extracts
	  {
	    u4_t Radd = Bits4(in, 16);
	    u2_t ror = Bits(in, 10, 2);
	    bool hw = Bit(in, 20);	    
	    const char *n0 = (unsignedf) ? "u" : "s";
	    const char *n1 = (hw) ? "h" : "b";
	    if (Radd == 15) 
	      { writef6("%sxt%s%s %s, %s, ROR #%i",  n0, n1, maskname(in), registername(Rd), registername(Rn), ror * 8); }
	    else
	      { writef7("%sxta%s%s %s, %s, %s, ROR #%i",  n0, n1, maskname(in), registername(Rd), registername(Radd), registername(Rn), ror * 8); }
	  }
	else if ((in &0x70) == 0x30)// byte reverse etc..
	  {
	    u4_t Rd = Bits4(in, 12);
	    u4_t Rn = Bits4(in, 0);
		  /*
    
	    //4:	e6bf1f32 	rev	r1, r2
	    //8:	e6bf3f34 	rev	r3, r4
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
	    const char *nem = 0;
	    int code = 2 * Bit(in, 22) + Bit(in, 7);
	    switch(code)
	      {
	      case 0:  //Reverse byte order in a word.
		nem = "rev";
		break;
	      case 1: // Reverse byte order in each halfword independently.
		nem = "rev16";
		break;
		
	      case 2://    RBIT       Reverse the bit order in a 32-bit word.
		nem = "rbit";
		break;
	      case 3: //REVSH      Reverse byte order in the bottom halfword, and sign extend to 32 bits.
		nem = "revsh";
		break;
	      }
	    writef4("%s%s %s, %s, ",  nem, maskname(in), registername(Rd), registername(Rn));
	  }
	else // bit field extract
	  {
	    int base = Bits(in,  7, 5);
	    int width = Bits(in, 16, 5) + 1;  
            const char *nem;
            if((in & 0x70) == 0x10 && !Bit(in, 21)) 
                // BFI (bit field insert) instruction
                nem = "bfi";
            else
                nem = (unsignedf) ? "ubfx" : "sbfx";
	    writef6("%s%s %s, %s, #%i, #%i",  nem, maskname(in), registername(Rd), registername(Rn), base, width);
	  }
	return true;
      }
    


    u32_t type    =  in & 0xf000000;
    u32_t updown  =  in & 0x800000;
    u32_t n       =  in & 0xfff;




    switch( (int) type )
    {
        case 0x03000000 :
	  if ((in & 0xFb00000) == 0x3200000)
	    {
	      //immediate move to PSR
	      int mask = Bits(in, 16, 4);
	      u4_t rot = Bits4(in, 8);
	      u8_t immed = Bits(in, 0, 8);
	      writef5("msr%s %s,#(0x%x<<%i) (mask=0x%X)",  maskname(in), Bit(in, 22) ? "SPSR": "CPSR", immed, rot*2, mask);
	      break;
	    }
        case 0x01000000 :
        case 0x00000000 :
        case 0x02000000 :

          if((in & 0x0FF00000) == 0x03400000) {
              u32_t rd = (in & 0xF000) >> 12;
              u4_t imm4 = (in & 0xF0000) >> 16;
              u12_t imm12 = in & 0xFFF;
              u16_t top = (((u16_t)imm4) << 12) | imm12;

              writef4("%s%s %s, #%d", "movt", maskname(in), registername(rd), top);
              break;
          }
          else {
              writef2( "%s%s", opcodename( in ), maskname( in ) );
          }

            {
                u32_t op  =  in & 0x1e00000;

                if( ((in & 0x100000) != 0) && 
		    (op != f_cmp) && 
		    (op != f_cmn) && 
		    (op != f_tst) && 
		    (op != f_teq) )
                    wrch( 's' );
            }

            tabto( operandpos );
            
            {
                u32_t r1  =  (in >> 12) & 15;
                u32_t r2  =  (in >> 16) & 15;

                if( movetype || movewtype )
                {
                    writef1( "%s, ", registername( r1 ) );
                }
		else

                if( noregisterupdate )
                {
                    writef1( "%s, ", registername( r2 ) );
                }
                else
                {
                    writef1( "%s, ", registername( r1 ) );
                    
                    if( r1 != r2 )
                        writef1( "%s, ", registername( r2 ) );
                }
            }

            if( (type & 0x2000000) == 0 ) printregisterandshift( in );
            else if (movewtype) printnumber( in & 4095, 0);
	    //	    else printnumber( ((((in << 12) >> 28) << 12) | in & 255), 0 );
	    else printnumber(Bits(in, 0, 8), Bits(in, 8, 4) * 2);
            break;

        case 0x8000000 :
        case 0x9000000 :
            writef3( "%sm%s%s", ((in & 0x100000) != 0) ? "ld" : "st", maskname( in ), f4type( in ) );
            tabto( operandpos );
            writes( registername( in >> 16 ) );

            if( (in & f_wb) != 0 )
                wrch( '!' );

            writes( ", {" );

            {
                u32_t b  =  in & 0xffff;

                if( b == 0 )
                {
                    wrch( '0' );
                }
                else
                {
                    u32_t r0  =  0;
                    u32_t r   =  0;
                    
                    int r0valid       =  0;
                    int commapending  =  0;

                    for( r = 0;  r <= 15;  r++ )
                    {
                        if( (b & 1) != 0 )
                        {
                            if( !r0valid )
                            {
                                if( commapending )  writes( ", " );
                                    
                                writes( registername( r ) );
                                
                                r0       =  r;
                                r0valid  =  1;
                            }
                        }
                        else
                        {
                            if( r0valid )
                            {
                                if( r0 != (r - 1) )
                                {
                                    writef1( "-%s", registername( r - 1 ) );  
                                }
                                
                                r0valid       =  0;
                                commapending  =  1;
                            }
                        }

                        b  =  b >> 1;
                    }
                    
                    if( r0valid  &&  (r0 != (r - 1)) )
                    {
                        writef1( "-%s", registername( r - 1 ) );  
                    }
                }
            }

            wrch( '}' );

            if( (in & f_pc) != 0 )
                wrch( '^' );

            break;

        case 0xa000000 :
        case 0xb000000 :
            writef2( "%s%s", ((in & 0x1000000) != 0) ? "bl" : "b", maskname( in ) );
            tabto( operandpos );
            
            {
                u32_t offset  =  in & 0x00ffffff;
                
                if( offset & 0x00800000 )  offset  |=  0xff000000u;
                    
                writef1( "&%08x", (loc + 8 + (offset << 2)) & 0x03ffffff );
            }

            break;

        case 0xf000000 :
            writef1( "swi%s", maskname( in ) );
            tabto( operandpos );
            printnumber(in & 0xffffff, 0 );

            break;

        case 0xe000000 :
	  if (in & (1<<4)) // copro reg xfer
	    {
	      int cpn = Bits(in, 8, 4);
	      int crn = Bits(in, 16, 4);
	      const char *nem = Bit(in, 20) ? "mrc": "mcr"; 
	      int op1 = Bits(in, 21, 3);
	      int op2 = Bits(in, 5, 3);
	      int Rr = Bits(in, 12, 4);
	      writef7( "%s%s  P%i, op1=%x, %s, crn=%i, op2=%x", nem, maskname( in ), cpn, op1, registername(Rr), crn, op2);
            //tabto( operandpos );
            //printnumber(in & 0xffffff, 0 );
	    }
	  else
	    {
	      writef1( ".copro.%s", maskname( in ) );
            //tabto( operandpos );
            //printnumber(in & 0xffffff, 0 );
	    }

            break;

        case 0x4000000 :
            (void) writef2start( in );
            wrch( ']' );
            
            if( n != 0 )
            {
                writes( ", " );
                printnumber( (updown == 0) ? 0-n : n, 0 );
            }

            break;

        case 0x5000000 :
            {
                u32_t r  =  writef2start( in );
                
                if( updown == 0 )  n  =  0-n;
                
                writes( ", " );
                printnumber( n, 0 );
                wrch( ']' );

                if( (in & f_wb) != 0 )
                    wrch( '!' );
                    
                if( r == 15 )
                {
                    tabto( addresspos );
                    
                    writef1( "(%08x)", loc + 8 + n );
                }
            }

            break;

        case 0x6000000 :
            (void) writef2start( in );
            writes( "], " );

            if( updown == 0 )
                wrch( '-' );

            printregisterandshift( in );

            break;

        case 0x7000000 :
            (void) writef2start( in );
            writes( ", " );

            if( updown == 0 )
                wrch( '-' );

            printregisterandshift( in );
            wrch( ']' );

            if( (in & f_wb) != 0 )
                wrch( '!' );

            break;

        case 0xc000000 :
        case 0xd000000 :
            writes( "dc" );
            tabto( operandpos );
            printnumber( in, 0 );
            
            break;
    }

    return true;
}



char armdis::printable( u32_t value )
{
/*  Return a printable version of the value given  */

    char ch  =  ((char) value) & 0x7f;
    
    return (isgraph( ch ) || (ch == ' ')) ? ch : '.';
}



void armdis::tabto( int n )
{
/*  Tab to a specific position on the line  */

    do
    {
        wrch( ' ' );
    }        
    while( outputpos < n );
}



const char *armdis::registername( u32_t r )
{
  const char *simplenames[]={"r0","r1","r2","r3","r4","r5","r6","r7","r8","r9","r10","r11","r12","r13","lr","pc"};
  return simplenames[(int)(r & 15)];

#if 0
    switch( (int) (r & 15) )
    {
        case 0 :
            return "a1";

        case 1 :
            return "a2";

        case 2 :
            return "a3";

        case 3 :
            return "a4";

        case 4 :
            return "v1";

        case 5 :
            return "v2";

        case 6 :
            return "v3";

        case 7 :
            return "v4";

        case 8 :
            return "v5";

        case 9 :
            return "rfp";

        case 10 :
            return "sl";

        case 11 :
            return "fp";

        case 12 :
            return "ip";

        case 13 :
            return "sp";

        case 14 :
            return "lr";

        case 15 :
            return "pc";

        default :
            return "??";
    }
#endif

}



const char *armdis::shiftname( u32_t sh )
{
    switch( (int) (sh & 0x60) )
    {
        case 0x00 :
            return "lsl";

        case 0x20 :
            return "lsr";

        case 0x40 :
            return "asr";

        case 0x60 :
            return "ror";

        default :
            return "???";
    }
}



const char *armdis::maskname( u32_t m )
{
    switch( (int) (m & 0xf0000000u) )
    {
        case m_eq :
            return "eq";

        case m_ne :
            return "ne";

        case m_geu :
            return "cs";

        case m_ltu :
            return "cc";

        case m_mi :
            return "mi";

        case m_pl :
            return "pl";

        case m_vs :
            return "vs";

        case m_vc :
            return "vc";

        case m_gtu :
            return "hi";

        case m_leu :
            return "ls";

        case m_ge :
            return "ge";

        case m_lt :
            return "lt";

        case m_gt :
            return "gt";

        case m_le :
            return "le";

        case m_always :
            return "";

        case m_never :
            return "nv";

        default :
            return "??";
    }
}



const char *armdis::opcodename( u32_t op )
{
    switch( (int) (op & 0x01e00000) )
    {
        case f_add :
            return "add";

        case f_sub :
            return "sub";

        case f_rsb :
            return "rsb";

        case f_adc :
            return "adc";

        case f_sbc :
            return "sbc";

        case f_rsc :
            return "rsc";

        case f_cmp :
            noregisterupdate  =  1;

            return "cmp";

        case f_cmn :
            noregisterupdate  =  1;

            return "cmn";

        case f_and :
            return "and";

        case f_eor :
            return "eor";

        case f_orr :
            return "orr";

        case f_bic :
            return "bic";

        case f_tst :
            noregisterupdate  =  1;
	    if(Bit(op, 20) == 0) {
	      movewtype = 1;
	      return "movw";
	    }
	    else
	      return "tst";

        case f_teq :
            noregisterupdate  =  1;
            return "teq";

        case f_mov :
            movetype  =  1;
            return "mov";

        case f_mvn :
            movetype  =  1;

            return "mvn";

        default :
            return "???";
    }
}



const char *armdis::f4type( u32_t a )
{
    switch( (int) (a & 0x01800000) )
    {
        case 0x01800000 :
            return "ib";

        case 0x01000000 :
            return "db";

        case 0x00800000 :
            return "ia";

        case 0x00000000 :
            return "da";

        default :
            return "??";
    }
}



void armdis::printregisterandshift( u32_t b )
{
    writes( registername( b ) );

    if( (b & (0xff0)) != 0 )
    {
        writef1( ", %s ", shiftname( b ) );

        if( (b & 0x10) != 0 )
        {
            writes( registername( b >> 8 ) );
        }
        else
            printnumber( (b >> 7) & 0x1f, 0 );
    }
}



u32_t armdis::rotateright( u32_t n, u32_t shift )
{
    while( shift > 0 )
    {
        u32_t bottombit  =  n & 1;

        n  =  n >> 1;

        if( bottombit != 0 )
            n  +=  0x80000000u;

        shift--;
    }

    return n;
}


// ARM immediate constants.
void armdis::printnumber( u32_t offset, u32_t shift )
{
    if( shift > 0 )
        offset  =  rotateright( offset, 2 * shift );

    if( (offset & 0xfffff000u) == 0 )
    {
        writef1( "#%d", offset );
    }
    else if( (offset & 0xfffff000u) == 0xfffff000u )
    {
        writef1( "#-%d", 0-offset );
    }
    else
    {
        writef1( "#0x%08x", offset );
    }
}



u32_t armdis::writef2start( u32_t in ) // load or store or prefetch.
{
  u32_t Rd  =  Bits4(in, 12);
  u32_t r2  =  Bits4(in, 16);
    
  if (Bits4(in, 28) == 15 && Rd == 15) // a prefetch/preload instruction.
    {
      const char *nem = (!Bit(in, 24)) ? "i": (Bit(in, 22)) ?  "d" : "dw";
      writef1("pl%s", nem);
    }
  else
    {
      writef2( "%sr%s", ((in & 0x00100000) != 0) ? "ld" : "st", maskname( in ) );
      if( (in  & 0x00400000) != 0 ) wrch( 'b' );
    }
  tabto( operandpos );
  writef2( "%s, [%s", registername( Rd ), registername( r2 ) );
  
    return r2;
}


// eof

