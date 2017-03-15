//
// Part of prazor-virtual-platform
// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2015 DJ Greaves and Milos Puzovic
// University of Cambridge, Computer Laboratory.

//  armthumb.cpp

#include "armisa.h"

#define THUMB_PC_CORRECTION 2
#define THUMB_DEBUG(X)

#include "ptrace_v1.h"

#define   THUMB2_32_INSTRUCTION_PTRACE(core_no, executedf, pc, ins32) BIN_INSTRUCTION_PTRACE(core_no, ATHUMB2_32_SET, executedf, pc, ins32)
#define   THUMB2_16_INSTRUCTION_PTRACE(core_no, executedf, pc, ins32) BIN_INSTRUCTION_PTRACE(core_no, ATHUMB2_16_SET, executedf, pc, ins32)



// Returns true if it is Thumb32 instruction
// otherwise returns false
bool armisa::DecodeThumbInstruction(bool *executed) {
    bool thumb32 = false;
    insDecodeT = UnknownT;

    bool execute = true;
    bool inITBlock = false;
    if(InITBlock()) {
        u4_t condition = (ITSTATE >> 4) & 0xF;
        DecodeCcode(condition);
        execute = insCcodeTest;
        ITAdvance();
        inITBlock = true;
    }

    // check whether it is 16-bit or 32-bit thumb instruction
    u5_t opcode = Bits(instruction, 11, 5);
    switch(opcode) {
      case 0x1D: // 0b11101
      case 0x1E: // 0b11110
      case 0x1F: // 0b11111
	THUMB2_32_INSTRUCTION_PTRACE(m_core_no, insCcodeTest, Reg(PcReg), opcode); // ptrace_v1
	thumb32 = true;
        *executed = DecodeThumb32(execute);
        break;
      default:
	THUMB2_16_INSTRUCTION_PTRACE(m_core_no, insCcodeTest, Reg(PcReg), opcode); // ptrace_v1
        *executed = DecodeThumb16(execute, inITBlock);
    }

    return thumb32;
}

bool armisa::DecodeThumb16(bool execute, bool inITBlock) {
  bool executed = true;
    if(!execute) {
      executed = false;
      return executed;
    }

    u16_t in = Bits(instruction, 0, 16);
    u6_t opcode = Bits(in, 10, 6);
    THUMB_DEBUG(printf("DecodeThumb16(): Instruction is %04x @ address %08x\n", in, insAddress));

    if((opcode & 0x30) == 0x0) {
        // 00xxxx
        // shift (immediate), add, subtract, move, and compare
        DecodeThumb16ALU1(in, inITBlock);
    } else if((opcode & 0x3F) == 0x10) {
        // 010000
        // data processing
        DecodeThumb16ALU2(in, inITBlock);
    } else if((opcode & 0x3F) == 0x11) {
        // 010001
        // special data instructions and branch and exchange
        DecodeThumb16Special(in);
    } else if((opcode & 0x3E) == 0x12) {
        // 01001x
        DecodeThumb16LDRLiteral(in);
    } else if((opcode & 0x34) == 0x14 ||
              (opcode & 0x38) == 0x18 ||
              (opcode & 0x38) == 0x20) {
        // 0101xx or 011xxx or 100xxx
        // load/store single data item
        DecodeThumb16MemOp(in);
    } else if((opcode & 0x3E) == 0x28) {
        // 10100x
        // generate pc-relative address
        u4_t d = Bits(in, 8, 3);
        u32_t imm32 = Bits(in, 0, 8) << 2;
        Reg(d) = ((Reg(PcReg) - THUMB_PC_CORRECTION) & ~3) + imm32;
	insRd = d;
	insRm = PcReg;
	insDecodeT = ThumbExecutedT;
	stats.m_ALU+=1;
    } else if((opcode & 0x3E) == 0x2A) {
        // 10101x
        // generate sp-relative address
        insS = false;
        insRd = Bits(in, 8, 3);
	insRn = 13; // SP
        insAluOperand1 = Reg(insRn); 
        insAluOperand2 = (Bits(in, 0, 8) << 2);
        insDecodeT = AluT;

        insALUOpcode = 4; // ADD
        
    } else if((opcode & 0x3C) == 0x2C) {
        // 1011xx
        // miscellaneous 16-bit instructions
        DecodeThumb16Misc(in, inITBlock);
    } else if((opcode & 0x3E) == 0x30) {
        // 11000x
        // store multiple registers
        insDecodeT = BlockT;
        insL = false;
        insS = false;
        insW = true;
        insU = true;
        insP = false;
        insMultRegisters = Bits(in, 0, 8);
        insRn = Bits(in, 8, 3);
        insSigned = false;
        
        runstate = MidMultipleS;
    } else if((opcode & 0x3E) == 0x32) {
        // 11001x
        // load multiple registers
        insDecodeT = BlockT;
        insL = true;
        insS = false;
        insMultRegisters = Bits(in, 0, 8);
        insRn = Bits(in, 8, 3);
        insU = true;
        insP = false;
        insW = !Bit(insMultRegisters, insRn);
        insSigned = false;

        runstate = MidMultipleS;        
    } else if((opcode & 0x3C) == 0x34) {
        // 1101xx
        // conditional branch, and Supervisor Call
        DecodeThumb16BranchAndSVC(in);
    } else if((opcode & 0x3E) == 0x38) {
        // 11100x
        // unconditional branch
        insL = false;
        u32_t offset = Bits(in, 0, 11) << 1;
        if(Bits(offset, 11, 1)) offset |= 0xFFFFF000;
        u32_t dest = Reg(PcReg) + offset - THUMB_PC_CORRECTION;
	OPERAND_PTRACE(dest);
	insDecodeT = ThumbExecutedT;
        ExecuteBL(dest);
    } else {
        printf("Unknown Thumb16 instruction %08x @ address %08x\n", instruction, insAddress);
    }

    return executed;
}


void armisa::DecodeThumb16Misc(u16_t in, bool inITBlock) {
    u7_t opcode = Bits(in, 5, 7);

    if((opcode & 0x7C) == 0x0 || 
       (opcode & 0x7C) == 0x4) {
        // Add immediate to SP
        // Subtract immediate to SP
        insS = false;
        insDecodeT = AluT;
	insRn = 13;
        insAluOperand1 = Reg(insRn); // SP
        insAluOperand2 = Bits(in, 0, 7) << 2;
        insRd = 13;

        insALUOpcode = Bits(in, 7, 1) ? 2 /* SUB */ : 4; /* ADD*/
    } else if((opcode & 0x7E) == 0x10) {
        // signed extend halfword
        insS = false;
        insDecodeT = AluT;
        insRd = Bits(in, 0, 3);
	insRn = Bits(in, 3, 3);
        insAluOperand2 = Reg(insRn) & 0xFFFF;
        if(Bits(insAluOperand2, 15, 1)) insAluOperand2 |= 0xFFFF0000;
        
        insALUOpcode = 13; // MOV        
    } else if((opcode & 0x7E) == 0x12) {
        // signed extend bytes
        insS = false;
        insDecodeT = AluT;
        insRd = Bits(in, 0, 3);
	insRn = Bits(in, 3, 3);
        insAluOperand2 = Reg(insRn) & 0xFF;
        if(Bits(insAluOperand2, 7, 1)) insAluOperand2 |= 0xFFFFFF00;
        
        insALUOpcode = 13; // MOV
    } else if((opcode & 0x7E) == 0x14) {
        // unsigned extend halword
        insS = false;
        insDecodeT = AluT;
        insRd = Bits(in, 0, 3);
	insRn = Bits(in, 3, 3);
        insAluOperand2 = Reg(insRn) & 0xFFFF;
        
        insALUOpcode = 13; // MOV

    } else if((opcode & 0x7E) == 0x16) {
        // unsigned extend byte
        insS = false;
        insDecodeT = AluT;
        insRd = Bits(in, 0, 3);
	insRn = Bits(in, 3, 3);
        insAluOperand2 = Reg(insRn) & 0xFF;
        
        insALUOpcode = 13; // MOV
    } else if((opcode & 0x70) == 0x20 || (opcode & 0x70) == 0x60) {
        // push and pop multiple registers
        insDecodeT = BlockT;
        insL = Bits(in, 11, 1) ? true : false;
        insS = false;
        insW = true;
        insMultRegisters = 0;
        if(!insL) {
            insP = true;
            insU = false;
            insMultRegisters = 
                (Bits(in, 8, 1) << 14) |
                Bits(in, 0, 8);
        } else {
            insP = false;
            insU = true;
            insSigned = false;
            insMultRegisters =
                (Bits(in, 8, 1) << 15)  |
                Bits(in, 0, 8);
        }

        insRn = 13; // we are pushing to stack pointer

        runstate = MidMultipleS;        
    }
    else if((opcode & 0x78) == 0x48 
            || (opcode & 0x78) == 0x08 
            || (opcode & 0x78) == 0x58 
            || (opcode & 0x78) == 0x18) {
        assert(!inITBlock);
        // CBNZ, CBZ
        u1_t op = Bit(in, 11);
        u32_t i = Bit(in, 9);
        u32_t imm5 = Bits(in, 3, 5);
        insRn = Bits(in, 0, 3);

        u1_t nonzero = op == 1;
        u32_t offset =  (i << 6) | (imm5 << 1) | 0;
	insDecodeT = ThumbExecutedT;
        if(nonzero ^ (Reg(insRn) == 0)) {
            u32_t dest = Reg(PcReg) + offset - THUMB_PC_CORRECTION;
            insL = false;
	    OPERAND_PTRACE(dest);
            ExecuteBL(dest);
        } else {
	  stats.m_Branches+=1;
	}
	
    }
    else if((opcode & 0x78) == 0x78) {
        DecodeThumb16IfThenHints(in, inITBlock);
    } else if((opcode & 0x7E) == 0x50) {
        // REV
        u32_t Rm = Reg(Bits(in, 3, 3));
        insRd = Bits(in, 0, 3);
	insRn = Rm;
        insAluResult = ((Rm>>24) & 0xFF) | ((Rm>>8) & 0xFF00) | ((Rm<<8) & 0xFF0000) | ((Rm<<24) & 0xFF000000);

        Reg(insRd) = insAluResult;
	OPERAND_PTRACE(insAluResult);
	insDecodeT = ThumbExecutedT;
	stats.m_ALU+=1;
    } else {
        printf("Unknown instruction %04x @ address %08x\n", in, insAddress);
        assert(0);
    }
}

void armisa::DecodeThumb16LDRLiteral(u16_t in) {
    insDecodeT = SingleT;
    insDataOffset = Bits(in, 0, 8) << 2;
    insRd = Bits(in, 8, 3);
    insRn = PcReg;
    u32_t base = (Reg(PcReg) - THUMB_PC_CORRECTION) & ~3;
    insDataAddress = base + insDataOffset;
    insSizeCode = 2;
    insEXCF = false;
    insSUser = false;
    insSigned = false;
    insL = true;
    insW = false;
    insP = true;

    runstate = MidSingleS;
}

