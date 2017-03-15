// Verilated -*- C++ -*-
#include "Vor1200_top_or1200_cpu.h" // For This
#include "Vor1200_top__Syms.h"

//--------------------
// STATIC VARIABLES


//--------------------

VL_CTOR_IMP(Vor1200_top_or1200_cpu) {
    VL_CELL (or1200_ctrl, Vor1200_top_or1200_ctrl);
    VL_CELL (or1200_rf, Vor1200_top_or1200_rf);
    // Reset internal values
    // Reset structure values
    __PVT__clk = VL_RAND_RESET_I(1);
    __PVT__rst = VL_RAND_RESET_I(1);
    __PVT__ic_en = VL_RAND_RESET_I(1);
    __PVT__icpu_adr_o = VL_RAND_RESET_I(32);
    __PVT__icpu_cycstb_o = VL_RAND_RESET_I(1);
    __PVT__icpu_sel_o = VL_RAND_RESET_I(4);
    __PVT__icpu_tag_o = VL_RAND_RESET_I(4);
    __PVT__icpu_dat_i = VL_RAND_RESET_I(32);
    __PVT__icpu_ack_i = VL_RAND_RESET_I(1);
    __PVT__icpu_rty_i = VL_RAND_RESET_I(1);
    __PVT__icpu_err_i = VL_RAND_RESET_I(1);
    __PVT__icpu_adr_i = VL_RAND_RESET_I(32);
    __PVT__icpu_tag_i = VL_RAND_RESET_I(4);
    __PVT__immu_en = VL_RAND_RESET_I(1);
    __PVT__ex_insn = VL_RAND_RESET_I(32);
    __PVT__ex_freeze = VL_RAND_RESET_I(1);
    __PVT__id_pc = VL_RAND_RESET_I(32);
    __PVT__branch_op = VL_RAND_RESET_I(3);
    __PVT__du_stall = VL_RAND_RESET_I(1);
    __PVT__du_addr = VL_RAND_RESET_I(32);
    __PVT__du_dat_du = VL_RAND_RESET_I(32);
    __PVT__du_read = VL_RAND_RESET_I(1);
    __PVT__du_write = VL_RAND_RESET_I(1);
    __PVT__du_dsr = VL_RAND_RESET_I(14);
    __PVT__du_hwbkpt = VL_RAND_RESET_I(1);
    __PVT__du_except = VL_RAND_RESET_I(13);
    __PVT__du_dat_cpu = VL_RAND_RESET_I(32);
    __PVT__rf_dataw = VL_RAND_RESET_I(32);
    __PVT__dcpu_adr_o = VL_RAND_RESET_I(32);
    __PVT__dcpu_cycstb_o = VL_RAND_RESET_I(1);
    __PVT__dcpu_we_o = VL_RAND_RESET_I(1);
    __PVT__dcpu_sel_o = VL_RAND_RESET_I(4);
    __PVT__dcpu_tag_o = VL_RAND_RESET_I(4);
    __PVT__dcpu_dat_o = VL_RAND_RESET_I(32);
    __PVT__dcpu_dat_i = VL_RAND_RESET_I(32);
    __PVT__dcpu_ack_i = VL_RAND_RESET_I(1);
    __PVT__dcpu_rty_i = VL_RAND_RESET_I(1);
    __PVT__dcpu_err_i = VL_RAND_RESET_I(1);
    __PVT__dcpu_tag_i = VL_RAND_RESET_I(4);
    __PVT__dc_en = VL_RAND_RESET_I(1);
    __PVT__dmmu_en = VL_RAND_RESET_I(1);
    __PVT__supv = VL_RAND_RESET_I(1);
    __PVT__spr_dat_pic = VL_RAND_RESET_I(32);
    __PVT__spr_dat_tt = VL_RAND_RESET_I(32);
    __PVT__spr_dat_pm = VL_RAND_RESET_I(32);
    __PVT__spr_dat_dmmu = VL_RAND_RESET_I(32);
    __PVT__spr_dat_immu = VL_RAND_RESET_I(32);
    __PVT__spr_dat_du = VL_RAND_RESET_I(32);
    __PVT__spr_addr = VL_RAND_RESET_I(32);
    __PVT__spr_dat_cpu = VL_RAND_RESET_I(32);
    __PVT__spr_dat_npc = VL_RAND_RESET_I(32);
    __PVT__spr_cs = VL_RAND_RESET_I(32);
    __PVT__spr_we = VL_RAND_RESET_I(1);
    __PVT__sig_int = VL_RAND_RESET_I(1);
    __PVT__sig_tick = VL_RAND_RESET_I(1);
    __PVT__if_insn = VL_RAND_RESET_I(32);
    __PVT__genpc_freeze = VL_RAND_RESET_I(1);
    __PVT__if_freeze = VL_RAND_RESET_I(1);
    __PVT__id_freeze = VL_RAND_RESET_I(1);
    __PVT__wb_freeze = VL_RAND_RESET_I(1);
    __PVT__muxed_b = VL_RAND_RESET_I(32);
    __PVT__wb_forw = VL_RAND_RESET_I(32);
    __PVT__wbforw_valid = VL_RAND_RESET_I(1);
    __PVT__operand_a = VL_RAND_RESET_I(32);
    __PVT__operand_b = VL_RAND_RESET_I(32);
    __PVT__sprs_dataout = VL_RAND_RESET_I(32);
    __PVT__except_type = VL_RAND_RESET_I(4);
    __PVT__flushpipe = VL_RAND_RESET_I(1);
    __PVT__extend_flush = VL_RAND_RESET_I(1);
    __PVT__branch_taken = VL_RAND_RESET_I(1);
    __PVT__flagforw = VL_RAND_RESET_I(1);
    __PVT__flag_we = VL_RAND_RESET_I(1);
    __PVT__cyforw = VL_RAND_RESET_I(1);
    __PVT__cy_we = VL_RAND_RESET_I(1);
    __PVT__lsu_stall = VL_RAND_RESET_I(1);
    __PVT__pc_we = VL_RAND_RESET_I(1);
    __PVT__epcr = VL_RAND_RESET_I(32);
    __PVT__eear = VL_RAND_RESET_I(32);
    __PVT__esr = VL_RAND_RESET_I(16);
    __PVT__sr_we = VL_RAND_RESET_I(1);
    __PVT__to_sr = VL_RAND_RESET_I(16);
    __PVT__sr = VL_RAND_RESET_I(16);
    __PVT__except_start = VL_RAND_RESET_I(1);
    __PVT__if_stall = VL_RAND_RESET_I(1);
    __PVT__mult_mac_result = VL_RAND_RESET_I(32);
    __PVT__mac_stall = VL_RAND_RESET_I(1);
    __PVT__genpc_refetch = VL_RAND_RESET_I(1);
    __PVT__except_align = VL_RAND_RESET_I(1);
    __PVT__except_dtlbmiss = VL_RAND_RESET_I(1);
    __PVT__except_dmmufault = VL_RAND_RESET_I(1);
    __PVT__except_dbuserr = VL_RAND_RESET_I(1);
    __PVT__or1200_genpc__DOT__pcreg = VL_RAND_RESET_I(30);
    __PVT__or1200_genpc__DOT__pc = VL_RAND_RESET_I(32);
    __PVT__or1200_if__DOT__insn_saved = VL_RAND_RESET_I(32);
    __PVT__or1200_if__DOT__addr_saved = VL_RAND_RESET_I(32);
    __PVT__or1200_if__DOT__saved = VL_RAND_RESET_I(1);
    __PVT__or1200_operandmuxes__DOT__muxed_a = VL_RAND_RESET_I(32);
    __PVT__or1200_operandmuxes__DOT__saved_a = VL_RAND_RESET_I(1);
    __PVT__or1200_operandmuxes__DOT__saved_b = VL_RAND_RESET_I(1);
    __PVT__or1200_alu__DOT__comp_a = VL_RAND_RESET_I(32);
    __PVT__or1200_alu__DOT__comp_b = VL_RAND_RESET_I(32);
    __PVT__or1200_alu__DOT__result_and = VL_RAND_RESET_I(32);
    __PVT__or1200_mult_mac__DOT__mul_prod_r = VL_RAND_RESET_Q(64);
    __PVT__or1200_mult_mac__DOT__mac_op_r1 = VL_RAND_RESET_I(2);
    __PVT__or1200_mult_mac__DOT__mac_op_r2 = VL_RAND_RESET_I(2);
    __PVT__or1200_mult_mac__DOT__mac_op_r3 = VL_RAND_RESET_I(2);
    __PVT__or1200_mult_mac__DOT__mac_r = VL_RAND_RESET_Q(64);
    __PVT__or1200_mult_mac__DOT__x = VL_RAND_RESET_I(32);
    __PVT__or1200_mult_mac__DOT__y = VL_RAND_RESET_I(32);
    __PVT__or1200_mult_mac__DOT__div_free = VL_RAND_RESET_I(1);
    __PVT__or1200_mult_mac__DOT__div_tmp = VL_RAND_RESET_I(32);
    __PVT__or1200_mult_mac__DOT__div_cntr = VL_RAND_RESET_I(6);
    __PVT__or1200_mult_mac__DOT__or1200_gmultp2_32x32__DOT__p0 = VL_RAND_RESET_Q(64);
    __PVT__or1200_mult_mac__DOT__or1200_gmultp2_32x32__DOT__p1 = VL_RAND_RESET_Q(64);
    __PVT__or1200_sprs__DOT__read_spr = VL_RAND_RESET_I(1);
    __PVT__or1200_sprs__DOT__npc_sel = VL_RAND_RESET_I(1);
    __PVT__or1200_sprs__DOT__ppc_sel = VL_RAND_RESET_I(1);
    __PVT__or1200_sprs__DOT__sr_sel = VL_RAND_RESET_I(1);
    __PVT__or1200_sprs__DOT__epcr_sel = VL_RAND_RESET_I(1);
    __PVT__or1200_sprs__DOT__eear_sel = VL_RAND_RESET_I(1);
    __PVT__or1200_sprs__DOT__esr_sel = VL_RAND_RESET_I(1);
    __PVT__or1200_sprs__DOT__sprs_op = VL_RAND_RESET_I(4);
    __PVT__or1200_lsu__DOT__or1200_mem2reg__DOT__aligned = VL_RAND_RESET_I(32);
    __PVT__or1200_freeze__DOT__multicycle_cnt = VL_RAND_RESET_I(2);
    __PVT__or1200_freeze__DOT__flushpipe_r = VL_RAND_RESET_I(1);
    __PVT__or1200_except__DOT__ex_pc = VL_RAND_RESET_I(32);
    __PVT__or1200_except__DOT__wb_pc = VL_RAND_RESET_I(32);
    __PVT__or1200_except__DOT__id_exceptflags = VL_RAND_RESET_I(3);
    __PVT__or1200_except__DOT__ex_exceptflags = VL_RAND_RESET_I(3);
    __PVT__or1200_except__DOT__state = VL_RAND_RESET_I(3);
    __PVT__or1200_except__DOT__ex_dslot = VL_RAND_RESET_I(1);
    __PVT__or1200_except__DOT__except_trig = VL_RAND_RESET_I(13);
    __PVT__or1200_except__DOT__except_flushpipe = VL_RAND_RESET_I(1);
    __PVT__or1200_except__DOT__delayed_iee = VL_RAND_RESET_I(3);
    __PVT__or1200_except__DOT__int_pending = VL_RAND_RESET_I(1);
    __PVT__or1200_except__DOT__tick_pending = VL_RAND_RESET_I(1);
    __Vdly__or1200_if__DOT__saved = VL_RAND_RESET_I(1);
    __Vdly__or1200_operandmuxes__DOT__saved_a = VL_RAND_RESET_I(1);
    __Vdly__or1200_operandmuxes__DOT__saved_b = VL_RAND_RESET_I(1);
    __Vdly__or1200_mult_mac__DOT__mul_prod_r = VL_RAND_RESET_Q(64);
    __Vdly__or1200_mult_mac__DOT__div_free = VL_RAND_RESET_I(1);
    __Vdly__or1200_mult_mac__DOT__div_cntr = VL_RAND_RESET_I(6);
    __Vdly__or1200_mult_mac__DOT__mac_r = VL_RAND_RESET_Q(64);
    __Vdly__or1200_freeze__DOT__multicycle_cnt = VL_RAND_RESET_I(2);
    __Vdly__or1200_except__DOT__delayed_iee = VL_RAND_RESET_I(3);
    __Vdly__or1200_except__DOT__ex_dslot = VL_RAND_RESET_I(1);
    __Vdly__or1200_except__DOT__state = VL_RAND_RESET_I(3);
    __Vdly__except_type = VL_RAND_RESET_I(4);
    __Vdly__extend_flush = VL_RAND_RESET_I(1);
}

void Vor1200_top_or1200_cpu::__Vconfigure(Vor1200_top__Syms* vlSymsp, bool first) {
    if (0 && first) {}  // Prevent unused
    this->__VlSymsp = vlSymsp;
}

Vor1200_top_or1200_cpu::~Vor1200_top_or1200_cpu() {
}

//--------------------
// Internal Methods

