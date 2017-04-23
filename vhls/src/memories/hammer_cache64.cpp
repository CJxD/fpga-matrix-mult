// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2012 M Puzovic and D J Greaves
// University of Cambridge, Computer Laboratory.
// $Id: $

#include "hammer_cache64.h"
#include "llsc_extension.h"

using namespace tlm;
using namespace boost;
using namespace std;

hammer_cache64::hammer_cache64(sc_module_name mname, 
			       consistent_group* grp,
			       int bytes, 
			       int ways, 
			       int linesize, 
			       int secondary_width, 
			       sc_time clock_period,
			       int cache_level, 
			       cache_ty ty,
			       bool invalidate_lower,
			       uint32_t hit_cycles,
			       int bl,
			       on_miss_ty store_miss,
			       on_miss_ty write_miss,
			       write_policy_ty write_policy,
			       bool use_write_buffer,
			       int secondary_linesize,
			       int secondary_banks,
			       bool big_endian,
			       cc_protocol_ty protocol)
  : m_peq(this, &hammer_cache64::peq_cb),
    m_moesi_peq(this, &hammer_cache64::moesi_peq_cb),
    m_moesi_mm(0),
    moesi_init(0),
    moesi_targ(0),
    lower_level_data(0),
    cache_invalid_msg(false),
    cache_invalid_addr(-1),
    processing_probe(-1),
    consistent_cache64(mname, grp, bytes, ways, linesize, secondary_width, 
		       cache_level, ty, protocol, clock_period, hit_cycles, bl, 
		       store_miss, write_miss,
		       write_policy, use_write_buffer, secondary_linesize,
		       secondary_banks, big_endian) {
  assert(cc_protocol == MOESI_CC);

  if(invalidate_lower) {
    moesi_init = new tlm_utils::simple_initiator_socket<hammer_cache64, 64, PW_TLM_TYPES>;
    moesi_targ = new tlm_utils::simple_target_socket<hammer_cache64, 64, PW_TLM_TYPES>;

    assert(moesi_targ != 0);
    moesi_targ->register_nb_transport_fw(this, &hammer_cache64::moesi_nb_transport_fw);
    assert(moesi_init != 0);
    moesi_init->register_nb_transport_bw(this, &hammer_cache64::moesi_nb_transport_bw);
  }

  printf("Created hammer_cache %s\n", name());
}

void hammer_cache64::operate_miss_type(cacheway* c, 
				       int dmap, 
				       cache_miss_extension* cme) {
  cacheway::mesi_t state = c->Status[dmap];
  switch(state) {
    case cacheway::shared:
      cme->resp = CT_MISS_SHARED;
      break;
    case cacheway::exclusive:
      cme->resp = CT_MISS_EXCLUSIVE;
      break;
    case cacheway::modified:
      cme->resp = CT_MISS_DIRTY;
      break;
    case cacheway::owned:
      cme->resp = CT_MISS_OWNED;
      break;
    default:
      assert(0);
  }
}

