/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

#ifndef SPEEDO_REG_MAPPING_H
#define SPEEDO_REG_MAPPING_H

//
// spEEDO is/was a project funded by the UK TSB to define a strawman debug and register API so that energy accounting information
// can be distributed throughout a SoC and its embedded operating system.
//



#define SOCDAM_SPEEDO_REGS_BASE_32   0xFFFD4000 /* on 32 bit systems */
#define SOCDAM_SPEEDO_REGS_BASE_64   0xFFFFffffFFFD4000uLL /* on 64 bit systems */

#ifdef M32
#define SOCDAM_SPEEDO_REGS_BASE SOCDAM_SPEEDO_REGS_BASE_32
#else
#define SOCDAM_SPEEDO_REGS_BASE SOCDAM_SPEEDO_REGS_BASE_64
#endif

// When compiling binaries for a given machine, define BIGE_ONE as either 0 or 1 for little and big endian respectively.
#ifndef BIGE_IONE
#define BIGE_IONE 0
#endif


#define WRITE_SPEEDO(X,D)    (((volatile unsigned int  *)(SOCDAM_SPEEDO_REGS_BASE + X))[BIGE_IONE]) = D
#define READ_SPEEDO(X)       (((volatile unsigned int  *)(SOCDAM_SPEEDO_REGS_BASE + X))[BIGE_IONE])
#define READ_SPEEDO_CTX1(X)  (READ_SPEEDO(SPEEDO_REG_CTX1_BASE + (X)))

#define SPEEDO_CHKPT(X)      WRITE_SPEEDO(SPEEDO_REG_CTX_CTRL, 1) // Write a simple unit value to do a basic checkpoint checkpoint, copying live CTX0 to CTX1.

// Write with bit 31 set to change active context.
// A write without bit 31 does the more simple copy between hardwired contexts.
// In other words, a write of any other value causes a snapshot of the active context to be written to CTX1
// for software differentiation.
#define SPEEDO_SET_HW_CTX(N) WRITE_SPEEDO(SPEEDO_REG_CTX_CTRL, (1U<<31) |((N)<<8)) 

// Typical hardware register to implement the spEEDO hardware API - unbanked version.

#define SPEEDO_REG_MONICA            0   // Contains an identifying constant
#define SPEEDO_REG_ABI               8   // Version number of the interface
#define SPEEDO_REG_ENERGY_UNITS      16  // This is the fraction of a Joule in the energy registers. Constant.
#define SPEEDO_REG_CMD_STATUS        40  // Capability description and commands for resetting totals and so on and also the current hardware context. 


#define SPEEDO_REG_TIME_UNITS        56  // Units in femtoseconds for ticks in the time register.
#define SPEEDO_REG_CORE_ID           64  // Core / Customer Number Identifier
#define SPEEDO_REG_CTX_CTRL          72  // Low 8 bits is number of h/w contexts (ro), bits 15-8 are current active context (r/w).
                                         // The active context is the one being updated in h/w. The remaining contexts are passive.

#define SPEEDO_REG_CTX0_BASE     512
#define SPEEDO_REG_CTX_SPACING   256
#define SPEEDO_REG_CTX1_BASE     (512+SPEEDO_REG_CTX_SPACING)

#define SPEEDO_REFLECTION_URL0   1024 // First location of a canned URL giving further information on this ABI 



// There may be just one useable context per CPU core on in a given h/w implementation but it will be
// visible in active and shadow forms: The active CTX defaults to CTX0 and CTX1 is the shadow.

// In simple operation, a value with bit 31 set is written to to the CTX_CTRL register and this
// copies the active CTX, CTX0, to CTX1.

// If a value with bit 31 set is written, it changes the active context to the one specified in bits 15-8 which
// must be less than or equal to the context count given by reading bits 7-0.



// 
// Each hardware context contains:
#define SPEEDO_CTX_REG_LOCAL_ENERGY  8  // Running local energy in the units given
#define SPEEDO_CTX_REG_LOCAL_TIME    16 // Running local time (if implemented) for the context in the time units given
#define SPEEDO_CTX_REG_GLOBAL_ENERGY 24  // Running total energy in the units given - includes local energy




// Interrupts?
// Do we want an interrupt watchpoint on these registers - e.g. interrupt every so many microjoules used?




// Sometimes the system has a separate resource for timer ticks, needed  so that energy reports can be 
// converted to power.  A hardware abstraction layer or device driver for this register set will integrate the time factor
// and provide the PET account abstraction.


// spEEDO monica is the following data (or low 32 bits of on a D32 machine).
//      data = ((u64_t)'s' << 0) | ((u64_t)'p' << 8) | ((u64_t)'E' << 16) | ((u64_t)'E' << 24) | ((u64_t)'D' << 32) | ((u64_t)'O' << 40);

#define SPEEDO_API_ALPHA1   1002  // This value is returned by the ABI register in the prototype implementation.

// The values returned in the energy registers need to be divided by the value in the units register to give a figure in Joules.
#define SPEEDO_UNIT_pJ      (1000*1000*1000*1000)  //
#define SPEEDO_UNIT_nJ      (1000*1000*1000)
#define SPEEDO_UNIT_uJ      (1000*1000)
#define SPEEDO_UNIT_mJ      (1000)
#define SPEEDO_UNIT_UNCAL   0 // The UNCAL value shall be returned if the calibration of the energy registers is unknown.




#if 0

// The API uses the concept of a customer number.  This is an opaque handle that is actually an integer with various bit fields used for custom purposes on a specific platform. We have standard accessor functions to access the bit fields. These generally include:
// \item an ip_block_number that is typically a CPU core number but may be used for other purposes.
// \item a virtual context number that is used for threads are processes running on a core or across cores. 

typedef unsigned int customer_t; // Value zero is reserved to denote the system global total.


extern customer_t get_local_customer_no(); 
extern int get_context_field(customer_t c); 
extern int get_core_field(customer_t c); 

int get_local_core_no()    { return get_core_field(get_local_customer_no()); }
int get_local_context_no() { return get_context_field(get_local_customer_no()); }

// C language 32-bit API - bare metal/single threaded for local core.

 extern u32_t get_units();
 extern u32_t get_local_energy(); // same as get_customer_energy(get_local_core_no());
 extern u32_t get_customer_energy(customer_t customer_no);
 extern u32_t get_global_energy();
 extern const char *get_reflection_uri();
 extern int reset_energy_counters(u32_t mask); // Returns 0 if ok. Returns -ve error code if a selected register cannot be reset.

extern float report_average_power(customer_t no, int window_milliseconds) ... // TBD some running average indication 


// C language 32-bit API - multitasking extensions


 extern int set_current_customer(int core_no, int context_no);

// Depending on the hardware implementation, an access-denied type of error may be returned if the core_no is not the local core.
// This will set the current virtual context number for the specified core.  The underlying hardware may support multiple contexts and so no context swap is needed. Or else the hardware abstraction layer will replace the current settings with new settings.  Having a minimum of two hardware contexts is helpful to enable an atomic swap from one set to the other with no energy potentially lost between reading and writing an active register.



#endif

// Return codes for C-API
#define SPEEDO_RC_NO_POWER_MONITOR -199
#define SPEEDO_RC_INVALID_REG -99


#endif