void Vor1200_top_or1200_cpu::_sequent__TOP__v__or1200_cpu__1(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"        Vor1200_top_or1200_cpu::_sequent__TOP__v__or1200_cpu__1"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_freeze__DOT__multicycle_cnt 
	= vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_freeze__DOT__multicycle_cnt;
    vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_if__DOT__saved 
	= vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_if__DOT__saved;
    vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_operandmuxes__DOT__saved_b 
	= vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_operandmuxes__DOT__saved_b;
    vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_except__DOT__ex_dslot 
	= vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_dslot;
    vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_except__DOT__delayed_iee 
	= vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__delayed_iee;
    vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_operandmuxes__DOT__saved_a 
	= vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_operandmuxes__DOT__saved_a;
    vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_mult_mac__DOT__mac_r 
	= vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__mac_r;
    vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_except__DOT__state 
	= vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__state;
    vlSymsp->TOP__v__or1200_cpu.__Vdly__extend_flush 
	= vlSymsp->TOP__v__or1200_cpu.__PVT__extend_flush;
    vlSymsp->TOP__v__or1200_cpu.__Vdly__except_type 
	= vlSymsp->TOP__v__or1200_cpu.__PVT__except_type;
    vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_mult_mac__DOT__div_free 
	= vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__div_free;
    vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_mult_mac__DOT__div_cntr 
	= vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__div_cntr;
    vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_mult_mac__DOT__mul_prod_r 
	= vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__mul_prod_r;
    // ALWAYS at ../or1k-verilog-rtl/or1200_freeze.v:178
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_freeze__DOT__flushpipe_r = 0;
    } else {
	if (((IData)(vlSymsp->TOP__v.__PVT__icpu_ack_qmem) 
	     | (IData)(vlSymsp->TOP__v.__PVT__icpu_err_immu))) {
	    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_freeze__DOT__flushpipe_r 
		= vlSymsp->TOP__v__or1200_cpu.__PVT__flushpipe;
	} else {
	    if ((1 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__flushpipe)))) {
		vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_freeze__DOT__flushpipe_r = 0;
	    }
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_wbmux.v:120
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu.__PVT__wb_forw = 0;
	vlSymsp->TOP__v__or1200_cpu.__PVT__wbforw_valid = 0;
    } else {
	if ((1 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)))) {
	    vlSymsp->TOP__v__or1200_cpu.__PVT__wb_forw 
		= vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw;
	    vlSymsp->TOP__v__or1200_cpu.__PVT__wbforw_valid 
		= (1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rfwb_op));
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_freeze.v:195
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_freeze__DOT__multicycle_cnt = 0;
    } else {
	if ((0 != (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_freeze__DOT__multicycle_cnt))) {
	    vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_freeze__DOT__multicycle_cnt 
		= (3 & ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_freeze__DOT__multicycle_cnt) 
			- (IData)(1)));
	} else {
	    if (((0 != (3 & ((0x38 == (0x3f & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
					       >> 0x1a)))
			      ? (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
				 >> 8) : 0))) & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)))) {
		vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_freeze__DOT__multicycle_cnt 
		    = (3 & ((0x38 == (0x3f & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
					      >> 0x1a)))
			     ? (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
				>> 8) : 0));
	    }
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_if.v:166
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_if__DOT__insn_saved = 0x14410000;
    } else {
	if (vlSymsp->TOP__v__or1200_cpu.__PVT__flushpipe) {
	    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_if__DOT__insn_saved = 0x14410000;
	} else {
	    if ((((IData)(vlSymsp->TOP__v.__PVT__icpu_ack_qmem) 
		  & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__if_freeze)) 
		 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_if__DOT__saved)))) {
		vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_if__DOT__insn_saved 
		    = vlSymsp->TOP__v.__PVT__icpu_dat_qmem;
	    } else {
		if ((1 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__if_freeze)))) {
		    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_if__DOT__insn_saved = 0x14410000;
		}
	    }
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_if.v:153
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_if__DOT__saved = 0;
    } else {
	if (vlSymsp->TOP__v__or1200_cpu.__PVT__flushpipe) {
	    vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_if__DOT__saved = 0;
	} else {
	    if ((((IData)(vlSymsp->TOP__v.__PVT__icpu_ack_qmem) 
		  & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__if_freeze)) 
		 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_if__DOT__saved)))) {
		vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_if__DOT__saved = 1;
	    } else {
		if ((1 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__if_freeze)))) {
		    vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_if__DOT__saved = 0;
		}
	    }
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_mult_mac.v:321
    vlSymsp->TOP__v__or1200_cpu.__PVT__mac_stall = 
	((~ (IData)(vlTOPp->rst_i)) & (((((0 != (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__mac_op)) 
					  | (0 != (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__mac_op_r1))) 
					 | (0 != (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__mac_op_r2))) 
					& (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_macrc_op)) 
				       | (0 != (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__div_cntr))));
    // ALWAYS at ../or1k-verilog-rtl/or1200_except.v:320
    vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_except__DOT__delayed_iee 
	= ((IData)(vlTOPp->rst_i) ? 0 : ((4 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr))
					  ? (1 | (6 
						  & ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__delayed_iee) 
						     << 1)))
					  : 0));
    // ALWAYS at ../or1k-verilog-rtl/or1200_genpc.v:307
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pcreg 
	    = (0x3fffffff & (((0x100 | (((0x4000 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr))
					  ? 0xf0000
					  : 0) << 0xc)) 
			      - (IData)(1)) >> 2));
    } else {
	if (vlSymsp->TOP__v__or1200_cpu.__PVT__pc_we) {
	    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pcreg 
		= (0x3fffffff & (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu 
				 >> 2));
	} else {
	    if ((1 & (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__no_more_dslot) 
		       | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__except_start)) 
		      | (((~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__genpc_freeze)) 
			  & (~ (IData)(vlSymsp->TOP__v.__PVT__icpu_rty_immu))) 
			 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__genpc_refetch)))))) {
		vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pcreg 
		    = (0x3fffffff & (vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pc 
				     >> 2));
	    }
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_mult_mac.v:300
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_mult_mac__DOT__mac_r = VL_ULL(0);
    } else {
	if ((((vlSymsp->TOP__v__or1200_cpu.__PVT__spr_cs 
	       >> 5) & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we)) 
	     & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)) {
	    vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_mult_mac__DOT__mac_r 
		= ((VL_ULL(0xffffffff00000000) & vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_mult_mac__DOT__mac_r) 
		   | (IData)((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu)));
	} else {
	    if ((((vlSymsp->TOP__v__or1200_cpu.__PVT__spr_cs 
		   >> 5) & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we)) 
		 & (~ vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr))) {
		vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_mult_mac__DOT__mac_r 
		    = ((VL_ULL(0xffffffff) & vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_mult_mac__DOT__mac_r) 
		       | ((QData)((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu)) 
			  << 0x20));
	    } else {
		if ((1 == (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__mac_op_r3))) {
		    vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_mult_mac__DOT__mac_r 
			= (vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__mac_r 
			   + vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__mul_prod_r);
		} else {
		    if ((2 == (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__mac_op_r3))) {
			vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_mult_mac__DOT__mac_r 
			    = (vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__mac_r 
			       - vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__mul_prod_r);
		    } else {
			if (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_macrc_op) 
			     & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)))) {
			    vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_mult_mac__DOT__mac_r = VL_ULL(0);
			}
		    }
		}
	    }
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_operandmuxes.v:117
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a = 0;
	vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_operandmuxes__DOT__saved_a = 0;
    } else {
	if ((((~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)) 
	      & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze)) 
	     & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_operandmuxes__DOT__saved_a)))) {
	    vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a 
		= vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_operandmuxes__DOT__muxed_a;
	    vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_operandmuxes__DOT__saved_a = 1;
	} else {
	    if ((1 & ((~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)) 
		      & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_operandmuxes__DOT__saved_a))))) {
		vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a 
		    = vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_operandmuxes__DOT__muxed_a;
	    } else {
		if ((1 & ((~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)) 
			  & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze))))) {
		    vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_operandmuxes__DOT__saved_a = 0;
		}
	    }
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_except.v:408
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_except__DOT__state = 0;
	vlSymsp->TOP__v__or1200_cpu.__Vdly__except_type = 0;
	vlSymsp->TOP__v__or1200_cpu.__Vdly__extend_flush = 0;
	vlSymsp->TOP__v__or1200_cpu.__PVT__epcr = 0;
	vlSymsp->TOP__v__or1200_cpu.__PVT__eear = 0;
	vlSymsp->TOP__v__or1200_cpu.__PVT__esr = 0x8001;
    } else {
	if ((4 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__state))) {
	    if ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__state))) {
		if ((1 & ((~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__if_stall)) 
			  & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze))))) {
		    vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_except__DOT__state = 0;
		    vlSymsp->TOP__v__or1200_cpu.__Vdly__except_type = 0;
		}
	    } else {
		if ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__state))) {
		    if ((1 & ((~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__if_stall)) 
			      & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze))))) {
			vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_except__DOT__state = 0;
			vlSymsp->TOP__v__or1200_cpu.__Vdly__except_type = 0;
		    }
		} else {
		    vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_except__DOT__state = 5;
		    vlSymsp->TOP__v__or1200_cpu.__Vdly__extend_flush = 0;
		}
	    }
	} else {
	    if ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__state))) {
		if ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__state))) {
		    vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_except__DOT__state = 4;
		} else {
		    if ((0xe == (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__except_type))) {
			vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_except__DOT__state = 0;
			vlSymsp->TOP__v__or1200_cpu.__Vdly__extend_flush = 0;
			vlSymsp->TOP__v__or1200_cpu.__Vdly__except_type = 0;
		    } else {
			vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_except__DOT__state = 3;
		    }
		}
	    } else {
		if ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__state))) {
		    if ((((IData)(vlSymsp->TOP__v.__PVT__icpu_ack_qmem) 
			  | (IData)(vlSymsp->TOP__v.__PVT__icpu_err_immu)) 
			 | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__genpc_freeze))) {
			vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_except__DOT__state = 2;
		    }
		} else {
		    if (vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__except_flushpipe) {
			vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_except__DOT__state = 1;
			vlSymsp->TOP__v__or1200_cpu.__Vdly__extend_flush = 1;
			vlSymsp->TOP__v__or1200_cpu.__PVT__esr 
			    = ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr_we)
			        ? (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__to_sr)
			        : (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr));
			if (((((((((0x1000 == (0x1000 
					       & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__except_trig))) 
				   | (0x800 == (0x1800 
						& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__except_trig)))) 
				  | (0x400 == (0x1c00 
					       & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__except_trig)))) 
				 | (0x200 == (0x1e00 
					      & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__except_trig)))) 
				| (0x100 == (0x1f00 
					     & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__except_trig)))) 
			       | (0x80 == (0x1f80 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__except_trig)))) 
			      | (0x40 == (0x1fc0 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__except_trig)))) 
			     | (0x20 == (0x1fe0 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__except_trig))))) {
			    if ((0x1000 == (0x1000 
					    & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__except_trig)))) {
				vlSymsp->TOP__v__or1200_cpu.__Vdly__except_type = 5;
				vlSymsp->TOP__v__or1200_cpu.__PVT__epcr 
				    = ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_dslot)
				        ? vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__wb_pc
				        : vlSymsp->TOP__v__or1200_cpu.__PVT__id_pc);
			    } else {
				if ((0x800 == (0x1800 
					       & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__except_trig)))) {
				    vlSymsp->TOP__v__or1200_cpu.__Vdly__except_type = 8;
				    vlSymsp->TOP__v__or1200_cpu.__PVT__epcr 
					= ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_dslot)
					    ? vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__wb_pc
					    : vlSymsp->TOP__v__or1200_cpu.__PVT__id_pc);
				} else {
				    if ((0x400 == (0x1c00 
						   & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__except_trig)))) {
					vlSymsp->TOP__v__or1200_cpu.__Vdly__except_type = 0xa;
					vlSymsp->TOP__v__or1200_cpu.__PVT__eear 
					    = vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_pc;
					vlSymsp->TOP__v__or1200_cpu.__PVT__epcr 
					    = ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_dslot)
					        ? vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__wb_pc
					        : vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_pc);
				    } else {
					if ((0x200 
					     == (0x1e00 
						 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__except_trig)))) {
					    vlSymsp->TOP__v__or1200_cpu.__Vdly__except_type = 4;
					    vlSymsp->TOP__v__or1200_cpu.__PVT__eear 
						= ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_dslot)
						    ? vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_pc
						    : vlSymsp->TOP__v__or1200_cpu.__PVT__id_pc);
					    vlSymsp->TOP__v__or1200_cpu.__PVT__epcr 
						= ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_dslot)
						    ? vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__wb_pc
						    : vlSymsp->TOP__v__or1200_cpu.__PVT__id_pc);
					} else {
					    if ((0x100 
						 == 
						 (0x1f00 
						  & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__except_trig)))) {
						vlSymsp->TOP__v__or1200_cpu.__Vdly__except_type = 2;
						vlSymsp->TOP__v__or1200_cpu.__PVT__eear 
						    = 
						    ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_dslot)
						      ? vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__wb_pc
						      : vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_pc);
						vlSymsp->TOP__v__or1200_cpu.__PVT__epcr 
						    = 
						    ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_dslot)
						      ? vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__wb_pc
						      : vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_pc);
					    } else {
						if (
						    (0x80 
						     == 
						     (0x1f80 
						      & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__except_trig)))) {
						    vlSymsp->TOP__v__or1200_cpu.__Vdly__except_type = 7;
						    vlSymsp->TOP__v__or1200_cpu.__PVT__eear 
							= vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_pc;
						    vlSymsp->TOP__v__or1200_cpu.__PVT__epcr 
							= 
							((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_dslot)
							  ? vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__wb_pc
							  : vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_pc);
						} else {
						    if (
							(0x40 
							 == 
							 (0x1fc0 
							  & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__except_trig)))) {
							vlSymsp->TOP__v__or1200_cpu.__Vdly__except_type = 6;
							vlSymsp->TOP__v__or1200_cpu.__PVT__eear 
							    = vlSymsp->TOP__v__or1200_cpu.__PVT__dcpu_adr_o;
							vlSymsp->TOP__v__or1200_cpu.__PVT__epcr 
							    = 
							    ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_dslot)
							      ? vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__wb_pc
							      : vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_pc);
						    } else {
							vlSymsp->TOP__v__or1200_cpu.__Vdly__except_type = 9;
							vlSymsp->TOP__v__or1200_cpu.__PVT__eear 
							    = vlSymsp->TOP__v__or1200_cpu.__PVT__dcpu_adr_o;
							vlSymsp->TOP__v__or1200_cpu.__PVT__epcr 
							    = 
							    ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_dslot)
							      ? vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__wb_pc
							      : vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_pc);
						    }
						}
					    }
					}
				    }
				}
			    }
			} else {
			    if ((0x10 == (0x1ff0 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__except_trig)))) {
				vlSymsp->TOP__v__or1200_cpu.__Vdly__except_type = 3;
				vlSymsp->TOP__v__or1200_cpu.__PVT__eear 
				    = vlSymsp->TOP__v__or1200_cpu.__PVT__dcpu_adr_o;
				vlSymsp->TOP__v__or1200_cpu.__PVT__epcr 
				    = ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_dslot)
				        ? vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__wb_pc
				        : vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_pc);
			    } else {
				if ((8 == (0x1ff8 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__except_trig)))) {
				    vlSymsp->TOP__v__or1200_cpu.__Vdly__except_type = 2;
				    vlSymsp->TOP__v__or1200_cpu.__PVT__eear 
					= vlSymsp->TOP__v__or1200_cpu.__PVT__dcpu_adr_o;
				    vlSymsp->TOP__v__or1200_cpu.__PVT__epcr 
					= ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_dslot)
					    ? vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__wb_pc
					    : vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_pc);
				} else {
				    if ((4 == (0x1ffc 
					       & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__except_trig)))) {
					vlSymsp->TOP__v__or1200_cpu.__Vdly__except_type = 0xb;
					vlSymsp->TOP__v__or1200_cpu.__PVT__epcr 
					    = ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_dslot)
					        ? vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__wb_pc
					        : vlSymsp->TOP__v__or1200_cpu.__PVT__id_pc);
				    } else {
					if ((2 == (0x1ffe 
						   & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__except_trig)))) {
					    vlSymsp->TOP__v__or1200_cpu.__Vdly__except_type = 0xe;
					    vlSymsp->TOP__v__or1200_cpu.__PVT__epcr 
						= ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_dslot)
						    ? vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__wb_pc
						    : vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_pc);
					} else {
					    if ((1 
						 == (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__except_trig))) {
						vlSymsp->TOP__v__or1200_cpu.__Vdly__except_type = 0xc;
						vlSymsp->TOP__v__or1200_cpu.__PVT__epcr 
						    = 
						    ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_dslot)
						      ? vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__wb_pc
						      : vlSymsp->TOP__v__or1200_cpu.__PVT__id_pc);
					    } else {
						vlSymsp->TOP__v__or1200_cpu.__Vdly__except_type = 0;
					    }
					}
				    }
				}
			    }
			}
		    } else {
			if (vlSymsp->TOP__v__or1200_cpu.__PVT__pc_we) {
			    vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_except__DOT__state = 1;
			    vlSymsp->TOP__v__or1200_cpu.__Vdly__extend_flush = 1;
			} else {
			    if (((0xe == (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sprs_op)) 
				 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__epcr_sel))) {
				vlSymsp->TOP__v__or1200_cpu.__PVT__epcr 
				    = vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b;
			    }
			    if (((0xe == (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sprs_op)) 
				 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__eear_sel))) {
				vlSymsp->TOP__v__or1200_cpu.__PVT__eear 
				    = vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b;
			    }
			    if (((0xe == (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sprs_op)) 
				 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__esr_sel))) {
				vlSymsp->TOP__v__or1200_cpu.__PVT__esr 
				    = (0x8000 | (0x7fff 
						 & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b));
			    }
			}
		    }
		}
	    }
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_mult_mac.v:235
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_mult_mac__DOT__mul_prod_r = VL_ULL(0);
	vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_mult_mac__DOT__div_free = 1;
	vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_mult_mac__DOT__div_cntr = 0;
    } else {
	if ((0 != (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__div_cntr))) {
	    vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_mult_mac__DOT__mul_prod_r 
		= ((0x80000000 & vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__div_tmp)
		    ? (VL_ULL(0xfffffffffffffffe) & 
		       (vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__mul_prod_r 
			<< 1)) : (VL_ULL(1) | (((QData)((IData)(
								(0x7fffffff 
								 & vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__div_tmp))) 
						<< 0x21) 
					       | ((QData)((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__mul_prod_r)) 
						  << 1))));
	    vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_mult_mac__DOT__div_cntr 
		= (0x3f & ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__div_cntr) 
			   - (IData)(1)));
	} else {
	    if ((((9 == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op)) 
		  | (0xa == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))) 
		 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__div_free))) {
		vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_mult_mac__DOT__mul_prod_r 
		    = ((QData)((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__x)) 
		       << 1);
		vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_mult_mac__DOT__div_cntr = 0x20;
		vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_mult_mac__DOT__div_free = 0;
	    } else {
		if ((1 & ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__div_free) 
			  | (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze))))) {
		    vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_mult_mac__DOT__mul_prod_r 
			= vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__or1200_gmultp2_32x32__DOT__p1;
		    vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_mult_mac__DOT__div_free = 1;
		}
	    }
	}
    }
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_freeze__DOT__multicycle_cnt 
	= vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_freeze__DOT__multicycle_cnt;
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__delayed_iee 
	= vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_except__DOT__delayed_iee;
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__mac_r 
	= vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_mult_mac__DOT__mac_r;
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_operandmuxes__DOT__saved_a 
	= vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_operandmuxes__DOT__saved_a;
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__state 
	= vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_except__DOT__state;
    vlSymsp->TOP__v__or1200_cpu.__PVT__except_type 
	= vlSymsp->TOP__v__or1200_cpu.__Vdly__except_type;
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__div_free 
	= vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_mult_mac__DOT__div_free;
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__div_cntr 
	= vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_mult_mac__DOT__div_cntr;
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__mul_prod_r 
	= vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_mult_mac__DOT__mul_prod_r;
    // ALWAYS at ../or1k-verilog-rtl/or1200_mult_mac.v:291
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__mac_op_r3 
	= ((IData)(vlTOPp->rst_i) ? 0 : (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__mac_op_r2));
    // ALWAYS at ../or1k-verilog-rtl/or1200_operandmuxes.v:133
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b = 0;
	vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_operandmuxes__DOT__saved_b = 0;
    } else {
	if ((((~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)) 
	      & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze)) 
	     & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_operandmuxes__DOT__saved_b)))) {
	    vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b 
		= vlSymsp->TOP__v__or1200_cpu.__PVT__muxed_b;
	    vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_operandmuxes__DOT__saved_b = 1;
	} else {
	    if ((1 & ((~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)) 
		      & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_operandmuxes__DOT__saved_b))))) {
		vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b 
		    = vlSymsp->TOP__v__or1200_cpu.__PVT__muxed_b;
	    } else {
		if ((1 & ((~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)) 
			  & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze))))) {
		    vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_operandmuxes__DOT__saved_b = 0;
		}
	    }
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_except.v:381
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__wb_pc = 0;
    } else {
	if ((1 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)))) {
	    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__wb_pc 
		= vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_pc;
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_gmultp2_32x32.v:121
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__or1200_gmultp2_32x32__DOT__p1 
	= ((IData)(vlTOPp->rst_i) ? VL_ULL(0) : vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__or1200_gmultp2_32x32__DOT__p0);
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_operandmuxes__DOT__saved_b 
	= vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_operandmuxes__DOT__saved_b;
    // ALWAYS at ../or1k-verilog-rtl/or1200_mult_mac.v:282
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__mac_op_r2 
	= ((IData)(vlTOPp->rst_i) ? 0 : (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__mac_op_r1));
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_alu__DOT__result_and 
	= (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a 
	   & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b);
    // ALWAYS at ../or1k-verilog-rtl/or1200_except.v:347
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_except__DOT__ex_dslot = 0;
	vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_pc = 0;
	vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_exceptflags = 0;
    } else {
	if (vlSymsp->TOP__v__or1200_cpu.__PVT__flushpipe) {
	    vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_except__DOT__ex_dslot = 0;
	    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_pc = 0;
	    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_exceptflags = 0;
	} else {
	    if (((~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)) 
		 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze))) {
		vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_pc 
		    = vlSymsp->TOP__v__or1200_cpu.__PVT__id_pc;
		vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_exceptflags = 0;
		vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_except__DOT__ex_dslot = 0;
	    } else {
		if ((1 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)))) {
		    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_pc 
			= vlSymsp->TOP__v__or1200_cpu.__PVT__id_pc;
		    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_exceptflags 
			= vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__id_exceptflags;
		    vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_except__DOT__ex_dslot 
			= vlSymsp->TOP__v__or1200_cpu.__PVT__branch_taken;
		}
	    }
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_gmultp2_32x32.v:112
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__or1200_gmultp2_32x32__DOT__p0 
	= ((IData)(vlTOPp->rst_i) ? VL_ULL(0) : VL_MULS_QQQ(64,64,64, 
							    VL_EXTENDS_QI(64,32, vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__x), 
							    VL_EXTENDS_QI(64,32, vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__y)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_dslot 
	= vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_except__DOT__ex_dslot;
    // ALWAYS at ../or1k-verilog-rtl/or1200_mult_mac.v:273
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__mac_op_r1 
	= ((IData)(vlTOPp->rst_i) ? 0 : (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__mac_op));
    // ALWAYS at ../or1k-verilog-rtl/or1200_except.v:297
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu.__PVT__id_pc = 0;
	vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__id_exceptflags = 0;
    } else {
	if (vlSymsp->TOP__v__or1200_cpu.__PVT__flushpipe) {
	    vlSymsp->TOP__v__or1200_cpu.__PVT__id_pc = 0;
	    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__id_exceptflags = 0;
	} else {
	    if ((1 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze)))) {
		vlSymsp->TOP__v__or1200_cpu.__PVT__id_pc 
		    = ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_if__DOT__saved)
		        ? vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_if__DOT__addr_saved
		        : vlSymsp->TOP__v.__PVT__icpu_adr_immu);
		vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__id_exceptflags 
		    = (((((IData)(vlSymsp->TOP__v.__PVT__icpu_err_immu) 
			  & (0xb == (IData)(vlSymsp->TOP__v.__PVT__icpu_tag_immu))) 
			 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__no_more_dslot))) 
			<< 2) | (((((IData)(vlSymsp->TOP__v.__PVT__icpu_err_immu) 
				    & (0xd == (IData)(vlSymsp->TOP__v.__PVT__icpu_tag_immu))) 
				   & (~ (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__no_more_dslot))) 
				  << 1) | (((IData)(vlSymsp->TOP__v.__PVT__icpu_err_immu) 
					    & (0xc 
					       == (IData)(vlSymsp->TOP__v.__PVT__icpu_tag_immu))) 
					   & (~ (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__no_more_dslot)))));
	    }
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_if.v:179
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_if__DOT__addr_saved = 0;
    } else {
	if (vlSymsp->TOP__v__or1200_cpu.__PVT__flushpipe) {
	    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_if__DOT__addr_saved = 0;
	} else {
	    if ((((IData)(vlSymsp->TOP__v.__PVT__icpu_ack_qmem) 
		  & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__if_freeze)) 
		 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_if__DOT__saved)))) {
		vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_if__DOT__addr_saved 
		    = vlSymsp->TOP__v.__PVT__icpu_adr_immu;
	    } else {
		if ((1 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__if_freeze)))) {
		    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_if__DOT__addr_saved 
			= vlSymsp->TOP__v.__PVT__icpu_adr_immu;
		}
	    }
	}
    }
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_if__DOT__saved 
	= vlSymsp->TOP__v__or1200_cpu.__Vdly__or1200_if__DOT__saved;
}

