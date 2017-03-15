// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2012 M Puzovic and D J Greaves
// University of Cambridge, Computer Laboratory.
// $Id: $

// AMD Opteron-like system messages definition file

#ifndef _OPTERON_MSGS_H
#define _OPTERON_MSGS_H

#include "systemc.h"

#ifndef PW_TLM_PAYLOAD
#define PW_TLM_PAYLOAD 3 // for now - need to make this work without.
#endif  // PW_TLM_PAYLOAD
#ifdef TLM_POWER3
#include "pw_tlm_payload.h"
#else
#include "tlm.h"
typedef tlm::tlm_base_protocol_types PW_TLM_TYPES;
typedef tlm::tlm_generic_payload PW_TLM_PAYTYPE;
#endif

#include "tenos.h"

// if it is set then we time moesi messages
#define MOESI_TIMING 1

DECLARE_EXTENDED_PHASE(ACK_DRAM);

enum opteron_msgs {
  DRAM_MSG,
  REQ_MSG,
  PROBE_MSG,
  ACK_MSG,
  UNBLOCK_MSG,
  UNKNOWN_MSG
};

typedef enum opteron_msgs opteron_msg_ty;

// SRI message sent from system request interface to crossbar
// it has address and node to which this address belongs

struct sri_msg : tlm::tlm_extension<sri_msg> {
  int home_node;
  int originating_node;
  u64_t cache_addr;

  // constructor
  sri_msg() {
    home_node = -1;
    originating_node = -1;
    cache_addr = -1;
    
  }

  // Must implement clone and copy_from in all extensions
  tlm_extension_base* clone() const {
    sri_msg* ext = new sri_msg;
    ext->home_node = this->home_node;
    ext->originating_node = this->originating_node;
    ext->cache_addr = this->cache_addr;
    
    return ext;
  }

  void copy_from(tlm_extension_base const &ext) {
    home_node = static_cast<sri_msg const &>(ext).home_node;
    originating_node = static_cast<sri_msg const&>(ext).originating_node;
    cache_addr = static_cast<sri_msg const &>(ext).cache_addr;
  }
};


struct req_msg : tlm::tlm_extension<req_msg> {
  // number of node from which request originated
  int source_node;
  // number of home node
  int home_node;
  // original cache address
  u64_t cache_addr;
  // creating node
  int creator_node;
  // paused time
  sc_time* paused_time;
  // owner used by stats collection
  int* owner_node;

  // constructor
  req_msg() {
    source_node = -1;
    home_node = -1;
    cache_addr = -1;
    creator_node = -1;
    paused_time = 0;
    owner_node = 0;
  }

  // Must implement clone and copy_from in all extensions
  tlm_extension_base* clone() const {
    req_msg* ext = new req_msg;
    ext->source_node = this->source_node;
    ext->home_node = this->home_node;
    ext->cache_addr = this->cache_addr;
    ext->creator_node = this->creator_node;
    ext->paused_time = this->paused_time;
    ext->owner_node = this->owner_node;
    
    return ext;
  }

  void copy_from(tlm_extension_base const &ext) {
    source_node = static_cast<req_msg const &>(ext).source_node;
    home_node = static_cast<req_msg const &>(ext).home_node;
    cache_addr = static_cast<req_msg const &>(ext).cache_addr;
    creator_node = static_cast<req_msg const &>(ext).creator_node;
    paused_time = static_cast<req_msg const &>(ext).paused_time;
    owner_node = static_cast<req_msg const &>(ext).owner_node;
  }
};

struct probe_msg : tlm::tlm_extension<probe_msg> {
  // number of node that is operating on the cache line
  int source_node;
  int dest_node;
  int home_node;
  // node that created the probe message
  int originator_node;
  u64_t cache_addr;
  // are we waiting for dram controller to get back
  bool dram;
  // it is set to true if we have data to write
  // otherwise it is just invalidation
  bool real_write;
  // was it hit in the lower level cache
  bool lower_level_hit;

  // constructor
  probe_msg() {
    source_node = -1;
    dest_node = -1;
    home_node = -1;
    cache_addr = -1;
    dram = false;
    real_write = true;
    lower_level_hit = false;
  }

  // Must implement clone and copy_from in all extensions
  probe_msg* clone() const {
    probe_msg* ext = new probe_msg;
    ext->source_node = this->source_node;
    ext->home_node = this->home_node;
    ext->dest_node = this->dest_node;
    ext->cache_addr = this->cache_addr;
    ext->dram = this->dram;
    ext->real_write = this->real_write;
    ext->lower_level_hit = this->lower_level_hit;
    
    return ext;
  }

