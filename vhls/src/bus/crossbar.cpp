// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2012 M Puzovic and D J Greaves
// University of Cambridge, Computer Laboratory.
// $Id: $

// AMD Opteron-like crossbar implementation file

#include "crossbar.h"
#include "llsc_extension.h"

using namespace std;
using namespace tlm;

extern bool dynload_done;

crossbar::crossbar(sc_core::sc_module_name names,
		   size_t sockets,
		   size_t hts,
		   u32_t node,
		   map<uint32_t, uint32_t> rt,
		   sc_mutex* addr_table_lock,
		   crossbar* prev,
		   opteron_msg_mm* mm,
		   sc_time period,
		   int src_clk_cycles,
		   int dest_clk_cycles)
  : sc_module(names), 
    total_sockets(sockets),
    ht_links(hts), 
    src_clk_cycles(src_clk_cycles),
    dest_clk_cycles(dest_clk_cycles),
    node_number(node), 
    route_table(rt),
    m_peq(this, &crossbar::peq_cb),
    cb_address_map(0),
    cb_address_table(0),
    m_addr_table_lock(addr_table_lock),
    m_mm(mm),
    m_period(period)
{

  // create lock
  char name[132];
  snprintf(name, 132, "crossbar_lock_%i", node);
  ml = new sc_mutex(name);

  // Register callbacks for incoming interface method calls
  //targ_socket.register_b_transport(this, &crossbar::b_transport);
  targ_socket.register_nb_transport_fw(this, &crossbar::nb_transport_fw);
  init_socket.register_nb_transport_bw(this, &crossbar::nb_transport_bw);
  targ_socket.register_get_direct_mem_ptr(this, &crossbar::get_direct_mem_ptr);

  // circular linked list
  if(prev) {
    crossbar* begin = prev->next;
    prev->next = this;
    next = begin;
  }
  else
    next = this;
}

