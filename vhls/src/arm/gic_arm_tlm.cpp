//
// Arm Generic Interrupter in SystemC
// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2014 DJ Greaves - University of Cambridge Computer Laboratory.

#include "gic_arm_tlm.h"
#include "tenos.h"

#define GIC_TRC(X)

gic_arm_tlm::gic_arm_tlm(sc_module_name name, int n_cpus) :   // constructor
    sc_module(name),
#ifdef TLM_POWER3
    pw_module(),
#if PW_TLM_PAYLOAD > 0
    read_bus_tracker(this),
#endif
#endif
  n_cpus(n_cpus),
  int_spi32("int_spi32"), int_spi33("int_spi33"),
  int_spi34("int_spi34"), int_spi35("int_spi35"),
  int_spi36("int_spi36"), int_spi37("int_spi37"),
  int_spi38("int_spi38"), int_spi39("int_spi39"),
  int_spi40("int_spi40"), int_spi41("int_spi41"),
  int_spi42("int_spi42"), int_spi43("int_spi43"),
  int_spi44("int_spi44"), int_spi45("int_spi45"),
  int_spi46("int_spi46"), int_spi47("int_spi47"),
  int_spi48("int_spi48"), int_spi49("int_spi49"),
  int_spi50("int_spi50"), int_spi51("int_spi51"),
  int_spi52("int_spi52"), int_spi53("int_spi53"),
  int_spi54("int_spi54"), int_spi55("int_spi55"),
  int_spi56("int_spi56"), int_spi57("int_spi57"),
  int_spi58("int_spi58"), int_spi59("int_spi59"),
  int_spi60("int_spi60"), int_spi61("int_spi61"),
  int_spi62("int_spi62"), int_spi63("int_spi63"),
  int_spi64("int_spi64"), int_spi65("int_spi65"),
  int_spi66("int_spi66"), int_spi67("int_spi67"),
  int_spi68("int_spi68"), int_spi69("int_spi69"),
  int_spi70("int_spi70"), int_spi71("int_spi71"),
  int_spi72("int_spi72"), int_spi73("int_spi73"),
  int_spi74("int_spi74"), int_spi75("int_spi75"),
  int_spi76("int_spi76"), int_spi77("int_spi77"),
  int_spi78("int_spi78"), int_spi79("int_spi79"),
  int_spi80("int_spi80"), int_spi81("int_spi81"),
  int_spi82("int_spi82"), int_spi83("int_spi83"),
  int_spi84("int_spi84"), int_spi85("int_spi85"),
  int_spi86("int_spi86"), int_spi87("int_spi87"),
  int_spi88("int_spi88"), int_spi89("int_spi89"),
  int_spi90("int_spi90"), int_spi91("int_spi91"),
  int_spi92("int_spi92"), int_spi93("int_spi93"),
  int_spi94("int_spi94"), int_spi95("int_spi95"),
  int_spi96("int_spi96"), int_spi97("int_spi97"),
  int_spi98("int_spi98"), int_spi99("int_spi99"),
  int_spi100("int_spi100"), int_spi101("int_spi101"),
  int_spi102("int_spi102"), int_spi103("int_spi103"),
  int_spi104("int_spi104"), int_spi105("int_spi105"),
  int_spi106("int_spi106"), int_spi107("int_spi107"),
  int_spi108("int_spi108"), int_spi109("int_spi109"),
  int_spi110("int_spi110"), int_spi111("int_spi111"),
  int_spi112("int_spi112"), int_spi113("int_spi113"),
  int_spi114("int_spi114"), int_spi115("int_spi115"),
  int_spi116("int_spi116"), int_spi117("int_spi117"),
  int_spi118("int_spi118"), int_spi119("int_spi119"),
  int_spi120("int_spi120"), int_spi121("int_spi121"),
  int_spi122("int_spi122"), int_spi123("int_spi123"),
  int_spi124("int_spi124"), int_spi125("int_spi125"),
  int_spi126("int_spi126"), int_spi127("int_spi127")
{
  assert(n_cpus >= 1 && n_cpus <= GIC_MAX_N_CPUS);
  traceregions = 0;
  latency = sc_time(50, SC_NS);
  cpu_target0.register_b_transport(this, &gic_arm_tlm::b_access);


  SC_METHOD(ppi_changed);
  for (int n=0;n<GIC_MAX_N_CPUS; n++) sensitive << ppi0[n] << ppi1[n] << ppi2[n] << ppi3[n] << ppi4[n] ;

   SC_METHOD(spi_word32);
      sensitive<<int_spi32<<int_spi33<<int_spi34<<int_spi35<<int_spi36<<int_spi37<<int_spi38<<int_spi39<<int_spi40<<int_spi41<<int_spi42<<int_spi43<<int_spi44<<int_spi45<<int_spi46<<int_spi47<<int_spi48<<int_spi49<<int_spi50<<int_spi51<<int_spi52<<int_spi53<<int_spi54<<int_spi55<<int_spi56<<int_spi57<<int_spi58<<int_spi59<<int_spi60<<int_spi61<<int_spi62<<int_spi63;
   SC_METHOD(spi_word64);
      sensitive<<int_spi64<<int_spi65<<int_spi66<<int_spi67<<int_spi68<<int_spi69<<int_spi70<<int_spi71<<int_spi72<<int_spi73<<int_spi74<<int_spi75<<int_spi76<<int_spi77<<int_spi78<<int_spi79<<int_spi80<<int_spi81<<int_spi82<<int_spi83<<int_spi84<<int_spi85<<int_spi86<<int_spi87<<int_spi88<<int_spi89<<int_spi90<<int_spi91<<int_spi92<<int_spi93<<int_spi94<<int_spi95;
   SC_METHOD(spi_word96);
      sensitive<<int_spi96<<int_spi97<<int_spi98<<int_spi99<<int_spi100<<int_spi101<<int_spi102<<int_spi103<<int_spi104<<int_spi105<<int_spi106<<int_spi107<<int_spi108<<int_spi109<<int_spi110<<int_spi111<<int_spi112<<int_spi113<<int_spi114<<int_spi115<<int_spi116<<int_spi117<<int_spi118<<int_spi119<<int_spi120<<int_spi121<<int_spi122<<int_spi123<<int_spi124<<int_spi125<<int_spi126<<int_spi127;


      for (int i=0; i<GIC_INTERRUPT_LIMIT/4; i++)
	{
	  diststate.targets[i] =  0; // 0x01010101; // All default to CPU0 is one option - but Zynq defaults to zero.
	}

      for (int i=0; i<GIC_INTERRUPT_LIMIT/32; i++)
	{
	  diststate.enables[i] = 0; 
	  diststate.pendings[i] = 0; 
	  diststate.actives[i] = 0; 
	  diststate.unmasked[i] = 0; 
	}

      for (int n=0;n<n_cpus; n++)
	{
	  for (int xx = 0; xx < 4; xx++) cpustate[n].targets[xx] = (0x01010101 << (n & 0x7));
	  cpustate[n].CTL = 0;
	  cpustate[n].PM = 1023;
	  for (int i=0; i<4; i++)
	    {
	      cpustate[n].sgi_matrix[i] = 0; 
	    }
	  
	}
  SC_METHOD(distribute); sensitive << inputs_have_changed;

}  // end constructor


