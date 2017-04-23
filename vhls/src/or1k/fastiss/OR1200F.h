//
// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2009-14 DJ Greaves (TLM 2.0 Version Arturs Prieditis) 
// SystemC TLM wrapper for fast ISS of OR1200 processor core.
// $Id: OR1200.h $
//
#ifndef OR1200F_H
#define OR1200F_H

#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/tlm_quantumkeeper.h"
#include "cpu_busaccess.h"

extern "C"
{
   #include "orsim.h"
}

#include "COREISA_IF.h"

#include "socdam_bdoor_mapping.h"
#include "../spr_defs.h"
#include "../remotecontrol_or1k.h"

#define BENCH_RESET_TIME 10


class OR1200F : 
  public COREISA_IF,
  public orsim
{
  remotecontrol_or1k debug_controller;
  bool *ext_interrupt;
  lt_delay master_runahead;
  lt_delay lt_i_delay;
  lt_delay lt_d_delay;
  u32_t where_last;
  u32_t eval_insn(oraddr_t a, int*);
  void run();
  u1_t over;
  int simulator_cmd(int cmd, int arg0, int arg1, u32_t *rp);

  int eval_bdoor_read(oraddr_t memaddr, u32_t &rdata);
  int eval_bdoor_write(oraddr_t memaddr, u32_t wdata);
  u32_t eval_mem32(oraddr_t a, int*);
  u16_t eval_mem16(oraddr_t a, int*);
  u8_t eval_mem8(oraddr_t a, int*);
  u8_t eval_direct8(oraddr_t a, int, int);
  void set_direct8(oraddr_t a, u8_t d, int, int);
  void set_mem32(oraddr_t a, u32_t d, int*);
  void set_mem16(oraddr_t a, u16_t d, int*);
  void set_mem8(oraddr_t, uint8_t, int*);
  void sim_done(const char *msg);
  //tlm_utils::tlm_quantumkeeper m_qk; // Quantum keeper for temporal decoupling 
  void decode();
  void corepause(int us, u32_t addr);  // Pause CPU for this time interval
  void atomic_prefix(); // Prefix following load/store pair as atomic.
  void serve_debugger();
  struct stats_t
  {
    void reset()
    {

    }

    stats_t() { reset(); }

  } stats;

  POWER3(pw_energy instruction_energy);

  struct ins_fetcher_t
  { // Instruction mini-cache for other half of a 64 bit word.
    COREISA_IF *parent;
    u64_t cached_adr, data;

  ins_fetcher_t(COREISA_IF *parent) : parent(parent) { cached_adr = 1; // an invalid address
    };
    void fetch(u32_t adr, u32_t &i, lt_delay &lt_busdelay);
  } ins_fetcher;


 public:
  virtual void dummy () { } ;
  OR1200F(sc_core::sc_module_name, u8_t pID, bool harvardf=false); // constructor

  // Todo: make this work so parent can bind directly to me.
  //sc_export<tlm_utils::simple_initiator_socket<OR1200, 64> > initiator_socket;

  void stat_report(const char *msg, FILE *fd, bool resetf=false);
  const char *kind() { return "OR1200F"; }
  void reset(bool selfstart);
  bool *connect_interrupt(bool *a);
  void /*override*/diagnostic_report(FILE *fd, int severity, const char *msg)
  {
    printf("%s: TODO dumpregs(fd, 1);\n\n", name());
  }

};


// eof
#endif
