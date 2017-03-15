// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>




#include "arm_abt.h"
#include "tenos.h"
#include <utility>

#define TRC3(X) X

#define BLOOM_DISCARD_SHIFT 2 /* 32 bits or better - log_2 of line size*/

/*
Trampoline detection in ARM (this is not generic code at the moment):
Typical example:
  82a4:       e28fc600        add     ip, pc, #0, 12
  82a8:       e28cca08        add     ip, ip, #8, 20  ; 0x8000
  82ac:       e5bcfd20        ldr     pc, [ip, #3360]!     
*/ 
void alternate_banch_target_buffer::ins32_monitor(u32_t in, u32_t ip, u32_t pc)
{
  // Trampoline detection for dynamic linking
  if(abt.t_det == 0 && ((in & 0xf800d000) == 0xf000c000)) {
    // BLX
    abt.t_det = 1;
  } 
  
  else if(abt.t_det == 1) {
    if((in & 0xFFFFf000) == 0xe28fc000) {
      abt.t_det = 2;
    } else {
      abt.t_det = 0;
    }
  }
  
  else if(abt.t_det == 2) {
    if ((in & 0xFFFFf000) == 0xe28cc000) {
      abt.t_det = 3;
    } else {
      abt.t_det = 0;
    }
  }
  
  else if (abt.t_det == 3) {
    if ((in & 0xFFFFf000) == 0xe5bcf000)
      abt.t_det = 4;
    
    if(abt.t_det == 4) {
      bool add = !!((in >> 23) & 1);
      bool index = !!((in > 24) & 1);
      u32_t imm = (u32_t)(in & 0xFFF);
      u32_t address = ip;
      if(index) {
	if(add)
	  address += imm;
	else
	  address -= imm;
      }
      
      printf("ARM32 dl-trampoline: (ins=%08x); src = %08x -- via = %08x\n",
	     in, pc-16, address);

      if(abt.via.find(address) == abt.via.end()) {
	abt.via.insert(address);
	// safety check
	// BEGIN
	assert(abt.via_to_src.find(address) == abt.via_to_src.end());
	// END
	abt.via_to_src.insert(std::make_pair(address, (pc - 16)));
      } else {
	// safety check
	// BEGIN
	assert(abt.via_to_src.find(address) != abt.via_to_src.end());
	assert(abt.via_to_src[address] == (pc - 16));
	// END
	
      }
      
      abt.t_det = 0;
#if 0	  
      m_bloomer.set_entry(abt.via >> BLOOM_DISCARD_SHIFT);
#endif
      stats.instructions += 3;
    }
    else {
      abt.t_det = 0;
    }
  }
}


void alternate_banch_target_buffer::install_alt(u32_t address, u32_t final_dest)
{
  if (abt.t_det == 4) 
    {
#if 0            
      stats.installs ++;
      printf("dl-trampoline final step, shortcut here installed : %x -> %x -- %08x\n", abt.src, final_dest, address);
      u32_t h = hash(abt.src);

      if (m_abtable[h]->tag == abt.src) printf(" ... already installed - should be redirected ... this is not encountered?\n"); 
      m_abtable[h]->tag = abt.src;
      m_abtable[h]->final_dest = final_dest;
#endif
      abt.t_det = 0;
    }
}


u32_t alternate_banch_target_buffer::query(u32_t pc, u32_t* cpsr)
{
  u32_t rc = 0xffffFFFF;

  u32_t h = hash(pc);
  if (m_abtable[h]->tag == pc)
    {
      u32_t final_dest = m_abtable[h]->final_dest;
      if((final_dest & 1) == 1) {
	*cpsr |= (1 << 5);
	final_dest &= ~1;
      } else if(((final_dest >> 1) & 1) == 0) {
	*cpsr &= ~(1 << 5);
      }
      
      printf("ARM32 dl-trampoline: query for destination %08x is hit and directs to %08x\n",
	     pc, final_dest);

      stats.deployments += 1;
      rc = final_dest;
    }

  return rc;
}



void alternate_banch_target_buffer::invalidate_all()
{
  m_bloomer.reset_all();
  for (int i =0; i<=m_mask; i++) m_abtable[i]->invalidate();
}



void alternate_banch_target_buffer::written_address_snoop(u32_t address, u32_t data)
{

  // check to see if we need to invalidate address
  if(abt.abtb_via_entries.find(address) != abt.abtb_via_entries.end()) {
    // we are writing to address for which we already have
    // mapping so we need to invalidate all entries
    printf("We are invalidating ABTB due to write to %08x\n",
	   address);
    stats.clears++;
    invalidate_all();
    abt.abtb_via_entries.clear();
  }
  
  std::set<u32_t>::iterator sit = abt.via.find(address);
  if(sit != abt.via.end()) {
    std::map<u32_t, u32_t>::iterator mit = abt.via_to_src.find(address);
    assert(mit != abt.via_to_src.end());
    
    printf("ARM32 dl-trampoline: patched from %08x via %08x to %08x\n",
	   mit->second, address, data);
    
    u32_t h = hash(mit->second);
    if(m_abtable[h]->tag == mit->second) {
      // Already installed, check to see if final destination is the same
      // if it is not then we need to invalidate
      if(m_abtable[h]->final_dest != data)
	printf("ARM32 dl-trampoline: INVALID INVALID INVALID\n");
    }

    abt.abtb_via_entries.insert(address);
    m_abtable[h]->tag = mit->second;
    m_abtable[h]->final_dest = data;
    stats.installs++;
    
    abt.via.erase(sit);
    abt.via_to_src.erase(mit);
  }
  
#if 0
  bool hit = m_bloomer.test_entry(address  >> BLOOM_DISCARD_SHIFT);
  if (hit)
    {
      //      printf("abtb write hit on bloom filter: trampoline at 0x%x has changed\n", address);
      stats.clears ++;
      invalidate_all();
    }
#endif
}


// We need to invalidate the whole Bloom filter on any snooped writes that hit in it.
void alternate_banch_target_buffer::b_access(PW_TLM_PAYTYPE &trans, sc_time &delay)

{
  u64_t addr = trans.get_address();
#if 0  
  written_address_snoop((addr & ~3));
#endif
  trans.set_response_status(tlm::TLM_OK_RESPONSE);
}



alternate_banch_target_buffer:: alternate_banch_target_buffer(const sc_module_name &module_name,  bool with_snoop_socket, int logsize, int bloom_logsize) :   // constructor
  sc_module(module_name),
  m_bloomer(bloom_logsize),
  m_abt_capacity(1<<logsize) // constructor
{
  m_mask = (1<<logsize)-1;
  m_abtable = new abtab_entry *[(1<<logsize)];
  for (int i =0; i<m_abt_capacity; i++) m_abtable[i] = new abtab_entry();
  abt.t_det = 0;
  if (with_snoop_socket)
    {
      snoop_socket = new tlm_utils::simple_target_socket<alternate_banch_target_buffer, 64, PW_TLM_TYPES> ;
      snoop_socket->register_b_transport(this, &alternate_banch_target_buffer::b_access);
    }
  else snoop_socket = 0;

  new tenos_report_item(module_name, "trampoline_instructions", &stats.instructions);

  //printf("abt created\n");
}


// eof