// blocking transport method
void gic_arm_tlm::b_access(int cpuID, PW_TLM_PAYTYPE &trans, sc_time &delay)
{
  tlm::tlm_command cmd = trans.get_command();

  POWER3(PW_TLM3(pw_agent_record l_agent = trans.pw_log_hop(this,  (cmd==tlm::TLM_READ_COMMAND ? PW_TGP_DATA: PW_TGP_NOFIELDS) | PW_TGP_ACCT_CKP,  &read_bus_tracker)));

  u64_t    adr = ((u64_t)trans.get_address() & 0x1ffc);
  u8_t*	   ptr = trans.get_data_ptr();
  // u32_t    len = trans.get_data_length();
  // u8_t*    lanes = trans.get_byte_enable_ptr();
  // u32_t    wid = trans.get_streaming_width();
  u32_t reg_offset = adr & 0x1FFC; // Extract low bits of address bus for internal register.

  if (cpuID >= n_cpus)
    {
      printf("%s: GIC was configured with %i cpu interfaces but is being operated on by CPU no %i\n", name(), n_cpus, cpuID);
      assert(0);
    }

  tlm::tlm_response_status rc = tlm::TLM_ADDRESS_ERROR_RESPONSE;
  if (cmd == tlm::TLM_READ_COMMAND)
    {
      u32_t r = 0x0;
      
      if (reg_offset >= GIC_DISTRIBUTOR_BASE_OFFSET)
	{
	  r = ReadDist(reg_offset - GIC_DISTRIBUTOR_BASE_OFFSET, cpuID);
	  rc = tlm::TLM_OK_RESPONSE; // for now
	}
      else
	{
#if 0
	  int cpu_file = (reg_offset - GIC_CPU_FILE_OFFSET)/GIC_CPU_FILE_SPACING;
	  if (cpu_file != cpuID)
	    {
	      printf("GIC: Wrong per-CPU register file read ? %i cf %i\n", cpuID, cpu_file);
	    }
#endif
	  assert(cpuID >= 0 && cpuID < n_cpus);
	  // can check we are reading the correct per-CPU register file
	  r = ReadCPU(reg_offset % GIC_CPU_FILE_SPACING, cpuID);
	  rc = tlm::TLM_OK_RESPONSE; // for now	  
	}

      //
      ((u64_t *)ptr)[0] = r | (((u64_t)r)<<32); // IO device uses endian-ness of hosting workstation.
      if (traceregions && traceregions->check(adr, TENOS_TRACE_IO_READ))
	printf("%s:%s: read GIC addr=" PFX64 " reg_offset=%02X data=%02X\n", name(), kind(),  adr, reg_offset, r);
      GIC_TRC(printf("%s:%s: cpu %i read GIC addr=" PFX64 " reg_offset=%02X data=%02X\n", name(), kind(),  cpuID, adr, reg_offset, r));
    }
    else if (cmd == tlm::TLM_WRITE_COMMAND)
      {
	rc = tlm::TLM_OK_RESPONSE;// Default to success - ignore writes to unimplemented registers.
	u32_t d = ((u64_t *)ptr)[0];
	u8_t* lanes = trans.get_byte_enable_ptr();
	bool hi_wordf = false;
	if (lanes)
	  {
	    if (lanes[4]) hi_wordf = true;
	
	    if (lanes[0] != lanes[1]) assert(0);
	    if (lanes[1] != lanes[2]) assert(0);
	    if (lanes[2] != lanes[3]) assert(0);

	    if (lanes[4] != lanes[5]) assert(0);
	    if (lanes[5] != lanes[6]) assert(0);
	    if (lanes[6] != lanes[7]) assert(0);

	  }
	if (hi_wordf) reg_offset += 4;

	if (traceregions && traceregions->check(adr, TENOS_TRACE_IO_WRITE))
	  printf("%s:%s: write GIC addr=" PFX64 " hi_wordf=%u reg_offset=%02X data=%02X '%c'\n", name(), kind(),  adr, hi_wordf, reg_offset, d, isprint(d) ? d: '.');


	if (reg_offset >= GIC_DISTRIBUTOR_BASE_OFFSET)
	  {
	    WriteDist(reg_offset - GIC_DISTRIBUTOR_BASE_OFFSET, d, cpuID);
	    rc = tlm::TLM_OK_RESPONSE; // for now
	  }
	else
	  {
	    int cpu_file = (reg_offset - GIC_CPU_FILE_OFFSET)/GIC_CPU_FILE_SPACING;
#if 0
	    if (cpu_file != cpuID)
	      {
		printf("GIC: Wrong per-CPU register file write ? %i cf %i\n", cpuID, cpu_file);
	      }
#endif
	    assert(cpuID >= 0 && cpuID < n_cpus);
	    // We check we are reading the correct per-CPU register file - it is allowed to read other core's files but we do not expect this behaviour generally.
	    WriteCPU(reg_offset % GIC_CPU_FILE_SPACING, cpuID, d);
	    rc = tlm::TLM_OK_RESPONSE; // for now	  
	  }
	
      }
  
  delay += latency;
  
  
  trans.set_response_status(rc);
#if PW_TLM_PAYLOAD > 0
  POWER3(l_agent.record_energy_use(pw_energy(1.0e-3 * 1.0e-6, PW_JOULE), &trans));
#else
  POWER3(record_energy_use(pw_energy(1.0e-3 * 1.0e-6, PW_JOULE), &trans));
#endif

  // Not all access cycles xfer a char however (especially when polled).
  // Very rough idea of GIC energy per character 10 microseconds * 1 milliamp.

}



// CPU bits

// Fig 4-24 - v2 without security extensions
#if 1

#define GICC_CTL_EnableGrp0    (1<<0) 
#define GICC_CTL_EnableGrp1    (1<<1) 
#define GICC_CTL_AckCtl        (1<<2) 
#define GICC_CTL_FIQEn         (1<<3)
#define GICC_CTL_CBPR          (1<<4)

#define GICC_CTL_EOImodeS      (1<<9) 
#define GICC_CTL_EOImodeNS     (1<<10) 