void hammer_cache64::probe_message(PW_TLM_PAYTYPE &trans, 
				   sc_time &delay_, 
				   u64_t addr, 
				   probe_msg* probem)
{  
  int dmap = (addr >> (u64_t)geom.dmap_shift) & (geom.dmapping-1LLU);
  u64_t line_addr = addr & ~(geom.linesize-1);
  assert(line_addr == addr);
  int loffset  = (addr >> geom.loffset_shift) & (geom.word64s_per_line-1LLU);

  ack_msg* mm = new ack_msg;
  mm->dest_node = probem->source_node;
  mm->home_node = probem->home_node;
  mm->addr = addr;
  mm->ty = ack_msg::CACHE;
  mm->resp = ack_msg::CACHE;
  if(trans.is_write()) mm->write = true;
  
  //PW_TLM_PAYTYPE* ackm = ack_msg_mm.allocate();

  PW_TLM_PAYTYPE* ackm = m_moesi_mm->allocate();
  ackm->acquire();

  llsc_extension* llsc = NULL;
  trans.get_extension(llsc);
  ackm->set_extension<llsc_extension>(llsc);
      
  ackm->set_extension<ack_msg>(mm);

  ackm->set_data_ptr(0);
  ackm->set_byte_enable_ptr(0);

  u8_t* lower_level_lanes = trans.get_byte_enable_ptr();
  u8_t* lower_level_data = trans.get_data_ptr();
  
  if(trans.is_read()) ackm->set_read();
  else if(trans.is_write()) ackm->set_write();
  else assert(0);
  ackm->set_address(trans.get_address());

  // this cache has sent the invalidation message so
  // we shouldn't invalidate the lines from it
  if(cache_invalid_msg && cache_invalid_addr == addr) {
    tlm_phase ph = BEGIN_REQ;
    sc_time d = SC_ZERO_TIME;
    inita_socket->nb_transport_fw(*ackm, ph, d);

    return;
  }

  // first look to see if the line is in write buffer
  // but do not check if it is me who is requesting
  if(probem->source_node != probem->dest_node
     || (probem->source_node == probem->dest_node && busyAddr != line_addr)) {
    if(buf0) {
      buf0->wbl.lock();
      if(buf0->hit(line_addr, loffset) ||
	 (/*probem->source_node != probem->dest_node &&*/
	  buf0->m_addr == -1 
	  && buf0->evict_addr == line_addr)) {
	CTRC_EMPTY(printf("%s: probe has found the line 0x%lx in write buffer\n", 
		    name(), line_addr));

	bool evict_in_fly = false;
	if(/*probem->source_node != probem->dest_node &&*/
	   buf0->m_addr == -1 
	   && buf0->evict_addr == line_addr) {
	  CTRC_EMPTY(printf("%s: source=%d, dest=%d, busyAddr=0x%lx, line_addr=0x%lx\n",
		      name(), probem->source_node, probem->dest_node, busyAddr, line_addr));

	  // this core is already evicting write buffer but it is not completed
	  evict_in_fly = true;
	}
	
	u8_t* lanemask = new u8_t[geom.linesize];
	u8_t* data = new u8_t[geom.linesize];
	for(int i = 0; i < geom.linesize; i++) {
	  if(lower_level_lanes == 0 || lower_level_lanes[i] == 0x00) { // lower level cache has preference
	    if(buf0->Dirty[i])
	      lanemask[i] = 0xFF;
	    else
	      lanemask[i] = 0x00;
	    data[i] = *(buf0->cline.read8p(0)+i);
	  }
	  else if(lower_level_lanes != 0 && lower_level_lanes[i] == 0xFF) {
	    lanemask[i] = 0xFF;
	    data[i] = lower_level_data[i];
	  }
	}
	CTRC_EMPTY(fprintf(stdout, "%s: data from write buffer is: ", name());\
	     for(int i = 0; i < geom.linesize; i++)			\
	       fprintf(stdout, "%02x", *(buf0->cline.read8p(0)+i));	\
	     fprintf(stdout, " (lanes): ");				\
	     for(int i = 0; i < geom.linesize; i++)			\
	       fprintf(stdout, "%02x", lanemask[i]);			\
	     fprintf(stdout, "\n"));
	
	// we will clean it without writing to memory but this line that is read
	// will be put to modified state
	if(!evict_in_fly) {
	  buf0->m_addr = -1;
	  // we also need to clean dirty flags
	  CTRC_EMPTY(printf("%s: setting all dirty bits to false for 0x%lx\n", name(), line_addr));
	  for(int i = 0; i < buf0->linesize; i++) 
	    buf0->Dirty[i] = false;
	}
	buf0->wbl.unlock();
	// need to clean it

	ackm->set_byte_enable_ptr(lanemask);
	ackm->set_byte_enable_length(geom.linesize);
	ackm->set_data_length(geom.linesize);
	ackm->set_data_ptr(data);

	mm->ty = ack_msg::WB;
	mm->resp = ack_msg::WB;
	mm->wb_hit = true;

	trans.release();

	/* TODO: If we ever get to using this implementation
	 * of cache need to properly propagate delays */
	sc_time delay__ = SC_ZERO_TIME;  
	tlm_phase ph = BEGIN_REQ;
	inita_socket->nb_transport_fw(*ackm, ph, delay__);

	return;
      }
      else
	buf0->wbl.unlock();
    }
  }
  
  // look for the line in the cache
  u8_t *cline = 0;
  /* TODO: If we ever get to using this implementation
   * of cache need to properly propagate delays */
  sc_time max_lookup_delay__ = SC_ZERO_TIME;
  cacheway *cw = 0;
  for(int w = 0; w < geom.ways; w++) {
    cw = Cont[w];
    if(cw->lookup(line_addr, dmap, &cline, max_lookup_delay__))
      break;	
  }

  tlm_command cmd = trans.get_command();
  switch(cmd) {
  case TLM_READ_COMMAND: {
    if(cline) {

      CTRC_EMPTY(fprintf(stdout, "%s: probe read hit for line address = 0x%lx, %d\n", 
		 name(), addr, cw->Status[dmap]));

      if(cw->Status[dmap] == cacheway::modified) {
	if(cc_protocol != MOESI_CC)
	  assert(0); 
	// for MESI need to send cache line back to memory first and then copy
	// it to the requesting cache, this is not possible at the moment because
	// when we send message to memory controller via SRI which is at the
	// moment implemented as blocking
	else {
	  CTRC_EMPTY(fprintf(stdout, "%s: data for remote node (0x%lx) is: ", name(), addr); \
	       for(int i = 0; i < trans.get_data_length(); i++)		\
		 fprintf(stdout, "%02x", *(cline + i));			\
	       fprintf(stdout, "\n"));
	  cw->waylock->lock();
	  cw->Status[dmap] = cacheway::owned;
	  cw->waylock->unlock();
	}
      }
      else if(cw->Status[dmap] == cacheway::owned) {
        CTRC_EMPTY(fprintf(stdout, "%s: data for remote node is: ", name());	\
  	   for(int i = 0; i < trans.get_data_length(); i++)		\
	     fprintf(stdout, "%02x", *(cline + i));			\
	   fprintf(stdout, "\n"));
      }

      else if(cw->Status[dmap] == cacheway::exclusive) {
	cw->waylock->lock();
	cw->Status[dmap] = cacheway::shared;
	cw->waylock->unlock();
      }
      else if(cw->Status[dmap] == cacheway::invalid)
	assert(0); // error
  
      ackm->set_data_length(trans.get_data_length());    
      u8_t* new_data = new u8_t[trans.get_data_length()];
      for(int i = 0; i < trans.get_data_length(); i++) {
	if(lower_level_lanes == 0 || lower_level_lanes[i] == 0x00)
	  new_data[i] = cline[i];
	else
	  new_data[i] = lower_level_data[i];
      }
      CTRC_EMPTY(fprintf(stdout, "%s: data for remote node (0x%lx) is: ", name(), addr); \
	   for(int i = 0; i < trans.get_data_length(); i++)		\
	     fprintf(stdout, "%02x", new_data[i]);			\
	   fprintf(stdout, "\n"));

      ackm->set_data_ptr(new_data);
    }
    // the cache line is not in higher level cache
    // but was find in lower level
    else if(!cline && probem->lower_level_hit) {
      ackm->set_data_length(trans.get_data_length());    
      u8_t* new_data = new u8_t[trans.get_data_length()];
      for(int i = 0; i < trans.get_data_length(); i++) {
	if(lower_level_lanes[i] == 0x00)
          new_data[i] = 0x00;
	else
	  new_data[i] = lower_level_data[i];
      }
      CTRC_EMPTY(fprintf(stdout, "%s: data for remote node (0x%lx) is: ", name(), addr); \
	   for(int i = 0; i < trans.get_data_length(); i++)		\
	     fprintf(stdout, "%02x", new_data[i]);			\
	   fprintf(stdout, "\n"));

      ackm->set_data_ptr(new_data);
    }
    
    trans.release();

    tlm_phase ph = BEGIN_REQ;
    inita_socket->nb_transport_fw(*ackm, ph, max_lookup_delay);
    
    break;
  }
  case TLM_WRITE_COMMAND: {
    // since it is write if there is a line in this cache
    // we need to move it to invalidate state
    ackm->set_data_ptr(0);
    bool pass_data = false;

    if(cline) {
      // if line is modified or owned copy it 
      if(cw->Status[dmap] == cacheway::modified
	|| cw->Status[dmap] == cacheway::owned)
	pass_data = true;
	
      // discard the hit if it is me and it was not satisifed at the lower level
      if(probem->source_node == probem->dest_node 
	 && busyAddr == line_addr)
	ackm->set_data_ptr(0);
      else if(pass_data || probem->lower_level_hit) {
	  ackm->set_data_length(trans.get_data_length());
	  u8_t* new_data = new u8_t[trans.get_data_length()];
	  for(int i = 0; i < trans.get_data_length(); i++)
	    if(lower_level_lanes == 0 || lower_level_lanes[i] == 0x00)
	      new_data[i] = cline[i];
	    else
	      new_data[i] = lower_level_data[i];
	  
	  
	  ackm->set_data_ptr(new_data);
	  CTRC_EMPTY(fprintf(stdout, "%s: PROBE WRITE HIT 0x%lx\n", name(), line_addr));
	}
      /*else if(probem->lower_level_hit) {
	
	CTRC_EMPTY(fprintf(stdout, "%s: should clean the line 0x%lx: ", name(), probem->cache_addr); \
	     for(int i = 0; i < trans.get_data_length(); i++)		\
	       fprintf(stdout, "%02x", lower_level_data[i]);		\
	     fprintf(stdout, "; lanes = ");				\
	     for(int i = 0; i < trans.get_data_length(); i++)		\
	       fprintf(stdout, "%02x", lower_level_lanes[i]);		\
	     fprintf(stdout, "\n"));					

	assert(0);
	}*/
      
      
      // regardless of the current state of the cache line it has to go to invalid
      CTRC_EMPTY(fprintf(stdout, "%s: line 0x%lx has been invalidated\n", 
		   name(), line_addr));
      cw->waylock->lock();
      cw->Status[dmap] = cacheway::invalid;
      cw->waylock->unlock();
    }
    /*else if(!cline && probem->lower_level_hit) {
      // it is not at this level but there is a line that has been modified
      // at the lower level, therefore we would need to write it back

      CTRC_EMPTY(fprintf(stdout, "%s: should clean the line 0x%lx: ", name(), probem->cache_addr); \
	   for(int i = 0; i < trans.get_data_length(); i++)		\
	     fprintf(stdout, "%02x", lower_level_lanes[i]);		\
	   fprintf(stdout, "\n"));

      CTRC_EMPTY(fprintf(stdout, "%d, %d, 0x%lx, 0x%lx\n", 
		   probem->source_node, probem->dest_node, busyAddr, line_addr));

      if(!probem->real_write) {
        ackm->set_data_length(trans.get_data_length());
	u8_t* new_data = new u8_t[trans.get_data_length()];
	u8_t* new_lanes = new u8_t[trans.get_data_length()];
	for(int i = 0; i < trans.get_data_length(); i++)
	  if(lower_level_lanes[i] == 0xFF) {
	    new_data[i] = lower_level_data[i];
	    new_lanes[i] = 0xFF;
	  }
	  else {
	    new_data[i] = 0x00;
	    new_lanes[i] = 0x00;
	  }
	
	ackm->set_data_ptr(new_data);
	ackm->set_byte_enable_ptr(new_lanes);
	
	mm->lower_level_hit = true;
      }
    }*/

    trans.release();

    tlm_phase ph = BEGIN_REQ;
    
    ack_msg* mm_t = 0;
    ackm->get_extension(mm_t);
    assert(mm_t != NULL);
    
    CTRC_EMPTY(fprintf(stdout, 
		 "%s: sending ACK to sri module to be sent to node %d, data %s; 0x%lx\n",
		 name(), mm_t->dest_node, ackm->get_data_ptr() != 0 ? "exists" : "doesn't exist", mm_t->addr));
    inita_socket->nb_transport_fw(*ackm, ph, max_lookup_delay);
    
    break;
  }
  default:
    assert(0);
  }
}

