// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) M Puzovic & DJ Greaves - University of Cambridge Computer Laboratory 2011.
// $Id: $

#ifndef SECONDARY_CACHE_WITH_DIRECTORY_H
#define SECONDARY_CACHE_WITH_DIRECTORY_H

#include "tlm.h"
#include "tlm_utils/multi_passthrough_target_socket.h"
#include "tlm_utils/multi_passthrough_initiator_socket.h"
#include "tenos.h"
#include "memories_extensions.h"

#ifdef TLM_POWER3
#include "tlm_power.h"
#include "pw_tlm_payload.h"
#define POWER3(X) X
#else
#define POWER3(X)
#endif

#define MAX_LINESIZE 1024

// Extension to store number of way
struct way_extension : tlm::tlm_extension<way_extension>
{
  int way;

  // constructor
  way_extension() {
    way = -1;
  }

  // Must implement clone and copy_from in all extensions
  tlm_extension_base* clone() const {
    way_extension* ext = new way_extension;
    ext->way = this->way;
    
    return ext;
  }

  void copy_from(tlm_extension_base const &ext) {
    way = static_cast<way_extension const &>(ext).way;
  }
};

// Memory manager
class way_mm_t : public tlm::tlm_mm_interface
{
 private:
  PW_TLM_PAYTYPE *freelist;

 public:
  way_mm_t () { // constructor
    freelist = 0;
  }

  PW_TLM_PAYTYPE* allocate() {
    PW_TLM_PAYTYPE *r;
    if (freelist) {
      r = freelist;
      freelist = 0;
    }
    else {
      r = new (PW_TLM_PAYTYPE)(this);
    }
    
    way_extension *ext;
    r->get_extension(ext);
    if (!ext) {
      ext = new way_extension;
      r->set_extension(ext);
    }
    return r;
  }

  void free(tlm::tlm_generic_payload* r) {
    assert(!freelist);
    freelist = (PW_TLM_PAYTYPE*)r;
  }

#ifdef TLM_POWER3
#if PW_TLM_PAYLOAD > 0 
  void free(PW_TLM_PAYTYPE* r) {
    assert(!freelist);
    freelist = r;
  }
#endif
#endif

};


class secondary_cache_with_directory:
  public sc_module
#ifdef TLM_POWER3
  , public pw_module