#else 
//
#define GICC_CTL_EnableGrp1    (1<<0) 
#define GICC_CTL_FIQBypDisGtp1 (1<<5) 
#define GICC_CTL_IRQBypDisGrp1 (1<<6) 
#endif




#define GICC_CTL_Enable     (1<<0)



//Distributor Register Bits


#define GICD_CTL_EnableGrp0     (1<<0)
#define GICD_CTL_EnableGrp1     (1<<1)
#define GICD_CTL_Enable         (1<<0)




u32_t gic_arm_tlm::ReadDist(unsigned int reg, int cpuID)
{
  u32_t rdata = 0xDEADBEEF;
  if (reg < 0x80)
    {
      switch(reg)
	{
	  
	case GICD_CTLR: // Control register
	    rdata = (diststate.EnableGrp1 << 1) | (diststate.EnableGrp1 << 0); 
	    break;
	    
	case GICD_IIDR: // Implementor:
	  {
	    u32_t variant = 0;
	    u32_t revision = 0;
	    rdata = (variant << 16) | (revision << 12) | 0x438; // ARM JEP106 code.
	  }
	  break;
	  
	case GICD_TYPER:
	  {
	    u32_t ITLinesNumber = GIC_INTERRUPT_LIMIT/32 - 1;
	    u32_t CPUNumber = n_cpus-1;
	    u32_t SecurityExtn = 0;
	    rdata = (SecurityExtn << 10) | (CPUNumber << 5) | ITLinesNumber;
	  }
	  break;
	  
	default:
	  printf("%s: gic read undefined distributor register 0x%x\n", name(), reg);
	  break;
	  
	}
    }
  else if ((reg >= GIC_DIST_SGI_PENDING_SET && reg < GIC_DIST_SGI_PENDING_SET + 16) ||
	   (reg >= GIC_DIST_SGI_PENDING_CLEAR && reg < GIC_DIST_SGI_PENDING_CLEAR + 16))
    {
      int n = (reg >> 2) & 3;
      if (cpuID >= 0 && cpuID < n_cpus)
	{
	  rdata = cpustate[cpuID].sgi_matrix [n];
	}
      else printf("%s: cpuID must be provided for reading SGI matrix\n", name());
    }
  
  else
    {
      u32_t *rp = dist_addresser(reg, cpuID);
      if (rp) rdata = *rp;
      else printf("%s: unsupported dist reg read reg_addr=0x%x\n", name(), reg);
    }

  return rdata;
}
      
      

u32_t *gic_arm_tlm::dist_addresser(u32_t reg, u32_t cpuID)
{
  u32_t *rp = 0;
  if (reg >= GIC_DIST_IGROUP && reg < GIC_DIST_IGROUP + 128)
    {
      int n = (reg >> 2) & (GIC_INTERRUPT_LIMIT/32 - 1);
      if (n==0)
	{
	  if (cpuID <0 || cpuID>=n_cpus) 
	    {
	      printf("%s: Banked dist reg operation on reg=0x%x requires a valid cpuID to proceed (provided=%i)\n", name(), reg, cpuID);
	    }
	  else
	    rp = &cpustate[cpuID].igroup;
	}
      else rp = &diststate.igroup[n];
    }
  else if ((reg >= GIC_DIST_ENABLE_SET && reg < GIC_DIST_ENABLE_SET + 128) ||
	   (reg >= GIC_DIST_ENABLE_CLEAR && reg < GIC_DIST_ENABLE_CLEAR + 128))
    {
      int n = (reg >> 2) & (GIC_INTERRUPT_LIMIT/32 - 1);
      if (n==0)
	{
	  if (cpuID<0 || cpuID>=n_cpus) 
	    {
	      printf("%s: Banked dist reg (enable) operation on reg=0x%x requires a valid cpuID to proceed (used=%i should be in range 0..%i)\n", name(), reg, cpuID, n_cpus-1);
	    }
	  else
	    rp = &cpustate[cpuID].enables;
	}
      else rp = &diststate.enables[n];
    }

  else if ((reg >= GIC_DIST_PENDING_SET && reg < GIC_DIST_PENDING_SET + 128) ||
	   (reg >= GIC_DIST_PENDING_CLEAR && reg < GIC_DIST_PENDING_CLEAR + 128))
   {
     int n = (reg >> 2) & (GIC_INTERRUPT_LIMIT/32 - 1);
     if (n==0)
       {
	 if (cpuID <0 || cpuID>=n_cpus) 
	   {
	     printf("%s: Banked dist reg (pending) operation on reg=0x%x requires a valid cpuID to proceed (provided=%i)\n", name(), reg, cpuID);
	   }
	 else
	   rp = &cpustate[cpuID].pendings;
       }
     else rp = &diststate.pendings[n];
    }

  else if ((reg >= GIC_DIST_ACTIVE_SET && reg < GIC_DIST_ACTIVE_SET + 128) ||
	   (reg >= GIC_DIST_ACTIVE_CLEAR && reg < GIC_DIST_ACTIVE_CLEAR + 128))
    {
      int n = (reg >> 2) & (GIC_INTERRUPT_LIMIT/32 - 1);
      if (n==0)
	{
	  if (cpuID <0 || cpuID>=n_cpus) 
	    {
	     printf("%s: Banked dist reg (active) operation on reg=0x%x requires a valid cpuID to proceed (provided=%i)\n", name(), reg, cpuID);
	    }
	  else
	    rp = &cpustate[cpuID].actives;
	}
      else rp = &diststate.actives[n];
    }

      
  else if (reg >= GIC_DIST_PRI && reg < GIC_DIST_PRI + 1024)
    {
      int n = (reg >> 2) & (GIC_INTERRUPT_LIMIT/4 - 1);
      if (n < 4)
	{
	  if (cpuID <0 || cpuID>=n_cpus) 
	    {
	      printf("%s: Banked dist reg (priority) operation on reg=0x%x requires a valid cpuID to proceed (provided=%i)\n", name(), reg, cpuID);
	    }
	  else
	    rp = &cpustate[cpuID].priorities[n];
	}
      else rp = &diststate.priorities[n];
    }

  else if (reg >= GIC_DIST_TARGET && reg < GIC_DIST_TARGET + 1024)
    {
      int n = (reg >> 2) & (GIC_INTERRUPT_LIMIT/4 - 1);
      if (n < 8) // banked per cpu (containing SWI and Privates). 
	{
	  if (cpuID <0 || cpuID>=n_cpus) 
	    {
	      printf("%s: Banked dist reg (target) operation on reg=0x%x requires a valid cpuID to proceed (provided=%i)\n", name(), reg, cpuID);

	    }
	  else
	    {
	      //printf("%s: Targets for a private interrupt should not be required!", name());
	      //This register never used in operation but ARM seems to set it up..
	      rp = &cpustate[cpuID].targets[n];
	    }
	}
      else rp = &diststate.targets[n];
    }
  else if (reg >= GIC_DIST_CONFIG && reg < GIC_DIST_CONFIG + 256)
    {
      int n = (reg >> 2) & (GIC_INTERRUPT_LIMIT/16 - 1);
      if (n < 2)
	{
	  if (cpuID <0 || cpuID>=n_cpus) 
	    {
	      printf("%s: Banked dist reg (config) operation on reg=0x%x requires a valid cpuID to proceed (provided=%i)\n", name(), reg, cpuID);
	    }
	  else
	    rp = &cpustate[cpuID].configs[n];
	}
      else rp = &diststate.configs[n];
    }

  return rp;

}


