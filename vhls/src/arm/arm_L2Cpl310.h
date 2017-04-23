// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2015 M Puzovic + D J Greaves
// ARM (Zynq) L2 Cache

#ifndef __ARM_L2CPL310_H__
#define __ARM_L2CPL310_H__



#include "arm_ccache.h"

#define ARM_L2CPL310_BASE 0xF8F02000u
#define ARM_L2CPL310_SIZE 0x1000

class arm_L2Cpl310 : public arm_ccache {

 public:
  tlm_utils::multi_passthrough_target_socket<arm_L2Cpl310, 64, PW_TLM_TYPES> controller_target;
  void b_access(int idx, PRAZOR_GP_T &trans, sc_time &delay);

  // Since this class extends arm_ccache
  // at the same time as when we are constructing
  // it we are also initialising arm_ccache class
  // TODO: Consider delaying creation of arm_ccache
  // class and only instantiate it when
  // we have written to control register that
  // will indicate properties of cache
  arm_L2Cpl310(
      sc_core::sc_module_name name,
      u32_t myid,
      u32_t addrsize, 
      u32_t size, 
      u32_t linesize, 
      u8_t ways, 
      bool snooped,
      type ty,
      u8_t level);

 private:
  u32_t reg0_cache_id;
  u32_t reg0_cache_type;

  u32_t reg1_control;
  u32_t reg1_aux_control;
  u32_t reg1_tag_ram_control;
  u32_t reg1_data_ram_control;

  u32_t reg2_ev_counter_ctrl;
  u32_t reg2_ev_counter1_cfg;
  u32_t reg2_ev_counter0_cfg;
  u32_t reg2_ev_counter1;
  u32_t reg2_ev_counter0;
  u32_t reg2_int_mask;
  u32_t reg2_int_mask_status;
  u32_t reg2_int_raw_status;
  u32_t reg2_int_clear;

  u32_t reg7_cache_sync;
  u32_t reg7_inv_pa;
  u32_t reg7_inv_way;
  u32_t reg7_clean_pa;
  u32_t reg7_clean_index;
  u32_t reg7_clean_way;
  u32_t reg7_clean_inv_pa;
  u32_t reg7_clean_inv_index;
  u32_t reg7_clean_inv_way;

  u32_t reg9_d_lockdown[8];
  u32_t reg9_i_lockdown[8];
  u32_t reg9_lock_line_en;
  u32_t reg9_unlock_way;

  u32_t reg12_addr_filtering_start;
  u32_t reg12_addr_filtering_end;

  u32_t reg15_debug_ctrl;
  u32_t reg15_prefetch_ctrl;
  u32_t reg15_power_ctrl;
};

// eof

#endif
