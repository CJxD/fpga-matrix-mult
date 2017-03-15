// Verilated -*- C++ -*-
#ifndef _Vor1200_top_or1200_ctrl_H_
#define _Vor1200_top_or1200_ctrl_H_

#include "verilated.h"
#include "Vor1200_top__Inlines.h"
class Vor1200_top__Syms;

//----------

VL_MODULE(Vor1200_top_or1200_ctrl) {
  public:
    // CELLS
    
    // PORTS
    VL_IN8(__PVT__clk,0,0);
    VL_IN8(__PVT__rst,0,0);
    VL_IN8(__PVT__id_freeze,0,0);
    VL_IN8(__PVT__ex_freeze,0,0);
    VL_IN8(wb_freeze,0,0);
    VL_IN8(__PVT__flushpipe,0,0);
    VL_OUT8(__PVT__branch_op,2,0);
    VL_IN8(__PVT__branch_taken,0,0);
    VL_OUT8(__PVT__rf_addrw,4,0);
    VL_OUT8(__PVT__rf_addra,4,0);
    VL_OUT8(__PVT__rf_addrb,4,0);
    VL_OUT8(__PVT__rf_rda,0,0);
    VL_OUT8(__PVT__rf_rdb,0,0);
    VL_OUT8(__PVT__alu_op,3,0);
    VL_OUT8(__PVT__mac_op,1,0);
    VL_OUT8(__PVT__shrot_op,1,0);
    VL_OUT8(__PVT__rfwb_op,2,0);
    VL_OUT8(__PVT__sel_a,1,0);
    VL_OUT8(__PVT__sel_b,1,0);
    VL_OUT8(__PVT__lsu_op,3,0);
    VL_OUT8(__PVT__comp_op,3,0);
    VL_OUT8(__PVT__multicycle,1,0);
    VL_OUT8(__PVT__cust5_op,4,0);
    VL_OUT8(__PVT__cust5_limm,5,0);
    VL_IN8(__PVT__wbforw_valid,0,0);
    VL_IN8(__PVT__du_hwbkpt,0,0);
    VL_OUT8(__PVT__sig_syscall,0,0);
    VL_OUT8(__PVT__sig_trap,0,0);
    VL_OUT8(__PVT__force_dslot_fetch,0,0);
    VL_OUT8(__PVT__no_more_dslot,0,0);
    VL_OUT8(__PVT__ex_void,0,0);
    VL_OUT8(__PVT__id_macrc_op,0,0);
    VL_OUT8(__PVT__ex_macrc_op,0,0);
    VL_OUT8(__PVT__rfe,0,0);
    VL_OUT8(__PVT__except_illegal,0,0);
    char	__VpadToAlign35[1];
    VL_OUT16(__PVT__spr_addrimm,15,0);
    char	__VpadToAlign38[2];
    VL_IN(__PVT__if_insn,31,0);
    VL_OUT(__PVT__ex_insn,31,0);
    VL_OUT(__PVT__wb_insn,31,0);
    VL_OUT(__PVT__simm,31,0);
    VL_OUT(__PVT__branch_addrofs,31,2);
    VL_OUT(__PVT__lsu_addrofs,31,0);
    
    // LOCAL SIGNALS
    VL_SIG8(__PVT__pre_branch_op,2,0);
    VL_SIG8(__PVT__wb_rfaddrw,4,0);
    VL_SIG8(__PVT__sel_imm,0,0);
    char	__VpadToAlign71[1];
    VL_SIG(__PVT__id_insn,31,0);
    
    // LOCAL VARIABLES
    
    // INTERNAL VARIABLES
  private:
    char	__VpadToAlign84[4];
    Vor1200_top__Syms*	__VlSymsp;		// Symbol table
  public:
    
    // PARAMETERS
    
    // METHODS
  private:
    Vor1200_top_or1200_ctrl& operator= (const Vor1200_top_or1200_ctrl&);	///< Copying not allowed
    Vor1200_top_or1200_ctrl(const Vor1200_top_or1200_ctrl&);	///< Copying not allowed
  public:
    Vor1200_top_or1200_ctrl(const char* name="TOP");
    ~Vor1200_top_or1200_ctrl();
    void	__Vconfigure(Vor1200_top__Syms* symsp, bool first);
    
    // Sensitivity blocks
    static void	_combo__TOP__v__or1200_cpu__or1200_ctrl__4(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_sequent__TOP__v__or1200_cpu__or1200_ctrl__1(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_sequent__TOP__v__or1200_cpu__or1200_ctrl__2(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_settle__TOP__v__or1200_cpu__or1200_ctrl__3(Vor1200_top__Syms* __restrict vlSymsp);
    uint32_t	get_wb_insn();
} VL_ATTR_ALIGNED(64);

#endif  /*guard*/