void gic_arm_tlm::WriteDist(unsigned int reg, u32_t d, int cpuID)
{
  if (reg < 0x80)
    {
      switch(reg)
	{

	case GICD_CTLR:
	  GIC_TRC(printf("%s dist set ctlr %x D=0x%x\n", name(), reg, d));
	  diststate.EnableGrp0 = (d >> 0) & 1;
	  diststate.EnableGrp1 = (d >> 1) & 1;
	  break;


	default:
	  printf("%s: gic write undefined distributor register 0x%x := %x\n", name(), reg, d);
	  break;
	  
	}
      
    }
  else if (reg >= GIC_DIST_IGROUP && reg < GIC_DIST_IGROUP + 128)
    {
      GIC_TRC(printf("%s dist set igroup %x D=0x%x\n", name(), reg, d));	
      u32_t *rp = dist_addresser(reg, cpuID);
      assert(rp); 
      *rp = d;
    }

  else if (reg >= GIC_DIST_ENABLE_SET && reg < GIC_DIST_ENABLE_SET + 128)
    {
      GIC_TRC(printf("%s dist enable interrupt %x %x, cpuID = %d\n", name(), reg, d, cpuID));	
      u32_t *rp = dist_addresser(reg, cpuID);
      assert(rp); 
      *rp |= d;
    }
  else if (reg >= GIC_DIST_ENABLE_CLEAR && reg < GIC_DIST_ENABLE_CLEAR + 128)
    {
      GIC_TRC(printf("%s dist disable interrupt %x %x\n", name(), reg, d));	
      u32_t *rp = dist_addresser(reg, cpuID);
      assert(rp); 
      *rp &= ~d;
    }
  else if (reg >= GIC_DIST_PENDING_SET && reg < GIC_DIST_PENDING_SET + 128)
    {
      GIC_TRC(printf("%s dist pending interrupt %x %x\n", name(), reg, d));	
      u32_t *rp = dist_addresser(reg, cpuID);
      assert(rp); 
      *rp |= d;
      // pending set of an SGI should be done with a special write to GICD_SGIR not by writing here...
    }
  else if (reg >= GIC_DIST_PENDING_CLEAR && reg < GIC_DIST_PENDING_CLEAR + 128)
    {
      GIC_TRC(printf("%s dist clear pending interrupt %x %x\n", name(), reg, d));	
      int nn = (reg >> 2) & (GIC_INTERRUPT_LIMIT/32 - 1);
      if (nn == 0)
	{
	  for (int i=0; i<16; i++) if (d & (1<<i)) clear_sgi(i, cpuID, -1);
	}
      u32_t *rp = dist_addresser(reg, cpuID);
      assert(rp); 
      *rp &= ~d;
    }

  else if (reg >= GIC_DIST_ACTIVE_SET && reg < GIC_DIST_ACTIVE_SET + 128)
    {
      GIC_TRC(printf("%s dist active interrupt %x %x\n", name(), reg, d));	
      u32_t *rp = dist_addresser(reg, cpuID);
      assert(rp); 
      *rp |= d;
    }
  else if (reg >= GIC_DIST_ACTIVE_CLEAR && reg < GIC_DIST_ACTIVE_CLEAR + 128)
    {
      GIC_TRC(printf("%s dist disable interrupt %x %x\n", name(), reg, d));	
      u32_t *rp = dist_addresser(reg, cpuID);
      assert(rp); 
      *rp &= ~d;
    }

  else if (reg >= GIC_DIST_PRI && reg < GIC_DIST_PRI + 1024)
    {
      //TODO byte write support is officially needed.
      GIC_TRC(printf("%s priority set %x %x\n", name(), reg, d));	
      u32_t *rp = dist_addresser(reg, cpuID);
      assert(rp); 
      *rp = d;
    }
  else if (reg >= GIC_DIST_TARGET && reg < GIC_DIST_TARGET + 1024)
    {
      //TODO byte write support is officially needed.
      GIC_TRC(printf("%s target set %x %x\n", name(), reg, d));	
      u32_t *rp = dist_addresser(reg, cpuID);
      assert(rp); 
      *rp = d;
    }
  else if (reg >= GIC_DIST_CONFIG && reg < GIC_DIST_CONFIG + 256)
    {
      //TODO byte write support is officially needed.
      GIC_TRC(printf("%s config set %x %x\n", name(), reg, d));	
      u32_t *rp = dist_addresser(reg, cpuID);
      assert(rp); 
      *rp = d;
    }

  else if (reg >= GIC_DIST_SGI_PENDING_SET && reg < GIC_DIST_SGI_PENDING_SET + 16)
    {
      int n = (reg >> 2) & 3;
      GIC_TRC(printf("%s dist SGI pending interrupt %x %x\n", name(), n, d));	
      sgi_set_nclear(true, n, d, cpuID);
    }
  else if (reg >= GIC_DIST_SGI_PENDING_CLEAR && reg < GIC_DIST_SGI_PENDING_CLEAR + 16)
    {
      int n = (reg >> 2) & 3;
      GIC_TRC(printf("%s dist SGI disable interrupt %x %x\n", name(), n, d));	
      sgi_set_nclear(false, n, d, cpuID);
    }

  else if (reg == GIC_DIST_SOFTINT) // aka GICD_SGIR
    {
      write_gic_sgir(d, cpuID);
    }


  else printf("%s: unsupported dist reg write reg_addr=0x%x data=0x%x\n", name(), reg, d);
}


