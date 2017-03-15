#ifndef DRAMSIM_SC_WRAPPER_H
#define DRAMSIM_SC_WRAPPER_H



// $Id: dram_cbg.h,v 1.12 2011/06/30 12:07:08 djg11 Exp $
//
// dramsim_sc_wrapper: A TLM dynamic memory abstraction with controller and power modelling using dramsim2.
// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2011 DJ Greaves
// University of Cambridge, Computer Laboratory.




// This DRAM model is not synchronous, although modern DRAMs are invariably so, with a fully-pipelined access latency of one or two clock cycles, depending on size.
// Strictly modelling synchronous DRAM is not appropriate for LT-TLM models, but the combined latency figure attributed to the bus and RAM should be the clock period for single issue operation (and half as much for dual issue if such a concept exists, etc).

#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"
#include "tenos.h"
#include "llsc_extension.h"
//#include "support/mamba_extension.h"
#include "memloaders.h"
#include "tlm_utils/peq_with_cb_and_phase.h"

#include <unordered_map>

#ifndef POWER3
#ifdef TLM_POWER3
#include <tlm_power>
#define POWER3(X) X
using namespace sc_pwr;
#else
typedef tlm::tlm_base_protocol_types PW_TLM_TYPES;
typedef tlm::tlm_generic_payload PW_TLM_PAYTYPE;
#define POWER3(X)
#endif
#endif


#include "dist/MemorySystem.h"
#include "dist/Callback.h"


class dramsim_ini_t
{
 public:
  const char *deviceIniFilename;
  const char *systemIniFilename;
  const char *pwdString;
  const char *traceFileName;
  int megsOfMemory;

  void set_as_an_example(int no, int sizeInMegs = 4 * 1024);
};

#define MAX_DRAMSIM_THREADS 32


class dramsim_sc_wrapper: 
  public sc_module,
#ifdef TLM_POWER3
  public pw_module,