  void copy_from(tlm_extension_base const &ext) {
    source_node = static_cast<probe_msg const &>(ext).source_node;
    home_node = static_cast<probe_msg const &>(ext).home_node;
    dest_node = static_cast<probe_msg const &>(ext).dest_node;
    cache_addr = static_cast<probe_msg const &>(ext).cache_addr;
    dram = static_cast<probe_msg const &>(ext).dram;
    real_write = static_cast<probe_msg const &>(ext).real_write;
    lower_level_hit = static_cast<probe_msg const &>(ext).lower_level_hit;
  }
};

struct ack_msg : tlm::tlm_extension<ack_msg> {
  enum ack_type {
    UNKNOWN,
    DRAM,
    CACHE,
    WB
  };

  typedef enum ack_type ack_type_t;
  
  // number of node that is operating on the cache line
  int dest_node;
  // originator
  int orig_node;
  // node where memory is
  int home_node;
  // cache address for which we are sending msg
  u64_t addr;
  // is it write to the cache line
  bool write;
  // type of the acknowledgment
  ack_type_t ty;
  // where the cache line was found
  ack_type resp;
  // evicting cache line
  bool evict_line;
  // was it satisfied from lower level
  bool lower_level_hit;
  // was it satisfied from write buffer
  bool wb_hit;

  // constructor
  ack_msg() {
    dest_node = -1;
    orig_node = -1;				
    home_node = -1;
    addr = -1;
    write = false;
    ty = UNKNOWN;
    resp = UNKNOWN;
    evict_line = false;
    lower_level_hit = false;
    wb_hit = false;
  }

  // Must implement clone and copy_from in all extensions
  tlm_extension_base* clone() const {
    ack_msg* ext = new ack_msg;
    ext->dest_node = this->dest_node;
    ext->orig_node = this->orig_node;
    ext->home_node = this->home_node;
    ext->addr = this->addr;
    ext->write = this->write;
    ext->ty = this->ty;
    ext->resp = this->resp;
    ext->evict_line = this->evict_line;
    ext->lower_level_hit = this->lower_level_hit;
    ext->wb_hit = this->wb_hit;
    
    return ext;
  }

  void copy_from(tlm_extension_base const &ext) {
    dest_node = static_cast<ack_msg const &>(ext).dest_node;
    orig_node = static_cast<ack_msg const &>(ext).orig_node;
    home_node = static_cast<ack_msg const &>(ext).home_node;
    addr = static_cast<ack_msg const&>(ext).addr;
    write = static_cast<ack_msg const&>(ext).write;
    ty = static_cast<ack_msg const&>(ext).ty;
    resp = static_cast<ack_msg const&>(ext).resp;
    evict_line = static_cast<ack_msg const&>(ext).evict_line;
    lower_level_hit = static_cast<ack_msg const&>(ext).lower_level_hit;
    wb_hit = static_cast<ack_msg const&>(ext).wb_hit;
  }
};

struct unblock_msg : tlm::tlm_extension<unblock_msg> {
  // number of node that is operating on the cache line
  int dest_node;
  // creator
  int creator_node;

  // constructor
  unblock_msg() {
    dest_node = -1;
    creator_node = -1;
  }

  // Must implement clone and copy_from in all extensions
  tlm_extension_base* clone() const {
    unblock_msg* ext = new unblock_msg;
    ext->dest_node = this->dest_node;
    ext->creator_node = this->creator_node;
    
    return ext;
  }

  void copy_from(tlm_extension_base const &ext) {
    dest_node = static_cast<unblock_msg const &>(ext).dest_node;
    creator_node = static_cast<unblock_msg const &>(ext).creator_node;
  }
};


// Memory manager for Opteron-type messages
class opteron_msg_mm : public tlm::tlm_mm_interface
{

 public:
  opteron_msg_mm();

  PW_TLM_PAYTYPE* allocate();

  void free(PW_TLM_PAYTYPE* r);
#ifdef TLM_POWER3
  void free(tlm::tlm_generic_payload* trans) {
    free((PW_TLM_PAYTYPE*)trans);
  }
#endif


 private:
  struct access {
    PW_TLM_PAYTYPE* trans;
    access* next;
    access* prev;
  };

  access* free_list;
  access* empties;

  sc_mutex lck;

#ifdef OPTERON_DEBUG
  int count;
#endif

};



#endif