u32_t gic_arm_tlm::notify_int_acknowledge(int grp, int cpuID)
    // Set the active state and attempt to clear the pending
    // state for the interrupt associated with the argument
    // InterruptID
{
  int pendID = cpustate[cpuID].grps[grp].IA;
  GIC_TRC(printf("%s: Acknowledge interrupt no=%i on core %i\n", name(), pendID, cpuID));
  // Move from pending to active state.
  int src_cpu = 0;
  int bit = 1<<(pendID %32), nn = pendID / 32;

  if (pendID <16) 
    {
      // If there are multiple SGIs pending from different src cpus we do them one at a time.
      u8_t src_bitfield = (cpustate[cpuID].sgi_matrix[pendID/4] >> (8 * (pendID %4))) & 0xFF;
      src_cpu = 0;
      for (src_cpu=0;src_cpu<n_cpus; src_cpu++)
	{
	  if (src_bitfield & 1) break;
	  src_bitfield >>= 1;
	}
      //assert(src_cpu<n_cpus); 
      clear_sgi(pendID, cpuID, src_cpu);
    }
  else if(pendID >= 16 && pendID < 32) {
      // clear bit that is set when PPI is changed
      cpustate[cpuID].unmasked &= ~bit;
  }
  else if(pendID < 1021) {
    // clear bit that is set with SPI
    diststate.unmasked[nn] &= ~bit;
  }

  u32_t *rp = dist_addresser(GIC_DIST_PENDING_CLEAR + nn * 4, cpuID);
  assert(rp); 
  *rp &= bit;

  rp = dist_addresser(GIC_DIST_ACTIVE_SET + nn * 4, cpuID);
  assert(rp); 
  *rp |= bit;

  inputs_have_changed.notify();
  return pendID | (src_cpu << 10);
}
void gic_arm_tlm::notify_end_of_int(int grp, int cpuID, u32_t wdata)
    // Clear the active state.
{
  int pendID = wdata & (GIC_INTERRUPT_LIMIT-1);
  GIC_TRC(printf("%s: Notift end of interrupt no=%i on core %i\n", name(), pendID, cpuID));
  // Move from active to inactive state
  int bit = 1<<(pendID %32), nn = pendID / 32;

  u32_t *rp = dist_addresser(GIC_DIST_ACTIVE_CLEAR + nn * 4, cpuID);
  assert(rp); 
  *rp &= ~bit;
  inputs_have_changed.notify();
}

void gic_arm_tlm::WriteCPU(u32_t reg, int cpuID, u32_t d)
{
  if (reg == GICC_EOIR) notify_end_of_int(0, cpuID, d); // End of interrupt - write only
  else if (reg == GICC_AEOIR)  notify_end_of_int(1, cpuID, d); 
  else
    {

      u32_t *rp = cpu_addresser(reg, cpuID);
      if (rp)
	{
	  *rp = d;
	}
      else
	printf("%s: write to unrecognised CPU register reg=0x%x\n", name(), reg);
    }
}

// Trigger a software-generated interrupt: an SGI.
void gic_arm_tlm:: write_gic_sgir(u32_t d, int cpuID)
{
  int no = (d >> 0) & 0xF;
  u8_t targets = (d >> 16) & 0xFF;
  u2_t filter = (d >> 24) & 0x3;

  if (cpuID < 0 || cpuID > n_cpus)
    {
      printf("%s: cpuID must be provided for SGI generation\n", name());
      return;
    }  

  // filter values:
  // 0 normal
  // 1 all but src
  // 2 self

  for (int n=0; n<n_cpus; n++)
    {
      if ((filter == 0 && ((1<<n) & targets)) ||
	  (filter == 1) ||
	  (filter == 2 && n == cpuID))
	{
	  cpustate[n].sgi_matrix[no/4] |= 1 << (cpuID + 8 * (no % 4));
	}
    }
  inputs_have_changed.notify();
}

void gic_arm_tlm::clear_sgi(u32_t no, int cpuID, int src_cpu)
{
  for (int n=0; n<n_cpus; n++) // For all dest cpus
    {
      if (src_cpu >= 0) n = src_cpu; // Just one.
      cpustate[cpuID].sgi_matrix[no/4] &= ~(1 << (src_cpu + 8 * (no % 4)));
      if (src_cpu >= 0) break;
    }
}



u32_t *gic_arm_tlm::cpu_addresser(unsigned int reg, int cpuID)
{
  u32_t *rp = 0;
  switch (reg)
    {

    case GICC_CTLR:
      rp = &cpustate[cpuID].CTL;
      break;

    case GICC_PMR: 
      rp = &cpustate[cpuID].PM; // Priority mask
      break;


      // Gourp 0 CPU Status
    case GICC_BPR:
      rp = &cpustate[cpuID].grps[0].BP; // Binary point
      break;
    case GICC_IAR:
      rp = &cpustate[cpuID].grps[0].IA; // Interrupt ack - read only
      break;
    case GICC_EOIR:  // End of interrupt - write only
      break;
    case GICC_RPR:
      rp = &cpustate[cpuID].grps[0].RP; // Running priority
      break;


      // Group 1 - the aliases
    case GICC_ABPR:
      rp = &cpustate[cpuID].grps[1].BP; // Binary point
      break;
    case GICC_AIAR:
      rp = &cpustate[cpuID].grps[1].IA;  // Interrupt ack - read only.
      break;
    case GICC_AEOIR:  // End of interrupt - write only
      break;
    case GICC_AHPPIR:
      rp = &cpustate[cpuID].grps[1].RP;  // Running priority
      break;
    }
  return rp;
}



u32_t gic_arm_tlm::ReadCPU(unsigned int reg, int cpuID)
{
  u32_t rd = 0xDEADBEEF;


  if (reg == GICC_IAR ) {
    rd = notify_int_acknowledge(0, cpuID);
  }

  else if (reg == GICC_AIAR ) rd = notify_int_acknowledge(1, cpuID);
  

  else
    {
      u32_t *rp = cpu_addresser(reg, cpuID);
      if (rp)
	{
	  rd = *rp;
	}
      else
	{
	  printf("%s: unsupported CPU register read reg=0x%x\n", name(), reg);
	}
    }
  return rd;
}

 void gic_arm_tlm::SignalIRQ(bool next_irq, int cpuID) // Signals an interrupt on the IRQ input to the
{
   IRQ_out[cpuID] = next_irq; // active high signalling
}

void gic_arm_tlm::SignalFIQ(bool next_irq, int cpuID) // Signals an interrupt on the IRQ input to the
{
   FIQ_out[cpuID] = next_irq; // active high signalling
}



void gic_arm_tlm::sgi_set_nclear(bool setflag, int n, u32_t d, int cpuID)
{
  // TODO should support byte access.
  u32_t &pendings = cpustate[cpuID].sgi_matrix [n];
  if (setflag) pendings |= d; else pendings &= ~d;
  inputs_have_changed.notify();
}


void gic_arm_tlm::new_spi_word32(u32_t base, u32_t data) // This function could double as a TLM input for interrupt requesting.
{
  diststate.unmasked[base/32] = data;
  inputs_have_changed.notify();
}



// Called when a net-level update occurs