void hammer_cache64::peq_cb(PW_TLM_PAYTYPE& trans, const tlm_phase& ph) {

  probe_msg* probem = 0;
  trans.get_extension(probem);
  assert(probem != 0);

  CTRC_EMPTY(fprintf(stdout, "%s: received probe request; 0x%lx, 0x%lx, %d\n", 
	       name(), probem->cache_addr, cache_invalid_addr, cache_invalid_msg));

  // we are sending invalidation to a lower level cache
  if(ph == BEGIN_REQ
     // do not need to forward to lower level caches if
     // this cache is sending the invalidation message
     && !(cache_invalid_msg 
       && cache_invalid_addr == probem->cache_addr)
     && (moesi_init != 0)) {
    CTRC_EMPTY(fprintf(stdout, "%s: forwarding to lower level cache; %d, 0x%lx, 0x%lx\n", 
		 name(), cache_invalid_msg, cache_invalid_addr, probem->cache_addr));
    sc_time delay = SC_ZERO_TIME;
    tlm_phase phase = ph;
    //assert(moesi_init != 0);
    assert(processing_probe == -1);
    processing_probe = probem->cache_addr;
    (*moesi_init)->nb_transport_fw(trans, phase, delay);
  }
  else if(ph == END_REQ
	  || (cache_invalid_msg
	      && cache_invalid_addr == probem->cache_addr)) {
    probe_msg *probem = 0;
    trans.get_extension(probem);
    assert(probem != 0);

    u64_t addr = probem->cache_addr;

    sc_time delay = SC_ZERO_TIME;
    if(ph == END_REQ && processing_probe != addr)
        assert(0);
    probe_message(trans, delay, addr, probem);
    processing_probe = -1;
  }
  // there is no lower level cache so do not send message
  else if(ph == BEGIN_REQ && moesi_init == 0) {
    probem->lower_level_hit = false;
    u64_t addr = probem->cache_addr;

    sc_time delay = SC_ZERO_TIME;
    probe_message(trans, delay, addr, probem);
  }
  else
    assert(0);
}

