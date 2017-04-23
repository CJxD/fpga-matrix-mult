// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2014 M Puzovic + D J Greaves

#include "base_mmu_tlm.h"

using namespace sc_core;
using namespace tlm;

base_mmu_tlm::base_mmu_tlm() :
    sc_module(""), 
    mmu(NULL),
    traceregions(NULL)
#ifdef TLM_POWER3
    , pw_module()
#endif
{}

base_mmu_tlm::base_mmu_tlm(
    sc_module_name name,
    base_mmu* _mmu) :
    sc_module(name),
    mmu(_mmu), 
    traceregions(NULL)
#ifdef TLM_POWER3
    , pw_module()
#endif
{
    targ_socket.register_b_transport(this, &base_mmu_tlm::b_transport);
}

// TLM-2 blocking transport method
void base_mmu_tlm::b_transport(int id, PRAZOR_GP_T &trans, sc_time &delay)
{
    u32_t vaddr = trans.get_address();        
    u32_t paddr = -1;

    tlm_command cmd = trans.get_command();
    base_mmu::access_type mode = base_mmu::Execute;
    if(cmd == TLM_READ_COMMAND) 
        mode = base_mmu::Read;
    else if(cmd = TLM_WRITE_COMMAND)
        mode = base_mmu::Write;

    bool success = mmu->translate(vaddr, paddr, mode);


    if(success)
        trans.set_address(paddr);

    trans.set_response_status(TLM_OK_RESPONSE);
}
