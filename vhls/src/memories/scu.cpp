// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2014 M Puzovic + D J Greaves

#include "scu.h"
#if TRACECOMM
#include "proc_id_extension.h"
#endif

using namespace std;
using namespace sc_core;
using namespace tlm;
using namespace tlm_utils;

scu::scu(sc_module_name name) :
  sc_module(name)
#ifdef TLM_POWER3
#if PW_TLM_PAYLOAD > 0
  , primary_bus_tracker(this),
  secondary_bus_tracker(this)  
#endif
#endif
{
    target_socket.register_b_transport(this, &scu::b_transport);

    snoop_socket = new multi_passthrough_initiator_socket<scu, 64, PW_TLM_TYPES>;
    snoop_socket->register_nb_transport_bw(this, &scu::nb_transport_bw);
    
    initiator_socket = new multi_passthrough_initiator_socket<scu, 64, PW_TLM_TYPES>;

    read_locked_tid.store(-1);
}

void scu::init() {
    wait_events = (sc_event**)malloc(sizeof(sc_event*) * target_socket.size());
    for(int i = 0; i < target_socket.size(); i++)
        wait_events[i] = NULL;

    served_requests = (atomic<u64_t>*)malloc(sizeof(sc_event*) * target_socket.size());
    for(int i = 0; i < target_socket.size(); i++) {
        served_requests[i].store(0);
    }

    m_stats = (stats_t*) malloc(sizeof(stats_t) * target_socket.size());
    for(int i = 0; i < target_socket.size(); i++) {
      m_stats[i].reset();
    }
}

