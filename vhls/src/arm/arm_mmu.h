// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2014 M Puzovic + D J Greaves

#ifndef __ARM_MMU_H__
#define __ARM_MMU_H__

#include "base_mmu.h"

#include "prazor.h"
#include "tenos.h"
#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/multi_passthrough_initiator_socket.h"
#include "tlm_utils/multi_passthrough_target_socket.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"


class armisa;

// describes entry in the TLB
struct tlb_entry {

  public:
    // default constructor
    tlb_entry() :
        pfn(0), size(0), vpn(0), asid(0), global(false), valid(true) {}

    void update_vpn(u32_t new_vpn) {
        vpn = new_vpn >> bits;
    }

    u32_t page_start() const {
        return pfn << bits;
    }

    bool match(u32_t vaddr, u8_t context) {
        u32_t v = vpn << bits;

        return valid 
            && asid == context 
            && vaddr >= v 
	    && vaddr < v + size;
    }

    u32_t get_pfn(u32_t vaddr) {
      return (pfn << bits) | (vaddr & (size-1));
    }

    bool is_valid() {
      return valid;
    }

    u32_t get_vpn() {
        return vpn << bits;
    }

    u8_t get_asid() {
        return asid;
    }

    
    std::string print() const {
        std::ostringstream stream;
        stream << "#" << hex << (vpn << bits) << " ppn #" << hex << (pfn << bits) << " size:" << size << " g:" << global;
        return stream.str();      
    }

    u32_t pfn; // physical frame number
    u32_t vpn; // virtual page number
    u32_t size; // size of the entry == number of bytes in a page
    u8_t bits; // number of bits in a page

    u8_t asid; // Application Space IDentifier
    bool global; // is entry global?
    bool valid; // is entry valid?    

    u2_t access; // access permision bits
    u4_t domain; // domain
};


class arm_mmu : public base_mmu, public sc_module
#ifdef TLM_POWER3
, public pw_module
#endif
{

  bool ever_enabled;
  public:    
    enum l1_access_type { 
        L1TranslationFault = 0, 
	PageTable,
	Section,
        ReservedTranslationFault
    };

    enum l2_access_type {
        L2TranslationFault = 0,
        LargePage, 
        SmallPage,
        ExtendedSmallPage 
    };

    // TODO: add more (table 6-11 on pp 6-34
    // in ARM DDI 0333H if we decide to
    // support TrustZone and external aborts
    enum faults {
        NoFault, 
        TLBMiss,
        TranslationSection,
        TranslationPage
    };

    arm_mmu(sc_module_name name, u32_t size);
    virtual ~arm_mmu();

    tlb_entry* lookup(u32_t vaddr, u8_t asid);
    void insert(u32_t addr, tlb_entry& entry);
    void print_tlb() const;

    faults page_table_walk(u32_t vaddr, tlb_entry& entry);
    faults l1_descriptor(u32_t l1_descriptor_addr, u32_t vaddr, tlb_entry& entry);
    faults l2_descriptor(u32_t l2_descriptor_addr, u32_t vaddr, tlb_entry& entry);
    u32_t external_memory_access(u32_t addr);

    virtual bool translate(
        u32_t vaddr, 
        u32_t& paddr, 
        base_mmu::access_type mode);


    virtual void stat_report(
        FILE* fd, 
        bool reset = false);

    tlm_utils::simple_target_socket<arm_mmu, 64, PW_TLM_TYPES> cp15_target;
    tlm_utils::multi_passthrough_target_socket<arm_mmu, 64, PW_TLM_TYPES> cores_target;
    tlm_utils::simple_initiator_socket<arm_mmu, 64, PW_TLM_TYPES> cp15_initiator;
    tlm_utils::multi_passthrough_initiator_socket<arm_mmu, 64, PW_TLM_TYPES> memories_initiator;

    void b_cp15_access(PRAZOR_GP_T& trans, sc_time &delay);
    void b_core_access(int id, PRAZOR_GP_T &trans, sc_time &delay);

    bool cp15_op(u4_t cpr, u4_t crm, u32_t& data, u3_t op1, u3_t op2, bool read);


    // disable mmu
    void disable() {
        m_active = false;
    }

    // enable mmu
    void enable() {
      ever_enabled = true;
        m_active = true;
    }
    
    // query mmu state
    bool is_active() {
        return m_active;
    }


  private:
      typedef struct stats {
        // Statistics when accessing
        // transaction lookaside buffer
        // Index 0 - Read operation
        // Index 1 - Write operation
        // Index 2 - Instruction
        u32_t tlb_hit[3]; // entry found in TLB
        u32_t tlb_miss[3]; // entry wasn't found in TLB
        u32_t tlb_insert[3]; // entry inserted in TLB
        u32_t tlb_invalid;  // entry invalidated form TLB

        // Statistics when page fault happened
        // This is equivalent in some nomenclatures as
        // second level page walk fault
        // Index 0 - Read operation
        // Index 1 - Write operation
        // Index 2 - Instruction
        u32_t page_fault[3]; // page fault
        u32_t page_fault_permission; // permission page fault

        // Statistics when section fault happened
        // This is equivalent in some nomenclatures as
        // first level page fault
        // Index 0 - Read operation
        // Index 1 - Write operation
        // Index 2 - Instruction
        u32_t section_fault[3]; // section fault
        u32_t section_fault_permission; // section fault on permission

        bool collecting;

        stats() { 
            collecting = false;
            reset(); 
        }

        void reset() {
            for(int i = 0; i < 3; i++) {
                tlb_hit[i] = 0;
                tlb_miss[i] = 0;
                tlb_insert[i] = 0;

                page_fault[i] = 0;

                section_fault[i] = 0;
            }

            tlb_invalid = 0;
            page_fault_permission = 0;
            section_fault_permission = 0;
	}
      } stats_t;

      stats_t m_stats;

      bool m_active;
      tlb_entry* table; // the page table

      // used to describe TLB entry
      u32_t size;
};

#endif
