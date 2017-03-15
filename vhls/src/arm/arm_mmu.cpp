// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>


//
//
//

#include "arm_mmu.h"
#include "base_mmu_tlm.h"
#include "armisa.h"
#include "arm_cp15.h"
#include "arm_timers.h"
#include "armcore_tlm.h"

#define ARM_MMU_STATS(X) X

using namespace sc_core;
using namespace tlm;

arm_mmu::arm_mmu(sc_module_name name, u32_t _size) : // constructor
    sc_module(name),
    m_active(true),
    base_mmu(),
    size(_size), 
    table(new tlb_entry[_size]) {
  ever_enabled = false;
  ARM_MMU_STATS(m_stats.collecting = true);
  cp15_target.register_b_transport(this, &arm_mmu::b_cp15_access);
  cores_target.register_b_transport(this, &arm_mmu::b_core_access);
}

void arm_mmu::b_cp15_access(PW_TLM_PAYTYPE &trans, sc_time &delay) {
    cp15_mmu_control_extension *ext = 0;
    trans.get_extension(ext);
    assert(ext);

    switch(ext->op) {
      // at the moment we are only supporting unified
      // TLB so when we receive invalid message we need
      // to invalidate TLB table here
      case cp15_mmu_control_extension::InvalidAll: {
          assert(ext->data == -1);
          for(int i = 0; i < size; i++) {
              table[i].valid = false;
              ARM_MMU_STATS(m_stats.tlb_invalid++);
          }
          break;
      }

      case cp15_mmu_control_extension::InvalidByMVA: {
          // we need to invalid virtual address
          u32_t vaddr = ext->data;
          for(int i = 0; i < size; i++) {
              u32_t start = table[i].get_vpn();
              u32_t end = start + table[i].size;
              if(table[i].valid &&
                  vaddr >= start &&
                  vaddr < end) {
                  table[i].valid = false;
                  ARM_MMU_STATS(m_stats.tlb_invalid++);
                  break;
              }
          }
      
          break;
      }

      case cp15_mmu_control_extension::InvalidByASID: {
          // we need to invalid virtual address
          u8_t asid = ext->data;
          for(int i = 0; i < size; i++) {
              if(table[i].get_asid() == asid && table[i].valid) {
                  ARM_MMU_STATS(m_stats.tlb_invalid++);
                  table[i].valid = false;
              }
          }
      
          break;
      }


      case cp15_mmu_control_extension::Enable: {
          assert(!is_active());
          enable();
          break;
      }
      case cp15_mmu_control_extension::Translate: {
          u8_t* dataptr = trans.get_data_ptr();

          u32_t vaddr = *((u32_t*)dataptr);
          u32_t paddr;
          if(!translate(vaddr, paddr, Read))
              paddr = vaddr;

          ((u32_t*)dataptr)[0] = paddr;
          break;
      }
      default:
        // we should never get here
        assert(0);
    }

    trans.set_response_status(tlm::TLM_OK_RESPONSE);
}

// There are 2 (two) possible income connections to the MMU.
// +-- (1) it is coming from data socket (initiator_socket)
// +-- (2) it is coming from instruction socket (ifetch_socket)

// There are 4 (four) possible outgoing connections
// +-- (1) private timer and watchdog (always present)
// +-- (2) global timer (always present)
// +-- (3) interrupt controller (always present)
// +-- (4a) if caches are present
// +------ (a) data cache
// +------ (b) instruction cache
// +-- (4b) if caches are not present
// +-------(a) SCU