void hammer_cache64::moesi_peq_cb(PW_TLM_PAYTYPE& trans, const tlm_phase& ph) {
  probe_msg *probem = 0;
  trans.get_extension(probem);
  assert(probem != 0);
  
  tlm_command cmd = trans.get_command();

  u8_t* lanes = trans.get_byte_enable_ptr();
  if(lanes == 0) {
    u8_t* new_lanes = new u8_t[trans.get_data_length()];
    for(int i = 0; i < trans.get_data_length(); i++)
      new_lanes[i] = 0x00;

    trans.set_byte_enable_ptr(new_lanes);
  }

  lt_sc_time delay = SC_ZERO_TIME;

  u64_t addr = probem->cache_addr;
  assert(geom.linesize <= trans.get_data_length());

  // this cache has sent the invalid message so we shouldn't
  // invalidate lines in it
  /*if(cache_invalid_msg && cache_invalid_addr == addr) {
    tlm_phase phase = END_REQ;
    assert(moesi_targ != 0);
    
    (*moesi_targ)->nb_transport_bw(trans, phase, delay);
  }
  else {
    CTRC_EMPTY(fprintf(stdout, "%s: invalid msg = %d, invalid adr = 0x%lx, adr = 0x%lx\n",
		 name(), cache_invalid_msg, cache_invalid_addr, addr));
  }*/

  // this should be done in parallel (?)
  for(int d = 0; d < trans.get_data_length(); d+= geom.linesize) {
    sc_time this_delay = SC_ZERO_TIME;
    int dmap = ((addr+d) >> (u64_t)geom.dmap_shift) & (geom.dmapping-1LLU);
    CTRC_EMPTY(fprintf(stdout, "%s: probing line 0x%lx, %d\n", name(), addr+d, dmap));
    u8_t* cline = 0;
    cacheway *cw = 0;

    sc_time max_lookup_delay = SC_ZERO_TIME;

    // this loop is done in parallel 
    for(int w = 0; w < geom.ways; w++) {
      cw = Cont[w];
      if(cw->lookup(addr+d, dmap, &cline, max_lookup_delay))
	break;
    }
    
#if MOESI_TIMING
    this_delay += max_lookup_delay;
#endif

    bool pass_data = false;
    
    if(cline) {
      switch(cmd) {
        case TLM_WRITE_COMMAND: {
	  if(cw->Status[dmap] == cacheway::modified
	     || cw->Status[dmap] == cacheway::owned) 
	    pass_data = true;
	  
	  // invalidate the line
	  cw->waylock->lock();
	  cw->Status[dmap] = cacheway::invalid;
	  cw->waylock->unlock();
	  
	  // we don't need to time this as the line should be invalidated
	  // when we are doing the lookup
	  
	  break;
        }
        case TLM_READ_COMMAND: {
	  pass_data = true;

	  CTRC_EMPTY(fprintf(stdout, "%s(1): address 0x%lx was found here, state = %d\n", name(), addr+d, cw->Status[dmap]));

	  switch(cw->Status[dmap]) {
	    case cacheway::modified:
	      cw->waylock->lock();
	      cw->Status[dmap] = cacheway::owned;
	      cw->waylock->unlock();

	      // we don't need to time this as the line should be invalidated
	      // when we are doing the lookup
	      
	      break;
	    case cacheway::exclusive:
	      cw->waylock->lock();
	      cw->Status[dmap] = cacheway::shared;
	      cw->waylock->unlock();

	      // we don't need to time this as the line should be invalidated
	      // when we are doing the lookup

	      break;
	    default:
	      break;
	  }
	  break;
        }
      }

      if(pass_data) {
	// set dirty lines and data
	u8_t* lanes = trans.get_byte_enable_ptr();
	u8_t* data = trans.get_data_ptr(); 
	if(data == 0) {
	  data = new u8_t[trans.get_data_length()];
	  trans.set_data_ptr(data);
	}
	if(lanes == 0) {
	  lanes = new u8_t[trans.get_data_length()];
	  trans.set_byte_enable_ptr(lanes);
	  for(int i = 0; i < trans.get_data_length(); i++)
	    lanes[i] = 0x00;
	}

	for(int i = 0; i < geom.linesize; i++) {
	  lanes[i+d] = 0xFF;
	  data[i+d] = cline[i];
	}

	CTRC_EMPTY(fprintf(stdout, "%s: data for higher level node %d-%d (0x%lx) is: ", 
		     name(), d, d+geom.linesize, addr+d);		\
	     for(int i = 0; i < trans.get_data_length(); i++)		\
	       fprintf(stdout, "%02x", data[i]);			\
	     fprintf(stdout, "; lanes =");				\
	     for(int i = 0; i < trans.get_data_length(); i++)		\
	       fprintf(stdout, "%02x", lanes[i]);			\
	     fprintf(stdout, "\n"));

	
	probem->lower_level_hit = true;
	CTRC_EMPTY(fprintf(stdout, "%s(2): address 0x%lx was found here, state = %d, dmap = %d\n", name(), addr+d, cw->Status[dmap], dmap));
	
      }
    }
#if MOESI_TIMING
    delay = max(delay, this_delay);
#endif
  }

  tlm_phase phase = END_REQ;
  assert(moesi_targ != 0);

  (*moesi_targ)->nb_transport_bw(trans, phase, delay);
}


