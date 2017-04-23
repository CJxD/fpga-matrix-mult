// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2012 M Puzovic and D J Greaves
// University of Cambridge, Computer Laboratory.
// $Id: $

// Implementation file for hypertransport links

#include "hypertransport.h"
#include "opteron_messages.h"

extern bool dynload_done;

using namespace tlm;

hypertransport::hypertransport(sc_core::sc_module_name names,
			       int socket_no,
			       int link_no,
			       sc_time period,
			       int link_size,
			       int src_clk_cycles,
			       int dest_clk_cycles)
  : sc_module(names), 
    m_socket_no(socket_no), 
    m_link_no(link_no),
    m_period(period), 
    m_link_size(link_size),
    m_next_time(SC_ZERO_TIME),
    m_src_clk_cycles(src_clk_cycles),
    m_dest_clk_cycles(dest_clk_cycles)
{

  // crate lock
  char name[132];
  snprintf(name, 132, "hypertransport_lock_%i_%i", socket_no, link_no);
  ht_lock = new sc_mutex(name);

  // Register callbacks for incoming interface method calls
  targ_socket.register_b_transport(this, &hypertransport::b_transport);
  targ_socket.register_nb_transport_fw(this, &hypertransport::nb_transport_fw);
  targ_socket.register_get_direct_mem_ptr(this, &hypertransport::get_direct_mem_ptr);
}


// TLM-2 blocking transport method
void hypertransport::b_transport(int id, PRAZOR_GP_T& trans, sc_time &delay_) {
  // if it comes from the crossbar then it needs to be forwarded
  if(id == 0) {
    init_socket[1]->b_transport(trans, delay_);
  }
  // if it comes from other hypertransport link then send it to the crossbar
  else if(id == 1) {
    init_socket[0]->b_transport(trans, delay_);
  }
  else
    assert(0);

}

tlm_sync_enum hypertransport::nb_transport_fw(int id,
					      PRAZOR_GP_T& trans,
					      tlm_phase& phase,
					      sc_time& delay_) {

  int dest_node = -1;
  u64_t cache_addr = 0;

  probe_msg* probem = 0;
  req_msg* reqm = 0;
  ack_msg* ackm = 0;
  unblock_msg* unblockm = 0;
  sri_msg* srim = 0;

  trans.get_extension(probem);
  trans.get_extension(reqm);
  trans.get_extension(ackm);
  trans.get_extension(unblockm);
  trans.get_extension(srim);

  if(probem != 0) {
    cache_addr = probem->cache_addr;
    dest_node = probem->dest_node;
  }
  else if(reqm != 0) {
    cache_addr = reqm->cache_addr;
    dest_node = reqm->home_node;
  }
  else if(ackm != 0) {
    cache_addr = ackm->addr;
    dest_node = ackm->dest_node;
  }
  else if(unblockm != 0) {
    dest_node = unblockm->dest_node;
  }
  else if(srim != 0) {
    dest_node = srim->home_node;
  }
  else {
    if(dynload_done)
      assert(0);
  }
  
  // if it is from crossbar forward it
  if(id == 0) {    
#if MOESI_TIMING
    sc_time msg_delay = SC_ZERO_TIME;
    
    if(probem != 0 || reqm != 0 || unblockm != 0) {
      // probe and req messages are command message
      // so the size of the message is 4 bytes
      msg_delay += (4/(m_link_size/8)) * m_period;
    }
    else if(ackm != 0) {
      if(trans.get_data_ptr() != 0 &&
	 trans.get_data_length() != 0) {
	msg_delay += (trans.get_data_length()/(m_link_size/8)) * m_period;
      }
      else {
	msg_delay += (4/(m_link_size/8)) * m_period;
      }
    }
    
    // add clock crossing delay if node that created
    // probe message is owner of this link
    if((probem && probem->originator_node == m_socket_no)
       || (reqm && reqm->creator_node == m_socket_no)
       || (unblockm && unblockm->creator_node == m_socket_no)
       || (ackm && ackm->orig_node == m_socket_no))
      {
	AUGMENT_LT_DELAY(trans.ltd, delay_, m_dest_clk_cycles * m_period);
      }
    ht_lock->lock();
    if(COLLECT_LT_DELAY(sc_time_stamp() + delay_, trans.ltd.point()) >= m_next_time) 
      {
	m_next_time = COLLECT_LT_DELAY(sc_time_stamp() + delay_, trans.ltd.point()) + msg_delay;
	AUGMENT_LT_DELAY(trans.ltd, delay, msg_delay);
    }
    else {
      // we should forward the message when the last one
      // before has gone through
      // OLD: sc_time diff = m_next_time - (sc_time_stamp() + delay);
      sc_time diff = m_next_time - COLLECT_LT_DELAY(sc_time_stamp() + delay_, trans.ltd.point());
      AUGMENT_LT_DELAY(trans.ltd, delay_, diff + msg_delay);
      m_next_time += msg_delay;
    }
    ht_lock->unlock();

#endif
    
    init_socket[1]->nb_transport_fw(trans, phase, delay_);
  }
  // if it comes from other hypertransport link then send it to the crossbar
  else if(id == 1) {
#if MOESI_TIMING
    // add clock crossing boundary
    if((probem && probem->dest_node == m_socket_no)
       || (reqm && reqm->home_node == m_socket_no)
       || (unblockm && unblockm->dest_node == m_socket_no)
       || (ackm && ackm->dest_node == m_socket_no))
      AUGMENT_LT_DELAY(trans.ltd, delay, m_src_clk_cycles * m_period);
#endif

    init_socket[0]->nb_transport_fw(trans, phase, delay_);
  }
  else
    assert(0);
  
  return TLM_ACCEPTED;
}

bool hypertransport::get_direct_mem_ptr(int n, 
					PRAZOR_GP_T& trans, 
					tlm::tlm_dmi& dd) {
  
  // MP: To be implemented (TODO)
  assert(0);
}

// eof
