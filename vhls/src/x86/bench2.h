/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

// Testbench that resembles DJG's AMD Phenom

#include <systemc>
#ifdef TLM_POWER3
#include <tlm_power>
#define POWER3(X) X
using namespace sc_pwr;
#else
typedef tlm::tlm_base_protocol_types PW_TLM_TYPES;
typedef tlm::tlm_generic_payload PW_TLM_PAYTYPE;
#define POWER3(X)
#endif

#include "../memories/hammer_cache64.h"
#include "../memories/memories_extensions.h"
#include "../dramsim2/dramsim_sc_wrapper.h"
#include "../bus/crossbar.h"
#include "../bus/sri.h"
#include "../bus/hypertransport.h"
#include "../bus/opteron_messages.h"

#include "COREISA_IF.h"

#include "../io/uart64_cbg.h"

#include "x86core.h"

#define L1_CACHE_SIZE (1<<13)
#define L1_CACHE_LINE_SIZE 8
#define MAX_NO_OF_CORES 6

#define L2_CACHE_SIZE (1<<16)
#define L2_CACHE_LINE_SIZE 8

#define DRAM_SIZE (0x2000000 * 8)

#define NB_CLOCK_PERIOD 0.625
#define CORE_CLOCK_PERIOD 1

#define SRC_CLK_CYCLES 3
#define DEST_CLK_CYCLES 5

#define CORE_GROUPS 2
#define CORES_PER_GROUP 3

#define HT_LINKS 1
#define HT_WIDTH 16
#define HT_CLOCK_PERIOD 1

//#define LEAVEOUT_L1_CACHE 1
//#define LEAVEOUT_L2_CACHE 1


#define BENCH_CORES (CORE_GROUPS * CORES_PER_GROUP)

using namespace sc_core;

// A single core + level 1 cache
class coreunit 
  : public sc_module
#ifdef TLM_POWER3
  , public pw_module
#endif
{
 public:
  COREISA_IF *core;
#ifndef LEAVEOUT_L1_CACHE
  hammer_cache64 *l1_d_cache;
  hammer_cache64 *l1_i_cache;
#endif
#ifndef LEAVEOUT_L2_CACHE
  hammer_cache64 *l2_cache;
#endif

  // Constructor
  coreunit(sc_core::sc_module_name p_name,
	   int i,
	   consistent_group* grp,
	   opteron_msg_mm* mm)
    : sc_module(p_name)
#ifndef LEAVEOUT_L1_CACHE
    , l1_i_cache(0) 
#endif
    {
      bool harvardf = true; // 
      char txt[132];
      snprintf(txt, 132, "x86_core_%d", i);
      printf("CREATING CORE %s\n", txt);

      // Create core
      core = new x86core(txt, i, 0);
      if(!core)
	SC_REPORT_FATAL("BENCH2", "Could not create x86 core");
      
#ifndef LEAVEOUT_L1_CACHE
    // Create L1 data cache
    snprintf(txt, 132, "l1_d_cache_%d", i);
    l1_d_cache = new hammer_cache64(txt,
				    grp,
				    L1_CACHE_SIZE,
				    4,
				    L1_CACHE_LINE_SIZE,
                                    64,
				    sc_time(5, SC_NS),
				    1,
				    CT_DATA,
#ifndef LEAVEOUT_L2_CACHE
				    true
#else
				    false
#endif
);
    l1_d_cache->m_moesi_mm = mm;

    // connect L1 data cache to core
    core->busaccess.initiator_socket.bind(l1_d_cache->targ_socket);
    if(harvardf) {
      snprintf(txt, 132, "l1_i_cache_%d", i);
      l1_i_cache = new hammer_cache64(txt,
				      NULL,
				      L1_CACHE_SIZE,
				      4,
				      L1_CACHE_LINE_SIZE,
				      64,
				      sc_time(5, SC_NS),
				      1,
				      CT_INSTRUCTION,
#ifndef LEAVEOUT_L2_CACHE
				      true
#else
				      false
#endif
);
      l1_i_cache->m_moesi_mm = mm;
      core->busaccess.ifetch_socket->bind(l1_i_cache->targ_socket);
    }    
#endif

#ifndef LEAVEOUT_L2_CACHE
    // Create L2 cache
    snprintf(txt, 132, "l2_cache_%d", i);
    printf("Creating coreunit %s\n", txt);
    l2_cache = new hammer_cache64(txt,
                                  NULL,
                                  L2_CACHE_SIZE,
                                  4,
                                  L2_CACHE_LINE_SIZE,
                                  64,
                                  sc_time(5, SC_NS),
                                  1,
                                  CT_UNIFORM,
#ifndef LEAVEOUT_L1_CACHE
				  true
#else
				  false
#endif
);
    l2_cache->m_moesi_mm = mm;
#endif

#ifndef LEAVEOUT_L2_CACHE
#ifndef LEAVEOUT_L1_CACHE
    l2_cache->set_lower_level_data_cache(l1_d_cache);
#else
    l2_cache->set_lower_level_data_cache(0);
#endif
#endif

#if !defined LEAVEOUT_L1_CACHE && !defined LEAVEOUT_L2_CACHE
    l1_d_cache->inita_socket.bind(l2_cache->targ_socket);
    if(harvardf) 
      l1_i_cache->inita_socket.bind(l2_cache->targ_socket);
    // this link is used to invalidate lines in L1 cache
    if(l2_cache->moesi_init && l1_d_cache->moesi_targ)
      l2_cache->moesi_init->bind(*l1_d_cache->moesi_targ);
    if(l1_d_cache->moesi_init && l2_cache->moesi_targ)
       l1_d_cache->moesi_init->bind(*l2_cache->moesi_targ);
    // L1 instruction cache is read only
    if(harvardf && l1_i_cache->moesi_init && l1_i_cache->moesi_targ)
       l1_i_cache->moesi_init->bind(*l1_i_cache->moesi_targ);
#elif !defined LEAVEOUT_L2_CACHE
    core->busaccess.initiator_socket.bind(l2_cache->targ_socket);
    if(harvardf)
      core->busaccess.ifetch_socket->bind(l2_cache->targ_socket);
#endif
  }

  // Statistics
  void stat_report(const char *msg, FILE *fd, bool resetf = 0) {
    core->stat_report(msg, fd, resetf);
#ifndef LEAVEOUT_L1_CACHE
    l1_d_cache->stat_report(msg, fd, resetf);
    if(l1_i_cache) 
      l1_i_cache->stat_report(msg, fd, resetf);
#endif
#ifndef LEAVEOUT_L2_CACHE
    l2_cache->stat_report(msg, fd, resetf);
#endif
  }

  const char *kind() {
    return "coreunit";
  }
};   

