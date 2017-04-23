#ifndef MIPS64_H
#define MIPS64_H

//
// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2009-14 DJ Greaves (TLM 2.0 Version Arturs Prieditis) 
// Blocking TLM wrapper for the fast versions of the MIPS64 core.
// $Id: mips64_btlm $
//

#include "systemc.h"
#include "prazor.h"
#include "tlm.h"
#include "tlm_utils/tlm_quantumkeeper.h"
#include "tlm_2_interfaces/tlm_dmi.h"



#include "mips64.h"


#define BENCH_RESET_TIME 10


class mips64_btlm : public mips64, public sc_module
#ifdef TLM_POWER3
 , public pw_module
#endif
{
#ifdef TLM_POWER3
  sc_pwr::pw_tlm_paytype_mm_t payload_mm;
#else
  llsc_mm_t payload_mm;
#endif

  POWER3(void energy_event(pw_energy v));
  POWER3(sc_pwr::tlm_bit_transition_tracker frontside_bus); // Used if counting wire transitions. 

  PW_TLM_PAYTYPE *llsc_payload_alloc();


  u64_t atomic_wdata, atomic_addr;
  bool harvardf;

 public:
  mips64_btlm(sc_core::sc_module_name, uint8_t pID, bool harvardf); // constructor
  cpu_busaccess busaccess;
  uint32_t where_last;



  int reg_trace(mips_addr_t trace_pc)
    {
      if (!busaccess.traceregions) return false;
      int m = busaccess.traceregions->check(trace_pc, TENOS_TRACE_CPU_KEY_REGS|TENOS_TRACE_CPU_ALL_REGS);
      return m;
    }



  struct stats_t
  {
    

    void reset()
    {
      //ticks = 0;
    }

    stats_t() { reset(); }

  } stats;


  void reset(bool enable_first_to_run);

 protected:
  lt_delay lt_busdelay;    // Loosely timed delay on external ops.

 private:
  lt_delay master_runahead;
  //tlm_utils::tlm_quantumkeeper m_qk; // Quantum keeper for temporal decoupling 
  bool m_over;
  bool *ext_interrupt; // How to direct to one spoke?
  void hw_interrupt(const char *);
  int simulator_cmd(int cmd, int arg0, int arg1);
 public:
  sc_time bus_period; // Reciprocal of front side bus frequency
  sc_time core_period; // Reciprocal of core clock frequency

  bool halted;  // True if not running (waiting for interrupt or other wakeup).

  void set_clock_frequencies(sc_time bus, sc_time core) 
    {
      bus_period = bus;
      core_period = core;
    }

  bool *connect_interrupt(bool *a);
  uint8_t procID;
  void run();
  // The following I/O functions are pure virtual in the ISS core, and must be provided here:
  void                corepause(int us);  // Pause CPU for this time interval
  //u32_t             eval_insn(mips_addr_t addr, breakpt_t *bp);
  bool                eval_mem64 (mips_addr_t memaddr, u64_t &rdata, breakpt_t *bp, bool update_presence_flag=false, bool new_polarity=false);
  bool                eval_mem32 (mips_addr_t memaddr, u32_t &d, breakpt_t *bp, bool linked=false);
  bool                eval_mem16 (mips_addr_t memaddr, u16_t &d, breakpt_t *bp);
  bool                eval_mem8 (mips_addr_t memaddr, u8_t &d, breakpt_t *bp);
  void                set_mem64 (mips_addr_t memaddr, u64_t value, breakpt_t *bp, bool);
  void                set_mem32 (mips_addr_t memaddr, u32_t value, breakpt_t *bp);
  void                set_mem16 (mips_addr_t memaddr, u16_t value, breakpt_t *bp);
  void                set_mem8 (mips_addr_t memaddr, u8_t value, breakpt_t *bp);
  //  u64_t               eval_exchg(mips_addr_t memaddr, u64_t wdata, sc_time &delay);
  int                 cpu_mem_store_conditional32(uint64_t a, uint32_t d);
  void                set_direct8 (mips_addr_t, u8_t, int, breakpt_t *bp);
  uint8_t             eval_direct8(mips_addr_t, int, int, breakpt_t *bp);
  void                atomic_prefix(); // Prefix following load/store pair as atomic.
  //bool                mamba_tastac(mips_addr_t memaddr, u64_t &rdata, sc_time &delay, bool new_polarity);

  void sim_done();

  //bool mem_read(mips_addr_t addr, u64_t &d, sc_time &delay, bool update_presence_flag=false, bool new_polarity=false, bool linked=false);
  //  void mem_write(mips_addr_t addr, u8_t mask, u64_t wdata, sc_time &delay);

  void stat_report(const char *msg, FILE *fd, bool resetf=false);
};

#endif

// eof
