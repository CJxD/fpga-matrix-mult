/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

/*
 * Testbench that resembles DJG's AMD Phenom
 */

#include <platform/platform.h>

#include <systemc>
#include <tlm.h>
#include "prazor.h"

#include <dramsim2/dramsim_sc_wrapper.h>
#include <io/uart64_cbg.h>
#include <memories/ccache.h>
#include <memories/scu.h>
#include <tenos/COREISA_IF.h>
#include <x86/x86core.h>

#define ADDR_IN_BITS 64

#define L1_CACHE_SIZE (1<<13)
#define L1_CACHE_LINE_SIZE 8
#define L1_CACHE_WAYS 4

#define L2_CACHE_SIZE (1<<16)
#define L2_CACHE_LINE_SIZE 8
#define L2_CACHE_WAYS 4

#define DRAM_SIZE (0x2000000 * 8)

//#define LEAVEOUT_L1_CACHE 1
//#define LEAVEOUT_L2_CACHE 1

extern int g_coremask;
extern int g_cores;
extern const char* g_name;
extern int g_watches;
extern int g_tracelevel;

class coreunit 
  : public sc_module
#ifdef TLM_POWER3
  , public pw_module
#endif
{
 public:
  COREISA_IF *core;
#ifndef LEAVEOUT_L1_CACHE
  ccache *l1_d_cache;
  ccache *l1_i_cache;
#endif

  // Constructor
  coreunit(sc_core::sc_module_name p_name,
	   int i,
	   scu* snoop,
	   bool harvardf);
  
  // Statistics
  void stat_report(const char *msg,
		   FILE *fd,
		   bool resetf = 0);

  const char *kind();
};   

class top :
  public platform, 
  public sc_module
#ifdef TLM_POWER3
  , public pw_module
#endif
{
 private:
  
  uart64_cbg uart0;
#ifndef LEAVEOUT_L2_CACHE
  ccache* l2_cache;
#endif
  scu* snoop;
  
 public:
  coreunit **Cores;
  
  dramsim_sc_wrapper *memory0;

  void reset(const char* image_lo, const char* image_hi);

#define iospace_base 0xFFFFffffe0000000LLU

  // Default constructor
  top(const sc_module_name& p_name, 
      const char* device_ini,
      const char* system_ini);

  void stat_report(const char *msg,
		   FILE *fd,
		   bool resetf = 0);

  void hup_handler(int p_arg);

  sc_module* get_Core(int i);
};