void crossbar::req_incoming(req_msg* reqm, PRAZOR_GP_T& trans) {
  int home_node = reqm->home_node;
  int source_node = reqm->source_node;

  llsc_extension *llsc = NULL;
  trans.get_extension(llsc);

  if(home_node == node_number) {
    ml->lock();
    if(req_busy) {
      CBDEBUG(fprintf(stdout, 
		      "%s: BUSY! Putting on hold REQ message from %d; 0x%lx\n", name(), source_node, reqm->cache_addr));
      *(reqm->paused_time) = sc_time_stamp();

      req_queue.push(&trans);

      ml->unlock();

      return;
    }
    else {
      CBDEBUG(fprintf(stdout,
		      "%s: processing REQ message from source node %d, home node %d; 0x%lx; %s\n", 
		      name(), source_node, home_node, reqm->cache_addr, trans.is_write() ? "write" : "read"));
      req_busy = true;
    }
    ml->unlock();

    for(int i = 0; i < total_sockets; i++) {
      // create a probe msg
      probe_msg* mm = new probe_msg;
      mm->source_node = source_node;
      mm->home_node = reqm->home_node;
      mm->originator_node = node_number;
      mm->dest_node = i;
      
      mm->cache_addr = reqm->cache_addr;

      // based on information in data ptr we decide whether the write is 
      // just invalidation (writing to write buffer) then data ptr is empty or 
      // it is invalidation with a write to main memory then data ptr is not empty
      if(trans.get_data_ptr() == 0)
	mm->real_write = false;

      //PRAZOR_GP_T* probem = probe_msg_mm.allocate();
      PRAZOR_GP_T* probem = m_mm->allocate();

      probem->set_extension<probe_msg>(mm);
      probem->acquire();

      probem->set_extension<llsc_extension>(llsc);
      CBDEBUG(if(llsc != NULL) \
		printf("req_incoming: llsc is NOT NULL for addres = 0x%lx\n", trans.get_address()); \
	      else							\
		printf("req_incoming: llsc is NULL for addres = 0x%lx\n", trans.get_address()));
      
      probem->set_data_length(trans.get_data_length());
      probem->set_byte_enable_length(trans.get_data_length());
      u8_t* lanes = new u8_t[1024];
      for(int k = 0; k < 1024; k++)
	lanes[k] = 0x00;
      probem->set_byte_enable_ptr(lanes);
      if(trans.is_read())
	probem->set_read();
      else if(trans.is_write())
	probem->set_write();
      else
	assert(0);
      probem->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE );
      probem->set_address(trans.get_address());

      probem->set_data_ptr(0);

      if(i == node_number) {
	// do not send request to DRAM if original request is from the same node for write
        if(!(/*reqm->source_node == node_number &&*/  // MP 14Aug13 - we don't need this check otherwise llsc fails
	     trans.is_write())
	   // or if we are not writing anything in a transaction
	   && !(trans.is_write() 
		&& trans.get_data_ptr() == 0))
	  mm->dram = true;
	CBDEBUG(fprintf(stdout, "%s: probe request to itself for 0x%lx\n",
			name(), reqm->cache_addr));
	tlm_phase ph = BEGIN_REQ;
	sc_time delay = SC_ZERO_TIME;

	init_socket[0]->nb_transport_fw(*probem, ph, delay);
	continue;
      }

      tlm_phase ph = BEGIN_REQ;
      u32_t port = route_table[i];
      CBDEBUG(fprintf(stdout, "%s: probe request from %d to %d for 0x%lx; port %d\n",
		      name(), node_number, i, mm->cache_addr, port));

      sc_time delay = SC_ZERO_TIME;
      
#if MOESI_TIMING
      // here we are crossing clock boundaries from northbridge
      // to the hypetransport link
      AUGMENT_LT_DELAY(trans.ltd, delay,  src_clk_cycles * m_period);
#endif

      tlm_sync_enum s = init_socket[1 + port]->nb_transport_fw(*probem, ph, delay);
      // return should always be accepted
      assert(s == TLM_ACCEPTED);
    }

    // MP 14Aug13 - we don't need this check otherwise llsc fails
    if(!(/*reqm->source_node == node_number &&*/ trans.is_write())
       && !(trans.is_write() 
	    && trans.get_data_ptr() == 0)) {
      // also send a message to the memory controller
      ack_msg* mm = new ack_msg;
      mm->dest_node = source_node;
      mm->home_node = reqm->home_node;
      mm->addr = reqm->cache_addr;
      mm->orig_node == -1;
      mm->ty = ack_msg::DRAM;
      if(trans.is_write()) mm->write = true;
      
      //PRAZOR_GP_T* ackm = ack_msg_mm.allocate();
      PRAZOR_GP_T* ackm = m_mm->allocate();
      ackm->acquire();

      ackm->set_extension<ack_msg>(mm);

      //llsc_extension *llsc = NULL;
      //trans.get_extension(llsc);
      CBDEBUG(printf("SETTING llsc extension to ack with 0x%lx\n", llsc));
      ackm->set_extension<llsc_extension>(llsc);
      
      ackm->set_data_length(trans.get_data_length());
      ackm->set_byte_enable_length(trans.get_byte_enable_length());
      u8_t* ly = trans.get_byte_enable_ptr();
      if(ly == 0)
	ackm->set_byte_enable_ptr(0);
      else {
	u8_t* lx = new u8_t[trans.get_byte_enable_length()];
	for(int i = 0; i < trans.get_byte_enable_length(); i++)
	  lx[i] = ly[i];
	ackm->set_byte_enable_ptr(lx);
      }

      ackm->set_byte_enable_ptr(0);
      ackm->set_streaming_width(trans.get_streaming_width());
      
      ackm->set_read();
      ackm->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
      ackm->set_address(trans.get_address());
      
      u8_t* cline = new u8_t[trans.get_data_length()];
      ackm->set_data_ptr(cline);
      
      sc_time delay = SC_ZERO_TIME;
      tlm_phase ph = BEGIN_REQ;
      CBDEBUG(fprintf(stdout, "%s: sending DRAM access at node %d for addr 0x%lx; real 0x%lx; llsc 0x%lx\n", 
		      name(), home_node, ackm->get_address(), trans.get_address(), llsc));

      init_socket[ht_links + 1]->nb_transport_fw(*ackm, ph, delay);
    }
    // we can release req_msg as it is not needed anymore
    trans.release();
    
  }
  else {
    // need to forward message
    CBDEBUG(fprintf(stdout, "%s: forwarding request from %d to %d\n",
		    name(), node_number, home_node));
    u32_t port = route_table[home_node];
    tlm_phase ph = BEGIN_REQ;
    sc_time delay = SC_ZERO_TIME;

#if MOESI_TIMING
    if(reqm->creator_node == node_number)
      // crossing clock boundary 
      AUGMENT_LT_DELAY(trans.ltd, delay,  src_clk_cycles * m_period);
#endif
    init_socket[1 + port]->nb_transport_fw(trans, ph, delay);
  }
}

