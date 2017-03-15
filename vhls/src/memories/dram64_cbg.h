#ifndef CBG_DRAM_H
#define CBG_DRAM_H

// $Id: dram64_cbg.h,v 1.12 2011/06/30 12:07:08 djg11 Exp $
//
// dram64 : A TLM static ram with with ELF and Intel Hex load facility.
// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2009-10 DJ Greaves (TLM 2.0 Version Arturs Prieditis). 
// (C) 2009-11 DJ Greaves
// University of Cambridge, Computer Laboratory.
// ACS P35 SoC D/M Classes - $Id: dram64_cbg.h,v 1.12 2011/06/30 12:07:08 djg11 Exp $



// This DRAM model is not synchronous (TLM modelling commonly neglects the clock), although modern DRAMs are invariably so, with a fully-pipelined access latency of one or two clock cycles, depending on size.
// Strictly modelling synchronous DRAM is not appropriate for LT-TLM models, but the combined latency figure attributed to the bus and RAM should be the clock period for single issue operation (and half as much for dual issue if such a concept exists, etc).

#include "generic_tlm_mem.h"

class dram64_cbg: public generic_tlm_mem
{

 public:
  // Constructor
  dram64_cbg(sc_core::sc_module_name name, u64_t mem_base, u64_t mem_size, u32_t width);
  void recompute_dram_pvt_parameters();
  void recompute_pvt_parameters() // Do not rename this method: it is an override called by TLM POWER3 when Vcc is changed.
  {   recompute_dram_pvt_parameters(); 
  }
  virtual const char *kind();

};

#endif


