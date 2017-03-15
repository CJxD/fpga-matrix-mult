// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2014 M Puzovic + D J Greaves

#include "arm_cp15.h"
#include "armcore_tlm.h"
#include "arm_ccache.h"

// ARM coprocessor number 15

// Constructor
arm_cp15::arm_cp15(sc_module_name name, u8_t core_no) :
    sc_module(name), 
    logfd(NULL),
    m_core_no(core_no),
    sctlr(0x08C50078), //SCTLR reset value
    actlr(0x00000020), // 6th bit is saying that SMP mode is enabled
    cpacr(0x0), 
    csselr(0x0),
    id_mmfr3(0x00102111),
    iciallu(0x0),
    icimvau(0x0),
    bpiall(0x0),
    icialluis(0x0),
    bpiallis(0x0),
    dccmvac(0x0),
    dccmvau(0x0),
    tlbimvais(0x0),
    tlbiasidis(0x0),
    tlbiall(0x0),
    dcimvac(0x0),
    dcisw(0x0),
    dccimvac(0x0),
    dccisw(0x0),
    ttbcr(0x0),
    ttbr0(0x0),
    ttbr1(0x0),
    prrr_mair0(0x0),
    nmrr_mair1(0x0),
    dacr(0x0),
    contextidr(0x0),
    tpidrurw(0x0),
    tpidruro(0x0), 
    tpidrprw(0x0),
    arch_specific_clock_gating(0) {
    target_socket.register_b_transport(this, &arm_cp15::b_access);
}

void arm_cp15::b_access(int id, PW_TLM_PAYTYPE& trans, sc_time &delay) {
    cp_request_extension* ext = 0;
    trans.get_extension(ext);
    assert(ext);

    u8_t* dataptr = trans.get_data_ptr();
    assert(dataptr);
    
    if(trans.is_read()) {
        // MMU is connected first to CP15, after that are cores, so subtract one from id.
        u32_t data = read(id-1, ext->cpr, ext->crm, ext->op1, ext->op2);
        ((u32_t*)dataptr)[0] = data;
    }
    else if(trans.is_write()) {
        u32_t data = *((u32_t*)(dataptr));
        write(ext->cpr, ext->crm, data, ext->op1, ext->op2);
    } else {
        // unknown
        assert(0);
    }

    trans.set_response_status(tlm::TLM_OK_RESPONSE);
}

