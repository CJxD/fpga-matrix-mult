/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

#include "phenom.h"

using namespace sc_core;

coreunit::coreunit(
  sc_module_name p_name,
  int i,
  scu* snoop,
  bool harvardf)
  : sc_module(p_name)
#ifndef LEAVEOUT_L1_CACHE
  , l1_i_cache(0) 
#endif
{
  char txt[132];
  snprintf(txt, 132, "x86_core_%d", i);
  printf("CREATING CORE %s\n", txt);

  // Create core
  core = new x86core(txt, i, 0);
  if(!core)
    SC_REPORT_FATAL("PHENOM", "Could not create x86 core");
      
#ifndef LEAVEOUT_L1_CACHE
  // Create L1 data cache
  snprintf(txt, 132, "l1_d_cache_%d", i);
  l1_d_cache = new ccache(txt,
			  i, 
			  ADDR_IN_BITS,
			  L1_CACHE_SIZE,
			  L1_CACHE_LINE_SIZE,
			  L1_CACHE_WAYS, // number of ways
			  snoop != NULL ? true : false);

  // connect L1 data cache to core
  core->busaccess.initiator_socket.bind(l1_d_cache->target_socket);
  if(snoop) {
    snoop->snoop_socket->bind(*(l1_d_cache->snooped_socket));
    l1_d_cache->initiator_socket.bind(snoop->target_socket);
  }
  
  if(harvardf) {
    snprintf(txt, 132, "l1_i_cache_%d", i);
    l1_i_cache = new ccache(txt,
			    i, 
			    ADDR_IN_BITS,
			    L1_CACHE_SIZE,
			    L1_CACHE_LINE_SIZE,
			    L1_CACHE_WAYS,
			    false);
    core->busaccess.ifetch_socket->bind(l1_i_cache->target_socket);
  }    
#endif  
}

void coreunit::stat_report(
  const char *msg,
  FILE *fd,
  bool resetf) {
  core->stat_report(msg, fd, resetf);
#ifndef LEAVEOUT_L1_CACHE
  l1_d_cache->stat_report(msg, fd, resetf);
  if(l1_i_cache) 
    l1_i_cache->stat_report(msg, fd, resetf);
#endif
}

const char *coreunit::kind() {
  return "coreunit";
}

void top::reset(const char* image_lo, const char* image_hi) {
  if(image_lo) 
    memory0->loadme(image_lo, true);
  
  for(int i = 0; i < g_cores; i++) {
    Cores[i]->core->reset(i<g_cores);
    Cores[i]->core->install_image(image_lo);
  }
}

top::top(
  const sc_module_name& p_name, 
  const char* device_ini,
  const char* system_ini) 
  : sc_module(p_name)
#ifdef TLM_POWER3
  , pw_module()
#endif
  , uart0("uart0", 1, 1)
  , snoop(0) {

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
#if 0  
  if (trace_flags) 
    for (int k=0; k==0 || k < g_watches; k++) {
      u64_t lo = k>=g_watches ? 0: Watches[k].lo;
      u64_t hi = k>=g_watches ? (u64_t)-1: Watches[k].hi;
      (new traceregion(lo, hi, INT_MAX, trace_flags))->add(itrace);
    }
#endif
  
    // Create main memory
    dramsim_ini_t *ini = new dramsim_ini_t;
    ini->set_as_an_example(1024);
    ini->deviceIniFilename = device_ini;
    ini->systemIniFilename = system_ini;

    char name[132];
    snprintf(name, 132, "memory_0");
    memory0 = new dramsim_sc_wrapper(name, 0, ini);

    if(g_cores > 1)
      snoop = new scu("snoop_control_unit");

    bool harvardf = true; 
    // create core units
    
    Cores = (coreunit**) malloc(sizeof(coreunit*) * g_cores);
    for(int i = 0; i < g_cores; i++) {
      char txt[132];
      snprintf(txt, 132, "coreunit_%d", i);
      
      Cores[i] = new coreunit(txt, i, snoop, harvardf);
    }
    
    snprintf(name, 132, "l2_cache");
    l2_cache = new ccache(name,
			  0,
			  ADDR_IN_BITS,
			  L2_CACHE_SIZE,
			  L2_CACHE_LINE_SIZE,
			  L2_CACHE_WAYS,
			  false);
    
    if(snoop) {
      snoop->initiator_socket->bind(l2_cache->target_socket);
    } else {
      assert(g_cores == 1);
      Cores[0]->l1_d_cache->initiator_socket.bind(l2_cache->target_socket);
    }

    l2_cache->initiator_socket.bind(memory0->port0);
    
    extern const char *uart_canned_input;
    if (uart_canned_input) uart0.register_canned_input(uart_canned_input);
    //Cores[0]->core->connect_interrupt(&uart0.interrupt_request);
    
    char line[444];
    snprintf (line, 444, "%s.%s.txt", g_name, uart0.name());
    uart0.open_log_file(line);
    
    if (trace_flags && itrace) {
      for (int i=0;i<g_cores;i++) {
	if (g_coremask < 0 || g_coremask == i)
	  itrace->add(Cores[i]->core->busaccess.traceregions);
      }
      
      // itrace->add(busmux0.traceregions);
      itrace->add(uart0.traceregions);
      itrace->add(memory0->traceregions);
#ifdef USE_ETHERCRC
      itrace->add(ethercrc_tlm0.traceregions);
      itrace->add(busmux1.traceregions);
#endif
    }    
}

void top::stat_report(const char *msg, FILE *fd, bool resetf) {
    if (fd)
      fprintf(fd, "%s %s  \n", name(), msg);
    for(int i = 0; i < g_cores; i++)
      Cores[i]->stat_report(msg, fd, resetf);

    uart0.stat_report(msg, fd, resetf);
    memory0->stat_report(msg, fd, resetf);
    //if (resetf) stats.reset();
}

// Get a little trace from a busy simulation by sending it a HUP.
void top::hup_handler(int p_arg) {
  for(int i = 0; i < g_cores; i++)
    Cores[i]->core->busaccess.trace100("hup handler");
}

sc_module* top::get_Core(int i) {
  return Cores[i];
}
