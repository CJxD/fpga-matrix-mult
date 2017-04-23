// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2012 M Puzovic and D J Greaves
// University of Cambridge, Computer Laboratory.
// $Id: $

// AMD Opteron-like system request interface implementation file

#include "sri.h"
#include "../memories/memories_extensions.h"

#include "tenos.h"
#include "llsc_extension.h"
#include "argv_backdoor.h"

using namespace tlm;
using namespace boost;

extern bool dynload_done;

extern argv_backdoor_setup *argv_backdoor_su;

sri::sri(sc_core::sc_module_name names,
	 sri* prev,
	 int node_,
	 int data_length_,
	 sc_mutex* addr_table_lock,
	 opteron_msg_mm *mm,
	 sc_time period,
	 int mem_table_access_cycles,
	 sc_time core_period,
	 int src_clk_cycles,
	 int dest_clk_cycles,
	 size_t total_sockets,
	 u64_t iostart_base,
         u64_t dram_base,
         bool migrate_memory)
  : sc_module(names), 
    m_core_period(core_period),
    m_src_clk_cycles(src_clk_cycles),
    m_dest_clk_cycles(dest_clk_cycles),
    m_total_sockets(total_sockets),
    m_iostart_base(iostart_base),
    node(node_), 
    data_length(data_length_),
    m_peq(this, &sri::peq_cb),
    last_addr(dram_base),
    probe_msg_busy(false),
    probe_addr(-1),
    cache_ack_data_tran(0),
    dram_ack_data_tran(0),
    wb_ack_data_tran(0),
    probe_ack_received(0),
    active_probe_invalidation(false),
    m_addr_table_lock(addr_table_lock),
    m_mm(mm),
    m_period(period),
    m_mem_table_access_cycles(mem_table_access_cycles),
    m_migrate_memory(migrate_memory)
{

  // create lock
  char name[132];
  snprintf(name, 132, "sri_lock_%i", node);
  ml = new sc_mutex(name);

  // Register callbacks for incoming interface method calls
  targ_socket.register_b_transport(this, &sri::b_transport);
  targ_socket.register_get_direct_mem_ptr(this, &sri::get_direct_mem_ptr);
  targ_socket.register_nb_transport_fw(this, &sri::nb_transport_fw);
  init_socket.register_nb_transport_bw(this, &sri::nb_transport_bw);

  // allocate memory for the address table
  if(prev) {
    sri* begin = prev->next;
    prev->next = this;
    next = begin;
  }
  else // in this case this is the first instance of sri
    next = this;
}


u64_t sri::get_real_address(u64_t addr) {
  if(address_map.count(addr) != 0)
    return address_map[addr];
  else {
    u64_t a;
    if(empty_addr.size() > 0) {
      int sz = empty_addr.size();
      a = empty_addr.back();
      empty_addr.pop_back();
      assert(empty_addr.size() == (sz - 1));
    }
    else {
      a = last_addr;
      last_addr += data_length;
    }
    address_table.insert(addr);
    address_map[addr] = a;
    return a;
  }
}

