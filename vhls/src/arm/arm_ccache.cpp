// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2014 M Puzovic + D J Greaves

#include "arm_ccache.h"
#include "arm_cp15.h"

arm_ccache::arm_ccache(
  sc_core::sc_module_name name,
  u32_t id,
  u32_t addrsize, 
  u32_t size, 
  u32_t linesize, 
  u8_t ways, 
  bool snooped,
  type ty,
  u8_t level) : 
  ccache(name, id, addrsize, size, linesize, ways, snooped),
  cp15_initiator_socket(NULL), 
  abt_initiator_socket(NULL), 
  m_ty(ty),
  m_region(Inner),
  m_shareable(false),
  m_PoU(true),
  m_level(level)
{

  cp15_target_socket.register_b_transport(this, &arm_ccache::b_cp15_access);
}


void arm_ccache::create_abt_socket() {
  // instantiate initiator socket because message
  // will need to be propagated until we reach
  // point of unification
  abt_initiator_socket =
    new tlm_utils::simple_initiator_socket<arm_ccache, 64, PW_TLM_TYPES>;
}

void arm_ccache::set_PoU(bool is_PoU) {
    m_PoU = is_PoU;
    if(!is_PoU) {
        assert(!cp15_initiator_socket);
        // instantiate initiator socket because message
        // will need to be propagated until we reach
        // point of unification
        cp15_initiator_socket =
            new tlm_utils::simple_initiator_socket<arm_ccache, 64, PW_TLM_TYPES>;
    }
}

void arm_ccache::b_cp15_access(int idx, PW_TLM_PAYTYPE &trans, sc_time &delay) {
    cp15_cache_control_extension* ext = 0;
    trans.get_extension(ext);
    assert(ext);

    // get type of the operation
    u3_t operation = ext->op;
    if(operation == (1 << cp15_cache_control_extension::Enable)) { // we need to enable caches
        // check to see if type of this cache
        // is what is in the message
        if(ext->types & (1 << m_ty)) {
            // need to enable cache 
            // and then forward message
            // to other caches
            enable();
            printf("Cache %s is ENABLED now\n", name());
        }
        if(cp15_initiator_socket)
            (*cp15_initiator_socket)->b_transport(trans, delay);        
    }
    else {
        // get type of the cache
        // check to see if type of this cache is what is expected
        // check to see if we need to operate on message
        if((ext->types & (1 << m_ty))) {
            // at this point operations will be either clean or invalidate or both
            // check to see if we need to clean first
            if(ext->op & (1 << cp15_cache_control_extension::Clean) ||
               ext->op & (1 << cp15_cache_control_extension::Invalid)) {
	      if(ext->data != -1 && ext->address) {
                u64_t addr = ext->data;

                // index within the cache way
                u64_t index = (addr >> m_geometry.boffsetb) & m_geometry.indexm;
                // tag
                u64_t tag = (addr >> (m_geometry.boffsetb + m_geometry.indexb));
                
                u8_t* clinep = 0;
                u64_t oldts;
                state_t clstate;
                way *cway = lookup(addr, index, tag, &clinep, oldts, clstate, delay);
		
                if(cway != NULL) {
                    u64_t ts = cway->m_status[index].load();
                    if(!cway->acquirets(ts, index)) // we must be able to acquire this line
                        assert(0);

                    // evict line
                    cway->clean(addr, index, tag, clstate, delay, m_geometry.linesize);

                    // if it is only clean than change state to SharedClean,
                    // otherwise change it to Invalid
                    u64_t newts;
                    if(ext->op & (1 << cp15_cache_control_extension::Invalid))
                        newts = cway->incts(ts, Invalid);
                    else
                        newts = cway->incts(ts, SharedClean);

                    assert(cway->releasets(index, ts, newts));

                }
	      }
              else if(ext->data != -1 && !ext->address) {
                  // Parse data so that we find way and index that we need to invalidate
                  int A = bound_log2(m_geometry.ways);
                  int S = bound_log2(m_geometry.indexm + 1);
                  int L = bound_log2(m_geometry.linesize);
                  int B = S + L;
                  //printf("A=%d, S=%d, L=%d\n", A, S, L);

                  int level = ((ext->data >> 1) & 0x7) + 1;
                  u64_t index = (ext->data >> L) & ((int)floor(pow(2, (B-L))) - 1);
                  int w = ext->data >> (32 - A);
                  //printf("level=%d, set=%d, way=%d\n", level, index, w);
                  if(m_level == level) {
                      way* cway = m_ways[w];
                      u64_t ts = cway->m_status[index].load();
                      state_t clinestate = (state_t)((ts >> 1) & 0x7);
                      
                      // check to see if line state is not invalid, if it
                      // is invalid than there is no need for eviction
                      if(clinestate != ccache::Invalid) {
                          
                          if(!cway->acquirets(ts, index)) // we must be able to acquire this line
                              assert(0);
                          
                          if(ext->op & (1 << cp15_cache_control_extension::Clean)) {
                              sc_time delay = SC_ZERO_TIME;
                              u64_t addr = cway->rebuild_address(index);
                              u64_t tag = (*(cway->m_tags))[index];
                              // evict line
                              cway->clean(addr, index, tag, clinestate, delay, m_geometry.linesize);
                          }
                          
                          u64_t newts;
                          if(ext->op & (1 << cp15_cache_control_extension::Invalid))
                              newts = cway->incts(ts, Invalid);
                          else
                              newts = cway->incts(ts, SharedClean);
                          
                          assert(cway->releasets(index, ts, newts));
                      }
                      
                  }
                  else if(m_level < level) {
                      // we need to forward invalidation transaction
                      // to cache above in hierarchy
                      assert(cp15_initiator_socket);
                      if(!ext->snooped) {
                          (*cp15_initiator_socket)->b_transport(trans, delay);
                      }
                  }

                  else {
                      // level should never be less
                      // then the level of current cache
                      assert(0);
                  }
              }

	      else {
		// this should be invalidation of all entries
		// since this is instruction cache it is not snooped
		// so we can simply just go through all entries
		// and make them invalid
		assert(m_ty == Instruction);
		invalidate();
	      }
	    }
	}

        // check to see if we need to forward
        if(ext->stop != cp15_cache_control_extension::None && !m_PoU) {
            assert(cp15_initiator_socket);
            if(!ext->snooped) {
                (*cp15_initiator_socket)->b_transport(trans, delay);
            }
        }
    }

    trans.set_response_status(tlm::TLM_OK_RESPONSE);

    return;
}		       



void arm_ccache::foreign_write(u64_t addr) // overload
{
  if (abt_initiator_socket)
    {
      PW_TLM_PAYTYPE trans;
      trans.set_byte_enable_length(0);
      trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
      trans.set_read();
      trans.set_address(addr);
      trans.set_data_length(0);
      trans.set_data_ptr(0);
      
      sc_time delay = SC_ZERO_TIME;
      // send the message to all connected caches
      // they start at 1 because at index 0 is MMU
      (*abt_initiator_socket)->b_transport(trans, delay);
      assert(!trans.is_response_error());
    }
}

// eof