void gic_arm_tlm::spi_word32()
{  u32_t  nd = 
    (int_spi32 << 0)|(int_spi33 << 1)|(int_spi34 << 2)|(int_spi35 << 3)|(int_spi36 << 4)|(int_spi37 << 5)|(int_spi38 << 6)|(int_spi39 << 7)|(int_spi40 << 8)|(int_spi41 << 9)|(int_spi42 << 10)|(int_spi43 << 11)|(int_spi44 << 12)|(int_spi45 << 13)|(int_spi46 << 14)|(int_spi47 << 15)|(int_spi48 << 16)|(int_spi49 << 17)|(int_spi50 << 18)|(int_spi51 << 19)|(int_spi52 << 20)|(int_spi53 << 21)|(int_spi54 << 22)|(int_spi55 << 23)|(int_spi56 << 24)|(int_spi57 << 25)|(int_spi58 << 26)|(int_spi59 << 27)|(int_spi60 << 28)|(int_spi61 << 29)|(int_spi62 << 30)|(int_spi63 << 31);
  new_spi_word32(32, nd);
}

void gic_arm_tlm::spi_word64()
{  
  u32_t  nd = 
    (int_spi64 << 0)|(int_spi65 << 1)|(int_spi66 << 2)|(int_spi67 << 3)|(int_spi68 << 4)|(int_spi69 << 5)|(int_spi70 << 6)|(int_spi71 << 7)|(int_spi72 << 8)|(int_spi73 << 9)|(int_spi74 << 10)|(int_spi75 << 11)|(int_spi76 << 12)|(int_spi77 << 13)|(int_spi78 << 14)|(int_spi79 << 15)|(int_spi80 << 16)|(int_spi81 << 17)|(int_spi82 << 18)|(int_spi83 << 19)|(int_spi84 << 20)|(int_spi85 << 21)|(int_spi86 << 22)|(int_spi87 << 23)|(int_spi88 << 24)|(int_spi89 << 25)|(int_spi90 << 26)|(int_spi91 << 27)|(int_spi92 << 28)|(int_spi93 << 29)|(int_spi94 << 30)|(int_spi95 << 31);
  new_spi_word32(64, nd);
}

 void gic_arm_tlm::spi_word96()
{  u32_t  nd = 
    (int_spi96 << 0)|(int_spi97 << 1)|(int_spi98 << 2)|(int_spi99 << 3)|(int_spi100 << 4)|(int_spi101 << 5)|(int_spi102 << 6)|(int_spi103 << 7)|(int_spi104 << 8)|(int_spi105 << 9)|(int_spi106 << 10)|(int_spi107 << 11)|(int_spi108 << 12)|(int_spi109 << 13)|(int_spi110 << 14)|(int_spi111 << 15)|(int_spi112 << 16)|(int_spi113 << 17)|(int_spi114 << 18)|(int_spi115 << 19)|(int_spi116 << 20)|(int_spi117 << 21)|(int_spi118 << 22)|(int_spi119 << 23)|(int_spi120 << 24)|(int_spi121 << 25)|(int_spi122 << 26)|(int_spi123 << 27)|(int_spi124 << 28)|(int_spi125 << 29)|(int_spi126 << 30)|(int_spi127 << 31);
  new_spi_word32(96, nd);
 }


u32_t gic_arm_tlm::read_requesting_disjunction(int sgi, int cpuID)
{
  u32_t req;
  if (sgi == 0) // SGI=0 denotes first 32 interrupts that are banked per cpu.
    {
      u32_t sgi_flags = 0;
      // Interrupts 0 to 15 need collecting from the SGI penging matrix
      for (int i=0; i<4; i++)
	{
	  for (int j=0; j<4; j++)
	    {
	      if ((cpustate[cpuID].sgi_matrix[i] >> (8*j)) & 0xFF)
		{
		  sgi_flags |= (1<<(4*i + j));
		}
	    }
	}

      req = sgi_flags | cpustate[cpuID].actives | cpustate[cpuID].unmasked; // These should be the private ones (16 to 31)

    }
  else 
    req = diststate.unmasked[sgi] | diststate.actives[sgi];

  return req;
}

void gic_arm_tlm::distribute()
{
  // Exotic count sequence: first iterate  through cpuIDs and then through SGIs greater than one.
  int cpuID = 0;
  int sgi = 0;
  
  int lopri0[GIC_MAX_N_CPUS], leader0[GIC_MAX_N_CPUS], lopri1[GIC_MAX_N_CPUS], leader1[GIC_MAX_N_CPUS];
  int *lopri[2], *leader[2];
  lopri[0] = lopri0; leader[0] = leader0;
  lopri[1] = lopri1; leader[1] = leader1;
  for (int n=0; n<n_cpus; n++) { leader[0][n] = -1; leader[1][n] = -1; }

  while(sgi<GIC_INTERRUPT_LIMIT/32)
    {
      u32_t req = read_requesting_disjunction(sgi, cpuID);
      req &= ReadDist(GIC_DIST_ENABLE_SET + sgi * 4, cpuID);

      if (req)
	{
	  for (int bit=0; bit<32; bit++)
	    {
	      if (!(req & (1<<bit))) continue;
	      u32_t no = sgi * 32 + bit;

              // The SPI interrupts should only be handled by one core
              // The number of core that should handle the interrupt
              // should be read from application processing unit (mpcore)
              // TODO: need to implement mpcore, for now we always
              // let core 0 to handle SPI interrupts
              if(no >= 32 && no <= 95) {
                  if(cpuID != 0)
                      continue;
              }

	      int pri = (ReadDist(GICD_IPRIORITYR + (no / 4) * 4, cpuID) >> ((no %4) * 8)) & 0xFF;

	      // This reads GICD_ICFGRn register
	      // Why do we need to read this register? (MP: 12th Jan 2105)
	      int cfg = (ReadDist(GIC_DIST_CONFIG + (no/16) * 4, cpuID) >> ((no %16) *2)) & 3;

	      // Here we find to which group the interrupt belongs
	      // This is specified in register GICD_IGROUPRn
	      int grp = (ReadDist(GIC_DIST_IGROUP + ((u32_t)floor(no/32)) * 4, cpuID) >> bit) & 1;
	      
	      int grp_enabled = grp ? diststate.EnableGrp1: diststate.EnableGrp0;
	       
	      int dest_cpu = (cpuID >= 0) ? cpuID: (ReadDist(GICD_ITARGETSR + no/4) >> (8 * (no % 4)), cpuID);
	      
	      int cpu_enabled = 
		(!grp) ? cpustate[cpuID].CTL & GICC_CTL_EnableGrp0:
 		         cpustate[cpuID].CTL & GICC_CTL_EnableGrp1;
	      if (cpu_enabled && grp_enabled && (leader[grp][dest_cpu] < 0 || pri < lopri[grp][dest_cpu]))
		{
                    GIC_TRC(printf("leader[%d][%d]=%d, no=%d, lopri[%d][%d]=%d, pri=%d\n", 
				 grp, dest_cpu, leader[grp][dest_cpu],
				 no, 
				 grp, dest_cpu, lopri[grp][dest_cpu],
				 pri));
		  lopri[grp][dest_cpu] = pri;
		  leader[grp][dest_cpu] = no;
		}
	    }
	}

      //if (leader >= 0) printf("%s: %s: interrupt %i pri=%i discovered.\n", name(), msg, leader, lopri);
      //printf ("Did sgi=%i cpuID=%i\n", sgi, cpuID);
      // Exotic advance composite counter:
      if (cpuID < n_cpus-1) cpuID ++;
      else { cpuID = 0; sgi++; }
    }

  
  // Drive outputs - combine the two group signals and direct to IRQ or FIQ
  // Grp1 is always IRQ. Grp0 is FIQ if FIQEn otherwise also IRQ.
  // We do not implement private bypasss.
  for (int n=0; n<n_cpus; n++) 
    {
      bool FIQEn = ReadCPUBit(GICC_CTLR, cpuID, GICC_CTL_FIQEn);
      bool new_IRQ = false;
      bool new_FIQ = false;
      for (int g=0; g<2; g++)
	{
	  bool intt = leader[g][n] >= 0;
	  if (intt)
	    {
                //if(!IRQ_out[n].read() && (cpustate[n].grps[g].IA == 0x3FF || cpustate[n].grps[g].IA == leader[g][n])) {

                    if (!g && FIQEn) new_FIQ = 1;
                    else new_IRQ = 1;

                    GIC_TRC(printf("%s: Assert interrupt %d with priority %d grp=%i to core=%i  (FIQ=%i IRQ=%i)\n", name(), leader[g][n], lopri[g][n], g, n, new_FIQ, new_IRQ));
                    cpustate[n].grps[g].RP = lopri[g][n];
                    cpustate[n].grps[g].IA = leader[g][n];
                    //}
	    }
	  else {
	    // set default value of 0x3FF to IA
	    cpustate[n].grps[g].IA = 0x3FF;
	  }
	}
      FIQ_out[n] = new_FIQ;
      IRQ_out[n] = new_IRQ;	
    }

}


