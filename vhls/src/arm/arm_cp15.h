// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2014 M Puzovic + D J Greaves

#ifndef __ARM_CP15__
#define __ARM_CP15__

// ARM coprocessor number 15

#include "prazor.h"
#include "tenos.h"
#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/multi_passthrough_initiator_socket.h"
#include "tlm_utils/multi_passthrough_target_socket.h"


class armcore_tlm;

struct cp15_cache_control_extension : tlm::tlm_extension<cp15_cache_control_extension> {
  public:
    enum operation {
        Invalid,
        Clean,
        Enable
    };

    enum upto {
        None,
        PoC,
        PoU
    };

    u3_t types;
    u1_t domain;
    u1_t shareable;
    u32_t data;
    u3_t op;
    u2_t stop;
    u1_t address;
    u1_t snooped;

    cp15_cache_control_extension() {
        types = -1;
        domain = -1;
        shareable = -1;
        data = -1;
        op = -1;
        stop = None;
        address = 1;
        snooped = 0;
    }

    tlm_extension_base* clone() const {
        cp15_cache_control_extension* ext = new cp15_cache_control_extension;
        ext->types = this->types;
        ext->domain = this->domain;
        ext->shareable = this->shareable;
        ext->data = this->data;
        ext->op = this->op;
        ext->stop = this->stop;
        ext->address = this->address;
        ext->snooped = this->snooped;

        return ext;
    }

    void copy_from(tlm_extension_base const &ext) {
        types = static_cast<cp15_cache_control_extension const&>(ext).types;
        domain = static_cast<cp15_cache_control_extension const&>(ext).domain;
        shareable = static_cast<cp15_cache_control_extension const&>(ext).shareable;
        data = static_cast<cp15_cache_control_extension const&>(ext).data;
        op = static_cast<cp15_cache_control_extension const&>(ext).op;
        stop = static_cast<cp15_cache_control_extension const&>(ext).stop;
        address = static_cast<cp15_cache_control_extension const&>(ext).address;
        snooped = static_cast<cp15_cache_control_extension const&>(ext).snooped;
    }
};

struct cp15_mmu_control_extension : tlm::tlm_extension<cp15_mmu_control_extension> {
  public:
    enum operation {
        InvalidAll,
        InvalidByMVA,
        InvalidByASID,
        Enable,
        Translate,
        UNKNOWN_OP
    };

    enum type {
        Instruction, 
        Data,
        Unified,
        UNKNOWN_TYPE
    };

    u32_t data;
    operation op;
    type ty;

    cp15_mmu_control_extension() {
        data = -1;
        op = UNKNOWN_OP;
        ty = UNKNOWN_TYPE;
    }

    tlm_extension_base* clone() const {
        cp15_mmu_control_extension* ext = new cp15_mmu_control_extension;
        ext->data = this->data;
        ext->op = this->op;
        ext->ty = this->ty;

        return ext;
    }

    void copy_from(tlm_extension_base const &ext) {
        data = static_cast<cp15_mmu_control_extension const&>(ext).data;
        op = static_cast<cp15_mmu_control_extension const&>(ext).op;
        ty = static_cast<cp15_mmu_control_extension const&>(ext).ty;
        
    }
};

class arm_cp15 : public sc_module 
#ifdef TLM_POWER3
, public pw_module
#endif
{

 public:
    // Constructor
    arm_cp15(sc_module_name, u8_t core_no);

    tlm_utils::multi_passthrough_target_socket<arm_cp15, 64, PW_TLM_TYPES> target_socket;
    tlm_utils::multi_passthrough_initiator_socket<arm_cp15, 64, PW_TLM_TYPES> initiator_socket;

    void b_access(int id, PRAZOR_GP_T &trans, sc_time &delay);

 private:
    // Read 
    u32_t read(int coreid, u4_t cpr, u4_t crm, u3_t op1, u3_t op2);
    // Write
    void write(u4_t cpr, u4_t crm, u32_t data, u3_t op1, u3_t op2);
    // Cache control messages
    void cache_control_msg(u3_t cache, u1_t domain, u1_t shareable, u32_t data, u3_t op, u2_t stop, u1_t address);
    // MMU control messages
    void mmu_control_msg(
        cp15_mmu_control_extension::operation op, 
        cp15_mmu_control_extension::type ty,
        u32_t& data);

    FILE* logfd;

    u8_t m_core_no;

    // TODO: Consider whether we should put
    // this into one contiguous memory block
    // and then access it using offsets instead
    // of having large number of private variables

    // CR=0
    // CRm=0
    // opc1=2
    // opc2=0
    u32_t csselr;

    // CRn=0
    // CRm=1
    // opc1=0
    // opc2=7
    u32_t id_mmfr3;

    // CRn=1
    // CRm=0
    // opc1=0
    // opc2=0
    u32_t sctlr;
    // opc2=1;
    u32_t actlr;
    // opc2=2
    u32_t cpacr;
    
    // CRn=2
    // CRm=0
    // opc1=0
    // opc2=0
    u32_t ttbr0;
    // opc2=1
    u32_t ttbr1;
    // opc2=2
    u32_t ttbcr;
    
    // CRn=3
    // CRm=0
    // opc1=0
    // opc2=0
    u32_t dacr;
    
    // CRn=5
    // Cm=0
    // opc1=0
    // opc2=0
    u32_t dfsr;
    // opc2=1
    u32_t ifsr;
    
    // CRn=6
    // Cm=0
    // opc1=0
    // opc2=0
    u32_t dfar;
    // opc2=2
    u32_t ifar;
    
    // CRn=7
    // Crm=1
    // opc1=0
    // opc2=0
    u32_t icialluis;
    // opc2=6
    u32_t bpiallis;
    // CRm=5
    // opc1=0
    // opc2=0
    u32_t iciallu;
    // opc2=1
    u32_t icimvau;
    // opc2=6
    u32_t bpiall;

    // Crm==6
    // opc1=0
    // opc2=1
    u32_t dcimvac;
    // opc2=2
    u32_t dcisw;
    
    
    // Crm=10
    // opc1=0
    // opc2=1
    u32_t dccmvac;
    // Crm=11
    // opc1=0
    // opc2=1
    u32_t dccmvau;
    // Crm=14
    // opc1=0
    // opc2=1
    u32_t dccimvac;
    // opc1=0
    // opc2=2
    u32_t dccisw;
    
    
    // CRn=8
    // CRm=3
    // opc1=0
    // opc2=1
    u32_t tlbimvais;
    // opc2=2
    u32_t tlbiasidis;
    // CRm=7
    // opc1=0
    // opc2=0
    u32_t tlbiall;
    
    // CRn=10
    // CRm=2
    // opc1=0
    // opc2=0
    u32_t prrr_mair0;
    // opc2=1
    u32_t nmrr_mair1;
    
    // CRn=13
    // Crm=0
    // opc1=0
    // opc2=1
    u32_t contextidr;
    // opc2=2
    u32_t tpidrurw;
    // opc2=3
    u32_t tpidruro;
    // opc2=4
    u32_t tpidrprw;


    u32_t arch_specific_clock_gating; // cpr=15, crm=0, op1=0, op2=0.
};
    
#endif
