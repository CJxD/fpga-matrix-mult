/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

#include "or1k.h"

using namespace sc_core;

#ifdef NEWCCACHE
coreunit::coreunit(sc_module_name p_name,
		   int i,
		   scu* snoop,
		   bool harvardf) :
#else
coreunit::coreunit(sc_module_name p_name,
		   int i,
		   consistent_group *grp,
		   bool harvardf) : // constructor
#endif
  sc_module(p_name) {
  core = 0;
  l1_d_cache = 0;
  l1_i_cache = 0;
  char txt[132];
  snprintf(txt, 132, "core_%d", i);   
  extern bool use_verilated;
  printf("or1k.h: Creating OR1200 core %s harvard=%i verilated=%i\n",
	 txt, harvardf, use_verilated);
#ifdef USE_VERILATED
  if (use_verilated) core = new OR1200V(txt, i, harvardf);
#endif
  
#ifdef USE_FASTISS
  if (!use_verilated) core = new OR1200F(txt, i, harvardf); 
#endif
  
  if (!core) {
    SC_REPORT_FATAL("OR1K", "No processor type installed");
  }
#ifdef L1_CACHE_SIZE
  if (!g_no_caches) {
    snprintf(txt, 132, "l1_d_cache_%d", i);   
#ifdef NEWCCACHE
    l1_d_cache =
      new ccache(txt,
		 i, 
		 64, // number of bits in address
		 L1_CACHE_SIZE * g_dcache_l1_ways / 8, // cache size
		 32, // line size in bytes
		 g_dcache_l1_ways, // number of ways
		 snoop != NULL ? true : false); // cache is snooped
    core->busaccess.initiator_socket.bind(l1_d_cache->target_socket);
    if(snoop) {
      assert(snoop->snoop_socket);
      snoop->snoop_socket->bind(*(l1_d_cache->snooped_socket));
      l1_d_cache->initiator_socket.bind(snoop->target_socket);
    }
#else
    l1_d_cache =
      new consistent_cache64(txt,
			     grp,
			     L1_CACHE_SIZE * g_dcache_l1_ways / 8,
			     g_dcache_l1_ways,
			     8,
			     64,
			     1,
			     CT_DATA,
			     g_cache_protocol);
    core->busaccess.initiator_socket.bind(l1_d_cache->targ_socket);
#endif
    if (harvardf) {
      snprintf(txt, 132, "l1_i_cache_%d", i);   
      // I-cache is not consistent - pass grp=0 to it.
      // Indeed should not receive front-side writes!
#ifdef NEWCCACHE	      
      l1_i_cache =
	new ccache(txt,
		   i, 
		   64, // number of bits in address
		   L1_CACHE_SIZE * g_icache_l1_ways / 8, // cache size
		   32, // line size in bytes
		   g_icache_l1_ways, // number of ways
		   false); // cache is not snooped
      core->busaccess.ifetch_socket->bind(l1_i_cache->target_socket);
#else	 
      l1_i_cache =
	new consistent_cache64(txt,
			       0,
			       L1_CACHE_SIZE * g_icache_l1_ways / 8,
			       g_icache_l1_ways,
			       8,
			       64,
			       1,
			       CT_INSTRUCTION,
			       g_cache_protocol);
      core->busaccess.ifetch_socket->bind(l1_i_cache->targ_socket);
#endif
    }
  }
#endif
  POWER3(pw_trace(g_txt_pt, *core));
  POWER3(pw_trace(g_txt_pt, *this, "topbench"));
  POWER3(pw_trace(g_txt_pt, core->busaccess, "busaccess"));
#if 0
  {
    std::cout << "casting 1 ok OR1200F cpu_busaccess this = "
	      << ((void *) & (core->busaccess))
	      << " pw_module_base = "
	      << ((void *)(dynamic_cast<pw_module_base *>(&(core->busaccess))))
	      << "\n";
  }
  
  // The compiler cheats here does it? It makes a successful downcast
  // owing to having the class definitions to hand and the conversion
  // to p_pos being in the same file or basic block?
  {
    sc_object* p_pos = (&(core->busaccess));
    pw_module_base* l_module = dynamic_cast< pw_module_base *>(p_pos);
    std::cout
      << "dox in bench1: attach_to dynamic_cast shows non ref transp "
      << ((void *)p_pos)
      << "  "
      << ((void *)l_module)
      << "\n";
  }
#endif
}

void coreunit::stat_report(const char *msg, FILE *fd, bool resetf) {
  core->stat_report(msg, fd, resetf);
  if (l1_d_cache)
    l1_d_cache->stat_report(msg, fd, resetf);
  if (l1_i_cache)
    l1_i_cache->stat_report(msg, fd, resetf);
}