void armisa::DecodeThumb16ALU1(u16_t in, bool inITBlock) {
    insDecodeT = AluT;
    u5_t opcode = Bits(in, 9, 5);
    insS = !inITBlock;
    u3_t mask = (opcode >> 2) & 0x7;
    
    switch(mask) {
      case 0:
      case 1:
      case 2: {
          if(opcode == 0 && Bits(in, 6, 3) == 0) {
              insS = true;
              insRd = Bits(in, 0, 3);
	      insRn = Bits(in, 3, 3);
              insAluOperand2 = Reg(insRn);
              insALUOpcode = 13;
          }
          else {
              insRd = Bits(in, 0, 3);
              insRm = Bits(in, 3, 3);
              insShiftType = mask;
              insShiftComputed = false;
              insShiftValue = Bits(in, 6, 5);
              
              exec_shift_operand_instr();
              
              insALUOpcode = 13; // MOV
          }
          break;
      }
      case 3: {
          u2_t op = Bits(in, 9, 2);
          insRd = Bits(in, 0, 3);
	  insRn = Bits(in, 3, 3);
          insAluOperand1 = Reg(insRn);

          switch(op) {
            case 0: {
                // add register
  	        insRm = Bits(in, 6, 3);	       
                insAluOperand2 = Reg(insRm);
                insALUOpcode = 4; // ADD
                break;
            }
            case 1: {
                // subtract register
   	        insRm = Bits(in, 6, 3);
                insAluOperand2 = Reg(insRm);
                insALUOpcode = 2; // SUB
                break;
            }
            case 2: {
                // add 3-bit immediate
	        insAluOperand2 = Bits(in, 6, 3);
                insALUOpcode = 4; // ADD
                break;
            }
            case 3: { 
                // subtract 3-bit immediate
 	        insAluOperand2 = Bits(in, 6, 3);
                insALUOpcode = 2; // SUB
                break;
            }
          }
          break;
      }

      case 4: {
          // MOV instruction
          insAluOperand2 = Bits(in, 0, 8);
          insRd = Bits(in, 8, 3);
          insALUOpcode = 13; // MOV
          
          break;
      }

      case 5: {
          // CMP instruction
	  insS = true;
	  insRn = Bits(in, 8, 3);
          insAluOperand1 = Reg(insRn);
          insAluOperand2 = Bits(in, 0, 8);
          insALUOpcode = 10; // CMP

          break;
      }

      case 6: {
          // Add 8-bit immediate
          insRd = Bits(in, 8, 3);
	  insRn = Bits(in, 8, 3);
          insAluOperand1 = Reg(insRn);
          insAluOperand2 = Bits(in, 0, 8);
          insALUOpcode = 4; // ADD

          break;
      }

      case 7: {
          // Subtract 8-bit immediate
          insRd = Bits(in, 8, 3);
	  insRn = Bits(in, 8, 3);
          insAluOperand1 = Reg(insRn);
          insAluOperand2 = Bits(in, 0, 8);
          insALUOpcode = 2; // SUB

          break;

      }

      default: {
          assert(0); // we should never hit this
      }
    }
}

void armisa::DecodeThumb16ALU2(u16_t in, bool inITBlock) {
    insDecodeT = AluT;
    insS = !inITBlock;
    insRd = Bits(in, 0, 3);
    insRm = Bits(in, 0, 3);
    insRn = Bits(in, 3, 3);
    insAluOperand1 = Reg(insRm);
    insAluOperand2 = Reg(insRn);

    // for shift operations
    insShiftComputed = true;
    insShiftValue = insAluOperand2;

    u4_t opcode = Bits(in, 6, 4);
    switch(opcode) {
      case 2: {
          // logical shift left
          insShiftType = 0;
          exec_shift_operand_instr();
          insALUOpcode = 13; // MOVE
          break;
      }

      case 3: {
          // logical shift right
          insShiftType = 1;
          exec_shift_operand_instr();
          insALUOpcode = 13; // MOVE
          break;
      }

      case 4: {
          // arithmetic shift right
          insShiftType = 2;
          exec_shift_operand_instr();
          insALUOpcode = 13; // MOVE
          break;
      }

      case 7: {
          // rotate right (ROR)
          insShiftType = 3;
          exec_shift_operand_instr();
          insALUOpcode = 13; // MOVE
          break;
      }
        
      case 9: {
          // reverse subtract from 0 (RSB);
          insAluOperand1 = insAluOperand2;
          insAluOperand2 = 0;
          insALUOpcode = 3;
          break;
      }

      case 13: {
          insDecodeT = MulT;
          insMultAccum = 0;
          insMultOp = MULT_Unsigned;
         
          break;
      }

      default: {
	  // For TST, CMP and CMD we need to update flags always
	  if(opcode == 8 || opcode == 10 || opcode == 11)
	      insS = true;
          insALUOpcode = opcode;
          break;
      }
    } 
    
}


void armisa::DecodeThumb16MemOp(u16_t in) {
    u4_t opA = Bits(in, 12, 4);
    u3_t opB = Bits(in, 9, 3);

    insDecodeT = SingleT;
    insEXCF = false;
    insW = false;
    insS = true;
    insSUser = false;
    insU = true;
    insP = true;
    insSigned = false;

    switch(opA) {
      case 5: { // 0b0101
          insRm = Bits(in, 6, 3);
          insRn = Bits(in, 3, 3);
          // destination register
          insRd = Bits(in, 0, 3);
          insDataAddress = Reg(insRm) + Reg(insRn);
          // check for the special case of loading signed byte/halfword
          if(Bits(opB, 0, 2) == 0x3) {
              insL = true;
              if(Bits(opB, 2, 1) == 0) {
                  // load register signed byte
                  insSigned = true;
                  insSizeCode = 0;
              } else if(Bits(opB, 2, 1) == 1) {
                  // load register signed halfword
                  insSigned = true;
                  insSizeCode = 1;
              } else {
                  printf("Unknown instruction %08x @ address %08x\n", in, insAddress);
              }
          } else {
              insL = Bits(opB, 2, 1) == 1;
              switch(Bits(opB, 0, 2)) {
                case 0:
                  insSizeCode = 2;
                  break;
                case 1:
                  insSizeCode = 1;
                  break;
                case 2:
                  insSizeCode = 0;
                  break;
                default:                 
                  assert(0); // we should never hit this
              }
          }
          break;
      }
      case 6:
      case 7:
      case 8: {
          u32_t imm5 = Bits(in, 6, 5);
          switch(opA) {
            case 6:
              imm5 <<= 2;
              insSizeCode = 2;
              break;
            case 7:
              insSizeCode = 0;
              break;
            case 8:
              imm5 <<= 1;
              insSizeCode = 1;
              break;
            default:
              assert(0); // should never reach this
          }
          insRd = Bits(in, 0, 3);
          insRn = Bits(in, 3, 3);
          insDataAddress = Reg(insRn) + imm5;
          insL = Bits(opB, 2, 1) == 1;

          break;
      }
      case 9: {
          u32_t imm8 = (Bits(in, 0, 8) << 2);
          insRd = Bits(in, 8, 3);
	  insRn = 13;
          insDataAddress = Reg(insRn) + imm8;
          insSizeCode = 2;
          insL = Bits(opB, 2, 1) == 1;

          break;
      }

      default: {
          printf("Unknown instruction %08x @ address %08x\n", in, insAddress);
          assert(0);
      }
    }
    OPERAND_PTRACE(insDataAddress);              
    runstate = MidSingleS;
}

void armisa::DecodeThumb16BranchAndSVC(u16_t in) {
    u4_t opcode = Bits(in, 8, 4);
    switch(opcode) {
      case 0xF: {
          // Supervisor call (SVC) or swi.
          insDecodeT = SwiT;
          insSWIArg = Bits(in, 0, 8);
	  break;
      }

      case 0xE: {
          // Permanently undefined (UDF)
          // TODO
          assert(0);
          break;

      }

      default: {
          insL = false; // Not a branch and link
          u32_t offset = Bits(in, 0, 8) << 1;
          if(Bits(offset, 8, 1)) offset |= 0xFFFFFE00;
          DecodeCcode(opcode);
	  u32_t dest = Reg(PcReg) + offset - THUMB_PC_CORRECTION;

	  u32_t rc = 0xFFFFffff;
	  if(insCcodeTest && m_abtb)
	    rc = m_abtb->query(dest, &Reg(CpsrReg));
	  if(rc != 0xFFFFffff) {
	    predictor_op((Reg(PcReg)>>1)-2, insCcodeTest, dest>>1);
	  }
	  if(rc != 0xFFFFffff)
	    dest = rc;
	  
	  insDecodeT = ThumbExecutedT;	  
          if(insCcodeTest) {
	    ExecuteBL(dest);
	  } else {
	    stats.m_Branches+=1;
	  }
          break;
      }
    }
}

void armisa::DecodeThumb16Special(u16_t in) {
    u4_t opcode = Bits(in, 6, 4);
    switch(opcode) {
      case 0: 
      case 1:
      case 2: 
      case 3: {
          // add registers
          u3_t Rdn = Bits(in, 0, 3);
          u4_t Rm = Bits(in, 3, 4);
          u4_t DN = Bits(in, 7, 1);
          u4_t d = (DN << 3) | Rdn;
          insS = false; // we are not updating flags
          insRd = d;
          insDecodeT = AluT;

          if(Rm == 13) {
	      insRn = 13;
	      insRm = d;
              insAluOperand1 = Reg(13);
              insAluOperand2 = Reg(d);
          } else {
	      insRn = d;
	      insRm = Rm;
              insAluOperand1 = Reg(d);
              insAluOperand2 = Reg(Rm);
              
          }

          insALUOpcode = 4; // ADD
          
          break;
      }

      case 4: 
      case 5: 
      case 6: 
      case 7: {
          // compare high registers
          insS = true;
          insDecodeT = AluT;
          u4_t N = Bits(in, 7, 1);
          u3_t Rn = Bits(in, 0, 3);
          u4_t Rm = Bits(in, 3, 4);
          u4_t n = (N << 3) | Rn;
	  insRn = n;
	  insRm = Rm;
          insAluOperand1 = Reg(n);
          insAluOperand2 = Reg(Rm);
          
          insALUOpcode = 10; //CMP
          
          break;
      }

      case 8:
      case 9:
      case 10: 
      case 11: {
          // move high and low registers
          insS = false;
          u4_t D = Bits(in, 7, 1);
          u3_t Rd = Bits(in, 0, 3);
          u4_t Rm = Bits(in, 3, 4);

          insRd = (D << 3) | Rd;
	  insRn = Rm;
          insAluOperand2 = Reg(Rm);
          
          insDecodeT = AluT;
          insALUOpcode = 13; // MOV

          break;
      }

      case 12:
      case 13: 
      case 14:
      case 15: {
          u32_t dest = Reg(Bits4(in, 3));
	  if(Bits4(in,3) == 15)
	    dest -= 2;
          if(dest & 1) {
              Reg(CpsrReg) |= (1 << 5);
              dest = dest & ~1;
          } else if(!((dest >> 1) & 1)) {
              Reg(CpsrReg) &= ~(1 << 5);
          } else {
              // it means that last two dest<1:0> = 10
	    printf("Bad opcode 0x%X, dest=0x%X\n", in, dest);
	    assert(0); // we should not hit his
          }
	  // This is failing  'bx  pc'  Bad opcode 0x4778, dest=0x459B6


          insL = false;
          if(Bit(in, 7)) { // BLX - why do we clear the insL flag and do the linking here instead of in ExecuteBL - the pc correction here is -4 and in Execute BL it is -2.  Why ?
	    insL = true;
	    Reg(PcReg) -= 2;
	    //Reg(LrReg) = ((Reg(PcReg) - 4) & ~1) | 1;
	  }

	  u32_t rc = 0xFFFFffff;
	  if(m_abtb)
	    rc = m_abtb->query(dest, &Reg(CpsrReg));
	  if(rc != 0xFFFFffff)
	    predictor_op((Reg(PcReg)>>1)-2, insCcodeTest, dest>>1, Bit(in, 7));
	  if(rc != 0xFFFFffff)
	    dest = rc;

	  insDecodeT = ThumbExecutedT;
	  OPERAND_PTRACE(dest);              
          ExecuteBL(dest);

          break;
      }

      default: {
          assert(0); // we should never reach this
      }
        
    }
}

