/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */


/////////////////////////////////////////////////////////////////////////////
//
// armcore.h  - Tenos ARM core simulation
// DJG created 29-05-2001
//
/////////////////////////////////////////////////////////////////////////////

#ifndef ARMCORES_H_
#define ARMCORES_H_

/////////////////////////////////////////////////////////////////////////////
//
// Armisa - generic ARM instruction set simulator.
//
/////////////////////////////////////////////////////////////////////////////

#define MAX_WATCHPOINTS 4
#define MAX_BREAKPOINTS 4

#define MAX_NO_OF_CORES 256
#define LOG_MAX_NO_OF_CORES 8

//
// Stop and start the processor using the following command codes
// not used currently - we use the Tenos one or debugger port.
//    int armisa::old_command_cmd(int command_code, u32_t arg);
#define armisa_cmd_stop               0
#define armisa_cmd_enquire            1
#define armisa_cmd_resume             2
#define armisa_cmd_set_bp             3
#define armisa_cmd_clear_bp           4
#define armisa_cmd_reset              5
#define armisa_cmd_drain_write_buffer 6
#define armisa_cmd_cach_clear and so on ...

// The value in the PC register is offset owing to pipelining.
// The execute phase sees an offset of 8, but the decode phase sees this offset.
#define ARM_PC_CORRECTION 4

// TODO: Need to make this configurable from the command line
#define IPC_DEPTH 2

#include "armdis.h"
#include <atomic>

class arm_mmu;


#include "generic_branch_predictor.h"
#include "arm_abt.h"


#if THREAD_COMM
#include <boost/icl/split_interval_map.hpp>

template<typename Type>
struct inplace_replace :
  public boost::icl::identity_based_inplace_combine<Type> {

  typedef inplace_replace<Type> type;

  void operator()(Type& object, const Type& operand) const {
    object = operand;
  }

  static void version(Type&) {
  }
};

typedef boost::icl::interval_map<uint64_t, int,
  boost::icl::partial_absorber,
  /*ICL_COMPARE Compare =*/ ICL_COMPARE_INSTANCE(ICL_COMPARE_DEFAULT, uint64_t), 
  /*ICL_COMBINE Combine =*/ ICL_COMBINE_INSTANCE(inplace_replace, int), 
  /*ICL_SECTION Section =*/ ICL_SECTION_INSTANCE(boost::icl::inter_section, int)
> addr_owners_t;

#endif

// armisa is the core class used by all the armisa models.
class armisa 
{
  void arm_swi_backdoor(int code);
  bool m_tracing_enabled;
  int stop_after_ins_count;
  bool instruction_grace; // Allow the next instruction to run without hitting a breakpoint.


 public:
  alternate_banch_target_buffer *m_abtb; // Alternative branch predictor - for Trampolines.

  void notify_tracing_might_be_needed(bool enable) {  m_tracing_enabled = enable; }
  int m_core_no;
  const char *m_myname;
  armdis dis;
  TargetSignal sigval;
  int thumb_mode;
  int prev_thumb_mode;
  void set_stop_after_count(int n);
  typedef enum { Watch, Break, SingleStep } wp_style_t;
  struct arm_watch_s
  {
    u32_t addr;
    wp_style_t style;
  } watchpoints[MAX_WATCHPOINTS]; // For data operands.
  struct arm_break_s
  {
    u32_t addr;
    wp_style_t style;
  } pc_breakpoints[MAX_BREAKPOINTS]; // For PC values.


  int delete_breakpoint(u32_t addr, int n=-1); // Note: there is a WpHash thing in tenos we could use for this structure?
  int watchpoints_active, breakpoints_active;
  int set_breakpoint(u32_t addr, wp_style_t type = Break);
  void setVectorBase(u32_t addr);
  int  set_watchpoint(u32_t addr, wp_style_t style=Watch);
  void start_emulation();

//------------------------------------
  typedef struct ArmWriteBufferEntry_s
    {
      u32_t addr, data;
      u2_t mas;
      struct ArmWriteBufferEntry_s *next;
  } ArmWriteBufferEntry;


