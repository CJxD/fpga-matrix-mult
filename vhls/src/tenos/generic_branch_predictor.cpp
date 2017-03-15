//
// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2015 - DJ Greaves, M Puzovic
//
// Generic Branch Predictor
//
//

#include "tenos.h"
#include "generic_branch_predictor.h"

#define TRC(X) 
#define TRC2(X) 

#define KEEP_EXTRA_STATS 1

//
//
//
generic_branch_predictor::generic_branch_predictor(int logsize, int ways, int history_capacity) : 
  m_btb_capacity(1<<logsize), // constructor
  m_history_capacity(history_capacity)
{
  stats.reset();



  m_history_table = new predict_entry*[m_history_capacity];
  for (int i =0; i<m_history_capacity; i++) m_history_table[i] = new predict_entry();


  m_btb_table = new btb_entry*[m_btb_capacity];
  for (int i =0; i<m_btb_capacity; i++) m_btb_table[i] = new btb_entry();
  m_local_hist_enabled = true;
  m_global_path_enabled = true;
  m_global_history = 0;
  m_uncollected_penalties = 0;
  m_return_stack_pointer = 0;
  invalidate_all();
}



void generic_branch_predictor::invalidate_all()
{
  // We do not have architecurally visible behaviour, so invalidate is a nop.

  // The abtb is a different matter.
  //if (m_abtb) m_abtb->invalidate_all();
}



//
//
//
u32_t generic_branch_predictor::operate(u32_t pc, bool taken, u32_t dest, int push_pop_style) // Will need to consider dest_ arg whether fwd/back for default/off operation mode.
{
  u32_t rc = 0xFFFFffff; // default return value

  if (push_pop_style == 100) // an indirect jump
   {
     stats.computed_jumps += 1;
     m_return_stack_pointer--;
     m_return_stack_pointer &= (RSTACK_ENTRIES-1);
     int hash_s = (pc) & (m_btb_capacity-1);

     // This btb should be set associative but this code is directly mapped!
     u32_t prediction = m_btb_table[hash_s]->computed_jump_target_prediction;
     m_btb_table[hash_s]->computed_jump_target_prediction = dest; // best prediction is simple the last dest used
     bool computed_correctly = (prediction == dest);
     TRC(printf("bpredict: computed operation from %x to %x (p=%x): predicted_correctly=%i\n", pc<<2, dest<<2, prediction << 2, computed_correctly)); // The left shifts in this debug statement only work for ARM mode, not Thumb: TODO.

     //printf("trampoline %x -> %x : %x -> %x\n",   btb_temp_last_call<<2, btb_temp_last_call_to<<2, pc<<2, dest<<2);

     if (!computed_correctly) 
       {
	 stats.computed_jump_mispredicts += 1;
	 m_uncollected_penalties += 1;
       }	 
     return rc;  
   }
  else if (push_pop_style < 0) // a subroutine return or 
   {
     stats.pop_jumps += 1;
     m_return_stack_pointer--;
     m_return_stack_pointer &= (RSTACK_ENTRIES-1);
     u32_t prediction = return_stack[m_return_stack_pointer];
     bool pop_correctly = (prediction == dest);
     TRC(printf("bpredict: pop operation from %x to %x (p=%x): predicted_correctly=%i\n", pc<<2, dest<<2, prediction << 2, pop_correctly)); // The left shifts in this debug statement only work for ARM mode, not Thumb: TODO.

     if (!pop_correctly) 
       {
	 stats.pop_jump_mispredicts += 1;
	 m_uncollected_penalties += 1;
       }	 
     return rc;  
   }

  if (taken && push_pop_style > 0) // note a taken subroutine call
   {
     abt_temp_last_call = pc;
     abt_temp_last_call_to = dest;
     return_stack[m_return_stack_pointer++] = pc+1; // Save the likely return address - the instruction after the call.
     m_return_stack_pointer &= (RSTACK_ENTRIES-1);
     TRC(printf("bpredict: push operation from %x to %x logged\n", pc<<2, dest<<2)); // The left shifts in this debug statement only work for ARM mode, not Thumb: TODO.
   }

  stats.simp_operations += 1; // Else a normal branch, and the actual branch part of a branch/jmp to subroutine.
  if (taken) stats.simp_taken += 1;
  //  if (abtb) rc = abtb->query(dest<<2, push_pop_style);
  //  if (rc != 0xFFFFffff) return rc; // If the trampoline bypass is detected, return it here.


  // pc has been right shifted already before call (e.g. 2 places for ARM and 1 for Thumb mode).
  int hash_s = (pc) & (m_history_capacity - 1);
  int hash_d = hash_s ^ (m_global_history) & (m_history_capacity - 1);
 
#define BP_LOCAL_HIST_MASK 3 /* ARM has two bit histories - the minimum sensible - this should then be (1<<2)-1 = 3 */

  bool local_prediction = ((m_history_table[hash_s]->history_history) & BP_LOCAL_HIST_MASK) > (BP_LOCAL_HIST_MASK/2);
  bool global_path_prediction = m_history_table[hash_d]->global_path_prediction;
  m_history_table[hash_d]->global_path_prediction = taken;


  bool use_global_here = m_history_table[hash_s]->tornament > (BP_TORNAMENT_MAX/2);

  bool prediction = 
    // Implement default fwd-not-taken backward-taken rule when both predictors are disabled.
    (!m_local_hist_enabled && !m_global_path_enabled) ? (dest <= pc):
    (m_local_hist_enabled && m_global_path_enabled) ? ((use_global_here) ? global_path_prediction: local_prediction):
    (m_local_hist_enabled) ? local_prediction: global_path_prediction;
  
  bool local_hist_correct = local_prediction == taken;
  bool global_path_correct = global_path_prediction == taken;
  bool correct = prediction == taken;

#if KEEP_EXTRA_STATS
  if (m_local_hist_enabled && !local_hist_correct) stats.simp_local_misses += 1;
  if (m_global_path_enabled && !global_path_correct) stats.simp_global_misses += 1;
  if (m_local_hist_enabled && m_global_path_enabled)
    {
      if (use_global_here) stats.simp_torns_glob += 1;
      else stats.simp_torns_loc += 1;
    }
#endif



  // Implement default fwd-not-taken backward-taken rule when both predictors are disabled.

  if (m_local_hist_enabled && m_global_path_enabled && local_hist_correct && !global_path_correct && m_history_table[hash_s]->tornament > 0) m_history_table[hash_s]->tornament -= 1;
  else if (global_path_correct && !local_hist_correct && m_history_table[hash_s]->tornament < BP_TORNAMENT_MAX) m_history_table[hash_s]->tornament += 1;
  
  m_history_table[hash_s]->history_history = (m_history_table[hash_s]->history_history << 1) | taken;
  m_global_history = (m_global_history << 1) | taken;

  if (!correct) 
    {
      stats.simp_mispredicts += 1;
      m_uncollected_penalties += 1;
    }

  TRC2(printf("bpredict: branch from %x to %x: predicted_correctly=%i torn=%i\n", pc<<2, dest<<2, correct, m_history_table[hash_s]->tornament)); // The left shifts in this debug statement only work for ARM mode, not Thumb: TODO.  

  return rc;
}



