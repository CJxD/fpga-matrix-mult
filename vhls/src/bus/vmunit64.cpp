// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2011 DJ Greaves + M Puzovic
// University of Cambridge, Computer Laboratory.
// ACS P35 SoC D/M Classes - $Id: vmunit64.cpp,v 1.10 2011/08/22 13:30:19 djg11 Exp $
// Generic payload blocking transport router

#include "vmunit64.h"
#include "mconfig.h"


// Constructor
vmunit64::vmunit64(
  sc_core::sc_module_name names,
  u16_t nlevels_,
  u64_t pagesize_)
  : sc_module(names), 
    ml("vmunit_mutex_lock"),
    targ_socket("targ_socket"), 
    init_socket("init_socket"),
    addr_space_bits(mconfig::adr_space_bits()), // hard coded for now
    nlevels(nlevels_),
    pagesize(pagesize_)
{
  // Register callbacks for incoming interface method calls
  targ_socket.register_b_transport(this, &vmunit64::b_transport);
  targ_socket.register_get_direct_mem_ptr(this, &vmunit64::get_direct_mem_ptr);

  int page_bits = bound_log2(pagesize);
  
  u32_t bits = 0;
  u32_t level = 0;

  vfn_bits = addr_space_bits - page_bits;
  u32_t bits_per_level = (u32_t)ceil(vfn_bits / nlevels);

  levels = (pt_level_t*)malloc(sizeof(pt_level_t)*nlevels);
  while(1) {
    if(bits + bits_per_level >= vfn_bits)
      break;

    levels[level].log_size = bits_per_level;
    levels[level].size = pow_2(levels[level].log_size);
    levels[level].is_leaf = false;

    bits += bits_per_level;
    level++;
  }

  levels[level].log_size = vfn_bits - bits;
  levels[level].size = pow_2(levels[level].log_size);
  levels[level].is_leaf = true;

  // create the first level
  root_table = pt_new_table(0);

  // page allocation algorithm
  u64_t physmem_size = mconfig::megsOfMemory() * (1 << 20);
  fault = new page_fault_sequential(physmem_size);
  
  // MP: TESTING
  /*fault = new page_fault_opteron(total_physmem);*/

  // should be only temporary (MP - 8 Dec 11)
  extern class vmunit64 *VMUnitBackdoor;
  VMUnitBackdoor = this;

}

pt_t* vmunit64::pt_new_table(int level) {
  pt_t * table;
  pt_level_t *config;

  config = &levels[level];
  assert(config);

  table = (pt_t*)malloc(sizeof(pt_t));
  assert(table);

  table->table = (void**)calloc(config->size, sizeof(void*));
  assert(table->table);

  table->level = level;

  return table;
}

pte_t* vmunit64::pt_lookup_vaddr(u64_t vaddr) {
  return pt_lookup_helper(vaddr, 0, vaddr, root_table);
}

pte_t* vmunit64::pt_lookup_helper(u64_t vfn, u64_t bits, u64_t mask,
					 pt_t *pages) {

  uint log_size = levels[pages->level].log_size;
  uint index = getbits(mask, vfn_bits - bits - 1, log_size);
  bits += log_size;
  mask = getbits(mask, vfn_bits - bits - 1, vfn_bits - bits);

  if(levels[pages->level].is_leaf) {
    if(pages->table[index] == NULL) {
      // first access to the page
      pages->table[index] = (void*)pt_new_pte(vfn);
    }
    return (pte_t*)(pages->table[index]);
  } else {
    if(pages->table[index] == NULL) {
      pages->table[index] = pt_new_table(pages->level+1);
    }
    return pt_lookup_helper(vfn, bits, mask, (pt_t*)pages->table[index]);
  }
}

pte_t* vmunit64::pt_new_pte(u64_t vfn) {
  pte_t *pte;
  pte = (pte_t*)(malloc(sizeof(pte_t)));
  assert(pte);

  pte->vfn = vfn;
  pte->pfn = -1;
  pte->node = -1;
  pte->valid = false;
  pte->modified = false;

  return pte;
}


// TLM-2 blocking transport method
void vmunit64::b_transport(int client, PW_TLM_PAYTYPE &trans, sc_time &delay)
{
  POWER3(PW_TLM3(trans.pw_log_hop(this))); // First call
  u64_t v_adr = (u64_t) trans.get_address(); 
  int tid =  sc_get_current_process_b()->proc_id;

  // Need SC_SIGNAL_WRITE_CHECK=DISABLE in the environment for this to work re-entrantly. 
  s_vaddr.write(v_adr);

  // FIXME: For now we are assuming that core is directly connected to
  // vmunit and that that even numbers are connected to instruction stream
  // while odd numbers are connected to data stream
  VMDEBUG(printf("VM: mapping address 0x%lx from client %d\n", v_adr, client));
  ml.lock();
  u64_t p_adr = xlat(v_adr, floor(client/2));
  ml.unlock();

  s_paddr.write(p_adr);
  VMDEBUG(printf("VM: address from client %d 0x%lx mapped to 0x%x\n", 
		 client, v_adr, p_adr));
  //printf("%s: op %llx >= %llx\n", name(), adr, threshold);

  trans.set_address(p_adr);
  init_socket[client]->b_transport(trans, delay);

  POWER3(PW_TLM3(trans.pw_log_hop(this))); //Second call, after return.
  //trans.set_address(v_adr);
}

bool vmunit64::get_direct_mem_ptr(int n, PW_TLM_PAYTYPE &trans, tlm::tlm_dmi &dmi_data)
{
  assert(0); // Does this need xlat too ?
  init_socket[n]->get_direct_mem_ptr(trans, dmi_data);
}


u64_t vmunit64::xlat(u64_t v_adr, u32_t coreno)
{
  pte_t* pte = pt_lookup_vaddr(vaddr_to_vfn(v_adr));
  u32_t nodeno = floor(coreno / mconfig::cores_per_socket);

  // there is no mapping for a page 
  if(!pte->valid) {
    // TODO: maybe add counter for page faults
    // tell to the handler which core has made page fault
    pte->node = nodeno;
    fault->handler(pte);
    //printf("Physical page %llx assigned to %llx\n", pte->pfn, pte->vfn);
  }


  // calculating physical address
  u64_t p_addr = 
    pte->pfn * pagesize + getbits(v_adr, 
				  addr_space_bits - vfn_bits - 1, 
				  addr_space_bits - vfn_bits);

  // modify the most significant bits to decode what
  // is the node id
  p_addr |= pte->node << bound_log2(total_physmem);
  return p_addr;
}

bool is_page_fault(vmunit64* me, u64_t addr) {
  pte_t* pte = me->pt_lookup_vaddr(me->vaddr_to_vfn(addr));
  //printf("PAGE_FAULT: 0x%lx, %d\n", addr, pte->valid);
  return !pte->valid;
}

void vmunit64::stat_report(const char *msg, FILE *fd, bool resetf)  
{ // for now 
}

// eof
