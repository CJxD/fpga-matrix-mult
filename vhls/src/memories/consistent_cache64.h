// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2011-14 D J Greaves + M Puzovic
// (C) 2009-11 D J Greaves
// University of Cambridge, Computer Laboratory.
// VHLS/PRAZOR TLM2.0 Simulator

/*
consistent_cache64   A parameterisable set-associative cache with two write strategies and a line buffer
                     to accumulate partially dirty words. Snooping on other caches within a consistent
                     group is enabled by passing each cache an instance of a consisent\_group object.
                     Each way consists of a pair of instances of sram64 to contain data and tags.

*/

#if 0

L2 cache on that platform is 4MB 16-way associative with 64 byte line
size and it is write-back (also it is L1 inclusive). On the other hand
L1 data cache is write-through. Size of L1 D-cache is 8KB (they have one per CPU core and on shared I-cache whose size is 16KB). The hit latency
of L2 is 20 cycles.
#endif

#ifndef CONSISTENT_CACHE64_H
#define CONSISTENT_CACHE64_H

#include "tlm.h"
#include "tlm_utils/multi_passthrough_initiator_socket.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/multi_passthrough_target_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/peq_with_cb_and_phase.h"
#include "tenos.h"
#include "memories_extensions.h"

#include <queue>
#include <utility>
#include <set>
#include <list>

#include <stdio.h>
#include <math.h>

#include <boost/parameter/name.hpp>

#ifdef TLM_POWER3
#include "tlm_power.h"
using namespace sc_pwr;
#define POWER3(X) X
#else
typedef tlm::tlm_base_protocol_types PW_TLM_TYPES;
typedef tlm::tlm_generic_payload PW_TLM_PAYTYPE;
typedef int pw_customer_acct; /* use a simple int as a placeholder */
#define PW_TLM3(X)
#define POWER3(X)
#endif


#define WBTRC(X)\
{\
  if(parent->traceregions && parent->traceregions->check(m_addr, TENOS_TRACE_WRITE_BUFFER_OP)) { \
    cout << sc_time_stamp() << "-";					\
    printf("tid=%d-%s (0x%lx): ", sc_get_current_process_b()->proc_id, parent->name(), m_addr);\
    X;\
  }\
}

#define CTRC(ADDR, X)\
{\
  if(traceregions && traceregions->check(ADDR, TENOS_TRACE_MEM_WRITE|TENOS_TRACE_MEM_READ)) {\
    cout << sc_time_stamp() << " - ";\
    printf("tid=%d-%s (0x%lx): ", sc_get_current_process_b()->proc_id, name(), ADDR);\
    X;\
  }\
}

#define CPTRC(ADDR, X)\
{\
  if(parent->traceregions && parent->traceregions->check(ADDR, TENOS_TRACE_MEM_WRITE|TENOS_TRACE_MEM_READ)) { \
    cout << sc_time_stamp() << " - ";\
    printf("tid=%d-%s (0x%lx): ", sc_get_current_process_b()->proc_id, parent->name(), ADDR);\
    X;\
  }\
}

#define CTRC_EMPTY(X)


#define MAX_TARGETS 64

#define MAX_LINESIZE 1024


// consistency protocol
typedef enum cc_protocols {
  MESI_CC,
  MOESI_CC
} cc_protocol_ty;




// forward declaration
class consistent_cache64;

class cache64:
 public sc_module
#ifdef TLM_POWER3
 ,  public pw_module