generic_branch_predictor::predictor_stats_s::predictor_stats_s() // constructor
{
  reset();
  //Tenos CSV report
  new tenos_report_item("bp", "simp_operations", &simp_operations);
  new tenos_report_item("bp", "simp_mispredicts", &simp_mispredicts);
  new tenos_report_item("bp", "computed_jumps", &computed_jumps);
  new tenos_report_item("bp", "computed_jump_mispredicts", &computed_jump_mispredicts);
  new tenos_report_item("bp", "pop_jump_operations", &pop_jumps);
  new tenos_report_item("bp", "pop_jump_mispredicts", &pop_jump_mispredicts);
};
 
 
void  generic_branch_predictor::predictor_stats_s::report(const char *msg, FILE *fd, bool resetf)
{
  if (fd) fprintf(fd, "%s: generic_predictor: ops=%li, mispredicts=%li, smp_ratio=%1.3f; "
    "taken=%li, ratio=%1.3f; "
#if KEEP_EXTRA_STATS
    "simp_local_misses=%li, simp_global_misses=%li,  simp_torns_loc=%li, simp_torns_glob=%li, l/(l+g)=%1.3f; "
#endif
    "computed_jumps=%li, computed_jump_mispredicts=%li, mcj_ratio=%1.3f; "
    "pop_jumps=%li, pop_jump_mispredicts=%li, mpj_ratio=%1.3f;"
    "\n", msg, 
    simp_operations, simp_mispredicts, (simp_operations) ? double(simp_mispredicts)/(double)simp_operations: 0.0, 
    simp_taken, (simp_operations) ? double(simp_taken)/(double)simp_operations: 0.0, 
#if KEEP_EXTRA_STATS
    simp_local_misses,    simp_global_misses,  simp_torns_loc, simp_torns_glob, (simp_torns_loc+simp_torns_glob)? (double)(simp_torns_loc)/(simp_torns_loc+simp_torns_glob):0.0,
#endif
    computed_jumps, computed_jump_mispredicts, computed_jumps ? double(computed_jump_mispredicts)/double(computed_jumps):0.0,
    pop_jumps, pop_jump_mispredicts, pop_jumps ? double(pop_jump_mispredicts)/double(pop_jumps):0.0
    );
  if (resetf) reset();
}


void generic_branch_predictor::predictor_stats_s::reset()
{
  computed_jumps = 0;
  computed_jump_mispredicts = 0;
  pop_jumps = 0;
  pop_jump_mispredicts = 0;
  simp_operations = 0;
  simp_mispredicts = 0;
  simp_taken = 0;
#if KEEP_EXTRA_STATS
  simp_local_misses = 0;
  simp_global_misses = 0;
  simp_torns_loc = 0;
  simp_torns_glob = 0;
#endif

}

// eof

