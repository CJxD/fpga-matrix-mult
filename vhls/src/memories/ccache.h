// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2014 M Puzovic + D J Greaves

#ifndef __CCACHE_H__
#define __CCACHE_H__

#include "tenos.h"
#include "tlm.h"
#include "tlm_utils/multi_passthrough_initiator_socket.h"
#include "tlm_utils/multi_passthrough_target_socket.h"
#include "tlm_utils/peq_with_cb_and_phase.h"
#include "smallram.h"

#include "llsc_extension.h"

#include <atomic>
#include <set>

/* Enabling this directive will collect type of
   cache misses. At the moment it has high memory 
   consumption because we put to set every line address
   that was access by this cache.

   TODO: instead of saving line address we should consider
   saving the _range_ of addresses accessed, due to locality of
   reference this should reduce memory footprint */
//#define CMISSTY 1

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

#define CTRC(ADDR, X)\
{\
  if(traceregions && traceregions->check(ADDR, TENOS_TRACE_MEM_WRITE | TENOS_TRACE_MEM_READ)) { \
    cout << sc_time_stamp() << " - ";\
    printf("tid=%d-%s (0x%lx): ", sc_get_current_process_b()->proc_id, name(), ADDR);\
    X;\
  }\
}

#define CWTRC(ADDR, X)\
{\
  if(m_parent->traceregions && m_parent->traceregions->check(ADDR, TENOS_TRACE_MEM_WRITE | TENOS_TRACE_MEM_READ)) {\
    cout << sc_time_stamp() << " - ";\
    printf("tid=%d-%s (0x%lx): ", sc_get_current_process_b()->proc_id, m_parent->name(), ADDR);\
    X;\
  }\
}

#define CCACHE_SCU_BIND(CACHE, SCU, SNOOPED)          \
    CACHE->initiator_socket.bind(SCU->target_socket); \
    SCU->snooped_target(SNOOPED);

class ccache_state_extension;

class ccache_state_mm_t : public tlm::tlm_mm_interface
{

 public:
  // constructor
  ccache_state_mm_t()
    : free_list(0), empties(0), lck("cache_mm") {}

  PW_TLM_PAYTYPE* allocate();
  void free(PW_TLM_PAYTYPE* r);

#ifdef TLM_POWER3
#if PW_TLM_PAYLOAD > 0 
  void free(tlm::tlm_generic_payload* trans) 
  {
    free((PW_TLM_PAYTYPE*)trans);
  }
#endif
#endif

 private:
  struct access {
    PW_TLM_PAYTYPE* trans;
    access* next;
    access* prev;
  };

  access* free_list;
  access* empties;

  sc_mutex lck;
};

class ccache : public smallramp, public sc_module
#ifdef TLM_POWER3
  , public pw_module
