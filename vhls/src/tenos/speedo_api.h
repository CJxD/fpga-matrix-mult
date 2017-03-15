/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

//
// spEEDO is/was a project funded by the UK TSB to define a strawman debug and register API so that energy accounting information
// can be distributed throughout a SoC and its embedded operating system.
//
// This file is included by busaccess to implement the spEEDO registers.
//
#ifndef SPEEDO_API_H
#define SPEEDO_API_H

#include <stdint.h>
#include "systemc.h"
#ifdef TLM_POWER3
#include <tlm_power>
#endif

/*
 When a context swap is done in h/w owing to watchpoint regs and so on, the 
 C-api could easily get confused.  E.g. it just checkpoints on each thread
 swap and does not expect things to change under its feet.

 We need to impose an hierarchy on these, with, for instance, all h/w contexts
 for a core being swapped by the thread swap.

 So, we need dynamically changing customer numbers in TLM power, or the busaccess to dynamically change
 its customer identity.


 */

class speedo_resource_ifc // an extra base for cpu_busaccess to implement.
{
 public:
  virtual int local_energy() = 0;
  virtual void resume_accounting_alias(int) = 0;
};

class speedo_ifc_t   // spEEDO energy interface
{
  class speedo_resource_ifc &parent;
 public:
#define SPEEDO_CONTEXTS 4  
  typedef struct speedo_context_s
  {
    uint32_t time;
    int local_energy, global_energy;
    speedo_context_s() // constructor
    {
      time = 0;
      local_energy = 0;
      global_energy = 0;
    }
  } slaved_ctx_t;
  slaved_ctx_t Slaved_ctxs[SPEEDO_CONTEXTS];
  int speedo_api(/*addr_t*/uint64_t addr, uint64_t &data, bool writef, sc_time &delay, int procID);
  int current_ctx;


 speedo_ifc_t(speedo_resource_ifc &parent): // constructor
  parent(parent) 
  {
    current_ctx = 0;
  };
  
  void snapshop_ctx(speedo_context_s &dest, sc_time &delay);
  int local_energy();
  
};

#endif
// eof