void armisa::DecodeThumb16IfThenHints(u16_t in, bool inITBlock) {
    u4_t opA = Bits4(in, 4);
    u4_t opB = Bits4(in, 0);

    if(opB != 0) {
        // If-Then
        assert(opB != 0);
        assert(!inITBlock);
        ITSTATE = opA << 4 | opB;
	insDecodeT = ThumbExecutedT;
	stats.m_Branches+=1;
    } else {
        switch(opA) {
          case 0: {
              // NOP
              // Do nothing
              break;
          }
          case 1: {
	    insDecodeT = ThumbExecutedT;
	    stats.m_Unknowns+=1;
	    iss_yield(-1);
	    break;
          }
          case 2: {
              assert(0); // WFE
              break;
          }
          case 3: {
              assert(0); // WFI
              break;
          }
          case 4: {
              assert(0); // SEV
              break;
          }
          default: 
            assert(0); // we should never reach this
        }
    }
}

bool armisa::DecodeThumb32(bool execute) {
    bool executed = true;
    // we need to read next 16 bits to decode this instruction
    int c = armisa_read1(insAddress+2, 1, true);
    assert(c);
    u16_t instruction2 = read_data;
    THUMB_DEBUG(printf("DecodeThumb32(): Instruction is %04x %04x @ address %08x\n", instruction, instruction2, insAddress));

    if(!execute) {
        Reg(PcReg) += 2;
	executed = false;
        return executed;
    }

    if(m_abtb) {
      m_abtb->ins32_monitor((instruction << 16) | instruction2, Reg(12), Reg(PcReg));
    }
    
    u2_t op1 = Bits(instruction, 11, 2);
    u7_t op2 = Bits(instruction, 4, 7);
    u1_t op = Bits(instruction2, 15, 1);

    switch(op1) {
      case 1: {
          if((op2 & 0x64) == 0) {
              // Load/store multiple
              DecodeThumb32LDRSTRMultiple(instruction, instruction2);
          } else if((op2 & 0x64) == 0x04) {
              // Load/store dual, load/store exclusive, table branch
              DecodeThumb32LDRSTRDualExclusive(instruction, instruction2);
          } else if((op2 & 0x60) == 0x20) {
              // Data-processing (shifted register)
              DecodeThumb32ALUShiftedRegister(instruction, instruction2);
          } else if((op2 & 0x40) == 0x40) {
              // Coprocessor, Advanced SIMD, and Floating-point instructions
	      DecodeThumb32CPRSIMDFloatInstrs(instruction, instruction2);
          } else {
              assert(0); // we should never reach this
          }
          break;
      }
      case 2: {
          if((op2 & 0x20) == 0x0 && op == 0) {
              // Data-processing (modified immediate)
              DecodeThumb32DataProcessingModifedImmediate(instruction, instruction2);
          } else if((op2 & 0x20) == 0x20 && op == 0) {
              // Data-processing (plain binary immediate)
              DecodeThumb32DataProcessingImmediate(instruction, instruction2);
          } else if(op == 1) {
              // Branches and miscellaneous control
              DecodeThumb32Branches(instruction, instruction2);
              return executed;
          } else {
              assert(0); // we should never reach this
          }
          break;
      }
      case 3: {
          if((op2 & 0x71) == 0x0) {
              // Store single data item
              DecodeThumb32STRSingleData(instruction, instruction2);
          } else if((op2 & 0x67) == 0x1) {
              // Load byte, memory hints
	      DecodeThumb32LoadByteMemoryHints(instruction, instruction2);
          } else if((op2 & 0x67) == 0x3) {
              // Load halfword, memory hints
              DecodeThumb32LoadHlfWord(instruction, instruction2);
          } else if((op2 & 0x67) == 0x5) {
              // Load word
              DecodeThumb32LoadWord(instruction, instruction2);
          } else if((op2 & 0x67) == 0x7) {
              // UNDEFINED
              assert(0);
          } else if((op2 & 0x71) == 0x10) {
              // Advanced SIMD element on structure load/store instructions
              assert(0);
          } else if((op2 & 0x70) == 0x20) {
              // Data-processing (register)
              DecodeThumb32DataProcessingRegister(instruction, instruction2);
          } else if((op2 & 0x78) == 0x30) {
              // Multiply, multiply accumulate and absoulte difference
              DecodeThumb32Multiply(instruction, instruction2);
          } else if((op2 & 0x78) == 0x38) {
              // Long multiply, long multiply accumulate and divide
              DecodeThumb32LongMultiply(instruction, instruction2);
          } else if((op2 & 0x20) == 0x40) {
              // Coprocessor, Advanced SIMD, Floating-point instruction
              assert(0);
          } else {
	    tenos_diagnostics.checkpoint(2, myname(), "thumb unreachable");
	    assert(0); // we should never reach this
          }
          break;
      }
      default: 

        assert(0); // we should never hit this
    }

    Reg(PcReg) += 2;
    return executed;
}

void armisa::DecodeThumb32Branches(u16_t in1, u16_t in2) {
    u7_t op = Bits(in1, 4, 7);
    u3_t op1 = Bits(in2, 12, 3);
    u4_t op2 = Bits(in2, 8, 4);
    
    if(((op1 & 0x5) == 0) && (op & 0x38) != 0x38) {
        assert(!Bit(in2, 12));
        // check to see if we are going to execute it
        u4_t cond = Bits(in1, 6, 4);
        DecodeCcode(cond);
        if(insCcodeTest) {
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
            u32_t dest = Reg(PcReg) + offset - THUMB_PC_CORRECTION;
            insL = false;

	    u32_t rc = 0xFFFFffff;
	    if(m_abtb)
	      rc = m_abtb->query(dest, &Reg(CpsrReg));
	    if(rc != 0xFFFFffff) {
	      predictor_op((Reg(PcReg)>>1)-2, insCcodeTest, dest>>1, insL);
	    }
	    if(rc != 0xFFFFffff)
	      dest = rc;
	    
	    OPERAND_PTRACE(dest);
	    insDecodeT = ThumbExecutedT;
            ExecuteBL(dest);
        }
        else {
	  /* Branch not taken */
	  insDecodeT = ThumbExecutedT;
	  stats.m_Branches+=1;
	  Reg(PcReg) += 2;
        }
    }
    else if((op1 & 5) == 5 || (op1 & 5) == 4 || (op1 & 5) == 1) {
        // Branch with link and Branch with link and exchange
        // Uncoditional branch
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
        bool is_thumb = !!Bit(in2, 12);
        Reg(CpsrReg) = (Reg(CpsrReg) & ~(1 << 5)) | (Bit(in2, 12) << 5);
        u32_t base = Reg(PcReg) - THUMB_PC_CORRECTION;
        if(!is_thumb)
            base &= ~3; // align
        u32_t dest = base + offset;
	if(Bit(in2, 14)) 
	   insL = true;
	else
	   insL = false;

	u32_t rc = 0xFFFFffff;
	if(m_abtb)
	  rc = m_abtb->query(dest, &Reg(CpsrReg));
	if(rc != 0xFFFFffff) {
	  predictor_op((Reg(PcReg)>>1)-2, insCcodeTest, dest>>1, insL?1:0);
	}
	if(rc != 0xFFFFffff)
	  dest = rc;

	insDecodeT = ThumbExecutedT;
        ExecuteBL(dest);
	OPERAND_PTRACE(dest);              
    } else {
        printf("Not yet implemented instruction %04x %04x\n", in1, in2);
        assert(0);
    }
}

void armisa::DecodeThumb32DataProcessingRegister(u16_t in1, u16_t in2) {
    u4_t op1 = Bits(in1, 4, 4);
    u4_t op2 = Bits(in2, 4, 4);
    u4_t Rn = Bits(in1, 0, 4);
    insS = false;

    if(((op1 >> 1) >= 0 && (op1 >> 1) <= 2) && op2 == 0) {
        // logical shift left, logical shift right, arithmetic shift right
        insDecodeT = AluT;
        insRd = Bits4(in2, 8);
        insRm = Bits4(in1, 0);
        insS = (Bit(in1, 4) == 1);
        insShiftComputed = true;
        insShiftType = (op1 >> 1) & 3;
        insShiftValue = Reg(Bits4(in2, 0));
	insRs = Bits4(in2, 0);

        exec_shift_operand_instr();

        insALUOpcode = 13; // MOV

    } else if((op1 >> 1) == 3 && op2 == 0) {
        // rotate right
        assert(0);
    } else if((op1 >> 3) == 0 && (op2 >> 3) == 1) {
        u8_t rotation = Bits(in2, 4, 2) << 3;
        insRd = Bits4(in2, 8);
        u4_t m = Bits4(in2, 0);
	insRm = m;
        u32_t rotated = Ror(Reg(m), rotation);

        switch(op1) {
          case 0: {
              u32_t val = rotated & 0xFFFF;
              if(val & 0x8000) val |= 0xFFFF0000;

              if(Rn == 15) {
                  // SXTH
                  Reg(insRd) = val;
              } else {
                  // SXATH
		  insRn = Rn;
                  Reg(insRd) = Reg(Rn) + val;
              }
              break;
          }
          case 1: {
              if(Rn == 15) {
                  // UXTH
                  Reg(insRd) = rotated & 0xFFFF;
                  
              } else {
                  assert(0); // UXTAH
              }

              break;
          }
          case 2: {
              assert(0); // SXTAB16/SXTB16
          }
          case 3: {
              assert(0); // UXTAB16/UXTB16
          }
          case 4: {
              assert(0); // SXTAB/SXTB
          }
          case 5 : {
	      if(Rn == 15) {
	          // UXTB
		  Reg(insRd) = rotated & 0xFF;
	      } else {
		  // UXTAB
		assert(0);
	      }
	      break;
          }
          default: {
              assert(0); // we should never reach this
          }	   	    
          OPERAND_PTRACE(Reg(insRd));              
        }

	insDecodeT = ThumbExecutedT;
	stats.m_ALU+=1;
    } else if((op1 >> 3) == 1 && (op2 >> 2) == 0) {
        // Parallel addition and subtraction, signed 
        printf("Instruction %04x %04x @ address %08x not yet implemented\n", in1, in2, insAddress); 
        assert(0);
    } else if((op1 >> 3) == 1 && (op2 >> 2) == 1) {
        // Parallel addition and subtraction, unsinged
        DecodeThumb32UnsignedParallelAddAndSub(in1, in2);
    } else if((op1 >> 2) == 2 && (op2 >> 2) == 2) {
        // Miscellaneous operations
        DecodeThumb32Misc(in1, in2);
    } else {
        assert(0); // we should never reach this
    }
}