#endif
{
  POWER3(pw_energy m_running_op_energy;)
  POWER3(void log_energy_use(pw_energy p) { m_running_op_energy  += p; })


 protected:

  POWER3(PW_TLM3(sc_pwr::tlm_bit_transition_tracker primary_bus_tracker)); // Used if counting wire transitions. 
  POWER3(PW_TLM3(sc_pwr::tlm_bit_transition_tracker secondary_bus_tracker)); // Used if counting wire transitions. 

  // This 'smallram' has a simple, non-TLM model, which includes power but not delay or contention.
  // Delay and contention arising from this delay should be modelled by the instantiator.
  class smallram64: 
  public sc_module
    #ifdef TLM_POWER3
    , public pw_module
    #endif
  {
    int words, width;
    cache64 *parent;
    void recompute_pvt_parameters();
    POWER3(pw_energy m_write_energy_op);
    POWER3(pw_energy m_read_energy_op);
    sc_time m_sr_latency;
    u64_t *Data;


  protected:
    friend class cache64;
    friend class hammer_cache64;

    u64_t operator[] (u64_t idx) { return read(idx); }
    u64_t read(u64_t idx);
    void write(u64_t idx, u64_t d);
    // constructor
    smallram64(sc_core::sc_module_name name_, int words, int width, cache64 *parent);

  public:
    void set_sr_latency(int words, int width, double factor);
  };

  class smallram8: 
  public sc_module
    #ifdef TLM_POWER3
    , public pw_module
    #endif
  {
    int words, width;
    cache64 *parent;
    void recompute_pvt_parameters();
    POWER3(pw_energy m_write_energy_op);
    POWER3(pw_energy m_read_energy_op);
    sc_time m_sr_latency;
    u8_t *Data;

  protected:
    friend class cache64;
    friend class consistent_cache64;
    friend class hammer_cache64;

    u8_t operator[] (u64_t idx) { return read(idx); }
    u8_t read(u64_t idx);
    u8_t *read8p(u64_t idx); // Reference to a word view of internal data.
    u64_t read64(u64_t idx); // Read a word 
    void write(u64_t idx, int blen, u8_t *d, u8_t *lanes);
    void print_content();
    void print_content(u64_t idx, u64_t length);

    // constructor
    smallram8(sc_core::sc_module_name name_, int words, int width, cache64 *parent);

  public:
    void set_sr_latency(int words, int width, double factor);
  };

  class write_buffer
  {
    u64_t m_addr;
    int m_dmap;
    sc_mutex wbl;

    // this is set to address that is being evicted from WB at the moment
    // it is set to -1 if address is still live
    u64_t evict_addr;

  protected:
    friend class cache64;
    friend class consistent_cache64;
    friend class hammer_cache64;
    cache64 * parent;
    int linesize, secondary_width_bytes;
    // constructor
    write_buffer(cache64 * parent, int linesize, int secondary_width);
    smallram8 cline;
    bool Dirty[MAX_LINESIZE];
    void clean(sc_time &delay, cache_miss_extension* e);
    void mark_dirty(int len, int loffset, u8_t *lanes, int bel);
    u64_t *hit(u64_t addr, int loffset);
    bool word_present(int);
    u64_t *init(u64_t, int off, int dmap, cache_miss_extension* e, sc_time& delay);
    sc_time data_latency(int len_in_bytes);
  };

  class cacheway
  {
  protected:
    typedef enum { 
      invalid, 
      exclusive, 
      modified, 
      shared,
      owned
    } mesi_t;

  private:
    mesi_t *Status;
    cache64 *parent;
    smallram8* Data; // Having one data and tag RAM per cacheway is not right really.
    // Regardless of modelled-system endianness, this data is stored modelling workstation endian so that cast to (u64_t *) does not require a byte swap.
    smallram64* Tags;
    int m_way;
    sc_mutex *waylock;

  protected:
    friend class cache64;
    friend class consistent_cache64;
    friend class hammer_cache64;
    bool insert(u64_t addr, 
		int dmap, 
		u8_t *data, 
		sc_time &delay, 
		cache_miss_extension* cme, 
		mesi_t ns);
    
    cacheway(cache64 *parent, int way);//constructor

    void clean(sc_time &delay, int dmap, cache_miss_extension* cme, bool force = false);
    mesi_t operate(cacheway::mesi_t op, u64_t addr, int dmap, bool &changed);    
    bool lookup(u64_t addr, int dmap, u8_t **clinep, sc_time &max_delay);
    void print(FILE *fd, int dmap);
  };

  cacheway *lookup(bool servicef, 
		   u64_t addr, 
		   int dmap, 
		   int loffset, 
		   u64_t * &data, 
		   sc_time &delay, 
		   cache_miss_extension* cme, 
		   pw_customer_acct *customer_id,
		   bool nested=false,
		   int second_trip=0
		   );
  
  int secondary_lookup(u64_t line_addr, 
		       u8_t *cline, 
		       sc_time &delay, 
		       int way,
		       cache_miss_extension* cme,
		       pw_customer_acct *customer_id);

 protected:
  virtual int do_a_snoop(bool, bool, u64_t, sc_time&, 
			 cache_miss_extension*, 				   
			 pw_customer_acct *customer_acct,
			 u8_t* data) { 
    return 0; 
  }

  virtual bool insert_line(cacheway* cw,
			   u64_t addr, 
			   int dmap, 
			   u8_t *data, 
			   sc_time &delay, 
			   cache_miss_extension* cme, 
			   cacheway::mesi_t ns) {
    return cw->insert(addr, dmap, data, delay, cme, cacheway::mesi_t::shared);
  }

  virtual void line_invalidation(u64_t addr, 
				 int loffset, 
				 int dmap, 
				 cache_miss_extension* e, 
				 sc_time& d) {
    // do nothing as we do not have consistent groups in the basic cache: use consistent_cache for that behaviour.
  }

  virtual void evict_lower_level_line(u64_t addr, size_t linesize, u8_t* data) {
    // do nothing
  }

  virtual std::pair<u8_t*, u8_t*> check_lower_level_lines(u64_t line, 
							  size_t linesize,
							  cacheway::mesi_t ns) {
    return std::pair<u8_t*, u8_t*>((u8_t*)NULL, (u8_t*)NULL);
  }

  virtual void operate_miss_type(cacheway*, int, cache_miss_extension*) {
    // do nothing
  }

  virtual tlm::tlm_sync_enum nb_transport_fw(int id,
					     PW_TLM_PAYTYPE& trans,
					     tlm::tlm_phase& ph,
					     sc_time& delay) {
    // do nothing
    return tlm::TLM_ACCEPTED;
  }

  virtual bool do_i_lookup(u64_t addr) {
      return true;
  }

    
 public:
 

  struct cache_geom // cache geometry
  {
    int bytes;       // Total capacity for data in bytes
    int ways;        // N ways of associativity.
    int linesize;    // Size of a cache block in bytes (always a multiple of 8).
    int dmapping;    // Directly-mapped arity.
    
    int word64s_per_line; // Words in a cache line (linesize/8).
    int loffset_shift;    // Always 3 for a 64 bit system.

    int secondary_bl;
    int dmap_shift;
    int secondary_blocksize_bytes;
    int secondary_linesize_bits;
    int secondary_banks;
    bool big_endian_;//Why does the cache need to know the endiannes?  There could be several cores connected to one cache of different endiannesses?  SystemC takes the approach that memory subsystems always work in the endianness of the modelling workstation.
  } geom;

  //typedef struct geom cache_geom;

  enum ON_MISS {
    READ_ALLOCATE,
    NO_READ_ALLOCATE,
    WRITE_ALLOCATE,
    NO_WRITE_ALLOCATE
  };

  enum WRITE_POLICY {
    WRITE_THROUGH,
    WRITE_BACK
  };

  typedef enum ON_MISS on_miss_ty;
  typedef enum WRITE_POLICY write_policy_ty;

  struct stats_t
  {
    int hits, misses;
    int sharing_evictions;
    int capacity_misses; // only in use if eviction history is maintained
    int snooped_reads;
    int writes;
    int reads;
    // compulsorary misses given by misses minus other types of miss.
    stats_t() { reset(); }

    void reset()
    {
      snooped_reads = 0;
      hits = 0;
      misses = 0;
      sharing_evictions = 0;
      capacity_misses = 0;
      writes = 0;
      reads = 0;
    }

    void totals(FILE* fd);

  } stats;

  void clean(sc_time &delay);
  virtual void operate(cacheway *cw, cacheway::mesi_t op, u64_t line_addr, int dmap, sc_time &delay);
  // Use tagged sockets to be able to distinguish incoming backward path calls
  tlm_utils::multi_passthrough_target_socket<cache64, 64, PW_TLM_TYPES>  targ_socket;
  tlm_utils::multi_passthrough_initiator_socket<cache64, 64, PW_TLM_TYPES> inita_socket;

  int lru;         // Random eviction pointer
  // Line that is being requested from secondary storage
  volatile u64_t secondary_storage_lreq;

  on_miss_ty store_miss;
  on_miss_ty write_miss;
  write_policy_ty write_policy;
  cache_ty my_type;
  uint32_t hit_cycle_time;
  int level; // level of the cache in hieararchy

  bool use_write_buffer;
  write_buffer* buf0;


  traceregion *traceregions;
  // Constructor
  cache64(sc_module_name name, 
	  int bytes, 
	  int ways, 
	  int linesize, 
	  int secondary_width, 
	  sc_time clock_period,
	  int cache_level, 
	  cache_ty ty,
	  uint32_t hit_cycles = 1,
	  int bl = 1,
	  on_miss_ty store_miss = READ_ALLOCATE,
	  on_miss_ty write_miss = WRITE_ALLOCATE,
	  write_policy_ty write_policy = WRITE_BACK,
	  bool use_write_buffer = true,
	  int secondary_linesize = -1,
	  int secondary_banks = 1,
	  cc_protocol_ty protocol = MESI_CC,
	  bool big_endian = true);

  sc_time clock_period;
  cacheway **Cont;

  // FORWARD PATH
  // TLM-2 blocking transport method
  bool get_direct_mem_ptr(int n, PW_TLM_PAYTYPE&, tlm::tlm_dmi& dmi_data);
  void b_transport(int id, PW_TLM_PAYTYPE &trans, sc_time &delay);

  void b_access(int id, PW_TLM_PAYTYPE &trans, sc_time &delay, u64_t addr);
   
  void end_of_simulation();
  void stat_report(const char *, FILE *, bool resetf = false, bool no_header = true);

  void stat_header(const char *, FILE *);
  void stat_accumulate(stats_t& t);

  const char *kind();  

  void set_sr_latencies(int tag_words, int tag_width, 
			int data_words, int data_width,
			double factor);

 private:
#ifdef TLM_POWER3
  pw_energy std_energy_op;
#endif
  void write_back(PW_TLM_PAYTYPE &trans, 
		  sc_time &delay,
		  u64_t line_addr,
		  int loffset,
		  u64_t addr,
		  u32_t len,
		  int dmap,
		  u8_t* ptr,
		  cache_miss_extension* cme);

  void write_through(int id,
		     PW_TLM_PAYTYPE &trans, 
		     sc_time &delay,
		     u64_t addr);

  sc_mutex ml;
  sc_event secondary_req_event;
  std::queue<std::pair<int, int> > invq;
  std::set<u64_t> secondary_reqs;
  std::set<u64_t> invalid_reqs;

 protected:
  cache_miss_mm_t cache_miss_mm;

  // if the first element of the pair is 0, then it is a request
  // from the lower-level cache, while if it is 1 then it is a request
  // from the SRI module
  volatile bool busyFlag;
  cacheway *busy_way;

  u64_t busyAddr;
  sc_event busyWait;
  std::queue<int> requests;

  cc_protocol_ty cc_protocol;
};

