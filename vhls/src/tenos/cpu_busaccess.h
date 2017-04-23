/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

#ifndef CPU_BUSACCESS_H
#define CPU_BUSACCESS_H

#define DMI_STATS 1
//
// $Id: $
//
// A generic bus access unit for processor cores.  This goes between the ISS/etc and the L1 caches.
//

#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
//#include "tlm_utils/tlm_quantumkeeper.h"
//#include "tlm_h/tlm_trans/tlm_2_interfaces/tlm_dmi.h
#include "tenos.h"
#include "prazor.h"




#include "llsc_extension.h"
#include "argv_backdoor.h"
#include "io_backdoor.h"


#ifdef WITH_SPEEDO
#include "speedo_api.h"
#endif

#include "pw_confidence_switcher.h"

typedef u64_t addr_t;
typedef addr_t breakpt_t;

struct bench_if // All mtracer test benches should implement this interface.
{
  sc_time run_start;
  sc_time run_end;

  sc_event dynload;
  sc_event done;

  virtual void flushout() = 0;
  virtual void stat_report(const char *, FILE *, bool resetf=false) = 0;
  virtual void set_bigendian(tlm::tlm_endianness) = 0;
};


//extern class bench_if *the_bench;
//extern void xspawn(int parent, int tidx);
extern int g_confidence;

#define TRACEBUFFER_LENGTH 1024
#define N_CUSTOMER_IDS_PER_CORE SPEEDO_CONTEXTS

class cpu_busaccess: 
  public sc_module
#ifdef TLM_POWER3
  , public speedo_resource_ifc
  , public pw_module