void Vor1200_top_or1200_cpu::_sequent__TOP__v__or1200_cpu__2(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"        Vor1200_top_or1200_cpu::_sequent__TOP__v__or1200_cpu__2"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_alu__DOT__comp_a 
	= ((0x80000000 & ((0x80000000 & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a) 
			  ^ (0x80000000 & ((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__comp_op) 
					   << 0x1c)))) 
	   | (0x7fffffff & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a));
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_alu__DOT__comp_b 
	= ((0x80000000 & ((0x80000000 & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b) 
			  ^ (0x80000000 & ((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__comp_op) 
					   << 0x1c)))) 
	   | (0x7fffffff & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b));
    // ALWAYS at ../or1k-verilog-rtl/or1200_mult_mac.v:192
    vlSymsp->TOP__v__or1200_cpu.__PVT__mult_mac_result 
	= ((9 == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))
	    ? ((0x80000000 & (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a 
			      ^ vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b))
	        ? ((IData)(1) + (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__mul_prod_r)))
	        : (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__mul_prod_r))
	    : (((0xa == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op)) 
		| (6 == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op)))
	        ? (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__mul_prod_r)
	        : (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__mac_r)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__x 
	= (((9 == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op)) 
	    & (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a 
	       >> 0x1f)) ? ((IData)(1) + (~ vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a))
	    : vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a);
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__y 
	= (((9 == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op)) 
	    & (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b 
	       >> 0x1f)) ? ((IData)(1) + (~ vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b))
	    : vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b);
}

void Vor1200_top_or1200_cpu::_combo__TOP__v__or1200_cpu__3(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"        Vor1200_top_or1200_cpu::_combo__TOP__v__or1200_cpu__3"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlSymsp->TOP__v__or1200_cpu.__PVT__genpc_freeze 
	= ((IData)(vlTOPp->dbg_stall_i) | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_freeze__DOT__flushpipe_r));
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sprs_op 
	= ((IData)(vlSymsp->TOP__v.__PVT__du_write)
	    ? 0xe : ((IData)(vlSymsp->TOP__v.__PVT__du_read)
		      ? 0xf : (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op)));
}

void Vor1200_top_or1200_cpu::_settle__TOP__v__or1200_cpu__4(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"        Vor1200_top_or1200_cpu::_settle__TOP__v__or1200_cpu__4"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlSymsp->TOP__v__or1200_cpu.__PVT__genpc_freeze 
	= ((IData)(vlTOPp->dbg_stall_i) | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_freeze__DOT__flushpipe_r));
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_alu__DOT__result_and 
	= (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a 
	   & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b);
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_alu__DOT__comp_a 
	= ((0x80000000 & ((0x80000000 & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a) 
			  ^ (0x80000000 & ((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__comp_op) 
					   << 0x1c)))) 
	   | (0x7fffffff & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a));
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_alu__DOT__comp_b 
	= ((0x80000000 & ((0x80000000 & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b) 
			  ^ (0x80000000 & ((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__comp_op) 
					   << 0x1c)))) 
	   | (0x7fffffff & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b));
    // ALWAYS at ../or1k-verilog-rtl/or1200_mult_mac.v:192
    vlSymsp->TOP__v__or1200_cpu.__PVT__mult_mac_result 
	= ((9 == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))
	    ? ((0x80000000 & (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a 
			      ^ vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b))
	        ? ((IData)(1) + (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__mul_prod_r)))
	        : (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__mul_prod_r))
	    : (((0xa == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op)) 
		| (6 == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op)))
	        ? (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__mul_prod_r)
	        : (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__mac_r)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__x 
	= (((9 == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op)) 
	    & (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a 
	       >> 0x1f)) ? ((IData)(1) + (~ vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a))
	    : vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a);
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__y 
	= (((9 == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op)) 
	    & (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b 
	       >> 0x1f)) ? ((IData)(1) + (~ vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b))
	    : vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b);
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sprs_op 
	= ((IData)(vlSymsp->TOP__v.__PVT__du_write)
	    ? 0xe : ((IData)(vlSymsp->TOP__v.__PVT__du_read)
		      ? 0xf : (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu 
	= ((IData)(vlSymsp->TOP__v.__PVT__du_write)
	    ? vlTOPp->dbg_dat_i : vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b);
    vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr = (
						   ((IData)(vlSymsp->TOP__v.__PVT__du_read) 
						    | (IData)(vlSymsp->TOP__v.__PVT__du_write))
						    ? vlTOPp->dbg_adr_i
						    : 
						   (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a 
						    | (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__spr_addrimm)));
    // ALWAYS at ../or1k-verilog-rtl/or1200_alu.v:300
    if ((0xd == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))) {
	vlSymsp->TOP__v__or1200_cpu.__PVT__flagforw 
	    = ((4 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__comp_op))
	        ? ((~ ((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__comp_op) 
		       >> 1)) & ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__comp_op))
				  ? (vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_alu__DOT__comp_a 
				     <= vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_alu__DOT__comp_b)
				  : (vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_alu__DOT__comp_a 
				     < vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_alu__DOT__comp_b)))
	        : ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__comp_op))
		    ? ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__comp_op))
		        ? (vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_alu__DOT__comp_a 
			   >= vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_alu__DOT__comp_b)
		        : (vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_alu__DOT__comp_a 
			   > vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_alu__DOT__comp_b))
		    : ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__comp_op))
		        ? (vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_alu__DOT__comp_a 
			   != vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_alu__DOT__comp_b)
		        : (vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_alu__DOT__comp_a 
			   == vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_alu__DOT__comp_b))));
	vlSymsp->TOP__v__or1200_cpu.__PVT__flag_we = 1;
    } else {
	vlSymsp->TOP__v__or1200_cpu.__PVT__flagforw = 0;
	vlSymsp->TOP__v__or1200_cpu.__PVT__flag_we = 0;
    }
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__div_tmp 
	= ((IData)((vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__mul_prod_r 
		    >> 0x20)) - vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__y);
    vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we = ((IData)(vlSymsp->TOP__v.__PVT__du_write) 
						 | (0xe 
						    == (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sprs_op)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__read_spr 
	= ((0xe != (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sprs_op)) 
	   & (0xf == (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sprs_op)));
}

void Vor1200_top_or1200_cpu::_sequent__TOP__v__or1200_cpu__5(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"        Vor1200_top_or1200_cpu::_sequent__TOP__v__or1200_cpu__5"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    // ALWAYS at ../or1k-verilog-rtl/or1200_alu.v:300
    if ((0xd == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))) {
	vlSymsp->TOP__v__or1200_cpu.__PVT__flagforw 
	    = ((4 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__comp_op))
	        ? ((~ ((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__comp_op) 
		       >> 1)) & ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__comp_op))
				  ? (vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_alu__DOT__comp_a 
				     <= vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_alu__DOT__comp_b)
				  : (vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_alu__DOT__comp_a 
				     < vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_alu__DOT__comp_b)))
	        : ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__comp_op))
		    ? ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__comp_op))
		        ? (vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_alu__DOT__comp_a 
			   >= vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_alu__DOT__comp_b)
		        : (vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_alu__DOT__comp_a 
			   > vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_alu__DOT__comp_b))
		    : ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__comp_op))
		        ? (vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_alu__DOT__comp_a 
			   != vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_alu__DOT__comp_b)
		        : (vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_alu__DOT__comp_a 
			   == vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_alu__DOT__comp_b))));
	vlSymsp->TOP__v__or1200_cpu.__PVT__flag_we = 1;
    } else {
	vlSymsp->TOP__v__or1200_cpu.__PVT__flagforw = 0;
	vlSymsp->TOP__v__or1200_cpu.__PVT__flag_we = 0;
    }
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__div_tmp 
	= ((IData)((vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__mul_prod_r 
		    >> 0x20)) - vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__y);
    // ALWAYS at ../or1k-verilog-rtl/or1200_sprs.v:365
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu.__PVT__sr = 0x8001;
    } else {
	if (((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__extend_flush) 
	     & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__except_start))) {
	    vlSymsp->TOP__v__or1200_cpu.__PVT__sr = 
		(1 | (0xfff8 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr)));
	    vlSymsp->TOP__v__or1200_cpu.__PVT__sr = 
		(0xff9f & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr));
	} else {
	    if (vlSymsp->TOP__v__or1200_cpu.__PVT__sr_we) {
		vlSymsp->TOP__v__or1200_cpu.__PVT__sr 
		    = vlSymsp->TOP__v__or1200_cpu.__PVT__to_sr;
	    }
	}
    }
    vlSymsp->TOP__v__or1200_cpu.__PVT__extend_flush 
	= vlSymsp->TOP__v__or1200_cpu.__Vdly__extend_flush;
    vlSymsp->TOP__v__or1200_cpu.__PVT__except_start 
	= ((0 != (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__except_type)) 
	   & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__extend_flush));
    // ALWAYS at ../or1k-verilog-rtl/or1200_alu.v:332
    if ((0 == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))) {
	vlSymsp->TOP__v__or1200_cpu.__PVT__cyforw = 
	    (1 & (IData)(((VL_ULL(0x1ffffffff) & ((QData)((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)) 
						  + (QData)((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b)))) 
			  >> 0x20)));
	vlSymsp->TOP__v__or1200_cpu.__PVT__cy_we = 1;
    } else {
	if ((1 == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))) {
	    vlSymsp->TOP__v__or1200_cpu.__PVT__cyforw 
		= (1 & (IData)(((VL_ULL(0x1ffffffff) 
				 & (((QData)((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)) 
				     + (QData)((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b))) 
				    + (QData)((IData)(
						      (1 
						       & ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr) 
							  >> 0xa)))))) 
				>> 0x20)));
	    vlSymsp->TOP__v__or1200_cpu.__PVT__cy_we = 1;
	} else {
	    vlSymsp->TOP__v__or1200_cpu.__PVT__cyforw = 0;
	    vlSymsp->TOP__v__or1200_cpu.__PVT__cy_we = 0;
	}
    }
}