tlm_sync_enum hammer_cache64::nb_transport_fw(int id,
					      PW_TLM_PAYTYPE& trans,
					      tlm_phase& phase,
					      sc_time& delay) {
  m_peq.notify(trans, phase, delay);
  return TLM_ACCEPTED;
}

tlm_sync_enum hammer_cache64::moesi_nb_transport_fw(PW_TLM_PAYTYPE& trans,
						    tlm_phase& ph,
						    sc_time& delay) {
  m_moesi_peq.notify(trans, ph, delay);
  return tlm::TLM_ACCEPTED;
}


tlm_sync_enum hammer_cache64::moesi_nb_transport_bw(PW_TLM_PAYTYPE& trans,
						    tlm_phase& ph,
						    sc_time& delay) {
  m_peq.notify(trans, ph, delay);
  return TLM_ACCEPTED;
}

void hammer_cache64::evict_lower_level_line(u64_t line, size_t linesize, u8_t* upper_data) {
  if(lower_level_data != 0) 
    for(u64_t step = 0; step < linesize; step += lower_level_data->geom.linesize) {
      u64_t a = line + step;
      int dmap = (a >> (u64_t)lower_level_data->geom.dmap_shift) & (lower_level_data->geom.dmapping-1LLU);

      CTRC_EMPTY(fprintf(stdout, "going to check 0x%lx, %d\n", a, dmap));

      for(int w = 0; w < lower_level_data->geom.ways; w++) {
	cacheway *cw = lower_level_data->Cont[w];
	if((*cw->Tags)[dmap] == a
	   && (cw->Status[dmap] == cacheway::modified 
	       || cw->Status[dmap] == cacheway::owned)) {
	  u8_t* dd = cw->Data->read8p(dmap * lower_level_data->geom.linesize);
	  for(int i = 0; i < lower_level_data->geom.linesize; i++) 
	    upper_data[i+step] = dd[i];

	  cw->waylock->lock();
	  cw->Status[dmap] = cacheway::invalid;
	  cw->waylock->unlock();
	}
	
      }
    }
}

