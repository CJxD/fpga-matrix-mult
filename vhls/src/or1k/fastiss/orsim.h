/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

#ifndef ORSIM_H
#define ORSIM_H

typedef unsigned int uint32_t;
typedef signed int int32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

#define LONGEST long long
#define ULONGEST unsigned long long
#include <stdio.h>

#define MAX_GPRS 32

#include "or32.h"

typedef uint32_t  uorreg_t;	/*!< An unsigned register of openrisc */
typedef uint32_t  oraddr_t;	/*!< Address as addressed by openrisc */
typedef uint32_t  uorreg_t;     /*!< An unsigned register of openrisc */
typedef int32_t  orreg_t;       /*!< A signed register of openrisc */

enum boolean {
  FALSE = 0,
  TRUE  = 1
};

#include "../spr_defs.h"
#define EXCEPT_TRAP     0x0e00
#define MAX_SBUF_LEN 256

struct bpbstat
{
  int hit;
  int miss;
  int correct;
  int incorrect;
};


struct bticstat
{
  int hit;
  int miss;
};

struct mstats_entry
{
  int byteadd;
  int bf[2][2];			/* [taken][fwd/bwd] */
  int bnf[2][2];		/* [taken][fwd/bwd] */
  struct bpbstat bpb;
  struct bticstat btic;
};				/*!< misc units stats */





struct runtime
{
#if 1
  struct
  {
    FILE *fprof;		/* Profiler file */
    FILE *fmprof;		/* Memory profiler file */
    FILE *fexe_log;		/* RTL state comparison file */
    FILE *fout;			/* file for standard output */
    char *filename;		/* Original Command Simulator file (CZ) */
    int iprompt;		/* Interactive prompt */
    int iprompt_run;		/* Interactive prompt is running */
    long long cycles;		/* Cycles counts fetch stages */
    long long int end_cycles;	/* JPB. Cycles to end of quantum */
    //double time_point;		/* JPB. Time point in the simulation */

    int mem_cycles;		/* Each cycle has counter of mem_cycles;
				   this value is joined with cycles
				   at the end of the cycle; no sim
				   originated memory accesses should be
				   performed inbetween. */
    int loadcycles;		/* Load and store stalls */
    int storecycles;

    long long reset_cycles;

    int  hush;			/* Is simulator to do reg dumps */
  } sim;
#endif

  struct
  {
    int instructions;	/* Instructions executed */
    int reset_instructions;
    uint64_t delperiods;
    int stalled_;
    int hazardwait;		/* how many cycles were wasted because of hazards */
    int supercycles;		/* Superscalar cycles */
  } cpu;

  struct
  {				/* Verification API, part of Advanced Core Verification */
    int enabled;		/* Whether is VAPI module enabled */
    FILE *vapi_file;		/* vapi file */
    int server_port;		/* A user specified port number for services */
  } vapi;

/* CUC configuration parameters */
  struct
  {
    //int mdelay[4];		/* average memory delays in cycles
    //				   {read single, read burst, write single, write burst} */
      //double cycle_duration;	/* in ns */
  } cuc;
};


/*! Sets a new SPR_SR_OV value, based on next register value */
#if SET_OV_FLAG
#define SET_OV_FLAG_FN(value) \
  if((value) & 0x80000000) \
    cpu_state.sprs[SPR_SR] |= SPR_SR_OV; \
  else \
    cpu_state.sprs[SPR_SR] &= ~SPR_SR_OV
#else
#define SET_OV_FLAG_FN(value)
#endif


extern struct dev_memarea *reg_mem_area (oraddr_t        addr,
					 uint32_t        size,
					 unsigned        mc_dev,
					 struct mem_ops *ops);



struct myconfig_s
{

  struct debug
  {
    bool gdb_enabled;
  } debug;

  struct  bpb
  {
    bool enabled;
  } bpb;

  struct sim
  {
    int profile;
    bool verbose;
    int exe_log;		/* Print out RTL states? */
    int exe_log_type;		/* Type of log */
    int history;
    long long int exe_log_start;	/* First instruction to log */
    long long int exe_log_end;	/* Last instr to log, -1 if continuous */
    int exe_log_marker;		/* If nonzero, place markers before */
  } sim;

  struct cpu
  {
    int sbuf_len;
    bool superscalar;
    int hazards;
    int dependstats;		/* dependency statistics */