#endif
{

  POWER3(pw_energy m_running_op_energy;)
  POWER3(void log_energy_use(pw_energy p) { m_running_op_energy  += p; })

  class directory_ty:
    public sc_module
    #ifdef TLM_POWER3
    , public pw_module
    #endif
 {
   int cores;
   int cols;
   int rows;
   int entry_size_bits;


   typedef u64_t** panel;

   panel** Data;
  protected:
   friend class secondary_cache_with_directory;

   panel read(int row, int col);
   // constructor
   directory_ty(sc_core::sc_module_name name_, int cores, int ways, int rows, int cols, int entry_size);
 };


  // This 'smallram' has a simple, non-TLM model, which includes power but not delay or contention.
  // Delay and contention arising from this delay should be modelled by the instantiator.
  class smallram64: 
  public sc_module
    #ifdef TLM_POWER3
    , public pw_module
    #endif
  {
    int words, width;
    secondary_cache_with_directory* parent;
    void recompute_ptv_parameters();
    #ifdef TLM_POWER3
    pw_energy m_write_energy_op;
    pw_energy m_read_energy_op;
    #endif

    sc_time m_sr_latency;
    u64_t *Data;

  protected:
    friend class secondary_cache_with_directory;

    u64_t operator[] (u64_t idx) { return read(idx); }
    u64_t read(u64_t idx);
    void write(u64_t idx, u64_t d);
    // constructor
    smallram64(sc_core::sc_module_name name_, int words, int width, secondary_cache_with_directory* parent);
  };

  class smallram8: 
  public sc_module
    #ifdef TLM_POWER3
    , public pw_module
    #endif
  {
    int words, width;
    secondary_cache_with_directory* parent;
    void recompute_ptv_parameters();
    POWER3(pw_energy m_write_energy_op);
    POWER3(pw_energy m_read_energy_op);
    sc_time m_sr_latency;
    u8_t *Data;
  protected:
    friend class secondary_cache_with_directory;

    u8_t operator[] (u64_t idx) { return read(idx); }
    u8_t read(u64_t idx);
    u8_t *read8p(u64_t idx); // Reference to a word view of internal data.
    u64_t read64(u64_t idx); // Read a word 
    void write(u64_t idx, int blen, u8_t *d, u8_t *lanes);
    // constructor
    smallram8(sc_core::sc_module_name name_, int words, int width,  secondary_cache_with_directory* parent);
  };

 directory_ty* directory;

 protected:
 POWER3(PW_TLM3(sc_pwr::tlm_bit_transition_tracker primary_bus_tracker)); // These three used if counting wire transitions. 
 POWER3(PW_TLM3(sc_pwr::tlm_bit_transition_tracker secondary_bus_tracker)); 
 POWER3(PW_TLM3(sc_pwr::tlm_bit_transition_tracker cache_to_processor_xbus));


  // cache geometry
  struct {
    int bytes;                     // Total capacity for data in bytes
    int ways;                      // Number of ways in a cache
    int linesize;                  // Size of a cache block in bytes;
    int dmapping;
    int loffset_shift;
    int secondary_blocksize_bytes;
    int secondary_bl;
    int banks;
    int word64s_per_line;
    int dmap_shift;
    int directory_panel_row_map;
    int directory_panel_col_map;
    int directory_panel_row_shift;
    int directory_panel_col_shift;
    int primary_tag_shift;
    int idx;
    int primary_ways;
  } geom;

  // geometry for instruction cache directory
  struct {
    int mask1; // mask used for cpu id
    
    int lshift2; 
    int mask2; // mask used for replacement way

    int rshift3;
    int lshift3;
    int mask3; // mask used for line

    int rshift4;
    int lshift4;
    int mask4; // mask used for index
  } idir_geom;

  class cacheway
  {
    secondary_cache_with_directory* parent;
    typedef enum { invalid, clean, dirty} state_t;
    state_t *Status;
    smallram8* Data; // Having one data and tag RAM per cacheway is not right really.
    smallram64* Tags;
    int m_way;
  protected:
    friend class secondary_cache_with_directory;

    cacheway(secondary_cache_with_directory* parent, int way);//constructor
    bool lookup(u64_t addr, int dmap, u8_t ** cline, sc_time& max_delay);
    void clear(sc_time& delay, int dmap, u64_t addr);
    void insert(u64_t addr, int dmap, u8_t* cline, sc_time& delay, u64_t full_addr);
  };

  sc_mutex ml;
  bool busy_flag;
  std::deque<int> requests;
  sc_event wake_event;

 public:
  struct stats_t
  {
    int hits, misses;
    int sharing_evictions;
    int capacity_misses; // only in use if eviction history is maintained
    int writes;
    // compulsorary misses given by misses minus other types of miss.
    stats_t() { reset(); }
    void reset()
    {
      hits = 0;
      misses = 0;
      sharing_evictions = 0;
      capacity_misses = 0;
      writes = 0;
    }
  } stats;

  void build_instruction_directory(int primary_instr_bytes,
				   int primary_instr_linesize,
				   int primary_instr_ways,
				   int core_num);
  u64_t calculate_idir_index(u64_t line_addr, u64_t way, u64_t cpuid);


  // Constructor
  secondary_cache_with_directory(sc_module_name name,
				 int id,
				 int bytes,
				 int linesize,
				 int ways,
				 int secondary_width,
				 int secondary_bl,
				 int banks,
				 int primary_bytes,
				 int primary_linesize,
				 int primary_ways,
				 int primary_instr_bytes,
				 int primary_instr_linesize,
				 int primary_instr_ways,
				 int corenum);

  int lru;
  cacheway **Cont;

  tlm_utils::multi_passthrough_target_socket<secondary_cache_with_directory, 64, PW_TLM_TYPES> targ_socket;
  tlm_utils::multi_passthrough_initiator_socket<secondary_cache_with_directory, 64, PW_TLM_TYPES> inita_socket;

  cacheway* lookup(bool service, 
		   u64_t line_addr, 
		   int dmap, 
		   int loffset, 
		   u64_t* &datap1, 
		   sc_time& delay,
		   u64_t full_addr);
  int main_memory_lookup(u64_t line_addr, u8_t *cline, sc_time& delay);
  void b_transport(int id, PW_TLM_PAYTYPE &trans, sc_time &delay);
  void clean(sc_time &delay);

  void invalidate_line_in_primary(int core, int w, int addr, sc_time &delay);

 private:
  u64_t get_dir_entry_content(int dmap, int way);
  way_mm_t way_mm;
#ifdef TLM_POWER3
    pw_energy std_energy_op;
#endif

};

    // eof
#endif
