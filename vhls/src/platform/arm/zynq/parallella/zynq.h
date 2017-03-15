/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */


/*
 * A example testbench. Multiple ARM cores with their own L1 caches.
 * This bench approximates to a Zynq 7000 chip when the number of cores
 * is set to 2.
 */

#include <platform/platform.h>
#include <systemc>
#include "tlm.h"
#ifdef TLM_POWER3
#include <tlm_power>
#define POWER3(X) X
using namespace sc_pwr;
#else
typedef tlm::tlm_base_protocol_types PW_TLM_TYPES;
typedef tlm::tlm_generic_payload PW_TLM_PAYTYPE;
#define POWER3(X)
#endif

#include "arm/arm_cortex_a9.h"
#include "arm/arm_scu.h"
#include "arm/arm_cp15.h"
#include "arm/arm_cp14.h"
#include "arm/arm_mmu.h"
#include "arm/arm_ccache.h"
#include "arm/gic_arm_tlm.h"
#include "arm/sclr_arm_tlm.h"
#include "arm/arm_timers.h"
#include "arm/arm_L2Cpl310.h"

#include "bus/busmux64.h"
#include "memories/sram64_cbg.h"
#include "memories/dram64_cbg.h"
#include "dramsim2/dramsim_sc_wrapper.h"
#include "gdbrsp/vhls_soc_debug.h"

#ifdef USE_PLUGINS
#include "plugins/plugins.h"
#endif

#include "io/uart64_cbg.h"
#include "io/sdio_cbg.h"

#define L1_CACHE_SIZE (1<<15) // 32 Kb 
#define L2_CACHE_SIZE (1<<19) // 512 Kb

#define USE_MAINMEM_MACRO_MODEL  0

#define SDIO_BASE_ADDR 0xE0101000
#define SDIO_SPACING 0x1000

#define N_UARTS 2

#ifdef TLM_POWER3
#define POWER3_TRC_1(IT)       { if (g_txt_pt) pw_trace(g_txt_pt, IT);  if (g_vcd_pt) pw_trace(g_vcd_pt, IT); }
#define POWER3_TRC_2(IT, X)    { if (g_txt_pt) pw_trace(g_txt_pt, IT, X);  if (g_vcd_pt) pw_trace(g_vcd_pt, IT, X); }
#define POWER3_TRC_3(IT, X, Y) { if (g_txt_pt) pw_trace(g_txt_pt, IT, X, Y);  if (g_vcd_pt) pw_trace(g_vcd_pt, IT, X, Y); }
#else
#define POWER3_TRC_1(IT) 
#define POWER3_TRC_2(IT, X) 
#define POWER3_TRC_3(IT, X, Y) 
#endif

#define UART_SPACING 0x1000

//The following defines set up the memory map
#define sram_start_adr 0
#define iospace_base    0xe0000000 // zync has uarts at E000_1000 and E000_0000

extern int g_no_caches;
extern bool g_wait_debugger;
extern int g_self_starting_cores;
extern int g_noxterm;
extern int g_dcache_tracelevel;
extern const char* g_name;
extern int g_tracelevel;
extern int g_watches;
extern struct watch_s Watches[MAXWATCH];

#ifdef TLM_POWER3
extern pw_trace_file* g_vcd_pt;
extern pw_trace_file* g_txt_pt;
extern pw_trace_file* g_slk_pt;
#endif // TLM_POWER3


// ===================================================================
//! TOP module containing a number of cores plus some memories etc.

// A single core + level optional 1 cache(s).
class coreunit:
  public sc_module
#ifdef TLM_POWER3
  , public pw_module
#endif
{
 public:
  SC_HAS_PROCESS(coreunit);
  arm_cortex_a9 *core;
  // each core has memory management unit (MMU)
  arm_mmu * l_mmu;
  // each core has a private timer and watchdog
  arm_twd* l_twd;
  // each core has the following coprocessors
  // CP14
  arm_cp14* l_cp14;
  // CP15
  arm_cp15* l_cp15;

  arm_ccache *l1_d_cache;
  arm_ccache *l1_i_cache;

 // Constructor
 coreunit(
	  sc_core::sc_module_name p_name, 
	  int i, 
	  bool harvardf,
	  arm_scu* l_scu,
	  arm_glbt* l_global_timer,
          bool snooping,
          gic_arm_tlm* l_gic);

 void stat_report(const char *msg, FILE *fd, bool resetf=0);

 const char *kind();
};

class top:
  public platform,
  public sc_module
#ifdef TLM_POWER3
 ,  public pw_module