void armisa::DecodeThumb32Multiply(u16_t in1, u16_t in2) {
    u3_t op1 = Bits(in1, 4, 3);
    u2_t op2 = Bits(in2, 4, 2);
    u4_t Ra = Bits(in2, 12, 4);
    insDecodeT = MulT;
    insMultAccum = 0;
    insAluOperand1 = Reg(Bits(in1, 0, 4));
    insAluOperand2 = Reg(Bits(in2, 0, 4));
    insRd = Bits(in2, 8, 4);
    insS = false;
    insMultOp = MULT_Unsigned;
    
    if(op1 == 0 && op2 == 0) {
        // we do not update flags
        if(Ra != 15) {
            // multiply accumulate
            insMultAccum = Reg(Ra);
	    insRn = Ra;
        }
    } else if(op1 == 0 && op2 == 1) {
        // MLS (multiply and subtract)
        insMultAccum = Reg(Ra);
	insRn = Ra;
        insAluOperand1 *= -1;
    } else {
        // multiply and subtract
        // signed multiples
        printf("Instruction %04x %04x @ address %08x not yet implemented.\n", in1, in2, insAddress);
	tenos_diagnostics.checkpoint(2, myname(), "thumb multiplies");
        assert(0); 
    }
}

void armisa::DecodeThumb32ALUShiftedRegister(u16_t in1, u16_t in2) {
    u4_t op = Bits(in1, 5, 4);
    u4_t Rn = Bits(in1, 0, 4);
    u4_t Rd = Bits(in2, 8, 4);
    u1_t S = Bit(in1, 4);
    u5_t RdS = (((u5_t)Rd) << 1) | S;

    insRm = Bits4(in2, 0);
    insShiftType = Bits(in2, 4, 2);
    u32_t imm3 = Bits(in2, 12, 3);
    u32_t imm2 = Bits(in2, 6, 2);
    insS = S;
    insShiftValue = (imm3 << 2) | imm2;
    insShiftComputed = false;
    
    // this gives us insAluOperand2
    exec_shift_operand_instr();
    insAluOperand1 = Reg(Rn);
    insRn = Rn;
    insDecodeT = AluT;
    insRd = Rd;

    switch(op) {
      case 0: {
          if(RdS != 0x1F) {
              insALUOpcode = 0; // AND
          } else {
              insALUOpcode = 8; // TST
          }
          break;
      }
      case 1: {
          insALUOpcode = 14; // BIC
          break;
      }
      case 2: {
          if(Rn != 15) {
              // ORR
              insALUOpcode = 12;
          } else {
              // Move register and immediate shifts
              DecodeThumb32MoveRegisterAndImmShifts(in1, in2);
          }
          break;
      }
      case 3: {
          if(Rn != 15) {
              // ORN
              insAluOperand2 = ~insAluOperand2;
              insALUOpcode = 12;
          } else {
              // MVN
              insALUOpcode = 15;
          }
          break;
      }
      case 4: {
          if(RdS != 0x1F) {
              // EOR
              insALUOpcode = 1;
          } else {
              // TEQ
              insALUOpcode = 9;
          }
          break;
      }
      case 6: {
          assert(0); // PKH
      }
      case 8: {
          if(RdS != 0x1F) {
              // ADD
              insALUOpcode = 4; // ADD
          }
          else {
              assert(0); // CMN
          }
          break;
      }
      case 10: {
          insALUOpcode = 5;
          break;
      }
      case 11: {
          // SBC
          insALUOpcode = 6;
          break;
      }
      case 13: {
          if(RdS != 0x1F) {
              // SUB
              insALUOpcode = 2; 
          } else {
              // CMP
              insALUOpcode = 10;
          }
          break;
      }
      case 14: {
          insALUOpcode = 3;
          break;
      }
      default: {
          assert(0); // we should never reach this
      }
    }
}

void armisa::DecodeThumb32LDRSTRMultiple(u16_t in1, u16_t in2) {
    u2_t op = Bits(in1, 7, 2);
    u5_t W = Bit(in1, 5);
    u1_t L = Bit(in1, 4);
    u4_t Rn = Bits(in1, 0, 4);
    u5_t WRn = (W << 4) | Rn;

    switch(op) {
      case 0: {
          switch(L) {
            case 0: {
                assert(0); // SRS
            }
            case 1: {
                assert(0); // RFE
            }
          }
          break;
      }

      case 1: {
	insDecodeT = BlockT;
	insS = false;
	insW = (W == 1);
	insU = true;
	insRn = Rn;
	insP = false;

	runstate = MidMultipleS;
	
	switch(L) {
	  case 0: {
	    // STM (STMIA, STMEA)
	    insL = false;
	    insMultRegisters =
	      (0 << 15) |
	      Bit(in2, 14) << 14 |
	      (0 << 13) |
	      Bits(in2, 0, 13);
	    break;
	  }
	    
	  case 1: {
	    // LDM/LDMIA/LDMFD (Thumb) and POP
	    insL = true;
	    insMultRegisters = Bits(in2, 0, 16);
	    break;
	  }
	}
	break;
      }

      case 2: {
	insDecodeT = BlockT;
	insS = false;
	insW = (W == 1);
	insU = false;
	insRn = Rn;
	insP = true;
	
	insMultRegisters = Bits(in2, 0, 16);
	
	runstate = MidMultipleS;

	switch(L) {
	  case 0: {
	    // STMDB (STMFD) or PUSH if WRn = 0x1D
	    insL = false;
	    break;
	  }
	    
	  case 1: {
	    // LDMDB/LDMEA
	    insL = true;
	    break;
	  }
	}

	break;
      }

      case 3: {
          switch(L) {
            case 0: {
                assert(0); // SRS (Thumb)
            }
            case 1: {
                assert(0); // RFE
            }
          }
          break;
      }

      default: 
        assert(0); // we should never hit this
    }
}

void armisa::DecodeThumb32Misc(u16_t in1, u16_t in2) {
    u2_t op1 = Bits(in1, 4, 2);
    u2_t op2 = Bits(in2, 4, 2);

    switch(op1) {
      case 0: {
          switch(op2) {
            case 0:
              assert(0); // QADD
            case 1:
              assert(0); // QDADD
            case 2:
              assert(0); // QSUB
            case 3:
              assert(0); // QDSUB
          }
          break;
      }

      case 1: {
          switch(op2) {
            case 0:
              assert(0); // REV
            case 1:
              assert(0); // REV16
            case 2:
              assert(0); // RBIT
            case 3:
              assert(0); // REVSH
          }
          break;
      }

      case 2: {
          assert(op2 == 0);
          u32_t val1;
          u32_t val2;
          u32_t val3;
          u32_t val4;

          u32_t Rn = Reg(Bits4(in1, 0));
          u32_t Rm = Reg(Bits4(in2, 0));
          u32_t Rd = Bits4(in2, 8);

          u32_t cpsr = Reg(CpsrReg);
          val1 = ((cpsr >> 16) & 0x1) ? (Rn & 0xFF) : (Rm & 0xFF);
          val2 = ((cpsr >> 17) & 0x1) ? ((Rn >> 8) & 0xFF) : ((Rm >> 8) & 0xFF);
          val3 = ((cpsr >> 18) & 0x1) ? ((Rn >> 16) & 0xFF) : ((Rm >> 16) & 0xFF);
          val4 = ((cpsr >> 19) & 0x1) ? ((Rn >> 24) & 0xFF) : ((Rm >> 24) & 0xFF);
          
          Reg(Rd) = 
              val1 | 
              (val2 << 8) | 
              (val3 << 16) | 
              (val4 << 24);

	  insRd = Rd;
	  insRn = Rn;
	  insRm = Rm;

	  insDecodeT = ThumbExecutedT;
	  stats.m_ALU+=1;
	  
          break;
      }

      case 3: {
          assert(op2 == 0);
          // Count Leading Zeros (CLZ)
          u4_t Rd = Bits4(in2, 8);
          u4_t Rm = Bits4(in2, 0);
          u32_t r = 0, a = Reg(Rm);
          // TODO: combine with implementation of clz in armisa.cpp
          for(r = 0; r < 32; r++) {
              if(a & (1 << 31))
                  break;
              a <<= 1;
          }
          
          Reg(Rd) = r;
	  insRd = Rd;
	  insRm = Rm;

	  insDecodeT = ThumbExecutedT;
	  stats.m_ALU+=1;
	  
          break;
      }

      default: 
        assert(0); // we should never reach this
    }
}

u32_t armisa::ThumbExpandImm(u12_t imm12) {
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
        u32_t unrotated_value = (u32_t)(1 << 7) | ((u32_t)imm12 & 0x7F);
        u8_t rotation = (imm12 >> 7) & 0x1F;
        imm32 = Ror(unrotated_value, rotation);
    }

    return imm32;
}

