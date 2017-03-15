// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2011 DJ Greaves + M Puzovic
// University of Cambridge, Computer Laboratory.
// ACS P35 SoC D/M Classes - $Id: vmunit64.h,v 1.6 2011/08/22 13:30:19 djg11 Exp $
// Generic payload blocking transport router

#ifndef VMUNIT64_H
#define VMUNIT64_H

#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/multi_passthrough_initiator_socket.h"
#include "tlm_utils/multi_passthrough_target_socket.h"
#include "tenos.h"

#include <stdio.h>
#include <math.h>

#include <algorithm>
#include <functional>

#ifdef TLM_POWER3
#include <tlm_power>
using namespace sc_pwr;
#define POWER3(X) X
#else
typedef tlm::tlm_base_protocol_types PW_TLM_TYPES;
typedef tlm::tlm_generic_payload PW_TLM_PAYTYPE;
#define POWER3(X)
#endif

#if 0
#define VMDEBUG(X) X
#else
#define VMDEBUG(X)
#endif


bool is_page_fault(class vmunit64* me, u64_t addr);

typedef struct _pt_level {
  uint size;
  uint log_size;
  bool is_leaf;
} pt_level_t;

// Structure representing multi-level pagetable 
typedef struct _pt {
  void **table;
  int level;
} pt_t;

// Page tabe entry
// The same structure is used for the physical memory
// and virtual memory, if valid is set it is physical
// memory structure otherwise it is virtual
typedef struct _pte {
  u64_t vfn; // virtual frame number
  u64_t pfn; // physical frame number
  u32_t node; // node number
  bool valid; 
  bool modified;
} pte_t;

class page_fault {
 protected:
  struct cmp_page_number : public std::unary_function<u64_t, bool> {
    u64_t ntosearch;
    
    // Default constructor
    cmp_page_number(u64_t number_) : ntosearch(number_) {}
     
     bool operator() (pte_t* pte) const {
       return ntosearch == pte->pfn;
     }
  };

  u64_t phys_pages;

  page_fault(u64_t phys_pages_)
    : phys_pages(phys_pages_) {}

 public:
  virtual void handler(pte_t *pte) = 0;
  virtual void evict_page(u64_t n) = 0;
  virtual void load_page(u64_t pfn, pte_t* new_page) = 0;
};

class page_fault_sequential : public page_fault {
 private:
  // vector of physical memory pages
  // TODO: if we decide to imlement page eviction
  // and more advanced page allocation algorithms
  // we might need to conisder using more efficient
  // data structure (MP - 8 Dec 11)
  typedef std::vector<pte_t*> physmem_t;
  physmem_t physmem;

  u64_t last_page;

 public:
  page_fault_sequential(u64_t phys_pages_)
    : last_page(0), page_fault(phys_pages_) {}

 public:
  void handler(pte_t* pte) {
    u64_t phn = last_page++;
    assert(0 <= phn && phn <= phys_pages);
    // we don't evict for this policy as we are
    // (a) assuming we can fit the whole executable in memory
    // (b) number of physical page is always increasing
    load_page(phn, pte);
  }

  void evict_page(u64_t n) {
    // search for the page
    physmem_t::iterator e = std::find_if(physmem.begin(), physmem.end(), 
					 cmp_page_number(n));

    if(e == physmem.end()) {
      // page doesn't exist so nothing to do here
      return;
    }

    // otherwise page exists
    pte_t* pte = *e;
    
    // TODO: We can check to see if page is dirty
    // by checking to see if modified flag is set to true

    // erase entry from the structure
    pte->modified = false;
    pte->valid = false;
    pte->pfn = -1;

    // TODO: Before deleting entry we should store
    // somewhere the page content so next time it is
    // retrieved we retrieved the correct content
    physmem.erase(e);    
  }

