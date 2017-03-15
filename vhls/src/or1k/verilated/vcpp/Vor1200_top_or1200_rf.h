// Verilated -*- C++ -*-
#ifndef _Vor1200_top_or1200_rf_H_
#define _Vor1200_top_or1200_rf_H_

#include "verilated.h"
#include "Vor1200_top__Inlines.h"
class Vor1200_top__Syms;
class Vor1200_top_or1200_rfram_generic;

//----------

VL_MODULE(Vor1200_top_or1200_rf) {
  public:
    // CELLS
    Vor1200_top_or1200_rfram_generic*	rf_a;
    
    // PORTS
    VL_IN8(__PVT__clk,0,0);
    VL_IN8(__PVT__rst,0,0);
    VL_IN8(__PVT__supv,0,0);
    VL_IN8(__PVT__wb_freeze,0,0);
    VL_IN8(__PVT__addrw,4,0);
    VL_IN8(__PVT__we,0,0);
    VL_IN8(__PVT__flushpipe,0,0);
    VL_IN8(__PVT__id_freeze,0,0);
    VL_IN8(__PVT__addra,4,0);
    VL_IN8(__PVT__addrb,4,0);
    VL_IN8(__PVT__rda,0,0);
    VL_IN8(__PVT__rdb,0,0);
    VL_IN8(__PVT__spr_cs,0,0);
    VL_IN8(__PVT__spr_write,0,0);
    char	__VpadToAlign14[2];
    VL_IN(__PVT__dataw,31,0);
    VL_OUT(__PVT__dataa,31,0);
    VL_OUT(__PVT__datab,31,0);
    VL_IN(__PVT__spr_addr,31,0);
    VL_IN(__PVT__spr_dat_i,31,0);
    VL_OUT(__PVT__spr_dat_o,31,0);
    
    // LOCAL SIGNALS
    VL_SIG8(__PVT__rf_addrw,4,0);
    VL_SIG8(__PVT__rf_we,0,0);
    VL_SIG8(__PVT__spr_valid,0,0);
    VL_SIG8(__PVT__rf_we_allow,0,0);
    VL_SIG64(__PVT__dataa_saved,32,0);
    VL_SIG64(__PVT__datab_saved,32,0);
    
    // LOCAL VARIABLES
    char	__VpadToAlign68[4];
    VL_SIG64(__Vdly__dataa_saved,32,0);
    VL_SIG64(__Vdly__datab_saved,32,0);
    
    // INTERNAL VARIABLES
  private:
    char	__VpadToAlign92[4];
    Vor1200_top__Syms*	__VlSymsp;		// Symbol table
  public:
    
    // PARAMETERS
    
    // METHODS
  private:
    Vor1200_top_or1200_rf& operator= (const Vor1200_top_or1200_rf&);	///< Copying not allowed
    Vor1200_top_or1200_rf(const Vor1200_top_or1200_rf&);	///< Copying not allowed
  public:
    Vor1200_top_or1200_rf(const char* name="TOP");
    ~Vor1200_top_or1200_rf();
    void	__Vconfigure(Vor1200_top__Syms* symsp, bool first);
    
    // Sensitivity blocks
    static void	_combo__TOP__v__or1200_cpu__or1200_rf__3(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_combo__TOP__v__or1200_cpu__or1200_rf__5(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_sequent__TOP__v__or1200_cpu__or1200_rf__1(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_settle__TOP__v__or1200_cpu__or1200_rf__2(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_settle__TOP__v__or1200_cpu__or1200_rf__4(Vor1200_top__Syms* __restrict vlSymsp);
} VL_ATTR_ALIGNED(64);

#endif  /*guard*/