void armisa::DecodeThumb32DataProcessingModifedImmediate(u16_t in1, u16_t in2) {
    u4_t op = Bits4(in1, 5);
    u4_t Rn = Bits4(in1, 0);
    u1_t S = Bit(in1, 4);
    u4_t Rd = Bits4(in2, 8);
    u5_t RdS = ((u5_t)Rd << 1) | S;

    insDecodeT = AluT;
    insS = (S == 1);
    insRd = Bits4(in2, 8);
    if(Rn != 15) {
        insAluOperand1 = Reg(Rn);
	insRn = Rn;
    }
    u12_t imm3 = Bits(in2, 12, 3);
    u12_t imm8 = Bits(in2, 0, 8);
    u12_t imm12 = (Bit(in1, 10) << 11) | (imm3 << 8) | imm8;
    insAluOperand2 = ThumbExpandImm(imm12);   

    switch(op) {
      case 0: {
        if(RdS != 0x1F) {
            insALUOpcode = 0; // AND
        } else {
            insALUOpcode = 8; // TST
        }
        break;
      }
      case 1: {
        insALUOpcode = 14;
        break;
      }
      case 2: {
          if(Rn == 15) {
              // MOV
              if((insAluOperand2 & (1 << 31)) && insS) 
                 Reg(CpsrReg) = Reg(CpsrReg) | (1 << 29);

              insALUOpcode = 13;                 
          } else {
              // ORR
              insALUOpcode = 12;
          }
        break;
      }
      case 3:
        if((insAluOperand2 & (1 << 31)) && insS)
            Reg(CpsrReg) = Reg(CpsrReg) | (1 << 29);        
        if(Rn == 15) {
            // MVN
            insALUOpcode = 15;
        } else {
            // ORN
            insAluOperand2 = ~insAluOperand2;
            insALUOpcode = 12;
        }
        break;
      case 4:  {
          if(RdS != 0x1F) {
              // EOR
              insALUOpcode = 1;
          } else {
              // TEQ
              insALUOpcode = 9;
          }
          break;
      }
      case 8: {
          if(RdS != 0x1F) {
              // ADD
              insALUOpcode = 4;
          } else {
              // CMN
              insALUOpcode = 11;
          }
        break;
      }
      case 10: {
        // ADC
        insALUOpcode = 5;
        break;
      }
      case 11: {
        // SBC
        insALUOpcode = 6;
        break;
      }
      case 13: {
          if(RdS != 0x1F) {
              insALUOpcode = 2; // SUB
          } else {
              insALUOpcode = 10; // CMP              
          } 
          break;
      }
      case 14: {
          // RSB
          insALUOpcode = 3;
          break;
      }
      default:
        assert(0); // we should never reach this
    }
}

void armisa::DecodeThumb32LongMultiply(u16_t in1, u16_t in2) {
    u3_t op1 = Bits(in1, 4, 3);
    u4_t op2 = Bits(in2, 4, 4);
    insDecodeT = MulT;

    // we do not update flags
    insS = false;
    insRn = Bits4(in1, 0);
    insRm = Bits4(in2, 0);
    insAluOperand1 = Reg(insRn);
    insAluOperand2 = Reg(insRm);
    insRdLo = Bits4(in2, 12);
    insRdHi = Bits4(in2, 8);

    switch(op1) {
      case 0:
        // SMULL
        assert(op2 == 0);
        insMultOp = MULT_SignedLong;
        insMultAccum = 0;
        break;
      case 1:
        assert(0); // SDIV
        break;
      case 2: {
          // UMULL
          assert(op2 == 0);
          insMultOp = MULT_UnsignedLong;
          insMultAccum = 0;
          break;
      }
      case 3:
        assert(0); // UDIV
        break;
      case 4:
        assert(0); // SMLAL, SMLALBB, SMLALBT, SMLALTB, SMLALTT, SMLALD
        break;
      case 5:
        assert(0); // SMLSLD
        break;
      case 6: {
          if(op2 == 0) {
              // UMLAL
              insMultOp = MULT_UnsignedLong;
              u64_t hi = Reg(insRdHi);
              insMultAccum = (hi << 32) | (u64_t)Reg(insRdLo);
	      insRd = insRdHi;
	      insRs = insRdLo;
              break;
          }
          else if(op2 == 6) {
              // UMAAL
              assert(0);
          }
          else 
              assert(0); // we should never reach thos
        break;
      }
      default:
        assert(0); // we should never reach this
    }
}

void armisa::DecodeThumb32LDRSTRDualExclusive(u16_t in1, u16_t in2) {
    u2_t op1 = Bits(in1, 7, 2);
    u2_t op2 = Bits(in1, 4, 2);
    u4_t Rn = Bits4(in1, 0);
    u2_t op3 = Bits(in2, 4, 4);
    
    insDecodeT = SingleT;        
    insSizeCode = 3;
    insSigned = false;
    insEXCF = false;
    insSUser = false;
    insU = Bit(in1, 7);
    insP = Bit(in1, 8);
    insW = Bit(in1, 5);
    insDataAddress = Reg(Rn);
    insDataOffset = ((u32_t)Bits(in2, 0, 8)) << 2;
    if(insU == 0) insDataOffset = 0 - insDataOffset;
    if(insP == 1) insDataAddress += insDataOffset;
    insRd = Bits4(in2, 12);
    insRn = Rn;

    if(op1 == 0 && op2 == 0) {
      // STREX
      insSizeCode = 2;
      insL = false;
      insEXCF = true;
      insRm = insRd;
      insRd = Bits4(in2, 8);
      insP = true;
    }
    else if(op1 == 0 && op2 == 1) {
      // LDREX
      insSizeCode = 2;
      insL = true;
      insEXCF = true;
      insP = true;
    }
    else if(((op1 >> 1) == 0 && op2 == 2) ||
       ((op1 >> 1) == 1 && ((op2 & 1) == 0))) {
        // STRD (immediate)
        assert((insRd+1) == Bits4(in2, 8));
        insL = false;
    } else if((((op1 >> 1) == 0 && op2 == 3) 
               || ((op1 >> 1) == 1 && (op2 & 1) == 1)) 
              && Rn != 15) {
        // LDRD (immediate)
        assert((insRd+1) == Bits4(in2, 8));
        insL = true;
    } else if(op1 == 1 && op2 == 1) {
        switch(op3) {
          case 0:
          case 1: {
              
              u4_t Rm = Bits4(in2, 0);
              bool is_tbh = Bit(in2, 4);
              insDecodeT = TableBranchT;
              insDataAddress = Reg(Rn);
              if(Rn == 15)
                  insDataAddress -= THUMB_PC_CORRECTION;
              
              if(is_tbh) {
                  insDataAddress += (Reg(Rm) << 1);
                  insSizeCode = 1;
              } else {
                  insDataAddress += Reg(Rm);
                  insSizeCode = 0;
              }
	      insRm = Rm;

              return;
          }
          default: {
              printf("Not yet implemented instruction %04x %04x\n", in1, in2);
              assert(0);
          }
        }
    } else {
        printf("Not yet implemented instruction %04x %04x\n", in1, in2);
        assert(0);
    }

    runstate = MidSingleS;
}

void armisa::DecodeThumb32LoadWord(u16_t in1, u16_t in2) {
    u2_t op1 = Bits(in1, 7, 2);
    u6_t op2 = Bits(in2, 6, 6);
    insRn = Bits4(in1, 0);
    insRd = Bits4(in2, 12);

    insDecodeT = SingleT;
    insL = true;
    insS = false;
    insEXCF  = false;
    insSUser = false;
    insSizeCode = 2;
    insW = false;
    insU = true;
    insP = true;
    insSigned = false;

    if(op1 == 0 && op2 == 0 & insRn != 15) {
        // LDR (register, Thumb)
        insRm = Bits4(in2, 0);
        insShiftType = 0; // lsl
        insShiftComputed = false;
        insShiftValue = Bits(in2, 4, 2);
        exec_shift_operand_instr();

        insDataOffset = insAluOperand2;
        insDataAddress = Reg(insRn) + insAluOperand2;
    } else if((op1 == 0 && (((op2 & 0x24) == 0x24) || ((op2 & 0x3C) == 0x30)) && insRn != 15) ||
              (op1 == 1 && insRn != 15)) {
        // LDR (immediate, Thumb)
        if(Bit(in1, 7)) {
            insDataOffset = Bits(in2, 0, 12);            
            insDataAddress = Reg(insRn) + insDataOffset;
        } else {
            insW = Bit(in2, 8);
            insU = Bit(in2, 9);
            insP = Bit(in2, 10);
            insDataOffset = Bits(in2, 0, 8);
            if(insU == 0) insDataOffset = 0 - insDataOffset;
            insDataAddress = Reg(insRn);
            if(insP == 1) insDataAddress += insDataOffset;
        } 
    } else if(op1 == 0 && (op2 & 0x3C == 0x38) && insRn != 15) {
        assert(0); // LDRT
    } else if((op1 >> 1) == 0 && insRn == 15) {
        u32_t base = (Reg(PcReg) - THUMB_PC_CORRECTION) & ~3;
	bool add = Bit(in1, 7);
	u32_t imm32 = Bits(in2, 0, 12);
	if(add) 
	  insDataAddress = base + imm32;
	else
	  insDataAddress = base - imm32;
    } else {
        assert(0); // we should never reach this
    }
    OPERAND_PTRACE(insDataAddress);              
    runstate = MidSingleS;
}

void armisa::DecodeThumb32DataProcessingImmediate(u16_t in1, u16_t in2) {
    u5_t op = Bits(in1, 4, 5);
    insRd = Bits4(in2, 8);
    u32_t imm32 = 
        (Bits(in1, 0, 4) << 12) | 
        (Bit(in1, 10) << 11) |
        (Bits(in2, 12, 3)) << 8 |
        (Bits(in2, 0, 8));
    u32_t imm12 =
        (Bit(in1, 10) << 11) | 
        (Bits(in2, 12, 3) << 8) |
        (Bits(in2, 0, 8));

    insRn = Bits4(in1, 0);

    insS = false;
    insDecodeT = AluT;

    switch(op) {
      case 0: {
          // ADD
          if(insRn != 15) {
              insAluOperand1 = Reg(insRn);
              insAluOperand2 = imm12;
              insALUOpcode = 4; 
          } else {
	    insAluOperand1 = (Reg(PcReg) - THUMB_PC_CORRECTION) & ~3;
	    insAluOperand2 = imm12;
	    insALUOpcode = 4;
          }
          break;
      }
      case 4: {
          // Move Wide (16-bit)
   	  insRn = UndefinedReg;
          insAluOperand2 = imm32;
          insALUOpcode = 13;             
          break;
      }

      case 10: {
          // SUB
          if(insRn != 15) {
              insAluOperand1 = Reg(insRn);
              insAluOperand2 = imm12;
              insALUOpcode = 2;
          } else {
              assert(0); // ADR
          }
          break;
      }

      case 12: {
          // MOVT
          u32_t val = Reg(insRd);
	  insRn = UndefinedReg;
          val &= 0x0000FFFF;
          val |= (imm32 << 16);
          insAluOperand2 = val;
          insALUOpcode = 13;
   
          break;
      }

      case 0x16: {
	if(insRn != 0xF) {
	  // BFI
	  u32_t msbit = Bits(in2, 0, 5);
	  u32_t lsbit =
	    (Bits(in2, 12, 3) << 2)
	    | Bits(in2, 6, 2);

	  u32_t width = msbit-lsbit+1;
	  u32_t new_value = ((Reg(insRn) << (32-width)) >> (32-width)) << lsbit;
	  u32_t old_value = Reg(insRd);
	  old_value &= ~(((1 << width) - 1) << lsbit);
	  insAluOperand2 = old_value | new_value;
	  insALUOpcode = 13;
	} else {
	  assert(0); // BFC
	}
	break;
      }

      case 0x1C: {
          // UBFX 
          u5_t lsbit = (Bits(in2, 12, 3) << 2) | Bits(in2, 6, 2);
          u5_t width = Bits(in2, 0, 5) + 1;
          u32_t val = Reg(insRn);
          insAluOperand2 = (val >> lsbit) & ((1 << width) - 1);
          insALUOpcode = 13;
          
          break;
      }

      default:
        printf("Not yet implemented instruction %04x %04x\n", in1, in2);
        assert(0);
    }
}

