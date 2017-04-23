#ifndef TENOS_H
#define TENOS_H

// 
// $Id: $
// Part of prazor-virtual-platform
// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2010-15 DJ Greaves, University of Cambridge, Computer Laboratory.
//
// General support code - all architecures



#include <vector>
#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <atomic>
#include <assert.h>

#define u1_t uint8_t
#define u2_t uint8_t
#define u3_t uint8_t
#define u4_t uint8_t
#define u5_t uint8_t
#define u6_t uint8_t
#define u7_t uint8_t
#define u8_t uint8_t

#define u9_t uint16_t
#define u10_t uint16_t
#define u11_t uint16_t
#define u12_t uint16_t
#define u13_t uint16_t
#define u14_t uint16_t
#define u15_t uint16_t
#define u16_t uint16_t

#define u17_t uint32_t
#define u18_t uint32_t
#define u19_t uint32_t
#define u20_t uint32_t
#define u21_t uint32_t
#define u22_t uint32_t
#define u23_t uint32_t
#define u24_t uint32_t
#define u25_t uint32_t
#define u26_t uint32_t
#define u27_t uint32_t
#define u28_t uint32_t
#define u29_t uint32_t
#define u30_t uint32_t
#define u31_t uint32_t
#define u32_t uint32_t


#ifndef TENOS_INT64
  #if defined(INT64_MAX)
    #define TENOS_INT64 int64_t
  #elif defined(_MSC_VER)
    #define TENOS_INT64 __int64
  #elif LONG_MAX == 2147483647
    /* Guess an LP32 machine:  long long int is 8 bytes */
    #define TENOS_INT64 long long int
  #else
    #define TENOS_INT64 long int
  #endif
#endif



// This seems to need a different definition on 32 and 64 bit machines?  
// Also the LL suffix on constants needs to be different!
#if LONG_MAX == 2147483647
//This is a 32 bit machine
#define TENOS_A32
#define PFX64   "%llx"
#define PFLX64  "%llx"
#define PFI64   "%lli"
#define PFU64   "%llu"
#else
#define TENOS_A64
#define PFX64   "%lx"
#define PFLX64  "%llx"
#define PFI64   "%li"
#define PFU64   "%lu"
#endif

#if defined(UINT64_MAX)
  #define TENOS_UINT64 uint64_t
#else
  #define TENOS_UINT64 unsigned TENOS_INT64
#endif


#define u33_t TENOS_UINT64
#define u34_t TENOS_UINT64
#define u35_t TENOS_UINT64
#define u36_t TENOS_UINT64
#define u37_t TENOS_UINT64
#define u38_t TENOS_UINT64
#define u39_t TENOS_UINT64
#define u40_t TENOS_UINT64
#define u41_t TENOS_UINT64
#define u42_t TENOS_UINT64
#define u43_t TENOS_UINT64
#define u44_t TENOS_UINT64
#define u45_t TENOS_UINT64
#define u46_t TENOS_UINT64
#define u47_t TENOS_UINT64
#define u48_t TENOS_UINT64
#define u49_t TENOS_UINT64
#define u50_t TENOS_UINT64
#define u51_t TENOS_UINT64
#define u52_t TENOS_UINT64
#define u53_t TENOS_UINT64
#define u54_t TENOS_UINT64
#define u55_t TENOS_UINT64
#define u56_t TENOS_UINT64
#define u57_t TENOS_UINT64
#define u58_t TENOS_UINT64
#define u59_t TENOS_UINT64
#define u60_t TENOS_UINT64
#define u61_t TENOS_UINT64
#define u62_t TENOS_UINT64
#define u63_t TENOS_UINT64
#define u64_t TENOS_UINT64

// These macros are for little-endian modelling workstation (such as x86) and convert when the modelled
// hardware is big endian.
#define ES16(BIG_ENDIAN, X) ((BIG_ENDIAN)?(((X)>>8)&0xFF)|(((X)<<8)&0xFF00):(X))
#define ES32(BIG_ENDIAN, X) ((BIG_ENDIAN)?(((X)>>24)&0xFF)|(((X)>>8)&0xFF00)|(((X)<<8)&0xFF0000)|((X)<<24):(X))
#define ES64(BIG_ENDIAN, X) ((BIG_ENDIAN)? \
   (((X)>>56LU)&0x00000000000000FFLLU)|   \
   (((X)>>40LU)&0x000000000000FF00LLU)|   \
   (((X)>>24LU)&0x0000000000FF0000LLU)|   \
   (((X)>>8LU )&0x00000000FF000000LLU)|   \
   (((X)<<8LU )&0x000000FF00000000LLU)|   \
   (((X)<<24LU)&0x0000FF0000000000LLU)|   \
   (((X)<<40LU)&0x00FF000000000000LLU)|   \
   (((X)<<56LU)&0xFF00000000000000LLU)    \
   : X)