pair<u8_t*, u8_t*> hammer_cache64::check_lower_level_lines(u64_t line, 
							   size_t linesize,
							   cacheway::mesi_t new_state) {
  u8_t* data = NULL;
  u8_t* flags = NULL;
  if(lower_level_data != 0) {
    CTRC_EMPTY(printf("%s: lower level data is not NULL\n", name()));
    struct cache_geom g = lower_level_data->geom;
    for(u64_t step = 0; step < linesize; step += lower_level_data->geom.linesize) {
      u64_t a = line + step;
      int dmap = (a >> (u64_t)g.dmap_shift) & (g.dmapping-1LLU);
      
      for(int w = 0; w < g.ways; w++) {
	cacheway *cw = lower_level_data->Cont[w];
	if((*cw->Tags)[dmap] == a
	   && (cw->Status[dmap] == cacheway::modified
	       || cw->Status[dmap] == cacheway::owned))	{
	  CTRC_EMPTY(fprintf(stdout, "%s: address 0x%lx, %d was modified\n", name(), a, dmap));
	  if(data == NULL) {
	    data = new u8_t[linesize];
	    for(int i = 0; i < linesize; i++)
	      data[i] == 0x00;
	  }
	  if(flags == NULL) {
	    flags = new u8_t[linesize];
	    for(int i = 0; i < linesize; i++)
	      flags[i] = 0x00;
	  }
	  u8_t* l_data = cw->Data->read8p(dmap * g.linesize);
	  for(int i = 0; i < g.linesize; i++) {
	    data[i+step] = l_data[i];
	    flags[i+step] = 0xFF;
	  }

	  cw->waylock->lock();
	  cw->Status[dmap] = new_state;
	  cw->waylock->unlock();
	}
      }
    }
  }

  return make_pair(data, flags);
}

