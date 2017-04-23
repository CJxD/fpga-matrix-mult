//
// ARM Generic Interrupter in SystemC
// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2014 DJ Greaves - University of Cambridge Computer Laboratory.
// (C) 2014 DJ Greaves and M Puzovic.
//
//
#ifndef GIC_ARM_TLM
#define GIC_ARM_TLM

/*
ARM generic interrupt controller.
=================================


The nominal base address for Zynq use is f8f01000 (but this is set outside the GIC by its busmux address decoding).

It does not (initially) support the security or hypervisor extensions.
Whether it needs to do interrupt grouping is unclear but probably it
will.



*/

// We will support up to this number of cores by default. The should be no overhead associated with unused core ports.
#define GIC_CLIENT_CORES 32

#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "prazor.h"
#include "tenos.h"
#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/multi_passthrough_target_socket.h"


// On the Zynq, the two register regions are typically, with the following offsets from $f8f0_0000

#define GIC_MAX_N_CPUS                     32
#define GIC_PPI_START_BIT 27
#define GIC_DISTRIBUTOR_BASE_OFFSET    0x1000
#define GIC_CPU_FILE_OFFSET     0x100
#define GIC_CPU_FILE_SPACING    0x100

// From device tree:	reg = <0xf8f01000 0x1000>,   <0xf8f00100 0x0100>;
#define GIC_DISTRIBUTOR_SPACING 0x1000

