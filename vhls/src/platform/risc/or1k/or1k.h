/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

//
// An example testbench. Multiple cores with caches.
//
// University of Cambridge, Computer Laboratory.
// VHLS/PRAZOR TLM2.0 Simulator
//
//
#include <platform/platform.h>
#include <systemc>
#include <tlm.h>
#include "prazor.h"

#define NEWCCACHE 1

#ifdef USE_VERILATED
#include "OR1200V.h"
#endif

#ifdef USE_FASTISS
#include "OR1200F.h"
#endif

#include <tenos/COREISA_IF.h>

#include <bus/busmux64.h>
#include <memories/sram64_cbg.h>
#include <memories/dram64_cbg.h>
#ifdef NEWCCACHE
#include <memories/ccache.h>
#include <memories/scu.h>
#else
#include <memories/consistent_cache64.h>
#endif
#include <dramsim2/dramsim_sc_wrapper.h>
#include <gdbrsp/vhls_soc_debug.h>

#ifdef USE_ETHERCRC
#include <ethercrc/ethercrc_tlm.h>
#endif

#include <io/uart64_cbg.h>

// 64 KBytes or so. (cache size is in 8 byte words?)
// Comment out the next line to remove the cache.
// A command line flag is also now implemented.
#define L1_CACHE_SIZE (1<<13)
#define L2_CACHE_SIZE (1<<16)

#define USE_MAINMEM_MACRO_MODEL  0

#define MAX_NO_OF_CORES 256

extern const char* g_name;
extern int g_no_caches;
extern bool g_wait_debugger;
extern int g_tracelevel;

#ifdef TLM_POWER3
pw_trace_file* g_vcd_pt = NULL;
pw_trace_file* g_txt_pt = NULL;
pw_trace_file* g_slk_pt = NULL;
#endif // TLM_POWER3

int g_icache_l1_ways = 8;
int g_dcache_l1_ways = 8;
int g_cache_l2_ways = 8;
int g_uart_use_x11 = 1;

#ifndef NEWCCACHE
#include "../memories/consistent_cache64.h"
cc_protocol_ty g_cache_protocol = MESI_CC;  //   or  MOESI_CC
#endif

// A single core + level optional 1 cache(s).
class coreunit:
  public sc_module
#ifdef TLM_POWER3
  , public pw_module
#endif
{
 public:
  SC_HAS_PROCESS(coreunit);
  COREISA_IF *core;
#ifdef NEWCCACHE
  ccache* l1_d_cache;
  ccache* l1_i_cache;
#else
  consistent_cache64 *l1_d_cache;
  consistent_cache64 *l1_i_cache;
#endif

  /* Constructor */
#ifdef NEWCCACHE
 coreunit(sc_core::sc_module_name p_name,
	  int i,
	  scu* snoop,
	  bool harvardf);
#else
 coreunit(sc_core::sc_module_name p_name,
	  int i,
	  consistent_group *grp,
	  bool harvardf); // constructor
#endif

 void stat_report(const char *msg, FILE *fd, bool resetf = 0);

 const char *kind(); 
};

class top:
  public sc_module
#ifdef TLM_POWER3
 , public pw_module
#endif
{
 public:
  u64_t hi_entrypt;
  int n_cores;
  coreunit *Cores[MAX_NO_OF_CORES];

  vhls_debug_port *external_debug_port;

#if USE_MAINMEM_MACRO_MODEL  
  sram64_cbg *memory0, *memory1;
#else
  dramsim_sc_wrapper *memory0, *memory1;
#endif

  uart64_cbg uart0;
  busmux64 busmux0;
#ifdef USE_ETHERCRC
  ethercrc_tlm ethercrc_tlm0;
#endif
  
#ifdef L2_CACHE_SIZE
#ifdef NEWCCACHE
  ccache* l2_cache;
#else
  consistent_cache64* l2_cache;
#endif
#endif

  void reset(const char* image_lo, const char *image_hi);

  // The following defines set up the memory map
#define sram_size   (0x2000000 * 8)
#define dram_size   (0x2000000 * 8)
#define sram_start_adr 0
#define dram_start_adr sram_size
#define iospace_base 0xe0000000LLU
#define iospace_size 0x1000

  // Default constructor
 top(const sc_module_name& p_name,
     int n_cores,
     bool harvardf,
     const char* device_ini,
     const char* system_ini);
 
 void hup_handler(int p_arg);

 const char *kind();
    
 ~top(void) {
   // DOES NOTHING
 }
 
 void stat_report(const char *msg, FILE *fd, bool resetf = 0);

 sc_module* get_Core(int i);
};

// EOF