void arm_mmu::b_core_access(int id, PW_TLM_PAYTYPE &trans, sc_time &delay) {

    if(!m_active) {
        // simply forward transaction
        return memories_initiator[id+3]->b_transport(trans, delay);
    }

    // MMU is active so we need to do address translation
    u32_t paddr;
    bool fault = false;
    // TODO: how do we deal when we have translation fault??
    if(!translate(trans.get_address(), paddr, trans.is_read() ? Read : Write)) {
        paddr = trans.get_address();
        fault = true;
    }

    if(fault) {
        trans.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
        return; // transaction will return as incomplete
    }

    // now forward transaction with this physical address to memories
    trans.set_address(paddr);

    // Check to see if address is for private timer
    if(paddr >= TWD_BASE_ADDR && paddr < (TWD_BASE_ADDR + TWD_LENGTH)) {
      // Send it to private timer
      memories_initiator[0]->b_transport(trans, delay);
    } else if(paddr >= GLBT_BASE_ADDR && paddr < (GLBT_BASE_ADDR + GLBT_LENGTH)) {
      // Send it to the global timer
      assert(0); // This has not be tested as we don't have use case (let MP know if you hit it)
      memories_initiator[1]->b_transport(trans, delay);
    } else if (((paddr >= (PERIPHBASE + GIC_CPU_FILE_OFFSET) )
                && (paddr  < (PERIPHBASE + GIC_CPU_FILE_OFFSET + GIC_CPU_FILE_SPACING)))
               || ((paddr >= (PERIPHBASE + GIC_DISTRIBUTOR_BASE_OFFSET)) 
                   && (paddr < (PERIPHBASE + GIC_DISTRIBUTOR_BASE_OFFSET + GIC_DISTRIBUTOR_SPACING)))) {
        // Forward to GIC controller
        memories_initiator[2]->b_transport(trans, delay);
    }
    else {
      memories_initiator[id+3]->b_transport(trans, delay);
    }
}

arm_mmu::~arm_mmu() {
    delete[] table;
}

tlb_entry* arm_mmu::lookup(u32_t vaddr, u8_t asid) {
    tlb_entry* ret = NULL;

    for(int x = 0; x < size; ++x) {
        if(table[x].match(vaddr, asid)) {
#if 1
            ret = &table[x];
#else
            // the policy is LRU
            // we mantain LRU array
            tlb_entry tmp_entry = table[x];
            for(int i = x; i > 0; i--)
                table[i] = table[i-1];
            table[0] = tmp_entry;
            ret = &table[0];
#endif

            break;
        }
    }

    return ret;
}

void arm_mmu::insert(u32_t addr, tlb_entry& entry) {
    for(int i = size - 1; i > 0; --i)
        table[i] = table[i-1];
    table[0] = entry;
}

void arm_mmu::print_tlb() const {
    fprintf(stderr, "Current TLB contents:\n");
    tlb_entry *te;
    for(int i = 0; i < size; i++) {
        te = &table[i];
        if(te->is_valid())
  	    fprintf(stderr, " -> %s\n", te->print().c_str());
    }
}