void Vor1200_top_or1200_cpu::_combo__TOP__v__or1200_cpu__6(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"        Vor1200_top_or1200_cpu::_combo__TOP__v__or1200_cpu__6"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu 
	= ((IData)(vlSymsp->TOP__v.__PVT__du_write)
	    ? vlTOPp->dbg_dat_i : vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b);
    vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr = (
						   ((IData)(vlSymsp->TOP__v.__PVT__du_read) 
						    | (IData)(vlSymsp->TOP__v.__PVT__du_write))
						    ? vlTOPp->dbg_adr_i
						    : 
						   (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a 
						    | (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__spr_addrimm)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we = ((IData)(vlSymsp->TOP__v.__PVT__du_write) 
						 | (0xe 
						    == (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sprs_op)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__read_spr 
	= ((0xe != (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sprs_op)) 
	   & (0xf == (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sprs_op)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__spr_cs = (((0x8000 
						   & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						   ? 
						  ((0x4000 
						    & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						    ? 
						   ((0x2000 
						     & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						     ? 
						    ((0x1000 
						      & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						      ? 
						     ((0x800 
						       & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						       ? 0x80000000
						       : 0x40000000)
						      : 
						     ((0x800 
						       & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						       ? 0x20000000
						       : 0x10000000))
						     : 
						    ((0x1000 
						      & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						      ? 
						     ((0x800 
						       & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						       ? 0x8000000
						       : 0x4000000)
						      : 
						     ((0x800 
						       & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						       ? 0x2000000
						       : 0x1000000)))
						    : 
						   ((0x2000 
						     & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						     ? 
						    ((0x1000 
						      & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						      ? 
						     ((0x800 
						       & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						       ? 0x800000
						       : 0x400000)
						      : 
						     ((0x800 
						       & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						       ? 0x200000
						       : 0x100000))
						     : 
						    ((0x1000 
						      & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						      ? 
						     ((0x800 
						       & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						       ? 0x80000
						       : 0x40000)
						      : 
						     ((0x800 
						       & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						       ? 0x20000
						       : 0x10000))))
						   : 
						  ((0x4000 
						    & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						    ? 
						   ((0x2000 
						     & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						     ? 
						    ((0x1000 
						      & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						      ? 
						     ((0x800 
						       & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						       ? 0x8000
						       : 0x4000)
						      : 
						     ((0x800 
						       & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						       ? 0x2000
						       : 0x1000))
						     : 
						    ((0x1000 
						      & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						      ? 
						     ((0x800 
						       & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						       ? 0x800
						       : 0x400)
						      : 
						     ((0x800 
						       & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						       ? 0x200
						       : 0x100)))
						    : 
						   ((0x2000 
						     & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						     ? 
						    ((0x1000 
						      & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						      ? 
						     ((0x800 
						       & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						       ? 0x80
						       : 0x40)
						      : 
						     ((0x800 
						       & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						       ? 0x20
						       : 0x10))
						     : 
						    ((0x1000 
						      & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						      ? 
						     ((0x800 
						       & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						       ? 8
						       : 4)
						      : 
						     ((0x800 
						       & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						       ? 2
						       : 1))))) 
						 & VL_UNARYMIN_I((IData)(
									 ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__read_spr) 
									  | (0xe 
									     == (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sprs_op))))));
}

void Vor1200_top_or1200_cpu::_settle__TOP__v__or1200_cpu__7(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"        Vor1200_top_or1200_cpu::_settle__TOP__v__or1200_cpu__7"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlSymsp->TOP__v__or1200_cpu.__PVT__except_start 
	= ((0 != (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__except_type)) 
	   & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__extend_flush));
    // ALWAYS at ../or1k-verilog-rtl/or1200_alu.v:332
    if ((0 == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))) {
	vlSymsp->TOP__v__or1200_cpu.__PVT__cyforw = 
	    (1 & (IData)(((VL_ULL(0x1ffffffff) & ((QData)((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)) 
						  + (QData)((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b)))) 
			  >> 0x20)));
	vlSymsp->TOP__v__or1200_cpu.__PVT__cy_we = 1;
    } else {
	if ((1 == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))) {
	    vlSymsp->TOP__v__or1200_cpu.__PVT__cyforw 
		= (1 & (IData)(((VL_ULL(0x1ffffffff) 
				 & (((QData)((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)) 
				     + (QData)((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b))) 
				    + (QData)((IData)(
						      (1 
						       & ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr) 
							  >> 0xa)))))) 
				>> 0x20)));
	    vlSymsp->TOP__v__or1200_cpu.__PVT__cy_we = 1;
	} else {
	    vlSymsp->TOP__v__or1200_cpu.__PVT__cyforw = 0;
	    vlSymsp->TOP__v__or1200_cpu.__PVT__cy_we = 0;
	}
    }
    vlSymsp->TOP__v__or1200_cpu.__PVT__spr_cs = (((0x8000 
						   & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						   ? 
						  ((0x4000 
						    & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						    ? 
						   ((0x2000 
						     & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						     ? 
						    ((0x1000 
						      & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						      ? 
						     ((0x800 
						       & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						       ? 0x80000000
						       : 0x40000000)
						      : 
						     ((0x800 
						       & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						       ? 0x20000000
						       : 0x10000000))
						     : 
						    ((0x1000 
						      & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						      ? 
						     ((0x800 
						       & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						       ? 0x8000000
						       : 0x4000000)
						      : 
						     ((0x800 
						       & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						       ? 0x2000000
						       : 0x1000000)))
						    : 
						   ((0x2000 
						     & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						     ? 
						    ((0x1000 
						      & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						      ? 
						     ((0x800 
						       & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						       ? 0x800000
						       : 0x400000)
						      : 
						     ((0x800 
						       & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						       ? 0x200000
						       : 0x100000))
						     : 
						    ((0x1000 
						      & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						      ? 
						     ((0x800 
						       & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						       ? 0x80000
						       : 0x40000)
						      : 
						     ((0x800 
						       & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						       ? 0x20000
						       : 0x10000))))
						   : 
						  ((0x4000 
						    & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						    ? 
						   ((0x2000 
						     & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						     ? 
						    ((0x1000 
						      & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						      ? 
						     ((0x800 
						       & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						       ? 0x8000
						       : 0x4000)
						      : 
						     ((0x800 
						       & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						       ? 0x2000
						       : 0x1000))
						     : 
						    ((0x1000 
						      & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						      ? 
						     ((0x800 
						       & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						       ? 0x800
						       : 0x400)
						      : 
						     ((0x800 
						       & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						       ? 0x200
						       : 0x100)))
						    : 
						   ((0x2000 
						     & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						     ? 
						    ((0x1000 
						      & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						      ? 
						     ((0x800 
						       & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						       ? 0x80
						       : 0x40)
						      : 
						     ((0x800 
						       & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						       ? 0x20
						       : 0x10))
						     : 
						    ((0x1000 
						      & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						      ? 
						     ((0x800 
						       & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						       ? 8
						       : 4)
						      : 
						     ((0x800 
						       & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						       ? 2
						       : 1))))) 
						 & VL_UNARYMIN_I((IData)(
									 ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__read_spr) 
									  | (0xe 
									     == (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sprs_op))))));
    vlSymsp->TOP__v__or1200_cpu.__PVT__dcpu_adr_o = 
	(vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a 
	 + vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_addrofs);
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__esr_sel 
	= (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_cs 
	   & (0x40 == (0x7ff & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__eear_sel 
	= (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_cs 
	   & (0x30 == (0x7ff & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__epcr_sel 
	= (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_cs 
	   & (0x20 == (0x7ff & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sr_sel 
	= (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_cs 
	   & (0x11 == (0x7ff & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__npc_sel 
	= (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_cs 
	   & (0x10 == (0x7ff & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__ppc_sel 
	= (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_cs 
	   & (0x12 == (0x7ff & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)));
}

void Vor1200_top_or1200_cpu::_sequent__TOP__v__or1200_cpu__8(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"        Vor1200_top_or1200_cpu::_sequent__TOP__v__or1200_cpu__8"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlSymsp->TOP__v__or1200_cpu.__PVT__dcpu_adr_o = 
	(vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a 
	 + vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_addrofs);
    vlSymsp->TOP__v__or1200_cpu.__PVT__dcpu_dat_o = 
	((0xff000000 & (((0x28 == (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op) 
				    << 2) | (3 & vlSymsp->TOP__v__or1200_cpu.__PVT__dcpu_adr_o)))
			  ? vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b
			  : ((0x30 == (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op) 
					<< 2) | (3 
						 & vlSymsp->TOP__v__or1200_cpu.__PVT__dcpu_adr_o)))
			      ? (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b 
				 >> 8) : (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b 
					  >> 0x18))) 
			<< 0x18)) | ((0xff0000 & ((
						   (0x38 
						    == 
						    (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op) 
						      << 2) 
						     | (3 
							& vlSymsp->TOP__v__or1200_cpu.__PVT__dcpu_adr_o)))
						    ? 
						   (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b 
						    >> 0x10)
						    : vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b) 
						  << 0x10)) 
				     | ((0xff00 & (
						   ((0x2a 
						     == 
						     (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op) 
						       << 2) 
						      | (3 
							 & vlSymsp->TOP__v__or1200_cpu.__PVT__dcpu_adr_o)))
						     ? vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b
						     : 
						    (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b 
						     >> 8)) 
						   << 8)) 
					| (0xff & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b))));
    vlSymsp->TOP__v__or1200_cpu.__PVT__except_align 
	= (((((0xc == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op)) 
	      | (4 == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op))) 
	     | (5 == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op))) 
	    & vlSymsp->TOP__v__or1200_cpu.__PVT__dcpu_adr_o) 
	   | ((((0xe == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op)) 
		| (6 == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op))) 
	       | (7 == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op))) 
	      & (0 != (3 & vlSymsp->TOP__v__or1200_cpu.__PVT__dcpu_adr_o))));
}

void Vor1200_top_or1200_cpu::_combo__TOP__v__or1200_cpu__9(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"        Vor1200_top_or1200_cpu::_combo__TOP__v__or1200_cpu__9"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__esr_sel 
	= (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_cs 
	   & (0x40 == (0x7ff & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__eear_sel 
	= (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_cs 
	   & (0x30 == (0x7ff & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__epcr_sel 
	= (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_cs 
	   & (0x20 == (0x7ff & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sr_sel 
	= (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_cs 
	   & (0x11 == (0x7ff & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__ppc_sel 
	= (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_cs 
	   & (0x12 == (0x7ff & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__npc_sel 
	= (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_cs 
	   & (0x10 == (0x7ff & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__to_sr = ((0x7ff 
						 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__to_sr)) 
						| (0xf800 
						   & (((6 
							== (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__branch_op))
						        ? 
						       ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__esr) 
							>> 0xb)
						        : 
						       (((0xe 
							  == (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sprs_op)) 
							 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sr_sel))
							 ? 
							(0x10 
							 | (0xf 
							    & (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu 
							       >> 0xb)))
							 : 
							((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr) 
							 >> 0xb))) 
						      << 0xb)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__to_sr = ((0xfe00 
						 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__to_sr)) 
						| (0x1ff 
						   & ((6 
						       == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__branch_op))
						       ? (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__esr)
						       : 
						      (((0xe 
							 == (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sprs_op)) 
							& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sr_sel))
						        ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu
						        : (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr)))));
    vlSymsp->TOP__v__or1200_cpu.__PVT__to_sr = ((0xfbff 
						 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__to_sr)) 
						| (0x400 
						   & (((6 
							== (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__branch_op))
						        ? 
						       ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__esr) 
							>> 0xa)
						        : 
						       ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__cy_we)
							 ? (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__cyforw)
							 : 
							(((0xe 
							   == (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sprs_op)) 
							  & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sr_sel))
							  ? 
							 (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu 
							  >> 0xa)
							  : 
							 ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr) 
							  >> 0xa)))) 
						      << 0xa)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__to_sr = ((0xfdff 
						 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__to_sr)) 
						| (0x200 
						   & (((6 
							== (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__branch_op))
						        ? 
						       ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__esr) 
							>> 9)
						        : 
						       ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__flag_we)
							 ? (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__flagforw)
							 : 
							(((0xe 
							   == (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sprs_op)) 
							  & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sr_sel))
							  ? 
							 (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu 
							  >> 9)
							  : 
							 ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr) 
							  >> 9)))) 
						      << 9)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__sr_we = ((((
						   (0xe 
						    == (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sprs_op)) 
						   & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sr_sel)) 
						  | (6 
						     == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__branch_op))) 
						 | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__flag_we)) 
						| (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__cy_we));
    vlSymsp->TOP__v__or1200_cpu.__PVT__pc_we = ((0xe 
						 == (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sprs_op)) 
						& ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__npc_sel) 
						   | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__ppc_sel)));
}

void Vor1200_top_or1200_cpu::_settle__TOP__v__or1200_cpu__10(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"        Vor1200_top_or1200_cpu::_settle__TOP__v__or1200_cpu__10"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlSymsp->TOP__v__or1200_cpu.__PVT__dcpu_dat_o = 
	((0xff000000 & (((0x28 == (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op) 
				    << 2) | (3 & vlSymsp->TOP__v__or1200_cpu.__PVT__dcpu_adr_o)))
			  ? vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b
			  : ((0x30 == (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op) 
					<< 2) | (3 
						 & vlSymsp->TOP__v__or1200_cpu.__PVT__dcpu_adr_o)))
			      ? (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b 
				 >> 8) : (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b 
					  >> 0x18))) 
			<< 0x18)) | ((0xff0000 & ((
						   (0x38 
						    == 
						    (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op) 
						      << 2) 
						     | (3 
							& vlSymsp->TOP__v__or1200_cpu.__PVT__dcpu_adr_o)))
						    ? 
						   (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b 
						    >> 0x10)
						    : vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b) 
						  << 0x10)) 
				     | ((0xff00 & (
						   ((0x2a 
						     == 
						     (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op) 
						       << 2) 
						      | (3 
							 & vlSymsp->TOP__v__or1200_cpu.__PVT__dcpu_adr_o)))
						     ? vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b
						     : 
						    (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b 
						     >> 8)) 
						   << 8)) 
					| (0xff & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b))));
    vlSymsp->TOP__v__or1200_cpu.__PVT__except_align 
	= (((((0xc == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op)) 
	      | (4 == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op))) 
	     | (5 == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op))) 
	    & vlSymsp->TOP__v__or1200_cpu.__PVT__dcpu_adr_o) 
	   | ((((0xe == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op)) 
		| (6 == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op))) 
	       | (7 == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op))) 
	      & (0 != (3 & vlSymsp->TOP__v__or1200_cpu.__PVT__dcpu_adr_o))));
    vlSymsp->TOP__v__or1200_cpu.__PVT__to_sr = ((0x7ff 
						 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__to_sr)) 
						| (0xf800 
						   & (((6 
							== (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__branch_op))
						        ? 
						       ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__esr) 
							>> 0xb)
						        : 
						       (((0xe 
							  == (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sprs_op)) 
							 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sr_sel))
							 ? 
							(0x10 
							 | (0xf 
							    & (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu 
							       >> 0xb)))
							 : 
							((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr) 
							 >> 0xb))) 
						      << 0xb)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__to_sr = ((0xfe00 
						 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__to_sr)) 
						| (0x1ff 
						   & ((6 
						       == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__branch_op))
						       ? (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__esr)
						       : 
						      (((0xe 
							 == (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sprs_op)) 
							& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sr_sel))
						        ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu
						        : (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr)))));
    vlSymsp->TOP__v__or1200_cpu.__PVT__to_sr = ((0xfbff 
						 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__to_sr)) 
						| (0x400 
						   & (((6 
							== (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__branch_op))
						        ? 
						       ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__esr) 
							>> 0xa)
						        : 
						       ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__cy_we)
							 ? (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__cyforw)
							 : 
							(((0xe 
							   == (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sprs_op)) 
							  & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sr_sel))
							  ? 
							 (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu 
							  >> 0xa)
							  : 
							 ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr) 
							  >> 0xa)))) 
						      << 0xa)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__to_sr = ((0xfdff 
						 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__to_sr)) 
						| (0x200 
						   & (((6 
							== (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__branch_op))
						        ? 
						       ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__esr) 
							>> 9)
						        : 
						       ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__flag_we)
							 ? (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__flagforw)
							 : 
							(((0xe 
							   == (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sprs_op)) 
							  & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sr_sel))
							  ? 
							 (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu 
							  >> 9)
							  : 
							 ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr) 
							  >> 9)))) 
						      << 9)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__sr_we = ((((
						   (0xe 
						    == (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sprs_op)) 
						   & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sr_sel)) 
						  | (6 
						     == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__branch_op))) 
						 | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__flag_we)) 
						| (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__cy_we));
    vlSymsp->TOP__v__or1200_cpu.__PVT__pc_we = ((0xe 
						 == (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sprs_op)) 
						& ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__npc_sel) 
						   | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__ppc_sel)));
    // ALWAYS at ../or1k-verilog-rtl/or1200_genpc.v:201
    if (vlSymsp->TOP__v__or1200_cpu.__PVT__pc_we) {
	vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pc 
	    = vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu;
	vlSymsp->TOP__v__or1200_cpu.__PVT__branch_taken = 0;
    } else {
	if (vlSymsp->TOP__v__or1200_cpu.__PVT__except_start) {
	    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pc 
		= ((((0x4000 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr))
		      ? 0xf0000 : 0) << 0xc) | ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__except_type) 
						<< 8));
	    vlSymsp->TOP__v__or1200_cpu.__PVT__branch_taken = 1;
	} else {
	    if ((4 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__branch_op))) {
		if ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__branch_op))) {
		    if ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__branch_op))) {
			vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pc 
			    = vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu;
			vlSymsp->TOP__v__or1200_cpu.__PVT__branch_taken = 0;
		    } else {
			vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pc 
			    = vlSymsp->TOP__v__or1200_cpu.__PVT__epcr;
			vlSymsp->TOP__v__or1200_cpu.__PVT__branch_taken = 1;
		    }
		} else {
		    if ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__branch_op))) {
			if ((0x200 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr))) {
			    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pc 
				= (0xfffffffc & (((IData)(1) 
						  + vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pcreg) 
						 << 2));
			    vlSymsp->TOP__v__or1200_cpu.__PVT__branch_taken = 0;
			} else {
			    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pc 
				= (0xfffffffc & (((vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_pc 
						   >> 2) 
						  + 
						  ((0x3c000000 
						    & (VL_UNARYMIN_I((IData)(
									     (1 
									      & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn 
										>> 0x19)))) 
						       << 0x1a)) 
						   | (0x3ffffff 
						      & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn))) 
						 << 2));
			    vlSymsp->TOP__v__or1200_cpu.__PVT__branch_taken = 1;
			}
		    } else {
			if ((0x200 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr))) {
			    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pc 
				= (0xfffffffc & (((vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_pc 
						   >> 2) 
						  + 
						  ((0x3c000000 
						    & (VL_UNARYMIN_I((IData)(
									     (1 
									      & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn 
										>> 0x19)))) 
						       << 0x1a)) 
						   | (0x3ffffff 
						      & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn))) 
						 << 2));
			    vlSymsp->TOP__v__or1200_cpu.__PVT__branch_taken = 1;
			} else {
			    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pc 
				= (0xfffffffc & (((IData)(1) 
						  + vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pcreg) 
						 << 2));
			    vlSymsp->TOP__v__or1200_cpu.__PVT__branch_taken = 0;
			}
		    }
		}
	    } else {
		if ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__branch_op))) {
		    if ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__branch_op))) {
			vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pc 
			    = (0xfffffffc & (((vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_pc 
					       >> 2) 
					      + ((0x3c000000 
						  & (VL_UNARYMIN_I((IData)(
									   (1 
									    & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn 
									       >> 0x19)))) 
						     << 0x1a)) 
						 | (0x3ffffff 
						    & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn))) 
					     << 2));
			vlSymsp->TOP__v__or1200_cpu.__PVT__branch_taken = 1;
		    } else {
			vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pc 
			    = vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b;
			vlSymsp->TOP__v__or1200_cpu.__PVT__branch_taken = 1;
		    }
		} else {
		    if ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__branch_op))) {
			vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pc 
			    = ((0xf0000000 & (VL_UNARYMIN_I((IData)(
								    (1 
								     & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn 
									>> 0x19)))) 
					      << 0x1c)) 
			       | (0xffffffc & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn 
					       << 2)));
			vlSymsp->TOP__v__or1200_cpu.__PVT__branch_taken = 1;
		    } else {
			vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pc 
			    = (0xfffffffc & (((IData)(1) 
					      + vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pcreg) 
					     << 2));
			vlSymsp->TOP__v__or1200_cpu.__PVT__branch_taken = 0;
		    }
		}
	    }
	}
    }
}

