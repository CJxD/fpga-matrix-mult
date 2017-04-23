// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2012 M Puzovic and D J Greaves
// University of Cambridge, Computer Laboratory.
// $Id: $

// Hammer cache as used in AMD Opteron

#ifndef HAMMER_CACHE64_H
#define HAMMER_CACHE64_H

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/peq_with_cb_and_phase.h"
#include "consistent_cache64.h"

#include "../bus/opteron_messages.h"

#include <systemc.h>

class hammer_cache64 : public consistent_cache64 {

  hammer_cache64* lower_level_data;

 private:
  bool cache_invalid_msg;
  u64_t cache_invalid_addr;
  u64_t processing_probe;

 public:
  // these sockets are used when doing MOESI broadcasting
  // they are initialized only if we want to use MOESI protocol
  tlm_utils::simple_initiator_socket<hammer_cache64, 64, PW_TLM_TYPES>* moesi_init;
  tlm_utils::simple_target_socket<hammer_cache64, 64, PW_TLM_TYPES>* moesi_targ;


  hammer_cache64(sc_module_name name, 
		 consistent_group* grp,
		 int bytes, 
		 int ways, 
		 int linesize, 
		 int secondary_width, 
		 sc_time clock_period,
		 int cache_level, 
		 cache_ty ty,
		 bool invalidate_lower = true,
		 uint32_t hit_cycles = 1,
		 int bl = 1,
		 on_miss_ty store_miss = READ_ALLOCATE,
		 on_miss_ty write_miss = WRITE_ALLOCATE,
		 write_policy_ty write_policy = WRITE_BACK,
		 bool use_write_buffer = true,
		 int secondary_linesize = -1,
		 int secondary_banks = 1,
		 bool big_endian = true,
		 cc_protocol_ty protocol = MOESI_CC);

  void set_lower_level_data_cache(hammer_cache64* l) {
    lower_level_data = l;
  }
  

  void probe_message(PW_TLM_PAYTYPE &trans, 
		     sc_time &delay_, 
		     u64_t addr, 
		     probe_msg* msg);

  void peq_cb(PW_TLM_PAYTYPE& trans, const tlm::tlm_phase& ph);
  void moesi_peq_cb(PW_TLM_PAYTYPE& trans, const tlm::tlm_phase& ph);
  tlm::tlm_sync_enum nb_transport_fw(int id,
				     PW_TLM_PAYTYPE& trans,
				     tlm::tlm_phase& phase,
				     sc_time& delay);
  
  tlm::tlm_sync_enum moesi_nb_transport_fw(PW_TLM_PAYTYPE& trans,
					   tlm::tlm_phase& phase,
					   sc_time& delay);
  
  tlm::tlm_sync_enum moesi_nb_transport_bw(PW_TLM_PAYTYPE& trans,
					   tlm::tlm_phase& phase,
					   sc_time& delay);
  
  // what to do on miss type when using MOESI protocol
  void operate_miss_type(cacheway*, int, cache_miss_extension*);

  void evict_lower_level_line(u64_t addr, size_t linesize, u8_t* data);

  void operate(cacheway *cw, 
	       cacheway::mesi_t op, 
	       u64_t line_addr, 
	       int dmap, 
	       sc_time &delay);

  std::pair<u8_t*, u8_t*> check_lower_level_lines(u64_t line, 
						  size_t linesize,
						  cacheway::mesi_t new_state);

  bool do_i_lookup(u64_t addr);

  tlm_utils::peq_with_cb_and_phase<hammer_cache64, PW_TLM_TYPES> m_peq;
  tlm_utils::peq_with_cb_and_phase<hammer_cache64, PW_TLM_TYPES> m_moesi_peq;

  opteron_msg_mm* m_moesi_mm;
};

#endif
