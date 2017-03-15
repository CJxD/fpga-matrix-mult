#ifndef MIPS641_H
#define MIPS641_H


// $Id: mips64.h,v 1.19 2011/06/21 11:41:42 my294 Exp $
// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2009-10 D J Greaves
// University of Cambridge, Computer Laboratory.
// SoC D/M Classes - CBG Prazor  


// This is a MIPS 64 ISS that can be instantiated on its own or as a blocking TLM SystemC module.
// This iss core is designed to be barrelled N_BARREL ways, but runs only as way 0 by default.
// To enable barelling, please extend the class with something that adds a scheduler.
// An extended version with presence bits and certain extra instructions needed for the C3D Mamba project.
//

#include <assert.h>
#include "systemc.h"
#include "tenos.h"
//#include "gdbrsp.h"

#include "cpu_busaccess.h"


#ifndef POWER3
#ifdef TLM_POWER3
#include <tlm_power>
using namespace sc_pwr;
#define POWER3(X) X
#else
typedef tlm::tlm_base_protocol_types PW_TLM_TYPES;
typedef tlm::tlm_generic_payload PW_TLM_PAYTYPE;
#define POWER3(X)
#endif
#endif



#define mips_addr_t u64_t


#define RESET_VECTOR 0x80 // xBFC00000

#define EXCEPTION_VECTOR 0x80000080
#define UTLB_VECTOR 0x80000000


#define EXCEPTION_VECTOR_BOOT 0xbfc00100
#define UTLB_VECTOR_BOOT 0xbfc00180

#define IMED 1
#define JTYPE 2
#define IDXD 4
#define JTYPE_R 8
#define JTYPE_2R 16

#define BLTZ      0x00
#define BGEZ      0x01
#define BLTZAL    0x10
#define BGEZAL    0x11

#define uint1_t unsigned char // sc_uint<1>

#define N_BARREL 8

// The PC appears to have this offset when executing an instruction.
//
#define PC_CORRECT  4LU  /* Amount by which pc is in advance of next instruction at decode */



#define MIPS64_COUNT_INSTRUCTIONS(X) X
     
class mips64
{


   struct decoder_s
   {
     uint32_t f0;
     POWER3(pw_energy energy);
     const char * opcode;
   }  opcodes0[64], specials0[64];

   void ins_opcode(int f0, int ec, const char *a, int b);
   void ins_special(int f0, int ec, const char *a, int b);

   POWER3(virtual void energy_event(pw_energy) = 0);

  void disinit();

  virtual void sim_done() = 0;
  // The following I/O functions are pure virtual in the iss core, and must be provided here:
  virtual void                corepause(int us) =0;  // Pause CPU for this time interval


  virtual bool                eval_mem64 (mips_addr_t memaddr, u64_t &d, breakpt_t *bp, bool update_presence_flag=false, bool new_polarity=false) =0;
  virtual bool                eval_mem32 (mips_addr_t memaddr, u32_t &d, breakpt_t *bp, bool linked=false) =0;
  virtual bool                eval_mem16 (mips_addr_t memaddr, u16_t &d, breakpt_t *bp) =0;
  virtual bool                eval_mem8 (mips_addr_t memaddr, u8_t &d, breakpt_t *bp) =0;
  virtual void                set_mem64 (mips_addr_t memaddr, u64_t value, breakpt_t *bp, bool mamba_swa_swp=0) =0;
  virtual void                set_mem32 (mips_addr_t memaddr, u32_t value, breakpt_t *bp) =0;
  virtual int                 cpu_mem_store_conditional32(mips_addr_t memaddr, u32_t value) =0;
  virtual void                set_mem16 (mips_addr_t memaddr, u16_t value, breakpt_t *bp) =0;
  virtual void                set_mem8 (mips_addr_t memaddr, u8_t value, breakpt_t *bp) =0;
  //virtual uint32_t            eval_compare_and_swap(uint32_t memaddr, uint32_t wdata, sc_time &delay) =0;
  virtual void                set_direct8 (mips_addr_t, uint8_t, int, breakpt_t *bp) =0;
  virtual uint8_t             eval_direct8(mips_addr_t, int, int, breakpt_t *bp) =0;
  virtual void                atomic_prefix() =0; // Prefix following load/store pair as atomic.

  enum { ENERGY_UNDEF, ENERGY_1, ENERGY_2, ENERGY_3, ENERGY_4 };
  int etable[5]; // energy per instruction class (crude)
  float VCC; // supply voltage (volts)


