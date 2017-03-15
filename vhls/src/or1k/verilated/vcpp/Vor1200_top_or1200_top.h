// Verilated -*- C++ -*-
#ifndef _Vor1200_top_or1200_top_H_
#define _Vor1200_top_or1200_top_H_

#include "verilated.h"
#include "Vor1200_top__Inlines.h"
class Vor1200_top__Syms;
class Vor1200_top_or1200_cpu;

//----------

VL_MODULE(Vor1200_top_or1200_top) {
  public:
    // CELLS
    Vor1200_top_or1200_cpu*	or1200_cpu;
    
    // PORTS
    VL_IN8(__PVT__clk_i,0,0);
    VL_IN8(__PVT__rst_i,0,0);
    VL_IN8(__PVT__iwb_clk_i,0,0);
    VL_IN8(__PVT__iwb_rst_i,0,0);
    VL_IN8(__PVT__dwb_clk_i,0,0);
    VL_IN8(__PVT__dwb_rst_i,0,0);
    VL_IN8(__PVT__clmode_i,1,0);
    VL_IN8(__PVT__iwb_ack_i,0,0);
    VL_IN8(__PVT__iwb_err_i,0,0);
    VL_IN8(__PVT__iwb_rty_i,0,0);
    VL_OUT8(__PVT__iwb_cyc_o,0,0);
    VL_OUT8(__PVT__iwb_stb_o,0,0);
    VL_OUT8(__PVT__iwb_we_o,0,0);
    VL_OUT8(__PVT__iwb_sel_o,3,0);
    VL_OUT8(__PVT__iwb_cab_o,0,0);
    VL_IN8(__PVT__dwb_ack_i,0,0);
    VL_IN8(__PVT__dwb_err_i,0,0);
    VL_IN8(__PVT__dwb_rty_i,0,0);
    VL_OUT8(__PVT__dwb_cyc_o,0,0);
    VL_OUT8(__PVT__dwb_stb_o,0,0);
    VL_OUT8(__PVT__dwb_we_o,0,0);
    VL_OUT8(__PVT__dwb_sel_o,3,0);
    VL_OUT8(__PVT__dwb_cab_o,0,0);
    VL_IN8(__PVT__dbg_stall_i,0,0);
    VL_IN8(__PVT__dbg_ewt_i,0,0);
    VL_OUT8(__PVT__dbg_lss_o,3,0);
    VL_OUT8(__PVT__dbg_is_o,1,0);
    VL_OUT8(__PVT__dbg_bp_o,0,0);
    VL_IN8(__PVT__dbg_stb_i,0,0);
    VL_IN8(__PVT__dbg_we_i,0,0);
    VL_OUT8(__PVT__dbg_ack_o,0,0);
    VL_IN8(__PVT__pm_cpustall_i,0,0);
    VL_OUT8(__PVT__pm_clksd_o,3,0);
    VL_OUT8(__PVT__pm_dc_gate_o,0,0);
    VL_OUT8(__PVT__pm_ic_gate_o,0,0);
    VL_OUT8(__PVT__pm_dmmu_gate_o,0,0);
    VL_OUT8(__PVT__pm_immu_gate_o,0,0);
    VL_OUT8(__PVT__pm_tt_gate_o,0,0);
    VL_OUT8(__PVT__pm_cpu_gate_o,0,0);
    VL_OUT8(__PVT__pm_wakeup_o,0,0);
    VL_OUT8(__PVT__pm_lvolt_o,0,0);
    char	__VpadToAlign41[1];
    VL_OUT16(__PVT__dbg_wp_o,10,0);
    VL_IN(__PVT__pic_ints_i,19,0);
    VL_IN(__PVT__iwb_dat_i,31,0);
    VL_OUT(__PVT__iwb_adr_o,31,0);
    VL_OUT(__PVT__iwb_dat_o,31,0);
    VL_IN(__PVT__dwb_dat_i,31,0);
    VL_OUT(__PVT__dwb_adr_o,31,0);
    VL_OUT(__PVT__dwb_dat_o,31,0);
    VL_IN(__PVT__dbg_adr_i,31,0);
    VL_IN(__PVT__dbg_dat_i,31,0);
    VL_OUT(__PVT__dbg_dat_o,31,0);
    
    // LOCAL SIGNALS
    VL_SIG8(__PVT__dcsb_cyc_dc,0,0);
    VL_SIG8(__PVT__dcsb_stb_dc,0,0);
    VL_SIG8(__PVT__dcsb_cab_dc,0,0);
    VL_SIG8(__PVT__sbbiu_ack_biu,0,0);
    VL_SIG8(__PVT__sbbiu_err_biu,0,0);
    VL_SIG8(__PVT__icbiu_cyc_ic,0,0);
    VL_SIG8(__PVT__icbiu_stb_ic,0,0);
    VL_SIG8(__PVT__icbiu_cab_ic,0,0);
    VL_SIG8(__PVT__icbiu_ack_biu,0,0);
    VL_SIG8(__PVT__icbiu_err_biu,0,0);
    VL_SIG8(__PVT__qmemdmmu_cycstb_dmmu,0,0);
    VL_SIG8(__PVT__dcpu_ack_qmem,0,0);
    VL_SIG8(__PVT__dcpu_err_dmmu,0,0);
    VL_SIG8(__PVT__dcpu_tag_dmmu,3,0);
    VL_SIG8(__PVT__icpu_ack_qmem,0,0);
    VL_SIG8(__PVT__icpu_err_immu,0,0);
    VL_SIG8(__PVT__icpu_tag_immu,3,0);
    VL_SIG8(__PVT__icpu_rty_immu,0,0);
    VL_SIG8(__PVT__qmemimmu_cycstb_immu,0,0);
    VL_SIG8(__PVT__icqmem_err_ic,0,0);
    VL_SIG8(__PVT__icqmem_cycstb_qmem,0,0);
    VL_SIG8(__PVT__icqmem_ack_ic,0,0);
    VL_SIG8(__PVT__dcqmem_err_dc,0,0);
    VL_SIG8(__PVT__dcqmem_cycstb_qmem,0,0);
    VL_SIG8(__PVT__dcqmem_ci_qmem,0,0);
    VL_SIG8(__PVT__dcqmem_we_qmem,0,0);
    VL_SIG8(__PVT__dcqmem_sel_qmem,3,0);
    VL_SIG8(__PVT__dcqmem_ack_dc,0,0);
    VL_SIG8(__PVT__du_read,0,0);
    VL_SIG8(__PVT__du_write,0,0);
    VL_SIG8(__PVT__iwb_biu__DOT__valid_div,1,0);
    VL_SIG8(__PVT__iwb_biu__DOT__aborted,0,0);
    VL_SIG8(__PVT__iwb_biu__DOT__aborted_r,0,0);
    VL_SIG8(__PVT__iwb_biu__DOT__previous_complete,0,0);
    VL_SIG8(__PVT__iwb_biu__DOT__repeated_access,0,0);
    VL_SIG8(__PVT__iwb_biu__DOT__repeated_access_ack,0,0);
    VL_SIG8(__PVT__dwb_biu__DOT__valid_div,1,0);
    VL_SIG8(__PVT__dwb_biu__DOT__aborted,0,0);
    VL_SIG8(__PVT__dwb_biu__DOT__aborted_r,0,0);
    VL_SIG8(__PVT__or1200_immu_top__DOT__itlb_spr_access,0,0);
    VL_SIG8(__PVT__or1200_immu_top__DOT__itlb_en,0,0);
    VL_SIG8(__PVT__or1200_immu_top__DOT__itlb_done,0,0);
    VL_SIG8(__PVT__or1200_immu_top__DOT__fault,0,0);
    VL_SIG8(__PVT__or1200_immu_top__DOT__miss,0,0);
    VL_SIG8(__PVT__or1200_immu_top__DOT__page_cross,0,0);
    VL_SIG8(__PVT__or1200_immu_top__DOT__itlb_en_r,0,0);
    VL_SIG8(__PVT__or1200_immu_top__DOT__dis_spr_access,0,0);
    VL_SIG8(__PVT__or1200_immu_top__DOT__or1200_immu_tlb__DOT__tlb_index,5,0);
    VL_SIG8(__PVT__or1200_immu_top__DOT__or1200_immu_tlb__DOT__tlb_mr_en,0,0);
    VL_SIG8(__PVT__or1200_immu_top__DOT__or1200_immu_tlb__DOT__tlb_tr_en,0,0);
    VL_SIG8(__PVT__or1200_immu_top__DOT__or1200_immu_tlb__DOT__itlb_mr_ram__DOT__addr_reg,5,0);
    VL_SIG8(__PVT__or1200_immu_top__DOT__or1200_immu_tlb__DOT__itlb_tr_ram__DOT__addr_reg,5,0);
    VL_SIG8(__PVT__or1200_ic_top__DOT__icfsm_biu_read,0,0);
    VL_SIG8(__PVT__or1200_ic_top__DOT__tagcomp_miss,0,0);
    VL_SIG8(__PVT__or1200_ic_top__DOT__ictag_addr,11,4);
    VL_SIG8(__PVT__or1200_ic_top__DOT__ictag_en,0,0);
    VL_SIG8(__PVT__or1200_ic_top__DOT__ic_inv,0,0);
    VL_SIG8(__PVT__or1200_ic_top__DOT__icfsm_first_miss_ack,0,0);
    VL_SIG8(__PVT__or1200_ic_top__DOT__or1200_ic_fsm__DOT__state,1,0);
    VL_SIG8(__PVT__or1200_ic_top__DOT__or1200_ic_fsm__DOT__cnt,2,0);
    VL_SIG8(__PVT__or1200_ic_top__DOT__or1200_ic_fsm__DOT__hitmiss_eval,0,0);
    VL_SIG8(__PVT__or1200_ic_top__DOT__or1200_ic_fsm__DOT__load,0,0);
    VL_SIG8(__PVT__or1200_ic_top__DOT__or1200_ic_fsm__DOT__cache_inhibit,0,0);
    VL_SIG8(__PVT__or1200_ic_top__DOT__or1200_ic_tag__DOT__ic_tag0__DOT__addr_reg,7,0);
    VL_SIG8(__PVT__or1200_dmmu_top__DOT__dtlb_en,0,0);
    VL_SIG8(__PVT__or1200_dmmu_top__DOT__fault,0,0);
    VL_SIG8(__PVT__or1200_dmmu_top__DOT__miss,0,0);
    VL_SIG8(__PVT__or1200_dmmu_top__DOT__dtlb_done,0,0);
    VL_SIG8(__PVT__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__tlb_index,5,0);
    VL_SIG8(__PVT__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__tlb_mr_en,0,0);
    VL_SIG8(__PVT__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__tlb_tr_en,0,0);
    VL_SIG8(__PVT__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__dtlb_mr_ram__DOT__addr_reg,5,0);
    VL_SIG8(__PVT__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__dtlb_tr_ram__DOT__addr_reg,5,0);
    VL_SIG8(__PVT__or1200_dc_top__DOT__dcram_we,3,0);
    VL_SIG8(__PVT__or1200_dc_top__DOT__dcfsm_biu_read,0,0);
    VL_SIG8(__PVT__or1200_dc_top__DOT__tagcomp_miss,0,0);
    VL_SIG8(__PVT__or1200_dc_top__DOT__dctag_addr,11,4);
    VL_SIG8(__PVT__or1200_dc_top__DOT__dctag_en,0,0);
    VL_SIG8(__PVT__or1200_dc_top__DOT__dc_inv,0,0);
    VL_SIG8(__PVT__or1200_dc_top__DOT__dcfsm_first_miss_ack,0,0);
    VL_SIG8(__PVT__or1200_dc_top__DOT__or1200_dc_fsm__DOT__state,2,0);
    VL_SIG8(__PVT__or1200_dc_top__DOT__or1200_dc_fsm__DOT__cnt,2,0);
    VL_SIG8(__PVT__or1200_dc_top__DOT__or1200_dc_fsm__DOT__hitmiss_eval,0,0);
    VL_SIG8(__PVT__or1200_dc_top__DOT__or1200_dc_fsm__DOT__store,0,0);
    VL_SIG8(__PVT__or1200_dc_top__DOT__or1200_dc_fsm__DOT__load,0,0);
    VL_SIG8(__PVT__or1200_dc_top__DOT__or1200_dc_fsm__DOT__cache_inhibit,0,0);
    VL_SIG8(__PVT__or1200_dc_top__DOT__or1200_dc_fsm__DOT__first_store_hit_ack,0,0);
    VL_SIG8(__PVT__or1200_dc_top__DOT__or1200_dc_tag__DOT__dc_tag0__DOT__addr_reg,7,0);
    VL_SIG8(__PVT__or1200_qmem_top__DOT__state,2,0);
    VL_SIG8(__PVT__or1200_qmem_top__DOT__qmem_dack,0,0);
    VL_SIG8(__PVT__or1200_qmem_top__DOT__qmem_iack,0,0);
    VL_SIG8(__PVT__or1200_qmem_top__DOT__qmem_en,0,0);
    VL_SIG8(__PVT__or1200_du__DOT__dbg_bp_r,0,0);
    VL_SIG8(__PVT__or1200_tt__DOT__match,0,0);
    VL_SIG8(__PVT__or1200_pm__DOT__sdf,3,0);
    VL_SIG8(__PVT__or1200_pm__DOT__dme,0,0);
    VL_SIG8(__PVT__or1200_pm__DOT__sme,0,0);
    VL_SIG8(__PVT__or1200_pm__DOT__dcge,0,0);
    VL_SIG16(__PVT__or1200_immu_top__DOT__or1200_immu_tlb__DOT__tlb_mr_ram_out,13,0);
    VL_SIG16(__PVT__or1200_ic_top__DOT__or1200_ic_ram__DOT__ic_ram0__DOT__addr_reg,9,0);
    VL_SIG16(__PVT__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__tlb_mr_ram_out,13,0);
    VL_SIG16(__PVT__or1200_dc_top__DOT__or1200_dc_ram__DOT__dc_ram__DOT__addr_reg,9,0);
    VL_SIG16(__PVT__or1200_qmem_top__DOT__or1200_qmem_ram__DOT__addr_reg,10,0);
    VL_SIG16(__PVT__or1200_du__DOT__dsr,13,0);
    VL_SIG16(__PVT__or1200_du__DOT__drr,13,0);
    VL_SIG16(__PVT__or1200_du__DOT__except_stop,13,0);
    char	__VpadToAlign202[2];
    VL_SIG(__PVT__icbiu_dat_biu,31,0);
    VL_SIG(__PVT__qmemdmmu_adr_dmmu,31,0);
    VL_SIG(__PVT__icpu_dat_qmem,31,0);
    VL_SIG(__PVT__icpu_adr_immu,31,0);
    VL_SIG(__PVT__qmemimmu_adr_immu,31,0);
    VL_SIG(__PVT__icqmem_adr_qmem,31,0);
    VL_SIG(__PVT__dcqmem_adr_qmem,31,0);
    VL_SIG(__PVT__dcqmem_dat_qmem,31,0);
    VL_SIG(__PVT__spr_dat_pm,31,0);
    VL_SIG(__PVT__iwb_biu__DOT__wb_dat_r,31,0);
    VL_SIG(__PVT__or1200_immu_top__DOT__icpu_vpn_r,31,13);
    VL_SIG(__PVT__or1200_immu_top__DOT__or1200_immu_tlb__DOT__tlb_tr_ram_out,21,0);
    VL_SIG(__PVT__or1200_ic_top__DOT__from_icram,31,0);
    VL_SIG(__PVT__or1200_ic_top__DOT__ic_addr,31,0);
    VL_SIG(__PVT__or1200_ic_top__DOT__or1200_ic_fsm__DOT__saved_addr_r,31,0);
    VL_SIG(__PVT__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__tlb_tr_ram_out,23,0);
    VL_SIG(__PVT__or1200_dc_top__DOT__to_dcram,31,0);
    VL_SIG(__PVT__or1200_dc_top__DOT__from_dcram,31,0);
    VL_SIG(__PVT__or1200_dc_top__DOT__dc_addr,31,0);
    VL_SIG(__PVT__or1200_dc_top__DOT__or1200_dc_fsm__DOT__saved_addr_r,31,0);
    VL_SIG(__PVT__or1200_qmem_top__DOT__qmem_do,31,0);
    VL_SIG(__PVT__or1200_qmem_top__DOT__qmem_addr,31,0);
    VL_SIG(__PVT__or1200_du__DOT__dmr1,24,0);
    VL_SIG(__PVT__or1200_pic__DOT__picmr,19,2);
    VL_SIG(__PVT__or1200_pic__DOT__picsr,19,0);
    VL_SIG(__PVT__or1200_pic__DOT__um_ints,19,0);
    VL_SIG(__PVT__or1200_tt__DOT__ttmr,31,0);
    VL_SIG(__PVT__or1200_tt__DOT__ttcr,31,0);
    char	__VpadToAlign316[4];
    VL_SIG16(__PVT__or1200_immu_top__DOT__or1200_immu_tlb__DOT__itlb_mr_ram__DOT__mem[64],13,0);
    VL_SIG(__PVT__or1200_immu_top__DOT__or1200_immu_tlb__DOT__itlb_tr_ram__DOT__mem[64],21,0);
    VL_SIG(__PVT__or1200_ic_top__DOT__or1200_ic_ram__DOT__ic_ram0__DOT__mem[1024],31,0);
    VL_SIG(__PVT__or1200_ic_top__DOT__or1200_ic_tag__DOT__ic_tag0__DOT__mem[256],20,0);
    VL_SIG16(__PVT__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__dtlb_mr_ram__DOT__mem[64],13,0);
    VL_SIG(__PVT__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__dtlb_tr_ram__DOT__mem[64],23,0);
    VL_SIG8(__PVT__or1200_dc_top__DOT__or1200_dc_ram__DOT__dc_ram__DOT__mem_0[1024],7,0);
    VL_SIG8(__PVT__or1200_dc_top__DOT__or1200_dc_ram__DOT__dc_ram__DOT__mem_1[1024],7,0);
    VL_SIG8(__PVT__or1200_dc_top__DOT__or1200_dc_ram__DOT__dc_ram__DOT__mem_2[1024],7,0);
    VL_SIG8(__PVT__or1200_dc_top__DOT__or1200_dc_ram__DOT__dc_ram__DOT__mem_3[1024],7,0);
    VL_SIG(__PVT__or1200_dc_top__DOT__or1200_dc_tag__DOT__dc_tag0__DOT__mem[256],20,0);
    VL_SIG(__PVT__or1200_qmem_top__DOT__or1200_qmem_ram__DOT__mem[2048],31,0);
    
    // LOCAL VARIABLES
    static VL_ST_SIG8(__Vtable1___PVT__or1200_qmem_top__DOT__state[512],2,0);
    static VL_ST_SIG8(__Vtable1___PVT__or1200_qmem_top__DOT__qmem_dack[512],0,0);
    static VL_ST_SIG8(__Vtable1___PVT__or1200_qmem_top__DOT__qmem_iack[512],0,0);
    VL_SIG8(__Vdly__iwb_we_o,0,0);
    VL_SIG8(__Vdly__iwb_biu__DOT__valid_div,1,0);
    VL_SIG8(__Vdly__dwb_biu__DOT__valid_div,1,0);
    VL_SIG8(__Vdly__or1200_ic_top__DOT__or1200_ic_fsm__DOT__state,1,0);
    VL_SIG8(__Vdly__or1200_ic_top__DOT__or1200_ic_fsm__DOT__hitmiss_eval,0,0);
    VL_SIG8(__Vdly__or1200_ic_top__DOT__or1200_ic_fsm__DOT__cnt,2,0);
    VL_SIG8(__Vdly__or1200_ic_top__DOT__or1200_ic_fsm__DOT__cache_inhibit,0,0);
    VL_SIG8(__Vdly__or1200_dc_top__DOT__or1200_dc_fsm__DOT__state,2,0);
    VL_SIG8(__Vdly__or1200_dc_top__DOT__or1200_dc_fsm__DOT__hitmiss_eval,0,0);
    VL_SIG8(__Vdly__or1200_dc_top__DOT__or1200_dc_fsm__DOT__cnt,2,0);
    VL_SIG8(__Vdly__or1200_dc_top__DOT__or1200_dc_fsm__DOT__cache_inhibit,0,0);
    VL_SIG8(__Vdly__or1200_qmem_top__DOT__state,2,0);
    VL_SIG8(__Vdly__dbg_is_o,1,0);
    VL_SIG8(__Vdly__dwb_we_o,0,0);
    VL_SIG8(__Vdlyvdim0__or1200_immu_top__DOT__or1200_immu_tlb__DOT__itlb_mr_ram__DOT__mem__v0,5,0);
    VL_SIG8(__Vdlyvset__or1200_immu_top__DOT__or1200_immu_tlb__DOT__itlb_mr_ram__DOT__mem__v0,0,0);
    VL_SIG8(__Vdlyvdim0__or1200_immu_top__DOT__or1200_immu_tlb__DOT__itlb_tr_ram__DOT__mem__v0,5,0);
    VL_SIG8(__Vdlyvset__or1200_immu_top__DOT__or1200_immu_tlb__DOT__itlb_tr_ram__DOT__mem__v0,0,0);
    VL_SIG8(__Vdlyvset__or1200_ic_top__DOT__or1200_ic_ram__DOT__ic_ram0__DOT__mem__v0,0,0);
    VL_SIG8(__Vdlyvdim0__or1200_ic_top__DOT__or1200_ic_tag__DOT__ic_tag0__DOT__mem__v0,7,0);
    VL_SIG8(__Vdlyvset__or1200_ic_top__DOT__or1200_ic_tag__DOT__ic_tag0__DOT__mem__v0,0,0);
    VL_SIG8(__Vdlyvdim0__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__dtlb_mr_ram__DOT__mem__v0,5,0);
    VL_SIG8(__Vdlyvset__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__dtlb_mr_ram__DOT__mem__v0,0,0);
    VL_SIG8(__Vdlyvdim0__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__dtlb_tr_ram__DOT__mem__v0,5,0);
    VL_SIG8(__Vdlyvset__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__dtlb_tr_ram__DOT__mem__v0,0,0);
    VL_SIG8(__Vdlyvval__or1200_dc_top__DOT__or1200_dc_ram__DOT__dc_ram__DOT__mem_0__v0,7,0);
    VL_SIG8(__Vdlyvset__or1200_dc_top__DOT__or1200_dc_ram__DOT__dc_ram__DOT__mem_0__v0,0,0);
    VL_SIG8(__Vdlyvval__or1200_dc_top__DOT__or1200_dc_ram__DOT__dc_ram__DOT__mem_1__v0,7,0);
    VL_SIG8(__Vdlyvset__or1200_dc_top__DOT__or1200_dc_ram__DOT__dc_ram__DOT__mem_1__v0,0,0);
    VL_SIG8(__Vdlyvval__or1200_dc_top__DOT__or1200_dc_ram__DOT__dc_ram__DOT__mem_2__v0,7,0);
    VL_SIG8(__Vdlyvset__or1200_dc_top__DOT__or1200_dc_ram__DOT__dc_ram__DOT__mem_2__v0,0,0);
    VL_SIG8(__Vdlyvval__or1200_dc_top__DOT__or1200_dc_ram__DOT__dc_ram__DOT__mem_3__v0,7,0);
    VL_SIG8(__Vdlyvset__or1200_dc_top__DOT__or1200_dc_ram__DOT__dc_ram__DOT__mem_3__v0,0,0);
    VL_SIG8(__Vdlyvdim0__or1200_dc_top__DOT__or1200_dc_tag__DOT__dc_tag0__DOT__mem__v0,7,0);
    VL_SIG8(__Vdlyvset__or1200_dc_top__DOT__or1200_dc_tag__DOT__dc_tag0__DOT__mem__v0,0,0);
    VL_SIG8(__Vdlyvset__or1200_qmem_top__DOT__or1200_qmem_ram__DOT__mem__v0,0,0);
    VL_SIG16(__Vtableidx1,8,0);
    VL_SIG16(__Vdly__or1200_du__DOT__drr,13,0);
    VL_SIG16(__Vdlyvval__or1200_immu_top__DOT__or1200_immu_tlb__DOT__itlb_mr_ram__DOT__mem__v0,13,0);
    VL_SIG16(__Vdlyvdim0__or1200_ic_top__DOT__or1200_ic_ram__DOT__ic_ram0__DOT__mem__v0,9,0);
    VL_SIG16(__Vdlyvval__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__dtlb_mr_ram__DOT__mem__v0,13,0);
    VL_SIG16(__Vdlyvdim0__or1200_dc_top__DOT__or1200_dc_ram__DOT__dc_ram__DOT__mem_0__v0,9,0);
    VL_SIG16(__Vdlyvdim0__or1200_dc_top__DOT__or1200_dc_ram__DOT__dc_ram__DOT__mem_1__v0,9,0);
    VL_SIG16(__Vdlyvdim0__or1200_dc_top__DOT__or1200_dc_ram__DOT__dc_ram__DOT__mem_2__v0,9,0);
    VL_SIG16(__Vdlyvdim0__or1200_dc_top__DOT__or1200_dc_ram__DOT__dc_ram__DOT__mem_3__v0,9,0);
    VL_SIG16(__Vdlyvdim0__or1200_qmem_top__DOT__or1200_qmem_ram__DOT__mem__v0,10,0);
    VL_SIG(or1200_ic_top__DOT__or1200_ic_tag__DOT____Vcellinp__ic_tag0__doq,20,0);
    VL_SIG(or1200_dc_top__DOT__or1200_dc_tag__DOT____Vcellinp__dc_tag0__doq,20,0);
    VL_SIG(__Vdly__or1200_ic_top__DOT__or1200_ic_fsm__DOT__saved_addr_r,31,0);
    VL_SIG(__Vdly__or1200_dc_top__DOT__or1200_dc_fsm__DOT__saved_addr_r,31,0);
    VL_SIG(__Vdly__or1200_pic__DOT__picsr,19,0);
    VL_SIG(__Vdly__or1200_tt__DOT__ttmr,31,0);
    VL_SIG(__Vdly__or1200_tt__DOT__ttcr,31,0);
    VL_SIG(__Vdlyvval__or1200_immu_top__DOT__or1200_immu_tlb__DOT__itlb_tr_ram__DOT__mem__v0,21,0);
    VL_SIG(__Vdlyvval__or1200_ic_top__DOT__or1200_ic_ram__DOT__ic_ram0__DOT__mem__v0,31,0);
    VL_SIG(__Vdlyvval__or1200_ic_top__DOT__or1200_ic_tag__DOT__ic_tag0__DOT__mem__v0,20,0);
    VL_SIG(__Vdlyvval__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__dtlb_tr_ram__DOT__mem__v0,23,0);
    VL_SIG(__Vdlyvval__or1200_dc_top__DOT__or1200_dc_tag__DOT__dc_tag0__DOT__mem__v0,20,0);
    VL_SIG(__Vdlyvval__or1200_qmem_top__DOT__or1200_qmem_ram__DOT__mem__v0,31,0);
    VL_SIG8(__Vtablechg1[512],2,0);
    
    // INTERNAL VARIABLES
  private:
    char	__VpadToAlign20148[4];
    Vor1200_top__Syms*	__VlSymsp;		// Symbol table
  public:
    
    // PARAMETERS
    
    // METHODS
  private:
    Vor1200_top_or1200_top& operator= (const Vor1200_top_or1200_top&);	///< Copying not allowed
    Vor1200_top_or1200_top(const Vor1200_top_or1200_top&);	///< Copying not allowed
  public:
    Vor1200_top_or1200_top(const char* name="TOP");
    ~Vor1200_top_or1200_top();
    void	__Vconfigure(Vor1200_top__Syms* symsp, bool first);
    
    // Sensitivity blocks
    static void	_combo__TOP__v__11(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_combo__TOP__v__16(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_combo__TOP__v__18(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_combo__TOP__v__21(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_combo__TOP__v__22(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_combo__TOP__v__24(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_combo__TOP__v__26(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_combo__TOP__v__28(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_combo__TOP__v__6(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_sequent__TOP__v__10(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_sequent__TOP__v__14(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_sequent__TOP__v__15(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_sequent__TOP__v__19(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_sequent__TOP__v__2(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_sequent__TOP__v__3(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_sequent__TOP__v__4(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_sequent__TOP__v__5(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_sequent__TOP__v__7(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_sequent__TOP__v__9(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_settle__TOP__v__1(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_settle__TOP__v__12(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_settle__TOP__v__13(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_settle__TOP__v__17(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_settle__TOP__v__20(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_settle__TOP__v__23(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_settle__TOP__v__25(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_settle__TOP__v__27(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_settle__TOP__v__29(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_settle__TOP__v__30(Vor1200_top__Syms* __restrict vlSymsp);
    static void	_settle__TOP__v__8(Vor1200_top__Syms* __restrict vlSymsp);
} VL_ATTR_ALIGNED(64);

#endif  /*guard*/