void armisa::DecodeThumb32LoadHlfWord(u16_t in1, u16_t in2) {
    u2_t op1 = Bits(in1, 7, 2);
    u6_t op2 = Bits(in2, 6, 6);
    u4_t Rn = Bits4(in1, 0);
    u4_t Rt = Bits4(in2, 12);
    u4_t Rm = Bits4(in2, 0);

    insDecodeT = SingleT;
    insL = true;
    insS = false;
    insEXCF = false;
    insSUser = false;
    insRd = Rt;
    insRn = Rn;
    insRm = Rm;
    insSizeCode = 1;
    insDataAddress = Reg(Rn);

    if(op1 == 3 && Rn != 15 && Rt != 15) {
        insP = true;
        insU = true;
        insW = false;
        insSigned = true;
        
        insDataOffset = Bits(in2, 0, 12);
    } else if((op1 == 1 && Rn != 15 && Rt != 15) ||
	      (op1 == 0 && 
	       (((op2 & 0x24) == 0x24 && Rn != 15) ||
		((op2 & 0x3C) == 0x30 && Rn != 15 && Rt != 15)))) {
        insSigned = false;
        // LDRH (immediate, Thumb)
        if(Bit(in1, 7)) {
	    insW = false;
	    insU = true;
	    insP = true;
	    insDataOffset = Bits(in2, 0, 12);
	} else {
	    insP = Bit(in2, 10);
	    insU = Bit(in2, 9);
	    insW = Bit(in2, 8);
	    insDataOffset = Bits(in2, 0, 8);
	}

    } else if(op1 == 2 && op2 == 0 && Rn != 15 && Rn != 15) {
      // LDRSH (register)
      insSigned = true;
      insU = true;
      insP = true;
      insW = false;
      insDataOffset = Reg(Rm);
      u2_t imm2 = Bits(in2, 4, 2);
      if(imm2 != 0)
	insDataOffset <<= imm2;
    } else {
        printf("Not yet implemented instruction %04x %04x\n", in1, in2);
        assert(0);
    }

    if(insU == 0) insDataOffset = 0 - insDataOffset;
    if(insP == 1) insDataAddress += insDataOffset;              

    runstate = MidSingleS;
}

void armisa::DecodeThumb32MoveRegisterAndImmShifts(u16_t in1, u16_t in2) {
    insS = Bit(in1, 4);
    insRd = Bits4(in2, 8);
    insRm = Bits4(in2, 0);
    u32_t imm3 = Bits(in2, 12, 3);
    u32_t imm2 = Bits(in2, 6, 2);
    u2_t type = Bits(in2, 4, 2);

    insShiftType = type;
    insShiftComputed = false;
    insShiftValue = (imm3 << 2) | imm2;

    insDecodeT = AluT;
    insALUOpcode = 13;

    switch(type) {
      case 0: {
          if(insShiftValue == 0) {
              insAluOperand2 = Reg(insRm);
          } else {
              exec_shift_operand_instr();
          }
          break;
      }

      case 1:
      case 2: 
      case 3: {
          exec_shift_operand_instr();
          break;
      }

      default:
        assert(0); // should never reach this
    }    
}
  
void armisa::DecodeThumb32STRSingleData(u16_t in1, u16_t in2) {
    u3_t op1 = Bits(in1, 5, 3);
    u6_t op2 = Bits(in2, 6, 6);

    insDecodeT = SingleT;
    insEXCF = false;
    insSUser = false;
    insSigned = false;
    insL = false;
    insRd = Bits4(in2, 12);
    insRn = Bits4(in1, 0);	
    insDataAddress = Reg(insRn);

    if((op1 == 2 && ((op2 & 0x24) == 0x24 || (op2 & 0x3C) == 0x30))
       || op1 == 6
       || (op1 == 0 && ((op2 & 0x24) == 0x24 || (op2 & 0x3C) == 0x30))
       || op1 == 4) {
      // STR or STRB (immediate, Thumb)
      if(op1 == 2 || op1 == 6)
	insSizeCode = 2;
      else if(op1 == 0 || op1 == 4)
	insSizeCode = 0;
      
      if(Bit(in1, 7)) {
	insP = true;
	insU = true;
	insW = false;
	insDataOffset = Bits(in2, 0, 12);
      } else {
	insP = Bit(in2, 10);
	insU = Bit(in2, 9);
	insW = Bit(in2, 8);
	insDataOffset = Bits(in2, 0, 8); 
      }
    }
    else if(op1 == 2 && op2 == 0) {
        // STR (register) 
        insW = false;
        insU = true;
        insP = true;
        insSizeCode = 2;
	insRm = Bits4(in2, 0);
        insDataOffset = Reg(insRm);
        u2_t imm2 = Bits(in2, 4, 2);
        if(imm2 != 0)
            insDataOffset <<= imm2;
    } else if(op1 == 5 || 
	      (op1 == 1 && ((op2 & 0x24) == 0x24 || (op2 & 0x3C) == 0x30))) {
        // STRH
        insSizeCode = 1;
	if(Bit(in1, 7)) {
	  insW = false;
	  insU = true;
	  insP = true;
	  insDataOffset = Bits(in2, 0, 12);
	} else {
	  insP = Bit(in2, 10);
	  insU = Bit(in2, 9);
	  insW = Bit(in2, 8);
	  insDataOffset = Bits(in2, 0, 8);
	}
    } else if(op1 == 1 && op2 == 0) {
      // STRH (register)
      insW = false;
      insU = true;
      insP = true;
      insSizeCode = 1;
      insRm = Bits4(in2, 0);
      insDataOffset = Reg(insRm);
      u2_t imm2 = Bits(in2, 4, 2);
      if(imm2 != 0)
	insDataOffset <<= imm2;
    } else if(op1 == 0 && op2 == 0) {
      // STRB (register)
      insW = false;
      insU = true;
      insP = true;
      insSizeCode = 0;
      insRm = Bits4(in2, 0);
      insDataOffset = Reg(insRm);
      u2_t imm2 = Bits(in2, 4, 2);
      if(imm2 != 0)
	insDataOffset <<= imm2;
    } else {
        printf("Instruction %04x %04x @ address %08x not yet implemented\n", in1, in2, insAddress);
        assert(0);
    }

    if(insU == 0) insDataOffset = 0 - insDataOffset;
    if(insP == 1) insDataAddress += insDataOffset;              

    runstate = MidSingleS;
}

void armisa::DecodeThumb32LoadByteMemoryHints(u16_t in1, u16_t in2) {
    u2_t op1 = Bits(in1, 7, 2);
    u6_t op2 = Bits(in2, 6, 6);

    u4_t Rn = Bits4(in1, 0);
    u4_t Rt = Bits4(in2, 12);

    insDecodeT = SingleT;
    insL = true;
    insS = false;
    insEXCF  = false;
    insSUser = false;
    insW = false;
    insU = true;
    insP = true;
    insRd = Rt;

    if(op1 == 0 && op2 == 0 && Rn != 15 && Rt != 15) {
        // LDRB (register)
        insSigned = false;
	insSizeCode = 0;
        insRm = Bits4(in2, 0);
        insShiftType = 0; // lsl
        insShiftComputed = false;
        insShiftValue = Bits(in2, 4, 2);
        exec_shift_operand_instr();

        insDataOffset = insAluOperand2;
        insDataAddress = Reg(Rn) + insAluOperand2;
	insRn = Rn;
    } else if((op1 == 0 && (op2 & 0x24) == 0x24 && Rn != 15)
	      || (op1 == 1 && Rn != 15 && Rt != 15)
	      || (op1 == 0 && (op2 & 0x3C) == 0x30 && Rn != 15 && Rt != 15)) {
        // LDRB (immediate, Thumb)
	insSigned = false;
	insSizeCode = 0;
	insDataAddress = Reg(Rn);
	insRn = Rn;

        if(Bit(in1, 7)) {
	    insP = true;
	    insU = true;
	    insW = false;
	    insDataOffset = Bits(in2, 0, 12);
        } else {
	    insP = Bit(in2, 10);
	    insU = Bit(in2, 9);
	    insW = Bit(in2, 8);
	    insDataOffset = Bits(in2, 0, 8);
	}
	
	if(insU == 0) insDataOffset = 0 - insDataOffset;
	if(insP == 1) insDataAddress += insDataOffset;
    } else if(op1 == 1 && Rt == 15) {
        // Preload data (PLD, PLDW)
        // ignore for now
        insDecodeT = ThumbExecutedT;
	stats.m_Loads+=1;
        return;
    } else {
        printf("Instruction %04x %04x @ address %08x not yet implemented\n", in1, in2, insAddress);
        assert(0);
    }

    runstate = MidSingleS;
}

