// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2014 M Puzovic + D J Greaves

#ifndef __ARM_CP14__
#define __ARM_CP14__


// ARM coprocessor number 14

#include "tenos.h"
#include "systemc.h"
#include "tlm.h"
#include "../memories/smallram.h"
#include "tlm_utils/multi_passthrough_initiator_socket.h"
#include "tlm_utils/multi_passthrough_target_socket.h"

#ifdef TLM_POWER3
#include <tlm_power>
#define POWER3(X) X
using namespace sc_pwr;
#else
typedef tlm::tlm_base_protocol_types PW_TLM_TYPES;
typedef tlm::tlm_generic_payload PW_TLM_PAYTYPE;
#define POWER3(X)
#endif

class armcore_tlm;

class arm_cp14 : public smallramp, public sc_module
#ifdef TLM_POWER3
  , public pw_module
#endif
{

  private:
    // using smallram to store 'registers'
    // that are kept by SCU, the store is
    // byte addressable (TODO: check whether
    // store should be byte- or word- addressable
    smallram<u8_t>* m_registers;

  public:
    // Constructor
    arm_cp14(sc_module_name);

    // connecting port
    tlm_utils::multi_passthrough_target_socket<arm_cp14, 64, PW_TLM_TYPES> cp14_target_socket;

    void b_access(int idx, PW_TLM_PAYTYPE &trans, sc_time &delay);

#ifdef TLM_POWER3
    // TODO: MP to see with DJG how to account for power in this unit
    virtual void log_energy_use(pw_energy p) {
      // TODO: empty for now
    }
#endif
};

#endif