class top : 
  public sc_module
#ifdef TLM_POWER3
  , public pw_module
#endif
{
 private:
  sri** sris;
  crossbar** crossbars;
  hypertransport** hypertransports;

  sc_mutex* addr_table_lock;

  opteron_msg_mm* mm;

  uart64_cbg uart0;

 public:
  coreunit *Cores[MAX_NO_OF_CORES];

  dramsim_sc_wrapper *memory0, *memory1;

  void reset(const char* image_lo, const char* image_hi) {
      if(image_lo) 
          memory0->loadme(image_lo, true);

      for(int i = 0; i < BENCH_CORES; i++) {
	Cores[i]->core->reset(i<g_enabled_cores);
	u64_t last_addr = Cores[i]->core->install_image(image_lo);
	if(i == 0) {
	  for(int j = 0; j < last_addr; j+= L2_CACHE_LINE_SIZE) 
	    u64_t real_addr = sris[0]->get_real_address(j);
	}
      }
  }


#define iospace_base 0xFFFFffffe0000000LLU

  // Default constructor
  top(const sc_module_name& p_name, 
      const char* device_ini,
      const char* system_ini) 
    : sc_module(p_name)
#ifdef TLM_POWER3
    , pw_module()
#endif
      , uart0("uart0", 1, 1) 
  {
    
    // number of corse is fixed to CORES_GROUP * CORES_PER_GROUP
    // thus we ignore second argument that is passed


    //    if (BENCH_CORES_ != BENCH_CORES * CORES_PER_GROUP)       printf("Disregarding command line number of cores\n");

    u32_t trace_flags = 0;
    if (g_tracelevel > 0) {
      trace_flags |=
           TENOS_TRACE_CPU_INTERRUPTS
	| ((g_tracelevel > 1) ? TENOS_TRACE_IO_READ | TENOS_TRACE_IO_WRITE: 0)
	| ((g_tracelevel > 2) ? TENOS_TRACE_CPU_IFETCH : 0)
	| ((g_tracelevel > 3) ? TENOS_TRACE_MEM_READ|TENOS_TRACE_MEM_WRITE|TENOS_TRACE_WRITE_BUFFER_OP: 0)
	| ((g_tracelevel > 4) ? TENOS_TRACE_CPU_IDECODE : 0)
	
	| ((g_tracelevel > 5) ? TENOS_TRACE_CPU_KEY_REGS: 0)
	| ((g_tracelevel > 6) ? TENOS_TRACE_CPU_READ|TENOS_TRACE_CPU_WRITE : 0)
        | ((g_tracelevel > 7) ? TENOS_TRACE_CPU_ALL_REGS: 0)
	| ((g_tracelevel > 8) ? TENOS_TRACE_BUS_OP: 0)
	;
      //printf("trace trace_flags 0x%X\n", trace_flags);
    
    }

    traceregion *itrace = 0;
    if (trace_flags) 
      for (int k=0; k==0 || k < g_watches; k++) {
	u64_t lo = k>=g_watches ? 0: Watches[k].lo;
	u64_t hi = k>=g_watches ? (u64_t)-1: Watches[k].hi;
	(new traceregion(lo, hi, INT_MAX, trace_flags))->add(itrace);
      }


    // Create main memory
    dramsim_ini_t *ini = new dramsim_ini_t;
    ini->set_as_an_example(1024);
    ini->deviceIniFilename = device_ini;
    ini->systemIniFilename = system_ini;

    // DRAM controller has same clock as northbridge
    //for(int i = 0; i < CORE_GROUPS; i++) {
    assert(CORE_GROUPS == 2);
    char name[132];
    snprintf(name, 132, "memory_0");
    memory0 = new dramsim_sc_wrapper(name, 0, ini);
    snprintf(name, 132, "memory_1");
    memory1 = new dramsim_sc_wrapper(name, DRAM_SIZE, ini);

    map<uint32_t, map<uint32_t, uint32_t> > fixed_route_table;
    // Node 0
    fixed_route_table[0][1] = 0; // for node 0 to send msg to node 1 use port 0
    fixed_route_table[1][0] = 0; // for node 1 to send msg to node 0 use port 1

    // memory manager for opteron-like messages
    opteron_msg_mm* mm = new opteron_msg_mm;

    // address table lock
    char addr_lock_name[132];
    snprintf(addr_lock_name, 132, "address_table_lock");
    addr_table_lock = new sc_mutex(addr_lock_name);

    crossbars = new crossbar*[CORE_GROUPS];
    sris = new sri*[CORE_GROUPS];
    // each node has only one HT link
    hypertransports = new hypertransport*[CORE_GROUPS];

    // Creating SRI, crossbars and HTs
    for(int i = 0; i < CORE_GROUPS; i++) {
      char names[132];

      // Building system request interface
      snprintf(names, 132, "sr_interface_%i", i);
      sris[i] = new sri(names, i == 0 ? NULL : sris[i-1],
			i,
			L2_CACHE_LINE_SIZE,
			addr_table_lock,
			mm,
			sc_time(NB_CLOCK_PERIOD, SC_NS),
			3, /* number of cycles required to access mem table*/
			sc_time(CORE_CLOCK_PERIOD, SC_NS),
			SRC_CLK_CYCLES,
			DEST_CLK_CYCLES,
			CORE_GROUPS,
			iospace_base,
                        i * DRAM_SIZE);

      assert(sris[i] != NULL);

      // Building crossbar
      snprintf(names, 132, "crossbar_%i", i);
      crossbars[i] = new crossbar(names,
				  CORE_GROUPS,
				  HT_LINKS,
				  i,
				  fixed_route_table[i],
				  addr_table_lock,
				  i == 0 ? NULL : crossbars[i-1],
				  mm,
				  sc_time(NB_CLOCK_PERIOD, SC_NS),
				  SRC_CLK_CYCLES,
				  DEST_CLK_CYCLES);
      assert(crossbars[i] != NULL);

      // So this manual coupling of the address tables is not a seamless wiring of TLM sockets...
      crossbars[i]->init_address_table(sris[i]->get_address_table_ptr(),
				       sris[i]->get_address_map_ptr());

      // Building hyper transport link
      snprintf(names, 132, "ht_link_%i_0", i);
      hypertransports[i] = new hypertransport(names,
					      i,
					      0,
					      sc_time(HT_CLOCK_PERIOD, SC_NS),
					      HT_WIDTH,
					      SRC_CLK_CYCLES,
					      DEST_CLK_CYCLES);

      assert(hypertransports[i] != NULL);
    }
    
    // we create CORE_GROUPS(two) two consistency groups
    // one is for one group of three cores
    // the other one is for another group
    consistent_group* grp[CORE_GROUPS];
    for(int i = 0; i < CORE_GROUPS; i++) 
      grp[i] = new consistent_group(CORES_PER_GROUP); 

    // create core units
    for(int i = 0; i < CORE_GROUPS * CORES_PER_GROUP; i++) {
      char txt[132];
      snprintf(txt, 132, "coreunit_%d", i);

      int group = floor(i / CORES_PER_GROUP);
      
      // create core unit

      /////////////////////////// all in one grp
      // Cores[i] = new coreunit(txt, i, grp[0 * group], harvardf, mm);

      // Cores[i] = new coreunit(txt, i, grp[1 * group], harvardf, mm);

      bool harvardf = true; // 

      Cores[i] = new coreunit(txt, i, grp[group], mm);
#if !defined LEAVEOUT_L1_CACHE && defined LEAVEOUT_L2_CACHE
      Cores[i]->l1_d_cache->inita_socket.bind(sris[group]->targ_socket);
      if(harvardf)
	Cores[i]->l1_i_cache->inita_socket.bind(sris[group]->targ_socket);
      sris[group]->init_socket.bind(Cores[i]->l1_d_cache->targ_socket);
      if(harvardf)
	Cores[i]->l1_i_cache->inita_socket.bind(sris[group]->targ_socket);
#elif !defined LEAVEOUT_L2_CACHE
      // connect cache(s) to SRI
      Cores[i]->l2_cache->inita_socket.bind(sris[group]->targ_socket);
      // connect SRI to cache(s)
      sris[group]->init_socket.bind(Cores[i]->l2_cache->targ_socket);
#else
      Cores[i]->core->busaccess.initiator_socket.bind(sris[group]->targ_socket);
      if(harvardf)
	Cores[i]->core->busaccess.ifetch_socket->bind(sris[group]->targ_socket);

#endif
    }

    for(int i = 0; i < CORE_GROUPS; i++) {
      // connect SRI to a crossbar
      sris[i]->init_socket.bind(crossbars[i]->targ_socket);
      sris[i]->local_iodev_socket.bind(uart0.port0);

      // connect crossbar to SRI
      crossbars[i]->init_socket.bind(sris[i]->targ_socket);
      // connect crossbar to hypertransport link
      crossbars[i]->init_socket.bind(hypertransports[i]->targ_socket);
      // connect hypertransport link to crossbar
      hypertransports[i]->init_socket.bind(crossbars[i]->targ_socket);
    }

    // crossbars to DRAM
    assert(CORE_GROUPS == 2);
    crossbars[0]->init_socket.bind(memory0->port0);
    crossbars[1]->init_socket.bind(memory1->port0);

    hypertransports[0]->init_socket.bind(hypertransports[1]->targ_socket);
    hypertransports[1]->init_socket.bind(hypertransports[0]->targ_socket);

    extern const char *uart_canned_input;
    if (uart_canned_input) uart0.register_canned_input(uart_canned_input);
    //Cores[0]->core->connect_interrupt(&uart0.interrupt_request);
    
    char line[444];
    snprintf (line, 444, "%s.%s.txt", g_name, uart0.name());
    uart0.open_log_file(line);
    
    if (trace_flags && itrace) {
      for (int i=0;i<BENCH_CORES;i++) {
	if (g_coremask < 0 || g_coremask == i) itrace->add(Cores[i]->core->busaccess.traceregions);
      }
      //      itrace->add(busmux0.traceregions);
      itrace->add(uart0.traceregions);
      itrace->add(memory0->traceregions);
      itrace->add(memory1->traceregions);
#ifdef USE_ETHERCRC
      itrace->add(ethercrc_tlm0.traceregions);
      itrace->add(busmux1.traceregions);
#endif
    }    
  }

  void stat_report(const char *msg, FILE *fd, bool resetf=0)
  {
    
    if (fd) fprintf(fd, "%s %s  \n", name(), msg);
    for(int i=0;i<BENCH_CORES;i++) Cores[i]->stat_report(msg, fd, resetf);

    uart0.stat_report(msg, fd, resetf);
    memory0->stat_report(msg, fd, resetf);
    memory1->stat_report(msg, fd, resetf);
    //if (resetf) stats.reset();


    // TODO (MP)
    // Add stat reports for opteron memory architecture
  }

  void hup_handler(int p_arg) // Get a little trace from a busy simulation by sending it a HUP.
  {
    for(int i=0;i<BENCH_CORES;i++) Cores[i]->core->busaccess.trace100("hup handler");
  }

};