const char* coreunit::kind() {
  return "coreunit";
}

void top::reset(const char* image_lo, const char *image_hi) {
  hi_entrypt = 0;
  if(image_lo)
    memory0->loadme(image_lo, true);

  for(int i = 0; i < n_cores; i++) {
    if (hi_entrypt && i == 0)
      Cores[i]->core->register_reset_vector(hi_entrypt);
    bool core_self_start = 
      (0) ?  (i==0) : // only let core 0 run
      1; // Let all cores start. 
    Cores[i]->core->reset(core_self_start && !g_wait_debugger);
  }
}

top::top(const sc_module_name& p_name,
	 int n_cores,
	 bool harvardf,
	 const char* device_ini,
	 const char* system_ini):
  sc_module(p_name),
  n_cores(n_cores),
#ifdef TLM_POWER3
  pw_module(), 
#endif
  //memory1("dram", dram_start_adr, 0),
  uart0("uart0", g_uart_use_x11, 1),
  busmux0("busmux0")
#ifdef USE_ETHERCRC
  , ethercrc_tlm0("ethercrc_tlm0")
#endif
{
  dramsim_ini_t *ini = new dramsim_ini_t;
  ini->set_as_an_example(1024);
  //"../dramsim2/dist/system.ini.example";
  ini->deviceIniFilename = device_ini;
  //"../dramsim2/dist/ini/DDR2_micron_16M_8b_x8_sg3E.ini";
  ini->systemIniFilename = system_ini; 

#if USE_MAINMEM_MACRO_MODEL
  memory0 =
    new sram64_cbg("memory0",
		   sram_start_adr,
		   sram_size,
		   64);
  memory1 =
    new sram64_cbg("memory0",
		   sram_start_adr + sram_size,
		   sram_size,
		   64);
#else
  memory0 = new dramsim_sc_wrapper("memory0", 0, ini);
  memory1 = new dramsim_sc_wrapper("memory1", 0 + dram_size, ini);
#endif

  if (g_wait_debugger) {
    external_debug_port = new vhls_debug_port("topsoc");
    printf("Created GDB/RSP debug port\n");
  }
  else
    external_debug_port = 0;

#ifdef NEWCCACHE
  scu* snoop = 0;
#else
  consistent_group *grp = 0;
#endif
#ifdef L1_CACHE_SIZE
  // Do no unset this from null for inconsistent caches.
  if (!g_no_caches) {
#ifdef NEWCCACHE
    if(n_cores > 1 && !g_no_caches)
      snoop = new scu("snoop_control_unit");
#else    
    grp = new consistent_group(n_cores);
#endif
  }
#endif
  
  BUSMUX64_BIND(busmux0, memory0->port0, sram_start_adr, sram_size);
  BUSMUX64_BIND(busmux0, memory1->port0, dram_start_adr, dram_size);
  BUSMUX64_BIND(busmux0, uart0.port0, iospace_base, iospace_size);
  
#ifdef USE_ETHERCRC
  BUSMUX64_BIND(busmux0,
		ethercrc_tlm0.port0,
		iospace_base + 0x2000,
		iospace_size);
#endif

  u32_t flags = 0;
  if (g_tracelevel > 0) {
    flags |=
      TENOS_TRACE_CPU_INTERRUPTS
      | ((g_tracelevel > 1) ? TENOS_TRACE_IO_READ | TENOS_TRACE_IO_WRITE: 0)
      | ((g_tracelevel > 2) ? TENOS_TRACE_CPU_IFETCH : 0)
      | ((g_tracelevel > 3) ? TENOS_TRACE_CPU_READ|TENOS_TRACE_CPU_WRITE: 0)
      | ((g_tracelevel > 4) ? TENOS_TRACE_CPU_KEY_REGS: 0)
      | ((g_tracelevel > 5) ?  TENOS_TRACE_MEM_READ|TENOS_TRACE_MEM_WRITE|TENOS_TRACE_WRITE_BUFFER_OP: 0)
      | ((g_tracelevel > 6) ? TENOS_TRACE_CPU_ALL_REGS: 0)
      | ((g_tracelevel > 7) ? TENOS_TRACE_BUS_OP: 0) ;
      //printf("trace flags 0x%X\n", flags);
  }
      

#ifdef L2_CACHE_SIZE
  if(!g_no_caches) {
    char txt[132];
    snprintf(txt, 132, "l2_cache");
#ifdef NEWCCACHE
    l2_cache = new ccache(txt,
			  0, 
			  64, // number of bits in address
			  L2_CACHE_SIZE * g_cache_l2_ways / 8, // size
			  32, // line size in bytes
			  g_cache_l2_ways, // number of ways
			  false); // cache is not snooped
#else
    l2_cache = new consistent_cache64(txt,
				      0,
				      L2_CACHE_SIZE * g_cache_l2_ways / 8,
				      g_cache_l2_ways,
				      8,
				      64,
				      1,
				      CT_DATA,
				      g_cache_protocol);
#endif
  }
#endif

  for(int i = 0; i < n_cores; i++) {
    char txt[132];
    snprintf(txt, 132, "coreunit_%d", i);

#ifdef NEWCCACHE
    Cores[i] = new coreunit(txt, i, snoop, harvardf);
#else
    Cores[i] = new coreunit(txt, i, grp, harvardf);
#endif
#ifdef L1_CACHE_SIZE
    if(!g_no_caches) {
#ifndef NEWCCACHE	
#ifdef L2_CACHE_SIZE
      Cores[i]->l1_d_cache->inita_socket.bind(l2_cache->targ_socket);
#else
      Cores[i]->l1_d_cache->inita_socket.bind(busmux0.targ_socket);
#endif
#endif
      
#if 0
      if (g_dcache_tracelevel > 0) {
	(new traceregion(0,
			 (u64_t)-1,
			 INT_MAX,
			 TENOS_TRACE_MEM_READ|TENOS_TRACE_MEM_WRITE|TENOS_TRACE_WRITE_BUFFER_OP))->add(Cores[i]->l1_d_cache->traceregions);
      }
#endif
      
      if (external_debug_port)
	external_debug_port->connect_ip_block(txt, Cores[i]->core);
      
      POWER3(pw_trace(g_vcd_pt, *Cores[i]->l1_d_cache, sum_children));
      POWER3(pw_trace(g_txt_pt, *Cores[i]->l1_d_cache, sum_children));
      
      if(harvardf) {
#ifdef L2_CACHE_SIZE
#ifdef NEWCCACHE
	Cores[i]->l1_i_cache->initiator_socket.bind(l2_cache->target_socket);
#else
	Cores[i]->l1_i_cache->inita_socket.bind(l2_cache->targ_socket);
#endif
#else
#ifdef NEWCCACHE
	Cores[i]->l1_i_cache->initiator_socket.bind(busmux0.targ_socket);
#else
	Cores[i]->l1_i_cache->inita_socket.bind(busmux0.targ_socket);
#endif
#endif
	
	POWER3(pw_trace(g_txt_pt, *Cores[i]->l1_i_cache, sum_children));
	POWER3(pw_trace(g_vcd_pt, *Cores[i]->l1_i_cache, sum_children));
      }
    } else {
      Cores[i]->core->busaccess.initiator_socket.bind(busmux0.targ_socket);
      if (harvardf)
	Cores[i]->core->busaccess.ifetch_socket->bind(busmux0.targ_socket);
    }
#else
    Cores[i]->core->busaccess.initiator_socket.bind(busmux0.targ_socket);
    if (harvardf)
      Cores[i]->core->busaccess.ifetch_socket->bind(busmux0.targ_socket);
#endif
  }
  
  
#ifdef NEWCCACHE
  if(snoop) {
#ifdef L2_CACHE_SIZE
    assert(snoop->initiator_socket);
    snoop->initiator_socket->bind(l2_cache->target_socket);
#else
    snoop->initiator_socket.bind(busmux0.targ_socket);
#endif
  } else if(!g_no_caches) {
    assert(n_cores == 1);
#ifdef L2_CACHE_SIZE
    Cores[0]->l1_d_cache->initiator_socket.bind(l2_cache->target_socket);
#else
    Cores[0]->l1_d_cache->initiator_socket.bind(busmux0.targ_socket);
#endif
  }
  
  if (!g_no_caches && harvardf) {
#ifdef L2_CACHE_SIZE
    Cores[0]->l1_i_cache->initiator_socket.bind(l2_cache->target_socket);
#else
    Cores[0]->l1_i_cache->initiator_socket.bind(busmux0.targ_socket);
#endif
  }
#endif
  
#ifdef L2_CACHE_SIZE
  if(!g_no_caches) {
#ifdef NEWCCACHE
    l2_cache->initiator_socket.bind(busmux0.targ_socket);
#else
    l2_cache->inita_socket.bind(busmux0.targ_socket);
#endif
  }
#endif
  
  traceregion *itrace = 0;
#if 0  
  if (flags) for (int k=0; k==0 || k < g_watches; k++) 
	       {
		 u64_t lo = k>=g_watches ? 0: Watches[k].lo;
		 u64_t hi = k>=g_watches ? (u64_t)-1: Watches[k].hi;
		 (new traceregion(lo, hi, INT_MAX, flags))->add(itrace);
	       }

  // Also trace main memory for this dcache region.
  if (g_dcache_tracelevel > 0) {
    int k = 0;
    u64_t lo = k>=g_watches ? 0: Watches[k].lo;
    u64_t hi = k>=g_watches ? (u64_t)-1: Watches[k].hi;
    (new traceregion(lo, hi, INT_MAX, TENOS_TRACE_MEM_READ|TENOS_TRACE_MEM_WRITE|TENOS_TRACE_WRITE_BUFFER_OP))->add(memory0->traceregions);
#ifdef L1_CACHE_SIZE
    for(int i = 0; i < n_cores; i++) {
      (new traceregion(lo, hi, INT_MAX, TENOS_TRACE_MEM_READ|TENOS_TRACE_MEM_WRITE|TENOS_TRACE_WRITE_BUFFER_OP))->add(Cores[i]->l1_d_cache->traceregions);
      //(new traceregion(lo, hi, INT_MAX, TENOS_TRACE_MEM_READ|TENOS_TRACE_MEM_WRITE|TENOS_TRACE_WRITE_BUFFER_OP))->add(Cores[i]->l1_i_cache->traceregions);
    }
#endif
#ifdef NEWCCACHE
    if(snoop)
      (new traceregion(lo,
		       hi,
		       INT_MAX,
		       TENOS_TRACE_SNOOP))->add(snoop->traceregions);
    
#endif
#ifdef L2_CACHE_SIZE
    (new traceregion(lo,
		     hi,
		     INT_MAX,
		     TENOS_TRACE_MEM_READ|TENOS_TRACE_MEM_WRITE|TENOS_TRACE_WRITE_BUFFER_OP))->add(l2_cache->traceregions);
#endif      
    }
#endif

  
#ifdef NEWCCACHE
  if(snoop)
    snoop->init();
#endif
  
  
  if (flags && itrace) {
    for (int i = 0; i < n_cores; i++)
      itrace->add(Cores[i]->core->busaccess.traceregions);
    itrace->add(busmux0.traceregions);
    itrace->add(uart0.traceregions);
    itrace->add(memory0->traceregions);
    itrace->add(memory1->traceregions);
#ifdef USE_ETHERCRC
    itrace->add(ethercrc_tlm0.traceregions);
#endif
  }
  POWER3(pw_trace(g_vcd_pt,
		  *this,
		  "TOPTHIS",
		  sum_children,
		  PlotCombinedAccounts));
  POWER3(pw_trace(g_txt_pt, *memory0, "Memory 0 (DRAM)"));
  POWER3(pw_trace(g_txt_pt, *memory1, "Memory 1 (DRAM)"));
  POWER3(pw_trace(g_txt_pt, uart0));
  //pw_trace(g_txt_pt, (the_benchconfig->busmux0), "BusMux 0");
  //pw_trace(g_txt_pt, (the_benchconfig->uart0),   "Uart 0");


  extern const char *uart_canned_input;
  if (uart_canned_input)
    uart0.register_canned_input(uart_canned_input);
  //  Cores[0]->core->connect_interrupt(&uart0.interrupt_request);

  char line[444];
  snprintf (line, 444, "%s.%s.txt", g_name, uart0.name());
  uart0.open_log_file(line);
}

// Get a little trace from a busy simulation by sending it a HUP.
void top::hup_handler(int p_arg) {
  for(int i = 0; i < n_cores; i++)
    Cores[i]->core->busaccess.trace100("hup handler");
}

const char *top::kind() {
  return "or1k";
}

void top::stat_report(const char *msg, FILE *fd, bool resetf) {
  if (fd)
    fprintf(fd, "%s %s  \n", name(), msg);
  
  for(int i = 0; i < n_cores; i++)
    Cores[i]->stat_report(msg, fd, resetf);
  
  busmux0.stat_report(msg, fd, resetf);
  uart0.stat_report(msg, fd, resetf);
  memory0->stat_report(msg, fd, resetf);
  memory1->stat_report(msg, fd, resetf);
#ifdef L2_CACHE_SIZE
  l2_cache->stat_report(msg, fd, resetf);
#endif
}

sc_module* top::get_Core(int i) {
  return Cores[i];
} 