void Vor1200_top_or1200_cpu::_settle__TOP__v__or1200_cpu__11(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"        Vor1200_top_or1200_cpu::_settle__TOP__v__or1200_cpu__11"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    // ALWAYS at ../or1k-verilog-rtl/or1200_mem2reg.v:384
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_lsu__DOT__or1200_mem2reg__DOT__aligned 
	= ((2 & vlSymsp->TOP__v__or1200_cpu.__PVT__dcpu_adr_o)
	    ? ((1 & vlSymsp->TOP__v__or1200_cpu.__PVT__dcpu_adr_o)
	        ? (0xff000000 & (((0x800000 == (0xfff00000 
						& vlSymsp->TOP__v.__PVT__qmemdmmu_adr_dmmu))
				   ? vlSymsp->TOP__v.__PVT__or1200_qmem_top__DOT__qmem_do
				   : ((1 & ((IData)(vlSymsp->TOP__v.__PVT__or1200_dc_top__DOT__dcfsm_first_miss_ack) 
					    | (~ ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr) 
						  >> 3))))
				       ? vlTOPp->dwb_dat_i
				       : vlSymsp->TOP__v.__PVT__or1200_dc_top__DOT__from_dcram)) 
				 << 0x18)) : (0xffff0000 
					      & (((0x800000 
						   == 
						   (0xfff00000 
						    & vlSymsp->TOP__v.__PVT__qmemdmmu_adr_dmmu))
						   ? vlSymsp->TOP__v.__PVT__or1200_qmem_top__DOT__qmem_do
						   : 
						  ((1 
						    & ((IData)(vlSymsp->TOP__v.__PVT__or1200_dc_top__DOT__dcfsm_first_miss_ack) 
						       | (~ 
							  ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr) 
							   >> 3))))
						    ? vlTOPp->dwb_dat_i
						    : vlSymsp->TOP__v.__PVT__or1200_dc_top__DOT__from_dcram)) 
						 << 0x10)))
	    : ((1 & vlSymsp->TOP__v__or1200_cpu.__PVT__dcpu_adr_o)
	        ? (0xffffff00 & (((0x800000 == (0xfff00000 
						& vlSymsp->TOP__v.__PVT__qmemdmmu_adr_dmmu))
				   ? vlSymsp->TOP__v.__PVT__or1200_qmem_top__DOT__qmem_do
				   : ((1 & ((IData)(vlSymsp->TOP__v.__PVT__or1200_dc_top__DOT__dcfsm_first_miss_ack) 
					    | (~ ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr) 
						  >> 3))))
				       ? vlTOPp->dwb_dat_i
				       : vlSymsp->TOP__v.__PVT__or1200_dc_top__DOT__from_dcram)) 
				 << 8)) : ((0x800000 
					    == (0xfff00000 
						& vlSymsp->TOP__v.__PVT__qmemdmmu_adr_dmmu))
					    ? vlSymsp->TOP__v.__PVT__or1200_qmem_top__DOT__qmem_do
					    : ((1 & 
						((IData)(vlSymsp->TOP__v.__PVT__or1200_dc_top__DOT__dcfsm_first_miss_ack) 
						 | (~ 
						    ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr) 
						     >> 3))))
					        ? vlTOPp->dwb_dat_i
					        : vlSymsp->TOP__v.__PVT__or1200_dc_top__DOT__from_dcram))));
    vlSymsp->TOP__v__or1200_cpu.__PVT__genpc_refetch 
	= ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_if__DOT__saved) 
	   & (IData)(vlSymsp->TOP__v.__PVT__icpu_ack_qmem));
}

void Vor1200_top_or1200_cpu::_combo__TOP__v__or1200_cpu__12(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"        Vor1200_top_or1200_cpu::_combo__TOP__v__or1200_cpu__12"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    // ALWAYS at ../or1k-verilog-rtl/or1200_genpc.v:201
    if (vlSymsp->TOP__v__or1200_cpu.__PVT__pc_we) {
	vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pc 
	    = vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu;
	vlSymsp->TOP__v__or1200_cpu.__PVT__branch_taken = 0;
    } else {
	if (vlSymsp->TOP__v__or1200_cpu.__PVT__except_start) {
	    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pc 
		= ((((0x4000 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr))
		      ? 0xf0000 : 0) << 0xc) | ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__except_type) 
						<< 8));
	    vlSymsp->TOP__v__or1200_cpu.__PVT__branch_taken = 1;
	} else {
	    if ((4 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__branch_op))) {
		if ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__branch_op))) {
		    if ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__branch_op))) {
			vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pc 
			    = vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu;
			vlSymsp->TOP__v__or1200_cpu.__PVT__branch_taken = 0;
		    } else {
			vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pc 
			    = vlSymsp->TOP__v__or1200_cpu.__PVT__epcr;
			vlSymsp->TOP__v__or1200_cpu.__PVT__branch_taken = 1;
		    }
		} else {
		    if ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__branch_op))) {
			if ((0x200 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr))) {
			    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pc 
				= (0xfffffffc & (((IData)(1) 
						  + vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pcreg) 
						 << 2));
			    vlSymsp->TOP__v__or1200_cpu.__PVT__branch_taken = 0;
			} else {
			    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pc 
				= (0xfffffffc & (((vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_pc 
						   >> 2) 
						  + 
						  ((0x3c000000 
						    & (VL_UNARYMIN_I((IData)(
									     (1 
									      & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn 
										>> 0x19)))) 
						       << 0x1a)) 
						   | (0x3ffffff 
						      & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn))) 
						 << 2));
			    vlSymsp->TOP__v__or1200_cpu.__PVT__branch_taken = 1;
			}
		    } else {
			if ((0x200 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr))) {
			    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pc 
				= (0xfffffffc & (((vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_pc 
						   >> 2) 
						  + 
						  ((0x3c000000 
						    & (VL_UNARYMIN_I((IData)(
									     (1 
									      & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn 
										>> 0x19)))) 
						       << 0x1a)) 
						   | (0x3ffffff 
						      & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn))) 
						 << 2));
			    vlSymsp->TOP__v__or1200_cpu.__PVT__branch_taken = 1;
			} else {
			    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pc 
				= (0xfffffffc & (((IData)(1) 
						  + vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pcreg) 
						 << 2));
			    vlSymsp->TOP__v__or1200_cpu.__PVT__branch_taken = 0;
			}
		    }
		}
	    } else {
		if ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__branch_op))) {
		    if ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__branch_op))) {
			vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pc 
			    = (0xfffffffc & (((vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_pc 
					       >> 2) 
					      + ((0x3c000000 
						  & (VL_UNARYMIN_I((IData)(
									   (1 
									    & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn 
									       >> 0x19)))) 
						     << 0x1a)) 
						 | (0x3ffffff 
						    & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn))) 
					     << 2));
			vlSymsp->TOP__v__or1200_cpu.__PVT__branch_taken = 1;
		    } else {
			vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pc 
			    = vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b;
			vlSymsp->TOP__v__or1200_cpu.__PVT__branch_taken = 1;
		    }
		} else {
		    if ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__branch_op))) {
			vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pc 
			    = ((0xf0000000 & (VL_UNARYMIN_I((IData)(
								    (1 
								     & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn 
									>> 0x19)))) 
					      << 0x1c)) 
			       | (0xffffffc & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn 
					       << 2)));
			vlSymsp->TOP__v__or1200_cpu.__PVT__branch_taken = 1;
		    } else {
			vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pc 
			    = (0xfffffffc & (((IData)(1) 
					      + vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pcreg) 
					     << 2));
			vlSymsp->TOP__v__or1200_cpu.__PVT__branch_taken = 0;
		    }
		}
	    }
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_mem2reg.v:384
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_lsu__DOT__or1200_mem2reg__DOT__aligned 
	= ((2 & vlSymsp->TOP__v__or1200_cpu.__PVT__dcpu_adr_o)
	    ? ((1 & vlSymsp->TOP__v__or1200_cpu.__PVT__dcpu_adr_o)
	        ? (0xff000000 & (((0x800000 == (0xfff00000 
						& vlSymsp->TOP__v.__PVT__qmemdmmu_adr_dmmu))
				   ? vlSymsp->TOP__v.__PVT__or1200_qmem_top__DOT__qmem_do
				   : ((1 & ((IData)(vlSymsp->TOP__v.__PVT__or1200_dc_top__DOT__dcfsm_first_miss_ack) 
					    | (~ ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr) 
						  >> 3))))
				       ? vlTOPp->dwb_dat_i
				       : vlSymsp->TOP__v.__PVT__or1200_dc_top__DOT__from_dcram)) 
				 << 0x18)) : (0xffff0000 
					      & (((0x800000 
						   == 
						   (0xfff00000 
						    & vlSymsp->TOP__v.__PVT__qmemdmmu_adr_dmmu))
						   ? vlSymsp->TOP__v.__PVT__or1200_qmem_top__DOT__qmem_do
						   : 
						  ((1 
						    & ((IData)(vlSymsp->TOP__v.__PVT__or1200_dc_top__DOT__dcfsm_first_miss_ack) 
						       | (~ 
							  ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr) 
							   >> 3))))
						    ? vlTOPp->dwb_dat_i
						    : vlSymsp->TOP__v.__PVT__or1200_dc_top__DOT__from_dcram)) 
						 << 0x10)))
	    : ((1 & vlSymsp->TOP__v__or1200_cpu.__PVT__dcpu_adr_o)
	        ? (0xffffff00 & (((0x800000 == (0xfff00000 
						& vlSymsp->TOP__v.__PVT__qmemdmmu_adr_dmmu))
				   ? vlSymsp->TOP__v.__PVT__or1200_qmem_top__DOT__qmem_do
				   : ((1 & ((IData)(vlSymsp->TOP__v.__PVT__or1200_dc_top__DOT__dcfsm_first_miss_ack) 
					    | (~ ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr) 
						  >> 3))))
				       ? vlTOPp->dwb_dat_i
				       : vlSymsp->TOP__v.__PVT__or1200_dc_top__DOT__from_dcram)) 
				 << 8)) : ((0x800000 
					    == (0xfff00000 
						& vlSymsp->TOP__v.__PVT__qmemdmmu_adr_dmmu))
					    ? vlSymsp->TOP__v.__PVT__or1200_qmem_top__DOT__qmem_do
					    : ((1 & 
						((IData)(vlSymsp->TOP__v.__PVT__or1200_dc_top__DOT__dcfsm_first_miss_ack) 
						 | (~ 
						    ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr) 
						     >> 3))))
					        ? vlTOPp->dwb_dat_i
					        : vlSymsp->TOP__v.__PVT__or1200_dc_top__DOT__from_dcram))));
    vlSymsp->TOP__v__or1200_cpu.__PVT__genpc_refetch 
	= ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_if__DOT__saved) 
	   & (IData)(vlSymsp->TOP__v.__PVT__icpu_ack_qmem));
}

void Vor1200_top_or1200_cpu::_settle__TOP__v__or1200_cpu__13(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"        Vor1200_top_or1200_cpu::_settle__TOP__v__or1200_cpu__13"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlSymsp->TOP__v__or1200_cpu.__PVT__icpu_adr_o = 
	(((((~ (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__no_more_dslot)) 
	    & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__except_start))) 
	   & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__pc_we))) 
	  & ((IData)(vlSymsp->TOP__v.__PVT__icpu_rty_immu) 
	     | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__genpc_refetch)))
	  ? vlSymsp->TOP__v.__PVT__icpu_adr_immu : vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_genpc__DOT__pc);
}

void Vor1200_top_or1200_cpu::_settle__TOP__v__or1200_cpu__15(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"        Vor1200_top_or1200_cpu::_settle__TOP__v__or1200_cpu__15"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    // ALWAYS at ../or1k-verilog-rtl/or1200_sprs.v:386
    vlSymsp->TOP__v__or1200_cpu.__PVT__sprs_dataout 
	= ((0xe == (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sprs_op))
	    ? 0 : ((0xf == (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sprs_op))
		    ? ((0x8000 & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
		        ? 0 : ((0x4000 & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
			        ? ((0x2000 & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
				    ? 0 : ((0x1000 
					    & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
					    ? ((0x800 
						& vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
					        ? 0
					        : (
						   (1 
						    & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						    ? vlSymsp->TOP__v.__PVT__or1200_tt__DOT__ttcr
						    : vlSymsp->TOP__v.__PVT__or1200_tt__DOT__ttmr))
					    : ((0x800 
						& vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
					        ? (
						   (0 
						    == 
						    (3 
						     & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr))
						    ? 
						   (vlSymsp->TOP__v.__PVT__or1200_pic__DOT__picmr 
						    << 2)
						    : vlSymsp->TOP__v.__PVT__or1200_pic__DOT__picsr)
					        : vlSymsp->TOP__v.__PVT__spr_dat_pm)))
			        : ((0x2000 & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
				    ? ((0x1000 & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
				        ? ((0x800 & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
					    ? 0 : (
						   (0x10 
						    == 
						    (0x7ff 
						     & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr))
						    ? vlSymsp->TOP__v.__PVT__or1200_du__DOT__dmr1
						    : 
						   ((0x14 
						     == 
						     (0x7ff 
						      & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr))
						     ? (IData)(vlSymsp->TOP__v.__PVT__or1200_du__DOT__dsr)
						     : 
						    ((0x15 
						      == 
						      (0x7ff 
						       & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr))
						      ? (IData)(vlSymsp->TOP__v.__PVT__or1200_du__DOT__drr)
						      : 0))))
				        : ((0x800 & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
					    ? ((1 & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
					        ? (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__mac_r)
					        : (IData)(
							  (vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__mac_r 
							   >> 0x20)))
					    : 0)) : 
				   ((0x1000 & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
				     ? ((0x800 & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
					 ? 0 : ((4 
						 & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_cs)
						 ? 
						((1 
						  & ((~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we)) 
						     & (~ 
							(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr 
							 >> 7))))
						  ? 
						 ((0xfff80000 
						   & ((IData)(vlSymsp->TOP__v.__PVT__or1200_immu_top__DOT__or1200_immu_tlb__DOT__tlb_mr_ram_out) 
						      << 0x12)) 
						  | ((((IData)(vlSymsp->TOP__v.__PVT__or1200_immu_top__DOT__or1200_immu_tlb__DOT__tlb_index) 
						       & VL_UNARYMIN_I((IData)(
									       (1 
										& (IData)(vlSymsp->TOP__v.__PVT__or1200_immu_top__DOT__or1200_immu_tlb__DOT__tlb_mr_ram_out))))) 
						      << 0xd) 
						     | (1 
							& (IData)(vlSymsp->TOP__v.__PVT__or1200_immu_top__DOT__or1200_immu_tlb__DOT__tlb_mr_ram_out))))
						  : 
						 ((1 
						   & ((~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we)) 
						      & (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr 
							 >> 7)))
						   ? 
						  ((0xffffe000 
						    & (vlSymsp->TOP__v.__PVT__or1200_immu_top__DOT__or1200_immu_tlb__DOT__tlb_tr_ram_out 
						       << 0xa)) 
						   | ((0x80 
						       & (vlSymsp->TOP__v.__PVT__or1200_immu_top__DOT__or1200_immu_tlb__DOT__tlb_tr_ram_out 
							  << 5)) 
						      | ((0x40 
							  & (vlSymsp->TOP__v.__PVT__or1200_immu_top__DOT__or1200_immu_tlb__DOT__tlb_tr_ram_out 
							     << 5)) 
							 | (2 
							    & (vlSymsp->TOP__v.__PVT__or1200_immu_top__DOT__or1200_immu_tlb__DOT__tlb_tr_ram_out 
							       << 1)))))
						   : 0))
						 : 0))
				     : ((0x800 & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
					 ? ((2 & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_cs)
					     ? ((1 
						 & (((vlSymsp->TOP__v__or1200_cpu.__PVT__spr_cs 
						      >> 1) 
						     & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))) 
						    & (~ 
						       (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr 
							>> 7))))
						 ? 
						((0xfff80000 
						  & ((IData)(vlSymsp->TOP__v.__PVT__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__tlb_mr_ram_out) 
						     << 0x12)) 
						 | ((((IData)(vlSymsp->TOP__v.__PVT__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__tlb_index) 
						      & VL_UNARYMIN_I((IData)(
									      (1 
									       & (IData)(vlSymsp->TOP__v.__PVT__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__tlb_mr_ram_out))))) 
						     << 0xd) 
						    | (1 
						       & (IData)(vlSymsp->TOP__v.__PVT__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__tlb_mr_ram_out))))
						 : 
						((1 
						  & (((vlSymsp->TOP__v__or1200_cpu.__PVT__spr_cs 
						       >> 1) 
						      & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))) 
						     & (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr 
							>> 7)))
						  ? 
						 ((0xffffe000 
						   & (vlSymsp->TOP__v.__PVT__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__tlb_tr_ram_out 
						      << 8)) 
						  | ((0x200 
						      & (vlSymsp->TOP__v.__PVT__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__tlb_tr_ram_out 
							 << 5)) 
						     | ((0x100 
							 & (vlSymsp->TOP__v.__PVT__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__tlb_tr_ram_out 
							    << 5)) 
							| ((0x80 
							    & (vlSymsp->TOP__v.__PVT__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__tlb_tr_ram_out 
							       << 5)) 
							   | ((0x40 
							       & (vlSymsp->TOP__v.__PVT__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__tlb_tr_ram_out 
								  << 5)) 
							      | (2 
								 & (vlSymsp->TOP__v.__PVT__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__tlb_tr_ram_out 
								    << 1)))))))
						  : 0))
					     : 0) : 
					((((((((((0 
						  != 
						  (0xfffffff 
						   & (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr 
						      >> 4)))
						  ? 0
						  : 
						 ((8 
						   & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						   ? 0
						   : 
						  ((4 
						    & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						    ? 
						   ((2 
						     & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						     ? 
						    ((1 
						      & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						      ? 0
						      : 0x2640)
						     : 
						    ((1 
						      & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						      ? 0x2640
						      : 0x418))
						    : 
						   ((2 
						     & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						     ? 
						    ((1 
						      & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						      ? 0x418
						      : 0x20)
						     : 
						    ((1 
						      & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						      ? 0x77f
						      : 0x12000001))))) 
						& VL_UNARYMIN_I((IData)(
									((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__read_spr) 
									 & (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_cs 
									    & (0 
									       == 
									       (0x7f 
										& (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr 
										>> 4)))))))) 
					       | (vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__do_a 
						  & VL_UNARYMIN_I((IData)(
									  ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__read_spr) 
									   & (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_cs 
									      & (0x20 
										== 
										(0x3f 
										& (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr 
										>> 5))))))))) 
					      | ((((5 
						    == 
						    (0x3f 
						     & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn 
							>> 0x1a))) 
						   & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn 
						      >> 0x10))
						   ? vlSymsp->TOP__v__or1200_cpu.__PVT__id_pc
						   : vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_pc) 
						 & VL_UNARYMIN_I((IData)(
									 ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__read_spr) 
									  & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__npc_sel)))))) 
					     | (vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__wb_pc 
						& VL_UNARYMIN_I((IData)(
									((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__read_spr) 
									 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__ppc_sel)))))) 
					    | ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr) 
					       & VL_UNARYMIN_I((IData)(
								       ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__read_spr) 
									& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sr_sel)))))) 
					   | (vlSymsp->TOP__v__or1200_cpu.__PVT__epcr 
					      & VL_UNARYMIN_I((IData)(
								      ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__read_spr) 
								       & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__epcr_sel)))))) 
					  | (vlSymsp->TOP__v__or1200_cpu.__PVT__eear 
					     & VL_UNARYMIN_I((IData)(
								     ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__read_spr) 
								      & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__eear_sel)))))) 
					 | ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__esr) 
					    & VL_UNARYMIN_I((IData)(
								    ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__read_spr) 
								     & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__esr_sel)))))))))))
		    : 0));
}