void hammer_cache64::operate(cacheway *cw, 
			     cacheway::mesi_t op, 
			     u64_t addr, 
			     int dmap, 
			     sc_time &delay)
{
  sysc_assert(cw);
  
  cw->waylock->lock();
  //bool changed=false;
  //cacheway::mesi_t os = cw->operate(op, addr, dmap, changed); 
  cacheway::mesi_t os = cw->Status[dmap];
  CTRC(addr, printf("%s: the previous state of cache line was %d; the new state is %d\n", name(), os, op));
  busy_way = cw;
  cw->waylock->unlock();

  // if the previous state was shared then it needs to invalidate
  // the same cache line from its consistent peers
  if(os != op
     && (os == cacheway::shared || os == cacheway::owned)
     && grp)
    for(int g = 0; g < grp->target_members; g++) {
      if(grp->Members[g] == this) continue;

      cache64* c = grp->Members[g];
      
      for(int w = 0; w < c->geom.ways; w++) {
	cacheway* g_cw = c->Cont[w];
	
	g_cw->waylock->lock();
	bool present = g_cw->lookup(addr, dmap, 0, delay);
	if(present) {
	  g_cw->Status[dmap] = cacheway::invalid;
	  CTRC(addr, printf("%s: invalidated addr=0x%lx, dmap=0x%x\n", c->name(), addr, dmap));
	}
	g_cw->waylock->unlock();
      }
    }

  if(os == cacheway::shared || os == cacheway::owned) {
    // need to broadcast invalidations to other caches that have it shared
    // also send the invalid message to all other cores
    PW_TLM_PAYTYPE *trans;
    
    CTRC(addr, printf("%s: sending invalidation msgs to addr=0x%lx, dmap=0x%x\n", 
		       name(), addr, dmap));

    cache_invalid_msg = true;
    cache_invalid_addr = addr;
    
    cache_miss_extension ext;    
    trans = cache_miss_mm.allocate();
    trans->set_auto_extension<cache_miss_extension>(&ext);
    trans->acquire();
    
    trans->set_data_length(geom.linesize);
    trans->set_data_ptr(0);
    trans->set_write();
    trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    trans->set_address(addr);       

    inita_socket->b_transport(*trans, delay);
    
    cache_invalid_msg = false;
    cache_invalid_addr = -1;

    assert(trans->is_response_error() == false);
    
    trans->release();
  }
  
  // call the method from the base class
  cache64::operate(cw, op, addr, dmap, delay);
} 

bool hammer_cache64::do_i_lookup(u64_t addr) {
    if(processing_probe != -1 
       && addr == processing_probe)
        // we are not going to lookup because at the moment
        // this cache is being updated due to probe request
        // and as a result will change state of cache line
        // that has the same address that we want to lookup
        return false;
    else
        return true;
}

// eof