// TLM-2 blocking transport method
void sri::b_transport(int id, PRAZOR_GP_T& trans, sc_time &delay) {
  // System request interface has mconfig::cores_per_socket incoming
  // links and for now it just redirects a message to the crossbar

  int owner = -1;

  // address
  u64_t a = ((u64_t)trans.get_address());
  u64_t size = ((u64_t)trans.get_data_length()); // size in bytes
  SRIDEBUG(fprintf(stdout, "%s: addr = 0x%lx, size = %d, from port %d\n",
		   name(), a, size, id));

  if(a >= m_iostart_base) {
    // send it to UART socket
    local_iodev_socket->b_transport(trans, delay);
    return;
  }

  if(!trans.is_write())
    assert(size == data_length);


  ml->lock();
  assert(id >= 0 && id < targ_socket.size() - 1);
  sc_event* my_wait_req = new sc_event;
  if(wait_tran_map.count(a) != 0 || 
     (cache_line_order.count(a) != 0 
      && cache_line_order[a].size() > 0
      && cache_line_order[a].front() != sc_get_current_process_b()->proc_id)) {   

      SRIDEBUG(fprintf(stdout, "%s: cache line order size %d, top is %d\n",
                       name(), cache_line_order[a].size(), cache_line_order[a].front()));
      SRIDEBUG(fprintf(stdout, "%s: waiting for neighbouring cache to process 0x%lx; flags %d, %d\n",
                       name(), a, wait_tran_map.count(a), cache_line_order.count(a)));

     
      if(wait_tran_map.count(a) != 0) {
          PRAZOR_GP_T* in_process_tran = wait_tran_map[a].get<0>();
          if(in_process_tran != NULL && in_process_tran->get_data_ptr() == 0 && in_process_tran->is_write()) {
              // the transaction that we are processing is invalidation, therefore
              // if the transaction that we want to process writes then it will
              // be already written with this invalidation transaction so we can skip it
              if(trans.is_write() && trans.get_data_ptr() != 0) {
                  trans.set_response_status(tlm::TLM_OK_RESPONSE);
                  delete my_wait_req;
                  return;
              }
          }
      }
      
            
      // make sure that accesses are done in order
      cache_line_order[a].push(sc_get_current_process_b()->proc_id);

      while(true) {
#if 0
	PRAZOR_GP_T* wait_tran;
	if(wait_tran_map.count(a) != 0) {
	  tie(wait_tran, tuples::ignore, tuples::ignore) = wait_tran_map[a];
	  if(trans.is_write() && 
	     trans.get_byte_enable_ptr() != 0 && 
	     trans.get_data_ptr() != 0 &&
	     wait_tran->is_write() && 
	     wait_tran->get_byte_enable_ptr() != 0 &&
	     wait_tran->get_data_ptr() != 0) {
            // if they are writing to the same cache line then the bytes they are writting
            // to have to be different
            assert(trans.get_byte_enable_length() == wait_tran->get_byte_enable_length());
            u8_t* wait_tran_lanes = wait_tran->get_byte_enable_ptr();
            u8_t* this_tran_lanes = trans.get_byte_enable_ptr();
            for(int k = 0; k < trans.get_byte_enable_length(); k++) {
	      if((wait_tran_lanes[k] & this_tran_lanes[k]) != 0) {
		u8_t* wait_data = wait_tran->get_data_ptr();
		u8_t* this_data = trans.get_data_ptr();
		SRIDEBUG(printf("%s: addr = 0x%lx --> k = %d: 0x%x, 0x%x, %02x vs %02x\n", 
				name(), a, k, wait_tran_lanes[k], this_tran_lanes[k], wait_data[k], this_data[k]));
		assert(wait_data[k] == this_data[k]);
	      }
	      else
		assert((wait_tran_lanes[k] & this_tran_lanes[k]) == 0);
            }
	  }
	}
#endif      

	cache_line_par.push(make_tuple(&trans, my_wait_req));    

	ml->unlock();
	wait(*my_wait_req);
        
	// check to see if the neighbouring cache didn't jump in again
	ml->lock();
	if(wait_tran_map.count(a) == 0 
	   && cache_line_order.count(a)
	   && cache_line_order[a].front() == sc_get_current_process_b()->proc_id) {
	  cache_line_order[a].pop();
	  
	  SRIDEBUG(fprintf(stdout, "%s: neighbouring cache finished processing 0x%lx, size %d\n",
			   name(), a, cache_line_order[a].size()));
	  
	  if(cache_line_order.count(a) == 0)
	    cache_line_order.erase(a);
	  
	  wait_tran_map[a] = make_tuple(&trans, my_wait_req, owner);
	  
	  ml->unlock();
          
	  
	  break;
	}
      }
  }
  else {
    wait_tran_map[a] = make_tuple(&trans, my_wait_req, owner);
    ml->unlock();
  }

  // check to see if a given address has already been written
  // to DRAM of some node, if there isn't any then this node should own it
  u64_t real_addr = 0;
  sri* adrt = this;
  m_addr_table_lock->lock();
  do {
    if(adrt->address_table.find(a) != adrt->address_table.end()) {
      owner = adrt->node;
      real_addr = adrt->get_real_address(a);
      adrt->local_core_owners.insert(a);
      break;
    }
    adrt = adrt->next;
  } while(adrt != this);

  SRIDEBUG(fprintf(stdout, "accessing SRI from node %d; 0x%lx vs 0x%lx; addr = 0x%lx, %s, owner = %d\n", 
                   this->node, adrt, this, a, trans.is_read() ? "READ" : "WRITE", owner));

  // none of the nodes have this memory address so this node will own it
  if(owner == -1) {
    // we should never have orphaned memory address if it is not writing for the first time
    if(trans.is_read()) {
      //printf("%s: Looking to read address 0x%lx that is not in the address table\n", name(), a);

      // djg temp fix
      //trans.set_response_status(tlm::TLM_OK_RESPONSE);
      //    return;
      //assert(0);
    }
    if(address_table.find(a) == address_table.end())
      address_table.insert(a);
    real_addr = get_real_address(a);
    owner = node;
    local_core_owners.insert(a);
  }
  m_addr_table_lock->unlock();


  get<2>(wait_tran_map[a]) = owner;

  // used for collecting latency information stats
  assert(address_start_access_time.count(a) == 0);
  address_start_access_time[a].first = sc_time_stamp() + delay;
  address_start_access_time[a].second = owner;
  assert(address_paused_time.count(a) == 0);
  address_paused_time[a] = SC_ZERO_TIME;
  
  SRIDEBUG(fprintf(stdout, "%s: real_addr = 0x%lx(0x%lx), size = %d, %s is %s(%d,%d)\n",
		   name(), real_addr, a, size, trans.is_write() ? "write" : "read", node != owner ? "REMOTE" : "LOCAL", node, owner));
  SRIDEBUG(
      if(trans.is_write() && trans.get_data_ptr() != NULL) {                    \
          u8_t* test = trans.get_data_ptr();                             \
          fprintf(stdout, "%s: data to be written by waiting transaction for addr 0x%lx(0x%lx), length %d: ", name(), real_addr, a, trans.get_data_length()); \
          for(int i = 0; i < trans.get_data_length(); i++)              \
              fprintf(stdout, "%02x", test[i]);                         \
          fprintf(stdout, "\n");                                       \
      });

  // send the message to crossbar with a correct DRAM address and to which
  // core this memory address belongs
  req_msg* mm = new req_msg;
  mm->source_node = node;
  mm->home_node = owner;
  mm->cache_addr = a;
  mm->creator_node = node;

  // used for stats
  mm->paused_time = &address_paused_time[a];
  mm->owner_node = &(address_start_access_time[a].second);
  //PRAZOR_GP_T* reqm = req_msg_mm.allocate();
  PRAZOR_GP_T* reqm = m_mm->allocate();
  reqm->acquire();
    
  reqm->set_extension<req_msg>(mm);

  // set extension for ll/sc if any
  llsc_extension *llsc = NULL;
  trans.get_extension(llsc);
  SRIDEBUG(if(llsc != NULL)						\
	     printf("LL/SC extension is NOT NULL in SRI for 0x%lx\n",  real_addr, a); \
	   else								\
	     printf("LL/SC extension is NULL in SRI for 0x%lx\n",  real_addr, a));   
  //reqm->set_extension<llsc_extension>(llsc);
  reqm->set_extension(llsc);
  
  reqm->set_data_length(trans.get_data_length());
  reqm->set_byte_enable_length(trans.get_byte_enable_length());
  reqm->set_byte_enable_ptr(0);
  reqm->set_streaming_width(trans.get_streaming_width());
  if(trans.is_read())
    reqm->set_read();
  else if(trans.is_write())
    reqm->set_write();
  reqm->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE );
  reqm->set_address(real_addr);
  reqm->set_data_ptr(trans.get_data_ptr());
  
#if MOESI_TIMING
  // here we are crossing clock boundary from processor to northbridge
  AUGMENT_LT_DELAY(trans.ltd, delay,  m_src_clk_cycles * m_core_period
		   + m_dest_clk_cycles * m_period);

  // it takes 3 cycles to access memory tables
  AUGMENT_LT_DELAY(trans.ltd, delay,  m_mem_table_access_cycles * m_period);
