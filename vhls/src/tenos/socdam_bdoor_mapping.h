/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

#ifndef SOCDAM_H
#define SOCDAM_H

#include "tenos_cmds.h"


#define SOCDAM_BDREGS_MASK_64  0xFFFFFFFFFFFF0000LLU
#define SOCDAM_BDREGS_BASE_64  0xFFFFFFFFFFFF0000LLU


#define SOCDAM_BDREGS_MASK_32    0xFFFF0000U
#define SOCDAM_BDREGS_BASE_32    0xFFFF0000U





#define SOCDAM_ARGV_BASE_64_LE 0xFFFFffffFFFE0100LLu  // Read argv (64 bit version) from simulator command line from here
#define SOCDAM_ARGV_BASE_64_BE 0xFFFFffffFFFE8100LLu  
#define SOCDAM_REGS_BASE_64    0xFFFFffffFFFFFF00LLu

#define SOCDAM_ARGV_BASE_32_LE 0xFFFE0000u  // Read argv (32 bit version) from simulator command line from here
#define SOCDAM_ARGV_BASE_32_BE 0xFFFE8000u  
#define SOCDAM_REGS_BASE_32    0xFFFFFF00u

#define SOCDAM_STDIN_BASE_64_LE 0xFFFFffffFFFD0100LLu
#define SOCDAM_STDIN_BASE_64_BE 0xFFFFffffFFFD8100LLu
#define SOCDAM_STDIN_BASE_32_LE 0xFFFD0300u
#define SOCDAM_STDIN_BASE_32_BE 0xFFFD8300u


#ifdef M32
#define SOCDAM_REGS_BASE    SOCDAM_REGS_BASE_32    
#else
#define SOCDAM_REGS_BASE    SOCDAM_REGS_BASE_64    
#endif

// Do not put endian-specific information in this file since it will be included by heterogenous cores in the same simulator compilation.
//
// When compiling binaries for a given machine, define BIGE_ONE as either [0] or [1] for little and big endian respectively.


#define PID_REG_A        0x10 // Read this address to find CPU core number

#define CMDLINE_ARGC     0x30 // Read argc cmd line arg count.

#define TICKS_REG_A      0x40 // Timer ticks 

#define TRACE_REG_A      0x50 // Tracing flag control

#define PID_REG_B        0x60 // 

#define PID_REG_C        0x70 // 

#define SOCDAM_SYSCONF_CALL 0x80 // Sysconf - read number of cores etc.


// Write to this control location to implement simulator commands of various forms (an alternative to overloading swi's and nops)
#define SOCDAM_CORE_CMD    0x90 // Core finished and so on.

#define SOCDAM_ATOMIC_PREFIX   0xA0 // Write here as a 'lock' prefix on OpenRISC (and perhaps some others) to implement an atomic exchange sequence.


// This or all cores finished.
#define SOCDAM_KILL_SIM(X)          ((((volatile int *)(SOCDAM_REGS_BASE_32 + SOCDAM_CORE_CMD))[BIGE_IONE]) = TENOS_CMD_SIM_DONE)

#define SOCDAM_DUMP_STATS(X)        ((((volatile int *)(SOCDAM_REGS_BASE_32 + SOCDAM_CORE_CMD))[BIGE_IONE]) = TENOS_CMD_DUMP_STATS)

#define SOCDAM_ATOMIC_PREFIX_32(X_) ((((volatile int *)(SOCDAM_REGS_BASE_32 + SOCDAM_ATOMIC_PREFIX))[BIGE_IONE]) = 1)

#define SOCDAM_READ_ARGC(X)     (((unsigned int  *)(SOCDAM_REGS_BASE + CMDLINE_ARGC))[BIGE_IONE]) // Add on 1 owing to big-endian machine.

#define SOCDAM_READ_TICKS(X)    (((unsigned int  *)(SOCDAM_REGS_BASE + TICKS_REG_A))[BIGE_IONE]) // Add on 1 owing to big-endian machine.

#define SOCDAM_READ_PID_REG(X)  (((unsigned int  *)(SOCDAM_REGS_BASE + PID_REG_A))[BIGE_IONE]) // Add on 1 owing to big-endian machine.



// Processor cores above one start execution with this...
extern int _extra_cores();
extern int kprintf(const char *, ...);

//#ifdef OR1K
//#define assert(X) do { if (!(X)) _assert(X, __FILE__, __LINE__); } while (0)
//#endif


#endif
// eof
