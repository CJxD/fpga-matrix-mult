// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2014 M Puzovic + D J Greaves


// ARM coprocessor number 14

#include "arm_cp14.h"
#include "armcore_tlm.h"

arm_cp14::arm_cp14(sc_module_name name) :
  sc_module(name) {
  cp14_target_socket.register_b_transport(this, &arm_cp14::b_access);

  char lname[64];
  snprintf(lname, 64, "CP14_Debug_registers");
  m_registers = new smallram<u8_t>(lname,
				   4, // number of bytes in a single word
				   1024, // total number of words (registers)
				   this);

  // The first register is DBGDIDR
  // For now we are just going to set bits [19:16] to indicate
  // that debug architecture version is ARMv7_1 0b0101
  // Number of breakpoints is [27:24] = 4
  u32_t dbgdidr = 0x05050000;
  m_registers->writepo(0x0, (u8_t*)(&dbgdidr), 4);
  
  u32_t allzeros = 0x00000000;
  // We are going to initialize all other registers to zeros
  for(int i = 1; i < 1024; i++) 
    m_registers->writepo(0x4*i, (u8_t*)(&allzeros), 4);
}  

void arm_cp14::b_access(int idx, PRAZOR_GP_T &trans, sc_time &delay) {
  cp_request_extension *ext = 0;
  trans.get_extension(ext);
  // instruction that we have received must be
  // of cp_request_extension
  assert(ext);

  // Get the register id (pp C6-2121 ARM DDI 0406C.b)
  assert(ext->op1 == 0);
  u4_t CRn = (ext->cpr & 0xF);
  // the highest bit must be 0
  assert((CRn & 0x8) == 0);
  u4_t opc2 = (ext->op2 & 0x7);
  u4_t CRm = (ext->crm & 0xF);

  u16_t regId = (CRn << 7) | (opc2 << 4) | CRm;

  // TODO: for now we are only aware of read access to CP14
  // once we see write access MP will implement write operations
  tlm::tlm_command cmd = trans.get_command();
  u8_t* ptr = trans.get_data_ptr();
  switch(cmd) {
    case tlm::TLM_READ_COMMAND: {
        memcpy(ptr, m_registers->readpo(4*regId), trans.get_data_length());       
        u32_t data = *((u32_t*)m_registers->readpo(4*regId));
        //printf("%s: Read from register %d is %08x\n", name(), regId, data);
        break;
    }

    case tlm::TLM_WRITE_COMMAND: {
        u32_t data = *((u32_t*)ptr);
        //printf("%s: Write to register %d is %08x\n", name(), regId, data);
        switch(regId) {
          case 34: { // DBGDSCRext
              // if we are writing to enable external we should also enable internal
              // see in Linux kernel arch/arm/kernel/hw_breakpoint.c function
              // enable_monitor_mode where for ARM_DEBUG_ARCH_V7_* it writes to 
              // external DBGDSCR but expects to see it propagated to DBGDSCRint (register 1)
              if(data & (1 << 15)) {
                  u32_t reg1 = *((u32_t*)m_registers->readpo(4));
                  reg1 |= (1 << 15);
                  memcpy(m_registers->readpo(4), (u8_t*)(&data), 4);
              }
              memcpy(m_registers->readpo(4*regId), ptr, 4);
              break;
          }

          case 112:
          case 113:
          case 114:
          case 115:
          case 116:
          case 117:
          case 118:
          case 119:
          case 120:
          case 121:
          case 122:
          case 123:
          case 124:
          case 125:
          case 126:
          case 127: { // DBGWCRm
              // we only support 4-bit byte address
              // so if we try to write support for 8-bit
              // byte address we will ignore and write 4-bit
              // byte address support
              if (((data >> 5) & 0xFF) == 0xFF) {
                  data &= ~(0xF0 << 5);
              }
              memcpy(m_registers->readpo(4*regId), (u8_t*)(&data), 4);
              break;
          }
            
          default:
            memcpy(m_registers->readpo(4*regId), ptr, 4);
            break;
        }


        break;
    }

    default:
      assert(0); // unknown transaction
  }
  trans.set_response_status(tlm::TLM_OK_RESPONSE);
}

// eof