    void defaults()
    {
      sbuf_len = 1;
      superscalar = false;
      hazards = 0;
      dependstats = 0;
    }
    cpu() { defaults(); } // constructor
  } cpu;

};

extern struct myconfig_s myconfig;


struct label_entry
{
  char *name;
};

#define EXE_LOG_SOFTWARE   2 /* Simple with some register output */
#define EXE_LOG_SIMPLE     1 /* Executed log prints out dissasembly */
#define EXE_LOG_HARDWARE   0	/* Print out RTL states */

// extern struct runtime runtime;
extern void      dump_exe_log ();

struct hist_exec
{
  oraddr_t addr;
  struct hist_exec *prev;
  struct hist_exec *next;
};


extern oraddr_t peek_into_itlb (oraddr_t virtaddr);
extern oraddr_t peek_into_dtlb (oraddr_t virtaddr, int, int);
extern bool has_breakpoint(oraddr_t virtaddr);


struct iqueue_entry
{
  uint32_t insn, insn_index, insn_addr;
};


extern void addsstats (int item, int cnt_dynamic);
extern void adddstats (int item1, int item2, int cnt_dynamic, int depend);
extern void addfstats (enum insn_type item1, enum insn_type item2,
		       int cnt_dynamic, int depend);

#define HISTEXEC_LEN               200


class orsim
{

  bool halted;  // True if not running (waiting for interrupt or other wakeup).
 public:
  bool is_halted() { return halted; }
  void stall(bool unstall)
  {
    halted = !unstall;
  }
 private:
/*!The main structure holding the current execution state of the CPU

   Not to be confused with @c runtime, which holds the state of the
   simulation.

   @c insn_ea field is only used to get dump_exe_log() correct.

   @c iqueue and @c icomplet fields are only used in analysis().

   The micro-operation queue, @c opqs, is only used to speed up
   recompile_page().                                                         */
  struct cpu_state {
    uorreg_t             reg[MAX_GPRS];	/*!< General purpose registers */
    uorreg_t             sprs[MAX_SPRS];	/*!< Special purpose registers */
    oraddr_t             insn_ea;		/*!< EA of instrs that have an EA */
    int                  delay_insn;	/*!< Is current instr in delay slot */
    int                  npc_not_valid;	/*!< NPC updated while stalled */
    oraddr_t             pc;		/*!< PC (and translated PC) */
    oraddr_t             pc_delay;	/*!< Delay instr EA register */
    uint32_t             pic_lines;	/*!< State of PIC lines */
    struct iqueue_entry  iqueue;		/*!< Decode of just executed instr */
    struct iqueue_entry  icomplet;        /*!< Decode of instr before this */
    
#if DYNAMIC_EXECUTION
    jmp_buf              excpt_loc;	/*!< Longjump here for exception */
    struct dyn_page     *curr_page;	/*!< Current page in execution */
    struct dyn_page    **dyn_pages;	/*!< Pointers to recompiled pages */
    int32_t              cycles_dec;
  struct op_queue     *opqs;		/*!< Micro-operation queue */
#endif
  } cpu_state;




  /*! Temporary program counter. Globally available */
  oraddr_t  pcnext;
  
  /*! Num cycles waiting for stores to complete. Globally available */
  int  sbuf_wait_cyc;
  
  /*! Number of total store cycles. Globally available */
  int  sbuf_total_cyc;
  
  /*! Whether we are doing statistical analysis. Globally available */
  int  do_stats;
  
  /*! History of execution. Globally available */
  struct hist_exec *hist_exec_tail;
  
  /* Benchmark multi issue execution. This class only */
  int  multissue[20];
  int  issued_per_cycle;
  
  /* Store buffer analysis - stores are accumulated and commited when IO is
     idle. This class only */
  int  sbuf_head             ;
  int  sbuf_tail             ;
  int  sbuf_count            ;

#if !(DYNAMIC_EXECUTION)
  int  sbuf_buf[MAX_SBUF_LEN];
#endif

  int sbuf_prev_cycles;


  int break_just_hit; // Disables instant rebreak on continue.

  /* Variables used throughout this class to share information */

  int  breakpoint;
  int  next_delay_insn;

  struct mstats_entry      or1k_mstats;
  struct myconfig_s myconfig;
  bool except_pending;
  struct runtime runtime;
  //struct breakpoint_entry *breakpoints;

