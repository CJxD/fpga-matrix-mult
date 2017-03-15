// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2012 M Puzovic and D J Greaves
// University of Cambridge, Computer Laboratory.
// $Id: $

// AMD Opteron-like crossbar header file.  This has a routing table to direct hypertransport messages according to the node number in the TLM header extension.

#ifndef CROSSBAR_H
#define CROSSBAR_H

#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/multi_passthrough_initiator_socket.h"
#include "tlm_utils/multi_passthrough_target_socket.h"
#include "tlm_utils/peq_with_cb_and_phase.h"

#include "tenos.h"

#include "opteron_messages.h"

#include <map>
#include <set>
#include <queue>

#if 0
#define CBDEBUG(X) cout << sc_time_stamp() << "-"; \
  printf("tid=%d-", sc_get_current_process_b()->proc_id); X
#else
#define CBDEBUG(X)
#endif


#ifdef TLM_POWER3
#include <tlm_power>
using namespace sc_pwr;
#define POWER3(X) X
#else
typedef tlm::tlm_base_protocol_types PW_TLM_TYPES;
typedef tlm::tlm_generic_payload PW_TLM_PAYTYPE;
#define POWER3(X)
#endif

class crossbar :
  public sc_module
#ifdef TLM_POWER3
  , public pw_module
#endif
{
 private:
  u32_t node_number;

  size_t total_sockets;
  size_t ht_links;

  int src_clk_cycles;
  int dest_clk_cycles;

  // Helper methods
  void sri_incoming(sri_msg* msg, PW_TLM_PAYTYPE& t);
  void req_incoming(req_msg* msg, PW_TLM_PAYTYPE& t);
  void probe_incoming(probe_msg* msg, PW_TLM_PAYTYPE& t);
  void ack_incoming(ack_msg* msg, PW_TLM_PAYTYPE& t);
  void unblock_incoming(unblock_msg* msg, PW_TLM_PAYTYPE& t);

  std::map<uint32_t, uint32_t> route_table;    

#if 0
  req_msg_mm_t req_msg_mm;
  probe_msg_mm_t probe_msg_mm;
  ack_msg_mm_t ack_msg_mm;
#endif

  opteron_msg_mm* m_mm;

  // working frequncy of the crossbar
  // should be the same as SRI as they are both
  // situated in the northbridge
  sc_time m_period;

  tlm_utils::peq_with_cb_and_phase<crossbar, PW_TLM_TYPES> m_peq;

  sc_mutex* ml;

  bool req_busy;
  std::queue<PW_TLM_PAYTYPE*> req_queue;

  std::set<u64_t>* cb_address_table;
  std::map<u64_t, u64_t>* cb_address_map;

  sc_mutex* m_addr_table_lock;

  crossbar* next;

 public:
  tlm_utils::multi_passthrough_target_socket<crossbar, 64, PW_TLM_TYPES> targ_socket;
  tlm_utils::multi_passthrough_initiator_socket<crossbar, 64, PW_TLM_TYPES> init_socket;

  // Constructor
  crossbar(sc_core::sc_module_name, 
	   size_t,
	   size_t,
	   u32_t, 
	   std::map<uint32_t, uint32_t> route_table,
	   sc_mutex*,
	   crossbar*,
	   opteron_msg_mm* m_mm,
	   sc_time period,
	   int,
	   int);

  // TLM-2 blocking transport method
  bool get_direct_mem_ptr(int n, PW_TLM_PAYTYPE&, tlm::tlm_dmi& dmi_data);
  //void b_transport(int id, PW_TLM_PAYTYPE& trans, sc_time &delay);

  tlm::tlm_sync_enum nb_transport_fw(int n,
				     PW_TLM_PAYTYPE& trans,
				     tlm::tlm_phase& phase,
				     sc_time& delay);

  tlm::tlm_sync_enum nb_transport_bw(int n,
				     PW_TLM_PAYTYPE& trans,
				     tlm::tlm_phase& phase,
				     sc_time& delay);

  void peq_cb(PW_TLM_PAYTYPE& trans, const tlm::tlm_phase& ph);

  void init_address_table(std::set<u64_t>* table, std::map<u64_t, u64_t>* t_map) {
    cb_address_table = table;
    cb_address_map = t_map;
  }
};

#endif
