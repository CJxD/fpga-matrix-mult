// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2014-15 M Puzovic + D J Greaves

#ifndef __BASE_MMU_H__
#define __BASE_MMU_H__

#include "tenos.h"
#include "systemc.h"

#include <string>


#define MMUTRC(ADDR, X)\
{\
  if(traceregions && traceregions->check(ADDR, TENOS_TRACE_MEM_WRITE | TENOS_TRACE_MEM_READ)) {\
    cout << sc_time_stamp() << " - ";\
    printf("tid=%d-MMU (0x%x): ", sc_get_current_process_b()->proc_id, ADDR);\
    X;\
  }\
}

class base_mmu 
{
  public:

    /* Type of the access */
    enum access_type { Read, Write, Execute };

    base_mmu() :
        traceregions(NULL) {}

    virtual bool translate(
        u32_t vaddr, 
        u32_t& paddr,
        access_type mode) = 0;

    virtual void stat_report(FILE* fd, bool reset = false) = 0;

  protected:
    traceregion* traceregions;
};

#endif