void crossbar::sri_incoming(sri_msg* srim, PRAZOR_GP_T& trans) {
  // need to check weather it needs to send it to the memory controller
  // or to send it to the one of hyper transport links
  int home_node = srim->home_node;
  if(home_node == node_number) {
    // it is the same node, so forward the message to the memory controller
    CBDEBUG(fprintf(stdout, "%s: sending SRI message to the memory controller for addr = 0x%lx\n", 
		    name(), srim->cache_addr));
    
    CBDEBUG(u8_t* src = trans.get_data_ptr();				\
	    u8_t* lanes = trans.get_data_ptr();				\
	    fprintf(stdout, "%s: will send to DRAM controller: ", name()); \
	    for(int i = 0; i < trans.get_data_length(); i++)		\
	      fprintf(stdout, "%02x", src[i]);				\
	    fprintf(stdout, "\n"));

    sc_time delay = SC_ZERO_TIME;
    tlm_phase ph = BEGIN_REQ;
    init_socket[ht_links + 1]->nb_transport_fw(trans, ph, delay);
  }
  else {

    bool isRead = trans.is_read();
#if 0
    if(!isRead) assert(trans.is_write());
    
    // create a request message
    req_msg* mm = new req_msg;
    mm->source_node = node_number;
    mm->home_node = home_node;
    mm->cache_addr = srim->cache_addr;
    //PRAZOR_GP_T* reqm = req_msg_mm.allocate();
    PRAZOR_GP_T* reqm = m_mm->allocate();
    reqm->acquire();

    reqm->set_extension<req_msg>(mm);
    
    reqm->set_data_length(trans.get_data_length());
    reqm->set_byte_enable_length(trans.get_byte_enable_length());
    reqm->set_byte_enable_ptr(trans.get_byte_enable_ptr()); 
    reqm->set_streaming_width(trans.get_streaming_width());
    if(trans.is_read())
      reqm->set_read();
    else if(trans.is_write())
      reqm->set_write();
    else
      assert(0);
    reqm->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE );
    reqm->set_address(trans.get_address());
    reqm->set_data_ptr(trans.get_data_ptr());
#endif
    // forward the request to the home node
    u32_t port = route_table[home_node];
    CBDEBUG(fprintf(stdout, 
		    "%s: request is %s on 0x%lx; forwarding to %d from %d using port %d\n",
		    name(), 
		    isRead ? "read" : "write", 
		    trans.get_address(), home_node, node_number, port));
    sc_time delay = SC_ZERO_TIME;
    tlm_phase ph = BEGIN_REQ;
    init_socket[1 + port]->nb_transport_fw(trans, ph, delay);
  }   
}

void crossbar::probe_incoming(probe_msg* probe, PRAZOR_GP_T& trans) {
  tlm_phase ph = BEGIN_REQ;
  sc_time delay = SC_ZERO_TIME;
  if(node_number == probe->dest_node) {
    // forward probe to sri module which should forward it
    // to the cache to check for data
    CBDEBUG(fprintf(stdout, "%s: forwarding probe to SRI module to send to caches\n", name()));
    init_socket[0]->nb_transport_fw(trans, ph, delay);
  }
  else {
    // reforward it to correct node
    u32_t port = route_table[probe->dest_node];
    init_socket[1 + port]->nb_transport_fw(trans, ph, delay);
  }
}

