// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2014 DJ Greaves and M Puzovic.
//
//
#ifndef SCLR_ARM_TLM
#define SCLR_ARM_TLM

/*
ARM generic interrupt controller.
=================================


The nominal base address for Zynq use is f8f01000 (but this is set outside the GIC by its busmux address decoding).

It does not (initially) support the security or hypervisor extensions.
Whether it needs to do interrupt grouping is unclear but probably it
will.



*/

#include "tenos.h"
#include "systemc.h"
#include "tlm.h"
#include "../memories/smallram.h"
#include "tlm_utils/multi_passthrough_target_socket.h"
#include "armcore_tlm.h"

// We will support up to this number of cores by default. The should be no overhead associated with unused core ports.
#define GIC_CLIENT_CORES 32
#define SLCR_BASE_ADDR 0xF8000000
#define SLCR_LAST_REG_OFFSET 0xB74

#ifdef TLM_POWER3
#include <tlm_power>
#define POWER3(X) X
using namespace sc_pwr;
#else
typedef tlm::tlm_base_protocol_types PW_TLM_TYPES;
typedef tlm::tlm_generic_payload PW_TLM_PAYTYPE;
#define POWER3(X)
#endif

class sclr_arm_tlm : public smallramp, public sc_module
#ifdef TLM_POWER3
, public pw_module
#endif
{
  traceregion *traceregions;
  sc_time latency;
  const char *kind();

  // blocking transport
  void b_access(int id, PW_TLM_PAYTYPE &trans, sc_time &delay);

 public:

  //TLM-2 socket, defaults to 32-bits wide, power protocol
  //tlm_utils::simple_target_socket<uart64_cbg, 64, PW_TLM_TYPES> port0;
  tlm_utils::multi_passthrough_target_socket<sclr_arm_tlm, 64, PW_TLM_TYPES> cpu_target0;

  // constructor
  sclr_arm_tlm(sc_module_name name);

  POWER3(PW_TLM3(sc_pwr::tlm_bit_transition_tracker read_bus_tracker)); // Used if counting wire transitions. 
#ifdef TLM_POWER3
      // TODO: MP to see with DJG how to account for power in this unit
    virtual void log_energy_use(pw_energy p) {
      // TODO: empty for now
    }
#endif

    void add_core(armcore_tlm* core);

 private:
    smallram<u8_t>* m_registers;
    bool writeprotect;

    std::vector<armcore_tlm*> cores;
};







#endif