#endif
{
  public:

    // ACE cache states
    typedef enum state {
        // cache line does not exist in this cache
        Invalid = 0,
        
        // cache line only exists in this cache
        // cache line is not modified with respect to main memory
        UniqueClean,
        
        // cache line only exists in this cache
        // cache line is modified with respect to main memory
        UniqueDirty,
        
        // cache line might be shared with another caches
        // it is not known if this cache line has been modified
        // with respect to main memory, but if it is this cache
        // is not responsible with updating main memory
        SharedClean,
        
        // cache line might be shared with another caches
        // cache line is modified with respect to main memory
        // and it is responsibility of this cache to update
        // main memory when needed
        SharedDirty
    } state_t;

    // ACE transactions
    typedef enum transaction {
        // Non-shared transactions
        Read,
        Write,

        // Shareable read
        ReadShared,

        // Shareable write
        ReadUnique,
        MakeUnique,

        // Cache maintance
        MakeInvalid,

        // Write-Back
        WriteBack,
        Evict,

        // Unknown transaction
        Unknown
    } transaction_t;
    
    // the effective memory address is split into
    /// MSB                                     LSB
    // +---------------+-----------+--------------+
    // |      tag      |   index   | block offset |
    // +---------------+-----------+--------------+

    // cache geometry
    typedef struct geometry {
        u32_t addrsize; // size of address in bytes
        u32_t size; // total size of cache in bytes
        u32_t linesize; // size of cache line in bytes
        u8_t ways; // number of ways in a cache
        
        // the following information is for
        // ease of access to cache lines from
        // internal data structures; they are
        // pre-calculated for efficiency
        // BEGIN

        // mask used to find index of byte 
        // in a cacheline (cache is byte addressable)
        // TODO: consider whether we should allow
        // cache to have different addressing modes
        u32_t boffsetm; 
        // number of bits in boffset
        u8_t boffsetb;
        // mask used to find index of a cache
        // line in cache way
        u32_t indexm; 
        // number of bits in index
        u8_t indexb;
        // mask used to find a tag from the
        // cache address
        u64_t tagm;

        // END
    } geometry_t;

    // statistics
    typedef struct stats {
      public:
        std::atomic<u64_t> read_hits;
        std::atomic<u32_t> read_miss;
        std::atomic<u32_t> write_hits;
        std::atomic<u32_t> write_miss;
        std::atomic<u32_t> reads;
        std::atomic<u32_t> writes;
        std::atomic<u32_t> llsc_reads;
        std::atomic<u32_t> llsc_writes;
        std::atomic<u32_t> conflicts_evictions;
        std::atomic<u32_t> read_contentions;
        std::atomic<u32_t> write_contentions;
        std::atomic<u32_t> snoop_contentions;
        std::atomic<u32_t> snoop_evictions;
        std::atomic<u32_t> snoop_reads;
        std::atomic<u32_t> snoop_hits;
        std::atomic<u32_t> snoop_miss;
#ifdef CMISSTY
        std::atomic<u32_t> compulsory;
        std::atomic<u32_t> capacity;
        std::atomic<u32_t> conflict;
        // all cache lines that have ever been in the cache
        std::set<u32_t> alladdrs; 
        // cache lines that has been removed because of conflict
        std::set<u32_t> caddrs;
#endif
        
        stats() { 
            reset(); 
        }

        void reset() {
            read_hits.store(0);
            read_miss.store(0);
            write_hits.store(0);
            write_miss.store(0);
            reads.store(0);
            writes.store(0);
            llsc_reads.store(0);
            llsc_writes.store(0);
            conflicts_evictions.store(0);
            read_contentions.store(0);
            write_contentions.store(0);
            snoop_contentions.store(0);
            snoop_evictions.store(0);
            snoop_reads.store(0);
            snoop_hits.store(0);
            snoop_miss.store(0);
#ifdef CMISSTY
            compulsory.store(0);
            capacity.store(0);
            conflict.store(0);
            alladdrs.clear();
#endif
        }
        
    } ccache_stats_t;

    void stat_report(const char *msg, FILE *fd, bool reset);

#if TRACECOMM
    void comm_stat_report(FILE *fd);
    u64_t hit_reads(int pid);
#endif    

    class way {
      public:
        // Constructor
        way(ccache* parent, u8_t way, u32_t columns);
        state_t lookup(
            u64_t addr,
            u64_t index, 
	    u64_t tag, 
	    u8_t** clinep, 
	    u64_t& ts, 
	    sc_time& delay);

        bool clean(
            u64_t addr,
            u64_t index,
            u64_t tag,
            state_t olds, 
            sc_time &delay,
	    int rsize);

	bool insert(
            u64_t addr, 
            u64_t index, 
	    u64_t tag, 
	    u8_t* clinep, 
            state_t news,
            state_t olds, 
	    sc_time& delay,
	    int rsize);

        bool releasets(u64_t index, u64_t oldts, u64_t newts);
        bool acquirets(u64_t &oldts, u64_t index);
        bool acquirets(u64_t &oldts, u64_t index, state_t newts);
	bool updatets(u64_t index, u64_t& oldts, u64_t newts);
        u64_t incts(u64_t oldts, state_t s);
        u64_t incts(u64_t oldts);
        u64_t rebuild_address(u64_t index);

        friend class ccache;
      
        // the status of the cache line is 
        // 64               3       1      0
        // +----------------+-------+------+
        // |   timestamp    | state | lock |
        // +----------------+-------+------+
        std::atomic<u64_t>* m_status;
        smallram<u8_t>* m_data;
        smallram<u64_t>* m_tags;

#if TRACECOMM	
	u32_t* m_owner;
#endif	

      private:
        ccache* m_parent;
        u8_t m_way;                
    };

#ifdef TLM_POWER3
    pw_energy m_running_op_energy;
    virtual void log_energy_use(pw_energy p) {
        m_running_op_energy += p;
    }
#endif
    
    ccache(
        sc_core::sc_module_name name,
	u32_t id,
	u32_t addrsize, 
	u32_t size, 
	u32_t linesize, 
	u8_t ways, 
	bool snooped);

    // TLM sockets
    tlm_utils::multi_passthrough_target_socket<ccache, 64, PW_TLM_TYPES>  target_socket;
    tlm_utils::multi_passthrough_initiator_socket<ccache, 64, PW_TLM_TYPES> initiator_socket;
    // this socket is only created if cache can be snooped
    tlm_utils::multi_passthrough_target_socket<ccache, 64, PW_TLM_TYPES>* snooped_socket;

    void b_transport(int id, PW_TLM_PAYTYPE &trans, sc_time &delay);
    tlm::tlm_sync_enum nb_transport_fw(
        int id,
        PW_TLM_PAYTYPE& trans,
        tlm::tlm_phase& ph,
        sc_time& delay);

    traceregion *traceregions;

    // disable cache
    void disable() {
        m_active = false;
    }

    // enable cache
    void enable() {
        m_active = true;
    }
    
    // query cache state
    bool is_active() {
        return m_active;
    }

  protected:
    POWER3(PW_TLM3(sc_pwr::tlm_bit_transition_tracker primary_bus_tracker)); // Used if counting wire transitions. 
    POWER3(PW_TLM3(sc_pwr::tlm_bit_transition_tracker secondary_bus_tracker)); // Used if counting wire transitions. 

    ccache_stats_t m_stats;

    bool invalidate();

    way* lookup(
        u64_t addr, 
        u64_t index, 
        u64_t tag, 
        u8_t** clinep, 
        u64_t& oldts, 
        state_t &clstate,
        sc_time& delay);

    // cache geometry
    geometry_t m_geometry;
    way** m_ways;

    virtual void foreign_write(u64_t addr) { };

  private:
    way* lruw();

    bool trans_to_secondary(
        ccache_state_extension* &cse,
        transaction type,
        u32_t length,
        u64_t addr,
        u8_t* &data,
        bool is_read,
        sc_time& delay,
	int rsize,
	int pid = -1);

    bool read(
        u64_t addr, 
	u64_t offset, 
	u64_t index, 
	u64_t tag, 
	u8_t* &destination,
	u32_t length, 
	ccache_state_extension* &cse,
	sc_time& delay,
	int size
#if TRACECOMM
	, int procid
#endif	
	);

    bool locked_op(
        u64_t addr, 
        u64_t offset, 
        u64_t index, 
        u64_t tag, 
        u8_t* &destination,
        u32_t length,
        ccache_state_extension* &cse, 
        llsc_extension* &linked,
        sc_time& delay,
	int size);

    bool write_locked(
        u64_t addr,
        u64_t offset, 
        u64_t index,
        u64_t tag,
        u8_t* data,
        u32_t dlength,
        u8_t* benable,
        u32_t blength,
        ccache_state_extension* &cse, 
        sc_time& delay);


    bool warm_write(
        way* cway,
        u64_t addr, 
        u64_t index,
        u64_t offset,
        u8_t* &data,
        u32_t dlength, 
        u8_t* benable,
        u32_t blength,
        u64_t& oldts,
        bool update);

    bool cold_write(
        u64_t addr,
        u64_t offset, 
        u64_t index,
        u64_t tag, 
        u8_t* data,
        u32_t dlength,
        u8_t* benable,
        ccache_state_extension* &cse, 
        sc_time& delay,
	int rsize);

    bool write(
        u64_t addr,
        u64_t offset, 
        u64_t index,
        u64_t tag,
        u8_t* data,
        u32_t dlength,
        u8_t* benable,
        u32_t blength,
        ccache_state_extension* &cse, 
        sc_time& delay,
	int size);

    bool insert(
        u64_t addr, 
        u64_t index,
        u64_t tag,
        u8_t* clinep,
        state_t news,
        u64_t& oldts, 
        way* &cway,
        sc_time& delay,
	int rsize);

    void secondary_lookup(
        u64_t addr,
	u8_t* &clinep,
        ccache_state_extension* &cse,
	sc_time& delay,
	int rsize,
	int pid = -1);

    void peq_cb(
	PW_TLM_PAYTYPE& trans, 
	const tlm::tlm_phase& ph);

    ccache_state_mm_t ccache_state_mm;
    u32_t lru;

    bool m_active;

    u32_t  myid;
    
#if TRACECOMM    
    u64_t* m_comm_reads;
    u64_t* m_hit_reads;
    u64_t m_total_reads;
#endif    

    tlm_utils::peq_with_cb_and_phase<ccache, PW_TLM_TYPES> m_peq;


#ifdef TLM_POWER3
    pw_energy std_energy_op;
#endif

};

struct ccache_state_extension : tlm::tlm_extension<ccache_state_extension> {
  public:
    u32_t origin;
    ccache::state_t state;
    ccache::transaction_t trans;

    ccache_state_extension() {
        origin = -1;
        state = ccache::Invalid;
        trans = ccache::Unknown;
    }
    
    tlm_extension_base* clone() const {
        ccache_state_extension* ext = new ccache_state_extension;
        ext->origin = this->origin;
        ext->state = this->state;
        ext->trans = this->trans;
      
        return ext;
    }

    void copy_from(tlm_extension_base const &ext) {
        origin = static_cast<ccache_state_extension const&>(ext).origin;
        state = static_cast<ccache_state_extension const&>(ext).state;
        trans = static_cast<ccache_state_extension const&>(ext).trans;
  }
};

#endif
