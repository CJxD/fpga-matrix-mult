// Verilated -*- C++ -*-
#include "Vor1200_top.h"       // For This
#include "Vor1200_top__Syms.h"

//--------------------
// STATIC VARIABLES


//--------------------

VL_CTOR_IMP(Vor1200_top) {
    Vor1200_top__Syms* __restrict vlSymsp = __VlSymsp = new Vor1200_top__Syms(this, name());
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    VL_CELL (v, Vor1200_top_or1200_top);
    // Reset internal values
    
    // Reset structure values
    clk_i = VL_RAND_RESET_I(1);
    rst_i = VL_RAND_RESET_I(1);
    clmode_i = VL_RAND_RESET_I(2);
    pic_ints_i = VL_RAND_RESET_I(20);
    iwb_clk_i = VL_RAND_RESET_I(1);
    iwb_rst_i = VL_RAND_RESET_I(1);
    iwb_ack_i = VL_RAND_RESET_I(1);
    iwb_err_i = VL_RAND_RESET_I(1);
    iwb_rty_i = VL_RAND_RESET_I(1);
    iwb_dat_i = VL_RAND_RESET_I(32);
    iwb_cyc_o = VL_RAND_RESET_I(1);
    iwb_adr_o = VL_RAND_RESET_I(32);
    iwb_stb_o = VL_RAND_RESET_I(1);
    iwb_we_o = VL_RAND_RESET_I(1);
    iwb_sel_o = VL_RAND_RESET_I(4);
    iwb_dat_o = VL_RAND_RESET_I(32);
    iwb_cab_o = VL_RAND_RESET_I(1);
    dwb_clk_i = VL_RAND_RESET_I(1);
    dwb_rst_i = VL_RAND_RESET_I(1);
    dwb_ack_i = VL_RAND_RESET_I(1);
    dwb_err_i = VL_RAND_RESET_I(1);
    dwb_rty_i = VL_RAND_RESET_I(1);
    dwb_dat_i = VL_RAND_RESET_I(32);
    dwb_cyc_o = VL_RAND_RESET_I(1);
    dwb_adr_o = VL_RAND_RESET_I(32);
    dwb_stb_o = VL_RAND_RESET_I(1);
    dwb_we_o = VL_RAND_RESET_I(1);
    dwb_sel_o = VL_RAND_RESET_I(4);
    dwb_dat_o = VL_RAND_RESET_I(32);
    dwb_cab_o = VL_RAND_RESET_I(1);
    dbg_stall_i = VL_RAND_RESET_I(1);
    dbg_ewt_i = VL_RAND_RESET_I(1);
    dbg_lss_o = VL_RAND_RESET_I(4);
    dbg_is_o = VL_RAND_RESET_I(2);
    dbg_wp_o = VL_RAND_RESET_I(11);
    dbg_bp_o = VL_RAND_RESET_I(1);
    dbg_stb_i = VL_RAND_RESET_I(1);
    dbg_we_i = VL_RAND_RESET_I(1);
    dbg_adr_i = VL_RAND_RESET_I(32);
    dbg_dat_i = VL_RAND_RESET_I(32);
    dbg_dat_o = VL_RAND_RESET_I(32);
    dbg_ack_o = VL_RAND_RESET_I(1);
    pm_cpustall_i = VL_RAND_RESET_I(1);
    pm_clksd_o = VL_RAND_RESET_I(4);
    pm_dc_gate_o = VL_RAND_RESET_I(1);
    pm_ic_gate_o = VL_RAND_RESET_I(1);
    pm_dmmu_gate_o = VL_RAND_RESET_I(1);
    pm_immu_gate_o = VL_RAND_RESET_I(1);
    pm_tt_gate_o = VL_RAND_RESET_I(1);
    pm_cpu_gate_o = VL_RAND_RESET_I(1);
    pm_wakeup_o = VL_RAND_RESET_I(1);
    pm_lvolt_o = VL_RAND_RESET_I(1);
    __Vclklast__TOP__dwb_clk_i = VL_RAND_RESET_I(1);
    __Vclklast__TOP__dwb_rst_i = VL_RAND_RESET_I(1);
    __Vclklast__TOP__iwb_clk_i = VL_RAND_RESET_I(1);
    __Vclklast__TOP__iwb_rst_i = VL_RAND_RESET_I(1);
    __Vclklast__TOP__clk_i = VL_RAND_RESET_I(1);
    __Vclklast__TOP__rst_i = VL_RAND_RESET_I(1);
}

