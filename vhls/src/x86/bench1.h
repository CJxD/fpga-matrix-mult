/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

//
// An example testbench. Multiple cores with caches.
//
//
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

#include "../bus/busmux64.h"
#include "../memories/sram64_cbg.h"
#include "../memories/dram64_cbg.h"
#include "../memories/consistent_cache64.h"
#include "../dramsim2/dramsim_sc_wrapper.h"
#include "COREISA_IF.h"

#ifdef USE_ETHERCRC
#include "ethercrc/ethercrc_tlm.h"
#endif

#include "../io/uart64_cbg.h"

#include "x86core.h"

using namespace sc_core;

// 64 KBytes or so. (cache size is in 8 byte words?)
#define L1_CACHE_SIZE (1<<13)

#define USE_MAINMEM_MACRO_MODEL  0


// ===================================================================
//! TOP module containing a number of cores plus some memories etc.

#define MAX_NO_OF_CORES 256

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
  consistent_cache64 *l1_d_cache;
  consistent_cache64 *l1_i_cache;

 coreunit(sc_core::sc_module_name p_name, int i, consistent_group *grp, bool harvardf): // constructor
  sc_module(p_name)
    {
      core = 0;
      l1_d_cache = 0;
      l1_i_cache = 0;
      char txt[132];
      snprintf(txt, 132, "x86_core_%d", i);   
      //printf("CREATING CORE %s\n", txt);
      extern bool use_verilated;

      core = new x86core(txt, i, harvardf);

      if (!core)
	{
	  SC_REPORT_FATAL("BENCH1", "No processor type installed");
	}
#ifdef L1_CACHE_SIZE
      snprintf(txt, 132, "l1_d_cache_%d", i);   
      l1_d_cache = new consistent_cache64(txt, grp, L1_CACHE_SIZE, 4, 8, 64, 1, CT_DATA);
      POWER3(pw_trace(g_txt_pt, *l1_d_cache));
      core->busaccess.initiator_socket.bind(l1_d_cache->targ_socket);
      if (harvardf)
	{
	  snprintf(txt, 132, "l1_i_cache_%d", i);   
	  // I-cache is not consistent - pass grp=0 to it: indeed should not receive front-side writes!
	  l1_i_cache = new consistent_cache64(txt, 0, L1_CACHE_SIZE, 4, 8, 64, 1, CT_INSTRUCTION);
	  POWER3(pw_trace(g_txt_pt, *l1_i_cache));
	  core->busaccess.ifetch_socket->bind(l1_i_cache->targ_socket);
	}
#endif
      POWER3(pw_trace(g_txt_pt, *core));

      POWER3(pw_trace(g_txt_pt, *this, "pog"));
      POWER3(pw_trace(g_txt_pt, core->busaccess, "pig"));

#if 0
 DELETE ME
      {
	std::cout << "casting 1 ok COREISA_IF cpu_busaccess this=" << ((void *)&(core->busaccess)) << " pw_module_base=" << ((void *)(dynamic_cast<pw_module_base *>(&(core->busaccess)))) << "\n";
      }

      // The compiler cheats here does it? It makes a successful downcast owing to having the class definitions to hand and the conversion
      // to p_pos being in the same file or basic block?
     
      printf("DOG D\n");
      {
	sc_core::sc_object* p_pos = (&(core->busaccess));
	pw_module_base* l_module = dynamic_cast< pw_module_base *>(p_pos);
	std::cout << "dox in bench1: attach_to dynamic_cast shows non ref transp " << ((void *)p_pos) << "  " << ((void *)l_module) << "\n";
      }
      printf("DOG E\n");
#endif
      
    }


    void stat_report(const char *msg, FILE *fd, bool resetf=0)
    {
      core->stat_report(msg, fd, resetf);
      if (l1_d_cache) l1_d_cache->stat_report(msg, fd, resetf);
      if (l1_i_cache) l1_i_cache->stat_report(msg, fd, resetf);
    }


    const char *kind() { return "coreunit"; }
};


class top:
  public sc_module
#ifdef TLM_POWER3
 ,  public pw_module