// CP15 is the system control for the current core.
u32_t arm_cp15::read(int coreid_, u4_t cpr, u4_t crm, u3_t op1, u3_t op2)
{
    
  u32_t r = -1;
  switch (cpr)
    {
      case 0: {
          switch(op1) {
            case 0: {
                switch (op2)
                {
                  case 0: {
                      if(crm == 1) {
                          // ID_PFR0, for parallela board we are only stating
                          // that ARM instruction set implemented bits[3:0] = 0b0001
                          r = 0x1;
                      }
                      else {
                          // TODO: check if this is correct, this is based on message
                          // when kernel is booting up on Parallella board
                          r = 0x413FC090; 
                      }

                      break;
                  }
                    
                  case 1:
                    // CTR (Cache Type Register) 
                    // Vritual Index/Physical Tag and number of words in the smallest
                    // cache line in data and instructions caches is log2(8 words)
                    r = 0x80038003;
                    break; 

                  case 3: //  Instruction set attributes ISAR3

		    /* From linux kernel arch_setup:                                                                                                    
		     * If the CPU supports LDREX/STREX and LDREXB/STREXB,  avoid advertising SWP; it may not be atomic with multiprocessing cores.
		     * sync_prim = ((read_cpuid_ext(CPUID_EXT_ISAR3) >> 8) & 0xf0) | ((read_cpuid_ext(CPUID_EXT_ISAR4) >> 20) & 0x0f);
		    */
		    if (crm == 2)
		      {
			// We need to populate ths from table 3-34 of the ARM tech ref according to the features we have implemented.
			// We have done more than this so far: TODO.
			r = 0;
			r |= 0x2<<12; // LRDEX/STREX
			//printf("ISAR3 CODE needed 0x%x\n", r);
		      }
		    else assert(0);
		    break;
                    

                    
                  case 4:
                    if(crm == 1) 
		      r = 0x3;                     // ID_MMFR0 - VMSA support is 0b0011 i.e. support for VMSAv7
		    else if (crm == 2) // Instruction set attributes ISAR4 
		      {
			r = 0;
			r |= 0x1<<12; // SMC
			r |= 0x1<<8;  // writebacks
			r |= 0x4<<4;  // with shift load/stores
			r |= 0x1<<0;  // Unpriv instructions
			//printf("ISAR4 CODE needed 0x%x\n", r);
		      }
                    else
		      assert(0);
                    break;

                  case 5: // MPIDR - Multiprocessor affinity register.
                    // Processor ID in the bottom byte (0..3). Cluster in the byte above that.
                    r = m_core_no;
                    r |= (1<<31); // processor includes the multiprocessiong extensions
		    tenos_diagnostics.checkpoint(2, name(), "trap readid");
                    break;
                    

                  case 7:
                    if(crm == 1)
                        r = id_mmfr3;
                    else 
                        assert(0);

                    break;
                            

                  default:
                    printf("Invalid CP15 read cpr=%i crm=%i op1=%i op2=%i\n", cpr, crm, op1, op2);
                    assert(0);
                }
                break;
            }
            case 1: {
                if(op2 == 0) {
                    // CCSIDR
                    // find out cache type and level
                    // TODO: this information should be obtainable
                    // directly from arm_ccache class
                    u1_t type = csselr & 0x1;
                    u3_t level = ((csselr >> 1) & 0x7) + 1;
                    if((type == 1 || type == 0) && level == 1) {
                        // for instruction and data cache at level 1
                        // information is:
                        // write-back bit[30] = 1
                        // write-allocation bit[28] = 1
                        // number of sets bits[27:13] = 255
                        // associativty bits[12:3] = 4 - 1
                        // line size bits[2:0] = log(8 words in cache line) - 2
                        r = 0x501FE019;
                    }                        
                    else if(type == 0 && level == 2) {
                        // for unified cache at level 2
                        // information is
                        // write-back bit[30] = 1
                        // write-allocation bit[28] = 1
                        // number of sets bits[27:13] = 2047
                        // associativity bits[12:3] = 8 - 1
                        // line size bits[2:0] = log(8 wods in cache line) - 2
                        r = 0x50FFE039;
                    }
                    else {
                        assert(0);
                    }
                }
                else if(op2 == 1) {
                    // CLIDR Parallela board has two levels, where L1 are
                    // separate I and D caches, while L2 is unified
                    
                    // LoUU = clean all caches (level 1 and level 2)
                    // LoC = clean all caches (level 1 and level 2)
                    // LoUIS = clean all caches (level 1 and level 2)
                    r = 0x12400023;
                }
                break;
            }

            case 2: {
                if(op2 == 0) {
                    // read for CSSELR
                    assert(0);
                }
                break;
            }
          }
          break;
      }
        
      case 1: {
          switch(op2) {
            case 0: {
                if(op1 == 0 && crm == 0)
                    // SCTLR
                    r = sctlr;
                break;

            }
            case 1: {
                if(crm == 0 && op1 == 0)
                    // ACTLR -- implementation defined
                    r = actlr;
                break;
            }
            case 2: {
                if(crm == 0 && op1 == 0) 
                    r = cpacr;
                break;
            }
          }
          break;
      }

      case 2: {
	  if(crm == 0 && op1 == 0) {
	      switch(op1) {
	        case 0: {
		    r = ttbr0;
		    break;
		  }
	        case 1: {
		    r = ttbr1;
		    break;
		}
	        case 2: {
		    r = ttbcr;
		    break;
		}
	      }
	  }
	  break;
      }

      case 3: {
          if(crm == 0 && op1 == 0 && op2 == 0) {
              r = dacr;
          }
          break;
      }

      case 5: {
	  if(crm == 0 && op1 == 0) {
	      switch(op2) {
	        case 0: {
		  r = dfsr;
		  break;
	        }
	        case 1: {
		  r = ifsr;
		  break;
	        }
	      }
	  }
	  break;
      }

      case 6: {
	  if(crm == 0 && op1 == 0) {
	      switch(op2) {
	        case 0: {
		  r = dfar;
		  break;
	        }
	        case 2: {
		  r = ifar;
		  break;
	        }
	      }
	  }
	  break;
      }

      case 13: {
	  if(op1 == 0 && crm == 0) {
	    switch(op2) {
	      case 1: {
		r = contextidr;
		break;
	      }
              case 2: {
                  r = tpidrurw;
                  break;
              }
              case 3: {
                  r = tpidruro;
                  break;
              }
	      case 4: {
		r = tpidrprw;
		break;
	      }
	    }
	  }
	  break;
      }
	
      case 15: 
	  if(op1 == 4 && crm == 0 && op2 == 0)
	    { // In Linux kernel this is obtained from
              // method scu_a9_get_base() in arch/arm/mach-zynq/common.c
              // and then used as virtual address for which
              // mapping needs to be created.
	      r = PERIPHBASE;
	  }
	  else if(op1==0 && crm==0 && op2==0)
	    {
	      r = arch_specific_clock_gating; // cpr=15, crm=0, op1=0, op2=0.
	      /*
	      c001d9c8 <zynq_secondary_init>:
	      c001d9c8:       ee1fcf10        mrc     15, 0, ip, cr15, cr0, {0}
	      c001d9cc:       e38cc001        orr     ip, ip, #1
	      c001d9d0:       ee0fcf10        mcr     15, 0, ip, cr15, cr0, {0}
	      c001d9d4:       ee111f30        mrc     15, 0, r1, cr1, cr0, {1}
	      c001d9d8:       e3811006        orr     r1, r1, #6
	      c001d9dc:       ee011f30        mcr     15, 0, r1, cr1, cr0, {1}
	      c001d9e0:       e12fff1e        bx      lr
	      */
	      fprintf(stderr, "cp15_read(cpr=%d, crm=%d, op1=%d, op2=%d) --> DUO READ\n", cpr, crm, op1, op2);
	      //tenos_diagnostics.checkpoint(2, name(), "cp15_read_failed new");
	    }

	  break;
      }
  if(logfd) fprintf(logfd, "cp15_read(cpr=%d, crm=%d, op1=%d, op2=%d) = %08x\n", cpr, crm, op1, op2, r);


  if(r == -1) 
    {
      fprintf(stderr, "cp15_read(cpr=%d, crm=%d, op1=%d, op2=%d) --> NOT IMPLEMENTED\n", cpr, crm, op1, op2);
      tenos_diagnostics.checkpoint(2, name(), "cp15_read_failed");
      assert(0);
    }
      


  return r;
}


