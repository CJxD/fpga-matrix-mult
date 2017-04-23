// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2014 M Puzovic + D J Greaves
// snoop control unit

#include "arm_scu.h"
#include "arm_cp15.h"

#define ARMSCUMAP(X) 

arm_scu::arm_scu(sc_module_name name) : 
    scu(name),
    arm_initiator_socket(0) {
    arm_target_socket.register_b_transport(this, &arm_scu::b_access);

    // We are going to have contiguous memory 
    // and use function offset to access registers
    char lname[64];
    snprintf(lname, 64, "SCU_Registers");
    m_registers = new smallram<u8_t>(lname,
				     4, // number of bytes in a single word
				     8, // total number of words
				     this);
    
    u32_t allzeros = 0x00000000;
    // TODO: this is hard coded for now but it should
    // try to get this information from some configuration
    // XML file or from caches that are connected to
    // snoop control unit

    // Control register (0x0), inform that SCU is enabled
    u32_t control = 0x00000001;
    m_registers->writepo(0x0, (u8_t*)(&control), 4);
    
    // Configuration register (0x4)
    // b[1:0] = 01 -> two Cortex-A9 processors, CPU0 and CPU1
    // b[7:4] = 0011 -> CPU0 and CPU1 are in SMP mode taking part in coherency
    // b[15:8] = 00000101 -> CPU0 and CPU1 are 32KB caches, 128 indexes per tag ram
    u32_t configuration = 0x00000531;
    m_registers->writepo(0x4, (u8_t*)(&configuration), 4);

    // Power status register (0x8), all CPUs power status is normal
    m_registers->writepo(0x8, (u8_t*)(&allzeros), 4);

    // Invalidate all registers in secure state
    m_registers->writepo(0xC, (u8_t*)(&allzeros), 4);

    // Filtering start address registers
    m_registers->writepo(0x10, (u8_t*)(&allzeros), 4);

    // Filtering end address registers
    m_registers->writepo(0x14, (u8_t*)(&allzeros), 4);

    // Access control register -> indicates that all CPUs
    // can access this resource
    u32_t access = 0x00000001; 
    m_registers->writepo(0x18, (u8_t*)(&allzeros), 4);
    
    // Non-secure access control register
    m_registers->writepo(0x1C, (u8_t*)(&allzeros), 4);
}

void arm_scu::b_access(int idx, PRAZOR_GP_T &trans, sc_time &delay) {
    // check to see if it has cp15 extension
    cp15_cache_control_extension* ext = 0;
    trans.get_extension(ext);
    if(ext) {
        if((ext->op & (1 << cp15_cache_control_extension::Clean)) ||
           (ext->op & (1 << cp15_cache_control_extension::Invalid))) {
            // we are going to forward invalidation to all cache
            // even those that we have received invalidation from
            // to be on the safe side
            // TODO: change payload to have enough information
            // so that we can decide where we need to send 
            // exactly invalidations
            int listeners = arm_initiator_socket->size();
            // this invalidation is not going to be forwarded further
            ext->snooped = true;
            trans.set_extension(ext);
            for(int i = 0; i < listeners - 1; i++) {
                (*arm_initiator_socket)[i]->b_transport(trans, delay);
            }
        }

        ext->snooped = false;
        trans.set_extension(ext);
        // check to see if stop is point of coherence if it is
        // we simply return, otherwise we forward       
        if(ext->stop != cp15_cache_control_extension::PoU) {
            assert(arm_initiator_socket);
            (*arm_initiator_socket)[arm_initiator_socket->size()-1]->b_transport(trans, delay);
        }
    }
    else {
        tlm::tlm_command cmd = trans.get_command();
        u32_t adr = (u32_t)trans.get_address();
        u8_t* ptr = trans.get_data_ptr();

	// TODO: for now are are only aware of read accesses to
	// this component, MP to implement write accesses once
	// we start seeing them
        assert(cmd == tlm::TLM_READ_COMMAND);
	memcpy(ptr, m_registers->readpo(offset(adr)), trans.get_data_length());
    }
    
    trans.set_response_status(tlm::TLM_OK_RESPONSE);
}

void arm_scu::init_arm_control_socket() {
    if(!arm_initiator_socket)
        arm_initiator_socket =
            new tlm_utils::multi_passthrough_initiator_socket<arm_scu, 64, PW_TLM_TYPES>;
}

u32_t arm_scu::offset(u32_t address) {
  u32_t offset = address - SCU_BASE_ADDRESS;

  switch(offset) {

    case 0x0:
    case 0x4:
    case 0x8:
    case 0xc: {
      return offset;
    }

    // in Linux kernel source there is a read at this
    // address saying that it is only applicable for
    // cortex-A9. (arch/arm/kernel/smp_scu.c) It looks 
    // like that at this address we are supposed to have 
    // control register
    case 0x30: {
        return offset - 0x30;
    }

    case 0x40:
    case 0x44: {
      return offset - 0x30;
    }

    case 0x50:
    case 0x54: {
      return offset - 0x38;
    }

    default: {
      // we should never reach this 
      // if we reach this point it means that 
      // register that we want to access is not 
      // within offsets that are in documentation
      printf("ARM SCU unit is trying to read address 0x%08x\n", address);
      assert(0);
    }
  }
}

// eof