void Vor1200_top::__Vconfigure(Vor1200_top__Syms* vlSymsp, bool first) {
    if (0 && first) {}  // Prevent unused
    this->__VlSymsp = vlSymsp;
}

Vor1200_top::~Vor1200_top() {
    delete __VlSymsp; __VlSymsp=NULL;
}

//--------------------


void Vor1200_top::eval() {
    Vor1200_top__Syms* __restrict vlSymsp = this->__VlSymsp; // Setup global symbol table
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Initialize
    if (VL_UNLIKELY(!vlSymsp->__Vm_didInit)) _eval_initial_loop(vlSymsp);
    // Evaluate till stable
    VL_DEBUG_IF(cout<<"\n----TOP Evaluate Vor1200_top::eval"<<endl; );
    int __VclockLoop = 0;
    IData __Vchange=1;
    while (VL_LIKELY(__Vchange)) {
	VL_DEBUG_IF(cout<<" Clock loop"<<endl;);
	vlSymsp->__Vm_activity = true;
	_eval(vlSymsp);
	__Vchange = _change_request(vlSymsp);
	if (++__VclockLoop > 100) vl_fatal(__FILE__,__LINE__,__FILE__,"Verilated model didn't converge");
    }
}

void Vor1200_top::_eval_initial_loop(Vor1200_top__Syms* __restrict vlSymsp) {
    vlSymsp->__Vm_didInit = true;
    _eval_initial(vlSymsp);
    vlSymsp->__Vm_activity = true;
    int __VclockLoop = 0;
    IData __Vchange=1;
    while (VL_LIKELY(__Vchange)) {
	_eval_settle(vlSymsp);
	_eval(vlSymsp);
	__Vchange = _change_request(vlSymsp);
	if (++__VclockLoop > 100) vl_fatal(__FILE__,__LINE__,__FILE__,"Verilated model didn't DC converge");
    }
}

//--------------------
// Internal Methods

void Vor1200_top::_settle__TOP__1(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"    Vor1200_top::_settle__TOP__1"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlTOPp->dbg_wp_o = 0;
    vlTOPp->dbg_lss_o = 0;
}

void Vor1200_top::_sequent__TOP__2(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"    Vor1200_top::_sequent__TOP__2"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlTOPp->dwb_cab_o = vlSymsp->TOP__v.__PVT__dwb_cab_o;
    vlTOPp->dwb_cyc_o = vlSymsp->TOP__v.__PVT__dwb_cyc_o;
    vlTOPp->dwb_dat_o = vlSymsp->TOP__v.__PVT__dwb_dat_o;
    vlTOPp->dwb_adr_o = vlSymsp->TOP__v.__PVT__dwb_adr_o;
    vlTOPp->dwb_we_o = vlSymsp->TOP__v.__PVT__dwb_we_o;
    vlTOPp->dwb_sel_o = vlSymsp->TOP__v.__PVT__dwb_sel_o;
    vlTOPp->dwb_stb_o = vlSymsp->TOP__v.__PVT__dwb_stb_o;
}

void Vor1200_top::_sequent__TOP__3(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"    Vor1200_top::_sequent__TOP__3"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlTOPp->iwb_cab_o = vlSymsp->TOP__v.__PVT__iwb_cab_o;
    vlTOPp->iwb_we_o = vlSymsp->TOP__v.__PVT__iwb_we_o;
    vlTOPp->iwb_cyc_o = vlSymsp->TOP__v.__PVT__iwb_cyc_o;
    vlTOPp->iwb_dat_o = vlSymsp->TOP__v.__PVT__iwb_dat_o;
    vlTOPp->iwb_adr_o = vlSymsp->TOP__v.__PVT__iwb_adr_o;
    vlTOPp->iwb_sel_o = vlSymsp->TOP__v.__PVT__iwb_sel_o;
    vlTOPp->iwb_stb_o = vlSymsp->TOP__v.__PVT__iwb_stb_o;
}