void scu::b_transport(
    int id, 
    PRAZOR_GP_T &trans, 
    sc_time &delay_) {

    u64_t addr = trans.get_address();

    ccache_state_extension* c_ext = 0;
    trans.get_extension(c_ext);
    if(!c_ext) {
        // forward directly to secondary storage
	assert(initiator_socket);
        (*initiator_socket)->b_transport(trans, delay_);
	return;
    }

    assert(c_ext != 0);

    m_stats[id].total_requests.fetch_add(1);

    // NON-SNOOPING transactions, they simply forward
    // transaction to the secondary storage
    // BEGIN
    if(c_ext->trans == ccache::Read ||
       c_ext->trans == ccache::Write ||
       c_ext->trans == ccache::WriteBack) {
        m_stats[id].nonsnoop_requests.fetch_add(1);
        SCUTRC(addr, printf("SCU b_transport() -> non-snoop request for address 0x%lx from port %d\n",
                            addr, id));


        // forward
	assert(initiator_socket);
        (*initiator_socket)->b_transport(trans, delay_);
        
        return;
    }
    // END

    // SNOOPING transactions
    // BEGIN
    m_stats[id].snoop_requests.fetch_add(1);

    assert(snoop_socket);

    // this is allocated on stack so that we don't have
    // to deal with memory manager
    u8_t data[snoop_socket->size() - 1][trans.get_data_length()];

    // Forward requests to snooped caches except
    // the one that sent the request
    PRAZOR_GP_T* snoop_trans[snoop_socket->size() - 1];
    
    u64_t expected = 0;
    if(!(served_requests[id].compare_exchange_strong(expected, (snoop_socket->size() - 1)))) {
        printf("%s: SCU b_transport() -> There are still %ld unserved requests for %d size %d\n", name(), expected, id, snoop_socket->size());
        assert(0);
    }
    wait_events[id] = new sc_event;
    for(int i = 0, tranid = 0; i < snoop_socket->size(); ++i) {
        if(i == snooped_id_from_target(id)) 
            continue;

        ccache_state_extension* new_cext = new ccache_state_extension;
        new_cext->origin = id;
        new_cext->trans = c_ext->trans;
        snoop_trans[tranid] = prazor_gp_mm_t::instance()->allocate(); 
        snoop_trans[tranid]->set_auto_extension<ccache_state_extension>(new_cext);
        snoop_trans[tranid]->acquire();
        
        snoop_trans[tranid]->set_data_length(trans.get_data_length());
        snoop_trans[tranid]->set_byte_enable_length(0);
        snoop_trans[tranid]->set_byte_enable_ptr(0);
        snoop_trans[tranid]->set_streaming_width(trans.get_streaming_width());
        snoop_trans[tranid]->set_read();
        
        snoop_trans[tranid]->set_response_status(TLM_INCOMPLETE_RESPONSE);
        snoop_trans[tranid]->set_address(trans.get_address());
        snoop_trans[tranid]->set_data_ptr(&(data[tranid][0]));

	snoop_trans[tranid]->ltd = trans.ltd;
        
        tlm_phase ph = BEGIN_REQ;
        sc_time snoop_delay_ = SC_ZERO_TIME;
        ((*snoop_socket)[i])->nb_transport_fw(*(snoop_trans[tranid]), ph, snoop_delay_);
        ++tranid;
    }

    SCUTRC(addr, printf("SCU b_transport() -> waiting to receive back all snoop request for address 0x%lx for port %d\n",
                        addr, id));

    wait(*wait_events[id]);
    if(wait_events[id] != NULL) {
        delete wait_events[id];
        wait_events[id] = NULL;
    }

    /* Pick up the longest delay observed */
    trans.ltd = snoop_trans[0]->ltd;
    for(int i = 1; i < snoop_socket->size() - 1; ++i) 
      trans.ltd << snoop_trans[i]->ltd;
      
    SCUTRC(addr, printf("SCU b_transport() -> received back all snoop request for address 0x%lx for port %d\n",
                        addr, id));

    // snooped state
    ccache::state_t sstate = ccache::Invalid;
    int snoop_trans_idx = 0;
    for(; snoop_trans_idx < snoop_socket->size() - 1; ++snoop_trans_idx) {
      ccache_state_extension* c_ext_l = 0;
      snoop_trans[snoop_trans_idx]->get_extension(c_ext_l);
      assert(c_ext_l != 0);
      
      if(c_ext_l->state != ccache::Invalid) {
          sstate = c_ext_l->state;
          break;
      }

    }

    sc_time secondary_lookup_delay_ = SC_ZERO_TIME;
    switch(c_ext->trans) {
      case ccache::ReadShared: 
      case ccache::ReadUnique:
      case ccache::MakeUnique: {
	  switch(sstate) {
   	      case ccache::Invalid: {
		  if(c_ext->trans == ccache::ReadShared ||
		     c_ext->trans == ccache::ReadUnique)
 	              // need to do secondary lookup
		      secondary_operation(
			  id,
		          trans.get_address(), 
			  trans,
			  trans.get_data_ptr(),
			  true, 
			  secondary_lookup_delay_);
		  if(c_ext->trans == ccache::ReadShared)
		    c_ext->state = ccache::UniqueClean;
		  else
		    c_ext->state = ccache::UniqueDirty;

		  break;
	      }
	      case ccache::UniqueClean: 
	      case ccache::SharedClean:
	      {
		  // doesn't need to do anything just need
                  // to copy data to original transaction
		  memcpy(
		      trans.get_data_ptr(), 
		      snoop_trans[snoop_trans_idx]->get_data_ptr(),
		      trans.get_data_length());

                  SCUTRC(addr, printf("SCU::b_transport() -> snooped data is: "); \
                         u8_t* data = snoop_trans[snoop_trans_idx]->get_data_ptr(); \
                         u32_t length = snoop_trans[snoop_trans_idx]->get_data_length(); \
                         for(int i = 0; i < length; i++) \
                             printf("%02x ", data[i]);                    \
                         printf("\n"));
		  
		  if(c_ext->trans == ccache::ReadShared)
		    c_ext->state = ccache::SharedClean;
		  else
		    c_ext->state = ccache::UniqueDirty;

		  break;
	      }

 	      case ccache::UniqueDirty : {
		// need to pass data to secondary storage first
		secondary_operation(
		    id,
		    trans.get_address(),
		    trans,
		    snoop_trans[snoop_trans_idx]->get_data_ptr(),
		    false,
		    secondary_lookup_delay_);

  		  // now copy data
		  memcpy(
		      trans.get_data_ptr(), 
		      snoop_trans[snoop_trans_idx]->get_data_ptr(),
		      trans.get_data_length());

                  SCUTRC(addr, printf("SCU::b_transport() -> snooped data is: "); \
                         u8_t* data = snoop_trans[snoop_trans_idx]->get_data_ptr(); \
                         u32_t length = snoop_trans[snoop_trans_idx]->get_data_length(); \
                         for(int i = 0; i < length; i++)                \
                             printf("%02x ", data[i]);                    \
                         printf("\n"));


		  if(c_ext->trans == ccache::ReadShared)
		    c_ext->state = ccache::SharedClean;
		  else
		    c_ext->state = ccache::UniqueDirty;

		  break;
	      }

    	      default: {
	          assert(0); // unknown state
	      }
	  }

          break;
      }

      case ccache::MakeInvalid: {
	  // do not need to do anything
	  // just need to wait to update
	  // all cache lines to invalid in
	  // snooping caches
          break;
      }

      case ccache::Evict: {
          switch(sstate) {
              case ccache::UniqueDirty: {
                  // need to pass data to secondary storage first
                  secondary_operation(
                      id,
                      trans.get_address(),
                      trans,
                      snoop_trans[snoop_trans_idx]->get_data_ptr(),
                      false,
                      secondary_lookup_delay_);
                  break;
              }

              case ccache::UniqueClean:
              case ccache::SharedClean: 
              case ccache::Invalid: {
                  break;
              }

          }

          break;
      }

      default: {
          assert(0);
          break;
      }
    }
    // END

    /* Free */
    for(snoop_trans_idx = 0; snoop_trans_idx < snoop_socket->size() - 1; ++snoop_trans_idx) {
      snoop_trans[snoop_trans_idx]->release();
    }
    
    trans.set_response_status(tlm::TLM_OK_RESPONSE);
    
    return;
}