  void load_page(u64_t pfn, pte_t* new_page) {
    assert(0 <= pfn && pfn < phys_pages);
    assert(new_page && !new_page->valid);
    
    physmem_t::iterator e = std::find_if(physmem.begin(), physmem.end(),
					 cmp_page_number(pfn));
    assert(e == physmem.end());

    new_page->pfn = pfn;
    new_page->modified = false;
    new_page->valid = true;

    physmem.push_back(new_page);    
  }

};


class page_fault_opteron : public page_fault {
 private:
  std::map<u32_t, u64_t> last_page;

  typedef std::map<u32_t, std::vector<pte_t*> > physmem_t;
  physmem_t physmem;

 public:
  page_fault_opteron(u64_t phys_pages_) 
    : page_fault(phys_pages_) {}

  void handler(pte_t* pte) {
    if(last_page.count(pte->node) == 0)
      last_page[pte->node] = 0;
    
    u64_t phn = last_page[pte->node]++;
    assert(0 <= phn && phn <= phys_pages);

    load_page(phn, pte);
  }

  void load_page(u64_t pfn, pte_t* new_page) {
    assert(0 <= pfn && pfn < phys_pages);
    assert(new_page && !new_page->valid);
    
    std::vector<pte_t*>::iterator e = std::find_if(physmem[new_page->node].begin(), 
						   physmem[new_page->node].end(),
						   cmp_page_number(pfn));
    assert(e == physmem[new_page->node].end());

    new_page->pfn = pfn;
    new_page->modified = false;
    new_page->valid = true;

    physmem[new_page->node].push_back(new_page);    
  }

  void evict_page(u64_t n) {
    assert(0);
  }
};

class vmunit64: 
  public sc_module
#ifdef TLM_POWER3
  , public pw_module
#endif
{

 private:
  sc_mutex ml;
  
  // size in bits of processor address space
  const u16_t addr_space_bits;

  // number of levels of page tables
  u16_t nlevels;
  // size of a page table
  u64_t pagesize;

  // number of bits in virtual frame number
  u64_t vfn_bits;

  // page table levels
  pt_level_t* levels;
  // pointer to the first level
  // usually it is kept in a dedicated register
  pt_t *root_table;

  // page allocation algorithm
  page_fault* fault;
  u64_t xlat(u64_t vadr, u32_t coreno);

  u64_t total_physmem;
 public:

  // Use tagged sockets to be able to distinguish incoming backward path calls
  tlm_utils::multi_passthrough_target_socket<vmunit64, 64, PW_TLM_TYPES>    targ_socket;
  tlm_utils::multi_passthrough_initiator_socket<vmunit64, 64, PW_TLM_TYPES> init_socket;

  // Constructor
  vmunit64(sc_core::sc_module_name, u16_t, u64_t);

  // Page table operations
  
  /*********
   * BEGIN *
   *********/

  // created a new page table at a given level
  pt_t* pt_new_table(int level);

  // gets a virtual frame number
  inline u64_t vaddr_to_vfn(u64_t v_adr) {
    return getbits(v_adr, addr_space_bits-1, vfn_bits);
  }

  // returns page table entry for a given virtual frame number
  pte_t* pt_lookup_vaddr(u64_t vfn);

  // Recursively searches the pagetables
  // vfn = virtual frame number
  // bits = number of (high) bits already considered by higher levels
  // pages = the pagetable for this level
  pte_t *pt_lookup_helper(u64_t vfn, u64_t bits, u64_t masked_vfn,
				 pt_t *pages);

  // creates a new pagetable entry
  pte_t* pt_new_pte(u64_t vfn);

  // FORWARD PATH
  // TLM-2 blocking transport method
  bool get_direct_mem_ptr(int n, PW_TLM_PAYTYPE&, tlm::tlm_dmi& dmi_data);
  void b_transport(int id, PW_TLM_PAYTYPE &trans, sc_time &delay); 
  void stat_report(const char *msg, FILE *fd, bool resetf=false);

  sc_signal <u64_t> s_vaddr, s_paddr; // Crude TLM monitoring using signals.

};
#endif
// eof