#endif
  public memloader_if
{
  SC_HAS_PROCESS(dramsim_sc_wrapper); 
#ifdef TLM_POWER3
  POWER3(PW_TLM3(sc_pwr::tlm_bit_transition_tracker read_bus_tracker)); // Used if counting wire transitions. 
#endif


  sc_mutex m_qlock;
  tlm::tlm_endianness m_big_endian;
  bool m_tracing_on;
  u64_t m_watchpoint;
  double stat_energy_sofar; 
  double dyn_energy_sofar; 
#ifdef TLM_POWER3
  void energy_accounter(sc_pwr::pw_agent_record *p_agent,
			PW_TLM_PAYTYPE *trans,
			tlm::tlm_command cmd = tlm::tlm_command::TLM_IGNORE_COMMAND);
#endif

  uint32_t manual_delay;

  //  sc_time local_quantum;// TODO use global
  //void keeper(sc_time &delay);
  // We support multiple outstanding controller operations by letting one thread clock the 
  // dramsim2 sub module while others are blocked in this queue.
  // 
  struct queue_item
  {
    Transaction trans;
    bool prim;
    bool active;
    
    // at what cycle trasnaction was added
    uint64_t add_clock_cycle;
    // calculated delay
    sc_time delay; 
    
    queue_item() // constructor 
    {
      active = false;
      add_clock_cycle = 0;
      delay = SC_ZERO_TIME;
    }
  } Queue[MAX_DRAMSIM_THREADS];
  int m_queued_items;


  class Transaction_mm // A complete memory manager just to implement 'postional new'
  {
    void *it;
  public:
    void *alloc(int size)
    {
      assert(size == sizeof(Transaction));
      assert(it);
      void *r = it; it = 0; return r;
      cout << " ret " << r << " for s=" << size << "\n";
    }
    Transaction_mm(Transaction *x) { it = (void *) x; cout << "mm made " << it << "\n"; }
  };

  int m_systemID;
  void blockwhile(TransactionType transType, u64_t adr, sc_time &delay); // Main blocking TLM operation.

#define INTERNALPAGESIZE (4096LLU * 8LLU) // 4K pages in bits
  // Internally we use pages to save VM on the modelling workstation
  // but these are not intended to represent anything physical.
  class internal_page
  {
  public:
    unsigned char data [INTERNALPAGESIZE / 8LLU];

    u64_t base;
  internal_page(int x) : base(x)
      {
	
      }


  };



  internal_page *last_page;

  std::unordered_map<u64_t, internal_page *> memory_hash_map;

  bool ip_read(u64_t a, u8_t &d);
  bool ip_read(u64_t a, u64_t &d); // 64 bit read (fast path).
  bool ip_clear_(u64_t a);
  bool ip_write(u64_t a, u8_t d, bool es=false);
  bool ip_write(u64_t a, u64_t d); // 64 bit write (fast path).
  
  dramsim_ini_t * ini;
  //#ifndef LLSC_DISABLE
  // Load linked(or locked), store conditional location(s).
  llsc_locks_t llsc_locks;
  //#endif
  void set_tracing(bool m) { m_tracing_on = m; }


  // blocking transport interface prototype
  void b_access(PW_TLM_PAYTYPE &trans, sc_time &delay);
  void peq_cb(PW_TLM_PAYTYPE &trans, const tlm::tlm_phase& ph);
  
  u64_t mem_base;      // Base address in bytes
  u64_t mem_size_bytes;// No of bytes
  u8_t mem_width;      // Bits per location
  u64_t mem_blocksize_bytes;
  bool get_direct_mem_ptr(PW_TLM_PAYTYPE&, tlm::tlm_dmi& dmi_data);
 private:

#ifdef TLM_POWER3
  pw_energy hi_energy_op, lo_energy_op, llsc_energy_op;
#endif

  const sc_time clock_period;
  bool tracing_on;
  bool dmi_on;  // Allow the direct memory access optimisation

  class cbgihex *iloader;
  class cbgelf *eloader;
  void pw_eng_callback(u32_t sysid, double stat_energy, double dyn_energy);
  void op_complete(u32_t sysid, u64_t, u64_t);
  MemorySystem *ms;
  void update();


  struct stats_t
  {
    u64_t m_read_ops, m_write_ops;
    u64_t m_read_bytes, m_write_bytes;
    u64_t m_row_activations;
    stats_t() { reset(); }
    void reset()
    {
      m_row_activations = 0;
      m_read_ops = 0;
      m_write_ops = 0;
      m_read_bytes = 0;
      m_write_bytes = 0;
    }
  } m_stats;

  tlm_utils::peq_with_cb_and_phase<dramsim_sc_wrapper, PW_TLM_TYPES> m_peq;

 public:   void tmpmain();
  void end_of_simulation();
  // Constructor
  dramsim_sc_wrapper(sc_core::sc_module_name name, 
		     u64_t mem_base, 
		     dramsim_ini_t *ini, 
		     uint32_t manual_delay = 1,
		     bool dmi_on=true,
		     sc_time clock_period = sc_time(5, SC_NS));
  void loadme(const char *filename, bool elff, u64_t startadr, u64_t *entrypt=0);
  void loadme(const char *filename, bool elff, u64_t *entrypt=0);
  
  void loaddtb(const char *blob, u64_t offset);

  tlm::tlm_sync_enum nb_transport_fw(PW_TLM_PAYTYPE& trans,
				     tlm::tlm_phase& phase,
				     sc_time& delay);
  
  //TLM-2 socket, 64-bits wide socket used but this model can support DRAM data width an arbitrary multiple of 64bits.
  tlm_utils::simple_target_socket<dramsim_sc_wrapper, 64, PW_TLM_TYPES> port0;
  u32_t width(); // DRAM databus word width in bits.
  u32_t bl();    // DRAM burst length in units of databus words.
  void stat_report(const char *msg, FILE *fd, bool resetf=false);
  void set_watchpoint(u64_t a)
    {
      m_watchpoint = (a & ~7LLU);
    }

  void set_bigendian(tlm::tlm_endianness v) { m_big_endian = v; } // This will be set from ELF load but this methods provides a manual override. The memory itself does not need this apart from load: it is in host-workstation u64_t order.
  traceregion *traceregions;
  const char *kind();

#if TRACECOMM
  uint64_t m_hit_reads[255];
#endif
};

#endif