bool arm_mmu::translate(
    u32_t vaddr, 
    u32_t& paddr,
    access_type mode) {

    // first check to see if MMU is disabled
    u32_t cp15_data;
    cp15_op(1, 0, cp15_data, 0, 0, true);
    u1_t mmu_status = cp15_data & 0x1;
    if(!mmu_status) {
      // MMU is disabled, we do not need to do
      // any translation and should simply forward
      // request to the main memory

      // TODO: additional checks described in
      // Section 6.4.3 in aRM DDI 0333H

      // TODO: mp to implement
      assert(0);
    }

    bool is_fetch = (mode == Execute);
    bool is_write = (mode == Write);

    // find what is ASID
    // ASID is lower 8 bits of register c13 in CP15
    cp15_op(13, 0, cp15_data, 0, 1, true);
    u8_t context = cp15_data & 0xFF;

    MMUTRC(vaddr,
	   printf("ARM_MMU::TRANSLATE(mode=%d) -> looking up virtual address 0x%x\n", mode, vaddr)); 
    tlb_entry* te = lookup(vaddr, context);
    if(te == NULL) {

        ARM_MMU_STATS(
            u32_t stats_idx;

            if (is_fetch) {
                m_stats.tlb_miss[2]++;
                stats_idx = 2;
            }
            else if(is_write) {
                m_stats.tlb_miss[1]++;
                stats_idx = 1;
            }
            else {
                m_stats.tlb_miss[0]++;
                stats_idx = 0;
            });
            
	MMUTRC(vaddr,
	       printf("ARM_MMU::TRANSLATE(mode=%d) -> lookup miss for virtual address 0x%x.\n", mode, vaddr));

        tlb_entry new_entry;
        // do the page table walk 
        faults f = page_table_walk(vaddr, new_entry);
        
        // Fault status and address
        // Section 6.10 pp 6-34 table 6-11 ARM DDI 0333H

        u32_t fsr;
        u32_t far;
        if(mode == Execute) {
            // instructions
	    cp15_op(5, 0, fsr, 0, 1, true); // MRC p15, 0, <Rd>, c5, c0, 1 -- IFSR
	    cp15_op(6, 0, far, 0, 2, true); // MRC p15, 0, <Rd>, c6, c0, 2 -- IFAR
        } else if(mode == Read || mode == Write) {
            // data
	    cp15_op(5, 0, fsr, 0, 0, true); // MRC p15, 0, <Rd>, c5, c0, 0 -- DFSR
	    cp15_op(6, 0, far, 0, 0, true); // MRC p15, 0, <Rd>, c6, c0, 0 -- DFAR
        }
        fsr = 0;
        far = vaddr;
        switch(f) {
           case NoFault:
           {
               new_entry.asid = context;
               new_entry.global = false;
               new_entry.valid = true;
               
               // need to insert the entry to TLB
               MMUTRC(vaddr,
                      printf("ARM_MMU::TRANSLATE(mode=%d) -> insert physical address 0x%08x for virtual address 0x%08x\n", 
                             mode, new_entry.pfn, new_entry.vpn));
               ARM_MMU_STATS(m_stats.tlb_insert[stats_idx]++);

               insert(vaddr, new_entry);
               
               // indicate TLB miss
               fsr = (fsr & ~0xf) | 0; 
               
               break;
           }

           case TranslationSection:
           {
               MMUTRC(vaddr,
                      printf("ARM_MMU::TRANSLATE(mode=%d) -> translation section FAULT virtual address 0x%x\n", 
                             mode, vaddr));

               // indicate translation section fault
               fsr = (fsr & ~0xf) | 5;
               ARM_MMU_STATS(m_stats.section_fault[stats_idx]++);

               break; 
           }

           case TranslationPage:
           {
               MMUTRC(vaddr,
                      printf("ARM_MMU::TRANSLATE(mode=%d) -> translation page FAULT virtual address 0x%x\n", 
                             mode, vaddr));

               fsr = (fsr & ~0xf) | 7;
               ARM_MMU_STATS(m_stats.page_fault[stats_idx]++);

               break;
           }
            
        }

        if(f != NoFault) {
            if(mode == Execute) {
                // instructions
                cp15_op(5, 0, fsr, 0, 1, false); // MCR p15, 0, <Rd>, c5, c0, 1
                cp15_op(6, 0, far, 0, 2, false); // MCR p15, 0, <Rd>, c6, c0, 1
            } else if(mode == Read || mode == Write) {
                // we need to set fsr's 12th bit to true
                // if we are writing, otherwise different
                // virtual address might map to the same
                // physical address by Linux kernel
                if(mode == Write)
                    fsr |= (1 << 11);
                // data
                cp15_op(5, 0, fsr, 0, 0, false); // MCR p15, 0, <Rd>, c5, c0, 1
                cp15_op(6, 0, far, 0, 0, false); // MCR p15, 0, <Rd>, c6, c0, 1
            }

            return false;
        }
        
        // when we do the second lookup
        // the entry must be in TLB
        te = lookup(vaddr, context);
        if(!te)
            print_tlb();
        // we should never hit this
        assert(te);
    }
    else {
	MMUTRC(vaddr,
	       printf("ARM_MMU::TRANSLATE(mode=%d) -> lookup hit for virtual address 0x%x.\n", mode, vaddr));

        ARM_MMU_STATS(
            if(is_fetch)
                m_stats.tlb_hit[2]++;
            else if(is_write)
                m_stats.tlb_hit[1]++;
            else
                m_stats.tlb_hit[0]++;
            );
    }

    // check to see access bits, if we are writing
    // and access is only read only we should create
    // page fault
    if(is_write && te->bits == 12 && te->access == 1 && te->domain == 1) {
        // we should page fault in this case
        // because we are trying to write to a 
        // page that is write protected
        u32_t far = vaddr;
        // according to Linux kernel source this is 
        // number that it expects to see if we are 
        // having page permission fault
        u32_t fsr = 15; 
        // set it that we are writing to page
        fsr |= (1 << 11);

        // Set TLB entry to be invalid
        te->valid = false;

        ARM_MMU_STATS(m_stats.page_fault_permission++);

        // updating FAR and FSR registers
        cp15_op(5, 0, fsr, 0, 0, false); // MCR p15, 0, <Rd>, c5, c0, 1
        cp15_op(6, 0, far, 0, 0, false); // MCR p15, 0, <Rd>, c6, c0, 1

        return false;
    }
        
    paddr = te->get_pfn(vaddr);
    MMUTRC(vaddr,
           printf("ARM_MMU::TRANSLATE(mode=%d) -> return physical address 0x%x for virtual address 0x%x.\n", 
		  mode, paddr, vaddr));

    return true;
}