void crossbar::ack_incoming(ack_msg* ack, PRAZOR_GP_T& trans) {
  tlm_phase ph = BEGIN_REQ;
  sc_time delay = SC_ZERO_TIME;
  if(ack->evict_line) {
    ph = ACK_DRAM;
    CBDEBUG(fprintf(stdout, "%s: forwarding ACK message for 0x%lx to SRI backwards\n",
		    name(), ack->addr));
    targ_socket[0]->nb_transport_bw(trans, ph, delay);
  }
  else if(ack->ty == ack_msg::DRAM) {
    // we have received ACK from DRAM controller so send it to its own SRI
    CBDEBUG(fprintf(stdout, "%s: forwarding ACK message from DRAM controller to SRI\n", name()));
    init_socket[0]->nb_transport_fw(trans, ph, delay);
  }
  else if(ack->ty == ack_msg::UNKNOWN && node_number == ack->dest_node) {
#if MOESI_TIMING
    // the message was received from the ht link so there was clock crossing
    if(ack->orig_node != ack->dest_node)
      AUGMENT_LT_DELAY(trans.ltd, delay,  dest_clk_cycles * m_period);
#endif

    // forward acknowledgment message to sri module 
    CBDEBUG(fprintf(stdout, "%s: forwarding ACK message from %d to sri module; %d\n", 
		    name(), ack->orig_node, ack->dest_node));
    init_socket[0]->nb_transport_fw(trans, ph, delay);
  }
  else {
    u32_t port = route_table[ack->dest_node];
    CBDEBUG(fprintf(stdout, "%s: forwarding ACK message to node %d via port %d\n",
		    name(), ack->dest_node, 1 + port));

#if MOESI_TIMING
    if(ack->orig_node == node_number)
      AUGMENT_LT_DELAY(trans.ltd, delay,  src_clk_cycles * m_period);
#endif
    init_socket[1 + port]->nb_transport_fw(trans, ph, delay);
  }
}

void crossbar::unblock_incoming(unblock_msg* unblock, PRAZOR_GP_T& trans) {
  tlm_phase ph = BEGIN_REQ;
  sc_time delay = SC_ZERO_TIME;
  if(node_number == unblock->dest_node) {
    // receiving unblocking message so we can process the next request
    CBDEBUG(fprintf(stdout, "%s: received unblocking message\n", name()));
    ml->lock();
    assert(req_busy);
    req_busy = false;
    if(req_queue.size() > 0) {
      PRAZOR_GP_T* t = req_queue.front();
      req_queue.pop();
      
      // check to see if this node still owns the memory address
      req_msg* mm = 0;
      t->get_extension(mm);
      assert(mm != 0);

      *(mm->paused_time) = sc_time_stamp() - *(mm->paused_time);
      CBDEBUG(cout << name() << ": request was paused for " << *(mm->paused_time) << endl);

      m_addr_table_lock->lock();
      if(cb_address_table->find(mm->cache_addr) == cb_address_table->end()) {
	CBDEBUG(fprintf(stdout, "%s: node %d doesn't own address 0x%lx\n",
			name(), node_number, mm->cache_addr));
	// find who owns it
	crossbar* adrt = next;
	u64_t real_addr = 0;
	int owner = -1;
	while(adrt != this) {
	  if(adrt->cb_address_table->find(mm->cache_addr) != adrt->cb_address_table->end()) {
	    assert(adrt->cb_address_map->count(mm->cache_addr) != 0);
	    real_addr = adrt->cb_address_map->at(mm->cache_addr);
	    owner = adrt->node_number;

	    CBDEBUG(fprintf(stdout, "%s: node %d owns address 0x%lx\n",
			    name(), adrt->node_number, mm->cache_addr));
	    break;
	  }
	  adrt = adrt->next;
	}
	assert(real_addr != 0);
	assert(adrt != this);
	assert(owner != -1);
	m_addr_table_lock->unlock();
	

	llsc_extension* llsc = NULL;
	trans.get_extension(llsc);
	if(llsc != NULL) {
	  printf("0x%llx vs 0x%llx vs 0x%lx vs 0x%lx\n", trans.get_address(), t->get_address(), mm->cache_addr, real_addr);
	  assert(0);
      }

	mm->home_node = owner;
	mm->creator_node = node_number;
	*(mm->owner_node) = owner;
	t->set_address(real_addr);
	u32_t port = route_table[owner];

	tlm_phase ph = BEGIN_REQ;
	sc_time delay = SC_ZERO_TIME;
#if MOESI_TIMING
	// clock crossing between norhtbridge and HT link
	AUGMENT_LT_DELAY(trans.ltd, delay,  src_clk_cycles * m_period);
#endif
	//llsc_extension* llsc;
	t->get_extension(llsc);
	if(llsc != NULL) 
	  assert(0);
	init_socket[1 + port]->nb_transport_fw(*t, ph, delay);
      }
      else {
	m_addr_table_lock->unlock();
	m_peq.notify(*t, ph, delay);
      }
    }
    ml->unlock();

    // release unblocking transaction
    trans.release();
  }
  else {
#if MOESI_TIMING
    if(unblock->creator_node == node_number)
      AUGMENT_LT_DELAY(trans.ltd, delay,  src_clk_cycles * m_period);
#endif
    u32_t port = route_table[unblock->dest_node];
    init_socket[1 + port]->nb_transport_fw(trans, ph, delay);
  }
}