#endif

  // type of access
  enum stat_ty stat_collect;
  if(trans.get_command() == tlm::TLM_WRITE_COMMAND) {
    if(trans.get_data_ptr() == 0)
      stat_collect = WRITE_INVALIDATION;
    else
      stat_collect = WRITE_DATA;
  }
  else {
    assert(trans.get_command() == tlm::TLM_READ_COMMAND);
    stat_collect = READ_DATA;
  }


  tlm_phase ph = BEGIN_REQ;
  tlm_sync_enum s = init_socket[init_socket.size() - 1]->nb_transport_fw(*reqm, 
									 ph, 
									 delay);
  assert(s == TLM_ACCEPTED);
  
  ml->lock();
  SRIDEBUG(fprintf(stdout, "%s: has a lock on incoming connection %d, addr = 0x%lx\n", 
		   name(), id, a));
  // map where we are going to be keeping ACK received from the home node
  assert(ack_msgs.count(a) == 0);
  ack_msgs[a] = 0;
  ack_response[a] = ack_msg::UNKNOWN;
  ack_old_mem_write[a] = 0;
  ack_msgs_bits[a] = new dynamic_bitset<>(m_total_sockets);
  ml->unlock();
  
  wait(*my_wait_req);


  ml->lock();
  delete my_wait_req;
  ack_msgs.erase(a);
  delete ack_msgs_bits[a];
  if(ack_old_mem_write[a] != 0)
    delete ack_old_mem_write[a];
  ack_old_mem_write.erase(a);
  ack_response.erase(a);
  ack_msgs_bits.erase(a);

  assert(address_start_access_time.count(a) != 0);
  sc_time latency = sc_time_stamp() 
    - address_start_access_time[a].first 
    - address_paused_time[a] 
    + delay;

  int node_that_owned = address_start_access_time[a].second;
  if(node_latency_time[stat_collect].count(node_that_owned) == 0) {
    node_latency_time[stat_collect][node_that_owned].n = 1;
    node_latency_time[stat_collect][node_that_owned].latency = latency;
  }
  else {
    node_latency_time[stat_collect][node_that_owned].n++;;
    node_latency_time[stat_collect][node_that_owned].latency += latency;
  }

  address_start_access_time.erase(a);
  address_paused_time.erase(a);
  SRIDEBUG(cout << name()  << ": " << (wait_tran_map[a].get<2>() != node ? "[REMOTE]" : "[LOCAL]") 
	   << " time to access address " << hex << a << " was " 
	   << latency << " from node " << dec << wait_tran_map[a].get<2>() << endl);

  int elem_removed = wait_tran_map.erase(a);
  assert(elem_removed > 0);
  SRIDEBUG(fprintf(stdout, "%s: release a lock on incoming connection %d, addr = 0x%lx\n", 
		   name(), id, a));
  SRIDEBUG(
      if(trans.is_read() && trans.get_data_ptr() != NULL) {                    \
          u8_t* test = trans.get_data_ptr();                             \
          fprintf(stdout, "%s: data to be read by waiting transaction for addr 0x%lx(0x%lx), length %d: ", name(), real_addr, a, trans.get_data_length()); \
          for(int i = 0; i < trans.get_data_length(); i++)              \
              fprintf(stdout, "%02x", test[i]);                         \
          fprintf(stdout, "\n");                                       \
      });

  notify_parallel_cache_lines(trans);

  ml->unlock();

  // if error is TLM_ADDRESS_ERROR_RESPONSE status then llsc failed
  if(!trans.is_response_error() 
     || (trans.is_response_error() 
       && trans.get_response_status() != tlm::TLM_ADDRESS_ERROR_RESPONSE)) {
    trans.set_response_status(tlm::TLM_OK_RESPONSE);
  }
}

bool sri::get_direct_mem_ptr(int n, PRAZOR_GP_T& trans, tlm::tlm_dmi& dd) {
  
  // MP: To be implemented (TODO)
  assert(0);
}