    enum ArmBank
    {
        UserBank,           // And also system mode: the same bank is shared.
        FiqBank,
        SupervisorBank,
        AbortBank,
        IrqBank,
        UndefinedBank
    };
    const char *ArmBankToString(ArmBank) const;

    enum ArmMode            // table in (ARM7 pp 3-9)
    {
        UserMode,
        FiqMode,
        IrqMode,
        SupervisorMode,
        AbortMode,
        UndefinedMode,
        SystemMode
    };
    const char *ArmModeToString(ArmMode) const;

    u32_t vectorBase;

    enum ArmVector          // table in (ARM7 pp 3-13)
    {
        ResetVector         = 0xFFFF0000,      // Reset Supervisor nominal - we use the value from COREISA_IF instead.
        UndefinedVector     = 0xFFFF0004,   // Undefined instruction Undefined
        SwiVector           = 0xFFFF0008,   // Software interrupt Supervisor
        PrefetchVector      = 0xFFFF000C,   // Abort (prefetch) Abort
        DataVector          = 0xFFFF0010,   // Abort (data) Abort
        ReservedVector      = 0xFFFF0014,   // Reserved Reserved
        IrqVector           = 0xFFFF0018,   // IRQ IRQ
        FiqVector           = 0xFFFF001C    // FIQ FIQ
    };
    const char *ArmVectorToString(ArmVector) const;

    // Common register names - usually cast to Integer
    enum ArmReg
    {
        SpReg = 13,
        LrReg = 14,
        PcReg = 15,
        CpsrReg = 16,
        SpsrReg = 17,
	UndefinedReg = 99
    };
    const char *ArmRegToString(ArmReg) const;
    FILE *logfd;

    // ----------------------------------------------------------------------
    // Processor control state

    u1_t fiqSet;
    u1_t irqSet;
    u1_t resetPending;
    u32_t instruction;

    enum ArmInstrType           // (ref ARM7 4.1.1 fig 4-1 pp 44) 
    {
        UnknownT, 
        AluT,
        ParallelAluT,
        MulT,
        MulLongT,
        SwapT,
        BxT,
        HalfwordRegT,
        HalfwordImmT,
        SingleT,                // LDR/STR
        SynchronisationT,       // LDREX(B|H|D)/STREX(B|H|D)
        UndefinedT,
        BlockT,                 // LDM/STM
        BranchT,
        CoprocXferT,
        CoprocOpT,
        CoprocRegT,
        SwiT,
        TableBranchT,
        FloatT,
	ThumbExecutedT,
	ARM32ExecutedT,
	FloatMemoryT,
	FloatALUT
    };

    u32_t pendingInsAddress;      // address of pending prefetch instruction
    u32_t insAddress;             // address of current instruction
    ArmInstrType insDecodeT;    // from decode of current instruction - type
    char insDisasm[128];        // disassembly of current instruction
    u1_t insCcodeTest;            // TRUE if CCodes say yes, execute instruction

    // Counts number of cycle that we waited before
    // serving interrupt because our core model was not
    // in runningS state. To be used for diagnostic purposes
    u32_t interrupt_wait; 

    // Decode utilities
    static inline u32_t Bits(u32_t a, const unsigned bitno, const unsigned width)
    {
        assert(width > 0);
        assert(bitno + width <= 32);
        
        if (bitno > 0) a >>= bitno;
        if (width < 32) a &= (1 << width) - 1;
        return a;
    }
    
    static inline u32_t Bits4(u32_t a, const unsigned bitno)
    {
        assert(bitno + 4 <= 32);
        
        if (bitno > 0) a >>= bitno;
        return a & 15;
    }
    
    static inline u32_t Bit(u32_t a, const unsigned bitno)
    {
        if (bitno > 0) a >>= bitno;
        return a & 1;
    }
    
    static inline u32_t Ror(u32_t a, unsigned count)
    {
        assert(count < 32);
        if (count == 0) return a;
        return (a >> count) | (a << (32 - count));
    }