extern int bound_log2(u64_t);
extern u64_t prazor_mask(int w);
extern int pow_2(u64_t);

// gets n bits from position p -- can use the prazor_mask - might be quicker?
inline u64_t getbits(u64_t x, u64_t p, u64_t n) {
  return (x >> (p+1-n)) & ~(~((u64_t)0) << n);
}

extern volatile bool sim_overp();

class tenos_simcontrol
{
  int m_active_workers;
  
  //  sc_mutex xlk;

 public:
 tenos_simcontrol()
  //xlk("xlk")
  { }; // constructor
  
  void start();
  void stop();

};


// Int to Void and Void to Int: without warnings on m32 or m64.
#define I2V(X) ((void *)((X) & 0xFFFFffffLLU))
#define V2I(X) (((unsigned long long int)X) & 0xFFFFffffLLU)

extern void tenos_assert_fail(const char *m, const char *iname, const char*file, int line);
// A fancy assert macro.  If this does not work on your system, just use normal assert.
#define sysc_assert1(where, expr)					\
  (expr) ? static_cast<void>(0):  tenos_assert_fail (__STRING(expr), where->name(), __FILE__, __LINE__)

#define sysc_assert(expr)  \
  (expr) ? static_cast<void>(0):  tenos_assert_fail (__STRING(expr), name(), __FILE__, __LINE__)



// Every sc_module should define this as a method.
#define TENOS_KIND_DEFINITION(X) const char * X :: kind() { return #X; }



#define TENOS_TRACE_CPU_KEY_REGS         (1<<0)
#define TENOS_TRACE_CPU_ALL_REGS         (1<<1)
#define TENOS_TRACE_MEM_READ             (1<<2)
#define TENOS_TRACE_MEM_WRITE            (1<<3)
#define TENOS_TRACE_CPU_READ             (1<<4)
#define TENOS_TRACE_CPU_WRITE            (1<<5)
#define TENOS_TRACE_IO_READ              (1<<6)
#define TENOS_TRACE_IO_WRITE             (1<<7)

#define TENOS_TRACE_CPU_INTERRUPTS       (1<<9) // 0x200
#define TENOS_TRACE_DEVICE_INTERRUPTS    (1<<10)
#define TENOS_TRACE_CPU_IFETCH           (1<<11) //0x800
#define TENOS_TRACE_CPU_IDECODE          (1<<12)
#define TENOS_TRACE_CPU_EXECUTE          (1<<12) /*same as decode for now*/

#define TENOS_TRACE_BUS_OP               (1<<13)
#define TENOS_TRACE_BUS_ROUTING_DECISION (1<<15)

#define TENOS_TRACE_WRITE_BUFFER_OP      (1<<16)
#define TENOS_TRACE_SUBROUTINE_CALL      (1<<17) // 0x20000
#define TENOS_TRACE_SNOOP                (1<<18)

#define TENOS_TRACE_DEFAULT_FLAGS    (0xFFffFF)
#define TENOS_TRACE_VERBOSE          (1<<24)


#define MAXWATCH 32
struct watch_s
{
  u64_t lo;
  u64_t hi;
  int level;
};

class traceregion // A region of memory to trace.
{
 public:
  u64_t start_address;
  u64_t end_address;
  u32_t count;
  u32_t flags;
  class traceregion *next;


 traceregion(u64_t start_address, u64_t end_address, u32_t count=(u32_t)(-1), u32_t flags=TENOS_TRACE_DEFAULT_FLAGS): // parameterised constructor
    start_address(start_address),
    end_address(end_address),
    count(count),
    flags(flags)
    {
      next = 0;
    };

  traceregion() // maximal constructor
    {
      next = 0;
      start_address = 0;
      end_address = (u64_t)(-1);
      count = (u32_t) (-1);
      flags = TENOS_TRACE_DEFAULT_FLAGS;
    }
      
  traceregion *clone(traceregion *end)
  {
    traceregion *r = new traceregion(*this);
    if (r->next) r->next = r->next->clone(end);
    else r->next = end;
    return r;
  }

  void add(traceregion * &ptr) // Insert a clone of myself in linked list
  {
    ptr = clone(ptr);
  }

  u32_t check1(u64_t testv, u32_t search_mask)
  {
    if ((testv == 0) || (testv >= start_address && testv < end_address && count > 0U)
      && ((search_mask==0)||(flags & search_mask)))
      {
	count -= 1;
	//printf("traceregion yes for %lx %lx %lx\n", testv, start_address, end_address);
	return (flags) ? flags : TENOS_TRACE_DEFAULT_FLAGS;
      }
    return 0;
  }

  void print(FILE *fd)
  {
    fprintf(fd, " traceregion count=%i lo=" PFX64 " hi=" PFX64 " flags = 0x%x\n", count, start_address, end_address, flags);
  }