void Vor1200_top_or1200_cpu::_combo__TOP__v__or1200_cpu__16(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"        Vor1200_top_or1200_cpu::_combo__TOP__v__or1200_cpu__16"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    // ALWAYS at ../or1k-verilog-rtl/or1200_sprs.v:386
    vlSymsp->TOP__v__or1200_cpu.__PVT__sprs_dataout 
	= ((0xe == (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sprs_op))
	    ? 0 : ((0xf == (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sprs_op))
		    ? ((0x8000 & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
		        ? 0 : ((0x4000 & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
			        ? ((0x2000 & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
				    ? 0 : ((0x1000 
					    & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
					    ? ((0x800 
						& vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
					        ? 0
					        : (
						   (1 
						    & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						    ? vlSymsp->TOP__v.__PVT__or1200_tt__DOT__ttcr
						    : vlSymsp->TOP__v.__PVT__or1200_tt__DOT__ttmr))
					    : ((0x800 
						& vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
					        ? (
						   (0 
						    == 
						    (3 
						     & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr))
						    ? 
						   (vlSymsp->TOP__v.__PVT__or1200_pic__DOT__picmr 
						    << 2)
						    : vlSymsp->TOP__v.__PVT__or1200_pic__DOT__picsr)
					        : vlSymsp->TOP__v.__PVT__spr_dat_pm)))
			        : ((0x2000 & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
				    ? ((0x1000 & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
				        ? ((0x800 & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
					    ? 0 : (
						   (0x10 
						    == 
						    (0x7ff 
						     & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr))
						    ? vlSymsp->TOP__v.__PVT__or1200_du__DOT__dmr1
						    : 
						   ((0x14 
						     == 
						     (0x7ff 
						      & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr))
						     ? (IData)(vlSymsp->TOP__v.__PVT__or1200_du__DOT__dsr)
						     : 
						    ((0x15 
						      == 
						      (0x7ff 
						       & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr))
						      ? (IData)(vlSymsp->TOP__v.__PVT__or1200_du__DOT__drr)
						      : 0))))
				        : ((0x800 & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
					    ? ((1 & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
					        ? (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__mac_r)
					        : (IData)(
							  (vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_mult_mac__DOT__mac_r 
							   >> 0x20)))
					    : 0)) : 
				   ((0x1000 & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
				     ? ((0x800 & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
					 ? 0 : ((4 
						 & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_cs)
						 ? 
						((1 
						  & ((~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we)) 
						     & (~ 
							(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr 
							 >> 7))))
						  ? 
						 ((0xfff80000 
						   & ((IData)(vlSymsp->TOP__v.__PVT__or1200_immu_top__DOT__or1200_immu_tlb__DOT__tlb_mr_ram_out) 
						      << 0x12)) 
						  | ((((IData)(vlSymsp->TOP__v.__PVT__or1200_immu_top__DOT__or1200_immu_tlb__DOT__tlb_index) 
						       & VL_UNARYMIN_I((IData)(
									       (1 
										& (IData)(vlSymsp->TOP__v.__PVT__or1200_immu_top__DOT__or1200_immu_tlb__DOT__tlb_mr_ram_out))))) 
						      << 0xd) 
						     | (1 
							& (IData)(vlSymsp->TOP__v.__PVT__or1200_immu_top__DOT__or1200_immu_tlb__DOT__tlb_mr_ram_out))))
						  : 
						 ((1 
						   & ((~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we)) 
						      & (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr 
							 >> 7)))
						   ? 
						  ((0xffffe000 
						    & (vlSymsp->TOP__v.__PVT__or1200_immu_top__DOT__or1200_immu_tlb__DOT__tlb_tr_ram_out 
						       << 0xa)) 
						   | ((0x80 
						       & (vlSymsp->TOP__v.__PVT__or1200_immu_top__DOT__or1200_immu_tlb__DOT__tlb_tr_ram_out 
							  << 5)) 
						      | ((0x40 
							  & (vlSymsp->TOP__v.__PVT__or1200_immu_top__DOT__or1200_immu_tlb__DOT__tlb_tr_ram_out 
							     << 5)) 
							 | (2 
							    & (vlSymsp->TOP__v.__PVT__or1200_immu_top__DOT__or1200_immu_tlb__DOT__tlb_tr_ram_out 
							       << 1)))))
						   : 0))
						 : 0))
				     : ((0x800 & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
					 ? ((2 & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_cs)
					     ? ((1 
						 & (((vlSymsp->TOP__v__or1200_cpu.__PVT__spr_cs 
						      >> 1) 
						     & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))) 
						    & (~ 
						       (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr 
							>> 7))))
						 ? 
						((0xfff80000 
						  & ((IData)(vlSymsp->TOP__v.__PVT__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__tlb_mr_ram_out) 
						     << 0x12)) 
						 | ((((IData)(vlSymsp->TOP__v.__PVT__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__tlb_index) 
						      & VL_UNARYMIN_I((IData)(
									      (1 
									       & (IData)(vlSymsp->TOP__v.__PVT__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__tlb_mr_ram_out))))) 
						     << 0xd) 
						    | (1 
						       & (IData)(vlSymsp->TOP__v.__PVT__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__tlb_mr_ram_out))))
						 : 
						((1 
						  & (((vlSymsp->TOP__v__or1200_cpu.__PVT__spr_cs 
						       >> 1) 
						      & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))) 
						     & (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr 
							>> 7)))
						  ? 
						 ((0xffffe000 
						   & (vlSymsp->TOP__v.__PVT__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__tlb_tr_ram_out 
						      << 8)) 
						  | ((0x200 
						      & (vlSymsp->TOP__v.__PVT__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__tlb_tr_ram_out 
							 << 5)) 
						     | ((0x100 
							 & (vlSymsp->TOP__v.__PVT__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__tlb_tr_ram_out 
							    << 5)) 
							| ((0x80 
							    & (vlSymsp->TOP__v.__PVT__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__tlb_tr_ram_out 
							       << 5)) 
							   | ((0x40 
							       & (vlSymsp->TOP__v.__PVT__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__tlb_tr_ram_out 
								  << 5)) 
							      | (2 
								 & (vlSymsp->TOP__v.__PVT__or1200_dmmu_top__DOT__or1200_dmmu_tlb__DOT__tlb_tr_ram_out 
								    << 1)))))))
						  : 0))
					     : 0) : 
					((((((((((0 
						  != 
						  (0xfffffff 
						   & (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr 
						      >> 4)))
						  ? 0
						  : 
						 ((8 
						   & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						   ? 0
						   : 
						  ((4 
						    & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						    ? 
						   ((2 
						     & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						     ? 
						    ((1 
						      & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						      ? 0
						      : 0x2640)
						     : 
						    ((1 
						      & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						      ? 0x2640
						      : 0x418))
						    : 
						   ((2 
						     & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						     ? 
						    ((1 
						      & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						      ? 0x418
						      : 0x20)
						     : 
						    ((1 
						      & vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr)
						      ? 0x77f
						      : 0x12000001))))) 
						& VL_UNARYMIN_I((IData)(
									((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__read_spr) 
									 & (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_cs 
									    & (0 
									       == 
									       (0x7f 
										& (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr 
										>> 4)))))))) 
					       | (vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__do_a 
						  & VL_UNARYMIN_I((IData)(
									  ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__read_spr) 
									   & (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_cs 
									      & (0x20 
										== 
										(0x3f 
										& (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr 
										>> 5))))))))) 
					      | ((((5 
						    == 
						    (0x3f 
						     & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn 
							>> 0x1a))) 
						   & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn 
						      >> 0x10))
						   ? vlSymsp->TOP__v__or1200_cpu.__PVT__id_pc
						   : vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_pc) 
						 & VL_UNARYMIN_I((IData)(
									 ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__read_spr) 
									  & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__npc_sel)))))) 
					     | (vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__wb_pc 
						& VL_UNARYMIN_I((IData)(
									((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__read_spr) 
									 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__ppc_sel)))))) 
					    | ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr) 
					       & VL_UNARYMIN_I((IData)(
								       ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__read_spr) 
									& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__sr_sel)))))) 
					   | (vlSymsp->TOP__v__or1200_cpu.__PVT__epcr 
					      & VL_UNARYMIN_I((IData)(
								      ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__read_spr) 
								       & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__epcr_sel)))))) 
					  | (vlSymsp->TOP__v__or1200_cpu.__PVT__eear 
					     & VL_UNARYMIN_I((IData)(
								     ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__read_spr) 
								      & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__eear_sel)))))) 
					 | ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__esr) 
					    & VL_UNARYMIN_I((IData)(
								    ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__read_spr) 
								     & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_sprs__DOT__esr_sel)))))))))))
		    : 0));
    vlSymsp->TOP__v__or1200_cpu.__PVT__dcpu_cycstb_o 
	= ((~ (((IData)(vlTOPp->dbg_stall_i) | (IData)(vlSymsp->TOP__v.__PVT__dcpu_ack_qmem)) 
	       | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__except_align))) 
	   & (0 != (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op)));
    // ALWAYS at ../or1k-verilog-rtl/or1200_wbmux.v:134
    vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw = (
						   (4 
						    & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rfwb_op))
						    ? 
						   ((2 
						     & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rfwb_op))
						     ? 
						    ((IData)(8) 
						     + 
						     (0xfffffffc 
						      & vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_pc))
						     : vlSymsp->TOP__v__or1200_cpu.__PVT__sprs_dataout)
						    : 
						   ((2 
						     & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rfwb_op))
						     ? 
						    ((8 
						      & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op))
						      ? vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_lsu__DOT__or1200_mem2reg__DOT__aligned
						      : 
						     ((4 
						       & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op))
						       ? 
						      ((2 
							& (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op))
						        ? vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_lsu__DOT__or1200_mem2reg__DOT__aligned
						        : 
						       ((1 
							 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op))
							 ? 
							((0xffff0000 
							  & (VL_UNARYMIN_I((IData)(
										(1 
										& (vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_lsu__DOT__or1200_mem2reg__DOT__aligned 
										>> 0x1f)))) 
							     << 0x10)) 
							 | (0xffff 
							    & (vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_lsu__DOT__or1200_mem2reg__DOT__aligned 
							       >> 0x10)))
							 : 
							(0xffff 
							 & (vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_lsu__DOT__or1200_mem2reg__DOT__aligned 
							    >> 0x10))))
						       : 
						      ((2 
							& (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op))
						        ? 
						       ((1 
							 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op))
							 ? 
							((0xffffff00 
							  & (VL_UNARYMIN_I((IData)(
										(1 
										& (vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_lsu__DOT__or1200_mem2reg__DOT__aligned 
										>> 0x1f)))) 
							     << 8)) 
							 | (0xff 
							    & (vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_lsu__DOT__or1200_mem2reg__DOT__aligned 
							       >> 0x18)))
							 : 
							(0xff 
							 & (vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_lsu__DOT__or1200_mem2reg__DOT__aligned 
							    >> 0x18)))
						        : vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_lsu__DOT__or1200_mem2reg__DOT__aligned)))
						     : 
						    ((8 
						      & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))
						      ? 
						     ((4 
						       & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))
						       ? 
						      ((2 
							& (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))
						        ? 
						       ((1 
							 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))
							 ? 
							((1 
							  & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
							  ? 1
							  : 
							 ((2 
							   & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
							   ? 2
							   : 
							  ((4 
							    & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
							    ? 3
							    : 
							   ((8 
							     & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
							     ? 4
							     : 
							    ((0x10 
							      & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
							      ? 5
							      : 
							     ((0x20 
							       & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
							       ? 6
							       : 
							      ((0x40 
								& vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
							        ? 7
							        : 
							       ((0x80 
								 & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
								 ? 8
								 : 
								((0x100 
								  & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
								  ? 9
								  : 
								 ((0x200 
								   & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
								   ? 0xa
								   : 
								  ((0x400 
								    & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
								    ? 0xb
								    : 
								   ((0x800 
								     & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
								     ? 0xc
								     : 
								    ((0x1000 
								      & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
								      ? 0xd
								      : 
								     ((0x2000 
								       & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
								       ? 0xe
								       : 
								      ((0x4000 
									& vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
								        ? 0xf
								        : 
								       ((0x8000 
									 & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
									 ? 0x10
									 : 
									((0x10000 
									  & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
									  ? 0x11
									  : 
									 ((0x20000 
									   & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
									   ? 0x12
									   : 
									  ((0x40000 
									    & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
									    ? 0x13
									    : 
									   ((0x80000 
									     & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
									     ? 0x14
									     : 
									    ((0x100000 
									      & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
									      ? 0x15
									      : 
									     ((0x200000 
									       & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
									       ? 0x16
									       : 
									      ((0x400000 
										& vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
									        ? 0x17
									        : 
									       ((0x800000 
										& vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
										 ? 0x18
										 : 
										((0x1000000 
										& vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
										 ? 0x19
										 : 
										((0x2000000 
										& vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
										 ? 0x1a
										 : 
										((0x4000000 
										& vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
										 ? 0x1b
										 : 
										((0x8000000 
										& vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
										 ? 0x1c
										 : 
										((0x10000000 
										& vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
										 ? 0x1d
										 : 
										((0x20000000 
										& vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
										 ? 0x1e
										 : 
										((0x40000000 
										& vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
										 ? 0x1f
										 : 
										((0x80000000 
										& vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
										 ? 0x20
										 : 0))))))))))))))))))))))))))))))))
							 : 
							((0x200 
							  & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr))
							  ? vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a
							  : vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b))
						        : 
						       ((1 
							 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))
							 ? vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_alu__DOT__result_and
							 : 
							((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_macrc_op)
							  ? vlSymsp->TOP__v__or1200_cpu.__PVT__mult_mac_result
							  : 
							 (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b 
							  << 0x10))))
						       : 
						      ((2 
							& (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))
						        ? 
						       ((1 
							 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))
							 ? vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b
							 : vlSymsp->TOP__v__or1200_cpu.__PVT__mult_mac_result)
						        : 
						       ((1 
							 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))
							 ? vlSymsp->TOP__v__or1200_cpu.__PVT__mult_mac_result
							 : 
							((0 
							  == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__shrot_op))
							  ? 
							 (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a 
							  << 
							  (0x1f 
							   & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b))
							  : 
							 ((1 
							   == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__shrot_op))
							   ? 
							  (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a 
							   >> 
							   (0x1f 
							    & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b))
							   : 
							  ((((0x3f 
							      & ((IData)(0x20) 
								 - 
								 (0x1f 
								  & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b))) 
							     <= 0x1f)
							     ? 
							    (VL_UNARYMIN_I((IData)(
										(1 
										& (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a 
										>> 0x1f)))) 
							     << 
							     (0x3f 
							      & ((IData)(0x20) 
								 - 
								 (0x1f 
								  & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b))))
							     : 0) 
							   | (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a 
							      >> 
							      (0x1f 
							       & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b))))))))
						      : 
						     ((4 
						       & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))
						       ? 
						      ((2 
							& (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))
						        ? 
						       ((1 
							 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))
							 ? 
							((1 
							  == 
							  (0x1f 
							   & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn))
							  ? 
							 ((0x40 
							   & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn)
							   ? 
							  ((0x20 
							    & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn)
							    ? 
							   ((0xff000000 
							     & (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b 
								<< 0x18)) 
							    | (0xffffff 
							       & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a))
							    : 
							   ((0xff000000 
							     & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a) 
							    | ((0xff0000 
								& (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b 
								   << 0x10)) 
							       | (0xffff 
								  & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a))))
							   : 
							  ((0x20 
							    & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn)
							    ? 
							   ((0xffff0000 
							     & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a) 
							    | ((0xff00 
								& (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b 
								   << 8)) 
							       | (0xff 
								  & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)))
							    : 
							   ((0xffffff00 
							     & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a) 
							    | (0xff 
							       & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b))))
							  : 
							 ((2 
							   == 
							   (0x1f 
							    & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn))
							   ? 
							  (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a 
							   | (((0x3f 
								& (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn 
								   >> 5)) 
							       <= 0x1f)
							       ? 
							      ((IData)(1) 
							       << 
							       (0x3f 
								& (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn 
								   >> 5)))
							       : 0))
							   : 
							  ((3 
							    == 
							    (0x1f 
							     & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn))
							    ? 
							   (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a 
							    & (~ 
							       (((0x3f 
								  & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn 
								     >> 5)) 
								 <= 0x1f)
								 ? 
								((IData)(1) 
								 << 
								 (0x3f 
								  & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn 
								     >> 5)))
								 : 0)))
							    : vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)))
							 : vlSymsp->TOP__v__or1200_cpu.__PVT__mult_mac_result)
						        : 
						       ((1 
							 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))
							 ? 
							(vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a 
							 ^ vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b)
							 : 
							(vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a 
							 | vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b)))
						       : 
						      ((2 
							& (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))
						        ? 
						       ((1 
							 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))
							 ? vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_alu__DOT__result_and
							 : 
							(vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a 
							 - vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b))
						        : 
						       ((1 
							 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))
							 ? (IData)(
								   (VL_ULL(0x1ffffffff) 
								    & (((QData)((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)) 
									+ (QData)((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b))) 
								       + (QData)((IData)(
										(1 
										& ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr) 
										>> 0xa)))))))
							 : (IData)(
								   (VL_ULL(0x1ffffffff) 
								    & ((QData)((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)) 
								       + (QData)((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b)))))))))));
}

void Vor1200_top_or1200_cpu::_settle__TOP__v__or1200_cpu__17(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"        Vor1200_top_or1200_cpu::_settle__TOP__v__or1200_cpu__17"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlSymsp->TOP__v__or1200_cpu.__PVT__dcpu_cycstb_o 
	= ((~ (((IData)(vlTOPp->dbg_stall_i) | (IData)(vlSymsp->TOP__v.__PVT__dcpu_ack_qmem)) 
	       | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__except_align))) 
	   & (0 != (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op)));
    // ALWAYS at ../or1k-verilog-rtl/or1200_wbmux.v:134
    vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw = (
						   (4 
						    & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rfwb_op))
						    ? 
						   ((2 
						     & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rfwb_op))
						     ? 
						    ((IData)(8) 
						     + 
						     (0xfffffffc 
						      & vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_pc))
						     : vlSymsp->TOP__v__or1200_cpu.__PVT__sprs_dataout)
						    : 
						   ((2 
						     & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rfwb_op))
						     ? 
						    ((8 
						      & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op))
						      ? vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_lsu__DOT__or1200_mem2reg__DOT__aligned
						      : 
						     ((4 
						       & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op))
						       ? 
						      ((2 
							& (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op))
						        ? vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_lsu__DOT__or1200_mem2reg__DOT__aligned
						        : 
						       ((1 
							 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op))
							 ? 
							((0xffff0000 
							  & (VL_UNARYMIN_I((IData)(
										(1 
										& (vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_lsu__DOT__or1200_mem2reg__DOT__aligned 
										>> 0x1f)))) 
							     << 0x10)) 
							 | (0xffff 
							    & (vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_lsu__DOT__or1200_mem2reg__DOT__aligned 
							       >> 0x10)))
							 : 
							(0xffff 
							 & (vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_lsu__DOT__or1200_mem2reg__DOT__aligned 
							    >> 0x10))))
						       : 
						      ((2 
							& (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op))
						        ? 
						       ((1 
							 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op))
							 ? 
							((0xffffff00 
							  & (VL_UNARYMIN_I((IData)(
										(1 
										& (vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_lsu__DOT__or1200_mem2reg__DOT__aligned 
										>> 0x1f)))) 
							     << 8)) 
							 | (0xff 
							    & (vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_lsu__DOT__or1200_mem2reg__DOT__aligned 
							       >> 0x18)))
							 : 
							(0xff 
							 & (vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_lsu__DOT__or1200_mem2reg__DOT__aligned 
							    >> 0x18)))
						        : vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_lsu__DOT__or1200_mem2reg__DOT__aligned)))
						     : 
						    ((8 
						      & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))
						      ? 
						     ((4 
						       & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))
						       ? 
						      ((2 
							& (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))
						        ? 
						       ((1 
							 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))
							 ? 
							((1 
							  & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
							  ? 1
							  : 
							 ((2 
							   & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
							   ? 2
							   : 
							  ((4 
							    & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
							    ? 3
							    : 
							   ((8 
							     & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
							     ? 4
							     : 
							    ((0x10 
							      & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
							      ? 5
							      : 
							     ((0x20 
							       & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
							       ? 6
							       : 
							      ((0x40 
								& vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
							        ? 7
							        : 
							       ((0x80 
								 & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
								 ? 8
								 : 
								((0x100 
								  & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
								  ? 9
								  : 
								 ((0x200 
								   & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
								   ? 0xa
								   : 
								  ((0x400 
								    & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
								    ? 0xb
								    : 
								   ((0x800 
								     & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
								     ? 0xc
								     : 
								    ((0x1000 
								      & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
								      ? 0xd
								      : 
								     ((0x2000 
								       & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
								       ? 0xe
								       : 
								      ((0x4000 
									& vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
								        ? 0xf
								        : 
								       ((0x8000 
									 & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
									 ? 0x10
									 : 
									((0x10000 
									  & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
									  ? 0x11
									  : 
									 ((0x20000 
									   & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
									   ? 0x12
									   : 
									  ((0x40000 
									    & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
									    ? 0x13
									    : 
									   ((0x80000 
									     & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
									     ? 0x14
									     : 
									    ((0x100000 
									      & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
									      ? 0x15
									      : 
									     ((0x200000 
									       & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
									       ? 0x16
									       : 
									      ((0x400000 
										& vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
									        ? 0x17
									        : 
									       ((0x800000 
										& vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
										 ? 0x18
										 : 
										((0x1000000 
										& vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
										 ? 0x19
										 : 
										((0x2000000 
										& vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
										 ? 0x1a
										 : 
										((0x4000000 
										& vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
										 ? 0x1b
										 : 
										((0x8000000 
										& vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
										 ? 0x1c
										 : 
										((0x10000000 
										& vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
										 ? 0x1d
										 : 
										((0x20000000 
										& vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
										 ? 0x1e
										 : 
										((0x40000000 
										& vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
										 ? 0x1f
										 : 
										((0x80000000 
										& vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)
										 ? 0x20
										 : 0))))))))))))))))))))))))))))))))
							 : 
							((0x200 
							  & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr))
							  ? vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a
							  : vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b))
						        : 
						       ((1 
							 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))
							 ? vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_alu__DOT__result_and
							 : 
							((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_macrc_op)
							  ? vlSymsp->TOP__v__or1200_cpu.__PVT__mult_mac_result
							  : 
							 (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b 
							  << 0x10))))
						       : 
						      ((2 
							& (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))
						        ? 
						       ((1 
							 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))
							 ? vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b
							 : vlSymsp->TOP__v__or1200_cpu.__PVT__mult_mac_result)
						        : 
						       ((1 
							 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))
							 ? vlSymsp->TOP__v__or1200_cpu.__PVT__mult_mac_result
							 : 
							((0 
							  == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__shrot_op))
							  ? 
							 (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a 
							  << 
							  (0x1f 
							   & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b))
							  : 
							 ((1 
							   == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__shrot_op))
							   ? 
							  (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a 
							   >> 
							   (0x1f 
							    & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b))
							   : 
							  ((((0x3f 
							      & ((IData)(0x20) 
								 - 
								 (0x1f 
								  & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b))) 
							     <= 0x1f)
							     ? 
							    (VL_UNARYMIN_I((IData)(
										(1 
										& (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a 
										>> 0x1f)))) 
							     << 
							     (0x3f 
							      & ((IData)(0x20) 
								 - 
								 (0x1f 
								  & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b))))
							     : 0) 
							   | (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a 
							      >> 
							      (0x1f 
							       & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b))))))))
						      : 
						     ((4 
						       & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))
						       ? 
						      ((2 
							& (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))
						        ? 
						       ((1 
							 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))
							 ? 
							((1 
							  == 
							  (0x1f 
							   & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn))
							  ? 
							 ((0x40 
							   & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn)
							   ? 
							  ((0x20 
							    & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn)
							    ? 
							   ((0xff000000 
							     & (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b 
								<< 0x18)) 
							    | (0xffffff 
							       & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a))
							    : 
							   ((0xff000000 
							     & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a) 
							    | ((0xff0000 
								& (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b 
								   << 0x10)) 
							       | (0xffff 
								  & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a))))
							   : 
							  ((0x20 
							    & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn)
							    ? 
							   ((0xffff0000 
							     & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a) 
							    | ((0xff00 
								& (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b 
								   << 8)) 
							       | (0xff 
								  & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)))
							    : 
							   ((0xffffff00 
							     & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a) 
							    | (0xff 
							       & vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b))))
							  : 
							 ((2 
							   == 
							   (0x1f 
							    & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn))
							   ? 
							  (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a 
							   | (((0x3f 
								& (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn 
								   >> 5)) 
							       <= 0x1f)
							       ? 
							      ((IData)(1) 
							       << 
							       (0x3f 
								& (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn 
								   >> 5)))
							       : 0))
							   : 
							  ((3 
							    == 
							    (0x1f 
							     & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn))
							    ? 
							   (vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a 
							    & (~ 
							       (((0x3f 
								  & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn 
								     >> 5)) 
								 <= 0x1f)
								 ? 
								((IData)(1) 
								 << 
								 (0x3f 
								  & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn 
								     >> 5)))
								 : 0)))
							    : vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)))
							 : vlSymsp->TOP__v__or1200_cpu.__PVT__mult_mac_result)
						        : 
						       ((1 
							 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))
							 ? 
							(vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a 
							 ^ vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b)
							 : 
							(vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a 
							 | vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b)))
						       : 
						      ((2 
							& (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))
						        ? 
						       ((1 
							 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))
							 ? vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_alu__DOT__result_and
							 : 
							(vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a 
							 - vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b))
						        : 
						       ((1 
							 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op))
							 ? (IData)(
								   (VL_ULL(0x1ffffffff) 
								    & (((QData)((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)) 
									+ (QData)((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b))) 
								       + (QData)((IData)(
										(1 
										& ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr) 
										>> 0xa)))))))
							 : (IData)(
								   (VL_ULL(0x1ffffffff) 
								    & ((QData)((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__operand_a)) 
								       + (QData)((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b)))))))))));
    vlSymsp->TOP__v__or1200_cpu.__PVT__lsu_stall = 
	(((0x800000 == (0xfff00000 & vlSymsp->TOP__v.__PVT__qmemdmmu_adr_dmmu))
	   ? (~ (IData)(vlSymsp->TOP__v.__PVT__or1200_qmem_top__DOT__qmem_dack))
	   : (~ (IData)(vlSymsp->TOP__v.__PVT__dcqmem_ack_dc))) 
	 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__dcpu_cycstb_o));
    // ALWAYS at ../or1k-verilog-rtl/or1200_operandmuxes.v:149
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_operandmuxes__DOT__muxed_a 
	= ((2 == ((((0x1f & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
			     >> 0x10)) == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rf_addrw)) 
		   & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rfwb_op))
		   ? 2 : ((((0x1f & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
				     >> 0x10)) == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__wb_rfaddrw)) 
			   & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wbforw_valid))
			   ? 3 : 0))) ? vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw
	    : ((3 == ((((0x1f & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
				 >> 0x10)) == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rf_addrw)) 
		       & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rfwb_op))
		       ? 2 : ((((0x1f & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
					 >> 0x10)) 
				== (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__wb_rfaddrw)) 
			       & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wbforw_valid))
			       ? 3 : 0))) ? vlSymsp->TOP__v__or1200_cpu.__PVT__wb_forw
	        : ((1 & (IData)((vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__dataa_saved 
				 >> 0x20))) ? (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__dataa_saved)
		    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__do_a)));
    // ALWAYS at ../or1k-verilog-rtl/or1200_operandmuxes.v:167
    vlSymsp->TOP__v__or1200_cpu.__PVT__muxed_b = ((1 
						   == 
						   ((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__sel_imm)
						     ? 1
						     : 
						    ((((0x1f 
							& (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
							   >> 0xb)) 
						       == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rf_addrw)) 
						      & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rfwb_op))
						      ? 2
						      : 
						     ((((0x1f 
							 & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
							    >> 0xb)) 
							== (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__wb_rfaddrw)) 
						       & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wbforw_valid))
						       ? 3
						       : 0))))
						   ? 
						  ((1 
						    & ((0x80000000 
							& vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
						        ? 
						       ((~ 
							 (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
							  >> 0x1e)) 
							& ((0x20000000 
							    & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
							    ? 
							   ((0x10000000 
							     & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
							     ? 
							    ((0x8000000 
							      & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
							      ? 
							     (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
							      >> 0x1a)
							      : 
							     (~ 
							      (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
							       >> 0x1a)))
							     : 
							    ((0x8000000 
							      & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
							      ? 
							     (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
							      >> 0x1a)
							      : 
							     (~ 
							      (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
							       >> 0x1a))))
							    : 
							   ((vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
							     >> 0x1c) 
							    & ((vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
								>> 0x1b) 
							       & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
								  >> 0x1a)))))
						        : 
						       ((vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
							 >> 0x1e) 
							& ((~ 
							    (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
							     >> 0x1d)) 
							   & ((~ 
							       (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
								>> 0x1c)) 
							      & ((vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
								  >> 0x1b) 
								 & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
								    >> 0x1a)))))))
						    ? 
						   ((0xffff0000 
						     & (VL_UNARYMIN_I((IData)(
									      (1 
									       & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
										>> 0xf)))) 
							<< 0x10)) 
						    | (0xffff 
						       & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn))
						    : 
						   (0xffff 
						    & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn))
						   : 
						  ((2 
						    == 
						    ((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__sel_imm)
						      ? 1
						      : 
						     ((((0x1f 
							 & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
							    >> 0xb)) 
							== (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rf_addrw)) 
						       & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rfwb_op))
						       ? 2
						       : 
						      ((((0x1f 
							  & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
							     >> 0xb)) 
							 == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__wb_rfaddrw)) 
							& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wbforw_valid))
						        ? 3
						        : 0))))
						    ? vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw
						    : 
						   ((3 
						     == 
						     ((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__sel_imm)
						       ? 1
						       : 
						      ((((0x1f 
							  & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
							     >> 0xb)) 
							 == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rf_addrw)) 
							& (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rfwb_op))
						        ? 2
						        : 
						       ((((0x1f 
							   & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
							      >> 0xb)) 
							  == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__wb_rfaddrw)) 
							 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wbforw_valid))
							 ? 3
							 : 0))))
						     ? vlSymsp->TOP__v__or1200_cpu.__PVT__wb_forw
						     : 
						    ((1 
						      & (IData)(
								(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__datab_saved 
								 >> 0x20)))
						      ? (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__datab_saved)
						      : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__do_b))));
}

