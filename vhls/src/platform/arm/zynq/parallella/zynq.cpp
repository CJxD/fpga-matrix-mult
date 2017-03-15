/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

#include "zynq.h"

using namespace sc_core;

// Constructor
coreunit::coreunit(
  sc_core::sc_module_name p_name, 
  int i, 
  bool harvardf,
  arm_scu* l_scu,
  arm_glbt* l_global_timer,
  bool snooping,
  gic_arm_tlm* l_gic) :
  sc_module(p_name) {

    core = 0;
    l1_d_cache = 0;
    l1_i_cache = 0;
    char txt[132];
    snprintf(txt, 132, "core_%d", i);   
    printf("bench1.h: Creating ARM core %s harvard=%i\n", txt, harvardf);
    core = new arm_cortex_a9(txt, i, harvardf);
    
    alternate_banch_target_buffer *abt = 0;
    
    // Optionally add this experimental feature
    if (0) {
      snprintf(txt, 132, "core_%d_abt", i);
      abt = core->add_abt(txt, /*with snoop socket=*/!g_no_caches);
      printf("abt created %s %i\n", __FILE__, __LINE__);
    }

    // Create coprocessor CP14
    snprintf(txt, 132, "core_%d_CP14", i);
    l_cp14 = new arm_cp14(txt);
    
    // Create coprocessor CP15
    snprintf(txt, 132, "core_%d_CP15", i);
    l_cp15 = new arm_cp15(txt, i);
    
    // Create memory management unit
    // TODO: check if for parallella board MMU has 128 TLB entries
    snprintf(txt, 132, "core_%d_MMU", i);
    l_mmu = new arm_mmu(txt, 128);
    
    // Connect MMU to CP15
    l_mmu->cp15_initiator.bind(l_cp15->target_socket);
    // Start MMU in disable state
    l_mmu->disable();
    
    // Private timer and watchdog
    snprintf(txt, 132, "core_%d_twd_timer", i);      
    l_twd = new arm_twd(txt);
    // Here we are connecting twd to the MMU as the first connection
    l_mmu->memories_initiator.bind(l_twd->twd_target0);
    // Here we are connecting global timer to the MMU as the second connection
    l_mmu->memories_initiator.bind(l_global_timer->glb_target);
    // Here we are connecting interrupt controller to the MMU as the third connection
    l_mmu->memories_initiator.bind(l_gic->cpu_target0);
    
    // Connect CP15 to MMU
    l_cp15->initiator_socket.bind(l_mmu->cp15_target);
    
      // Connects core to CP15
    core->cp15_port.bind(l_cp15->target_socket);
    // Connects core to CP14
    core->cp14_port.bind(l_cp14->cp14_target_socket);
    // Connects core to MMU
    core->busaccess.initiator_socket.bind(l_mmu->cores_target);
      
    if (!core) {
      SC_REPORT_FATAL("BENCH1", "No processor type installed");
    }
#ifdef L1_CACHE_SIZE
    if (!g_no_caches) {
      snprintf(txt, 132, "l1_d_cache_%d", i);   
      l1_d_cache = new arm_ccache(
				  txt,
				  i, 
				  32, // number of bits in address
				  L1_CACHE_SIZE, // cache size
				  32, // line size in bytes -- 8 words * 4 bytes
				  4, // number of ways,
				  snooping, // cache is snooped
				  arm_ccache::Unified, // data cache
				  1); // level 1
      l1_d_cache->set_region(arm_ccache::Inner);
      l1_d_cache->set_shareable(true);
      l1_d_cache->set_PoU(false);
      // disable cache
      // ARM SCTLR control register should be used to enable it
      l1_d_cache->disable();
      
      // Bind CP15 to cache in order to send control messages
      l_cp15->initiator_socket.bind(l1_d_cache->cp15_target_socket);
      // Bind MMU to L1 cache in order to access physical address
      // This is second connection to the MMU
      l_mmu->memories_initiator.bind(l1_d_cache->target_socket);
      if(snooping) {
	assert(l_scu->snoop_socket);
	l_scu->snoop_socket->bind(*(l1_d_cache->snooped_socket));
      }
      // connect data sockets 
      CCACHE_SCU_BIND(l1_d_cache, l_scu, true);
      
      if (abt) {
	  l1_d_cache->create_abt_socket();
	  l1_d_cache->abt_initiator_socket->bind(*(abt->snoop_socket));
      }
      
      // connect control data sockets
      assert(l1_d_cache->cp15_initiator_socket);
      l1_d_cache->cp15_initiator_socket->bind(l_scu->arm_target_socket);
      l_scu->init_arm_control_socket();
      l_scu->arm_initiator_socket->bind(l1_d_cache->cp15_target_socket);
      
      // Separate instruction cache
      if (harvardf) {
	snprintf(txt, 132, "l1_i_cache_%d", i);   
	l1_i_cache = new arm_ccache(
				    txt,
				    i, 
				    32, // number of bits in address
				    L1_CACHE_SIZE, // cache size
				    32, // line size in bytes -- 8 words * 4 bytes
				    4, // number of ways
				    false, // cache is not snooped
				    arm_ccache::Instruction, // instruction cache
				    1); // level 1
	l1_i_cache->set_region(arm_ccache::Inner);
	l1_i_cache->set_shareable(false);
	l1_i_cache->set_PoU(false);
	// disable cache
	// ARM SCTLR control register should be used to enable it
	l1_i_cache->disable();
	
	l_cp15->initiator_socket.bind(l1_i_cache->cp15_target_socket);
	core->busaccess.ifetch_socket->bind(l_mmu->cores_target);
	// This is the third connection to the MMU
	l_mmu->memories_initiator.bind(l1_i_cache->target_socket);
	CCACHE_SCU_BIND(l1_i_cache, l_scu, false);
	assert(l1_i_cache->cp15_initiator_socket);
	l1_i_cache->cp15_initiator_socket->bind(l_scu->arm_target_socket);
	l_scu->init_arm_control_socket();
	l_scu->arm_initiator_socket->bind(l1_d_cache->cp15_target_socket);
      }
    }
    else {
      l_mmu->memories_initiator.bind(l_scu->target_socket);
    }
#else
    l_mmu->memories_initiator.bind(l_scu->target_socket);
#endif
    POWER3_TRC_1(*core);
    POWER3_TRC_2(*this, "topbench");
    POWER3_TRC_2(core->busaccess, "busaccess");
#if 0
    {
      std::cout << "casting 1 ok ARM cpu_busaccess this=" << ((void *)&(core->busaccess)) << " pw_module_base=" << ((void *)(dynamic_cast<pw_module_base *>(&(core->busaccess)))) << "\n";
    }

    // The compiler cheats here does it? It makes a successful downcast owing to
    // having the class definitions to hand and the conversion
    // to p_pos being in the same file or basic block?
     
    {
      sc_core::sc_object* p_pos = (&(core->busaccess));
      pw_module_base* l_module = dynamic_cast< pw_module_base *>(p_pos);
      std::cout << "dox in bench1: attach_to dynamic_cast shows non ref transp " << ((void *)p_pos) << "  " << ((void *)l_module) << "\n";
    }
#endif
}