void scu::stat_report(const char *msg, FILE *fd, bool reset) {
    if(fd) {
        fprintf(fd, "Snoop id: %s\n\n", name());
	for(int i = 0; i < target_socket.size(); i++) {
  	    fprintf(fd, "In-port %3d requests:\n", i);
	    fprintf(fd, "---------------------");
	    fprintf(fd, "  Snoop    Non Snoop      Total      Secondary Read   Secondary Write\n");
	    fprintf(fd, "---------------------------------------------------------------------\n");
	    fprintf(fd, "%7d%14d%11d%20d%18d\n",
		    m_stats[i].snoop_requests.load(),
		    m_stats[i].nonsnoop_requests.load(),
		    m_stats[i].total_requests.load(),
		    m_stats[i].secondary_read.load(),
		    m_stats[i].secondary_write.load());
	    if(reset) {
	      m_stats[i].reset();
	    }
	}
    }
}


tlm_sync_enum scu::nb_transport_bw(
    int id,
    PRAZOR_GP_T& trans,
    tlm_phase& phase,
    sc_time& ldelay) {

    ccache_state_extension* c_ext = 0;
    trans.get_extension(c_ext);
    assert(c_ext != 0);

    u64_t addr = trans.get_address();
    u64_t val = served_requests[c_ext->origin].fetch_sub(1);
    SCUTRC(addr, printf("SCU::b_transport() -> received back snoop request from port %d for port %d; left %ld\n",
			    id, c_ext->origin, val-1));

    if((val - 1) == 0)
        wait_events[c_ext->origin]->notify();

    return TLM_ACCEPTED;
}

void scu::secondary_operation(
    int id,
    u64_t addr,
    PRAZOR_GP_T& otrans,
    u8_t* data, 
    bool read, 
    sc_time& delay_) {

    PRAZOR_GP_T* trans;



    // allocate transaction
    trans = prazor_gp_mm_t::instance()->allocate(); 
    trans->acquire();

    trans->set_data_length(otrans.get_data_length());
    trans->set_byte_enable_length(0);
    trans->set_byte_enable_ptr(0);
    trans->set_streaming_width(otrans.get_streaming_width());

    if(read) {
#if TRACECOMM
      proc_id_extension* pid = 0;
      otrans.get_extension(pid);
      assert(pid != 0);
      trans->set_extension(pid);
#endif      
      trans->set_read();
    }
    else 
      trans->set_write();

    trans->set_response_status(TLM_INCOMPLETE_RESPONSE);
    trans->set_address(addr);
    trans->set_data_ptr(data);
    trans->ltd = otrans.ltd;

    if(read)
        m_stats[id].secondary_read.fetch_add(1);
    else
        m_stats[id].secondary_write.fetch_add(1);

    SCUTRC(addr, printf("SCU::secondary_operation() -> doing secondary operation for address 0x%lx\n",
                        addr));
    if(!read) {
        SCUTRC(addr, printf("SCU::secondary_operation() -> content of cache line to be written to secondary storage: "); \
               for(int i = 0; i < trans->get_data_length(); i++)        \
		   printf("%x ", data[i]);				\
               printf("\n"));
    }
    
    (*initiator_socket)->b_transport(*trans, delay_);
    if (trans->is_response_error()) {
        printf("ERROR: %s\n", trans->get_response_string().c_str());
	assert(0);
    }

    otrans.ltd = trans->ltd;
    trans->release();
}

// eof

