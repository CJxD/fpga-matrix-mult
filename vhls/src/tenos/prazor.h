#ifndef PRAZOR_H
#define PRAZOR_H
//
// parzor.h CBG PRAZOR pre-layout power estimation and accounting framework
// $Id: prazor.h,v 1.4 2010/09/24 10:49:52 djg11 Exp $ 
// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2010 DJ Greaves, University of Cambridge, Computer Laboratory.
//


#include "tenos.h"
#include "lt_delay.h"

#ifdef TLM_POWER3
#define TEMP_PT_BYPASS 1
#include <tlm_power>
#define POWER3(X) X
using namespace sc_pwr;
#else
typedef tlm::tlm_base_protocol_types PW_TLM_TYPES;
typedef tlm::tlm_generic_payload PW_TLM_PAYTYPE;
#define POWER3(X)
#endif

class prazor_gp_t;

class prazor_gp_mm_t: public tlm::tlm_mm_interface
{
 public:
  prazor_gp_t* allocate();

  void free(tlm::tlm_generic_payload* disposeme);

  static prazor_gp_mm_t* instance();
  
  void free(prazor_gp_t *disposeme);

 protected:
  /* Private constructor */
 prazor_gp_mm_t() 
  : free_list(0),
    empties(0),
    lck("prazor_gp_mm_t") {
  }
  
 private:
  struct access {
    prazor_gp_t* trans;
    access* next;
    access* prev;
  };

  access* free_list;
  access* empties;

  sc_mutex lck;
  
  prazor_gp_t *freelist;
  static prazor_gp_mm_t* _instance; 
};


class prazor_gp_t : public PW_TLM_PAYTYPE
{
 public:
  lt_delay ltd;

  // Constructors 1/2
 prazor_gp_t() 
   : PW_TLM_PAYTYPE() {
  }

  // Constructors 2/2
  explicit prazor_gp_t(prazor_gp_mm_t* mm) 
    : PW_TLM_PAYTYPE(mm) {
    }
 
};


typedef prazor_gp_t PRAZOR_GP_T;


struct tlm_pw_base_protocol_types
{
  //  typedef PW_TLM_PAYTYPE tlm_payload_type;
  typedef prazor_gp_t tlm_payload_type;
  typedef tlm::tlm_phase tlm_phase_type;
};

typedef tlm_pw_base_protocol_types PW_TLM_TYPES;


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


// These macros are intended as a bridging feature as we migrate from the old sc_time to the new lt_dleay.
// Use this macro to register a delay in the loose-time system.
// I have renamed the variable delay as delay_ in many places to show we now deprecate it.
#define  AUGMENT_LT_DELAY(TRANS, DELAY, LL)  TRANS += (LL)
#define  COLLECT_LT_DELAY(OLD, NEW)  (NEW)
#define  LT_RESYNCH(OLD, NEW)  NEW.force_synch() /* or wait(OLD); OLD = SC_ZERO_TIME; */
#endif