    typedef enum { MULT_Signed, MULT_SignedLong, MULT_Unsigned, MULT_UnsignedLong } fmult_op_t;

 private:    
    // various fields of a disassembled instruction,
    // as named by the ARM7 instruction type table.
    u32_t insOpcode;
    u32_t insWBdefault;
    u32_t insS;
    u32_t insRn;
    u32_t insRd;
    u32_t insOperand2;
    u32_t insA;
    u32_t insRs;
    u32_t insRm;
    u32_t insRdHi;
    u32_t insRdLo;
    u32_t insRt;
    u32_t insRt2;
    u32_t insOffset;
    u32_t insP;
    u32_t insU;
    u32_t insW;
    u32_t insL;
    u32_t insN;
    u32_t insB;
    u32_t insCRd;
    u32_t insCRn;
    u32_t insCRm;
    u32_t insCP;
    u32_t insAluOperand1;
    u32_t insAluOperand2;
    u64_t insFloatOperand1;
    u32_t insShiftCarryOut;
    u32_t insAluResult;
    u32_t insAluResultHi;
    u32_t insDataAddress;
    u32_t insDataOffset;
    u32_t insMultCount;
    u32_t insMultReg;
    u32_t insMultWriteback;
    u32_t insSigned;
    u32_t insEXCF;
    u32_t insSUser;
    u16_t insMultRegisters;
    u4_t insALUOpcode;
    u2_t insShiftType;
    u1_t insShiftComputed;
    u32_t insShiftValue;
    u64_t insMultAccum;
    fmult_op_t insMultOp;
    u32_t insSWIArg;
    u1_t insFloatSingle;
    bool prefetchIns;

    // IPC analysis
    
    // If bit i is set then it means that register i
    // has been used as a source in the instruction
    u16_t uses[IPC_DEPTH];
    // If bit i is set then it means that register i
    // has been used as a destination in the instruction
    u16_t defs[IPC_DEPTH];

    void add_use(u8_t index, u32_t reg) {
      if(reg != UndefinedReg)
	uses[index] |= 1 << reg;
    }

    void add_def(u8_t index, u32_t reg) {
      if(reg != UndefinedReg)
	defs[index] |= 1 << reg;
    }

    void clear_uses_and_defs(void) {
      for(int i = 0; i < IPC_DEPTH; ++i) {
	uses[i] = 0;
	defs[i] = 0;
      }
    }

    u1_t m_uses_defs_idx;
    // if this flag is set then only dependence
    // between two 16bit instructions is checked
    // if no dependency then it is assumed that
    // they can be issued at the same time, otherwise
    // if this flag is not set (default) then we check
    // for dependency between any two instructions
    // regardless if they are 16bit or 32bit
    bool m_thumb16_only_ipc;
    // states whether last decoded instruction was 32bit thumb
    bool m_thumb32;

    void do_ipc_analysis(void);

 protected:
    void set_thumb16_only_ipc(bool flag) {
      m_thumb16_only_ipc = flag;
    }
    

 public:
    u32_t insSizeCode;

    class stats_c
    {
    public:
      u64_t m_Cycles;
      u64_t m_Instructions;
      u64_t m_InstructionBlocks;
      u64_t m_Stores;
      u64_t m_Loads;
      u64_t m_Floats;
      u64_t m_Branches;
      u64_t m_ALU;
      u64_t m_Unknowns;
      
      void reset()
      {
	m_Cycles = 0;
	m_Instructions = 0;
	m_InstructionBlocks = 0;
	m_Stores = 0;
	m_Loads = 0;
	m_Floats = 0;
	m_Branches = 0;
	m_ALU = 0;
	m_Unknowns = 0;
      }

      void pmu_readoff(u64_t &cp, u64_t &ic)
      {
	cp = m_Cycles;
	ic = m_Instructions;
      }