void gic_arm_tlm::ppi_changed()
{
  int cpus = n_cpus;
  for (int n = 0; n<cpus; n++)
    {
      //ppi_collated[n] =
	u32_t nd =
	(ppi0[n]<<0) |
	(ppi1[n]<<1) |
	(ppi2[n]<<2) |
	(ppi3[n]<<3) |
	(ppi4[n]<<4);
    
	cpustate[n].unmasked &= 0xFFFF; // SGI in here or else in actives.

	// Private interrupts on Zynq-7000 should be from bits 27 to 31
	// This is document in Zynq-7000-TRM.pdf pp 217
	// Therefore we are going to shift nd to 27
	cpustate[n].unmasked |= (nd << GIC_PPI_START_BIT);  
    }

  inputs_have_changed.notify();

}



#if 0

// unsused ARM pseudocode now follows


// Lower number represents higher priority.
// GIC_PriorityMask()
// ==================
// NOTE: where the Security Extensions are not supported, NS_mask = '0'
u8_t gic_arm_tlm::GIC_PriorityMask(int n, bool NS_mask) // Calculate the Binary Point (group) mask.
{ 
  assert(n >= 0 && n <= 7);
  // Range check for the priority mask.
  if (NS_mask)  n = n - 1;
  if (n < MINIMUM_BINARY_POINT)  n = MINIMUM_BINARY_POINT;
  //mask = '1111_1111_0000_0000'<14-n:7-n>;
  int mask = (0xFF00 >> n) & 0xFF;
  return mask;
}


bool ACTIVE(int)
{
  assert(0);
  return 0; // ?
}

bool gic_arm_tlm::IsPending(int interruptID, int cpuID) // Returns TRUE if the interrupt specified by argument interruptID is pending for the CPU specified by cpuID
{
  bool pending = false;
  u32_t target_cpus = ReadDist(GICD_ITARGETSR + interruptID/4, cpuID);
  bool PEND = 0; // ?
  if (PEND && !ACTIVE(interruptID) && (target_cpus & (1<<cpuID))) pending = true;
  return pending;
}



//HighestPriorityPendingInterrupt(): returns the ID of the highest priority interrupt that is pending and enabled.
//Otherwise, returns 1023 (i.e. a spurious interrupt)
//In implementations where interrupts are masked by the distributor group enable bits AFTER
//prioritisation (i.e. IGNORE_GROUP_ENABLE is TRUE), this function may return the ID of a pending
//interrupt in a disabled group even though there is a (lower priority) pending interrupt that is
//fully enabled (i.e. enabled in GICD_IENABLER and the appropriate group enable bit is '1' in
//GICD_CTLR). This is a helper function only and does not explain the full efect of GICC_HPPIR.
// The value returned by a read of GICC_HPPIR is explained in the pseudocode provided with the register description.
int gic_arm_tlm::HighestPriorityPendingInterrupt(int cpu_id)
{
  int num_interrupts = 32 * (Bits(ReadDist(GICD_TYPER), 0, 5) + 1); // Work out how many interrupts are supported
  int hppi = 1023;
  // Set initial ID to be no intterupt pending
  for (int intID = 0; intID <num_interrupts; intID++)
    {
      bool group_enabled = (IsGrp0Int(intID) && ReadDistBit(GICD_CTLR, GICD_CTL_EnableGrp0)) ||  (!IsGrp0Int(intID) && ReadDistBit(GICD_CTLR, GICC_CTL_EnableGrp1));
      if (IsPending(intID, cpu_id) && IsEnabled(intID, cpu_id))
	{
	  if (group_enabled || IGNORE_GROUP_ENABLE)
	    {
	      if (PriorityIsHigher(ReadDist(GICD_IPRIORITYR + intID * 4), ReadDist(GICD_IPRIORITYR + hppi * 4))) 
		{
		  hppi = intID;
		}
	    }
	}
    }
  return(hppi);
}


