#ifndef GENERIC_TLM_MEM_H
#define GENERIC_TLM_MEM_H

// $Id: $
//
// sram64 : A TLM static ram with with ELF and Intel Hex load facility.
// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2009-10 DJ Greaves (TLM 2.0 Version Arturs Prieditis). 
// (C) 2009-12 DJ Greaves
// University of Cambridge, Computer Laboratory.



// This SRAM model is not synchronous (TLM modelling commonly neglects the clock), although modern SRAMs are invariably so, with a fully-pipelined access latency of one or two clock cycles, depending on size.
// Strictly modelling synchronous SRAM is not appropriate for LT-TLM models, but the combined latency figure attributed to the bus and RAM should be the clock period for single issue operation (and half as much for dual issue if such a concept exists, etc).

#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"
#include "prazor.h"
#include "tenos.h"
#include "llsc_extension.h"
//#include "support/mamba_extension.h"
#include "memloaders.h"

#include "tlm_utils/peq_with_cb_and_phase.h"

#define HTRC(X) 

#include <unordered_map>


// Load-locked/store-conditional.
#define LLSC_ENABLE 2

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

// One presence bit every 4 bytes?
#define PBITS_PER_BYTE 4


class generic_tlm_mem: 
  public sc_module,
#ifdef TLM_POWER3
  public pw_module,