      void report(const char *msg, FILE *fd, bool resetf)
	{
	  u64_t diff = m_Instructions - (m_Loads + m_Stores + m_Floats + m_Branches + m_ALU + m_Unknowns);
	  if (fd) fprintf(fd, "%s: arm Cycles=%li Instructions=%li Blocks=%li Loads=%li Stores=%li Floats=%li Branches=%li ALU=%li Unknowns=%li Diff=%li\n", msg, m_Cycles, m_Instructions, m_InstructionBlocks, m_Loads, m_Stores, m_Floats, m_Branches, m_ALU, m_Unknowns, diff);

	  if (resetf) reset();
	}

      stats_c()  // constructor
	{ 
	  report(0, 0, true);
	};

    } stats;
 private:

    typedef enum { FMODE_logic32, FMODE_arith32, FMODE_logic64, FMODE_arith64 } fmode_t ;
    void updateFlags(fmode_t, bool carry=false, bool overflow=false);



    // Thumb Instruction decoding routine
    bool DecodeThumbInstruction(bool*);
    bool DecodeThumb16(bool, bool);
    void DecodeThumb16Misc(u16_t, bool);
    void DecodeThumb16LDRLiteral(u16_t);
    void DecodeThumb16ALU1(u16_t in, bool);
    void DecodeThumb16ALU2(u16_t in, bool);
    void DecodeThumb16MemOp(u16_t in);
    void DecodeThumb16BranchAndSVC(u16_t in);
    void DecodeThumb16Special(u16_t in);
    void DecodeThumb16IfThenHints(u16_t in, bool);

    bool DecodeThumb32(bool);
    void DecodeThumb32Branches(u16_t in1, u16_t in2);
    void DecodeThumb32DataProcessingRegister(u16_t in1, u16_t in2);
    void DecodeThumb32Multiply(u16_t in1, u16_t in2);
    void DecodeThumb32ALUShiftedRegister(u16_t in1, u16_t in2);
    void DecodeThumb32LDRSTRMultiple(u16_t in1, u16_t in2);
    void DecodeThumb32Misc(u16_t in1, u16_t in2);
    void DecodeThumb32DataProcessingModifedImmediate(u16_t in1, u16_t in2);
    void DecodeThumb32LongMultiply(u16_t in1, u16_t in2);
    void DecodeThumb32LDRSTRDualExclusive(u16_t in1, u16_t in2);
    void DecodeThumb32LoadWord(u16_t in1, u16_t in2);
    void DecodeThumb32DataProcessingImmediate(u16_t in1, u16_t in2);
    void DecodeThumb32LoadHlfWord(u16_t in1, u16_t in2);
    void DecodeThumb32MoveRegisterAndImmShifts(u16_t in1, u16_t in2);
    void DecodeThumb32STRSingleData(u16_t in1, u16_t in2);
    void DecodeThumb32LoadByteMemoryHints(u16_t in1, u16_t in2);
    void DecodeThumb32CPRSIMDFloatInstrs(u16_t in1, u16_t in2);
    void DecodeThumb32UnsignedParallelAddAndSub(u16_t in1, u16_t in2);


    u32_t ThumbExpandImm(u12_t imm12);


    bool DecodeArmInstruction(bool uncond, bool* executed);
    void DecodeIllegal();           // can be called by DecodeArmInstruction

    /* Float operations */
    int exec_float_memory_instr();
    int exec_float_instr();
    void exec_float_alu_instr();
    
    int exec_mem_operand_instr(); 
    void exec_alu_operand_instr();
    void exec_shift_operand_instr();
    void exec_mul_operand_instr();
    void exec_parallel_alu_operand_instr();


    void DecodeHalfword();
    void ExecuteBL(u32_t npc);

    u1_t DecodeCoprocessor_DT();
    void DecodeCoprocessor_DO();
    void DecodeCoprocessor_RT();
    void DecodeTrap(ArmMode,ArmVector);
    void DecodeSetModeFromCpsr();
    void DecodeMrs();
    void DecodeMsr(int mask);
    void DecodeCcode(u4_t coder);

    // single data transfer, LDR or STR or copro
    void DecodeSingle();  

    void DecodeBarrier(u4_t type, u4_t mask);


    // multiple
    bool DecodeMultiple();
    int DecodeMultiple2();


