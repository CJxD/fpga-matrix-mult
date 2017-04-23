// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2014 M Puzovic + D J Greaves

#ifndef __BASE_MMU_TLM_H__
#define __BASE_MMU_TLM_H__

#include "systemc.h"

#include "tlm.h"
#include "tlm_utils/multi_passthrough_initiator_socket.h"
#include "tlm_utils/multi_passthrough_target_socket.h"
#include "prazor.h"
#include "tenos.h"

#include <string>
#include "base_mmu.h"


class base_mmu_tlm : public sc_module 
#ifdef TLM_POWER3
, public pw_module
#endif
{
  public:

    base_mmu_tlm();
    base_mmu_tlm(
        sc_core::sc_module_name name, 
        base_mmu* mmu);

    void b_transport(int id, PRAZOR_GP_T &trans, sc_time &delay);

  protected:
    traceregion* traceregions;
    
    tlm_utils::multi_passthrough_target_socket<base_mmu_tlm, 64, PW_TLM_TYPES> targ_socket;
    tlm_utils::multi_passthrough_initiator_socket<base_mmu_tlm, 64, PW_TLM_TYPES> init_socket;

  private:
    base_mmu* mmu;
};

#endif