void sri::peq_cb(PRAZOR_GP_T& trans, const tlm_phase& ph) {
  ml->lock();
  if(ph == END_REQ) {
    sri_msg* mm = 0;
    trans.get_extension(mm);
    assert(mm != 0);
    SRIDEBUG(fprintf(stdout, "%s: request satisfied by memory controller of node %d for cache addr 0x%lx at node %d; me %d\n", 
                     name(), mm->home_node, mm->cache_addr, mm->originating_node, node));
    assert(mm->originating_node == node);
    //assert(probe_msg_busy == false && ack_msgs_busy == false);
    // this means that the request was satisfied by the memory controller
    SRIDEBUG1(fprintf(stdout, "%s: request satisfied by memory controller for cache addr 0x%lx\n", 
		     name(), mm->cache_addr));		      
    assert(wait_tran_map.count(mm->cache_addr) != 0);
    PRAZOR_GP_T* wait_tran; sc_event* mem_req;
    tie(wait_tran, mem_req, tuples::ignore) = wait_tran_map[mm->cache_addr];
    //wait_tran_map.erase(mm->cache_addr);

    // if received transaction is error and error is address error response then llsc failed
    if(trans.is_response_error() 
       && trans.get_response_status() == tlm::TLM_ADDRESS_ERROR_RESPONSE) {
        wait_tran->set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
    }

    cache_miss_extension* cme = 0;
    wait_tran->get_extension(cme);
    if(cme != 0)
      cme->resp = CT_MISS_DIRTY;

    ml->unlock();
    /*if(trans.is_response_error()) {
      char txt[256];
      snprintf(txt, 256, "%s: Error from secondary access addr = %llx, response status = %s", 
	       name(), trans.get_address(), trans.get_response_string().c_str());
      SC_REPORT_ERROR(__FILE__, txt);
      }*/
    
    mem_req->notify();
    //ml->lock();
    //notify_parallel_cache_lines(trans);
    //ml->unlock();

    // release transaction that arrived from DRAM controller
    trans.release();
  }
  else if(ph == BEGIN_REQ) {
    probe_msg* probem = 0;
    trans.get_extension(probem);
    ack_msg* ackm = 0;
    trans.get_extension(ackm);
    if(probem != 0 && ackm != 0)
      assert(0);
    if(probem) {

      if(trans.is_write()) {
	SRIDEBUG(fprintf(stdout, "%s: XXX addr = 0x%lx, %d, %d\n", name(), probem->cache_addr, probe_msg_busy, probe_requests.size()));
      }
      if(probe_msg_busy) {
	SRIDEBUG(fprintf(stdout, 
			  "%s: XXX already is processing a PROBE, pausing 0x%lx; size(%d)\n", 
			  name(), probem->cache_addr, probe_requests.size()));			 
	probe_requests.push(&trans);
	ml->unlock();
	return;
      }
      else {
	SRIDEBUG(fprintf(stdout, "%s: processing PROBE from %d; 0x%lx\n", 
			 name(), probem->source_node, probem->cache_addr));
	probe_msg_busy = true;
	probe_ack_received = init_socket.size() - 1;
	probe_addr = probem->cache_addr;
	active_probe_invalidation = !probem->real_write; 
	if(probem->dram)
	  probe_ack_received++;

	SRIDEBUG(fprintf(stdout, "%s: need to receive %d acks; %d\n", name(), probe_ack_received, probem->dram));

	// check to see if we have already received DRAM response
	if(dram_probe_asynch_msg.count(probem->cache_addr) != 0) {
	  // safety check
	  assert(probem->dram);
	  dram_ack_data_tran = dram_probe_asynch_msg[probem->cache_addr];
	  dram_probe_asynch_msg.erase(probem->cache_addr);

	  probe_ack_received--;
	}

      }
      
      ml->unlock();

      llsc_extension *llsc = NULL;
      trans.get_extension(llsc);

      if(!probe_ack_received) {
	assert(init_socket.size() == 1);
	ml->lock();
	probe_addr = -1;
	probe_msg_busy = false;
	active_probe_invalidation = false;
	if(probe_requests.size() > 0) {
	  PRAZOR_GP_T* tt = probe_requests.front();
	  tlm::tlm_phase ph = tlm::BEGIN_REQ;
	  sc_time delay = SC_ZERO_TIME;
	  probe_requests.pop();
	  m_peq.notify(*tt, ph, delay);
	}

	ml->unlock();
      }

      // forwarding probe to caches and acquiring it
      if(trans.is_write() && 
	 (init_socket.size() == 1) &&
	 (probem->originator_node == node)) {
	// SRI is not connected to caches, only to the
	// crossbar so we should immediately send 
	// ack message to the originator
	ack_msg* ack = new ack_msg;
	ack->dest_node = probem->source_node;
	ack->home_node = probem->home_node;
	ack->orig_node = node;
	SRIDEBUG(fprintf(stdout,
			 "%s: sending ack write no caches to %d; orig %d, home %d, me %d\n",
			 name(), ack->dest_node, probem->originator_node, ack->home_node, node));

	ack->addr = probem->cache_addr;
	ack->ty = ack_msg::UNKNOWN;

	PRAZOR_GP_T* ackm = m_mm->allocate();	

	ackm->set_extension<ack_msg>(ack);
	ackm->acquire();

	ackm->set_data_length(0);
	ackm->set_byte_enable_length(0);
	ackm->set_data_ptr(0);
	ackm->set_byte_enable_ptr(0);

	tlm_phase ph = BEGIN_REQ;
	sc_time delay = SC_ZERO_TIME;

	init_socket[init_socket.size() - 1]->nb_transport_fw(*ackm, ph, delay);
      }

      for(int i = 0; i < init_socket.size() - 1; i++) {
	tlm_phase ph = BEGIN_REQ;
	sc_time delay = SC_ZERO_TIME;
	
#if MOESI_TIMING	
	// here we are crossing clock boundary from northbridge to processor
	AUGMENT_LT_DELAY(trans.ltd, delay,  m_src_clk_cycles * m_period
			 + m_dest_clk_cycles * m_core_period);
#endif

	/*trans.acquire();
	  init_socket[i]->nb_transport_fw(trans, ph, delay);*/
	
	probe_msg* probem_clone = probem->clone();
	PRAZOR_GP_T* trans_clone = m_mm->allocate();
	
	trans_clone->acquire();
	
	// set extensions
	trans_clone->set_extension<llsc_extension>(llsc);
	trans_clone->set_extension<probe_msg>(probem_clone);
	
	trans_clone->set_command(trans.get_command());
	trans_clone->set_data_length(trans.get_data_length());
	trans_clone->set_byte_enable_ptr(0);
	trans_clone->set_data_ptr(0);
	trans_clone->set_address(trans.get_address());
	
	SRIDEBUG1(fprintf(stdout,
			  "%s: sending probe to cache on socket %d for 0x%lx\n",
			  name(), i, probem_clone->cache_addr));
	
	init_socket[i]->nb_transport_fw(*trans_clone, ph, delay);
      }
      
      trans.release();
    }
    else if(ackm) {
      bool done = false;
      SRIDEBUG(fprintf(stdout, 
		       "%s: Receiving ACK message; %d, 0x%lx, %d, 0x%lx, %d, %d\n", 
		       name(), 
		       probe_msg_busy, 
		       probe_addr, 
		       ackm->ty, 
		       ackm->addr,
		       ackm->orig_node,
		       ackm->dest_node));

      // forward the acknolwedgment to the requesting core
      if(ackm->orig_node == -1) {	
	// check to see if what we have received is from DRAM 
	// and if we are not currently
	// processing probe or if we are processing it is not the same address
	if(!probe_msg_busy
	   || (probe_msg_busy && probe_addr != ackm->addr)) {
	  // safety check
	  assert(ackm->ty == ack_msg::DRAM);
	  assert(dram_probe_asynch_msg.count(ackm->addr) == 0);
	  
	  ackm->orig_node = node;
	  ackm->ty = ack_msg::UNKNOWN;

	  dram_probe_asynch_msg[ackm->addr] = &trans;

	  return;
	}

	assert(probe_msg_busy);

	// grouping acks from caches
	probe_ack_received--;
	bool release = true;
	if(trans.get_data_ptr() != 0) {
	  if(ackm->ty == ack_msg::CACHE) {
	    SRIDEBUG1(fprintf(stdout, "%s: storing CACHE ack\n", name()));
	    cache_ack_data_tran = &trans;
	    release = false;
	  }
	  if(ackm->ty == ack_msg::DRAM) {
	    SRIDEBUG1(fprintf(stdout, "%s: storing DRAM ack\n", name()));
	    dram_ack_data_tran = &trans;
	    release = false;
	  }
	  if(ackm->ty == ack_msg::WB) {
	    SRIDEBUG1(fprintf(stdout, "%s: storing WB ack\n", name()));
	    wb_ack_data_tran = &trans;
	    release = false;
	  }

	  ackm->orig_node = node;
	  ackm->ty  = ack_msg::UNKNOWN;
	}
	done = probe_ack_received == 0;
	SRIDEBUG(fprintf(stdout, "%s: probe ack left to recieve %d\n", name(), probe_ack_received));
	if(done) {	  
	  ml->unlock();
	  
	  // all ACK messages have been recieved from caches
	  // therefore we have moved between clock boundaries
	  // from processor to northbridge
	  sc_time delay = SC_ZERO_TIME;
#if MOESI_TIMING
	  AUGMENT_LT_DELAY(trans.ltd, delay,  m_core_period * m_src_clk_cycles
			   + m_period * m_dest_clk_cycles);
#endif

	  if(ackm->write 
	     && ackm->dest_node != node
	     // do not disown memory if request is just invalidation from WB
	     && !active_probe_invalidation) {
	    // if this node owns this cache line and it is write
	    // then we have to remove it from its address table
	    // as it will now be owned by node who wrote to it
	    m_addr_table_lock->lock();
	    if(m_migrate_memory && address_table.find(ackm->addr) != address_table.end()) {
	      //printf("1: removing address 0x%lx from node %d, owned now by %d\n", ackm->addr, this->node, ackm->dest_node);
	      address_table.erase(ackm->addr);
	      assert(address_map.count(ackm->addr) != 0);
	      u64_t a = address_map[ackm->addr];
	      address_map.erase(ackm->addr);
	      empty_addr.push_back(a);
	      SRIDEBUG(fprintf(stdout, 
			       "%s: cache address is 0x%lx NOT OWNED by %d anymore; %d; 0x%lx; %d\n",
			       name(), ackm->addr, node, trans.get_data_length(), a, ackm->dest_node));

	      if(remote_core_owners.count(ackm->dest_node) == 0)
		remote_core_owners[ackm->dest_node] = 1;
	      else
		remote_core_owners[ackm->dest_node]++;

	      local_core_owners.erase(ackm->addr);

	      sri* adrt = this->next;
	      while(adrt->node != ackm->dest_node)
		adrt = adrt->next;
	      assert(adrt->address_table.find(ackm->addr) == adrt->address_table.end());
	      adrt->get_real_address(ackm->addr);
	      //printf("Node %d now owns address 0x%lx\n", adrt->node, ackm->addr);
	    }
	    m_addr_table_lock->unlock();
	  }
	  
	  if(!ackm->write && wb_ack_data_tran != 0) {
	    // we need to combine it with cache or DRAM
	    if(cache_ack_data_tran != 0) {
	      u8_t* dest = wb_ack_data_tran->get_data_ptr();
	      u8_t* src = cache_ack_data_tran->get_data_ptr();

	      u8_t* lanes = wb_ack_data_tran->get_byte_enable_ptr();

	      for(int i = 0; i < wb_ack_data_tran->get_byte_enable_length(); i++) 
		if(lanes[i] == 0x00) {
		  *(dest+i) = *(src+i);
		  lanes[i] = 0xFF;
		}
	    }
	    else if(dram_ack_data_tran != 0) {
	      u8_t* dest = wb_ack_data_tran->get_data_ptr();
	      u8_t* src = dram_ack_data_tran->get_data_ptr();

	      u8_t* lanes = wb_ack_data_tran->get_byte_enable_ptr();

	      for(int i = 0; i < wb_ack_data_tran->get_byte_enable_length(); i++) 
		if(lanes[i] == 0x00) {
		  *(dest+i) = *(src+i);
		  lanes[i] = 0xFF;
		}
		
	    }
	  }
	  
	  if(active_probe_invalidation 
	     && ((cache_ack_data_tran != 0
		  && cache_ack_data_tran->get_data_ptr() != 0)
		 || (wb_ack_data_tran != 0
		     && wb_ack_data_tran->get_data_ptr() != 0))) {
	    	    
	    PRAZOR_GP_T* tt = 0;
	    if(cache_ack_data_tran != 0)
	      tt = cache_ack_data_tran;
	    else if(wb_ack_data_tran != 0) 
	      tt = wb_ack_data_tran;

	    SRIDEBUG1(fprintf(stdout, "%s: address 0x%lx needs to be written: ", 
			     name(), ackm->addr);		    \
		     u8_t* data = tt->get_data_ptr();			\
		     for(int i = 0; i < tt->get_data_length(); i++)	\
		       fprintf(stdout, "%02x", data[i]);		\
		     fprintf(stdout, "\n"));	    

	    sri* adrt = next;
	    m_addr_table_lock->lock();
            if(m_migrate_memory) {
              while(adrt != this) {
	        if(adrt->address_table.find(ackm->addr) != adrt->address_table.end()) {
		  adrt->address_table.erase(ackm->addr);
		  //printf("2: removing address 0x%lx from node %d\n", ackm->addr, adrt->node);
                  assert(adrt->address_map.count(ackm->addr) != 0);
                  u64_t a = adrt->address_map[ackm->addr];
                  adrt->address_map.erase(ackm->addr);
                  adrt->empty_addr.push_back(a);
                  SRIDEBUG(fprintf(stdout, 
                                   "%s: cache address is 0x%lx NOT OWNED by %d anymore; %d; 0x%lx\n",
                                   name(), ackm->addr, adrt->node, trans.get_data_length(), a));

                  if(adrt->remote_core_owners.count(node) == 0)
                    adrt->remote_core_owners[node] = 1;
                  else
                    adrt->remote_core_owners[node]++;
		
		  adrt->local_core_owners.erase(ackm->addr);

		  break;
                }
	      adrt = adrt->next;
              }
	    
	      u64_t real_addr = 0;
	      if(address_table.find(ackm->addr) == address_table.end()) {
	        address_table.insert(ackm->addr);
	        real_addr = get_real_address(ackm->addr);
	        tt->set_address(real_addr);
	        local_core_owners.insert(ackm->addr);
	      }
	      else 
	        // safety check
	        assert(tt->get_address() == get_real_address(ackm->addr));
            }
	    m_addr_table_lock->unlock();

#if MOESI_TIMING
	    // access to memory mapping tables
	    AUGMENT_LT_DELAY(trans.ltd, delay,  m_mem_table_access_cycles * m_period);
#endif

	    ack_msg* mm = 0;
	    tt->get_extension(mm);
	    assert(mm != 0);
	    mm->evict_line = true;

	    tlm_phase ph = ACK_DRAM;
	    sc_time delay = SC_ZERO_TIME;
	    
	    init_socket[init_socket.size() - 1]->nb_transport_fw(*tt, 
								 ph, 
								 delay);
	  }
	  else {
	    PRAZOR_GP_T* to_send = &trans;	  
	    if(wb_ack_data_tran) {
                // If we have cache hierarchy (say L1 and L2) then it is
                // possible for core A to have cache line for address X
                // present in L1 and for core B to have the cache line
                // for the same address X present in write buffer of L2.
                // This happens when we snoop for data from core A when
                // cache line for address X is present in core B's L1 in
                // modified state. Then according to our algorithm we
                // will copy the content of B's L1 line to A's L1 cache,
                // set line to shared and evict B's L1 line to L2 write
                // buffer. When this situation arrives we need to send
                // response back that line exists in cache and not in
                if(cache_ack_data_tran) {
                    ackm->resp = ack_msg::CACHE;
                    to_send = cache_ack_data_tran;
                }
                else {
                    ackm->resp = ack_msg::WB;
                    to_send = wb_ack_data_tran;
                }
	      SRIDEBUG1(fprintf(stdout, "%s: sending WB ack\n", name()));
	    }
	    else if(cache_ack_data_tran) { // cache has preference 
	      ackm->resp = ack_msg::CACHE;
	      to_send = cache_ack_data_tran;
	      SRIDEBUG1(fprintf(stdout, "%s: sending cache ack\n", name()));
	    }
	    else if(dram_ack_data_tran) {
	      ackm->resp = ack_msg::DRAM;
	      to_send = dram_ack_data_tran;
	      SRIDEBUG1(fprintf(stdout, "%s: sending DRAM ack\n", name()));
	    }
	    else {
	      ackm->ty = ack_msg::UNKNOWN;
	    }
	    ackm->orig_node = node;
	    
	    tlm_phase ph = BEGIN_REQ;
	    sc_time delay = SC_ZERO_TIME;
	    
	    ack_msg* ackm_t = 0;
	    to_send->get_extension(ackm_t);
	    assert(ackm_t != 0);
	    SRIDEBUG1(fprintf(stdout, "%s: sending ACK message to %d\n",
			     name(), ackm_t->dest_node));
	    
	    init_socket[init_socket.size() - 1]->nb_transport_fw(*to_send, ph, delay);
	  
	    ml->lock();
	    if(to_send != &trans 
	       && &trans != cache_ack_data_tran
	       && &trans != dram_ack_data_tran
	       && &trans != wb_ack_data_tran) {
	      trans.release();
	    }

	    if(cache_ack_data_tran != 0 && to_send != cache_ack_data_tran) {
	      cache_ack_data_tran->release();
	    }
	    cache_ack_data_tran = 0;
	    if(dram_ack_data_tran != 0 && to_send != dram_ack_data_tran) {
	      dram_ack_data_tran->release();
	    }
	    dram_ack_data_tran = 0;
	    if(wb_ack_data_tran != 0 && to_send != wb_ack_data_tran) {
	      wb_ack_data_tran->release();
	    }
	    wb_ack_data_tran = 0;

	    probe_ack_received = 0;
	    probe_msg_busy = false;
	    probe_addr = -1;
	    active_probe_invalidation = false;
	    if(probe_requests.size() > 0) {
	      PRAZOR_GP_T* tt = probe_requests.front();
	      tlm::tlm_phase ph = tlm::BEGIN_REQ;
	      sc_time delay = SC_ZERO_TIME;
	      probe_requests.pop();
	      m_peq.notify(*tt, ph, delay);
	    }
	    ml->unlock();
	  }
	}
	else {
	  ml->unlock();
	  if(release)
	    trans.release();
	}
      }
      else if(ackm->dest_node == node && ackm->orig_node != -1) {
	SRIDEBUG(fprintf(stdout, "%s: address 0x%lx\n", name(), ackm->addr));
	assert(ack_msgs.count(ackm->addr) != 0);
	assert(ack_response.count(ackm->addr) != 0);
	assert(ack_old_mem_write.count(ackm->addr) != 0);
	ack_msgs[ackm->addr]++;
	assert(ack_msgs_bits.count(ackm->addr) != 0);
	assert(ackm->orig_node >=0 && ackm->orig_node < m_total_sockets);
	ack_msgs_bits[ackm->addr]->set(ackm->orig_node, true);
	SRIDEBUG(fprintf(stdout, "%s: received ACK message from %d for address 0x%lx - total: %d, received %d\n", 
			 name(), ackm->orig_node, ackm->addr, m_total_sockets, ack_msgs[ackm->addr]));

	assert(wait_tran_map.count(ackm->addr) != 0);
	PRAZOR_GP_T* wait_tran;
	tie(wait_tran, tuples::ignore, tuples::ignore) = wait_tran_map[ackm->addr];

	if(trans.get_data_ptr() != 0) {
	  SRIDEBUG(fprintf(stdout, "%s: response arrives %d, old %d for line 0x%lx\n", 
			   name(), ackm->resp, ack_response[ackm->addr], ackm->addr));

	  SRIDEBUG(fprintf(stdout, "data from response is: ");		\
		   u8_t* dd = trans.get_data_ptr();
		   for(int i = 0; i < trans.get_data_length(); i++)	\
		     fprintf(stdout, "%02x", dd[i]);			\
		   fprintf(stdout, "\n"));

	  // we should never have responses from two caches when writing
	  if(wait_tran->is_write() 
	     && ack_response[ackm->addr] == ack_msg::CACHE 
	     && ackm->resp == ack_msg::CACHE)
	    assert(0);

	  if(wait_tran->is_write() && ackm->resp == ack_msg::WB) {
	    // we can receive response from write buffer if we are evicting
	    // a line from the cache and at the same time some other core is
	    // writing to the same line, since the line was evicted the other
	    // core wouldn't see it and will happily write to write buffer and then
	    // when asked about it will return the data although data is newer
	  }

	  if(wait_tran->is_read() 
	     && ack_response[ackm->addr] == ack_msg::WB
	     && ackm->resp == ack_msg::CACHE)
	    assert(0); // this shouldn't occur (?)

	  // responses from caches always have priority
	  if(ack_response[ackm->addr] == ack_msg::UNKNOWN 
	     || (ack_response[ackm->addr] == ack_msg::DRAM 
		 && (ackm->resp == ack_msg::CACHE
		     || ackm->resp == ack_msg::WB))
	     || ackm->resp == ack_msg::WB
	     || (ack_response[ackm->addr] == ack_msg::WB
		 && ackm->resp == ack_msg::DRAM)) {
	    
	    if(wait_tran->is_read()) {
	      u8_t* dest = wait_tran->get_data_ptr();
	      u8_t* src = trans.get_data_ptr();

	      if(ackm->resp == ack_msg::WB) {
		assert(ack_wb_lanes.count(ackm->addr) == 0);
		ack_wb_lanes[ackm->addr] = new u8_t[trans.get_byte_enable_length()];
		u8_t* lanes = trans.get_byte_enable_ptr();
		for(int i = 0; i < trans.get_byte_enable_length(); i++) {
		  if(lanes[i] == 0xFF) {
		    *(dest+i) = *(src+i);
		  }
		  ack_wb_lanes[ackm->addr][i] = lanes[i];
		}
		
	      }
	      else {
		if(ack_response[ackm->addr] == ack_msg::WB) {
		  assert(ack_wb_lanes.count(ackm->addr) != 0);
		  for(int i = 0; i < trans.get_data_length(); i++)
		    if(ack_wb_lanes[ackm->addr][i] == 0x00)
		      *(dest+i) = *(src+i);
		}
		else
		  for(int i = 0; i < trans.get_data_length(); i++)
		    *(dest+i) = *(src+i);
	      }
	    }
	    else if(wait_tran->is_write()) {
	      if(trans.get_data_length() > 0 && trans.get_data_ptr() != 0) {
		u8_t* src = trans.get_data_ptr();
		if(ackm->resp != ack_msg::WB) {
		  SRIDEBUG1(fprintf(stdout, "%s: from other memory for 0x%lx: ", name(), ackm->addr); \
			   for(int i = 0; i < trans.get_data_length(); i++) \
			     fprintf(stdout, "%02x", src[i]);		\
			   fprintf(stdout, "\n"));
		  
		  if(ack_response[ackm->addr] == ack_msg::DRAM) {
		    assert(ack_old_mem_write[ackm->addr] != 0);
		    SRIDEBUG1(fprintf(stdout, "%s: replacing request for 0x%lx from DRAM by cache\n",
				     name(), ackm->addr));
		    delete ack_old_mem_write[ackm->addr];
		    ack_old_mem_write[ackm->addr] = 0;
		  }
		  assert(ack_old_mem_write[ackm->addr] == 0);
		  ack_old_mem_write[ackm->addr] = new u8_t[trans.get_data_length()];
		  for(int i = 0; i < trans.get_data_length(); i++)
		  ack_old_mem_write[ackm->addr][i] = src[i];
		}
		else {
		  u8_t* src = wait_tran->get_data_ptr();
		  u8_t* flags = wait_tran->get_byte_enable_ptr();
		  if(flags == 0) {
                      SRIDEBUG1(for(int i = 0; i < wait_tran->get_byte_enable_length(); i++) \
                                    printf("%02x", flags[i]));
		    assert(flags == 0);
		  }
		  u8_t* wb_data = trans.get_data_ptr();
		  u8_t* wb_flags = trans.get_byte_enable_ptr();
		  for(int i = 0; i < wait_tran->get_data_length(); i++) 
		    if(wb_flags[i] == 0xFF) 
		      src[i] = wb_data[i];
		}
	      }
	    }
	    else
	      assert(0);
	    
	    if(ackm->resp > ack_response[ackm->addr]
	       && !(wait_tran->is_write() && ackm->resp == ack_msg::WB))
	      ack_response[ackm->addr] = ackm->resp;
	  }
	}
	
	// need to wake up
	if(ack_msgs[ackm->addr] == m_total_sockets 
	   || (init_socket.size() == 1 && ack_msgs[ackm->addr] == 1)) {
	  assert(ack_msgs_bits[ackm->addr]->count() == m_total_sockets 
		 || (init_socket.size() == 1 && ack_msgs_bits[ackm->addr]->count() == 1));
	  assert(wait_tran_map.count(ackm->addr) != 0);
	  PRAZOR_GP_T* wait_tran; sc_event* mem_req; int home_node;
	  tie(wait_tran, mem_req, home_node) = wait_tran_map[ackm->addr];
	  assert(home_node != -1);
	  // send message to home tile to inform that all 
	  // messages has been passed around
	  unblock_msg* mm = new unblock_msg;
	  mm->dest_node = ackm->home_node;
	  mm->creator_node = node;
	  //PRAZOR_GP_T* unblockm = unblock_msg_mm.allocate();
	  PRAZOR_GP_T* unblockm = m_mm->allocate();
	  unblockm->acquire();
	  
	  unblockm->set_extension<unblock_msg>(mm);

	  llsc_extension* llsc = NULL;
	  wait_tran->get_extension(llsc);
	  unblockm->set_extension<llsc_extension>(llsc);

	  tlm_phase ph = BEGIN_REQ;
	  sc_time delay = SC_ZERO_TIME;
	  SRIDEBUG1(fprintf(stdout, "%s: sending unblock message to %d\n", name(), mm->dest_node));

	  init_socket[init_socket.size() - 1]->nb_transport_fw(*unblockm, ph, delay);

	  unblock_msg* mm_test = 0;
	  unblockm->get_extension(mm_test);
	  if(mm_test == 0)
	    assert(0);

	  // tell to the cache where the read request was satisfied
	  if(wait_tran->is_read() && init_socket.size() > 1) {
	    assert(ack_response[ackm->addr] != ack_msg::UNKNOWN);
	    cache_miss_extension* w = 0;
	    wait_tran->get_extension(w);
            assert(w != 0);
            if(ack_response[ackm->addr] == ack_msg::CACHE) w->resp = CT_MISS_SHARED;
            else if(ack_response[ackm->addr] == ack_msg::DRAM) w->resp = CT_MISS_EXCLUSIVE;
            else if(ack_response[ackm->addr] == ack_msg::WB) w->resp = CT_MISS_DIRTY;
            else assert(0);	                
	  }

	  if(ack_wb_lanes.count(ackm->addr) != 0) {
	    delete ack_wb_lanes[ackm->addr];
	    ack_wb_lanes.erase(ackm->addr);
	  }

	  ack_response[ackm->addr] = ack_msg::UNKNOWN;
	  // if we are writing to cache line then this node needs to own
	  // the cache line from now on (only if m_migrate_memory is true)
	  if(wait_tran->is_write()) {
	    if(wait_tran->get_data_length() > 0
	       && wait_tran->get_data_ptr() != 0) {
	      
	      if(wait_tran->get_byte_enable_length() == 0 
		 && wait_tran->get_byte_enable_ptr() == 0) {
		// do not need to concanate with old memory as all flags are set
		// or if nothing was read from the main memory
		SRIDEBUG1(fprintf(stdout, "%s: we are not concatenating\n", name()));
	      }
	      else {
		if(ack_old_mem_write[ackm->addr] != 0) {
		  assert(wait_tran->get_byte_enable_length() == wait_tran->get_data_length());
		  u8_t* dd = wait_tran->get_data_ptr();
		  u8_t* be = wait_tran->get_byte_enable_ptr();
		  SRIDEBUG1(fprintf(stdout, "data in wait tran for 0x%lx: ", ackm->addr);
			   for(int i = 0; i < wait_tran->get_data_length(); i++)
			     fprintf(stdout, "%02x", dd[i]);
			   fprintf(stdout, "\n"));
		  for(int i = 0; i < wait_tran->get_data_length(); i++) {
		    if(be[i] == 0x00)
		      dd[i] = ack_old_mem_write[ackm->addr][i];
		    be[i] = 0xFF;
		  }
		}
	      }
	      
	      if(ack_old_mem_write[ackm->addr] != 0) {
		delete ack_old_mem_write[ackm->addr];
		ack_old_mem_write[ackm->addr] = 0;
	      }
	      
	      m_addr_table_lock->lock();
              /*if(m_migrate_memory) {
  	        if(address_table.find(ackm->addr) == address_table.end()) {
		  SRIDEBUG(fprintf(stdout, "%s: %d NOW OWNS cache line 0x%lx\n",
                                   name(), node, ackm->addr));
                  address_table.insert(ackm->addr);
                  local_core_owners.insert(ackm->addr);
                  

#if MOESI_TIMING
                // updates memory tables
              AUGMENT_LT_DELAY(trans.ltd, delay,  m_mem_table_access_cycles * m_period;
#endif
}*/
              sri* adrt = this;
              u64_t real_addr = -1;
              int owner = -1;
              do {
                if(adrt->address_table.find(ackm->addr) != adrt->address_table.end()) {
                  real_addr = adrt->get_real_address(ackm->addr);
                  owner = adrt->node;
                  break;
                }
                adrt = adrt->next;
              } while(adrt != this);

              assert(real_addr != -1);
              assert(owner != -1);
              m_addr_table_lock->unlock();
	      // once we have found what is it real address we need to
	      // forward the transaction to the main memory to do the writing
	      sri_msg* sri_mm = new sri_msg;
	      sri_mm->home_node = owner;
              sri_mm->originating_node = node;
	      sri_mm->cache_addr = ackm->addr;
	      
	      //PRAZOR_GP_T* srim = sri_msg_mm.allocate();
	      PRAZOR_GP_T* srim = m_mm->allocate();
	      srim->acquire();

	      srim->set_extension<sri_msg>(sri_mm);
              llsc_extension* llsc = NULL;
              wait_tran->get_extension(llsc);
              srim->set_extension<llsc_extension>(llsc);

              SRIDEBUG(fprintf(stdout, "tid = %d-%s: wants to write to main memory at addr 0x%lx; node %d, owner %d, llsc = %d\n",
                               sc_get_current_process_b()->proc_id, name(), real_addr, node, owner, llsc != NULL));
	      
	      srim->set_data_length(wait_tran->get_data_length());
	      srim->set_byte_enable_length(wait_tran->get_byte_enable_length());
	      srim->set_byte_enable_ptr(wait_tran->get_byte_enable_ptr()); 
	      srim->set_streaming_width(wait_tran->get_streaming_width());
	      srim->set_write();
	      srim->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE );
	      srim->set_address(real_addr);
	      srim->set_data_ptr(wait_tran->get_data_ptr());
	      
	      SRIDEBUG(u8_t* test = srim->get_data_ptr();		\
		       fprintf(stdout, "%s: will write to main memory at addr 0x%lx: ", 
			       name(), real_addr);			\
		       for(int i = 0; i < srim->get_data_length(); i++)	\
			 fprintf(stdout, "%02x", test[i]);		\
		       test = srim->get_byte_enable_ptr();		\
		       fprintf(stdout, " (flags): ");			\
		       for(int i = 0; i < srim->get_byte_enable_length(); i++) \
			 fprintf(stdout, "%02x", test[i]);		\
		       fprintf(stdout, "\n"));
	      
	      tlm_phase ph = BEGIN_REQ;
	      tlm_sync_enum s = init_socket[init_socket.size() - 1]->nb_transport_fw(*srim, ph, delay);
	    }
	    else {
	      // invalidating because we are writing to WB
	      // wake up
	      //wait_tran_map.erase(ackm->addr);
	      mem_req->notify();
	      //notify_parallel_cache_lines(*wait_tran);
	    }
	  }
	  else if(wait_tran->is_read()) {
	    SRIDEBUG(u8_t* test = wait_tran->get_data_ptr();		\
		     fprintf(stdout, "%s: data to be read by waiting transaction: ", name()); \
		     for(int i = 0; i < wait_tran->get_data_length(); i++) \
		       fprintf(stdout, "%02x", test[i]);		\
		     fprintf(stdout, "\n"));

	    //wait_tran_map.erase(ackm->addr);
	    mem_req->notify();
	    //notify_parallel_cache_lines(*wait_tran);
	  }
	  else
	    assert(0);

	  SRIDEBUG1(fprintf(stdout, "%s: recevied all ACK messages for 0x%lx\n", name(), ackm->addr));
	}
	
	// release transaction
	trans.release();

	ml->unlock();
      }
      else {
	SRIDEBUG1(fprintf(stdout, "%s: dest_node = %d, orig_node = %d, node = %d\n",
			 name(), ackm->dest_node, ackm->orig_node, node));
	assert(0);
      }
    }
    else {
      SRIDEBUG1(fprintf(stdout, "%s: has unknown message\n", name()));
      assert(0);
    }
  }
  else if(ph == ACK_DRAM) {
    ack_msg* mm = 0;
    trans.get_extension(mm);
    assert(mm != 0);

    mm->evict_line = false;
    mm->resp = ack_msg::DRAM;
    mm->orig_node = node;

    tlm_phase ph = BEGIN_REQ;
    sc_time delay = SC_ZERO_TIME;

    init_socket[init_socket.size() - 1]->nb_transport_fw(trans, ph, delay);

    ml->lock();
    if(cache_ack_data_tran != 0 && cache_ack_data_tran != &trans)
      cache_ack_data_tran->release();
    cache_ack_data_tran = 0;
    if(dram_ack_data_tran != 0 && dram_ack_data_tran != &trans)
      dram_ack_data_tran->release();
    dram_ack_data_tran = 0;
    if(wb_ack_data_tran != 0 && wb_ack_data_tran != &trans)
      wb_ack_data_tran->release();
    wb_ack_data_tran = 0;
    probe_ack_received = 0;
    probe_msg_busy = false;
    probe_addr = -1;
    active_probe_invalidation = false;
    if(probe_requests.size() > 0) {
      PRAZOR_GP_T* tt = probe_requests.front();
      tlm::tlm_phase ph = tlm::BEGIN_REQ;
      sc_time delay = SC_ZERO_TIME;
      probe_requests.pop();
      m_peq.notify(*tt, ph, delay);
    }
    ml->unlock();
  }
  else
    assert(0);
}

