// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2014 M Puzovic + D J Greaves
// snoop control unit

// This is for now a class that is acting
// as SCU control register

// This is needed in order to for kernel to boot as SMP system

#ifndef __ARMSCU_H__
#define __ARMSCU_H__

#include "../memories/scu.h"
#include "../memories/smallram.h"
#include "tlm_utils/multi_passthrough_initiator_socket.h"
#include "tlm_utils/multi_passthrough_target_socket.h"
#include "armcore_tlm.h"

// acording to documentation SCU is at this address (Zynq-7000-TRM pp. 115 Table 4-4)
#define SCU_BASE_ADDRESS PERIPHBASE 
#define SCU_SIZE 0xFC
#define SCU_REGISTERS 8

// From documentation ARM SCU has the following registers:
// +----------+---------------------------------------------+
// | Offset   | Name                                        |
// +----------+---------------------------------------------+
// | 0x0      | Control Register                            |
// | 0x04     | Configuration Register                      |
// | 0x08     | CPU Power Status Register                   |
// | 0x0C     | Invalidate All Registers in Secure State    | 
// | 0x40     | Filtering Start Address Register            |
// | 0x44     | Filtering End Address Register              |
// | 0x50     | Access Control (SAC) Register               | 
// | 0x54     | Non-secure Access Control (SNSAC) Register  |
// +----------+---------------------------------------------+

class arm_scu : public smallramp, public scu
{

  private:
    // using smallram to store 'registers'
    // that are kept by SCU, the store is
    // byte addressable (TODO: check whether
    // store should be byte- or word- addressable
    smallram<u8_t>* m_registers;

  public:
    // Constructor
    arm_scu(sc_module_name);

    // connecting port
    tlm_utils::multi_passthrough_target_socket<arm_scu, 64, PW_TLM_TYPES> arm_target_socket;
    tlm_utils::multi_passthrough_initiator_socket<arm_scu, 64, PW_TLM_TYPES>* arm_initiator_socket;

    void b_access(int idx, PRAZOR_GP_T &trans, sc_time &delay);

    // if arm_initiator_socket is NULL it will be allocated
    // if it is not NULL this method does nothing
    void init_arm_control_socket();

#ifdef TLM_POWER3
    // TODO: MP to see with DJG how to account for power in this unit
    virtual void log_energy_use(pw_energy p) {
      // TODO: empty for now
    }
#endif

 private:
    u32_t offset(u32_t address);

};

#endif