#endif
  public memloader_if
{
  tlm::tlm_endianness m_big_endian;

  POWER3(PW_TLM3(sc_pwr::tlm_bit_transition_tracker read_bus_tracker)); // Used if counting wire transitions. 

  struct stats_t
  {
    u64_t read_ops, write_ops, read_words;
    stats_t() { reset(); }
    void reset()
    {
      read_words = 0;
      read_ops = 0;
      write_ops = 0;
    }
  } stats;

#define PAGESIZE (4096LLU * 8LLU) // 4K pages in bits

  // Internally we use pages to save VM on the modelling workstation
  // but these are not intended to represent anything physical.
  class internal_page
  {
  public:
    unsigned char data [PAGESIZE / 8LLU];

#ifdef USE_PRESENCE_BITS    
    bool presence_bits [PAGESIZE / 32LLU];
#endif
    u64_t base;
  internal_page(int x) : base(x)
      {
#ifdef USE_PRESENCE_BITS    
	//printf("Constructing page with presence bits\n");
	for (int i=0; i<PAGESIZE / 32LLU; i++) presence_bits[i] = false;
#else
	//printf("Constructing page without presence bits %x\n", x);
#endif
      }


  };


  internal_page *last_page;

  std::unordered_map<u64_t, internal_page *> memory_hash_map;

  bool ip_read(u64_t a, u8_t &d)
  {
    u64_t a0 = a & ~0xFFF;
    u32_t a1 = a & 0xFFF;
    internal_page *x = memory_hash_map[a0];
    last_page = x;
    bool ov = (x!=0);
#ifdef USE_PRESENCE_BITS    
    if (!ov || !x->presence_bits[a1/PBITS_PER_BYTE]) return false;
#endif
    if (!ov) 
      {
	printf("%s read-8 from unset location " PFX64 "\n", name(), a);
	d = 0;
	return 0;
      }
    d = x -> data[a1 ^ 0 ];
    HTRC(printf("%s: read8 %x from " PFX64 "\n", name(), d, a));
    return true;
  }


  bool ip_read(u64_t a, u64_t &d) // 64 bit read (fast path).
  {
    u64_t a0 = a & ~0xFFF;
    u32_t a1 = a & 0xFFF;
    assert(!(a1 & 7)); // check aligned access
    internal_page *x = memory_hash_map[a0];
    if (!x) 
      {
	printf("%s: read-64 from unset location " PFX64 "\n", name(), a);
	//extern void tmp_trace_bd(const char *); // hack debug output. please delete.
	//tmp_trace_bd("RAM64");
	d = 0;
	return true;
      }
    bool ov = (x!=0);
#ifdef USE_PRESENCE_BITS    
    if (!ov || !x->presence_bits[a1/PBITS_PER_BYTE]) return false;
#endif
    d = ((u64_t *)(x -> data + (a1 & ~7)))[0];

    last_page = x;
    int jj = a1 & ~7;
    //for (int i = 0; i< 8; i++) printf(".%02X", x->data[jj + i]);
    HTRC(printf("jj=%i %s: qread-64 " PFX64 " from " PFX64 "\n", jj, name(), d, a));
    return true;
  }


  bool ip_clear_(u64_t a)
  {
    u64_t a0 = a & ~0xFFF;
    u32_t a1 = a & 0xFFF;
    internal_page *x = memory_hash_map[a0];
    last_page = x;
    bool ov = (x != 0);
#ifdef USE_PRESENCE_BITS    
    ov = ov && x->presence_bits[a1/PBITS_PER_BYTE];
    if (ov) x->presence_bits[a1/PBITS_PER_BYTE] = false;
#endif
    return ov;
  }


  // Update presence bit: return ov
  bool ip_presence(u64_t a, bool newpol)
  {
    bool ov = false;
    u64_t a0 = a & ~0xFFF;
    u32_t a1 = a & 0xFFF;
    internal_page *x = memory_hash_map[a0];
    last_page = x;
    //printf("Presence smile %i at %16X\n", newpol, a);
#ifdef USE_PRESENCE_BITS    
    if (x) ov = x->presence_bits[a1/PBITS_PER_BYTE];
    else
      {
	x = new internal_page(a0);
	memory_hash_map[a0] = x;
      }
    x->presence_bits[a1/PBITS_PER_BYTE] = newpol;
#endif
    return ov;
  }


  bool ip_write(u64_t a, u8_t d, bool es=false)
  {
    u64_t a0 = a & ~0xFFF;
    u32_t a1 = a & 0xFFF;
    internal_page *x = memory_hash_map[a0];
    //printf("Write smile %02X to %16X\n", d, a);
    if (!x)
      {
	x = new internal_page(a0);
	memory_hash_map[a0] = x;
        //printf("\n Memory ip_write a0== " PFX64 ", x== %p", a0, x);
      }
    x -> data[a1 ^ ((es) ? 7:0)] = d;
#ifdef USE_PRESENCE_BITS    
    x->presence_bits[a1/PBITS_PER_BYTE] = true;
#endif
    last_page = x;
    HTRC(printf("%s: write-8 " PFX64 " to " PFX64 "\n", name(), d, a));
    return true;
  }

  bool ip_write(u64_t a, u64_t &d) // 64 bit write (fast path).
  {
    u64_t a0 = a & ~0xFFF;
    u32_t a1 = a & 0xFFF;
    assert(!(a1 & 7)); // check aligned access
    internal_page *x = memory_hash_map[a0];
    bool ov = (x!=0);
    if (!x)
      {
	x = new internal_page(a0);
	memory_hash_map[a0] = x;
        //printf("Memory ip_write to 0x" PFX64 ", \n", mem_base + a0);
      }
    u64_t *ap = (u64_t *) (&(x->data[a1 & ~7]));
    *ap = d;
    last_page = x;
#ifdef USE_PRESENCE_BITS    
    x->presence_bits[a1/PBITS_PER_BYTE] = true;
#endif
    HTRC(printf("%s: Write64 " PFX64 " to " PFX64 "\n", name(), d, a));
    return true;
  }

  
#ifdef LLSC_ENABLE
  // Load locked, store conditional locations.
  llsc_locks_t llsc_locks;
#endif


  u64_t mem_base;      // Base address in bytes
  u64_t mem_size_bytes;// No of bytes
  u8_t mem_width;      // Bits per location
  //u8_t *mem; // byte-addressed memory
 private:
  virtual void recompute_pvt_parameters() = 0; // Pure virtual in this generic model.
  void access(PW_TLM_PAYTYPE &trans, sc_time &delay);
  // nonblocking transport interface prototype
  tlm::tlm_sync_enum nb_access_fw(PW_TLM_PAYTYPE &trans, tlm::tlm_phase &phase, sc_time &delay);
  // callback method for nonblocking transport
  void peq_cb(PW_TLM_PAYTYPE &trans, const tlm::tlm_phase& ph);

  // blocking transport interface prototype
  void b_access(PW_TLM_PAYTYPE &trans, sc_time &delay);  

  tlm_utils::peq_with_cb_and_phase<generic_tlm_mem, PW_TLM_TYPES> m_peq;

#ifdef TLM_POWER3
  pw_energy m_read_energy_op, m_write_energy_op, m_llsc_excess_energy_op;
#endif

 protected:
  friend class sram64_cbg;
  friend class dram64_cbg;
  sc_time m_latency; // Time per word (assuming fully pipelined - i.e initial latency would be longer by n times)
  u64_t m_bits; // Capacity in bits.


  class cbgihex *iloader;
  class cbgelf *eloader;

 public:
  void set_latency(sc_time l) { m_latency = l; }
  // Constructor
  generic_tlm_mem(sc_core::sc_module_name name, u64_t mem_base, u64_t mem_size, u32_t width);

  //TLM-2 socket, defaults to 64-bits wide, power protocol
  tlm_utils::simple_target_socket<generic_tlm_mem, 64, PW_TLM_TYPES> port0;
  void loadme(const char *filename, bool elff, u64_t *entrypt=0);
  bool get_direct_mem_ptr(PW_TLM_PAYTYPE&, tlm::tlm_dmi& dmi_data);
  void stat_report(const char *msg, FILE *fd, bool resetf=false);
  void set_bigendian(tlm::tlm_endianness v) { m_big_endian = v; } // This will be set from ELF load but this methods provides a manual override. The memory itself does not need this apart from load: it is in host-workstation u64_t order.

  traceregion *traceregions;
  virtual const char *kind() { return "generic_tlm_mem"; }
};

#endif


