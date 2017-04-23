/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

#ifndef COREISA_IF_H
#define COREISA_IF_H

#include "systemc.h"
#include "tlm.h"
#include "prazor.h"
#include "tenos.h"
#include "../gdbrsp/vhls_soc_debug.h"
#include "cpu_busaccess.h"

// An interface/wrapper for various forms of the CPU (e.g. fast, slow, power-annotated, verilated ...).


extern int g_dmi_disablef;

#define VISIBLE __attribute__ ((visibility("default")))

class VISIBLE COREISA_IF : // CPU interface
public sc_core::sc_module,
  public vhls_debug_comms,
#ifdef TLM_POWER3
  public pw_module,
#endif
  public tenos_diagnostic_if
{

 public:
  SC_HAS_PROCESS(COREISA_IF);
  uint8_t procID;
  bool harvardf;
  u64_t reset_vector;
  int reset_or_yield_countdown; // Number of clock perids to hold off before next instruction start.

  // Todo: make this work so parent can bind directly to me.
  //sc_export<tlm_utils::simple_initiator_socket<OR1200, 64, PW_TLM_TYPES> > initiator_socket;

  double m_average_ipc; // Instructions per clock on average excluding load/store overheads.
  sc_time m_bus_period; // Reciprocal of front side bus frequency
  sc_time m_core_period; // Reciprocal of core clock frequency
  sc_time m_effective_instruction_period; // Core_period scaled by average nominal IPC
  sc_time m_mispredict_time_penalty; // Time lost in a branch misprediction.

  void set_core_frequency(double frequency, double average_ipc=1.0)
  {
    m_core_period = sc_time(1.0/frequency * 1e9, SC_NS);
    m_average_ipc = average_ipc;
    m_effective_instruction_period = m_core_period / m_average_ipc;
    m_mispredict_time_penalty = m_effective_instruction_period * 5; // Wild guess
  }

  void set_clock_frequencies(sc_time bus, sc_time core) 
    {
      m_bus_period = bus;
      m_core_period = core;
      m_effective_instruction_period = m_core_period / m_average_ipc;
    }

  void register_reset_vector(u64_t a) { reset_vector = a; };

  virtual void run()=0;
  //virtual bool *connect_interrupt(bool *a)=0;
  virtual void stat_report(const char *msg, FILE *fd, bool resetf=false)=0;
  virtual const char *kind() { return "COREISA_IF"; }
  virtual void reset(bool selfstart)=0;
  virtual u64_t install_image(const char* image) {
    // do nothing by default
  }


  virtual int simulator_cmd(int cmd, int arg0, int arg1, u32_t *rp) = 0;
  cpu_busaccess busaccess;
  void traceregion_add(traceregion *n) { n->add(busaccess.traceregions); }

  u64_t xlat32to64(u32_t a)
  {
    // 32 bit I/O space is defined to be when the top four bits of the address are set.
    // Caches should not cache this!
    u64_t r = a;
    if ((a & 0xF0000000)==0xF0000000) r |= 0xFFFFffff00000000LLU;
    return r;
  }

 COREISA_IF(sc_core::sc_module_name names, int endian_flip, u8_t pID, bool harvardf, int countdown=0) : // Constructor.
  sc_module(names),
    vhls_debug_comms(names),
    reset_or_yield_countdown(countdown),
    harvardf(harvardf),
    tenos_diagnostic_if(names),
    busaccess(this, "busaccess", endian_flip, pID, harvardf?1:0, g_dmi_disablef),
    m_bus_period(5, SC_NS),
    m_core_period(5, SC_NS),
    procID(pID)
      { 
	m_average_ipc = 1.0;
	m_effective_instruction_period = m_core_period / m_average_ipc;
      };

  //
 private:
    COREISA_IF(); // This constructor is hidden since one should never directly instantiate this interface.

 public:
  virtual ~COREISA_IF() 
    {
      
    };

};




#endif
