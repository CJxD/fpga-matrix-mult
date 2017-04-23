//
// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2009-10 DJ Greaves (TLM 2.0 Version Arturs Prieditis) 
// Blocking TLM wrapper for the verilated OR1200 core.
// $Id: OR1200.h,v 1.2 2011-02-15 15:13:33 djg11 Exp $
//
#ifndef OR1200_H
#define OR1200_H

#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/tlm_quantumkeeper.h"


#include "vcpp/Vor1200_top.h"
#include "OrpsocAccess.h"


#include "socdam_bdoor_mapping.h"
#include "../spr_defs.h"
#include "COREISA_IF.h"

#define BENCH_RESET_TIME 10


class OR1200V :
  public COREISA_IF
{
  typedef u32_t oraddr_t;
  bool *ext_interrupt;
  void update(bool c);
  Vor1200_top or1200_cpu;

  OrpsocAccess accessor; 
  u1_t over;

 private:

  struct ins_fetcher_t
  { // Instruction mini-cache for other half of a 64 bit word.
    COREISA_IF *parent;
    u64_t cached_adr, data;

  ins_fetcher_t(COREISA_IF *parent) : parent(parent) { cached_adr = 1; // an invalid address
    };
    void fetch(u32_t adr, u32_t &i, lt_delay &delay);
  } ins_fetcher;



  void monitor_or_decode();
  void corepause(int us);  // Pause CPU for this time interval

  POWER3(pw_energy instruction_energy);

  struct stats_t
  {


    void reset()
    {

    }

    stats_t() { reset(); }

  } stats;

 private:
  lt_delay master_runahead;
  void ifetch_step(lt_delay &delay);
  void run();
  //tlm_utils::tlm_quantumkeeper m_qk; // Quantum keeper for temporal decoupling 

  int simulator_cmd(int cmd, int arg0, int arg1, u32_t *rp);

  int eval_bdoor_read(oraddr_t memaddr, u32_t &rdata, lt_delay &lt_d_delay);
  int eval_bdoor_write(oraddr_t memaddr, u32_t wdata, lt_delay &lt_d_delay);

 public:

  void dumpregs(FILE *fd, int allregs);
  // Constructor
  OR1200V(sc_core::sc_module_name name, uint8_t pID, bool harvardf=false);

  // Todo: make this work so parent can bind directly to me.
  //sc_export<tlm_utils::simple_initiator_socket<OR1200, 64, PW_TLM_PAYTYPE> > initiator_socket;

  void sim_done(const char *msg);
  void atomic_prefix(); // Prefix following load/store pair as atomic.
  // COREISA_IF interface methods:
  void stat_report(const char *msg, FILE *fd, bool resetf=false);
  const char *kind() { return "OR1200V"; }
  void reset(bool selfstart);
  bool *connect_interrupt(bool *a);

  void /*override*/diagnostic_report(FILE *fd, int severity, const char *msg)
  {
    dumpregs(fd, 1);
  }

};


// eof
#endif
