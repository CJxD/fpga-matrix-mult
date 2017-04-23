/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

#ifndef X86CORE_H
#define X86CORE_H

#include "tlm.h"
#include "tlm_utils/tlm_quantumkeeper.h"

#include "../memories/memloaders.h"

#include "COREISA_IF.h"

#include <ptlhwdef.h>
#include <osbackdoor.h>

struct SequentialCore;

class exec_image;

class x86core : public COREISA_IF
{
 public:
  u64_t cycles;

  struct  {
    u64_t insns;
    u64_t uops;
    u64_t pauses;
    u64_t basicblocks;

    void reset() { 
      insns = 0;
      uops = 0;
      pauses = 0;
      basicblocks = 0;
    }
  } corestats;

  void run();
  bool* connect_interrupt(bool*);
  void stat_report(const char*, FILE*, bool);
  const char* kind() { return "x86core"; }
  void reset(bool);
  u64_t install_image(const char*);

  
  //tlm_utils::tlm_quantumkeeper m_qk; // Quantum keeper for temporal decoupling 

  bool halted; 
  lt_delay master_runahead, lt_i_delay, lt_d_delay;

  int simulator_cmd(int cmd, int arg0, int arg1, u32_t *rp);

  // Constructor
  x86core(sc_core::sc_module_name, u8_t, int stopctrl = -1);

 private:  
  bool stop_sim_on_exit;
  BasicBlock* current_bb;
  RIPVirtPhys fetchrip;

  int bb_transop_index;

  // Sequential core from PLTSim
  SequentialCore* me;

  // need this in order to be able to use
  // basic block optimisation from PTLSim - can multiple kernels load different images in non-homo address space?
  exec_image* im;


  int os_cmd(const char cmd, char *data, long long int addr, int len);
};

class exec_image : public memloader_if {
  public:
   virtual bool ip_write(u64_t a, u8_t d, bool es = false);
   virtual bool ip_read(u64_t a, u8_t &d);

  exec_image(x86core* c) : core(c) {}

  private:
   x86core* core;
};


#endif