void Vor1200_top::_sequent__TOP__4(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"    Vor1200_top::_sequent__TOP__4"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlTOPp->dbg_ack_o = vlSymsp->TOP__v.__PVT__dbg_ack_o;
    vlTOPp->dbg_is_o = vlSymsp->TOP__v.__PVT__dbg_is_o;
    vlTOPp->pm_clksd_o = vlSymsp->TOP__v.__PVT__or1200_pm__DOT__sdf;
    vlTOPp->dbg_bp_o = vlSymsp->TOP__v.__PVT__or1200_du__DOT__dbg_bp_r;
}

void Vor1200_top::_settle__TOP__5(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"    Vor1200_top::_settle__TOP__5"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlTOPp->dwb_cab_o = vlSymsp->TOP__v.__PVT__dwb_cab_o;
    vlTOPp->dwb_cyc_o = vlSymsp->TOP__v.__PVT__dwb_cyc_o;
    vlTOPp->dwb_dat_o = vlSymsp->TOP__v.__PVT__dwb_dat_o;
    vlTOPp->dwb_adr_o = vlSymsp->TOP__v.__PVT__dwb_adr_o;
    vlTOPp->dwb_we_o = vlSymsp->TOP__v.__PVT__dwb_we_o;
    vlTOPp->dwb_sel_o = vlSymsp->TOP__v.__PVT__dwb_sel_o;
    vlTOPp->dwb_stb_o = vlSymsp->TOP__v.__PVT__dwb_stb_o;
    vlTOPp->iwb_cab_o = vlSymsp->TOP__v.__PVT__iwb_cab_o;
    vlTOPp->iwb_we_o = vlSymsp->TOP__v.__PVT__iwb_we_o;
    vlTOPp->iwb_cyc_o = vlSymsp->TOP__v.__PVT__iwb_cyc_o;
    vlTOPp->iwb_dat_o = vlSymsp->TOP__v.__PVT__iwb_dat_o;
    vlTOPp->iwb_adr_o = vlSymsp->TOP__v.__PVT__iwb_adr_o;
    vlTOPp->iwb_sel_o = vlSymsp->TOP__v.__PVT__iwb_sel_o;
    vlTOPp->iwb_stb_o = vlSymsp->TOP__v.__PVT__iwb_stb_o;
    vlTOPp->dbg_ack_o = vlSymsp->TOP__v.__PVT__dbg_ack_o;
    vlTOPp->dbg_is_o = vlSymsp->TOP__v.__PVT__dbg_is_o;
    vlTOPp->pm_clksd_o = vlSymsp->TOP__v.__PVT__or1200_pm__DOT__sdf;
    vlTOPp->dbg_bp_o = vlSymsp->TOP__v.__PVT__or1200_du__DOT__dbg_bp_r;
}

void Vor1200_top::_settle__TOP__6(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"    Vor1200_top::_settle__TOP__6"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlTOPp->pm_wakeup_o = (0 != vlSymsp->TOP__v.__PVT__or1200_pic__DOT__um_ints);
    vlTOPp->pm_tt_gate_o = ((IData)(vlSymsp->TOP__v.__PVT__or1200_pm__DOT__sme) 
			    & (~ (IData)((0 != vlSymsp->TOP__v.__PVT__or1200_pic__DOT__um_ints))));
}

void Vor1200_top::_combo__TOP__7(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"    Vor1200_top::_combo__TOP__7"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlTOPp->pm_wakeup_o = (0 != vlSymsp->TOP__v.__PVT__or1200_pic__DOT__um_ints);
    vlTOPp->pm_tt_gate_o = ((IData)(vlSymsp->TOP__v.__PVT__or1200_pm__DOT__sme) 
			    & (~ (IData)((0 != vlSymsp->TOP__v.__PVT__or1200_pic__DOT__um_ints))));
    vlTOPp->pm_dc_gate_o = vlSymsp->TOP__v.__PVT__pm_cpu_gate_o;
    vlTOPp->pm_ic_gate_o = vlSymsp->TOP__v.__PVT__pm_cpu_gate_o;
    vlTOPp->pm_dmmu_gate_o = vlSymsp->TOP__v.__PVT__pm_cpu_gate_o;
    vlTOPp->pm_immu_gate_o = vlSymsp->TOP__v.__PVT__pm_cpu_gate_o;
    vlTOPp->pm_cpu_gate_o = vlSymsp->TOP__v.__PVT__pm_cpu_gate_o;
    vlTOPp->pm_lvolt_o = ((IData)(vlSymsp->TOP__v.__PVT__pm_cpu_gate_o) 
			  | (IData)(vlTOPp->pm_cpustall_i));
}