void coreunit::stat_report(const char *msg, FILE *fd, bool resetf) {
  core->stat_report(msg, fd, resetf);
  if (l1_d_cache) {
    l1_d_cache->stat_report(msg, fd, resetf);
  }
  if (l1_i_cache) l1_i_cache->stat_report(msg, fd, resetf);
  if (l_mmu) l_mmu->stat_report(fd, resetf);
}


const char* coreunit::kind() {
  return "coreunit";
}

void top::reset(
  const char* boot,
  const char* dtb,
  const char* kernel,
  const char* vdd) {

    assert(kernel || boot || dtb); // currently we ensure at least one binary for sanity.
      
    entrypt = 0;
    
    // load kernel 
    if (kernel) memory0->loadme(kernel, true, 0xc0000000, &entrypt);
    // load loboot.S
    if (boot) memory0->loadme(boot, true, (u64_t)0x0);
    // load device tree blob at physical address 0x1000
    if (dtb) memory0->loaddtb(dtb, 0x1000);
    // load virtual disk drive
    if (vdd) SDIO->load(vdd);
    
    for(int i = 0; i < n_cores; i++) {
      if(entrypt && i == 0) Cores[i]->core->register_reset_vector(entrypt);
      // For Zynq, only core 0 starts off running and a write to the SCLR subsequently enables the second one to be reset.
      // For general experiments we want g_self_starting_cores to start off running.
      Cores[i]->core->reset((i < g_self_starting_cores) && !g_wait_debugger);
    }
}

