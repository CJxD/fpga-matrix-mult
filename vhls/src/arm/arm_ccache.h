// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2014 M Puzovic + D J Greaves

#ifndef __ARM_CCACHE_H__
#define __ARM_CCACHE_H__

#include "../memories/ccache.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/simple_initiator_socket.h"

class arm_ccache : public ccache {

public:
  enum type {
    Instruction, 
    Data,
    Unified
  };

  enum region {
    Inner,
    Outer
  };

  // Constructor
  arm_ccache(
    sc_core::sc_module_name name,
    u32_t id,
    u32_t addrsize, 
    u32_t size, 
    u32_t linesize, 
    u8_t ways, 
    bool snooped,
    type ty,
    u8_t level);

  // Socket used by CP15 to send control messages
  tlm_utils::multi_passthrough_target_socket<arm_ccache, 64, PW_TLM_TYPES> cp15_target_socket;
  tlm_utils::simple_initiator_socket<arm_ccache, 64, PW_TLM_TYPES>* cp15_initiator_socket;
  tlm_utils::simple_initiator_socket<arm_ccache, 64, PW_TLM_TYPES>* abt_initiator_socket;


  void create_abt_socket();
  void foreign_write(u64_t addr); // overload

  void b_cp15_access(int idx, PW_TLM_PAYTYPE &trans, sc_time &delay);

  void set_region(region region) {
      m_region = region;
  }

  void set_shareable(bool is_shareable) {
      m_shareable = is_shareable;
  }

  void set_PoU(bool is_PoU);

 private:
  u8_t m_level;
  type m_ty;
  region m_region;
  bool m_shareable;
  bool m_PoU;
};

#endif
