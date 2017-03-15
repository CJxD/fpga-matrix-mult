// Verilated -*- C++ -*-
#ifndef _Vor1200_top_or1200_cpu_H_
#define _Vor1200_top_or1200_cpu_H_

#include "verilated.h"
#include "Vor1200_top__Inlines.h"
class Vor1200_top__Syms;
class Vor1200_top_or1200_ctrl;
class Vor1200_top_or1200_rf;

//----------

VL_MODULE(Vor1200_top_or1200_cpu) {
  public:
    // CELLS
    Vor1200_top_or1200_ctrl*	or1200_ctrl;
    Vor1200_top_or1200_rf*	or1200_rf;
    
    // PORTS
    VL_IN8(__PVT__clk,0,0);
    VL_IN8(__PVT__rst,0,0);
    VL_OUT8(__PVT__ic_en,0,0);
    VL_OUT8(__PVT__icpu_cycstb_o,0,0);
    VL_OUT8(__PVT__icpu_sel_o,3,0);
    VL_OUT8(__PVT__icpu_tag_o,3,0);
    VL_IN8(__PVT__icpu_ack_i,0,0);
    VL_IN8(__PVT__icpu_rty_i,0,0);
    VL_IN8(__PVT__icpu_err_i,0,0);
    VL_IN8(__PVT__icpu_tag_i,3,0);
    VL_OUT8(__PVT__immu_en,0,0);
    VL_OUT8(__PVT__ex_freeze,0,0);
    VL_OUT8(__PVT__branch_op,2,0);
    VL_IN8(__PVT__du_stall,0,0);
    VL_IN8(__PVT__du_read,0,0);
    VL_IN8(__PVT__du_write,0,0);
    VL_IN8(__PVT__du_hwbkpt,0,0);
    VL_OUT8(__PVT__dcpu_cycstb_o,0,0);
    VL_OUT8(__PVT__dcpu_we_o,0,0);
    VL_OUT8(__PVT__dcpu_sel_o,3,0);
    VL_OUT8(__PVT__dcpu_tag_o,3,0);
    VL_IN8(__PVT__dcpu_ack_i,0,0);
    VL_IN8(__PVT__dcpu_rty_i,0,0);
    VL_IN8(__PVT__dcpu_err_i,0,0);
    VL_IN8(__PVT__dcpu_tag_i,3,0);
    VL_OUT8(__PVT__dc_en,0,0);
    VL_OUT8(__PVT__dmmu_en,0,0);
    VL_OUT8(__PVT__supv,0,0);
    VL_OUT8(__PVT__spr_we,0,0);
    VL_IN8(__PVT__sig_int,0,0);
    VL_IN8(__PVT__sig_tick,0,0);
    char	__VpadToAlign31[1];
    VL_IN16(__PVT__du_dsr,13,0);
    VL_OUT16(__PVT__du_except,12,0);
    VL_OUT(__PVT__icpu_adr_o,31,0);
    VL_IN(__PVT__icpu_dat_i,31,0);
    VL_IN(__PVT__icpu_adr_i,31,0);
    VL_OUT(__PVT__ex_insn,31,0);
    VL_OUT(__PVT__id_pc,31,0);
    VL_IN(__PVT__du_addr,31,0);
    VL_IN(__PVT__du_dat_du,31,0);
    VL_OUT(__PVT__du_dat_cpu,31,0);
    VL_OUT(__PVT__rf_dataw,31,0);
    VL_OUT(__PVT__dcpu_adr_o,31,0);
    VL_OUT(__PVT__dcpu_dat_o,31,0);
    VL_IN(__PVT__dcpu_dat_i,31,0);
    VL_IN(__PVT__spr_dat_pic,31,0);
    VL_IN(__PVT__spr_dat_tt,31,0);
    VL_IN(__PVT__spr_dat_pm,31,0);
    VL_IN(__PVT__spr_dat_dmmu,31,0);
    VL_IN(__PVT__spr_dat_immu,31,0);
    VL_IN(__PVT__spr_dat_du,31,0);
    VL_OUT(__PVT__spr_addr,31,0);
    VL_OUT(__PVT__spr_dat_cpu,31,0);
    VL_OUT(__PVT__spr_dat_npc,31,0);
    VL_OUT(__PVT__spr_cs,31,0);
    
    // LOCAL SIGNALS
    VL_SIG8(__PVT__genpc_freeze,0,0);
    VL_SIG8(__PVT__if_freeze,0,0);
    VL_SIG8(__PVT__id_freeze,0,0);
    VL_SIG8(__PVT__wb_freeze,0,0);
    VL_SIG8(__PVT__wbforw_valid,0,0);
    VL_SIG8(__PVT__except_type,3,0);
    VL_SIG8(__PVT__flushpipe,0,0);
    VL_SIG8(__PVT__extend_flush,0,0);
    VL_SIG8(__PVT__branch_taken,0,0);
    VL_SIG8(__PVT__flagforw,0,0);
    VL_SIG8(__PVT__flag_we,0,0);
    VL_SIG8(__PVT__cyforw,0,0);
    VL_SIG8(__PVT__cy_we,0,0);
    VL_SIG8(__PVT__lsu_stall,0,0);
    VL_SIG8(__PVT__pc_we,0,0);
    VL_SIG8(__PVT__sr_we,0,0);
    VL_SIG8(__PVT__except_start,0,0);
    VL_SIG8(__PVT__if_stall,0,0);
    VL_SIG8(__PVT__mac_stall,0,0);
    VL_SIG8(__PVT__genpc_refetch,0,0);
    VL_SIG8(__PVT__except_align,0,0);
    VL_SIG8(__PVT__except_dtlbmiss,0,0);
    VL_SIG8(__PVT__except_dmmufault,0,0);
    VL_SIG8(__PVT__except_dbuserr,0,0);
    VL_SIG8(__PVT__or1200_if__DOT__saved,0,0);
    VL_SIG8(__PVT__or1200_operandmuxes__DOT__saved_a,0,0);
    VL_SIG8(__PVT__or1200_operandmuxes__DOT__saved_b,0,0);
    VL_SIG8(__PVT__or1200_mult_mac__DOT__mac_op_r1,1,0);
    VL_SIG8(__PVT__or1200_mult_mac__DOT__mac_op_r2,1,0);
    VL_SIG8(__PVT__or1200_mult_mac__DOT__mac_op_r3,1,0);
    VL_SIG8(__PVT__or1200_mult_mac__DOT__div_free,0,0);
    VL_SIG8(__PVT__or1200_mult_mac__DOT__div_cntr,5,0);
    VL_SIG8(__PVT__or1200_sprs__DOT__read_spr,0,0);
    VL_SIG8(__PVT__or1200_sprs__DOT__npc_sel,0,0);
    VL_SIG8(__PVT__or1200_sprs__DOT__ppc_sel,0,0);
    VL_SIG8(__PVT__or1200_sprs__DOT__sr_sel,0,0);
    VL_SIG8(__PVT__or1200_sprs__DOT__epcr_sel,0,0);
    VL_SIG8(__PVT__or1200_sprs__DOT__eear_sel,0,0);
    VL_SIG8(__PVT__or1200_sprs__DOT__esr_sel,0,0);
    VL_SIG8(__PVT__or1200_sprs__DOT__sprs_op,3,0);
    VL_SIG8(__PVT__or1200_freeze__DOT__multicycle_cnt,1,0);
    VL_SIG8(__PVT__or1200_freeze__DOT__flushpipe_r,0,0);
    VL_SIG8(__PVT__or1200_except__DOT__id_exceptflags,2,0);
    VL_SIG8(__PVT__or1200_except__DOT__ex_exceptflags,2,0);
    VL_SIG8(__PVT__or1200_except__DOT__state,2,0);
    VL_SIG8(__PVT__or1200_except__DOT__ex_dslot,0,0);
    VL_SIG8(__PVT__or1200_except__DOT__except_flushpipe,0,0);
    VL_SIG8(__PVT__or1200_except__DOT__delayed_iee,2,0);
    VL_SIG8(__PVT__or1200_except__DOT__int_pending,0,0);
    VL_SIG8(__PVT__or1200_except__DOT__tick_pending,0,0);
    VL_SIG16(__PVT__esr,15,0);
    VL_SIG16(__PVT__to_sr,15,0);
    VL_SIG16(__PVT__sr,15,0);
    VL_SIG16(__PVT__or1200_except__DOT__except_trig,12,0);
    char	__VpadToAlign186[2];
    VL_SIG(__PVT__if_insn,31,0);
    VL_SIG(__PVT__muxed_b,31,0);
    VL_SIG(__PVT__wb_forw,31,0);
    VL_SIG(__PVT__operand_a,31,0);
    VL_SIG(__PVT__operand_b,31,0);
    VL_SIG(__PVT__sprs_dataout,31,0);
    VL_SIG(__PVT__epcr,31,0);
    VL_SIG(__PVT__eear,31,0);
    VL_SIG(__PVT__mult_mac_result,31,0);
    VL_SIG(__PVT__or1200_genpc__DOT__pcreg,31,2);
    VL_SIG(__PVT__or1200_genpc__DOT__pc,31,0);
    VL_SIG(__PVT__or1200_if__DOT__insn_saved,31,0);
    VL_SIG(__PVT__or1200_if__DOT__addr_saved,31,0);
    VL_SIG(__PVT__or1200_operandmuxes__DOT__muxed_a,31,0);
    VL_SIG(__PVT__or1200_alu__DOT__comp_a,31,0);
    VL_SIG(__PVT__or1200_alu__DOT__comp_b,31,0);
    VL_SIG(__PVT__or1200_alu__DOT__result_and,31,0);
    VL_SIG(__PVT__or1200_mult_mac__DOT__x,31,0);
    VL_SIG(__PVT__or1200_mult_mac__DOT__y,31,0);
    VL_SIG(__PVT__or1200_mult_mac__DOT__div_tmp,31,0);
    VL_SIG(__PVT__or1200_lsu__DOT__or1200_mem2reg__DOT__aligned,31,0);
    VL_SIG(__PVT__or1200_except__DOT__ex_pc,31,0);
    VL_SIG(__PVT__or1200_except__DOT__wb_pc,31,0);
    VL_SIG64(__PVT__or1200_mult_mac__DOT__mul_prod_r,63,0);
    VL_SIG64(__PVT__or1200_mult_mac__DOT__mac_r,63,0);
    VL_SIG64(__PVT__or1200_mult_mac__DOT__or1200_gmultp2_32x32__DOT__p0,63,0);
    VL_SIG64(__PVT__or1200_mult_mac__DOT__or1200_gmultp2_32x32__DOT__p1,63,0);
    
    // LOCAL VARIABLES
    VL_SIG8(__Vdly__or1200_if__DOT__saved,0,0);
    VL_SIG8(__Vdly__or1200_operandmuxes__DOT__saved_a,0,0);
    VL_SIG8(__Vdly__or1200_operandmuxes__DOT__saved_b,0,0);
    VL_SIG8(__Vdly__or1200_mult_mac__DOT__div_free,0,0);
    VL_SIG8(__Vdly__or1200_mult_mac__DOT__div_cntr,5,0);
    VL_SIG8(__Vdly__or1200_freeze__DOT__multicycle_cnt,1,0);
    VL_SIG8(__Vdly__or1200_except__DOT__delayed_iee,2,0);
    VL_SIG8(__Vdly__or1200_except__DOT__ex_dslot,0,0);
    VL_SIG8(__Vdly__or1200_except__DOT__state,2,0);
    VL_SIG8(__Vdly__except_type,3,0);
    VL_SIG8(__Vdly__extend_flush,0,0);
    char	__VpadToAlign327[1];
    VL_SIG64(__Vdly__or1200_mult_mac__DOT__mul_prod_r,63,0);
    VL_SIG64(__Vdly__or1200_mult_mac__DOT__mac_r,63,0);
    
    // INTERNAL VARIABLES
  private:
    char	__VpadToAlign348[4];
    Vor1200_top__Syms*	__VlSymsp;		// Symbol table
  public:
    
    // PARAMETERS
    
    // METHODS
  private:
    Vor1200_top_or1200_cpu& operator= (const Vor1200_top_or1200_cpu&);	///< Copying not allowed
    Vor1200_top_or1200_cpu(const Vor1200_top_or1200_cpu&);	///< Copying not allowed
  public:
    Vor1200_top_or1200_cpu(const char* name="TOP");
    ~Vor1200_top_or1200_cpu();
    void	__Vconfigure(Vor1200_top__Syms* symsp, bool first);
    
    // Sensitivity blocks
    static void	_combo__TOP__v__or1200_cpu__12(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_combo__TOP__v__or1200_cpu__16(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_combo__TOP__v__or1200_cpu__18(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_combo__TOP__v__or1200_cpu__20(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_combo__TOP__v__or1200_cpu__22(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_combo__TOP__v__or1200_cpu__24(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_combo__TOP__v__or1200_cpu__3(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_combo__TOP__v__or1200_cpu__6(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_combo__TOP__v__or1200_cpu__9(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_sequent__TOP__v__or1200_cpu__1(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_sequent__TOP__v__or1200_cpu__2(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_sequent__TOP__v__or1200_cpu__5(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_sequent__TOP__v__or1200_cpu__8(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_settle__TOP__v__or1200_cpu__10(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_settle__TOP__v__or1200_cpu__11(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_settle__TOP__v__or1200_cpu__13(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_settle__TOP__v__or1200_cpu__15(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_settle__TOP__v__or1200_cpu__17(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_settle__TOP__v__or1200_cpu__19(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_settle__TOP__v__or1200_cpu__21(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_settle__TOP__v__or1200_cpu__23(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_settle__TOP__v__or1200_cpu__4(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_settle__TOP__v__or1200_cpu__7(Vor1200_top__Syms* __restrict vlSymsp);
} VL_ATTR_ALIGNED(64);

#endif  /*guard*/
