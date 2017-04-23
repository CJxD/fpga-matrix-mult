/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */


#ifndef ARM_ABT_H__
#define ARM_ABT_H__

#include "prazor.h"
#include "tenos.h"
#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"
#include <set>
#include <map>


class bloom_filter2
{
  int m_bloom_capacity, m_mask;
  u32_t *m_bits;


 void set_bit(u32_t xx)
 {
   m_bits[xx>>5] |= (1<<(xx & 0x1F));
 }

 bool test_bit(u32_t xx)
 {
   return (m_bits[xx>>5] & (1<<(xx & 0x1F))) ? 1:0;
 }

 public:

 void reset_all()
   {
     for (int i =0; i<(m_bloom_capacity+31)/32; i++) m_bits[i] = 0;
   }
  
  bloom_filter2(int logsize = 22) // constructor
    {
      m_mask = (1<<logsize) - 1;
      m_bits = new u32_t [(m_bloom_capacity+31)/32]; // Just a bit map
      reset_all();
    }


 void set_entry(u32_t xx)
    {
      u32_t h1 = (xx ^ (xx >> 16)) & m_mask;
      set_bit(h1);
      h1 ^= (h1 * 151) & m_mask;
      set_bit(h1);
    }


 bool test_entry(u32_t xx)
    {
      u32_t h1 = (xx ^ (xx >> 16)) & m_mask;
      if (!test_bit(h1)) return 0;
      h1 ^= (h1 * 151) & m_mask;
      return test_bit(h1);
    }
};



// The following ASPLOS paper describes the ABTB structure:
// http://compas.cs.stonybrook.edu/~mferdman/downloads.php/ASPLOS15_Architectural_Support_for_Dynamic_Linking.pdf
class alternate_banch_target_buffer :
 public sc_module
#ifdef TLM_POWER3
  , public pw_module
#endif
{
  int m_abt_capacity;

  struct abtab_entry
  {
    u32_t tag;
    u32_t final_dest;
    abtab_entry()  // constructor
      {
	invalidate();
      };

    void invalidate()
      {
	tag = -1;
      };

  };


  bloom_filter2 m_bloomer;
  abtab_entry **m_abtable; 
  u32_t m_mask;

  int hash(u32_t x) { return (x>>2) & m_mask;  }



 public:

   tlm_utils::simple_target_socket<alternate_banch_target_buffer, 64, PW_TLM_TYPES> *snoop_socket;

  struct alt_predictor_stats_s
  {
  public:
    u64_t installs, clears, deployments, instructions;
    
    void report(const char *msg, FILE *fd, bool resetf)
    {
      if (fd)
	fprintf(fd,
		"%s: trampoline bypass: "
		"installs=%li, clears=%li, deployments=%li\n"
		"\n", msg, 
		installs, clears, deployments		      
		);

      // Prints out how many instructions in a trampoline
      // have been executed
      if (fd)
	fprintf(fd, "%s: trampoline instructions %li\n",
		msg, instructions);
      
      if (resetf) reset();
    }

    void reset()
    {
      installs = 0;
      clears = 0;
      deployments = 0;
      instructions = 0;
    }
  } stats;


  void b_access(PRAZOR_GP_T &trans, sc_time &delay);

  // constructor
  alternate_banch_target_buffer(const sc_module_name &module_name,  bool with_snoop_socket, int logsize=8, int bloom_logsize=22);


  struct 
  {
    int t_det;

    std::map<u32_t, u32_t> via_to_src;
    std::set<u32_t> via;
    std::set<u32_t> abtb_via_entries;
  } abt;

  // Update operations
  void ins32_monitor(u32_t in, u32_t ip, u32_t pc);
  void install_alt(u32_t a, u32_t rd);

  // Write snoop
  void written_address_snoop(u32_t address, u32_t data);

  // Use operations
  u32_t query(u32_t pc, u32_t* cpsr);

  // Invalidate
  void invalidate_all();
};

#endif
// eof