#endif
{
  SC_HAS_PROCESS(cpu_busaccess);
  PRAZOR_GP_T *llsc_payload_alloc(); // Payload allocator with llsc extensions.
#ifdef TLM_POWER3
  sc_pwr::pw_accounting_base *a_customer_observer;
  int n_customer_ids_in_use;
  int current_alias; // Which customer id_to_use.
#ifdef WITH_SPEEDO
 public:
  speedo_ifc_t speedo;
#endif
 private:
  pw_stat_observer_record *a_record;
  pw_energy energy_report(bool printit);
  sc_pwr::pw_customer_id *Customer_ids[N_CUSTOMER_IDS_PER_CORE];
#if PW_TLM_PAYLOAD > 0
  sc_pwr::tlm_bit_transition_tracker(bus_tracker0); // Used if counting wire transitions. 
  sc_pwr::tlm_bit_transition_tracker(bus_tracker1); // Used if counting wire transitions. 
#endif
#endif

  //#define ENDIAN_FLIP 7 // Modelling big-endian on a little endian host (x86).
  // For a big-endian CPU, modelled on x86, ENDIAN_FLIP will be 7, meaning byte zero is at the top and needs the most shifting down.
  int ENDIAN_FLIP;

  int atomics;  // Non zero while attempting atomic bus transaction.
  int atomic_width;
  u64_t atomic_wdata;
  addr_t atomic_addr;
#define NO_CPU_BUSACCESS_SKTS 2
  FILE *trace_fd;

  // Direct memory interface (DMI is a SystemC speedy bus bypass).
  struct dmi_record
  {
    tlm::tlm_dmi data;
    bool data_valid;
#ifdef TLM_POWER3
    confidence_switcher<pw_energy> read_energy;
    confidence_switcher<pw_energy> write_energy;
#endif
    confidence_switcher<sc_time> read_latency;
    confidence_switcher<sc_time> write_latency;

  dmi_record(std::string names) : // constructor
#ifdef TLM_POWER3
    read_energy(PW_ZERO_ENERGY, names, "read_energy", g_confidence),
      write_energy(PW_ZERO_ENERGY, names, "write_energy", g_confidence),
#endif
      read_latency(SC_ZERO_TIME, names, "read_latency", g_confidence),
      write_latency(SC_ZERO_TIME, names, "write_latency", g_confidence)
    {
#ifdef DMI_STATS
      dmi_stats.reset();
#endif
    };
#ifdef DMI_STATS
    struct {
      int too_low, too_high, not_valid, linked, measure_needed, aok; 
      long long int too_high_margin;

      void reset()
      {
	too_low = 0; 
	too_high_margin = 0;
	too_high = 0; not_valid = 0; linked = 0; measure_needed = 0; 
	aok = 0;
      }
    } dmi_stats;


#endif

    
  } *DMI_records[NO_CPU_BUSACCESS_SKTS];

#ifdef TLM_POWER3
  void resume_accounting_alias(int n); // Change token put in my initiated traffic.
#endif


  struct stats_t
  {
    int reads;
    int writes;
    int dmi_reads;

    void reset()
    {
      dmi_reads = 0;
      reads = 0;
      writes = 0;
    }

    stats_t() { reset(); } //todo does not reset dmi_stats part.

  } stats;

 protected:
  friend class OR1200IF;

 public:
  bool core_finished;
  sc_module *m_module;
  int disable_dmi;
  int harvardf; // Set to 1 for separate I and D fetch.
  traceregion *traceregions;
  void traceregion_add(traceregion *n) { n->add(traceregions); }
  virtual const char *kind() { return "cpu_busaccess"; }
  void stat_report(const char *msg, FILE *fd, bool resetf=false);
  // constructor
  cpu_busaccess(sc_module *module, sc_module_name name, int endian_flip, int idx=0, int harvardf=0, int disable_dmi=0);
  void invalidate_direct_mem_ptr0(sc_dt::uint64 start, sc_dt::uint64 end);
  void invalidate_direct_mem_ptr1(sc_dt::uint64 start, sc_dt::uint64 end);
  tlm_utils::simple_initiator_socket<cpu_busaccess, 64, PW_TLM_TYPES> initiator_socket;
  tlm_utils::simple_initiator_socket<cpu_busaccess, 64, PW_TLM_TYPES> *ifetch_socket;

  bool tracing;
  volatile bool halted;
  void reset(bool enable_first_to_run);
  u64_t lastfetch; // A 1-place trace buffer.

 protected:
  void  doInitiatorTrans(PRAZOR_GP_T &trans,sc_time& delay);

  sc_signal <u64_t> *ea_mon;
 private:


#ifdef TLM_POWER3
  void energy_event(pw_energy, sc_time);
 public:
  int local_energy();
#endif
 public:
  void trace_msg(const char *);
  void start_trace(const char *name=0);

  uint8_t procID;
  void run();
  // The following I/O functions are pure virtual in the iss core, and must be provided here:
  void                corepause(int us);  // Pause CPU for this time interval
  bool             instr_fetch64 (addr_t memaddr, u64_t &rdata, breakpt_t *bp, lt_delay &delay);
  bool                eval_mem64 (addr_t memaddr, u64_t &rdata, breakpt_t *bp, lt_delay &delay, bool linked=false);
  bool                eval_mem32 (addr_t memaddr, u32_t &d, breakpt_t *bp, lt_delay &delay, bool linked=false);
  bool                eval_mem16 (addr_t memaddr, u16_t &d, breakpt_t *bp, lt_delay &delay, bool linked=false);
  bool                eval_mem8 (addr_t memaddr, u8_t &d, breakpt_t *bp, lt_delay &delay, bool linked=false);
  bool                set_mem64 (addr_t memaddr, u64_t value, breakpt_t *bp, lt_delay &delay, bool linked=false);
  bool                set_mem32 (addr_t memaddr, u32_t value, breakpt_t *bp, lt_delay &delay);
  bool                set_mem16 (addr_t memaddr, u16_t value, breakpt_t *bp, lt_delay &delay);
  bool                set_mem8 (addr_t memaddr, u8_t value, breakpt_t *bp, lt_delay &delay);
  int                eval_xchg(addr_t memaddr, u64_t wdata, u64_t &rdata, int width, lt_delay &delay, bool tf=false);
  void                atomic_prefix(); // Prefix following load/store pair as atomic. - watch out for ifetch in between!
  void sim_done(const char *msg);

  void reset() 
  {
    atomics = 0;
  }

  int cpu_mem_store_conditional(addr_t memaddr, u64_t value, int width, lt_delay &delay);
  bool mem_read(u64_t addr, u64_t &d, lt_delay &delay, bool linked=false, int skt=0, bool tf=false, int not64=3/*64 bits*/, int size=4 /*in bytes*/); 
  bool mem_write(u64_t addr, u8_t mask, u64_t wdata, lt_delay &delay);

  void qk_sync();
  bool qk_need_sync();

  void trace100(const char *msg);

  struct tracebuffer_s
  {
    int in_ptr;
    int hwm;
    u64_t buffer[TRACEBUFFER_LENGTH];
    tracebuffer_s() // constructor
      {
	in_ptr = 0;
	hwm = -1;
      }
    void report_trace(FILE *fd, const char *names);
    void inc()
    {
      if (in_ptr > hwm) hwm = in_ptr;
      in_ptr = in_ptr + 1;
      if (in_ptr >= TRACEBUFFER_LENGTH) in_ptr = 0;
    }

  } tracebuffer;


  void diagnostic_report(FILE *fd, int severity, const char *msg);

};




#endif

// eof