void gic_arm_tlm::GIC_GenerateExceptions_with_security(bool systemFIQ, bool systemIRQ)
{
  int cpu_count = Bits(ReadDist(GICD_TYPER), 5, 3) + 1; // Determine the number of CPU interfaces.
  while (true) 		  // Loop continuously.
    {
      for (int cpuID = 0; cpuID < cpu_count; cpuID ++)
      {
	  // Loop though CPU interfaces. The iterations of
	  // this loop are permitted to occur in parallel.
	  int xi = UpdateExceptionState_with_security(cpuID); // Returns pending interrupts, masked
	  bool next_int = (xi >> 1) & 1;
	  bool next_grp0 = xi & 1;
	  // by distributor enables but not cpu i/f enables
	  bool irq_wake = false; // IRQ wake up signal to power management, if required
	  bool fiq_wake = false; // FIQ wake up signal to power management, if required
	  bool cpu_irq = false; // IRQ signal to CPU
	  bool cpu_fiq = false; // FIQ signal to CPU
	  if (next_int) 
	    {
	      if (next_grp0 && ReadCPUBit(GICC_CTLR, cpuID, GICC_CTL_FIQEn))
		{
		  fiq_wake = true;
		  if (ReadCPUBit(GICC_CTLR, cpuID, GICC_CTL_EnableGrp0)) cpu_fiq = true;
		}
	      else
		{
		  irq_wake = true;
		  if (next_grp0 && ReadCPUBit(GICC_CTLR, cpuID, GICC_CTL_EnableGrp0) || !next_grp0 && ReadCPUBit(GICC_CTLR, cpuID, GICC_CTL_EnableGrp1)) cpu_irq = true;
		}
	    }
	  

	  // Optional bypass logic
	  if (!GICC_CTLR[cpuID].EnableGrp0 || !ReadCPUBit(GICC_CTLR, cpuID, GICC_CTL_FIQEn))
	    {
	      
	      if (!GICC_CTLR[cpuID].FIQBypDisGrp0 ||
		  (!GICC_CTLR[cpuID].FIQBypDisGrp1 && !ReadCPUBit(GICC_CTLR, cpuID, GICC_CTL_FIQEn)) cpu_fiq = systemFIQ;  // Set FIQ to bypass
		  
		  
		  if (!ReadCPUBit(GICC_CTLR, cpuID, GICC_CTL_EnableGrp1) &&
		  (!ReadCPUBit(GICC_CTLR, cpuID, GICC_CTL_EnableGrp0) || ReadCPUBit(GICC_CTLR, cpuID, GICC_CTL_FIQEn))
		    {
		      
		      if (!GICC_CTLR[cpuID].IRQBypDisGrp1 ||
			  (!GICC_CTLR[cpuID].IRQBypDisGrp0 && GICC_CTLR[cpuID].FIQEn)) cpu_irq = systemIRQ; // Set IRQ to bypass/
		    }
		      // End, optional bypass logic
}
		    
		    SignalFIQ(cpu_fiq, cpuID); // Update driven status of FIQ.
SignalIRQ(cpu_irq, cpuID); // Update driven status of IRQ.
}
		
		}
	}
    }
}

bool gic_arm_tlm::AnyActiveInterrupts(int cpuID) 
{
  assert(0); // todo
  return 0;
}


int /*(boolean, boolean)*/ gic_arm_tlm::UpdateExceptionState_with_security(int cpuID)
{
  int sbp = (unsigned)(ReadCPU(GICC_BPR, cpuID) & 0x7);
  int nsbp = (unsigned)(ReadCPU(GICC_ABPR, cpuID) & 0x7);
  // Secure version of this register.
  bool next_int = false;
  bool next_grp0 = false;
  int intID = HighestPriorityPendingInterrupt(cpuID);
  // Establish the ID of the highest pending
  // interrupt on the this CPU interface.
  int nsmsk;
  if (PriorityIsHigher(ReadDist(GICD_IPRIORITYR + intID * 4), ReadCPU(GICC_PMR, cpuID) & 0xFF) &&
      IsPending(intID, cpuID))
    {
      int smsk = GIC_PriorityMask(sbp, 0);
      if (ReadCPUBit(GICC_CTLR, cpuID, GICC_CTL_CBPR))
	{
	  nsmsk = smsk;
	}
      else
	{
	  nsmsk = GIC_PriorityMask(nsbp, 1);
	  if (IsGrp0Int(intID) && // Highest pending interrupt is secure
	      ReadDistBit(GICD_CTLR, GICC_CTL_EnableGrp0))  // and secure interrupts are enabled
	    {
	      if (!AnyActiveInterrupts(cpuID) ||
		  PriorityIsHigher(ReadDist(GICD_IPRIORITYR + intID * 4), ReadCPU(GICC_RPR, cpuID) & 0xFF & smsk))
		{
		  next_int = true;
		  next_grp0 = true;
		}
	      else
		if (!IsGrp0Int(intID) && // Highest pending interrupt is non-secure
		    ReadDistBit(GICD_CTLR, GICC_CTL_EnableGrp1)) // and non-secure interrupts are enabled
		  {
		    if (!AnyActiveInterrupts(cpuID) ||
			PriorityIsHigher(ReadDist(GICD_IPRIORITYR + intID * 4), ReadCPU(GICC_RPR, cpuID) & 0xFF & nsmsk))
		      {
			next_int = true;
		      }
		    next_grp0 = false;
		  }
	    }
	}
    }
  return (next_int<<1) | next_grp0;
}



// GenerateExceptions()
void gic_arm_tlm::GIC_GenerateExceptions_sans_grouping()
{
  int cpu_count = Bits(ReadDist(GICD_TYPER), 5, 3) + 1; // Determine the number of CPU interfaces.
  while (1)
    {
      // Loop continuously.
      // Determine the number of CPU interfaces.
      for (int cpuID = 0; cpuID < cpu_count; cpuID ++)
	{
	  // Loop though CPU interfaces. The iterations of
	  // this loop are permitted to occur in parallel.
	  int next_irq = UpdateExceptionState_sans_grouping(cpuID);
	  SignalIRQ(next_irq, cpuID);

	  // Update driven status of IRQ.

	}
    }
}

int gic_arm_tlm::UpdateExceptionState_sans_grouping(int cpuID)
{
  bool next_irq = false;
  int intID = HighestPriorityPendingInterrupt(cpuID);   // Establish the ID of the highest pending
  // interrupt on the this CPU interface.
  if (PriorityIsHigher(ReadDist(GICD_IPRIORITYR + intID * 4), ReadCPU(GICC_PMR, cpuID) & 0xFF) && IsPending(intID, cpuID))
    {
      if (ReadDistBit(GICD_CTLR, GICD_CTL_Enable) && ReadCPUBit(GICC_CTLR, cpuID, GICC_CTL_Enable))
	{
	  int mask = GIC_PriorityMask(ReadCPU(GICC_BPR, cpuID) & 0x7, 0);
	  if (!AnyActiveInterrupts(cpuID) ||
	      PriorityIsHigher(ReadDist(GICD_IPRIORITYR + intID * 4), ReadCPU(GICC_RPR, cpuID) & 0xFF & mask)) next_irq = true;
	}
    }
  return(next_irq); // Packed pair of bools?
}

#endif



	
TENOS_KIND_DEFINITION(gic_arm_tlm);



// eof