#endif
{
 public:
  u64_t hi_entrypt;
  int n_cores;
  coreunit *Cores[MAX_NO_OF_CORES];

#if USE_MAINMEM_MACRO_MODEL  
  sram64_cbg *memory0, *memory1;
#else
  dramsim_sc_wrapper *memory0, *memory1;
#endif

  uart64_cbg uart0;
  busmux64 busmux0;
#ifdef USE_ETHERCRC
  busmux64 busmux1;
  ethercrc_tlm ethercrc_tlm0;
#endif

  void reset(const char* image_lo, const char *image_hi) {
    hi_entrypt = 0;
//    if (image_hi) memory1.loadme(image_hi, true, &hi_entrypt);
    if (image_lo) memory0->loadme(image_lo, true);

    for(int i=0;i<n_cores;i++)  {
      if (hi_entrypt && i==0) 
	Cores[i]->core->register_reset_vector(hi_entrypt);
      Cores[i]->core->reset(1 /*i==0*/);
      Cores[i]->core->install_image(image_lo);
    }
  }



//The following defines set up the memory map
#define sram_size   (0x2000000 * 8)
#define dram_size   (0x2000000 * 8)
#define sram_start_adr 0
#define dram_start_adr sram_size
  //#define iospace_base 0xFFFF00000000LLU
#define iospace_base 0xFFFFffffe0000000LLU

// default constructor
 top(const sc_module_name& p_name, int n_cores, bool harvardf, const char* device_ini, const char* system_ini):
  sc_module(p_name),
    n_cores(n_cores),
#ifdef TLM_POWER3
    pw_module(), 
#endif
    //memory1("dram", dram_start_adr, 0),
    uart0("uart0", 1, 1),
    busmux0("busmux0", dram_start_adr, iospace_base)
#ifdef USE_ETHERCRC
    , busmux1("busmux1", iospace_base + 0x2000),
    ethercrc_tlm0("ethercrc_tlm0")
#endif

{

  dramsim_ini_t *ini = new dramsim_ini_t;
  ini->set_as_an_example(1024);
  ini->deviceIniFilename = device_ini; //"../dramsim2/dist/system.ini.example";
  ini->systemIniFilename = system_ini; //"../dramsim2/dist/ini/DDR2_micron_16M_8b_x8_sg3E.ini";

#if USE_MAINMEM_MACRO_MODEL
  memory0 = new sram64_cbg("memory0", sram_start_adr, sram_size, 64);
  memory1 = new sram64_cbg("memory0", sram_start_adr+sram_size, sram_size, 64);
#else
  memory0 = new dramsim_sc_wrapper("memory0",  0, ini);
  memory1 = new dramsim_sc_wrapper("memory1",  0+dram_size, ini);
#endif

  consistent_group *grp = 0;
#ifdef L1_CACHE_SIZE
  //  Do no unset this from null for inconsistent caches.
  grp = new consistent_group(n_cores);
#endif

  busmux0.init_socket.bind(memory0->port0); // The order of binding here sets their order in memory space.
  busmux0.init_socket.bind(memory1->port0);

#ifdef USE_ETHERCRC
  busmux0.init_socket.bind(busmux1.targ_socket);

  // This second busmux demultiplexes withing the I/O space for the connected peripherals.
  busmux1.init_socket.bind(ethercrc_tlm0.port0);  // E0001000
  busmux1.init_socket.bind(uart0.port0);  // E0009000
  //busmux1.init_socket.bind(uart1.port0);
#else
  busmux0.init_socket.bind(uart0.port0);  // E0009000
#endif

  u32_t trace_flags = 0;
  if (g_tracelevel >0)
    {
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
      

  for(int i=0;i<n_cores;i++)
    {
      char txt[132];
      snprintf(txt, 132, "coreunit_%d", i);

      Cores[i] = new coreunit(txt, i, grp, harvardf);
#ifdef L1_CACHE_SIZE
      Cores[i]->l1_d_cache->inita_socket.bind(busmux0.targ_socket);
      POWER3(pw_trace(g_vcd_pt, *Cores[i]->l1_d_cache, sum_children));
      if (harvardf) 
	{
	  if (g_dcache_tracelevel > 0) 
	    {
	      (new traceregion(0, (u64_t)-1, INT_MAX, TENOS_TRACE_MEM_READ|TENOS_TRACE_MEM_WRITE|TENOS_TRACE_WRITE_BUFFER_OP))->add(Cores[i]->l1_d_cache->traceregions);
	    }
	  Cores[i]->l1_i_cache->inita_socket.bind(busmux0.targ_socket);
	  POWER3(pw_trace(g_vcd_pt, *Cores[i]->l1_i_cache, sum_children));
	}
#else
      Cores[i]->core->busaccess.initiator_socket.bind(busmux0.targ_socket);
      if (harvardf) Cores[i]->core->busaccess.ifetch_socket->bind(busmux0.targ_socket);

#endif

}

  traceregion *itrace = 0;
  if (trace_flags) for (int k=0; k==0 || k < g_watches; k++) 
    {
      u64_t lo = k>=g_watches ? 0: Watches[k].lo;
      u64_t hi = k>=g_watches ? (u64_t)-1: Watches[k].hi;
      (new traceregion(lo, hi, INT_MAX, trace_flags))->add(itrace);
    }

  if (g_dcache_tracelevel > 0)  // Also trace main memory for this dcache region.
    {
      int k = 0;
      u64_t lo = k>=g_watches ? 0: Watches[k].lo;
      u64_t hi = k>=g_watches ? (u64_t)-1: Watches[k].hi;
      (new traceregion(lo, hi, INT_MAX, TENOS_TRACE_MEM_READ|TENOS_TRACE_MEM_WRITE|TENOS_TRACE_WRITE_BUFFER_OP))->add(memory0->traceregions);
    }

  if (trace_flags && itrace)
    {
      for (int i=0;i<n_cores;i++) itrace->add(Cores[i]->core->busaccess.traceregions);
      itrace->add(busmux0.traceregions);
      itrace->add(uart0.traceregions);
      itrace->add(memory0->traceregions);
      itrace->add(memory1->traceregions);
#ifdef USE_ETHERCRC
      itrace->add(ethercrc_tlm0.traceregions);
      itrace->add(busmux1.traceregions);
#endif
    }
  POWER3(pw_trace(g_vcd_pt, *this, "TOPTHIS", sum_children, PlotCombinedAccounts));
  POWER3(pw_trace(g_txt_pt, *memory0, "Memory 0 (DRAM)"));
  POWER3(pw_trace(g_txt_pt, *memory1, "Memory 1 (DRAM)"));
//    pw_trace(g_txt_pt, (the_benchconfig->busmux0), "BusMux 0");
//    pw_trace(g_txt_pt, (the_benchconfig->busmux1), "BusMux 1");
//    pw_trace(g_txt_pt, (the_benchconfig->uart0),   "Uart 0");


  extern const char *uart_canned_input;
  if (uart_canned_input) uart0.register_canned_input(uart_canned_input);
  Cores[0]->core->connect_interrupt(&uart0.interrupt_request);

  char line[444];
  snprintf (line, 444, "%s.%s.txt", g_name, uart0.name());
  uart0.open_log_file(line);

 }

  void hup_handler(int p_arg) // Get a little trace from a busy simulation by sending it a HUP.
  {
    for(int i=0;i<n_cores;i++) Cores[i]->core->busaccess.trace100("hup handler");
  }

  const char *kind() { return "bench1"; }
    
// ---------------------------------------------------------------------------
    ~top(void)
      {
	
      }
    
// ===========================================================================



    void stat_report(const char *msg, FILE *fd, bool resetf=0)
    {
      
      if (fd) fprintf(fd, "%s %s  \n", name(), msg);
      for(int i=0;i<n_cores;i++) Cores[i]->stat_report(msg, fd, resetf);
      busmux0.stat_report(msg, fd, resetf);
#ifdef USE_ETHERCRC
      busmux1.stat_report(msg, fd, resetf);
#endif
      uart0.stat_report(msg, fd, resetf);
      memory0->stat_report(msg, fd, resetf);
      memory1->stat_report(msg, fd, resetf);
      //if (resetf) stats.reset();
    }


};


// eof