    enum ArmState // ARM7 style - ARM9 might have fewer states owing to greater parallelism
    {
      //We now use the halted status in the COREISA to control the master stop/start/pause of the core, so do not need StoppedS state.
      //StoppedS,       // Used when a breakpoint is hit.
      ResettingS,     // restarted, setting PC+mode
      StartingS,      // running, abandon old memory request
      //      LoadingS,       // running, loading new instruction
      RunningS,       // running, pipelined instruction fetch issued.
      MidSingleS,     // running, mid-instruction, LDR/STR
      MidHalfwordS,   // running, mid-instruction
      MidCoproXferS,  // running, mid-instruction, Coprocessor memory load or store
      MidMultipleS,   // running, mid-instruction, LDM/STM
      MidMultiple2S,  // running, mid-instruction, LDM/STM, part 2
    };


    int old_command_cmd(int command_code, u32_t arg);

    const char *state_string(unsigned S);
    unsigned midInstruction; // 0 -> start of instruction, else which one

 public:
    void execute();
    void PrintRegs(FILE *, bool allf=false, bool atStartOnly=false);
    void PrintStats();
    const char *myname() { return m_myname; }
	

 private:
    // ----------------------------------------------------------------------
    // ARM registers

    ArmMode currentMode; // current mode
    ArmBank currentBank; // current bank (determined by mode)
    // There are six banks.  This declaration code fully banks all registers.  Access functions only use banks for regs 13, 14(SP,LR) and 17 (SPSR), except FIQ doing 8 to 12 as well.
    u32_t raw_reg[18 * 6];
    //
    // This holds all the ARM registers. (ARM7 3-5 Fig 3-3)
    //
    // Index by 18*bank + reg
    //

    u8_t ITSTATE;
    inline u1_t InITBlock() { return (ITSTATE & 0xF) != 0; }
    inline void ITAdvance() { if((ITSTATE & 0x7) == 0) ITSTATE = 0; else  ITSTATE = (ITSTATE & 0xE0) | (((ITSTATE & 0x1F) << 1) & 0x1F); }

    // floating point registers
    u64_t d_reg[16];
    // floating point status register;
    u4_t fpscr_val;

 public:
    u32_t &Reg(ArmBank bank, unsigned r);
    inline u32_t &Reg(unsigned r) {return Reg(currentBank, r);}
    inline u32_t &Reg(ArmReg r) {return Reg(currentBank, (unsigned) r);}
    inline u1_t IRQFlag() const {return 0 != (raw_reg[CpsrReg] & 0x80UL);}
    inline u1_t FIQFlag() const {return 0 != (raw_reg[CpsrReg] & 0x40UL);}


 private:
    void SetMode(ArmMode b, bool thumb=false); // set bank and mode and CPSR mode bits

    // Processor flags, (ref ARM7 3-8 pp 34)
    inline u1_t NFlag() const {return 0 != (raw_reg[CpsrReg] & 0x80000000UL);}
    inline u1_t ZFlag() const {return 0 != (raw_reg[CpsrReg] & 0x40000000UL);}
    inline u1_t CFlag() const {return 0 != (raw_reg[CpsrReg] & 0x20000000UL);}
    inline u1_t VFlag() const {return 0 != (raw_reg[CpsrReg] & 0x10000000UL);}


    u8_t unsignedsat(u32_t n) {
        if(n > 0xFF)
            return 0xFF;
        else if(n < 0)
            return 0;
        else 
            return n;
    }

    // 
    ArmState runstate;
 public:
    armisa(int core_no, const char *name); // constructor

    union
    {
      u32_t read_data;
      u64_t read_data64;
    };

 private:
    // Memory IO
    // The sequential flag here is a hint from the core generating AMBA busy and the external sequential
    // signals are generated by the BIU detecting adjacent addresses.
    virtual u1_t armisa_write1(u32_t addr, u64_t data, u2_t sf, u32_t *llsc_scp=0) = 0;
    virtual bool armisa_read1(u32_t addr, u2_t sf, u1_t opf, bool linkedf=false, bool mmu=false) = 0;
    virtual u1_t wbuf_logged() = 0;