/*
 *  The following register defines are copied from the GNU linux kernel: include/linux/irqchip/arm-gic.h
 *
 *  Copyright (C) 2002 ARM Limited, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#define GIC_CPU_CTRL			0x00  // Arm and Linux documentation differs on register names - we define both.
#define     GICC_CTLR			0x00

#define GIC_CPU_PRIMASK			0x04
#define     GICC_PMR			0x04

#define GIC_CPU_BINPOINT		0x08
#define     GICC_BPR 		        0x08

#define GIC_CPU_INTACK			0x0c
#define     GICC_IAR			0x0c

#define GIC_CPU_EOI			0x10
#define     GICC_EOIR			0x10

#define GIC_CPU_RUNNINGPRI		0x14
#define     GICC_RPR		        0x14

#define GIC_CPU_HIGHPRI			0x18

#define GIC_CPU_ALIAS_BINPOINT		0x1c
#define     GICC_ABPR  		        0x1c
#define     GICC_AIAR                   0x20
#define     GICC_AEOIR                  0x24
#define     GICC_AHPPIR                 0x28

#define GIC_CPU_ACTIVEPRIO		0xd0 // 4 registers d0 d4 d8 dc.

#define GIC_CPU_IDENT			0xfc


// Distributor API
#define GIC_DIST_CTRL			0x000 // Arm and Linux documentation differs on register names - we define both.
#define     GICD_CTLR			0x000
#define GIC_DIST_CTR			0x004
#define     GICD_TYPER			0x004
#define     GICD_IIDR			0x008
#define GIC_DIST_IGROUP			0x080
#define GIC_DIST_ENABLE_SET		0x100
#define GIC_DIST_ENABLE_CLEAR		0x180
#define GIC_DIST_PENDING_SET		0x200
#define GIC_DIST_PENDING_CLEAR		0x280
#define GIC_DIST_ACTIVE_SET		0x300
#define GIC_DIST_ACTIVE_CLEAR		0x380
#define GIC_DIST_PRI			0x400
#define     GICD_IPRIORITYR		0x400
#define GIC_DIST_TARGET			0x800
#define     GICD_ITARGETSR		0x800
#define GIC_DIST_CONFIG			0xc00

#define GIC_DIST_SOFTINT		0xf00 // Write Only
#define     GICD_SGIR   		0xf00 // Write Only
#define GIC_DIST_SGI_PENDING_CLEAR	0xf10
#define GIC_DIST_SGI_PENDING_SET	0xf20

#define GICH_HCR			0x0
#define GICH_VTR			0x4
#define GICH_VMCR			0x8
#define GICH_MISR			0x10
#define GICH_EISR0 			0x20
#define GICH_EISR1 			0x24
#define GICH_ELRSR0 			0x30
#define GICH_ELRSR1 			0x34
#define GICH_APR			0xf0
#define GICH_LR0			0x100

#define GICH_HCR_EN			(1 << 0)
#define GICH_HCR_UIE			(1 << 1)

#define GICH_LR_VIRTUALID		(0x3ff << 0)
#define GICH_LR_PHYSID_CPUID_SHIFT	(10)
#define GICH_LR_PHYSID_CPUID		(7 << GICH_LR_PHYSID_CPUID_SHIFT)
#define GICH_LR_STATE			(3 << 28)
#define GICH_LR_PENDING_BIT		(1 << 28)
#define GICH_LR_ACTIVE_BIT		(1 << 29)
#define GICH_LR_EOI			(1 << 19)

#define GICH_VMCR_CTRL_SHIFT		0
#define GICH_VMCR_CTRL_MASK		(0x21f << GICH_VMCR_CTRL_SHIFT)
#define GICH_VMCR_PRIMASK_SHIFT		27
#define GICH_VMCR_PRIMASK_MASK		(0x1f << GICH_VMCR_PRIMASK_SHIFT)
#define GICH_VMCR_BINPOINT_SHIFT	21
#define GICH_VMCR_BINPOINT_MASK		(0x7 << GICH_VMCR_BINPOINT_SHIFT)
#define GICH_VMCR_ALIAS_BINPOINT_SHIFT	18
#define GICH_VMCR_ALIAS_BINPOINT_MASK	(0x7 << GICH_VMCR_ALIAS_BINPOINT_SHIFT)

#define GICH_MISR_EOI			(1 << 0)
#define GICH_MISR_U			(1 << 1)





class gic_arm_tlm : public sc_module
#ifdef TLM_POWER3
, public pw_module
#endif
{
  SC_HAS_PROCESS(gic_arm_tlm);
  POWER3(PW_TLM3(sc_pwr::tlm_bit_transition_tracker read_bus_tracker)); // Used if counting wire transitions. 
  traceregion *traceregions;
  sc_time latency;
  const char *kind();

  // blocking transport
  void b_access(int id, PRAZOR_GP_T &trans, sc_time &delay);


#define GIC_INTERRUPT_LIMIT 1024
  struct diststate_s
  {
    bool EnableGrp0;
    bool EnableGrp1;

    u32_t swi_pending[4];
    u32_t unmasked[GIC_INTERRUPT_LIMIT/32];      
    u32_t igroup[GIC_INTERRUPT_LIMIT/32];  
    u32_t actives[GIC_INTERRUPT_LIMIT/32];  
    u32_t enables[GIC_INTERRUPT_LIMIT/32];  
    u32_t pendings[GIC_INTERRUPT_LIMIT/32];  

    u32_t priorities[GIC_INTERRUPT_LIMIT/4];  
    u32_t targets[GIC_INTERRUPT_LIMIT/4];  
    u32_t configs[GIC_INTERRUPT_LIMIT/16];  
  
    
  } diststate;

  struct per_cpu_file_base
  {
    u32_t CTL;
    u32_t PM;     // Priority mask

    struct alias_grp_s
    {
      u32_t BP; // binary point
      u32_t IA; // pending no
      u32_t RP; // Running priority
    } grps[2];

    u32_t unmasked;
    u32_t igroup;
    u32_t actives;
    u32_t enables;
    u32_t pendings;

    u32_t priorities[4];  
    u32_t targets[4];  
    u32_t configs[2];  

    u32_t sgi_matrix[4];  // Each CPU has 16 bytes that each contain a bit mask of which cores are interrupting it 

  } cpustate[GIC_MAX_N_CPUS];


  sc_event inputs_have_changed;
  void distribute();

  bool IsGrp0Int(int intID)
  {
    assert(0);
    return 0;
  }

  inline u32_t Bits(u32_t a, const unsigned bitno, const unsigned width)
  {
    assert(width > 0);
    assert(bitno + width <= 32);
    
    if (bitno > 0) a >>= bitno;
    if (width < 32) a &= (1 << width) - 1;
    return a;
  }

  inline u32_t Bit(u32_t a, const unsigned bitno)
  {
    if (bitno > 0) a >>= bitno;
    return a & 1;
  }

  u32_t *cpu_addresser(unsigned int reg, int cpuID);
  u32_t read_requesting_disjunction(int sgi, int cpuID);
  u32_t *dist_addresser(u32_t reg, u32_t cpuID);
  u32_t ReadDist(u32_t, int cpuID = -1);

  u32_t ReadDistBit(u32_t reg, u32_t bit)
  {
    return (ReadDist(reg) >> bit) & 1;
  }

  void WriteDist(u32_t a, u32_t d, int cpuID=-1);

  u32_t ReadCPU(u32_t reg, int cpuID);
  u32_t ReadCPUBit(u32_t reg, int cpuID, int bit)
  {
    return (ReadCPU(reg, cpuID) >> bit) & 1;
  }

  void WriteCPU(u32_t reg, int cpuID, u32_t d);


  //Register names and descriptions  Copyright  2008, 2011, 2013 ARM
  // Helper functions
  // ================
  void SignalFIQ(bool next_fiq, int cpuID); // Signals an interrupt on the FIQ input to the
                                               // processor, according to the value of next_fiq.

  void SignalIRQ(bool next_irq, int cpuID); // Signals an interrupt on the IRQ input to the
                                               // processor, according to the value of next_irq.

  bool IsGrp0Int(int InterruptID, int cpu_id);
  // Returns TRUE if the field in the GICD_IGROUPRn register associated with the argument InterruptID
  //		      is set to 0, indicating that the interrupt is
  //configured as a Group 0 interrupt.

  bool IsEnabled(int InterruptID, int cpu_id);
		      // Returns TRUE if the interrupt specified by the
		      // argument InterruptID is enabled in the associated
		      // GICD_ISENABLERn or GICD_ICENABLERn register.

  u3_t SGI_CpuID(int InterruptID, int cpu_id);
  // Returns the ID of a source CPU for a pending interrupt
  // with the given interruptID targeting the current
  // 				       CPU. If there are multiple source CPUs, the one
  // chosen is IMPLEMENTATION SPECIFIC.

  u8_t ReadGICD_ITARGETSR(int InterruptID, int cpu_id);
				       // Returns an 8-bit field specifying which CPUs should
				       // receive the interrupt specified by argument InterruptID

  bool AnyActiveInterrupts(int cpu_id); // Returns TRUE if any interrupts are active on this
				       // processor.
				       

  u8_t ReadGICD_IPRIORITYR(int InterruptID, int cpu_id);
				       // Returns the 8-bit priority field from the
				       // GICD_IPRIORITYR associated with the argument InterruptID.

  void WriteGICD_IPRIORITYR(int InterruptID, int cpu_id, u8_t Value);
  // Updates the priority field in the GICD_IPRIORITYR
  // associated with the argument InterruptID with the 8-bit Value.

  void IgnoreWriteRequest();
  // Ignore the register write request (no operation).
  
  void AcknowledgeInterrupt(int grp, int InterruptID, int cpu_id);
    // Set the active state and attempt to clear the pending
    // state for the interrupt associated with the argument
    // InterruptID
  
  // PriorityIsHigher()
  // ==================
  bool PriorityIsHigher(int pr1, int pr2)
  {
    return (unsigned)(pr1) < (unsigned)(pr2);
  }


  // global constants
  // ================
#define MINIMUM_BINARY_POINT 2 // A minimum binary point value of 0,1,2 or 3,
    // this is an IMPLEMENTATION DEFINED value.
    // NOTE: min. value is the SECURE value where supported

#define IGNORE_GROUP_ENABLE 0 // IMPLEMENTATION DEFINED boolean that determines whether the
    // highest priority pending interrupt is masked by the distributor
    // enable BEFORE or AFTER prioritisation:
    //                    BEFORE prioritisation                  Value = FALSE
    //                    AFTER prioritisation                   Value = TRUE

#define GICC_MASK_HPPIR 0 // IMPLEMENTATION DEFINED boolean that determines whether a read
    // of GICC_HPPIR returns a spurious interrupt for pending
    // interrupts disabled by GICC_CTLR.EnableGrp{0,1}} == '0'


#define  P_MASK 0xF0 // IMPLEMENTATION DEFINED mask of valid priority bits:
    //    Consists of an 8-bit field where the top N bits are set to 1,
    //    where N is the number of priority bits implemented.
    //    For systems without the Security Extensions, supported values are 0xF0, 0xF8, 0xFC, 0xFE and 0xFF.
    //    For systems with the Security Extensions, supported values are 0xF8, 0xFC, 0xFE and 0xFF.


  bool IsPending(int interruptID,  int cpu_id);
  u8_t GIC_PriorityMask(int n, bool NS_mask); // Calculate the Binary Point (group) mask.
  int HighestPriorityPendingInterrupt(int cpu_id);


  void GIC_GenerateExceptions_with_security(bool systemFIQ, bool systemIRQ);
  int /*(boolean, boolean)*/ UpdateExceptionState_with_security(int cpu_id);


  int /*(boolean, boolean)*/ UpdateExceptionState_sans_grouping(int cpu_id);
  void GIC_GenerateExceptions_sans_grouping();
  void new_spi_word32(u32_t base, u32_t data);

  u32_t notify_int_acknowledge(int grp, int cpuID);
  void notify_end_of_int(int grp, int cpuID, u32_t wdata);
  void clear_sgi(u32_t no, int cpuID, int src_cpu);
  void spi_word32(), spi_word64(), spi_word96();
  void write_gic_sgir(u32_t d, int cpuID);  
  void sgi_set_nclear(bool setflag, int n, u32_t d, int cpuID);
 public:

  //TLM-2 socket, defaults to 32-bits wide, power protocol
  //tlm_utils::simple_target_socket<uart64_cbg, 64, PW_TLM_TYPES> port0;
  tlm_utils::multi_passthrough_target_socket<gic_arm_tlm, 64, PW_TLM_TYPES> cpu_target0;


  // constructor
  gic_arm_tlm(sc_module_name name, int n_cpus);



  sc_in<bool> int_spi32, int_spi33, int_spi34, int_spi35, int_spi36, int_spi37, int_spi38, int_spi39;
  sc_in<bool> int_spi40, int_spi41, int_spi42, int_spi43, int_spi44, int_spi45, int_spi46, int_spi47, int_spi48, int_spi49;
  sc_in<bool> int_spi50, int_spi51, int_spi52, int_spi53, int_spi54, int_spi55, int_spi56, int_spi57, int_spi58, int_spi59;
  sc_in<bool> int_spi60, int_spi61, int_spi62, int_spi63, int_spi64, int_spi65, int_spi66, int_spi67, int_spi68, int_spi69;
  sc_in<bool> int_spi70, int_spi71, int_spi72, int_spi73, int_spi74, int_spi75, int_spi76, int_spi77, int_spi78, int_spi79;
  sc_in<bool> int_spi80, int_spi81, int_spi82, int_spi83, int_spi84, int_spi85, int_spi86, int_spi87, int_spi88, int_spi89;
  sc_in<bool> int_spi90, int_spi91, int_spi92, int_spi93, int_spi94, int_spi95, int_spi96, int_spi97, int_spi98, int_spi99;
  sc_in<bool> int_spi100, int_spi101, int_spi102, int_spi103, int_spi104, int_spi105, int_spi106, int_spi107, int_spi108, int_spi109;
  sc_in<bool> int_spi110, int_spi111, int_spi112, int_spi113, int_spi114, int_spi115, int_spi116, int_spi117, int_spi118, int_spi119;
  sc_in<bool> int_spi120, int_spi121, int_spi122, int_spi123, int_spi124, int_spi125, int_spi126, int_spi127;



  // Interrupt outputs to the separate CPU cores.
  sc_out<bool> IRQ_out[GIC_MAX_N_CPUS];
  sc_out<bool> FIQ_out[GIC_MAX_N_CPUS];

  sc_in<bool> ppi0[GIC_MAX_N_CPUS], ppi1[GIC_MAX_N_CPUS], ppi2[GIC_MAX_N_CPUS], ppi3[GIC_MAX_N_CPUS], ppi4[GIC_MAX_N_CPUS];

  void ppi_changed();
  int n_cpus;

};







#endif