// TLM-2 blocking transport method
void crossbar::peq_cb(PRAZOR_GP_T& trans, const tlm_phase& ph) {
  if(ph == END_REQ) {    
    sc_time delay = SC_ZERO_TIME;
    tlm_phase ph = END_REQ;

    sri_msg* sri_mm = 0;
    trans.get_extension(sri_mm);
    
    ack_msg* ack_mm = 0;
    trans.get_extension(ack_mm);

    if(sri_mm) {
      if(sri_mm->originating_node != node_number) {
        // need to send it to correct SRI
        u32_t port = route_table[sri_mm->originating_node];
        sc_time delay = SC_ZERO_TIME;
        tlm_phase ph = END_REQ;
        init_socket[1 + port]->nb_transport_fw(trans, ph, delay);
      }
      else 
        targ_socket[0]->nb_transport_bw(trans, ph, delay);
    }
    else if(ack_mm) {
      CBDEBUG(fprintf(stdout, "%s: received acknolwedgment from DRAM controller for 0x%lx; trans addr = 0x%lx\n", 
		      name(), ack_mm->addr, trans.get_address()));
      u8_t* src = trans.get_data_ptr();
      CBDEBUG(fprintf(stdout, "%s: from DRAM controller: ", name());	\
	      for(int i = 0; i < trans.get_data_length(); i++)		\
		 fprintf(stdout, "%02x", src[i]);			\
		 fprintf(stdout, "\n"));

      ack_mm->resp = ack_msg::DRAM;
      ack_incoming(ack_mm, trans);
    }
    else
      assert(0);
    
  }
  else if(ph == BEGIN_REQ) {
    // what to do depends on the type of the message that we have received
    // the messages are:
    sri_msg* srim = 0; // message from SRI (system request interface)
    req_msg* reqm = 0; // it is a request message to the home node
    probe_msg* probem = 0; // it is a probe message from the home node
    ack_msg* ackm = 0; // it is acknowledgment message
    unblock_msg* unblockm = 0; // it is unblocking message
        
    // find out what is the message type
    trans.get_extension(srim);
    if(srim == 0) { 
      trans.get_extension(reqm);
      if(reqm == 0) {
	trans.get_extension(probem);
	if(probem == 0) {
	  trans.get_extension(ackm);
	  if(ackm == 0) {
	    trans.get_extension(unblockm);
	    if(unblockm == 0) {
	      CBDEBUG(fprintf(stdout, "%s: cannot figure out the type of the message\n", name()));
	      assert(0);
	    }
	  }
	}
      }
    }

    if(srim)
      sri_incoming(srim, trans);
    else if(reqm)
      req_incoming(reqm, trans);
    else if(probem)
      probe_incoming(probem, trans);
    else if(ackm) 
      ack_incoming(ackm, trans);
    else if(unblockm)
      unblock_incoming(unblockm, trans);
    else
      assert(0);
  }
  else if(ph == ACK_DRAM) {
    ack_msg* mm = 0;
    trans.get_extension(mm);
    assert(mm != 0);

    CBDEBUG(fprintf(stdout, "%s: sending ACK DRAM to the memory controller 0x%lx\n",
		    name(), mm->addr));
    CBDEBUG(u8_t* src = trans.get_data_ptr();				\
	    fprintf(stdout, "%s: will send to DRAM controller: ", name()); \
	    for(int i = 0; i < trans.get_data_length(); i++)		\
	      fprintf(stdout, "%02x", src[i]);				\
	    fprintf(stdout, "\n"));

    sc_time delay = SC_ZERO_TIME;
    tlm_phase ph = BEGIN_REQ;

    u8_t* lanes = trans.get_byte_enable_ptr();

    if(mm->lower_level_hit || mm->wb_hit) 
      assert(trans.get_byte_enable_ptr() != 0);
    else {
      if(lanes != 0)
	delete lanes;
      trans.set_byte_enable_length(0);
      trans.set_byte_enable_ptr(0);
    }
    init_socket[ht_links + 1]->nb_transport_fw(trans, ph, delay);
  }
  else
    assert(0);
}