  // Methods


  void except_handle(unsigned int, unsigned int);
  uorreg_t evalsim_reg (unsigned int  regno);
  struct dev_memarea *verify_memoryarea (oraddr_t addr);
  void  set_stall_state (int state);
  void sbuf_load ();
  void sbuf_store (int cyc);
  void setsim_reg (unsigned int  regno, uorreg_t      value);
  int fetch ();
  void update_pc();
  void analysis (struct iqueue_entry *current);
  void decode_execute_wrapper (struct iqueue_entry *current);

  oraddr_t peek_into_itlb (oraddr_t virtaddr);
  oraddr_t peek_into_dtlb (oraddr_t virtaddr, int, int);
  bool has_breakpoint(oraddr_t virtaddr);
  void addsstats (int item, int cnt_dynamic);
  void adddstats (int item1, int item2, int cnt_dynamic, int depend);
  void addfstats (enum insn_type item1, enum insn_type item2, int cnt_dynamic, int depend);




#if SIMPLE_EXECUTION
  void l_invalid (struct iqueue_entry *current);
#else
  void l_invalid ();
#endif
  void exit_backdoor(const char *m);
  void decoder (struct iqueue_entry *current);

 protected:
  friend class remotecontrol_or1k;
  virtual void                corepause(int, u32_t a) = 0;
  virtual void                atomic_prefix() = 0;
  virtual uint32_t            eval_insn(oraddr_t addr, int *bp) = 0;
  virtual uint32_t            eval_mem32 (oraddr_t memaddr, int *) = 0;
  virtual uint16_t            eval_mem16 (oraddr_t memaddr, int *) = 0;
  virtual uint8_t             eval_mem8 (oraddr_t memaddr, int *) = 0;
  virtual uint8_t             eval_direct8(oraddr_t, int, int) = 0;

  virtual void                set_mem32 (oraddr_t memaddr, uint32_t value, int *) = 0;
  virtual void                set_mem16 (oraddr_t memaddr, uint16_t value, int *) = 0;
  virtual void                set_mem8 (oraddr_t memaddr, uint8_t value, int *) = 0;
  virtual void                set_direct8 (oraddr_t, uint8_t, int, int) = 0;
  virtual int simulator_cmd(int, int, int, u32_t *) = 0;


  void orSpr(int a, u32_t v) { set_spr(a, get_spr(a) | v); }
  void andSpr(int a, u32_t v) { set_spr(a, get_spr(a) & v); }
 private:


  int disassemble_index(unsigned long a, int d);
  struct label_entry *get_label (oraddr_t addr);
  void disassemble_memory (oraddr_t  from, oraddr_t  to, int nl);

  void orsim_sim_done (const char *msg);
  void l_none(struct iqueue_entry *current);
  void l_none(struct op_queue *opq, int *param_t, orreg_t *param, int delay_slot);
  void l_none (void);
  uorreg_t  mfspr (const uint16_t  regno);
  uorreg_t  mtspr (const uint16_t  regno);
  uorreg_t  mfspr (const uint16_t  regno, uorreg_t &a);
  uorreg_t  mtspr (const uint16_t  regno, uorreg_t &a);

 public:

  // These following ones are part of the debug port "interface" that serve_debugger uses - but not a formal interface since serve_debugger is locally customised for an IP block.
  oraddr_t get_pc() { return cpu_state.pc; }
  oraddr_t get_reg(int x) { return cpu_state.reg[x]; } // ensure is inlined!
  oraddr_t get_spr(int x);
  void set_spr(int x, int d) { assert(x>=0 && x<MAX_SPRS); cpu_state.sprs[x] = d; }
  void set_reg(int x, int d) { assert(x>=0 && x<MAX_GPRS); cpu_state.reg[x] = d; }
  void core_reset();


  const char *sname;
  void hw_interrupt(const char *msg);
  orsim(const char *sname); // constructor
  void dump_exe_log ();
  void dumpreg(FILE *, bool fullset);
  void dumpreg();

  int step();
  void core_stat_report(const char *msg, FILE *fd, bool resetf=false);
  void log_delperiods(u64_t delperiods, u32_t addr);

 private:
  void  btic_update (oraddr_t targetaddr);
  void  bpb_update (oraddr_t addr, int taken);

};

#endif
