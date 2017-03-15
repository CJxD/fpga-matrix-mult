#ifndef GENERIC_BRANCH_PREDICTOR_H
#define GENERIC_BRANCH_PREDICTOR_H

//
// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2015 - DJ Greaves, M Puzovic
// Generic Branch Predictor
//
//

// Note: installing a branch predictor speeds up real hardware but slows down a high-level simulator. !



class generic_branch_predictor
{

  // The 2-way associativity is missing.
  int m_btb_capacity;
  int m_history_capacity;
  u16_t m_global_history;

  bool m_global_path_enabled;
  bool m_local_hist_enabled;

  u64_t m_uncollected_penalties; // Local account of penalites to be added.


  // This is a general-purpose predictor but the comments currently in here give ARM sizes.
  // Zynq (Cortex A9) has a 4096 entry GHB buffer with 2-bit histories.


#define RSTACK_ENTRIES 8 // Zynq has 8 levels here.
  int m_return_stack_pointer;
  u32_t return_stack[RSTACK_ENTRIES];

  class predict_entry
  {
  public:
    u3_t tornament;
    u8_t history_history;
    bool global_path_prediction;
#define BP_TORNAMENT_MAX 15 // Use 15 as max count for 4-bit tornament counters.
    predict_entry()  // constructor
      { 
	global_path_prediction = 0; history_history = 0; tornament=BP_TORNAMENT_MAX/2; 
      } // constructor


  };

  class btb_entry
  {
  public:
    u32_t tag;
    u32_t computed_jump_target_prediction;
    btb_entry() { tag = -1; computed_jump_target_prediction = 0;  } // constructor
  };

  btb_entry **m_btb_table; // On Zynq this is two-way set associative, total 512 entries.

  predict_entry **m_history_table; 

 public:

  u64_t collect_penalties() // Penalties are accumulated inside the predictor and aggregated to ISS total when this method is invoked.
  {
    u64_t r = m_uncollected_penalties;
    m_uncollected_penalties = 0;
    return r;
  }


  struct predictor_stats_s
  {
  public:
    u64_t simp_operations;
    u64_t simp_mispredicts;
    u64_t simp_taken;

    u64_t computed_jumps;
    u64_t computed_jump_mispredicts;
    u64_t pop_jumps;
    u64_t pop_jump_mispredicts;


    // Additional stats
    u64_t simp_local_misses;
    u64_t simp_global_misses;
    u64_t simp_torns_loc;
    u64_t simp_torns_glob;


    predictor_stats_s(); // constructor
    void report(const char *msg, FILE *fd, bool resetf);
    void reset();


  } stats;

  generic_branch_predictor(int btb_logsize, int ways, int history_capacity); // constructor

  void invalidate_all();

  // Branch type is encoded in push_pop_style as follows:
  //   0  - a normal conditional branch with unchanging target address
  //   -1 - a subroutine return where the best prediction will be a pop of the return address stack
  //   +1 - a normal call to an unchanging target address - needs prediction if predicated as per ARM and return address needs to be noted for the return address predictor.
  //  100 - a computed branch where the destination predictor needs to be invoked. - generally unconditional and used in dynamic linking.

  u32_t operate(u32_t pc, bool taken, u32_t dest_pc, int push_pop_style=0);

  u32_t abt_temp_last_call;
  u32_t abt_temp_last_call_to;

};

// eof
#endif

