#ifndef PRAZOR_H
#define PRAZOR_H
//
// parzor.h CBG PRAZOR pre-layout power estimation and accounting framework
// $Id: prazor.h,v 1.4 2010/09/24 10:49:52 djg11 Exp $ 
// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2010 DJ Greaves, University of Cambridge, Computer Laboratory.
//

// SystemC modules should be defined with SC_MODULE_PR instead of SC_MODULE

// This module is unused now - we use TLM POWER3 instead.


#include "tenos.h"

#ifdef PRAZOR
#define IF_PRAZOR(X) X
#define SC_MODULE_PR(XNAME) \
  struct  XNAME : public sc_module_pr
#else
#define IF_PRAZOR(X) 
#define SC_MODULE_PR(XNAME) SC_MODULE(XNAME)
#endif

typedef unsigned int prazor_nano_t;

class prazor_size_t
{
 public:
  prazor_nano_t x, y, z; // Units of nanometers.
  prazor_size_t(prazor_nano_t x, prazor_nano_t y, prazor_nano_t z) : x(x), y(y), z(z) { };
  prazor_size_t() : x(-1), y(-1), z(0) { };
};

class prazor_position_t
{
 public:
  prazor_nano_t x, y, z; // Units of nanometers.
  prazor_position_t(prazor_nano_t x, prazor_nano_t y, prazor_nano_t z) : x(x), y(y), z(z) { };
  prazor_position_t() : x(-1), y(-1), z(0) { };
};

#define PR_NANOM(X)  (X)
#define PR_MICRON(X) ((X)*1000)


// Energy is internally stored in nano-Joules.
typedef unsigned long long int prazor_energy_t;
#define PR_PICOJ(X) (((X)+500)/1000)
#define PR_NANOJ(X) ((X))

// Power is internally stored in nano watts
typedef unsigned long long int prazor_power_t;
#define PR_PICOW(X) (((X)+500)/1000)
#define PR_NANOW(X) ((X))
#define PR_MICROW(X) ((X)*1000)

struct sc_module_pr;

class prazor_manager
{
 public:
  static prazor_energy_t total_energy;

  static void make_report(const char *filename);

  static double dynamic_energy_joules(prazor_energy_t p)
  {
    return 1.0e-9 * ((double) p);
  }

  static double dynamic_power_watts(prazor_power_t p)
  {
    return 1.0e-9 * ((double) p);
  }


 protected:
  friend class sc_module_pr;
  static class sc_module_pr *components;
};


// User SystemC modules extend sc_module_pr instead of the OSCI library sc_module.

struct sc_module_pr : ::sc_core::sc_module
{
  class sc_module_pr *next_component; 

 public:
  prazor_position_t      pr_position;
  prazor_size_t          pr_dims;
  prazor_energy_t        energy_current, energy_previously;
  prazor_power_t         static_power;
   
  // constructor
  sc_module_pr (::sc_core::sc_module_name &name);


  void pr_size(prazor_nano_t wid, prazor_nano_t len) // Set my dimensions
  {
    pr_dims.x = wid;
    pr_dims.y = len;
  }

  void pr_static_power(prazor_power_t s)  // Record my static power consumption
  {
    static_power = s;
  }

  void pr_dynamic_event(prazor_energy_t use) // Log dynamic consumption of a fraction of a Joule.
  {
    energy_current += use;
  }


  prazor_energy_t report(FILE *fd);
};


#endif