void armisa::DecodeThumb32CPRSIMDFloatInstrs(u16_t in1, u16_t in2) {
    u6_t op1 = Bits(in1, 4, 6);
    u4_t coproc = Bits4(in2, 8);
    u1_t op = Bit(in2, 4);

    insDecodeT = CoprocRegT;
    if((coproc & 0xE) != 10) {
      if((op1 & 0x31) == 0x20 && op == 1) {
	// MCR (move to coprocessor from ARM core register)
	insRn = Bits4(in2, 12);
	u32_t data = Reg(insRn);
	u4_t cpr = Bits4(in1, 0);
	u4_t cpm = Bits4(in2, 0);
	u3_t opc1 = Bits(in1, 5, 3);
	u3_t opc2 = Bits(in2, 5, 3);
	copro_write(coproc, cpr, cpm, data, opc1, opc2);
      }
      else if((op1 & 0x31) == 0x21 && op == 1) {
	// MRC (move from coprocessor to ARM core register)
	u4_t cpr = Bits4(in1, 0);
	u4_t cpm = Bits4(in2, 0);
	u3_t opc1 = Bits(in1, 5, 3);
	u3_t opc2 = Bits(in2, 5, 3);
	u32_t data = copro_read(coproc, cpr, cpm, opc1, opc2);
	//printf("MRC implemented instruction op1=%x op=%x data=%x\n", op1, op, data);
	insRd = Bits4(in2, 12);
	Reg(insRd) = data;
      }
      else {
	  printf("Not yet implemented instruction %04x %04x @ address %08x\n", in1, in2, insAddress);
	  assert(0);
	}
    } else {
      if(Bits4(in1, 12) == 0xE) {
	int single_reg = (Bit(in2, 8) == 0);
	insRn = Bits4(in1, 0);
	u8_t imm8 = Bits(in2, 0, 8);
	insDataOffset = (u32_t)imm8 << 2;
	insL = true;
	insSizeCode = single_reg == 1 ? 2 : 3;
	insDecodeT = FloatMemoryT;
	insFloatSingle = (single_reg == 1);
	insRd =
	  insFloatSingle ?
	  (Bits4(in2, 12) << 1 | Bit(in1, 6)) :
	  (Bit(in1, 6) << 4 | Bits4(in2, 12));

	insU = Bit(in1, 7);
	u32_t base = Reg(insRn);
	if(insRn == PcReg) {
	  base = (base - THUMB_PC_CORRECTION) & ~3;
	}
	insDataAddress = base;

	if(Bits(in1, 9, 3) == 0x6
	   && Bit(in1, 4) == 0x1
	   && Bits(in2, 9, 3) == 0x5
	   && !(Bit(in1, 8) == 0 && Bit(in1, 7) == 0 && Bit(in1, 5) == 0)) {
	  insDecodeT = FloatMemoryT;
	  insP = Bit(in1, 8);
	  insW = Bit(in1, 5);
	  if(insP == 1 && insW == 0) {	    
	    /* VLDR */
	    insMultRegisters = 1;
	    if(insU)
	      insDataAddress = base + insDataOffset;
	    else
	      insDataAddress = base - insDataOffset;
	  } else if(insP == 0 && insU == 1 && insW == 1 && insRn == 13) {
	    /* VPOP */
	    if(single_reg) 
	      insMultRegisters = imm8;
	    else
	      insMultRegisters = imm8/2;
	    insDataAddress = base;
	    Reg(insRn) += insDataOffset;
	  } else {
	    /* VLDM{mode} */
	    if(single_reg)
	      insMultRegisters = imm8;
	    else
	      insMultRegisters = imm8/2;
	    
	    if(!insU) {
	      insDataAddress -= insDataOffset;
	    }
	    
	    if(insW) {
	      if(insU)
		Reg(insRn) += insDataOffset;
	      else
		Reg(insRn) -= insDataOffset;
	    }
	  }
	} else if(Bits(in1, 9, 3) == 0x6 &&
		  Bit(in1, 4) == 0 &&
		  Bits(in2, 9, 3) == 0x5 &&
		  !(Bit(in1, 8) == 0 && Bit(in1, 7) == 0 && Bit(in1, 5) == 0)) {
	  insDecodeT = FloatMemoryT;
	  insP = Bit(in1, 8);
	  insW = Bit(in1, 5);
	  insL = false;
	  if(insP == 1 && insW == 0) {
	    /* VSTR */
	    insDataAddress = Reg(Bits4(in1, 0));
	    if(insU)
	      insDataAddress += insDataOffset;
	    else
	      insDataAddress -= insDataOffset;
	    insMultRegisters = 1;
	  } else if(insP == 1 && insU == 0 && insW == 1 && insRn == 13) {
	    /* VPUSH */
	    assert(!single_reg);
	    insRd = Bit(in1, 6) << 4 | Bits4(in2, 12);
	    insMultRegisters = imm8 / 2;
	    insDataAddress = base - insDataOffset;
	    Reg(insRn) -= insDataOffset;
	    insSizeCode = 3;
	  } else {
	    /* VSTM */
	    u1_t add = insU == 1;
	    u1_t wback = insW == 1;
	    if(single_reg)
	      insMultRegisters = imm8;
	    else
	      insMultRegisters = imm8 / 2;
	    insDataAddress = add ? base : (base - insDataOffset);
	    if(wback)
	      Reg(insRn) = add ? Reg(insRn) + insDataOffset : Reg(insRn) - insDataOffset;
	  }
	} else if(Bits4(in1, 8) == 0xD &&
		  Bits(in1, 4, 2) == 0x0 &&
		  Bits(in2, 9, 3) == 0x5) {
	  /* VSTR */
	  /* should never pick this branch */
	  assert(0);
	} else if(Bits4(in1, 8) == 0xD
		  && Bits(in1, 4, 2) == 0x1 
		  && Bits(in2, 9, 3) == 0x5) {
	  /* VLDR */
	  /* Should never pick this branch */
	  assert(0);
	} else if(Bits4(in1, 8) == 0xD
		  && Bits(in1, 4, 2) == 0x2
		  && Bits4(in1, 0) == 0xD
		  && Bits4(in2, 8) == 0xB) {
	  /* VPUSH */
	  /* Should never pick this branch */
	  assert(0);
	} else if(Bits(in1, 7, 5) == 0x19
		  && Bits(in1, 0, 6) == 0x3D
		  && Bits(in2, 9, 3) == 0x5) {
	  /* VPOP */
	  /* should never pick this branch */
	  assert(0);
	} else if(Bits(in1, 7, 5) == 0x1C &&
		  Bits(in1, 4, 2) == 0 &&
		  Bits(in2, 9, 3) == 0x5 &&
		  Bit(in2, 4) == 0x0) {
	  /* VMLA */
	  insDecodeT = FloatALUT;
	  insALUOpcode = 0;
	  insU = Bit(in2, 6);
	  if(insFloatSingle) {
	    insRn = Bits4(in1, 0) << 1 | Bit(in2, 7);
	    insRm = Bits4(in2, 0) << 1 | Bit(in2, 5);
	  } else {
	    insRn = Bit(in2, 7) << 4 | Bits4(in1, 0);
	    insRm = Bit(in2, 5) << 4 | Bits4(in2, 0);
	  }	  
	} else if(Bits(in1, 7, 5) == 0x1D &&
		  Bits(in1, 4, 2) == 0 &&
		  Bits(in2, 9, 3) == 0x5 &&
		  Bit(in2, 6) == 0 &&
		  Bit(in2, 4) == 0) {
	  /* VDIV */
	  insDecodeT = FloatALUT;
	  insALUOpcode = 1;
	  if(insFloatSingle) {
	    insRn = Bits4(in1, 0) << 1 | Bit(in2, 7);
	    insRm = Bits4(in2, 0) << 1 | Bit(in2, 5);
	  } else {
	    insRn = Bit(in2, 7) << 4 | Bits4(in1, 0);
	    insRm = Bit(in2, 5) << 4 | Bits4(in2, 0);
	  }
	} else if(Bits(in1, 7, 5) == 0x1C &&
		  Bits(in1, 4, 2) == 0x2 &&
		  Bits(in2, 9, 3) == 0x5 &&
		  Bit(in2, 6) == 0 &&
		  Bit(in2, 4) == 0) {
	  /* VMUL */
	  insDecodeT = FloatALUT;
	  insALUOpcode = 2;
	  if(insFloatSingle) {
	    insRn = Bits4(in1, 0) << 1 | Bit(in2, 7);
	    insRm = Bits4(in2, 0) << 1 | Bit(in2, 5);
	  } else {
	    insRn = Bit(in2, 7) << 4 | Bits4(in1, 0);
	    insRm = Bit(in2, 5) << 4 | Bits4(in2, 0);
	  }
	} else if(Bits(in1, 7, 5) == 0x1C &&
		  Bits(in1, 4, 2) == 0x3 &&
		  Bits(in2, 9, 3) == 0x5 &&
		  Bit(in2, 6) == 0 &&
		  Bit(in2, 4) == 0) {
	  /* VADD */
	  insDecodeT = FloatALUT;
	  insALUOpcode = 3;
	  if(insFloatSingle) {
	    insRn = Bits4(in1, 0) << 1 | Bit(in2, 7);
	    insRm = Bits4(in2, 0) << 1 | Bit(in2, 5);
	  } else {
	    insRn = Bit(in2, 7) << 4 | Bits4(in1, 0);
	    insRm = Bit(in2, 5) << 4 | Bits4(in2, 0);
	  }
	} else if(Bits(in1, 7, 5) == 0x1C &&
		  Bits(in1, 4, 2) == 0x3 &&
		  Bits(in2, 9, 3) == 0x5 &&
		  Bit(in2, 6) == 1 &&
		  Bit(in2, 4) == 0) {
	  /* VSUB */
	  insDecodeT = FloatALUT;
	  insALUOpcode = 11;
	  if(insFloatSingle) {
	    insRn = Bits4(in1, 0) << 1 | Bit(in2, 7);
	    insRm = Bits4(in2, 0) << 1 | Bit(in2, 5);
	  } else {
	    insRn = Bit(in2, 7) << 4 | Bits4(in1, 0);
	    insRm = Bit(in2, 5) << 4 | Bits4(in2, 0);
	  }
	} else if(Bits(in1, 7, 5) == 0x1C &&
		  Bits(in1, 4, 2) == 0x1 &&
		  Bits(in2, 9, 3) == 0x5 &&
		  Bit(in2, 4) == 0) {
	  insDecodeT = FloatALUT;
	  if(insFloatSingle) {
	    insRn = Bits4(in1, 0) << 1 | Bit(in2, 7);
	    insRm = Bits4(in2, 0) << 1 | Bit(in2, 5);
	  } else {
	    insRn = Bit(in2, 7) << 4 | Bits4(in1, 0);
	    insRm = Bit(in2, 5) << 4 | Bits4(in2, 0);
	  }
	  /* VNMLA = 4 */
	  /* VNMLS = 5 */
	  insALUOpcode = (Bit(in2, 6) == 1) ? 4 : 5;
	} else if(Bits(in1, 7, 5) == 0x1D 
		  && Bits(in1, 4, 2) == 0x3
		  && Bits(in2, 9, 3) == 0x5
		  && Bits4(in1, 0) == 0x0
		  && Bit(in2, 7) == 0x0
		  && Bit(in2, 4) == 0x0
		  && Bit(in2, 6) == 1) {
	  /* VMOV (register) */
	  insDecodeT = FloatALUT;
	  if(insFloatSingle) {
	    insRm = Bits4(in2, 0) << 1 | Bit(in2, 5);
	  } else {
	    insRm = Bit(in2, 5) << 4 | Bits4(in2, 0);
	  }
	  insALUOpcode = 6;
	} else if(Bits(in1, 7, 5) == 0x1D 
		  && Bits(in1, 4, 2) == 0x3
		  && Bits(in1, 0, 4) == 0x1
		  && Bits(in2, 9, 3) == 0x5
		  && Bit(in2, 7) == 0
		  && Bit(in2, 6) == 1
		  && Bit(in2, 4) == 0) {
	  /* VNEG */
	  insDecodeT = FloatALUT;
	  if(insFloatSingle) {
	    insRm = Bits4(in2, 0) << 1 | Bit(in2, 5);
	  } else {
	    insRm = Bit(in2, 5) << 4 | Bits4(in2, 0);
	  }
	  insALUOpcode = 12;
	} else if(Bits(in1, 8, 4) == 0xE
		  && Bits(in1, 5, 3) == 0x0
		  && Bits(in2, 8, 4) == 0xA
		  && Bit(in2, 4) == 0x1) {
	  /* VMOV (between ARM core register and single-precision register) */
	  stats.m_Floats += 1;
	  insDecodeT = ThumbExecutedT;
	  u32_t* data = (u32_t*)(&d_reg[0]);
	  int n = (Bits4(in1, 0) << 1) | Bit(in2, 7);
	  int t = Bits4(in2, 12);
	  if(Bit(in1, 4) == 1) {
	    /* to arm register */
	    int dd = (int)ceil(n/2);
	    Reg(t) = data[n];
	  }
	  else {
	    data[n] = Reg(t);	    
	  }
	} else if(Bits(in1, 8, 4) == 0xC
		  && Bits(in1, 5, 3) == 0x2
		  && Bits(in2, 8, 4) == 0xB
		  && Bits(in2, 6, 2) == 0x0
		  && Bit(in2, 4) == 0x1) {
	  /* VMOV (between two ARM core registers and a doubleword extension register) */
	  stats.m_Floats += 1;
	  insDecodeT = ThumbExecutedT;
	  int t = Bits4(in2, 12);
	  int t2 = Bits4(in1, 0);
	  int m = (Bit(in2, 5) << 4) | Bits4(in2, 0);
	  if(Bit(in1, 4) == 1) {
	    Reg(t) = d_reg[m] & 0xFFFFFFFF;
	    Reg(t2) = d_reg[m] >> 32;
	  } else {
	    d_reg[m] = ((uint64_t)Reg(t2) << 32) | Reg(t);
	  }
	} else if(Bits(in1, 7, 5) == 0x1D 
		  && Bits(in1, 4, 2) == 0x3
		  && Bits4(in1, 0) == 0x0
		  && Bits(in2, 9, 3) == 0x5
		  && Bit(in2, 7) == 0x1
		  && Bit(in2, 6) == 1
		  && Bit(in2, 4) == 0x0) {
	  /* VABS */
	  insDecodeT = FloatALUT;
	  if(insFloatSingle) {
	    insRm = Bits4(in2, 0) << 1 | Bit(in2, 5);
	  } else {
	    insRm = Bit(in2, 5) << 4 | Bits4(in2, 0);
	  }
	  insALUOpcode = 10;
	} else if(Bits(in1, 7, 5) == 0x1D 
		  && Bits(in1, 4, 2) == 0x3
		  && Bits(in2, 9, 3) == 0x5
		  && Bits4(in1, 0) == 0x5
		  && Bit(in2, 6) == 1
		  && Bit(in2, 4) == 0) {
	  /* VCMP{E} */
	  /* Comparison with zero */
	  insDecodeT = FloatALUT;
	  insALUOpcode = 7;
	} else if(Bits(in1, 7, 5) == 0x1D 
		  && Bits(in1, 4, 2) == 0x3
		  && Bits(in2, 9, 3) == 0x5
		  && Bits4(in1, 0) == 0x4
		  && Bit(in2, 6) == 1
		  && Bit(in2, 4) == 0) {
	  /* VCMP{E} */
	  /* Comparison with register */
	  insDecodeT = FloatALUT;
	  if(insFloatSingle) {
	    insRm = Bits4(in2, 0) << 1 | Bit(in2, 5);
	  } else {
	    insRm = Bit(in2, 5) << 4 | Bits4(in2, 0);
	  }
	  insALUOpcode = 9;
	} else if(Bits(in1, 7, 5) == 0x1D 
		  && Bits(in1, 4, 3) == 0x7
		  && Bits4(in1, 0) == 0x1
		  && Bits(in2, 8, 4) == 0xA
		  && Bit(in2, 4) == 1) {
	  /* VMRS */
	  stats.m_Floats += 1;
	  insDecodeT = ThumbExecutedT;
	  int Rt = Bits4(in2, 12);
	  if(Rt != 15) {
	    Reg(Rt) = (fpscr_val << 28);
	  } else {
	    u1_t newN = (fpscr_val >> 3) & 0x1;
	    u1_t newZ = (fpscr_val >> 2) & 0x1;
	    u1_t newC = (fpscr_val >> 1) & 0x1;
	    u1_t newV = (fpscr_val & 0x1);
	    
	    u32_t &cpsr = Reg(CpsrReg);
	    cpsr &= 0x0FFFFFFFUL;
	    cpsr |= (newN << 31) +
	      (newZ << 30) +
	      (newC << 29) +
	      (newV << 28);
	  }
	} else if(Bits(in1, 7, 5) == 0x1D &&
		  Bits(in1, 4, 3) == 0x6 &&
		  Bits4(in1, 0) == 0x1 &&
		  Bits4(in2, 8) == 0xA
		  && Bit(in2, 4) == 1) {
	  /* VMSR */
	  stats.m_Floats += 1;
	  insDecodeT = ThumbExecutedT;
	  int Rt = Bits4(in2, 12);
	  fpscr_val = Reg(Rt);
	} else if(Bits(in1, 7, 5) == 0x1D 
		  && Bits(in1, 4, 2) == 0x3
		  && Bits4(in1, 0)  == 0x1
		  && Bits(in2, 9, 3) == 0x5
		  && Bit(in2, 7) == 1
		  && Bit(in2, 6) == 1
		  && Bit(in2, 4) == 0) {
	  /* VSQRT */
	  insDecodeT = FloatALUT;
	  if(insFloatSingle) {
	    insRm = Bits4(in2, 0) << 1 | Bit(in2, 5);
	  } else {
	    insRm = Bit(in2, 5) << 4 | Bits4(in2, 0);
	  }
	  insALUOpcode = 8;
	} else if(Bits(in1, 7, 5) == 0x1D 
		  && Bits(in1, 4, 2) == 0x3
		  && Bits(in2, 9, 3) == 0x5
		  && Bit(in2, 6) == 0
		  && Bit(in2, 4) == 0) {
	  /* VMOV (immediate) */
	  insDecodeT = FloatALUT;
	  insFloatSingle = single_reg;
	  insALUOpcode = 13;
	  u4_t imm4H = Bits4(in1, 0);
	  u4_t imm4L = Bits4(in2, 0);

	  u8_t imm8 = ((u8_t)imm4H << 4) | imm4L;

	  u1_t imm8_7 = (imm8 >> 7);
	  u1_t imm8_6 = (imm8 >> 6) & 0x1;
	  u1_t imm8_6_not = imm8_6 ^ 0x1;
	  u6_t imm8_5_0 = imm8 & 0x3F;

	  if(!insFloatSingle) {
	    insRd = Bit(in1, 6) << 4 | Bits4(in2, 12);
	    u8_t repl = imm8_6 ? 0xFF : 0x0;
	    u64_t imm64 =
	      ((u64_t)imm8_7 << 63) |
	      ((u64_t)(imm8_6_not) << 62) |
	      ((u64_t)repl << 54) |
	      ((u64_t)imm8_5_0 << 48);
	    insFloatOperand1 = imm64;
	  } else {
	    insRd = (Bits4(in2, 12) << 1) | Bit(in1, 6);
	    u5_t repl = imm8_6 ? 0x1F : 0x0;
	    u32_t imm32 =
	      ((u32_t)imm8_7 << 31) |
	      ((u32_t)(imm8_6_not) << 30) |
	      ((u32_t)repl << 25) |
	      ((u32_t)imm8_5_0 << 19);
	    insAluOperand1 = imm32;	    
	  }
	} else if(Bits(in1, 7, 5) == 0x1D &&
		  Bits(in1, 3, 3) == 0x7 &&
		  Bits(in2, 9, 3) == 0x5 &&
		  Bit(in2, 6) == 1 &&
		  Bit(in2, 4) == 0) {
	  /* VCVT */
	  insDecodeT = ThumbExecutedT;
	  stats.m_Floats+=1;
	  u3_t opc2 = Bits(in1, 0, 3);
	  u1_t op = Bit(in2, 7);
	  u1_t to_integer = ((opc2 >> 2) && 0x1) == 1;
	  u1_t unsign;

	  u32_t* data = (u32_t*)(&d_reg[0]);
	  if(to_integer) {
	    unsign = (opc2 & 0x1) == 0;
	    u1_t round_zero = op == 1;
	    insRd = (Bits4(in2, 12) << 1) | Bit(in1, 6);
	    insRm = !single_reg
	      ? ((Bit(in2, 5) << 4) | Bits4(in2, 0))
	      : ((Bits4(in2, 0) << 1) | Bit(in2, 5));
	    float val;
	    if(single_reg) {
	      val = *((float*)(&data[insRm]));
	    } else {
	      val = *((float*)(&d_reg[insRm]));
	    }
	      
	    val = round_zero ? floor(val) : ceil(val);
	    if(unsign) {
	      u32_t integer = (u32_t)val;
	      data[insRd] = integer;
	    } else {
	      int integer = (int)val;
	      data[insRd] = integer;
	    }
	  } else {
	    unsign = op == 0;
	    u1_t round_nearest = false;
	    insRm = (Bits4(in2, 0) << 1) | Bit(in2, 5);
	    insRd = !single_reg
	      ? (Bit(in1, 6) << 4) | Bits4(in2, 12)
	      : (Bits4(in2, 12) << 1) | Bit(in1, 6);
	    if(single_reg) {
	      if(unsign) {
		u32_t integer = *((u32_t*)(&data[insRm]));
		float val = (float)integer;
		data[insRd] = *((u32_t*)(&val));
	      } else {
		int integer = *((int*)(&data[insRm]));
		float val = (float)integer;
		data[insRd] = *((u32_t*)(&val));
	      }
	    } else {
	      /* TODO: Convert 64-bit integer to 32-bit float */
	      assert(0);
	    }
	  }
	} else {
	printf("Not yet implemented instruction %04x %04x @ address %08x\n", in1, in2, insAddress);
	assert(0);
	}
      }
    }
}

void armisa::DecodeThumb32UnsignedParallelAddAndSub(u16_t in1, u16_t in2) {
    u3_t op1 = Bits(in1, 4, 3);
    u2_t op2 = Bits(in2, 4, 2);
    insDecodeT = ParallelAluT;

    if(op1 == 0 && op2 == 0) {
        // UADD8
        insRn = Bits4(in1, 0);
        insRd = Bits4(in2, 8);
        insRm = Bits4(in2, 0);
        insALUOpcode = 0; // UADD8
    }
    else {
        printf("Unknown unsigned parallel add and sub instruction (op1=%d, op2=%d)\n", op1, op2);
        assert(0);
    }
}

// eof