arm_mmu::faults arm_mmu::page_table_walk(u32_t vaddr, tlb_entry& entry) {
  // get translation table base control register (TTBCR)
  u32_t ttbcr;
  cp15_op(2, 0, ttbcr, 0, 2, true);
  u3_t N = ttbcr & 0x7;
  u1_t pd0 = (ttbcr & 0x10) >> 4;
  u1_t pd1 = (ttbcr & 0x20) >> 5 ;

  u32_t l1_descriptor_addr = 0;
  // figure out whether we are using TTBR0 or TTBR1
  if(N > 0 && (vaddr & ~((1 << (32-N))-1)) != 0) {
    // use TTBR1
    u32_t ttbr1;
    cp15_op(2, 0, ttbr1, 0, 1, true);
    // {TTBR1[31:14], MVA[31:20], 00}
    l1_descriptor_addr = ((ttbr1 >> 14) << 14) | ((vaddr >> 20) << 2);
    MMUTRC(vaddr,
           printf("ARM_MMU::PAGE_TABLE_WALK() -> N=%d, TTBR1=0x%x, L1 descriptor address=0x%x\n",
                  N, ttbr1, l1_descriptor_addr));

  } 
  else {
    // use TTBR0
    u32_t ttbr0;
    cp15_op(2, 0, ttbr0, 0, 0, true);
    u4_t shift = 14 - N;
    // {TTBR0[31:14-N], MVA[31-N:20], 00}
    l1_descriptor_addr = ((ttbr0 >> shift) << shift) | ((((vaddr << N) >> N) >> 20) << 2);
    MMUTRC(vaddr,
           printf("ARM_MMU::PAGE_TABLE_WALK() -> N=%d, TTBR0=0x%x, L1 descriptor address=0x%x\n",
                  N, ttbr0, l1_descriptor_addr));

  }
  return l1_descriptor(l1_descriptor_addr, vaddr, entry); 
}

arm_mmu::faults arm_mmu::l1_descriptor(
    u32_t l1_descriptor_addr, 
    u32_t vaddr, 
    tlb_entry& entry) {
    // fetch data from external memory
    MMUTRC(vaddr,
           printf("ARM_MMU::L1_DESCRIPTOR() -> access to external memory at 0x%x for address 0x%x\n",
                  l1_descriptor_addr, vaddr));
    u32_t content = external_memory_access(l1_descriptor_addr);
    MMUTRC(vaddr,
           printf("ARM_MMU::L1_DESCRIPTOR() -> value returned from external memory = 0x%x for address 0x%x\n",
                  content, l1_descriptor_addr));

    // lower 2 bits indicate what type of access it is
    l1_access_type ty = (l1_access_type)(content & 0x3);
    // bits from 5 to 8 are domain bits
    u32_t domain_selector = (content >> 5) & 0xF;
    // get domain register
    u32_t domains;
    cp15_op(3, 0, domains, 0, 0, true);
    u2_t domain = (domains >> (domain_selector*2)) & 3;
    u8_t access = (content >> 10) & 0x3;
    switch(ty) {

      case L1TranslationFault:
      case ReservedTranslationFault: 
        {
            MMUTRC(vaddr,
                   printf("ARM_MMU::L1_DESCRIPTOR() -> FAULT\n"));
            return TranslationSection;
        }

      case PageTable: 
        {
            u32_t l2_descriptor_addr = 
                ((content >> 10) << 10) | (((vaddr << 12) >> 24) << 2);
            entry.domain = domain;
            MMUTRC(vaddr,
                   printf("ARM_MMU::L1_DESCRIPTOR() -> calling L2_DESCRIPTOR() with address 0x%x\n",
                          l2_descriptor_addr));
            return l2_descriptor(l2_descriptor_addr, vaddr, entry);
            break;
        }

      case Section: 
        {
            // This assumes that section is 1MB
            // TODO: mp to find out what check
            // needs to be performed to find out
            // if it is a supersection (16MB)
            
            // TODO: perform memory access control
            u32_t paddr = ((content >> 20) << 20) | (vaddr & ((1 << 20) - 1));
            
            MMUTRC(vaddr,
                   printf("ARM_MMU::L1_DESCRIPTOR() -> physical address 0x%x for virtual address 0x%x -- section 1MB\n", 
                          paddr, vaddr));

            entry.domain = domain;
            entry.access = access;
            
	    entry.size = (1 << 20); // 1MB
            entry.bits = 20; // number of bits
	    
	    entry.vpn = (vaddr >> 20);
            entry.pfn = (paddr >> 20);
            break; 
        }
    }
    
    return NoFault;
}