  u32_t check(u64_t testv, u32_t search_mask=0)
  {
    u32_t r = 0;
    // Flags field is disregarded if search_mask is left at zero.
    // For disjunction of all active flags - some regions will be more heavily watched than others.
    //int howmany = 0;
    for (traceregion *ptr = this; ptr; ptr=ptr->next)
      {
	r |= ptr->check1(testv, search_mask);
	//printf("%i  : R is now %x sm=%x\n", howmany++, r, search_mask);
      }
    if (search_mask) return (r & search_mask);
    return r;
  }
};


struct tenos_diagnostic_if; // fwd reference


struct tenos_diagnostics_s
{
  std::vector<tenos_diagnostic_if *> m_vec;
  void checkpoint(int severity, const char *where, const char *msg);
};

extern struct tenos_diagnostics_s tenos_diagnostics;

struct tenos_diagnostic_if // Nearly a pure interface 
{
  virtual void diagnostic_report(FILE *fd, int severity, const char *msg);
  tenos_diagnostic_if(const char *name) // Constructor
  {
    tenos_diagnostics.m_vec.push_back(this);
  }
};


#define UNCACHED_ADDRESS_SPACE32(X)  ((X) >= 0xE0000000) // TODO we need to put this in the generic payload in the same way an MMU makes this distinction ...
#define UNCACHED_ADDRESS_SPACE64(X)  ((X) >= 0xE0000000)


extern void tenos_report_items_report(const char *msg, FILE *fd);

class tenos_report_item
{
  class snapshot_s
  {
  public:
    union snapshot_data
    {
      const char *string_value;
      int int_value;
      u32_t u32_value;
      u64_t u64_value;
      float float_value;
    } v;
    int snapshot_number;
    snapshot_s * previous_snapshots;
  };

  snapshot_s * snapshots;
  snapshot_s * find_snapshot(int shot_number);

 public:
  const char *name1, *name2;

  void take_snapshot(int no);
  void print_csv_title(FILE *fd);
  void print_value(FILE *fd, bool humanForm, int shot_number=-1, int relative_shot_number=-1);


  // Data values - only one is active.
  const char *string_value;
  int *int_value;
  u64_t *u64_value;
  float *float_value;
  std::atomic<u32_t> *u32_avalue;
  std::atomic<u64_t> *u64_avalue;
  tenos_report_item *next;  
  // constructor 1/3
  tenos_report_item(const char *name1, const char *name2,  const char *string_value);
  // constructor 2/4
  tenos_report_item(const char *name1, const char *name2,  float *float_value);
  // constructor 3/4
  tenos_report_item(const char *name1, const char *name2,  int *int_value);
  // constructor 4/4
  tenos_report_item(const char *name1, const char *name2,  u64_t *u64_value);
  // constructor 5/6
  tenos_report_item(const char *name1, const char *name2,  std::atomic<u32_t > *u32_value);
  // constructor 6/6
  tenos_report_item(const char *name1, const char *name2,  std::atomic<u64_t > *u64_value);

};

extern void tenos_check_correct_magic_no(int w, int magic_no, bool stop_on_error);
extern void install_expected_magic_no(int n, const char *name);
extern u32_t tenos_simulator_cmd(const char *msg, int cmd, int arg0, int arg1, u32_t *rp);


#define TENOS_CMD_SIM_DONE 10
#define TENOS_CMD_WRCH     11
#define TENOS_CMD_TRACE100 12
//efine TENOS_CMD_DUMP_STATS 13 // Old API - please use STATS_CHECKPOINT now.
#define TENOS_CMD_COREPAUSE 14  // This is a yield not a halt, e.g. for use in spinlock body.
#define TENOS_CMD_CORE_HALT 15  // This is a halt, called a pause in some parts of the code.

#define TENOS_CMD_STATS_CHECKPOINT 16  // 

#define TENOS_STATS_CHECKPOINT_FLAG_PRINT_CSV               1
#define TENOS_STATS_CHECKPOINT_FLAG_PRINT_HUMAN             2
#define TENOS_STATS_CHECKPOINT_FLAG_PRINT_TAKE_SNAPSHOT     4


//! Definition of GDB target signals.
//! Data taken from the GDB 6.8 source. Only those we use defined here.
enum TargetSignal {
  TARGET_SIGNAL_NONE = 0,
  TARGET_SIGNAL_INT = 2,
  TARGET_SIGNAL_ILL = 4,
  TARGET_SIGNAL_TRAP = 5,
  TARGET_SIGNAL_FPE = 8,
  TARGET_SIGNAL_BUS = 10,
  TARGET_SIGNAL_SEGV = 11,
  TARGET_SIGNAL_ALRM = 14,
  TARGET_SIGNAL_USR2 = 31,
  TARGET_SIGNAL_PWR = 32
};

#define TRACECOMM 0
#define THREAD_COMM 0
#define TEMP_PT_BYPASS 1

#endif