void arm_cp15::write(u4_t cpr, u4_t crm, u32_t data, u3_t op1, u3_t op2) 
{
    bool wsuccess = false;

    if(logfd)
        fprintf(logfd, "cp15_write(data=%08x, cpr=%d, crm=%d, op1=%d, op2=%d)\n", data, cpr, crm, op1, op2);
    switch(cpr) {
      case 0: {
          if(op1 == 2 && op2 == 0) {
              // CSSELR
              csselr = data;
              wsuccess = true;
          }

          break;
      }

      case 1: {
          switch(op2) {
            case 0: {
                if(crm == 0 && op1 == 0) {
                    // SCTLR
                    if(((sctlr & 1) == 0) &&
                       ((data & 1) == 1)) {
                        // we need to enable MMU
                        u32_t temp_data = -1;
                        cp15_mmu_control_extension::operation op = cp15_mmu_control_extension::Enable;
                        mmu_control_msg(
                            op,
                            cp15_mmu_control_extension::UNKNOWN_TYPE, // type is not important
                            temp_data);
                    }
                    
                    if(!(sctlr & (1 << 2)) &&
                       (data & (1 << 2))) {
                        // we are enabling data and unified caches
                        u3_t types = (1 << arm_ccache::Data) | (1 << arm_ccache::Unified);
                        u3_t op = (1 << cp15_cache_control_extension::Enable);

                        cache_control_msg(types, -1, -1, -1, op, -1, 1);
                    }
                    
                    if(!(sctlr & (1 << 12)) &&
                       (data & (1 << 12))) {
                        // we are enabling instruction caches
                        u3_t types = (1 << arm_ccache::Instruction);
                        u3_t op = (1 << cp15_cache_control_extension::Enable);

                        cache_control_msg(types, -1, -1, -1, op, -1, 1);
                    }

                    sctlr = data;
                    
                    wsuccess = true;
                }
                break;
            }
              
            case 1: {
                if(crm == 0 && op1 == 0) {
                    actlr = data;
                    wsuccess = true;
                }
                break;
            }

            case 2: {
                if(crm == 0 && op1 == 0) {
                    cpacr = data;
                    wsuccess = true;
                }
                break;
            }
            
          }
          break;
      }

      case 2: {
          switch(crm) {
            case 0: {
                if(op1 == 0) {
                    switch(op2) {
                      case 0: {
                          ttbr0 = data;
                          wsuccess = true;
                          break;
                      }
                      case 1: {
                          ttbr1 = data;
                          wsuccess = true;
                          break;
                      }

                      case 2: {
                          // TTBCR
                          ttbcr = data;
                          wsuccess = true;
                          break;
                      }
                    }
                }
                break;
            }
          }
          break;
      }

      case 3: {
          if(crm == 0 && op1 == 0 && op2 == 0) {
              // DACR
              dacr = data;
              wsuccess = true;
          }
          break;
      }

      case 5: {
	  if(crm == 0 && op1 == 0) {
	      switch(op2) {
	        case 0: {
		  dfsr = data;
		  wsuccess = true;
		  break;
	        }
	        case 1: {
		  ifsr = data;
		  wsuccess = true;
		  break;
	        }
	      }
	  }
	  break;
      }

      case 6: {
	  if(crm == 0 && op1 == 0) {
	      switch(op2) {
	        case 0: {
		  dfar = data;
		  wsuccess = true;
		  break;
	        }
	        case 2: {
		  ifar = data;
		  wsuccess = true;
		  break;
	        }
	      }
	  }
	  break;
      }


      case 7: {
          switch(crm) {
            case 1: {
                if(op1 == 0) {
                    switch(op2) {
                      case 0: {
                          u3_t cache = 1 << arm_ccache::Instruction;
                          u1_t domain = arm_ccache::Inner;
                          u1_t shareable = true;
                          u3_t op = 1 << cp15_cache_control_extension::Invalid;
                          u2_t stop = cp15_cache_control_extension::PoU;
                          cache_control_msg(cache, domain, shareable, -1, op, stop, 1);

                          icialluis = data;
                          wsuccess = true;
                          break;
                      }
                      case 6: {
                          // TODO: we do not have branch predictor
                          // at the moment
                          // BPIALLIS 
                          bpiallis = data;
                          wsuccess = true;
                          break;
                      }
                    }
                }
                    
                break;
            }
            case 5: {
	      if(op1 == 0) {
		switch(op2) {
		  case 0: {
		    // ICIALLU
                    // instruction cache
                    u3_t cache = 1 << arm_ccache::Instruction;
                    u3_t op = 1 << cp15_cache_control_extension::Invalid;
                    u2_t stop = cp15_cache_control_extension::PoU;
                    cache_control_msg(cache, -1, -1, -1, op, stop, 1);

		    iciallu = data;

		    wsuccess = true;
		    break;
		  }
		  case 1: {
                    // ICIMVAU
                    // obtain physical address (only if cache is physically tagged, physically indexed)
                    mmu_control_msg(
                        cp15_mmu_control_extension::Translate,
                        cp15_mmu_control_extension::UNKNOWN_TYPE,
                        data);

                    u3_t cache = (1 << arm_ccache::Instruction);
                    u3_t op = (1 << cp15_cache_control_extension::Invalid);
                    u2_t stop = cp15_cache_control_extension::PoU;
                    cache_control_msg(cache, -1, -1, data, op, stop, 1);
 
		    icimvau = data;
		    wsuccess = true;
		    break;
		  }

		  case 6: {
                    // TODO: we do not have a branch bredictor
                    // at the moment
		    // BPIALL
		    bpiall = data;
		    wsuccess = true;
		    break;
		  }

		}
	      }
	      break;
            }

            case 6: {
                if(op1 == 0 && op2 == 1) {
                    // DCIMVAC
                    // invalidate data or unified cache line by MVA to PoC
                    
                    // obtain physical address (only if cache is physically tagged, physically indexed)
                    mmu_control_msg(
                        cp15_mmu_control_extension::Translate,
                        cp15_mmu_control_extension::UNKNOWN_TYPE,
                        data);
                    
                    u3_t cache = 1 << (arm_ccache::Data) | (1 << arm_ccache::Unified);
                    u3_t op = (1 << cp15_cache_control_extension::Clean) | (1 << cp15_cache_control_extension::Invalid);
                    u2_t stop = cp15_cache_control_extension::PoC;
                    cache_control_msg(cache, -1, -1, data, op, stop, 1);
                  
                    dcimvac = data;
                    wsuccess = true;
                }
                else if(op1 == 0 && op2 == 2) {
                    // DCISW 
                    // data cache invalidate by set/way
                    u3_t cache = 1 << (arm_ccache::Data) | (1 << arm_ccache::Unified);
                    u3_t op = (1 << cp15_cache_control_extension::Invalid);
                    u2_t stop = cp15_cache_control_extension::None;
                    cache_control_msg(cache, -1, -1, data, op, stop, 0);

                    dcisw = data;
                    wsuccess = true;
                    // TODO: ignored for now, needs to be implemented
                }
                break;
                
            }
	      
	    case 10: {
	      if(op1 == 0 && op2 == 1) {
                  // DCCMVAC 
                  // clean data or unified cache line by MVA to PoC

                  // obtain physical address (only if cache is physically tagged, physically indexed)
                  mmu_control_msg(
                      cp15_mmu_control_extension::Translate,
                      cp15_mmu_control_extension::UNKNOWN_TYPE,
                      data);
                  
                  u3_t cache = 1 << (arm_ccache::Data) | (1 << arm_ccache::Unified);
                  u3_t op = 1 << cp15_cache_control_extension::Clean;
                  u2_t stop = cp15_cache_control_extension::PoC;
                  cache_control_msg(cache, -1, -1, data, op, stop, 1);
                                    
                  dccmvac = data;
                  wsuccess = true;
	      }
	      break;
	    }

	    case 11: {
              if(op1 == 0 && op2 == 1) {
                  // obtain physical address (only if cache is physically tagged, physically indexed)
                  mmu_control_msg(
                      cp15_mmu_control_extension::Translate,
                      cp15_mmu_control_extension::UNKNOWN_TYPE,
                      data);

                  u3_t cache = 1 << (arm_ccache::Data) | (1 << arm_ccache::Unified);
                  u3_t op = (1 << cp15_cache_control_extension::Clean) | (1 << cp15_cache_control_extension::Invalid);
                  u2_t stop = cp15_cache_control_extension::PoU;
                  cache_control_msg(cache, -1, -1, data, op, stop, 1);
                  
                  dccmvau = data;
                  wsuccess = true;
              }
	      break;
	    }

	    case 14: {
	      if(op1 == 0 && op2 == 1) {
                 // obtain physical address (only if cache is physically tagged, physically indexed)
                mmu_control_msg(
                    cp15_mmu_control_extension::Translate,
                    cp15_mmu_control_extension::UNKNOWN_TYPE,
                    data);

                u3_t cache = 1 << (arm_ccache::Data) | (1 << arm_ccache::Unified);
                u3_t op = (1 << cp15_cache_control_extension::Clean) | (1 << cp15_cache_control_extension::Invalid);
                u2_t stop = cp15_cache_control_extension::PoC;
                cache_control_msg(cache, -1, -1, data, op, stop, 1);
                
                dccimvac = data;
		wsuccess = true;
	      }
              else if(op1 == 0 && op2 == 2) {
                  u3_t cache = 1 << (arm_ccache::Data) | (1 << arm_ccache::Unified);
                  u3_t op = (1 << cp15_cache_control_extension::Clean) | (1 << cp15_cache_control_extension::Invalid);
                  u2_t stop = cp15_cache_control_extension::None;
                  cache_control_msg(cache, -1, -1, data, op, stop, 0);
                  
                  dccisw = data;
                  wsuccess = true;
              }
	      break;
	    }


	  }
	    
          break;
      }

      case 8: {
          switch(crm) {
            case 3: {
                if(op1 == 0 && op2 == 1) {
                    // TLBIMVAIS - invalidate unified TLB by virtual address
                    cp15_mmu_control_extension::operation op = cp15_mmu_control_extension::InvalidByMVA;
                    cp15_mmu_control_extension::type ty = cp15_mmu_control_extension::Unified;
                    mmu_control_msg(op, ty, data);

                    tlbimvais = data;
                    wsuccess = true;
                }
                else if(op1 == 0 && op2 == 2){ 
                    // TLBIASIDIS - invalidate unified TLB by ASID
                    cp15_mmu_control_extension::operation op = cp15_mmu_control_extension::InvalidByASID;
                    cp15_mmu_control_extension::type ty = cp15_mmu_control_extension::Unified;
                    mmu_control_msg(op, ty, data);

                    tlbiasidis = data;
                    wsuccess = true;
                }
                break;
            }
            case 7: {
                if(op1 == 0 && op2 == 0) {
                    // TLBIALL
                    cp15_mmu_control_extension::operation op = cp15_mmu_control_extension::InvalidAll;
                    cp15_mmu_control_extension::type ty = cp15_mmu_control_extension::Unified;
                    u32_t data = -1;
                    
                    mmu_control_msg(op, ty, data);
                    
                    tlbiall = data;
                    wsuccess = true;
                }
                break;
            }
          }
          break;
      }

      case 10: {
          switch(crm) {
            case 2: {
                if(op1 == 0) {
                    switch(op2) {
                      case 0: {
                          // PRRR = primary region remap register when
                          // without large physical address extension
                          // MAIR0 = memory attribute indirection register 0
                          // with large physical address extension
                          prrr_mair0 = data;
                          wsuccess = true;
                          break;
                      }
                      case 1: {
                          // NMRR = normal memory remap register when
                          // without large physical address extension
                          // MAIR1 = memory attribute indirection register 1
                          // with lage physical address extension
                          nmrr_mair1 = data;
                          wsuccess = true;
                          break;
                      }
                    }
                }
                break;
            }
          }
          break;
      }

      case 13: {
          switch(op1) {
            case 0 : {
                switch(op2) {
                  case 1: {
                      if(crm == 0) {
                          contextidr = data;
                          wsuccess = true;
                      }
                  }
                  case 2: {
                      if(crm == 0) {
                          tpidrurw = data;
                          wsuccess = true;
                      }
                      break;
                  }
                  case 3: {
                      if(crm == 0) {
                          tpidruro = data;
                          wsuccess = true;
                      }
                      break;
                  }
                  case 4: {
                      if(crm == 0) {
                          tpidrprw = data;
                          wsuccess = true;
                          break;
                      }
                      break;
                  }
                }
                break;
            }
          }
          break;
      }

    case 15:
      if(op1==0 && crm==0 && op2==0)
	{
	  arch_specific_clock_gating = data; // cpr=15, crm=0, op1=0, op2=0.
	  wsuccess = true;
	  break;
	}
      break;

    }

    if(!wsuccess) {
        fprintf(stderr, "cp15_write(data=%08x, cpr=%d, crm=%d, op1=%d, op2=%d) -> NOT IMPLEMENTED\n", data, cpr, crm, op1, op2);
	tenos_diagnostics.checkpoint(2, name(), "cp15_write_failed");
        assert(0);
    }

}