void Vor1200_top_or1200_cpu::_combo__TOP__v__or1200_cpu__18(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"        Vor1200_top_or1200_cpu::_combo__TOP__v__or1200_cpu__18"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlSymsp->TOP__v__or1200_cpu.__PVT__lsu_stall = 
	(((0x800000 == (0xfff00000 & vlSymsp->TOP__v.__PVT__qmemdmmu_adr_dmmu))
	   ? (~ (IData)(vlSymsp->TOP__v.__PVT__or1200_qmem_top__DOT__qmem_dack))
	   : (~ (IData)(vlSymsp->TOP__v.__PVT__dcqmem_ack_dc))) 
	 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__dcpu_cycstb_o));
    // ALWAYS at ../or1k-verilog-rtl/or1200_operandmuxes.v:149
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_operandmuxes__DOT__muxed_a 
	= ((2 == ((((0x1f & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
			     >> 0x10)) == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rf_addrw)) 
		   & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rfwb_op))
		   ? 2 : ((((0x1f & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
				     >> 0x10)) == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__wb_rfaddrw)) 
			   & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wbforw_valid))
			   ? 3 : 0))) ? vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw
	    : ((3 == ((((0x1f & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
				 >> 0x10)) == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rf_addrw)) 
		       & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rfwb_op))
		       ? 2 : ((((0x1f & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
					 >> 0x10)) 
				== (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__wb_rfaddrw)) 
			       & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wbforw_valid))
			       ? 3 : 0))) ? vlSymsp->TOP__v__or1200_cpu.__PVT__wb_forw
	        : ((1 & (IData)((vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__dataa_saved 
				 >> 0x20))) ? (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__dataa_saved)
		    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__do_a)));
    // ALWAYS at ../or1k-verilog-rtl/or1200_operandmuxes.v:167
    vlSymsp->TOP__v__or1200_cpu.__PVT__muxed_b = ((1 
						   == 
						   ((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__sel_imm)
						     ? 1
						     : 
						    ((((0x1f 
							& (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
							   >> 0xb)) 
						       == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rf_addrw)) 
						      & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rfwb_op))
						      ? 2
						      : 
						     ((((0x1f 
							 & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
							    >> 0xb)) 
							== (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__wb_rfaddrw)) 
						       & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wbforw_valid))
						       ? 3
						       : 0))))
						   ? 
						  ((1 
						    & ((0x80000000 
							& vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
						        ? 
						       ((~ 
							 (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
							  >> 0x1e)) 
							& ((0x20000000 
							    & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
							    ? 
							   ((0x10000000 
							     & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
							     ? 
							    ((0x8000000 
							      & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
							      ? 
							     (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
							      >> 0x1a)
							      : 
							     (~ 
							      (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
							       >> 0x1a)))
							     : 
							    ((0x8000000 
							      & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
							      ? 
							     (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
							      >> 0x1a)
							      : 
							     (~ 
							      (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
							       >> 0x1a))))
							    : 
							   ((vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
							     >> 0x1c) 
							    & ((vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
								>> 0x1b) 
							       & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
								  >> 0x1a)))))
						        : 
						       ((vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
							 >> 0x1e) 
							& ((~ 
							    (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
							     >> 0x1d)) 
							   & ((~ 
							       (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
								>> 0x1c)) 
							      & ((vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
								  >> 0x1b) 
								 & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
								    >> 0x1a)))))))
						    ? 
						   ((0xffff0000 
						     & (VL_UNARYMIN_I((IData)(
									      (1 
									       & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
										>> 0xf)))) 
							<< 0x10)) 
						    | (0xffff 
						       & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn))
						    : 
						   (0xffff 
						    & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn))
						   : 
						  ((2 
						    == 
						    ((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__sel_imm)
						      ? 1
						      : 
						     ((((0x1f 
							 & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
							    >> 0xb)) 
							== (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rf_addrw)) 
						       & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rfwb_op))
						       ? 2
						       : 
						      ((((0x1f 
							  & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
							     >> 0xb)) 
							 == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__wb_rfaddrw)) 
							& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wbforw_valid))
						        ? 3
						        : 0))))
						    ? vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw
						    : 
						   ((3 
						     == 
						     ((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__sel_imm)
						       ? 1
						       : 
						      ((((0x1f 
							  & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
							     >> 0xb)) 
							 == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rf_addrw)) 
							& (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rfwb_op))
						        ? 2
						        : 
						       ((((0x1f 
							   & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
							      >> 0xb)) 
							  == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__wb_rfaddrw)) 
							 & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wbforw_valid))
							 ? 3
							 : 0))))
						     ? vlSymsp->TOP__v__or1200_cpu.__PVT__wb_forw
						     : 
						    ((1 
						      & (IData)(
								(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__datab_saved 
								 >> 0x20)))
						      ? (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__datab_saved)
						      : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__do_b))));
    vlSymsp->TOP__v__or1200_cpu.__PVT__except_dtlbmiss 
	= ((IData)(vlSymsp->TOP__v.__PVT__dcpu_err_dmmu) 
	   & (0xd == (IData)(vlSymsp->TOP__v.__PVT__dcpu_tag_dmmu)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__except_dmmufault 
	= ((IData)(vlSymsp->TOP__v.__PVT__dcpu_err_dmmu) 
	   & (0xc == (IData)(vlSymsp->TOP__v.__PVT__dcpu_tag_dmmu)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__except_dbuserr 
	= ((IData)(vlSymsp->TOP__v.__PVT__dcpu_err_dmmu) 
	   & (0xb == (IData)(vlSymsp->TOP__v.__PVT__dcpu_tag_dmmu)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn = ((
						   ((IData)(vlSymsp->TOP__v.__PVT__icpu_err_immu) 
						    | (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__no_more_dslot)) 
						   | ((6 
						       == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__pre_branch_op)) 
						      | (6 
							 == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__branch_op))))
						   ? 0x14410000
						   : 
						  ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_if__DOT__saved)
						    ? vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_if__DOT__insn_saved
						    : 
						   ((IData)(vlSymsp->TOP__v.__PVT__icpu_ack_qmem)
						     ? vlSymsp->TOP__v.__PVT__icpu_dat_qmem
						     : 0x14610000)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__if_stall = (1 
						   & (((~ (IData)(vlSymsp->TOP__v.__PVT__icpu_err_immu)) 
						       & (~ (IData)(vlSymsp->TOP__v.__PVT__icpu_ack_qmem))) 
						      & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_if__DOT__saved))));
}

void Vor1200_top_or1200_cpu::_settle__TOP__v__or1200_cpu__19(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"        Vor1200_top_or1200_cpu::_settle__TOP__v__or1200_cpu__19"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlSymsp->TOP__v__or1200_cpu.__PVT__except_dtlbmiss 
	= ((IData)(vlSymsp->TOP__v.__PVT__dcpu_err_dmmu) 
	   & (0xd == (IData)(vlSymsp->TOP__v.__PVT__dcpu_tag_dmmu)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__except_dmmufault 
	= ((IData)(vlSymsp->TOP__v.__PVT__dcpu_err_dmmu) 
	   & (0xc == (IData)(vlSymsp->TOP__v.__PVT__dcpu_tag_dmmu)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__except_dbuserr 
	= ((IData)(vlSymsp->TOP__v.__PVT__dcpu_err_dmmu) 
	   & (0xb == (IData)(vlSymsp->TOP__v.__PVT__dcpu_tag_dmmu)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn = ((
						   ((IData)(vlSymsp->TOP__v.__PVT__icpu_err_immu) 
						    | (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__no_more_dslot)) 
						   | ((6 
						       == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__pre_branch_op)) 
						      | (6 
							 == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__branch_op))))
						   ? 0x14410000
						   : 
						  ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_if__DOT__saved)
						    ? vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_if__DOT__insn_saved
						    : 
						   ((IData)(vlSymsp->TOP__v.__PVT__icpu_ack_qmem)
						     ? vlSymsp->TOP__v.__PVT__icpu_dat_qmem
						     : 0x14610000)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__if_stall = (1 
						   & (((~ (IData)(vlSymsp->TOP__v.__PVT__icpu_err_immu)) 
						       & (~ (IData)(vlSymsp->TOP__v.__PVT__icpu_ack_qmem))) 
						      & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_if__DOT__saved))));
    vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze = 
	(((((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__lsu_stall) 
	    | ((~ (IData)(vlSymsp->TOP__v.__PVT__dcpu_ack_qmem)) 
	       & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__if_stall))) 
	   | (0 != (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_freeze__DOT__multicycle_cnt))) 
	  | (IData)(vlTOPp->dbg_stall_i)) | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__mac_stall));
    vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze = 
	((((((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__lsu_stall) 
	     | ((~ (IData)(vlSymsp->TOP__v.__PVT__dcpu_ack_qmem)) 
		& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__if_stall))) 
	    | (0 != (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_freeze__DOT__multicycle_cnt))) 
	   | (IData)(vlTOPp->dbg_stall_i)) | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__mac_stall)) 
	 | (((((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__except_dbuserr) 
	       | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__except_dmmufault)) 
	      | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__except_dtlbmiss)) 
	     | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__except_align)) 
	    | (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__except_illegal)));
}