  // Core Registers
  class barrel
  {
    MIPS64_COUNT_INSTRUCTIONS(u32_t instruction_count);
    int spoke; // What  number
    class mips64 *node;
    friend class mips64;
    friend class mamba_btlm;
    void fatal(const char *);
  protected:
    u64_t regfile[32];
    u64_t multhi, multlo, cp0_status, pv_context;
    u64_t pc, next_pc;
#define ICACHE_LOGSIZE 10LLU
#define ICACHE_LINES (1<<ICACHE_LOGSIZE)
    // Instruction cache
    struct {
      u64_t a, d; 
    } icache[ICACHE_LINES];
    
    // Coprocessor 0 Registers used for exceptions
    u64_t Cause, EPC, Status, BadVAddr, context, PRId;


    

#ifdef USE_CPU_PRESENCE_BITS
    bool m_ready; // Set when all bits present... clear'd when attemt to use a register that is not present.
    bool regfile_snoop(u5_t reg, bool write, u64_t &data);
    u32_t presencebits;
    u32_t masks[32];
    bool checkpres(u5_t v)
    {
      assert(v <= 31);
      return (presencebits & masks[v]);
    }

    int presence_munge(u5_t r)
    {
      return r;
    }

    void unready(u5_t why) { m_ready = false; }

    bool presreg(u5_t r)
    {
      if (!r) return 1;      // Register 0 always present (contains zero).
      return checkpres(r);
    }

    void set_presence(u5_t v) {  presencebits |=  masks[v]; if (presencebits == 0xFFFFffff) m_ready = true; }
    void clear_presence(u5_t v) {  presencebits &=  ~masks[v]; }
    
    bool pres_pc() { return checkpres(0); }
    bool pres_lo() { return presreg(26); }
    bool pres_hi() { return presreg(27); }

    void set_presence_word(u32_t d)  { presencebits = d; };

#define PRESENCECHECK(X) X
#else
#define PRESENCECHECK(X)  
#endif
    u1_t stalled;
    u64_t write_back_data;
    u5_t write_back_reg;
    u1_t write_back_flag;
    u64_t write_back_data1;
    u5_t write_back_reg1;
    u1_t write_back_flag1;
    u8_t skip_next;
    u32_t next_ins;
    // Methods
    
    void overflow_xnn();
    void regfile_up_xflow32(u5_t d, char op, u64_t a0, u64_t a1);
    void regfile_up_xflow64(u5_t d, char op, u64_t a0, u64_t a1);
    int cpu_mem_r32i(mips_addr_t a, u32_t *d); 
    void dumpregs(FILE *fd, bool allregs, int cmd=0);
    int step();
    void regfile_up(u5_t, u64_t);
    void setpc(u64_t);
    void reset(bool enable_to_run);
    u64_t coprocessor_read(u8_t cp, u5_t rd, u3_t sel);
    void coprocessor_write(u8_t cp, u5_t rd, u3_t sel, u64_t data);

  // constructor
    barrel(class mips64 *par, int n);

  public:

    bool ready()
    {
      bool rv = 0;
      // PRESENCECHECK(rv = (presencebits == 0xFFFFffff));
      PRESENCECHECK(rv = m_ready);
      return rv;
    }

    int get_presence_word()
    {
      u32_t rv = 0;
      PRESENCECHECK(rv = presencebits);
      return rv;
    }

  };

  int cpu_mem_write(u64_t d, mips_addr_t adr, uint1_t bytef);
  int cpu_mem_r64(mips_addr_t a, u64_t *d);
  int cpu_mem_r32(mips_addr_t a, u64_t *d, bool locked=false);
  int cpu_mem_r16(mips_addr_t a, u64_t *d);
  int cpu_mem_r8(mips_addr_t a, u64_t *d);


 protected:
  int eflip;
  barrel *barrels[N_BARREL];
  int way;

  // protected methods:
  int step();
  void core_reset(bool enable_first_to_run);
  void irq();


 public: 
  u64_t reset_vector;
  const char *corename;
  //  gdbrsp *rsprsp;
  virtual int reg_trace(mips_addr_t trace_pc) = 0;
  traceregion **traceregions_ptr;
 //constructor
  mips64(const char *name, u8_t pID, traceregion **ptr); // class tiny_mips *wrap) : wrap(wrap)
  void dumpregs(FILE *fd, bool fullregs=false, int cmd=0);
#ifdef USE_CPU_PRESENCE_BITS

// define processor memory spacing - 512kB per processor spoke
// 512*1024=52488
#define procMemSpacing 524288LLU
  bool node_snoop_callback(u64_t addr, bool write, u64_t &data);
  u64_t framebase;

#endif

};



#endif