bool crossbar::get_direct_mem_ptr(int n, PRAZOR_GP_T& trans, tlm::tlm_dmi& dd) {
  
  // MP: To be implemented (TODO)
  assert(0);
}

tlm_sync_enum crossbar::nb_transport_fw(int id,
					PRAZOR_GP_T& trans,
					tlm_phase& phase,
					sc_time& delay) {
  // Crossbar incoming links should be
  // 0 - from SRI
  // from 1 to HT_LINKS - from hyper transport links
  
  // Crossbar outgoing links are
  // 0 - to SRI
  // from 1 to HT_LINKS - to hyper transport links
  // HT_LINKS + 1 - to memory controller 

  sri_msg* srim = 0; // message from SRI (system request interface)
  req_msg* reqm = 0; // it is a request message to the home node
  probe_msg* probem = 0; // it is a probe message from the home node
  ack_msg* ackm = 0; // it is acknowledgment message
  unblock_msg* unblockm = 0; // it is unblocking message
  
  // find out what is the message type
  trans.get_extension(srim);
  if(srim == 0) { 
    trans.get_extension(reqm);
    if(reqm == 0) {
      trans.get_extension(probem);
      if(probem == 0) {
	trans.get_extension(ackm);
	if(ackm == 0) {
	  trans.get_extension(unblockm);
	  if(unblockm == 0) {
	    CBDEBUG(fprintf(stdout, "%s: cannot figure out the type of the message\n", name()));
	    assert(0);
	  }
	}
      }
    }
  }

  // clock boundary crossing if it was retrieved from hypertransport links
#if MOESI_TIMING
  if((id > 0 && id <= ht_links)
     && ((probem && probem->dest_node == node_number)
	 || (reqm && reqm->home_node == node_number)
	 || (unblockm && unblockm->dest_node == node_number)
	 || (ackm && ackm->dest_node == node_number)))
    AUGMENT_LT_DELAY(trans.ltd, delay,  dest_clk_cycles * m_period);
#endif

  m_peq.notify(trans, phase, delay);

  return TLM_ACCEPTED;
}

tlm_sync_enum crossbar::nb_transport_bw(int id,
					PRAZOR_GP_T& trans,
					tlm_phase& phase,
					sc_time& delay) {
  // Crossbar incoming links should be
  // 0 - from SRI
  // from 1 to HT_LINKS - from hyper transport links
  
  // Crossbar outgoing links are
  // 0 - to SRI
  // from 1 to HT_LINKS - to hyper transport links
  // HT_LINKS + 1 - to memory controller 
  m_peq.notify(trans, phase, delay);
  return TLM_ACCEPTED;
}

// eof