    // Coprocessor IO - need a pluggable array of sc_modules?
    virtual void copro_write(u4_t cpn, u4_t cp, u4_t cpr, u32_t data, u3_t op1=0, u3_t op2=0) = 0;
    virtual u32_t copro_read(u4_t cpn, u4_t cp, u4_t cpr, u3_t op1=0, u3_t op2=0) = 0;

    virtual void iss_yield(int cycles) = 0;

    // Command and Control.
    virtual int arm_sim_command(int, int, int, u32_t *) = 0;


    virtual u32_t check_tenos_trace(u32_t mask, u32_t pc) = 0; // Slow path.

    // Tracing check - needs to be fast and inlineable
    u32_t user_trace(u32_t mask, u32_t pc)
    {
      //printf("user_trace in armisa.h m_tracing_enabled=%i\n", m_tracing_enabled);
      if (!m_tracing_enabled) return 0; // Fast path.
      u32_t m = check_tenos_trace(pc, mask); // Slow path - note arg order is swapped.
      //printf("user_trace in armisa.h m=%x\n", m);
      return m;
    }

 public:
  // These following ones are part of the debug port "interface" that serve_debugger uses - but not a formal interface since serve_debugger is locally customised for an IP block.
    TargetSignal get_sigval(); // Return the most recent exception (reason why the core is stalled/halted - e.g. SEGV or breakpoint).
    u32_t get_pc() { return Reg(PcReg); }

    u32_t get_reg(int x) { return Reg(x); } // ensure is inlined!
    u32_t get_spr(int x);
    void set_spr(int x, int d);
    void set_reg(int x, int d);
    void iss_reset();
 private:
    // Keep as much private as possible to allow gcc to optimise.
    void core_reset();
    //
    //

    u1_t prevReadPending;
    u1_t nextReadPending;
    u1_t lastReadPending;
    u32_t lastAddress;
    u32_t nextAddress;
    u32_t prevAddress;
    
    u32_t nextWriteData;
    u1_t nextWritePending, prevWritePending;

    u32_t oldData, thisData;
    u1_t busCyclePending;

    void bodystate_clock(); 

    u1_t nFIQ, nIRQ;

 public:
    bool hw_interrupt(const char *msg);
    bool wait_for_interrupt;
    bool wait_for_event;
    u1_t event_register;
    u32_t last_event;

 public:

    std::atomic<u32_t>* inter_core_event;
    // used to see if sequence of instructions is
    // what Linux issues when shutdown is executed.
    // The sequence of instructions should be:
    //   (a) on the main core (usually core 0) the instruction
    // is a branch that branches to itself 
    //   (b) for all other cores the sequence of instructions
    // is first DMB ISH instruction followed by a branch to the
    // previous instruction
    bool shutdown_sequence;
    bool last_shutdown_sequence;


    generic_branch_predictor *m_branch_predictor;
    u32_t predictor_op(u32_t pc, bool taken, u32_t dest, int push_pop_style=0)
    {
      u32_t rc = 0xffffFFFF;
      
      if (rc == 0xffffFFFF && m_branch_predictor)
	rc = m_branch_predictor->operate(pc, taken, dest, push_pop_style);
      
      return rc;
    }
};


extern void armdis(u32_t loc, u32_t a, char *buffer);

// Prazor backdoor SWI codes for ARM
// Note Thumb mode only has an 8 bit SWI code space.
#define ARM_SWI_BACKDOOR_BASE 200
#define ARM_SWI_BACKDOOR_DONE 1
#define ARM_SWI_BACKDOOR_WRCH 2
#define ARM_SWI_BACKDOOR_TRACE100 3
#define ARM_SWI_BACKDOOR_DUMP_STATS 4
#define ARM_SWI_BACKDOOR_PMU_CYCLES 5
#define ARM_SWI_BACKDOOR_PMU_INSTRUCTIONS 6
#define ARM_SWI_BACKDOOR_STATS_CHECKPOINT 7

#endif
// eof