u32_t arm_mmu::external_memory_access(
    u32_t addr) {

    PW_TLM_PAYTYPE trans;
    trans.set_byte_enable_length(0);
    trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    trans.set_read();
    trans.set_address(addr & ~7LLU);
    trans.set_data_length(8);
    u64_t data64;
    trans.set_data_ptr((unsigned char*) &data64);

    sc_time delay = SC_ZERO_TIME;
    // To access external memory we are always going via 
    // the second connection which is either data cache
    // or SCU
    memories_initiator[3]->b_transport(trans, delay);

    assert(!trans.is_response_error());

    u32_t data32 = data64 >> (8 * (addr & 4));
    return data32;
}

arm_mmu::faults arm_mmu::l2_descriptor(
    u32_t l2_descriptor_addr, 
    u32_t vaddr, 
    tlb_entry& entry) {
    MMUTRC(vaddr,
           printf("ARM_MMU::L2_DESCRIPTOR() -> access to external memory at 0x%x for address 0x%x\n",
                  l2_descriptor_addr, vaddr));
    u32_t content = external_memory_access(l2_descriptor_addr);
    MMUTRC(vaddr,
           printf("ARM_MMU::L2_DESCRIPTOR() -> value returned from external memory = 0x%x for address 0x%x\n",
                  content, l2_descriptor_addr));


    // lower 2 bits indicate what type of access it is
    l2_access_type ty = (l2_access_type)(content & 0x3);
    // access permision bits
    // for what we have noticed here if 10th bit is set
    // than this page is read only
    u2_t access = (content >> 9) & 0x1;
    u32_t paddr = 0;
    switch(ty) {
      case L2TranslationFault: 
        {
            MMUTRC(vaddr,
                   printf("ARM_MMU::L2_DESCRIPTOR() -> FAULT\n"));

            return TranslationPage;
        }
          
      case LargePage: 
        {
            paddr = 
                ((content >> 16) << 16) |
                ((vaddr << 16) >> 16);

            MMUTRC(vaddr,
                   printf("ARM_MMU::L2_DESCRIPTOR() -> physical address 0x%x for virtual address 0x%x -- large page 64Kb\n", 
                          paddr, vaddr));
	    
            assert(0);
	    entry.size = (1 << 16); // 64Kb
	    entry.bits = 16; // number of bits
	    entry.pfn = (paddr >> 16);
	    entry.vpn = (vaddr >> 16);

            break;
        }
        
      case SmallPage: 
      case ExtendedSmallPage: 
        {
            paddr = 
                ((content >> 12) << 12) |
                ((vaddr << 20) >> 20);

            MMUTRC(vaddr,
                   printf("ARM_MMU::L2_DESCRIPTOR() -> physical address 0x%x for virtual address 0x%x -- small page 4Kb\n", 
                          paddr, vaddr));

	    entry.size = (1 << 12); // 4Kb
	    entry.bits = 12; // number of bits	
	    entry.pfn = (paddr >> 12);
	    entry.vpn = (vaddr >> 12);
            entry.access = access;
    
            break;
        }
    }
    
    return NoFault;
}

bool arm_mmu::cp15_op(u4_t cpr, u4_t crm, u32_t& data, u3_t op1, u3_t op2, bool read) {
    cp_request_extension ext;
    ext.cpr = cpr;
    ext.crm = crm;
    ext.op1 = op1;
    ext.op2 = op2;      
    
    PW_TLM_PAYTYPE trans;
    trans.set_extension(&ext);
    trans.set_byte_enable_length(0);
    trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    if(read)
        trans.set_read();
    else
        trans.set_write();
    trans.set_address(0);
    trans.set_data_length(4);
    trans.set_data_ptr((unsigned char*) &data);

    sc_time delay = SC_ZERO_TIME;
    cp15_initiator->b_transport(trans, delay);

    trans.clear_extension<cp_request_extension>(&ext);

    assert(!trans.is_response_error());

    return true;
}