void Vor1200_top::_settle__TOP__8(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"    Vor1200_top::_settle__TOP__8"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlTOPp->pm_dc_gate_o = vlSymsp->TOP__v.__PVT__pm_cpu_gate_o;
    vlTOPp->pm_ic_gate_o = vlSymsp->TOP__v.__PVT__pm_cpu_gate_o;
    vlTOPp->pm_dmmu_gate_o = vlSymsp->TOP__v.__PVT__pm_cpu_gate_o;
    vlTOPp->pm_immu_gate_o = vlSymsp->TOP__v.__PVT__pm_cpu_gate_o;
    vlTOPp->pm_cpu_gate_o = vlSymsp->TOP__v.__PVT__pm_cpu_gate_o;
    vlTOPp->pm_lvolt_o = ((IData)(vlSymsp->TOP__v.__PVT__pm_cpu_gate_o) 
			  | (IData)(vlTOPp->pm_cpustall_i));
}

void Vor1200_top::_combo__TOP__9(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"    Vor1200_top::_combo__TOP__9"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlTOPp->dbg_dat_o = ((IData)(vlSymsp->TOP__v.__PVT__du_write)
			  ? vlTOPp->dbg_dat_i : ((IData)(vlSymsp->TOP__v.__PVT__du_read)
						  ? vlSymsp->TOP__v__or1200_cpu.__PVT__sprs_dataout
						  : vlSymsp->TOP__v__or1200_cpu.__PVT__operand_b));
}