u32_t top::flags_for_level(int tracelevel) {

  u32_t r =
    ((tracelevel > 0) ? TENOS_TRACE_CPU_INTERRUPTS: 0)
    | ((tracelevel > 1) ? TENOS_TRACE_SUBROUTINE_CALL: 0)
    | ((tracelevel > 2) ? TENOS_TRACE_IO_READ | TENOS_TRACE_IO_WRITE: 0)
    | ((tracelevel > 3) ? TENOS_TRACE_CPU_IFETCH : 0)
    | ((tracelevel > 4) ? TENOS_TRACE_CPU_READ|TENOS_TRACE_CPU_WRITE: 0)
    | ((tracelevel > 5) ? TENOS_TRACE_CPU_KEY_REGS: 0)
    | ((tracelevel > 6) ? TENOS_TRACE_MEM_READ|TENOS_TRACE_MEM_WRITE|TENOS_TRACE_WRITE_BUFFER_OP: 0)
    | ((tracelevel > 7) ? TENOS_TRACE_CPU_ALL_REGS: 0)
    | ((tracelevel > 8) ? TENOS_TRACE_BUS_OP: 0)
    ;
  //printf("Debug flag vector for tracelevel %i is 0x%x\n", tracelevel, r);
  return r;
}

top::top(
  const sc_module_name& p_name,
  int n_cores,
  double core_frequency,
  bool harvardf,
  const char* device_ini,
  const char* system_ini):
  sc_module(p_name),
  n_cores(n_cores),
#ifdef TLM_POWER3
  pw_module(), 
#endif
  busmux0("busmux0"),
#ifdef USE_PLUGINS
#ifdef PLUGIN_NAME
  plugin(PLUGIN_NAME),
#else
  plugin("plugin0"),
