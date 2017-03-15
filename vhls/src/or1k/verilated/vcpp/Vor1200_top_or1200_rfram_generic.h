// Verilated -*- C++ -*-
#ifndef _Vor1200_top_or1200_rfram_generic_H_
#define _Vor1200_top_or1200_rfram_generic_H_

#include "verilated.h"
#include "Vor1200_top__Inlines.h"
class Vor1200_top__Syms;

//----------

VL_MODULE(Vor1200_top_or1200_rfram_generic) {
  public:
    // CELLS
    
    // PORTS
    VL_IN8(__PVT__clk,0,0);
    VL_IN8(__PVT__rst,0,0);
    VL_IN8(__PVT__ce_a,0,0);
    VL_IN8(__PVT__addr_a,4,0);
    VL_IN8(__PVT__ce_b,0,0);
    VL_IN8(__PVT__addr_b,4,0);
    VL_IN8(__PVT__ce_w,0,0);
    VL_IN8(__PVT__we_w,0,0);
    VL_IN8(__PVT__addr_w,4,0);
    char	__VpadToAlign9[3];
    VL_OUT(__PVT__do_a,31,0);
    VL_OUT(__PVT__do_b,31,0);
    VL_IN(__PVT__di_w,31,0);
    
    // LOCAL SIGNALS
    VL_SIG8(__PVT__intaddr_a,4,0);
    VL_SIG8(__PVT__intaddr_b,4,0);
    char	__VpadToAlign30[2];
    VL_SIGW(__PVT__mem,1023,0,32);
    
    // LOCAL VARIABLES
    
    // INTERNAL VARIABLES
  private:
    char	__VpadToAlign44[4];
    Vor1200_top__Syms*	__VlSymsp;		// Symbol table
  public:
    
    // PARAMETERS
    
    // METHODS
  private:
    Vor1200_top_or1200_rfram_generic& operator= (const Vor1200_top_or1200_rfram_generic&);	///< Copying not allowed
    Vor1200_top_or1200_rfram_generic(const Vor1200_top_or1200_rfram_generic&);	///< Copying not allowed
  public:
    Vor1200_top_or1200_rfram_generic(const char* name="TOP");
    ~Vor1200_top_or1200_rfram_generic();
    void	__Vconfigure(Vor1200_top__Syms* symsp, bool first);
    
    // Sensitivity blocks
    static void	_sequent__TOP__v__or1200_cpu__or1200_rf__rf_a__1(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_settle__TOP__v__or1200_cpu__or1200_rf__rf_a__2(Vor1200_top__Syms* __restrict vlSymsp);
    uint32_t	get_gpr(uint32_t __PVT__gpr_no);
    uint32_t	get_pc();
} VL_ATTR_ALIGNED(64);

#endif  /*guard*/