tlm_sync_enum sri::nb_transport_fw(int id, 
				   PRAZOR_GP_T& trans,
				   tlm_phase& phase,
				   sc_time& delay) {
  // check to see if message is ack message
  // if it is then add what is the node id
  m_peq.notify(trans, phase, delay);
  return TLM_ACCEPTED;
}


tlm_sync_enum sri::nb_transport_bw(int id,
				   PRAZOR_GP_T& trans,
				   tlm_phase& phase,
				   sc_time& delay) {
  m_peq.notify(trans, phase, delay);
  return TLM_ACCEPTED;
}

void sri::notify_parallel_cache_lines(PRAZOR_GP_T& trans) {
  while(!cache_line_par.empty()) {
    PRAZOR_GP_T* t; sc_event* e;
    tie(t, e) = cache_line_par.front();
    cache_line_par.pop();
    
    /*t->set_response_status(trans.get_response_status());
    if(trans.get_data_ptr() != 0) {
      u8_t* dest = t->get_data_ptr();
      u8_t* src = trans.get_data_ptr();
      assert(t->get_data_length() == trans.get_data_length());
      for(int i = 0; i < trans.get_data_length(); i++)
	*(dest+i) = *(src+i);
    }*/

    e->notify();
  }
}

void sri::stat_report(const char* msg, FILE* f, bool resetf, bool no_header) {
  fprintf(f, "SRI %d\n", node);
  // write invalidation first
  fprintf(f, "Write invalidation: ");
  if(node_latency_time.count(WRITE_INVALIDATION) != 0)
    for(std::map<u64_t, struct stats>::iterator i = node_latency_time[WRITE_INVALIDATION].begin();
	i != node_latency_time[WRITE_INVALIDATION].end();
	i++) {
      sc_time avg = i->second.latency / i->second.n;
      double avg_in_ns = avg.to_seconds()*1e9;
      fprintf(f, "%ld:(%d,%lf) ", i->first, i->second.n, avg_in_ns);
    }
  fprintf(f, "\n");

  // then data write
  fprintf(f, "Write data: ");
  if(node_latency_time.count(WRITE_DATA) != 0)
    for(std::map<u64_t, struct stats>::iterator i = node_latency_time[WRITE_DATA].begin();
	i != node_latency_time[WRITE_DATA].end();
	i++) {
      sc_time avg = i->second.latency / i->second.n;
      double avg_in_ns = avg.to_seconds()*1e9;
      fprintf(f, "%ld:(%d,%lf) ", i->first, i->second.n, avg_in_ns);
    }
  fprintf(f, "\n");

  // adn finally read data
  fprintf(f, "Read data: ");
  if(node_latency_time.count(READ_DATA) != 0)
    for(std::map<u64_t, struct stats>::iterator i = node_latency_time[READ_DATA].begin();
	i != node_latency_time[READ_DATA].end();
	i++) {
      sc_time avg = i->second.latency / i->second.n;
      double avg_in_ns = avg.to_seconds()*1e9;
      fprintf(f, "%ld:(%d,%lf) ", i->first, i->second.n, avg_in_ns);
    }
  fprintf(f, "\n");  

  fprintf(f, "Disowned lines: ");
  fprintf(f, "T = %ld -- ", local_core_owners.size());
  for(std::map<u64_t, u64_t>::iterator i = remote_core_owners.begin();
      i != remote_core_owners.end();
      i++) {
    fprintf(f, "%ld:%ld ", i->first, i->second);
  }
  fprintf(f, "\n");

  if(resetf)
    clean();
}
