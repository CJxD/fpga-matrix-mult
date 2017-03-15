// Verilated -*- C++ -*-
#ifndef _Vor1200_top_H_
#define _Vor1200_top_H_

#include "verilated.h"
#include "Vor1200_top__Inlines.h"
class Vor1200_top__Syms;
class Vor1200_top_or1200_top;

//----------

VL_MODULE(Vor1200_top) {
  public:
    // CELLS
    Vor1200_top_or1200_top*	v;
    
    // PORTS
    VL_IN8(clk_i,0,0);
    VL_IN8(rst_i,0,0);
    VL_IN8(iwb_clk_i,0,0);
    VL_IN8(iwb_rst_i,0,0);
    VL_IN8(dwb_clk_i,0,0);
    VL_IN8(dwb_rst_i,0,0);
    VL_IN8(clmode_i,1,0);
    VL_IN8(iwb_ack_i,0,0);
    VL_IN8(iwb_err_i,0,0);
    VL_IN8(iwb_rty_i,0,0);
    VL_OUT8(iwb_cyc_o,0,0);
    VL_OUT8(iwb_stb_o,0,0);
    VL_OUT8(iwb_we_o,0,0);
    VL_OUT8(iwb_sel_o,3,0);
    VL_OUT8(iwb_cab_o,0,0);
    VL_IN8(dwb_ack_i,0,0);
    VL_IN8(dwb_err_i,0,0);
    VL_IN8(dwb_rty_i,0,0);
    VL_OUT8(dwb_cyc_o,0,0);
    VL_OUT8(dwb_stb_o,0,0);
    VL_OUT8(dwb_we_o,0,0);
    VL_OUT8(dwb_sel_o,3,0);
    VL_OUT8(dwb_cab_o,0,0);
    VL_IN8(dbg_stall_i,0,0);
    VL_IN8(dbg_ewt_i,0,0);
    VL_OUT8(dbg_lss_o,3,0);
    VL_OUT8(dbg_is_o,1,0);
    VL_OUT8(dbg_bp_o,0,0);
    VL_IN8(dbg_stb_i,0,0);
    VL_IN8(dbg_we_i,0,0);
    VL_OUT8(dbg_ack_o,0,0);
    VL_IN8(pm_cpustall_i,0,0);
    VL_OUT8(pm_clksd_o,3,0);
    VL_OUT8(pm_dc_gate_o,0,0);
    VL_OUT8(pm_ic_gate_o,0,0);
    VL_OUT8(pm_dmmu_gate_o,0,0);
    VL_OUT8(pm_immu_gate_o,0,0);
    VL_OUT8(pm_tt_gate_o,0,0);
    VL_OUT8(pm_cpu_gate_o,0,0);
    VL_OUT8(pm_wakeup_o,0,0);
    VL_OUT8(pm_lvolt_o,0,0);
    char	__VpadToAlign41[1];
    VL_OUT16(dbg_wp_o,10,0);
    VL_IN(pic_ints_i,19,0);
    VL_IN(iwb_dat_i,31,0);
    VL_OUT(iwb_adr_o,31,0);
    VL_OUT(iwb_dat_o,31,0);
    VL_IN(dwb_dat_i,31,0);
    VL_OUT(dwb_adr_o,31,0);
    VL_OUT(dwb_dat_o,31,0);
    VL_IN(dbg_adr_i,31,0);
    VL_IN(dbg_dat_i,31,0);
    VL_OUT(dbg_dat_o,31,0);
    
    // LOCAL SIGNALS
    
    // LOCAL VARIABLES
    VL_SIG8(__Vclklast__TOP__dwb_clk_i,0,0);
    VL_SIG8(__Vclklast__TOP__dwb_rst_i,0,0);
    VL_SIG8(__Vclklast__TOP__iwb_clk_i,0,0);
    VL_SIG8(__Vclklast__TOP__iwb_rst_i,0,0);
    VL_SIG8(__Vclklast__TOP__clk_i,0,0);
    VL_SIG8(__Vclklast__TOP__rst_i,0,0);
    char	__VpadToAlign98[2];
    
    // INTERNAL VARIABLES
    Vor1200_top__Syms*	__VlSymsp;		// Symbol table
    
    // PARAMETERS
    
    // METHODS
  private:
    Vor1200_top& operator= (const Vor1200_top&);	///< Copying not allowed
    Vor1200_top(const Vor1200_top&);	///< Copying not allowed
  public:
    Vor1200_top(const char* name="TOP");
    ~Vor1200_top();
    void	__Vconfigure(Vor1200_top__Syms* symsp, bool first);
    
    // Sensitivity blocks
    void	final();	///< Function to call when simulation completed
    void	eval();	///< Main function to call from calling app when inputs change
  private:
    static void _eval_initial_loop(Vor1200_top__Syms* __restrict vlSymsp);
    static bool	_change_request(Vor1200_top__Syms* __restrict vlSymsp);
  public:
    static void	_combo__TOP__7(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_combo__TOP__9(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_eval(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_eval_initial(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_eval_settle(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_sequent__TOP__2(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_sequent__TOP__3(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_sequent__TOP__4(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_settle__TOP__1(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_settle__TOP__5(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_settle__TOP__6(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_settle__TOP__8(Vor1200_top__Syms* __restrict vlSymsp);
} VL_ATTR_ALIGNED(64);

#endif  /*guard*/