void Vor1200_top::_eval(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"    Vor1200_top::_eval"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    if ((((IData)(vlTOPp->dwb_clk_i) & (~ (IData)(vlTOPp->__Vclklast__TOP__dwb_clk_i))) 
	 | ((IData)(vlTOPp->dwb_rst_i) & (~ (IData)(vlTOPp->__Vclklast__TOP__dwb_rst_i))))) {
	vlSymsp->TOP__v._sequent__TOP__v__2(vlSymsp);
	vlTOPp->_sequent__TOP__2(vlSymsp);
    }
    if ((((IData)(vlTOPp->iwb_clk_i) & (~ (IData)(vlTOPp->__Vclklast__TOP__iwb_clk_i))) 
	 | ((IData)(vlTOPp->iwb_rst_i) & (~ (IData)(vlTOPp->__Vclklast__TOP__iwb_rst_i))))) {
	vlSymsp->TOP__v._sequent__TOP__v__3(vlSymsp);
	vlTOPp->_sequent__TOP__3(vlSymsp);
    }
    if ((((IData)(vlTOPp->clk_i) & (~ (IData)(vlTOPp->__Vclklast__TOP__clk_i))) 
	 | ((IData)(vlTOPp->rst_i) & (~ (IData)(vlTOPp->__Vclklast__TOP__rst_i))))) {
	vlSymsp->TOP__v._sequent__TOP__v__4(vlSymsp);
	vlSymsp->TOP__v__or1200_cpu._sequent__TOP__v__or1200_cpu__1(vlSymsp);
	vlSymsp->TOP__v__or1200_cpu__or1200_ctrl._sequent__TOP__v__or1200_cpu__or1200_ctrl__1(vlSymsp);
	vlSymsp->TOP__v__or1200_cpu__or1200_rf._sequent__TOP__v__or1200_cpu__or1200_rf__1(vlSymsp);
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a._sequent__TOP__v__or1200_cpu__or1200_rf__rf_a__1(vlSymsp);
	vlTOPp->_sequent__TOP__4(vlSymsp);
	vlSymsp->TOP__v._sequent__TOP__v__5(vlSymsp);
	vlSymsp->TOP__v__or1200_cpu._sequent__TOP__v__or1200_cpu__2(vlSymsp);
    }
    vlSymsp->TOP__v._combo__TOP__v__6(vlSymsp);
    vlSymsp->TOP__v__or1200_cpu._combo__TOP__v__or1200_cpu__3(vlSymsp);
    if (((IData)(vlTOPp->clk_i) & (~ (IData)(vlTOPp->__Vclklast__TOP__clk_i)))) {
	vlSymsp->TOP__v._sequent__TOP__v__7(vlSymsp);
    }
    if ((((IData)(vlTOPp->clk_i) & (~ (IData)(vlTOPp->__Vclklast__TOP__clk_i))) 
	 | ((IData)(vlTOPp->rst_i) & (~ (IData)(vlTOPp->__Vclklast__TOP__rst_i))))) {
	vlSymsp->TOP__v._sequent__TOP__v__9(vlSymsp);
	vlSymsp->TOP__v__or1200_cpu__or1200_ctrl._sequent__TOP__v__or1200_cpu__or1200_ctrl__2(vlSymsp);
	vlSymsp->TOP__v__or1200_cpu._sequent__TOP__v__or1200_cpu__5(vlSymsp);
	vlSymsp->TOP__v._sequent__TOP__v__10(vlSymsp);
    }
    vlSymsp->TOP__v._combo__TOP__v__11(vlSymsp);
    vlSymsp->TOP__v__or1200_cpu._combo__TOP__v__or1200_cpu__6(vlSymsp);
    vlTOPp->_combo__TOP__7(vlSymsp);
    if ((((IData)(vlTOPp->clk_i) & (~ (IData)(vlTOPp->__Vclklast__TOP__clk_i))) 
	 | ((IData)(vlTOPp->rst_i) & (~ (IData)(vlTOPp->__Vclklast__TOP__rst_i))))) {
	vlSymsp->TOP__v._sequent__TOP__v__14(vlSymsp);
	vlSymsp->TOP__v__or1200_cpu._sequent__TOP__v__or1200_cpu__8(vlSymsp);
	vlSymsp->TOP__v._sequent__TOP__v__15(vlSymsp);
    }
    vlSymsp->TOP__v._combo__TOP__v__16(vlSymsp);
    vlSymsp->TOP__v__or1200_cpu__or1200_rf._combo__TOP__v__or1200_cpu__or1200_rf__3(vlSymsp);
    vlSymsp->TOP__v__or1200_cpu._combo__TOP__v__or1200_cpu__9(vlSymsp);
    vlSymsp->TOP__v__or1200_cpu._combo__TOP__v__or1200_cpu__12(vlSymsp);
    vlSymsp->TOP__v._combo__TOP__v__18(vlSymsp);
    vlSymsp->TOP__v__or1200_cpu__or1200_ctrl._combo__TOP__v__or1200_cpu__or1200_ctrl__4(vlSymsp);
    if ((((IData)(vlTOPp->clk_i) & (~ (IData)(vlTOPp->__Vclklast__TOP__clk_i))) 
	 | ((IData)(vlTOPp->rst_i) & (~ (IData)(vlTOPp->__Vclklast__TOP__rst_i))))) {
	vlSymsp->TOP__v._sequent__TOP__v__19(vlSymsp);
    }
    vlSymsp->TOP__v._combo__TOP__v__21(vlSymsp);
    vlSymsp->TOP__v__or1200_cpu._settle__TOP__v__or1200_cpu__13(vlSymsp);
    vlSymsp->TOP__v._combo__TOP__v__22(vlSymsp);
    vlSymsp->TOP__v._combo__TOP__v__24(vlSymsp);
    vlSymsp->TOP__v__or1200_cpu._combo__TOP__v__or1200_cpu__16(vlSymsp);
    vlTOPp->_combo__TOP__9(vlSymsp);
    vlSymsp->TOP__v._combo__TOP__v__26(vlSymsp);
    vlSymsp->TOP__v__or1200_cpu._combo__TOP__v__or1200_cpu__18(vlSymsp);
    vlSymsp->TOP__v._combo__TOP__v__28(vlSymsp);
    vlSymsp->TOP__v__or1200_cpu._combo__TOP__v__or1200_cpu__20(vlSymsp);
    vlSymsp->TOP__v__or1200_cpu__or1200_rf._combo__TOP__v__or1200_cpu__or1200_rf__5(vlSymsp);
    vlSymsp->TOP__v._settle__TOP__v__30(vlSymsp);
    vlSymsp->TOP__v__or1200_cpu._combo__TOP__v__or1200_cpu__22(vlSymsp);
    vlSymsp->TOP__v__or1200_cpu._combo__TOP__v__or1200_cpu__24(vlSymsp);
    // Final
    vlTOPp->__Vclklast__TOP__dwb_clk_i = vlTOPp->dwb_clk_i;
    vlTOPp->__Vclklast__TOP__dwb_rst_i = vlTOPp->dwb_rst_i;
    vlTOPp->__Vclklast__TOP__iwb_clk_i = vlTOPp->iwb_clk_i;
    vlTOPp->__Vclklast__TOP__iwb_rst_i = vlTOPp->iwb_rst_i;
    vlTOPp->__Vclklast__TOP__clk_i = vlTOPp->clk_i;
    vlTOPp->__Vclklast__TOP__rst_i = vlTOPp->rst_i;
}