#endif
#endif
  int_ppi0("int_ppi0", GIC_MAX_N_CPUS),
  int_ppi1("int_ppi1", GIC_MAX_N_CPUS),
  int_ppi2("int_ppi2", GIC_MAX_N_CPUS),
  int_ppi3("int_ppi3", GIC_MAX_N_CPUS),
  int_ppi4("int_ppi4", GIC_MAX_N_CPUS),
  int_spi32("int_spi32"),
  int_spi33("int_spi33"),
  int_spi34("int_spi34"),
  int_spi35("int_spi35"),
  int_spi36("int_spi36"),
  int_spi37("int_spi37"),
  int_spi38("int_spi38"),
  int_spi39("int_spi39"), 
  int_spi40("int_spi40"),
  int_spi41("int_spi41"),
  int_spi42("int_spi42"),
  int_spi43("int_spi43"),
  int_spi44("int_spi44"),
  int_spi45("int_spi45"),
  int_spi46("int_spi46"),
  int_spi47("int_spi47"),
  int_spi48("int_spi48"),
  int_spi49("int_spi49"),
  int_spi50("int_spi50"),
  int_spi51("int_spi51"),
  int_spi52("int_spi52"),
  int_spi53("int_spi53"),
  int_spi54("int_spi54"),
  int_spi55("int_spi55"),
  int_spi56("int_spi56"),
  int_spi57("int_spi57"),
  int_spi58("int_spi58"),
  int_spi59("int_spi59"),
  int_spi60("int_spi60"),
  int_spi61("int_spi61"),
  int_spi62("int_spi62"),
  int_spi63("int_spi63"),
  int_spi64("int_spi64"),
  int_spi65("int_spi65"),
  int_spi66("int_spi66"),
  int_spi67("int_spi67"),
  int_spi68("int_spi68"),
  int_spi69("int_spi69"),
  int_spi70("int_spi70"),
  int_spi71("int_spi71"),
  int_spi72("int_spi72"),
  int_spi73("int_spi73"),
  int_spi74("int_spi74"),
  int_spi75("int_spi75"),
  int_spi76("int_spi76"),
  int_spi77("int_spi77"),
  int_spi78("int_spi78"),
  int_spi79("int_spi79"),
  int_spi80("int_spi80"),
  int_spi81("int_spi81"),
  int_spi82("int_spi82"),
  int_spi83("int_spi83"),
  int_spi84("int_spi84"),
  int_spi85("int_spi85"),
  int_spi86("int_spi86"),
  int_spi87("int_spi87"),
  int_spi88("int_spi88"),
  int_spi89("int_spi89"),
  int_spi90("int_spi90"),
  int_spi91("int_spi91"),
  int_spi92("int_spi92"),
  int_spi93("int_spi93"),
  int_spi94("int_spi94"),
  int_spi95("int_spi95"),
  int_spi96("int_spi96"),
  int_spi97("int_spi97"),
  int_spi98("int_spi98"),
  int_spi99("int_spi99"),
  int_spi100("int_spi100"),
  int_spi101("int_spi101"),
  int_spi102("int_spi102"),
  int_spi103("int_spi103"),
  int_spi104("int_spi104"),  
  int_spi105("int_spi105"),
  int_spi106("int_spi106"),
  int_spi107("int_spi107"),
  int_spi108("int_spi108"),
  int_spi109("int_spi109"),  
  int_spi110("int_spi110"),
  int_spi111("int_spi111"),
  int_spi112("int_spi112"),
  int_spi113("int_spi113"),
  int_spi114("int_spi114"),  
  int_spi115("int_spi115"),
  int_spi116("int_spi116"),
  int_spi117("int_spi117"),
  int_spi118("int_spi118"),
  int_spi119("int_spi119"),  
  int_spi120("int_spi120"),
  int_spi121("int_spi121"),
  int_spi122("int_spi122"),
  int_spi123("int_spi123"),
  int_spi124("int_spi124"),  
  int_spi125("int_spi125"),
  int_spi126("int_spi126"),
  int_spi127("int_spi127") {
  
    dramsim_ini_t *ini = new dramsim_ini_t;
    ini->set_as_an_example(1024); // This 1024 is ignored.
    ini->deviceIniFilename = device_ini; //"../dramsim2/dist/system.ini.example";
    ini->systemIniFilename = system_ini; //"../dramsim2/dist/ini/DDR2_micron_16M_8b_x8_sg3E.ini";

    memory0 = new dramsim_sc_wrapper("memory0",  0, ini);
    
    if (g_wait_debugger) {
      external_debug_port = new vhls_debug_port("zynq");
      printf("Created GDB/RSP debug port\n");
    }
    else
      external_debug_port = 0;

    SDIO = new sdio_cbg("sdio", SDIO_BASE_ADDR, sdio_cbg::SD);
    
    for (int uu=0; uu<N_UARTS; uu++) {
      bool use_x11 = !g_noxterm, use_stdout = true;
      char spoolfile_name [132];
      snprintf(spoolfile_name, 132, "uart%i", uu);
      UARTS[uu] = new uart64_cbg(spoolfile_name, use_x11, use_stdout);
    }
    
    BUSMUX64_BIND(busmux0, memory0->port0, 0, iospace_base);
  
    m_scu = new arm_scu("arm_scu");
    bool snooping = false;
    // SCU is only active if L1 caches are defined
    // and number of cores is more then 1
#ifdef L1_CACHE_SIZE
    if(!g_no_caches && n_cores > 1) {
      snooping = true;
    }
#endif
    // TODO: implement logic for this in scu class
    if(!snooping) {
      delete m_scu->snoop_socket;
      m_scu->snoop_socket = NULL;
    }
    
    // TODO: all these bindings that we are doing here manually for modules
    // should be possible automatically to create by parsing DTS and Zynq
    // reference manual...maybe something to consider later
    
    
    // why are we calling xlat32to64 in armcore_tlm and do we need
    // to extend 32-bit address whose top 4 bits are 1 in this manner?
    BUSMUX64_BIND(busmux0, m_scu->arm_target_socket, SCU_BASE_ADDRESS, SCU_SIZE);
    
    for (int uu=0; uu<N_UARTS; uu++) {
      // E000_0000 E000_1000
      u64_t start = iospace_base + UART_SPACING * uu;
      BUSMUX64_BIND(busmux0, UARTS[uu]->port0, start, UART_SPACING); 
    }
    
    BUSMUX64_BIND(busmux0, SDIO->port0, SDIO_BASE_ADDR, SDIO_SPACING);
    // connect initiator socket to bus for DMA 
    SDIO->dma.bind(busmux0.targ_socket);
    
    gic = new gic_arm_tlm("gic", n_cores);
    sclr = new sclr_arm_tlm("sclr");
    
    // Timers
    ttc0 = new arm_ttc("ttc0");
    BUSMUX64_BIND(busmux0, ttc0->ttc_target0, TTC0_BASE_ADDR, 0x1000);
    ttc0->slcr_initiator.bind(sclr->cpu_target0);
    // Global Timer
    m_global_timer = new arm_glbt("global_timer", n_cores);
    m_global_timer->slcr_initiator.bind(sclr->cpu_target0);
    // This global timer is connected to each core after MMU
    // System watchdog timer
    swdt = new arm_swdt("swdt");
    swdt->slcr_initiator.bind(sclr->cpu_target0);
    BUSMUX64_BIND(busmux0, swdt->swdt_target, SWDT_BASE_ADDR, SWDT_LENGTH);
    
    // SCLR region:reg = <0xf8000000 0x1000>;
    BUSMUX64_BIND(busmux0, sclr->cpu_target0, 0xF8000000u, 0x1000);  //
    
#ifdef USE_PLUGINS
    BUSMUX64_BIND(busmux0, plugin.port0, iospace_base + UART_SPACING * 2, UART_SPACING);  // E000_2000
#endif
    
    u32_t trace_flags = 0;
    
#ifdef L2_CACHE_SIZE
    if(!g_no_caches) {
      char txt[132];
      snprintf(txt, 132, "l2_cache_and_controller");
      l2_cache = new arm_L2Cpl310(
				  txt,
				  0, 
				  32, // number of bits in address
				  L2_CACHE_SIZE, // cache size
				  32, // line size in bytes,
				  8, // number of ways
				  false, // cache is not snooped
				  arm_ccache::Unified, // both data and instructions
				  2); // level 2
      l2_cache->set_region(arm_ccache::Outer); // cache is in outer region
      l2_cache->set_shareable(false); // cache is not shared
      l2_cache->set_PoU(true); // cache is a point of unification
      l2_cache->disable(); // disabled at the start
      
      // L2 cache controller region: <0xf8f02000 0x1000>
      BUSMUX64_BIND(busmux0, l2_cache->controller_target, ARM_L2CPL310_BASE, ARM_L2CPL310_SIZE);
    }
#endif  
    
    for(int i=0;i<n_cores;i++) {
      char txt[132];
      snprintf(txt, 132, "coreunit_%d", i);
      Cores[i] = new coreunit(txt, i, harvardf, m_scu, m_global_timer, snooping, gic);
      Cores[i]->core->set_core_frequency(core_frequency, 1.6);
      sclr->add_core(Cores[i]->core);
      Cores[i]->l_twd->slcr_initiator.bind(sclr->cpu_target0);
#ifdef L1_CACHE_SIZE
      if (!g_no_caches) {
	printf("zynq.h: wiring ARM core %i. with caches. harvardf=%i\n", i, harvardf);
	if (g_dcache_tracelevel > 0) {
	  (new traceregion(0,
			   (u64_t)-1,
			   INT_MAX,
			   TENOS_TRACE_MEM_READ|TENOS_TRACE_MEM_WRITE)
	   )->add(Cores[i]->l1_d_cache->traceregions);
	}
	
	POWER3_TRC_2(*Cores[i]->l1_d_cache, sum_children);
	if (harvardf) {
	  if (g_dcache_tracelevel > 0) {
	    (new traceregion(0,
			     (u64_t)-1,
			     INT_MAX,
			     TENOS_TRACE_MEM_READ|TENOS_TRACE_MEM_WRITE|TENOS_TRACE_WRITE_BUFFER_OP)
	     )->add(Cores[i]->l1_d_cache->traceregions);
	  }
	  POWER3_TRC_2(*Cores[i]->l1_i_cache, sum_children);
	}
      }
      else {
	m_scu->initiator_socket->bind(busmux0.targ_socket);
	if (harvardf) Cores[i]->core->busaccess.ifetch_socket->bind(busmux0.targ_socket);
      }
#else
      m_scu->initiator_socket->bind(busmux0.targ_socket);
      if (harvardf)
	Cores[i]->core->busaccess.ifetch_socket->bind(busmux0.targ_socket);
#endif
      
      if (external_debug_port) external_debug_port->connect_ip_block(txt, Cores[i]->core);
      
    }
    
#ifdef L2_CACHE_SIZE
    if(!g_no_caches) {
      m_scu->initiator_socket->bind(l2_cache->target_socket);
      m_scu->init_arm_control_socket();
      m_scu->arm_initiator_socket->bind(l2_cache->cp15_target_socket);
    }
#endif
    
#ifdef L2_CACHE_SIZE
    if(!g_no_caches) {
      l2_cache->initiator_socket.bind(busmux0.targ_socket);
    }
#endif
    
    // Initialise SCU
    m_scu->init();

    traceregion *itrace = 0;
    if (g_tracelevel || g_watches)
      for (int k=0; k <= g_watches; k++) {  // Add a final, global tracelevel beyond the watch regions.
	int flags = flags_for_level(k>=g_watches ? g_tracelevel: Watches[k].level);
	if (flags) {
	  u64_t lo = k>=g_watches ? 0: Watches[k].lo;
	  u64_t hi = k>=g_watches ? (u64_t)-1: Watches[k].hi;
	  (new traceregion(lo, hi, INT_MAX, flags))->add(itrace);
	}
      }
    
    if (g_dcache_tracelevel > 0) {  // Also trace main memory for this dcache region. 
      int k = 0;
      u64_t lo = k>=g_watches ? 0: Watches[k].lo;
      u64_t hi = k>=g_watches ? (u64_t)-1: Watches[k].hi;
      (new traceregion(lo,
		       hi,
		       INT_MAX,
		       TENOS_TRACE_MEM_READ|TENOS_TRACE_MEM_WRITE|TENOS_TRACE_WRITE_BUFFER_OP)
       )->add(memory0->traceregions);
    }

    if (itrace) {
      for (int i=0;i<n_cores;i++) itrace->add(Cores[i]->core->busaccess.traceregions);
      itrace->add(busmux0.traceregions);
      for (int u=0;u<N_UARTS;u++) itrace->add(UARTS[u]->traceregions);
      itrace->add(memory0->traceregions);
#ifdef USE_PLUGINS
      //itrace->add(plugin.traceregions);
#endif
    }
    POWER3_TRC_3(*this, "TOPTHIS", sum_children); // PlotCombinedAccounts
    POWER3_TRC_2(*memory0, "Memory 0 (DRAM)");
    POWER3_TRC_1(*UARTS[0]);
    //    pw_trace(g_txt_pt, (the_benchconfig->busmux0), "BusMux 0");
    //    pw_trace(g_txt_pt, (the_benchconfig->uart0),   "Uart 0");
    
    
    int uu = 1;
    extern const char *uart_canned_input;
    if (uart_canned_input) UARTS[uu]->register_canned_input(uart_canned_input);
    
#ifdef OLD
    // We do not have a GIC - generic interrupt controller - model at the moment, so here we will
    // hardwire UART[0]s interrupt to the zeroth ARM core.
    Cores[0]->core->connect_interrupt(&UARTS[0]->interrupt_request);
#endif

    if (gic) {
      for (int i = 0; i<GIC_MAX_N_CPUS; i++) {
	(gic->IRQ_out[i])(IRQs[i]);
	(gic->FIQ_out[i])(FIQs[i]);
	(gic->ppi0[i])(int_ppi0[i]);
	(gic->ppi1[i])(int_ppi1[i]);
	(gic->ppi2[i])(int_ppi2[i]);
	(gic->ppi3[i])(int_ppi3[i]);
	(gic->ppi4[i])(int_ppi4[i]);
	
      }
      
      for(int i=0;i<n_cores;i++)  // Wire GIC interrupt outputs to cores and event inputs between cores.
	{
	  Cores[i]->core->IRQ(IRQs[i]);
	  
	  //Cores[i]->core->event_in(core_events[i]);
	  // if (!(n_cores &1) && i != n_cores -1) Cores[i]->core->event_out(core_events[i ^ 1]); // wire odd-even pairs
	  
	  
	  
	   // Skip FIQ wiring since our ARM model has that input missing - but only a simple edit should be needed to implement it since the register banks and so on are all implemented properly.
           // Cores[i]->core->FIQ(FIQs[i]); 
	}
      
      gic->int_spi32(int_spi32);
      gic->int_spi33(int_spi33);
      gic->int_spi34(int_spi34);
      gic->int_spi35(int_spi35);
      gic->int_spi36(int_spi36);
      gic->int_spi37(int_spi37);
      gic->int_spi38(int_spi38);
      gic->int_spi39(int_spi39);
      gic->int_spi40(int_spi40);
      gic->int_spi41(int_spi41);
      gic->int_spi42(int_spi42);
      gic->int_spi43(int_spi43);
      gic->int_spi44(int_spi44);
      gic->int_spi45(int_spi45);
      gic->int_spi46(int_spi46);
      gic->int_spi47(int_spi47);
      gic->int_spi48(int_spi48);
      gic->int_spi49(int_spi49);
      gic->int_spi50(int_spi50);
      gic->int_spi51(int_spi51);
      gic->int_spi52(int_spi52);
      gic->int_spi53(int_spi53);
      gic->int_spi54(int_spi54);
      gic->int_spi55(int_spi55);
      gic->int_spi56(int_spi56);
      gic->int_spi57(int_spi57);
      gic->int_spi58(int_spi58);
      gic->int_spi59(int_spi59);
      gic->int_spi60(int_spi60);
      gic->int_spi61(int_spi61);
      gic->int_spi62(int_spi62);
      gic->int_spi63(int_spi63);
      gic->int_spi64(int_spi64);
      gic->int_spi65(int_spi65);
      gic->int_spi66(int_spi66);
      gic->int_spi67(int_spi67);
      gic->int_spi68(int_spi68);
      gic->int_spi69(int_spi69);
      gic->int_spi70(int_spi70);
      gic->int_spi71(int_spi71);
      gic->int_spi72(int_spi72);
      gic->int_spi73(int_spi73);
      gic->int_spi74(int_spi74);
      gic->int_spi75(int_spi75);
      gic->int_spi76(int_spi76);
      gic->int_spi77(int_spi77);
      gic->int_spi78(int_spi78);
      gic->int_spi79(int_spi79);
      gic->int_spi80(int_spi80);
      gic->int_spi81(int_spi81);
      gic->int_spi82(int_spi82);
      gic->int_spi83(int_spi83);
      gic->int_spi84(int_spi84);
      gic->int_spi85(int_spi85);
      gic->int_spi86(int_spi86);
      gic->int_spi87(int_spi87);
      gic->int_spi88(int_spi88);
      gic->int_spi89(int_spi89);
      gic->int_spi90(int_spi90);
      gic->int_spi91(int_spi91);
      gic->int_spi92(int_spi92);
      gic->int_spi93(int_spi93);
      gic->int_spi94(int_spi94);
      gic->int_spi95(int_spi95);
      gic->int_spi96(int_spi96);
      gic->int_spi97(int_spi97);
      gic->int_spi98(int_spi98);
      gic->int_spi99(int_spi99);
      gic->int_spi100(int_spi100);
      gic->int_spi101(int_spi101);
      gic->int_spi102(int_spi102);
      gic->int_spi103(int_spi103);
      gic->int_spi104(int_spi104);
      gic->int_spi105(int_spi105);
      gic->int_spi106(int_spi106);
      gic->int_spi107(int_spi107);
      gic->int_spi108(int_spi108);
      gic->int_spi109(int_spi109);
      gic->int_spi110(int_spi110);
      gic->int_spi111(int_spi111);
      gic->int_spi112(int_spi112);
      gic->int_spi113(int_spi113);
      gic->int_spi114(int_spi114);
      gic->int_spi115(int_spi115);
      gic->int_spi116(int_spi116);
      gic->int_spi117(int_spi117);
      gic->int_spi118(int_spi118);
      gic->int_spi119(int_spi119);
      gic->int_spi120(int_spi120);
      gic->int_spi121(int_spi121);
      gic->int_spi122(int_spi122);
      gic->int_spi123(int_spi123);
      gic->int_spi124(int_spi124);
      gic->int_spi125(int_spi125);
      gic->int_spi126(int_spi126);
      gic->int_spi127(int_spi127);
    }

    inter_core_event.store(0);
    for(int i = 0; i < n_cores; i++) {
      Cores[i]->core->set_inter_core_event(&inter_core_event);
    }
    
    
    //Interrupt wiring: 7-3 on page 218 in Zynq-7000-TRM.pdf
    for(int i = 0; i < GIC_MAX_N_CPUS; i++)
      m_global_timer->IRQ[i](int_ppi0[i]);
    
    for(int i = 0; i < n_cores; i++) {
      // private timer
      Cores[i]->l_twd->IRQ(int_ppi2[i]);
      // watchdog
      Cores[i]->l_twd->wdg_IRQ(int_ppi3[i]);
    }
    swdt->IRQ(int_spi41);
    ttc0->IRQ[0](int_spi42);
    ttc0->IRQ[1](int_spi43);
    ttc0->IRQ[2](int_spi44);
    UARTS[0]->IRQ(int_spi59);
    SDIO->IRQ(int_spi79);
    UARTS[1]->IRQ(int_spi82);
    
    char line[444];
    printf("WE ARE HERE2222 %s -- %s\n", UARTS[uu]->name(), g_name);
    snprintf (line, 444, "%s.%s.txt", g_name, UARTS[uu]->name());
    printf("WE ARE HERE3333\n");
    UARTS[uu]->open_log_file(line);
}