void arm_cp15::cache_control_msg(u3_t cache, u1_t domain, u1_t shareable, u32_t data, u3_t op, u2_t stop, u1_t address) {

    for(int i = 1; i < initiator_socket.size(); i++) {

        cp15_cache_control_extension ext;
        ext.types = cache;
        ext.domain = domain;
        ext.shareable = shareable;
        ext.data = data;      
        ext.op = op;
        ext.stop = stop;
        ext.address = address;

        PW_TLM_PAYTYPE trans;
        trans.set_extension(&ext);
        trans.set_byte_enable_length(0);
        trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
        trans.set_read();
        trans.set_address(0);
        trans.set_data_length(4);
        trans.set_data_ptr((unsigned char*) &data);
        
        sc_time delay = SC_ZERO_TIME;
        // send the message to all connected caches
        // they start at 1 because at index 0 is MMU
        initiator_socket[i]->b_transport(trans, delay);
        assert(!trans.is_response_error());
        
        trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
        
        trans.clear_extension<cp15_cache_control_extension>(&ext);
    }

    return;
}

void arm_cp15::mmu_control_msg(
    cp15_mmu_control_extension::operation op, 
    cp15_mmu_control_extension::type ty,
    u32_t& data) {

    cp15_mmu_control_extension ext;
    ext.data = data;      
    ext.op = op;
    ext.ty = ty;
    
    PW_TLM_PAYTYPE trans;
    trans.set_extension(&ext);
    trans.set_byte_enable_length(0);
    trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    trans.set_read();
    trans.set_address(0);
    trans.set_data_length(4);
    trans.set_data_ptr((unsigned char*) &data);

    sc_time delay = SC_ZERO_TIME;
    // MMU is at index 0
    initiator_socket[0]->b_transport(trans, delay);
    assert(!trans.is_response_error());
    
    trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    
    trans.clear_extension<cp15_mmu_control_extension>(&ext);   
}

// eof