// For consistent caches, first make one of these, giving it a size, and then add caches to it.
// For inconsistent (standalone) caches, do not pass them a grp pointer.
class consistent_group 
{

 private:  
  int target_members;
  int current_members;
  class consistent_cache64 **Members;

 public:
 consistent_group(int members)
   : target_members(members) {
    assert(members >= 1);
    Members = (class consistent_cache64 **) malloc(members * sizeof(consistent_cache64 *));
    current_members = 0;
  }

 protected:
  friend class consistent_cache64;
  friend class hammer_cache64;

  void add(class consistent_cache64 *item) {
    assert(current_members < target_members);
    //printf("Added member %i/%i\n", current_members, target_members);
    Members[current_members++] = item;
  }
};

class consistent_cache64 : public cache64 {
 protected:
  int do_a_snoop(bool, bool, u64_t, sc_time&, cache_miss_extension*, 
		 pw_customer_acct *customer_acct,
		 u8_t* data);
  consistent_group *grp;
  friend class hammer_cache64;
  
 public:
  consistent_cache64(sc_module_name name,  // constructor
		     consistent_group* grp,
		     int bytes, 
		     int ways, 
		     int linesize, 
		     int secondary_width, 		   
		     int cache_level, 
		     cache_ty ty,
		     cc_protocol_ty protocol = MESI_CC,
		     sc_time clock_period = sc_time(5, SC_NS),
		     uint32_t hit_cycles = 1,
		     int bl = 1,
		     on_miss_ty store_miss = READ_ALLOCATE,
		     on_miss_ty write_miss = WRITE_ALLOCATE,
		     write_policy_ty write_policy = WRITE_BACK,
		     bool use_write_buffer = true,
		     int secondary_linesize = -1,
		     int secondary_banks = 1,
		     bool big_endian = true// ??
		     );

  void operate(cacheway *cw, 
	       cacheway::mesi_t op, 
	       u64_t line_addr, 
	       int dmap, 
	       sc_time &delay);


  bool grp_snoop(u64_t addr, 
		 bool exclude, 
		 sc_time &delay, 
		 cache_miss_extension* cme,
		 pw_customer_acct *customer_acct,
		 cacheway::mesi_t& state,
		 u8_t* data);

  bool insert_line(cacheway* cw,
		   u64_t addr, 
		   int dmap, 
		   u8_t *data, 
		   sc_time &delay, 
		   cache_miss_extension* cme, 
		   cacheway::mesi_t ns);

  void line_invalidation(u64_t addr, 
			 int loffset, 
			 int dmap, 
			 cache_miss_extension* e, 
			 sc_time& d);

};

#endif
// eof