#endif
{
 public:
  u64_t entrypt;
  int n_cores;
  coreunit *Cores[MAX_NO_OF_CORES];

  vhls_debug_port *external_debug_port;

#if USE_MAINMEM_MACRO_MODEL  
  sram64_cbg *memory0;
#else
  dramsim_sc_wrapper *memory0;
#endif

  sdio_cbg *SDIO;
  uart64_cbg *UARTS[N_UARTS];
  busmux64 busmux0;

  arm_scu* m_scu; 
  arm_L2Cpl310* l2_cache;

  gic_arm_tlm *gic;
  sclr_arm_tlm *sclr;

  arm_ttc *ttc0;
  arm_glbt *m_global_timer;
  arm_swdt *swdt;

  std::atomic<u32_t> inter_core_event;

#ifdef USE_PLUGINS
  plugin_t plugin;
#endif

  // Interrupt wires.
  sc_vector <sc_signal<bool,SC_UNCHECKED_WRITERS> > int_ppi0, int_ppi1, int_ppi2, int_ppi3, int_ppi4;
  sc_signal<bool,SC_UNCHECKED_WRITERS> int_spi32, int_spi33, int_spi34, int_spi35, int_spi36, int_spi37, int_spi38, int_spi39;
  sc_signal<bool,SC_UNCHECKED_WRITERS> int_spi40, int_spi41, int_spi42, int_spi43, int_spi44, int_spi45, int_spi46, int_spi47, int_spi48, int_spi49;
  sc_signal<bool,SC_UNCHECKED_WRITERS> int_spi50, int_spi51, int_spi52, int_spi53, int_spi54, int_spi55, int_spi56, int_spi57, int_spi58, int_spi59;
  sc_signal<bool,SC_UNCHECKED_WRITERS> int_spi60, int_spi61, int_spi62, int_spi63, int_spi64, int_spi65, int_spi66, int_spi67, int_spi68, int_spi69;
  sc_signal<bool,SC_UNCHECKED_WRITERS> int_spi70, int_spi71, int_spi72, int_spi73, int_spi74, int_spi75, int_spi76, int_spi77, int_spi78, int_spi79;
  sc_signal<bool,SC_UNCHECKED_WRITERS> int_spi80, int_spi81, int_spi82, int_spi83, int_spi84, int_spi85, int_spi86, int_spi87, int_spi88, int_spi89;
  sc_signal<bool,SC_UNCHECKED_WRITERS> int_spi90, int_spi91, int_spi92, int_spi93, int_spi94, int_spi95, int_spi96, int_spi97, int_spi98, int_spi99;
  sc_signal<bool,SC_UNCHECKED_WRITERS> int_spi100, int_spi101, int_spi102, int_spi103, int_spi104, int_spi105, int_spi106, int_spi107, int_spi108, int_spi109;
  sc_signal<bool,SC_UNCHECKED_WRITERS> int_spi110, int_spi111, int_spi112, int_spi113, int_spi114, int_spi115, int_spi116, int_spi117, int_spi118, int_spi119;
  sc_signal<bool,SC_UNCHECKED_WRITERS> int_spi120, int_spi121, int_spi122, int_spi123, int_spi124, int_spi125, int_spi126, int_spi127;


  sc_signal<bool> IRQs[MAX_NO_OF_CORES];
  sc_signal<bool> FIQs[MAX_NO_OF_CORES];


  void reset(const char* boot, const char* dtb, const char* kernel, const char* vdd);

  u32_t flags_for_level(int tracelevel);

  // default constructor - it takes args - how can it be the default?
  top(const sc_module_name& p_name,
      int n_cores,
      double core_frequency,
      bool harvardf,
      const char* device_ini,
      const char* system_ini);

  // Get a little trace from a busy simulation by sending it a HUP.
  void hup_handler(int p_arg);

  const char *kind();
    
  ~top(void);

  void stat_report(const char *msg, FILE *fd, bool resetf = 0);

  sc_module* get_Core(int i);
};

extern "C" void test(void) {
    printf("Hello\n");
}

// MP: Can be used to instrument SystemC kernel
// TODO: Needs to be properly factorised
#if 0
#include "papi.h"
class arm_observer : public sc_observer {
  private:
    arm_cortex_a9* m_core;
    std::vector<int> m_instructions;
    std::vector<long long> m_papi;
    std::vector<u64_t> m_cycles;

    uint64_t m_instruction_count;
    uint64_t m_cycle_count;
    long long m_values[1];
    uint64_t m_total_evals;

    float real_time_i, proc_time_i, ipc_i;
    long long ins_i;
    float real_time, proc_time,ipc;
    long long ins;


  public:
    arm_observer(arm_cortex_a9* core) 
        : m_core(core), 
        m_instruction_count(0),
        m_total_evals(0),
        sc_observer() {
        PAPI_ipc(&real_time_i, &proc_time_i, &ins_i, &ipc_i);
    }

    void start_evaluation() {
        m_instruction_count = m_core->stats.Instructions;
        m_cycle_count = m_core->stats.Cycles;
        //PAPI_read_counters(m_values, 1);
    }

    void end_evaluation() {
        long long previnstr = m_values[0];
        PAPI_read_counters(m_values, 1);
        PAPI_ipc(&real_time, &proc_time, &ins, &ipc);
        long long mips = ins - ins_i;
        ins_i = ins;

        //long long papi = m_values[0] - previnstr;
        uint64_t instructions = 
            m_core->stats.Instructions - m_instruction_count;
        uint64_t cycles =
            m_core->stats.Cycles - m_cycle_count;
        //printf("papi=%ld, instructions=%ld\n", papi, instructions);
        m_instructions.push_back(instructions);
        m_cycles.push_back(cycles);
        m_papi.push_back(mips);
        m_total_evals++;
    }

    void report() {
        printf("Total evaluations: %d\n", m_total_evals);
        double sum = std::accumulate(
            m_instructions.begin(),
            m_instructions.end(),
            0.0);
        double mean = sum / m_instructions.size();
        printf("Mean of instructions %.2f\n", mean);
        double sum2 = std::accumulate(
            m_papi.begin(),
            m_papi.end(),
            0.0);
        double mean2 = sum2 / m_papi.size();
        printf("Mean of hw instructions %.2f\n", mean2);

        double sum3 = std::accumulate(
            m_cycles.begin(),
            m_cycles.end(),
            0.0);
        double mean3 = sum3 / m_cycles.size();
        printf("Mean of cycles %.2f\n", mean3);
        

        printf("Maximum: %d\n", 
               *std::max_element(m_instructions.begin(), m_instructions.end()));
        printf("Minimum: %d\n", 
               *std::min_element(m_instructions.begin(), m_instructions.end()));

    }
};

#endif

// eof