void arm_mmu::stat_report(FILE* fd, bool reset) {
  if (!ever_enabled)
    {
      fprintf(fd, "\nMemory management unit (MMU) %s: never turned on\n", name());
      return;
}
    if(fd) {
        fprintf(fd, "\nMemory management unit (MMU) id: %s\n", name());
        fprintf(fd, "----------------------------------\n");

        if(m_stats.collecting) {
            fprintf(fd, "\nTranslation lookaside buffer (TLB)\n");
            fprintf(fd, "----------------------------------\n");
            fprintf(fd, "Size: %d\n", size);
            fprintf(fd, "Invalidations: %d\n", m_stats.tlb_invalid);
            fprintf(fd, "--------------\n");
            fprintf(fd, "       |         Hit        Miss        Insert          Total\n");
            fprintf(fd, "-------|-----------------------------------------------------\n");
            fprintf(fd, "Reads  |%12d%12d%14d%15d\n", 
                    m_stats.tlb_hit[0], 
                    m_stats.tlb_miss[0], 
                    m_stats.tlb_insert[0],
                    m_stats.tlb_hit[0]+m_stats.tlb_miss[0]+m_stats.tlb_insert[0]);
            fprintf(fd, "Writes |%12d%12d%14d%15d\n", 
                    m_stats.tlb_hit[1], 
                    m_stats.tlb_miss[1], 
                    m_stats.tlb_insert[1],
                    m_stats.tlb_hit[1]+m_stats.tlb_miss[1]+m_stats.tlb_insert[1]);
            fprintf(fd, "Instrs |%12d%12d%14d%15d\n", 
                    m_stats.tlb_hit[2], 
                    m_stats.tlb_miss[2], 
                    m_stats.tlb_insert[2],
                    m_stats.tlb_hit[2]+m_stats.tlb_miss[1]+m_stats.tlb_insert[2]);
            fprintf(fd, "-------|-----------------------------------------------------\n");
            fprintf(fd, "Totals |%12d%12d%14d%15d\n",
                    m_stats.tlb_hit[0]+m_stats.tlb_hit[1]+m_stats.tlb_hit[2],
                    m_stats.tlb_miss[0]+m_stats.tlb_miss[1]+m_stats.tlb_miss[2],
                    m_stats.tlb_insert[0]+m_stats.tlb_insert[1]+m_stats.tlb_insert[2],
                    m_stats.tlb_hit[0]+m_stats.tlb_hit[1]+m_stats.tlb_hit[2]+
                    m_stats.tlb_miss[0]+m_stats.tlb_miss[1]+m_stats.tlb_miss[2]+
                    m_stats.tlb_insert[0]+m_stats.tlb_insert[1]+m_stats.tlb_insert[2]);

            fprintf(fd, "\nFaults\n");
            fprintf(fd, "------\n");
            fprintf(fd, "Section fault permissions: %d\n", m_stats.section_fault_permission);
            fprintf(fd, "Page fault permissions: %d\n", m_stats.page_fault_permission);
            fprintf(fd, "-----------------------\n");
            fprintf(fd, "       | Section      Page      Total\n");
            fprintf(fd, "-------|-----------------------------\n");
            fprintf(fd, "Reads  |%8d%10d%11d\n",
                    m_stats.section_fault[0], 
                    m_stats.page_fault[0], 
                    m_stats.section_fault[0]+m_stats.page_fault[0]);
            fprintf(fd, "Writes |%8d%10d%11d\n",
                    m_stats.section_fault[1], 
                    m_stats.page_fault[1], 
                    m_stats.section_fault[1]+m_stats.page_fault[1]);
            fprintf(fd, "Instrs |%8d%10d%11d\n",
                    m_stats.section_fault[2], 
                    m_stats.page_fault[2], 
                    m_stats.section_fault[2]+m_stats.page_fault[2]);
            fprintf(fd, "-------|-----------------------------\n");
            fprintf(fd, "Totals |%8d%10d%11d\n",
                    m_stats.section_fault[0]+m_stats.section_fault[1]+m_stats.section_fault[2], 
                    m_stats.page_fault[0]+m_stats.page_fault[1]+m_stats.page_fault[2], 
                    m_stats.section_fault[0]+m_stats.section_fault[1]+m_stats.section_fault[2]+ 
                    m_stats.page_fault[0]+m_stats.page_fault[1]+m_stats.page_fault[2]);
        }
        else {
            fprintf(fd, "No statistics was collected for this module!\n");
        }
    }        

    if(reset)
        m_stats.reset();
}