void Vor1200_top::_eval_initial(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"    Vor1200_top::_eval_initial"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
}

void Vor1200_top::final() {
    VL_DEBUG_IF(cout<<"    Vor1200_top::final"<<endl; );
    // Variables
    Vor1200_top__Syms* __restrict vlSymsp = this->__VlSymsp;
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
}

void Vor1200_top::_eval_settle(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"    Vor1200_top::_eval_settle"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlTOPp->_settle__TOP__1(vlSymsp);
    vlSymsp->TOP__v._settle__TOP__v__1(vlSymsp);
    vlTOPp->_settle__TOP__5(vlSymsp);
    vlSymsp->TOP__v._settle__TOP__v__8(vlSymsp);
    vlSymsp->TOP__v__or1200_cpu._settle__TOP__v__or1200_cpu__4(vlSymsp);
    vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a._settle__TOP__v__or1200_cpu__or1200_rf__rf_a__2(vlSymsp);
    vlTOPp->_settle__TOP__6(vlSymsp);
    vlSymsp->TOP__v._settle__TOP__v__12(vlSymsp);
    vlSymsp->TOP__v__or1200_cpu__or1200_ctrl._settle__TOP__v__or1200_cpu__or1200_ctrl__3(vlSymsp);
    vlSymsp->TOP__v__or1200_cpu._settle__TOP__v__or1200_cpu__7(vlSymsp);
    vlTOPp->_settle__TOP__8(vlSymsp);
    vlSymsp->TOP__v._settle__TOP__v__13(vlSymsp);
    vlSymsp->TOP__v__or1200_cpu__or1200_rf._settle__TOP__v__or1200_cpu__or1200_rf__2(vlSymsp);
    vlSymsp->TOP__v__or1200_cpu._settle__TOP__v__or1200_cpu__10(vlSymsp);
    vlSymsp->TOP__v._settle__TOP__v__17(vlSymsp);
    vlSymsp->TOP__v__or1200_cpu__or1200_rf._settle__TOP__v__or1200_cpu__or1200_rf__4(vlSymsp);
    vlSymsp->TOP__v__or1200_cpu._settle__TOP__v__or1200_cpu__11(vlSymsp);
    vlSymsp->TOP__v__or1200_cpu__or1200_ctrl._combo__TOP__v__or1200_cpu__or1200_ctrl__4(vlSymsp);
    vlSymsp->TOP__v._settle__TOP__v__20(vlSymsp);
    vlSymsp->TOP__v__or1200_cpu._settle__TOP__v__or1200_cpu__13(vlSymsp);
    vlSymsp->TOP__v._settle__TOP__v__23(vlSymsp);
    vlSymsp->TOP__v__or1200_cpu._settle__TOP__v__or1200_cpu__15(vlSymsp);
    vlSymsp->TOP__v__or1200_cpu._settle__TOP__v__or1200_cpu__17(vlSymsp);
    vlSymsp->TOP__v._settle__TOP__v__25(vlSymsp);
    vlTOPp->_combo__TOP__9(vlSymsp);
    vlSymsp->TOP__v._settle__TOP__v__27(vlSymsp);
    vlSymsp->TOP__v__or1200_cpu._settle__TOP__v__or1200_cpu__19(vlSymsp);
    vlSymsp->TOP__v._settle__TOP__v__29(vlSymsp);
    vlSymsp->TOP__v__or1200_cpu._settle__TOP__v__or1200_cpu__21(vlSymsp);
    vlSymsp->TOP__v__or1200_cpu__or1200_rf._combo__TOP__v__or1200_cpu__or1200_rf__5(vlSymsp);
    vlSymsp->TOP__v._settle__TOP__v__30(vlSymsp);
    vlSymsp->TOP__v__or1200_cpu._settle__TOP__v__or1200_cpu__23(vlSymsp);
}

bool Vor1200_top::_change_request(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"    Vor1200_top::_change_request"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    // Change detection
    IData __req = false;  // Logically a bool
    return __req;
}