void Vor1200_top_or1200_cpu::_combo__TOP__v__or1200_cpu__20(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"        Vor1200_top_or1200_cpu::_combo__TOP__v__or1200_cpu__20"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze = 
	(((((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__lsu_stall) 
	    | ((~ (IData)(vlSymsp->TOP__v.__PVT__dcpu_ack_qmem)) 
	       & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__if_stall))) 
	   | (0 != (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_freeze__DOT__multicycle_cnt))) 
	  | (IData)(vlTOPp->dbg_stall_i)) | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__mac_stall));
    vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze = 
	((((((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__lsu_stall) 
	     | ((~ (IData)(vlSymsp->TOP__v.__PVT__dcpu_ack_qmem)) 
		& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__if_stall))) 
	    | (0 != (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_freeze__DOT__multicycle_cnt))) 
	   | (IData)(vlTOPp->dbg_stall_i)) | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__mac_stall)) 
	 | (((((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__except_dbuserr) 
	       | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__except_dmmufault)) 
	      | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__except_dtlbmiss)) 
	     | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__except_align)) 
	    | (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__except_illegal)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__if_freeze = 
	((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze) 
	 | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__extend_flush));
    vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.wb_freeze 
	= vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze;
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__tick_pending 
	= (1 & ((((((vlSymsp->TOP__v.__PVT__or1200_tt__DOT__ttmr 
		     >> 0x1c) & ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr) 
				 >> 1)) & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze))) 
		  & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__branch_taken))) 
		 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_dslot))) 
		& (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr_we))));
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__int_pending 
	= (((((((0 != vlSymsp->TOP__v.__PVT__or1200_pic__DOT__um_ints) 
		& ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr) 
		   >> 2)) & ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__delayed_iee) 
			     >> 2)) & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze))) 
	     & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__branch_taken))) 
	    & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_dslot))) 
	   & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr_we)));
}

void Vor1200_top_or1200_cpu::_settle__TOP__v__or1200_cpu__21(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"        Vor1200_top_or1200_cpu::_settle__TOP__v__or1200_cpu__21"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlSymsp->TOP__v__or1200_cpu.__PVT__if_freeze = 
	((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze) 
	 | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__extend_flush));
    vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.wb_freeze 
	= vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze;
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__tick_pending 
	= (1 & ((((((vlSymsp->TOP__v.__PVT__or1200_tt__DOT__ttmr 
		     >> 0x1c) & ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr) 
				 >> 1)) & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze))) 
		  & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__branch_taken))) 
		 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_dslot))) 
		& (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr_we))));
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__int_pending 
	= (((((((0 != vlSymsp->TOP__v.__PVT__or1200_pic__DOT__um_ints) 
		& ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr) 
		   >> 2)) & ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__delayed_iee) 
			     >> 2)) & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze))) 
	     & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__branch_taken))) 
	    & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_dslot))) 
	   & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr_we)));
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__except_trig 
	= ((((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__tick_pending) 
	     & (~ ((IData)(vlSymsp->TOP__v.__PVT__or1200_du__DOT__dsr) 
		   >> 4))) << 0xc) | ((((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__int_pending) 
					& (~ ((IData)(vlSymsp->TOP__v.__PVT__or1200_du__DOT__dsr) 
					      >> 7))) 
				       << 0xb) | ((0x400 
						   & (((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_exceptflags) 
						       << 9) 
						      & ((~ 
							  ((IData)(vlSymsp->TOP__v.__PVT__or1200_du__DOT__dsr) 
							   >> 9)) 
							 << 0xa))) 
						  | ((0x200 
						      & (((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_exceptflags) 
							  & (~ 
							     ((IData)(vlSymsp->TOP__v.__PVT__or1200_du__DOT__dsr) 
							      >> 3))) 
							 << 9)) 
						     | ((0x100 
							 & (((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_exceptflags) 
							     << 6) 
							    & ((~ 
								((IData)(vlSymsp->TOP__v.__PVT__or1200_du__DOT__dsr) 
								 >> 1)) 
							       << 8))) 
							| ((((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__except_illegal) 
							     & (~ 
								((IData)(vlSymsp->TOP__v.__PVT__or1200_du__DOT__dsr) 
								 >> 6))) 
							    << 7) 
							   | ((((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__except_align) 
								& (~ 
								   ((IData)(vlSymsp->TOP__v.__PVT__or1200_du__DOT__dsr) 
								    >> 5))) 
							       << 6) 
							      | ((((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__except_dtlbmiss) 
								   & (~ 
								      ((IData)(vlSymsp->TOP__v.__PVT__or1200_du__DOT__dsr) 
								       >> 8))) 
								  << 5) 
								 | ((((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__except_dmmufault) 
								      & (~ 
									 ((IData)(vlSymsp->TOP__v.__PVT__or1200_du__DOT__dsr) 
									  >> 2))) 
								     << 4) 
								    | ((((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__except_dbuserr) 
									 & (~ 
									    ((IData)(vlSymsp->TOP__v.__PVT__or1200_du__DOT__dsr) 
									     >> 1))) 
									<< 3) 
								       | (((((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__sig_trap) 
									     & (~ 
										((IData)(vlSymsp->TOP__v.__PVT__or1200_du__DOT__dsr) 
										>> 0xd))) 
									    & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze))) 
									   << 1) 
									  | (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__sig_syscall) 
									      & (~ 
										((IData)(vlSymsp->TOP__v.__PVT__or1200_du__DOT__dsr) 
										>> 0xb))) 
									     & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze))))))))))))));
}

void Vor1200_top_or1200_cpu::_combo__TOP__v__or1200_cpu__22(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"        Vor1200_top_or1200_cpu::_combo__TOP__v__or1200_cpu__22"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__except_trig 
	= ((((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__tick_pending) 
	     & (~ ((IData)(vlSymsp->TOP__v.__PVT__or1200_du__DOT__dsr) 
		   >> 4))) << 0xc) | ((((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__int_pending) 
					& (~ ((IData)(vlSymsp->TOP__v.__PVT__or1200_du__DOT__dsr) 
					      >> 7))) 
				       << 0xb) | ((0x400 
						   & (((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_exceptflags) 
						       << 9) 
						      & ((~ 
							  ((IData)(vlSymsp->TOP__v.__PVT__or1200_du__DOT__dsr) 
							   >> 9)) 
							 << 0xa))) 
						  | ((0x200 
						      & (((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_exceptflags) 
							  & (~ 
							     ((IData)(vlSymsp->TOP__v.__PVT__or1200_du__DOT__dsr) 
							      >> 3))) 
							 << 9)) 
						     | ((0x100 
							 & (((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__ex_exceptflags) 
							     << 6) 
							    & ((~ 
								((IData)(vlSymsp->TOP__v.__PVT__or1200_du__DOT__dsr) 
								 >> 1)) 
							       << 8))) 
							| ((((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__except_illegal) 
							     & (~ 
								((IData)(vlSymsp->TOP__v.__PVT__or1200_du__DOT__dsr) 
								 >> 6))) 
							    << 7) 
							   | ((((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__except_align) 
								& (~ 
								   ((IData)(vlSymsp->TOP__v.__PVT__or1200_du__DOT__dsr) 
								    >> 5))) 
							       << 6) 
							      | ((((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__except_dtlbmiss) 
								   & (~ 
								      ((IData)(vlSymsp->TOP__v.__PVT__or1200_du__DOT__dsr) 
								       >> 8))) 
								  << 5) 
								 | ((((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__except_dmmufault) 
								      & (~ 
									 ((IData)(vlSymsp->TOP__v.__PVT__or1200_du__DOT__dsr) 
									  >> 2))) 
								     << 4) 
								    | ((((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__except_dbuserr) 
									 & (~ 
									    ((IData)(vlSymsp->TOP__v.__PVT__or1200_du__DOT__dsr) 
									     >> 1))) 
									<< 3) 
								       | (((((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__sig_trap) 
									     & (~ 
										((IData)(vlSymsp->TOP__v.__PVT__or1200_du__DOT__dsr) 
										>> 0xd))) 
									    & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze))) 
									   << 1) 
									  | (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__sig_syscall) 
									      & (~ 
										((IData)(vlSymsp->TOP__v.__PVT__or1200_du__DOT__dsr) 
										>> 0xb))) 
									     & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze))))))))))))));
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__except_flushpipe 
	= ((0 != (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__except_trig)) 
	   & (~ (IData)((0 != (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__state)))));
}

void Vor1200_top_or1200_cpu::_settle__TOP__v__or1200_cpu__23(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"        Vor1200_top_or1200_cpu::_settle__TOP__v__or1200_cpu__23"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__except_flushpipe 
	= ((0 != (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__except_trig)) 
	   & (~ (IData)((0 != (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__state)))));
    vlSymsp->TOP__v__or1200_cpu.__PVT__flushpipe = 
	(((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__except_flushpipe) 
	  | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__pc_we)) 
	 | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__extend_flush));
}

void Vor1200_top_or1200_cpu::_combo__TOP__v__or1200_cpu__24(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"        Vor1200_top_or1200_cpu::_combo__TOP__v__or1200_cpu__24"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlSymsp->TOP__v__or1200_cpu.__PVT__flushpipe = 
	(((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__or1200_except__DOT__except_flushpipe) 
	  | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__pc_we)) 
	 | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__extend_flush));
}
