// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2012 M Puzovic and D J Greaves
// University of Cambridge, Computer Laboratory.
// $Id: $

// AMD Opteron-like system request interface

#ifndef SRI_H
#define SRI_H

#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/multi_passthrough_target_socket.h"
#include "tlm_utils/multi_passthrough_initiator_socket.h"
#include "tlm_utils/peq_with_cb_and_phase.h"

#include "opteron_messages.h"
#include "prazor.h"
#include "tenos.h"

#include <set>
#include <queue>
#include <map>

#include <boost/dynamic_bitset.hpp>
#include <boost/tuple/tuple.hpp>

#if 0
#define SRIDEBUG(X) cout << sc_time_stamp() << "-"; \
  printf("tid=%d-", sc_get_current_process_b()->proc_id); X
#define SRIDEBUG1(X)
#else
#define SRIDEBUG(X)
#define SRIDEBUG1(X)
#endif

class sri :
  public sc_module
#ifdef TLM_POWER3
  , public pw_module
#endif
{
 private:
  int node;
  int data_length;

  std::map<u64_t, u64_t> address_map;
  u64_t last_addr;
  std::vector<u64_t> empty_addr;
  sc_mutex* m_addr_table_lock;

  // frequency at which SRI works = Northbridge frequncy
  sc_time m_period;
  // number of cycles that it takes to access memory tables
  int m_mem_table_access_cycles;

  // core frequency
  sc_time m_core_period;
  
  int m_src_clk_cycles;
  int m_dest_clk_cycles;

  size_t m_total_sockets;

  u64_t m_iostart_base;

  bool m_migrate_memory;

#if 0
  sri_msg_mm_t sri_msg_mm;
  unblock_msg_mm_t unblock_msg_mm;
  req_msg_mm_t req_msg_mm;
#endif
  
  opteron_msg_mm* m_mm;
  

  std::map<u64_t, boost::tuple<PRAZOR_GP_T*, sc_event*, int> > wait_tran_map;
  std::queue<boost::tuple<PRAZOR_GP_T*, sc_event*> > cache_line_par;
  std::map<u64_t, std::queue<int> > cache_line_order;

  tlm_utils::peq_with_cb_and_phase<sri, PW_TLM_TYPES> m_peq;
  void peq_cb(PRAZOR_GP_T& trans, const tlm::tlm_phase& ph);

  int crossbar_id;

  bool probe_msg_busy;
  std::queue<PRAZOR_GP_T*> probe_requests;
  PRAZOR_GP_T* cache_ack_data_tran; // ack that contains data if any
  PRAZOR_GP_T* dram_ack_data_tran;
  PRAZOR_GP_T* wb_ack_data_tran;
  int probe_ack_received;
  bool active_probe_invalidation;
  u64_t probe_addr;

  // in the crossbar we send a message in parallel to DRAM and local caches
  // if SRI is already busy with probing caches the local caches probe message
  // will be delayed and therefore DRAM answer can come in sooner so we
  // need to store it before we start processing cache probes
  std::map<u64_t, PRAZOR_GP_T*> dram_probe_asynch_msg;

  std::map<u64_t, int> ack_msgs;
  // used as safety check, can be used instead of ack_msgs
  std::map<u64_t, boost::dynamic_bitset<>* > ack_msgs_bits;
  std::map<u64_t, u8_t*> ack_old_mem_write;
  std::map<u64_t, ack_msg::ack_type_t> ack_response;
  std::map<u64_t, u8_t*> ack_wb_lanes;
  

  void notify_parallel_cache_lines(PRAZOR_GP_T&);

 protected:
  std::set<u64_t> address_table;
  
  sc_mutex* ml;

  class sri* next;

 public:
  tlm_utils::multi_passthrough_target_socket<sri, 64, PW_TLM_TYPES> targ_socket;
  tlm_utils::multi_passthrough_initiator_socket<sri, 64, PW_TLM_TYPES> init_socket;

  // socket to send to UART
  tlm_utils::simple_initiator_socket<sri, 64, PW_TLM_TYPES> local_iodev_socket;

  // Constructor
  sri(sc_core::sc_module_name, 
      sri* prev, 
      int node, 
      int data_length,
      sc_mutex* addr_table_lock,
      opteron_msg_mm* mm,
      sc_time period,
      int mem_table_access_cycles,
      sc_time core_period,
      int src_clk_cycles,
      int dest_clk_cycles,
      size_t total_sockets,
      u64_t,
      u64_t,
      bool memory_migration = false);

  // TLM-2 blocking transport method
  bool get_direct_mem_ptr(int n, PRAZOR_GP_T&, tlm::tlm_dmi& dmi_data);
  void b_transport(int id, PRAZOR_GP_T& trans, sc_time &delay);
  tlm::tlm_sync_enum nb_transport_bw(int id,
				     PRAZOR_GP_T& trans,
				     tlm::tlm_phase& phase,
				     sc_time& delay);
  tlm::tlm_sync_enum nb_transport_fw(int id, 
				     PRAZOR_GP_T& trans,
				     tlm::tlm_phase& phase,
				     sc_time& delay);

  u64_t get_real_address(u64_t);

  std::set<u64_t>* get_address_table_ptr() {
    return &address_table;
  }

  std::map<u64_t, u64_t>* get_address_map_ptr() {
    return &address_map;
  }

  struct stats {
    sc_time latency;
    int n;
  };

  // type of the stats that we want to record
  enum stat_ty {
    WRITE_INVALIDATION,
    WRITE_DATA,
    READ_DATA
  } stats_ty;

  std::map<enum stat_ty, std::map<u64_t, struct stats> > node_latency_time;
  std::map<u64_t, std::pair<sc_time, int > > address_start_access_time;
  std::map<u64_t, sc_time> address_paused_time;

  // used to collect how many lines are not owned by this core anymore
  std::map<u64_t, u64_t> remote_core_owners;
  std::set<u64_t> local_core_owners;

public:
  void flushout() {
    //address_map.clear();
    
    clean();
  }

  void clean() {
    for(std::map<enum stat_ty, std::map<u64_t, struct stats> >::iterator i = node_latency_time.begin();
	i != node_latency_time.end();
	i++) {
      if(node_latency_time.count(i->first) != 0)
	node_latency_time[i->first].clear();

      node_latency_time.clear();
    }
    node_latency_time.clear();

    address_start_access_time.clear();
    address_paused_time.clear();

    remote_core_owners.clear();
    local_core_owners.clear();
  }

  void stat_report(const char*, FILE*, bool resetf = false, bool no_header = true);
};

#endif