void top::hup_handler(int p_arg) { // Get a little trace from a busy simulation by sending it a HUP.
  for(int i=0;i<n_cores;i++) Cores[i]->core->busaccess.trace100("hup handler");
}

const char* top::kind() {
  return "bench1";
}
    
top::~top(void) {
}
    
void top::stat_report(const char *msg, FILE *fd, bool resetf) {
  if (fd) fprintf(fd, "%s %s  \n", name(), msg);
#if TRACECOMM
  FILE *comm_f = fopen("comm_matrix.csv", "w");
  if(comm_f != NULL) {
    fprintf(comm_f, ",");
    for(int i = 0; i < n_cores; ++i)
      fprintf(comm_f, "C%d,", i);
    fprintf(comm_f, "\n");
  }
#endif
  for(int i=0;i<n_cores;i++) {
    Cores[i]->stat_report(msg, fd, resetf);
#if TRACECOMM    
    Cores[i]->l1_d_cache->comm_stat_report(comm_f);
#endif
  }
  busmux0.stat_report(msg, fd, resetf);
  int uu = 0;
  UARTS[uu]->stat_report(msg, fd, resetf);
  memory0->stat_report(msg, fd, resetf);
#ifdef L2_CACHE_SIZE
  // Causing a segment fault.
  if (l2_cache) l2_cache->stat_report(msg, fd, resetf);
#endif
#if TRACECOMM
  FILE *hit_f = fopen("hit_matrix.csv", "w");
  if(hit_f != NULL) {
    fprintf(hit_f, ",L1,L2,MEM\n");
    for(int i = 0; i < n_cores; ++i) {
      fprintf(hit_f, "C%d,", i);
      fprintf(hit_f, "%ld,", Cores[i]->l1_d_cache->hit_reads(i));
      fprintf(hit_f, "%ld,", l2_cache->hit_reads(i));
      fprintf(hit_f, "%ld\n",memory0->m_hit_reads[i]);
    }
  }
#endif    
  
  if(SDIO) SDIO->stat_report(msg, fd, resetf);
  //if (resetf) stats.reset();
}

sc_module* top::get_Core(int i) {
  return Cores[i];
}

extern "C" {
  platform *maker(
      const sc_module_name& p_name,
      int n_cores,
      double core_frequency,
      bool harvardf,
      const char* device_ini,
      const char* system_ini) {
    return new top(p_name,
		   n_cores,
		   core_frequency,
		   harvardf,
		   device_ini,
		   system_ini);
  }
}

// eof

