// Verilated -*- C++ -*-
#include "Vor1200_top_or1200_rf.h" // For This
#include "Vor1200_top__Syms.h"

//--------------------
// STATIC VARIABLES


//--------------------

VL_CTOR_IMP(Vor1200_top_or1200_rf) {
    VL_CELL (rf_a, Vor1200_top_or1200_rfram_generic);
    // Reset internal values
    // Reset structure values
    __PVT__clk = VL_RAND_RESET_I(1);
    __PVT__rst = VL_RAND_RESET_I(1);
    __PVT__supv = VL_RAND_RESET_I(1);
    __PVT__wb_freeze = VL_RAND_RESET_I(1);
    __PVT__addrw = VL_RAND_RESET_I(5);
    __PVT__dataw = VL_RAND_RESET_I(32);
    __PVT__we = VL_RAND_RESET_I(1);
    __PVT__flushpipe = VL_RAND_RESET_I(1);
    __PVT__id_freeze = VL_RAND_RESET_I(1);
    __PVT__addra = VL_RAND_RESET_I(5);
    __PVT__addrb = VL_RAND_RESET_I(5);
    __PVT__dataa = VL_RAND_RESET_I(32);
    __PVT__datab = VL_RAND_RESET_I(32);
    __PVT__rda = VL_RAND_RESET_I(1);
    __PVT__rdb = VL_RAND_RESET_I(1);
    __PVT__spr_cs = VL_RAND_RESET_I(1);
    __PVT__spr_write = VL_RAND_RESET_I(1);
    __PVT__spr_addr = VL_RAND_RESET_I(32);
    __PVT__spr_dat_i = VL_RAND_RESET_I(32);
    __PVT__spr_dat_o = VL_RAND_RESET_I(32);
    __PVT__dataa_saved = VL_RAND_RESET_Q(33);
    __PVT__datab_saved = VL_RAND_RESET_Q(33);
    __PVT__rf_addrw = VL_RAND_RESET_I(5);
    __PVT__rf_we = VL_RAND_RESET_I(1);
    __PVT__spr_valid = VL_RAND_RESET_I(1);
    __PVT__rf_we_allow = VL_RAND_RESET_I(1);
    __Vdly__dataa_saved = VL_RAND_RESET_Q(33);
    __Vdly__datab_saved = VL_RAND_RESET_Q(33);
}

void Vor1200_top_or1200_rf::__Vconfigure(Vor1200_top__Syms* vlSymsp, bool first) {
    if (0 && first) {}  // Prevent unused
    this->__VlSymsp = vlSymsp;
}

Vor1200_top_or1200_rf::~Vor1200_top_or1200_rf() {
}

//--------------------
// Internal Methods

void Vor1200_top_or1200_rf::_sequent__TOP__v__or1200_cpu__or1200_rf__1(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"          Vor1200_top_or1200_rf::_sequent__TOP__v__or1200_cpu__or1200_rf__1"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlSymsp->TOP__v__or1200_cpu__or1200_rf.__Vdly__datab_saved 
	= vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__datab_saved;
    vlSymsp->TOP__v__or1200_cpu__or1200_rf.__Vdly__dataa_saved 
	= vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__dataa_saved;
    // ALWAYS at ../or1k-verilog-rtl/or1200_rf.v:242
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu__or1200_rf.__Vdly__datab_saved = VL_ULL(0);
    } else {
	if (((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze) 
	     & (~ (IData)((vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__datab_saved 
			   >> 0x20))))) {
	    vlSymsp->TOP__v__or1200_cpu__or1200_rf.__Vdly__datab_saved 
		= (VL_ULL(0x100000000) | (QData)((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__do_b)));
	} else {
	    if ((1 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze)))) {
		vlSymsp->TOP__v__or1200_cpu__or1200_rf.__Vdly__datab_saved = VL_ULL(0);
	    }
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_rf.v:229
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu__or1200_rf.__Vdly__dataa_saved = VL_ULL(0);
    } else {
	if (((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze) 
	     & (~ (IData)((vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__dataa_saved 
			   >> 0x20))))) {
	    vlSymsp->TOP__v__or1200_cpu__or1200_rf.__Vdly__dataa_saved 
		= (VL_ULL(0x100000000) | (QData)((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__do_a)));
	} else {
	    if ((1 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze)))) {
		vlSymsp->TOP__v__or1200_cpu__or1200_rf.__Vdly__dataa_saved = VL_ULL(0);
	    }
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_rf.v:202
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_we_allow = 1;
    } else {
	if ((1 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)))) {
	    vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_we_allow 
		= (1 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__flushpipe)));
	}
    }
    vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__datab_saved 
	= vlSymsp->TOP__v__or1200_cpu__or1200_rf.__Vdly__datab_saved;
    vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__dataa_saved 
	= vlSymsp->TOP__v__or1200_cpu__or1200_rf.__Vdly__dataa_saved;
}

void Vor1200_top_or1200_rf::_settle__TOP__v__or1200_cpu__or1200_rf__2(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"          Vor1200_top_or1200_rf::_settle__TOP__v__or1200_cpu__or1200_rf__2"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid 
	= (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_cs 
	   & (0x20 == (0x3f & (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr 
			       >> 5))));
}

void Vor1200_top_or1200_rf::_combo__TOP__v__or1200_cpu__or1200_rf__3(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"          Vor1200_top_or1200_rf::_combo__TOP__v__or1200_cpu__or1200_rf__3"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid 
	= (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_cs 
	   & (0x20 == (0x3f & (vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr 
			       >> 5))));
    vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_addrw 
	= (0x1f & (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
		    & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))
		    ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr
		    : (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rf_addrw)));
}

void Vor1200_top_or1200_rf::_settle__TOP__v__or1200_cpu__or1200_rf__4(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"          Vor1200_top_or1200_rf::_settle__TOP__v__or1200_cpu__or1200_rf__4"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_addrw 
	= (0x1f & (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
		    & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))
		    ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr
		    : (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rf_addrw)));
}

void Vor1200_top_or1200_rf::_combo__TOP__v__or1200_cpu__or1200_rf__5(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"          Vor1200_top_or1200_rf::_combo__TOP__v__or1200_cpu__or1200_rf__5"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_we 
	= (((((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
	      & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we)) 
	     | ((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rfwb_op) 
		& (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)))) 
	    & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_we_allow)) 
	   & ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__sr) 
	      | (0 != (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_addrw))));
}
