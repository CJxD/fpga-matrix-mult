// Verilated -*- C++ -*-
#include "Vor1200_top_or1200_rfram_generic.h" // For This
#include "Vor1200_top__Syms.h"

//--------------------
// STATIC VARIABLES


//--------------------

VL_CTOR_IMP(Vor1200_top_or1200_rfram_generic) {
    // Reset internal values
    // Reset structure values
    __PVT__clk = VL_RAND_RESET_I(1);
    __PVT__rst = VL_RAND_RESET_I(1);
    __PVT__ce_a = VL_RAND_RESET_I(1);
    __PVT__addr_a = VL_RAND_RESET_I(5);
    __PVT__do_a = VL_RAND_RESET_I(32);
    __PVT__ce_b = VL_RAND_RESET_I(1);
    __PVT__addr_b = VL_RAND_RESET_I(5);
    __PVT__do_b = VL_RAND_RESET_I(32);
    __PVT__ce_w = VL_RAND_RESET_I(1);
    __PVT__we_w = VL_RAND_RESET_I(1);
    __PVT__addr_w = VL_RAND_RESET_I(5);
    __PVT__di_w = VL_RAND_RESET_I(32);
    __PVT__intaddr_a = VL_RAND_RESET_I(5);
    __PVT__intaddr_b = VL_RAND_RESET_I(5);
    VL_RAND_RESET_W(1024,__PVT__mem);
}

void Vor1200_top_or1200_rfram_generic::__Vconfigure(Vor1200_top__Syms* vlSymsp, bool first) {
    if (0 && first) {}  // Prevent unused
    this->__VlSymsp = vlSymsp;
}

Vor1200_top_or1200_rfram_generic::~Vor1200_top_or1200_rfram_generic() {
}

//--------------------
// Internal Methods

uint32_t Vor1200_top_or1200_rfram_generic::get_gpr(uint32_t __PVT__gpr_no) {
    VL_DEBUG_IF(cout<<"            Vor1200_top_or1200_rfram_generic::get_gpr"<<endl; );
    // Variables
    VL_OUT(__PVT__get_gpr,31,0);
    Vor1200_top__Syms* __restrict vlSymsp = this->__VlSymsp;
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    __PVT__get_gpr = ((0x80000000 & ((vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[
				      (0x1f & (((IData)(0x1f) 
						+ ((IData)(__PVT__gpr_no) 
						   << 5)) 
					       >> 5))] 
				      >> (0x1f & ((IData)(0x1f) 
						  + 
						  ((IData)(__PVT__gpr_no) 
						   << 5)))) 
				     << 0x1f)) | ((0x40000000 
						   & ((vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[
						       (0x1f 
							& (((IData)(0x1e) 
							    + 
							    ((IData)(__PVT__gpr_no) 
							     << 5)) 
							   >> 5))] 
						       >> 
						       (0x1f 
							& ((IData)(0x1e) 
							   + 
							   ((IData)(__PVT__gpr_no) 
							    << 5)))) 
						      << 0x1e)) 
						  | ((0x20000000 
						      & ((vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[
							  (0x1f 
							   & (((IData)(0x1d) 
							       + 
							       ((IData)(__PVT__gpr_no) 
								<< 5)) 
							      >> 5))] 
							  >> 
							  (0x1f 
							   & ((IData)(0x1d) 
							      + 
							      ((IData)(__PVT__gpr_no) 
							       << 5)))) 
							 << 0x1d)) 
						     | ((0x10000000 
							 & ((vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[
							     (0x1f 
							      & (((IData)(0x1c) 
								  + 
								  ((IData)(__PVT__gpr_no) 
								   << 5)) 
								 >> 5))] 
							     >> 
							     (0x1f 
							      & ((IData)(0x1c) 
								 + 
								 ((IData)(__PVT__gpr_no) 
								  << 5)))) 
							    << 0x1c)) 
							| ((0x8000000 
							    & ((vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[
								(0x1f 
								 & (((IData)(0x1b) 
								     + 
								     ((IData)(__PVT__gpr_no) 
								      << 5)) 
								    >> 5))] 
								>> 
								(0x1f 
								 & ((IData)(0x1b) 
								    + 
								    ((IData)(__PVT__gpr_no) 
								     << 5)))) 
							       << 0x1b)) 
							   | ((0x4000000 
							       & ((vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[
								   (0x1f 
								    & (((IData)(0x1a) 
									+ 
									((IData)(__PVT__gpr_no) 
									 << 5)) 
								       >> 5))] 
								   >> 
								   (0x1f 
								    & ((IData)(0x1a) 
								       + 
								       ((IData)(__PVT__gpr_no) 
									<< 5)))) 
								  << 0x1a)) 
							      | ((0x2000000 
								  & ((vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[
								      (0x1f 
								       & (((IData)(0x19) 
									   + 
									   ((IData)(__PVT__gpr_no) 
									    << 5)) 
									  >> 5))] 
								      >> 
								      (0x1f 
								       & ((IData)(0x19) 
									  + 
									  ((IData)(__PVT__gpr_no) 
									   << 5)))) 
								     << 0x19)) 
								 | ((0x1000000 
								     & ((vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[
									 (0x1f 
									  & (((IData)(0x18) 
									      + 
									      ((IData)(__PVT__gpr_no) 
									       << 5)) 
									     >> 5))] 
									 >> 
									 (0x1f 
									  & ((IData)(0x18) 
									     + 
									     ((IData)(__PVT__gpr_no) 
									      << 5)))) 
									<< 0x18)) 
								    | ((0x800000 
									& ((vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[
									    (0x1f 
									     & (((IData)(0x17) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)) 
										>> 5))] 
									    >> 
									    (0x1f 
									     & ((IData)(0x17) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)))) 
									   << 0x17)) 
								       | ((0x400000 
									   & ((vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[
									       (0x1f 
										& (((IData)(0x16) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)) 
										>> 5))] 
									       >> 
									       (0x1f 
										& ((IData)(0x16) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)))) 
									      << 0x16)) 
									  | ((0x200000 
									      & ((vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[
										(0x1f 
										& (((IData)(0x15) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)) 
										>> 5))] 
										>> 
										(0x1f 
										& ((IData)(0x15) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)))) 
										<< 0x15)) 
									     | ((0x100000 
										& ((vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[
										(0x1f 
										& (((IData)(0x14) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)) 
										>> 5))] 
										>> 
										(0x1f 
										& ((IData)(0x14) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)))) 
										<< 0x14)) 
										| ((0x80000 
										& ((vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[
										(0x1f 
										& (((IData)(0x13) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)) 
										>> 5))] 
										>> 
										(0x1f 
										& ((IData)(0x13) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)))) 
										<< 0x13)) 
										| ((0x40000 
										& ((vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[
										(0x1f 
										& (((IData)(0x12) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)) 
										>> 5))] 
										>> 
										(0x1f 
										& ((IData)(0x12) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)))) 
										<< 0x12)) 
										| ((0x20000 
										& ((vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[
										(0x1f 
										& (((IData)(0x11) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)) 
										>> 5))] 
										>> 
										(0x1f 
										& ((IData)(0x11) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)))) 
										<< 0x11)) 
										| ((0x10000 
										& ((vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[
										(0x1f 
										& (((IData)(0x10) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)) 
										>> 5))] 
										>> 
										(0x1f 
										& ((IData)(0x10) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)))) 
										<< 0x10)) 
										| ((0x8000 
										& ((vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[
										(0x1f 
										& (((IData)(0xf) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)) 
										>> 5))] 
										>> 
										(0x1f 
										& ((IData)(0xf) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)))) 
										<< 0xf)) 
										| ((0x4000 
										& ((vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[
										(0x1f 
										& (((IData)(0xe) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)) 
										>> 5))] 
										>> 
										(0x1f 
										& ((IData)(0xe) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)))) 
										<< 0xe)) 
										| ((0x2000 
										& ((vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[
										(0x1f 
										& (((IData)(0xd) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)) 
										>> 5))] 
										>> 
										(0x1f 
										& ((IData)(0xd) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)))) 
										<< 0xd)) 
										| ((0x1000 
										& ((vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[
										(0x1f 
										& (((IData)(0xc) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)) 
										>> 5))] 
										>> 
										(0x1f 
										& ((IData)(0xc) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)))) 
										<< 0xc)) 
										| ((0x800 
										& ((vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[
										(0x1f 
										& (((IData)(0xb) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)) 
										>> 5))] 
										>> 
										(0x1f 
										& ((IData)(0xb) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)))) 
										<< 0xb)) 
										| ((0x400 
										& ((vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[
										(0x1f 
										& (((IData)(0xa) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)) 
										>> 5))] 
										>> 
										(0x1f 
										& ((IData)(0xa) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)))) 
										<< 0xa)) 
										| ((0x200 
										& ((vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[
										(0x1f 
										& (((IData)(9) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)) 
										>> 5))] 
										>> 
										(0x1f 
										& ((IData)(9) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)))) 
										<< 9)) 
										| ((0x100 
										& ((vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[
										(0x1f 
										& (((IData)(8) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)) 
										>> 5))] 
										>> 
										(0x1f 
										& ((IData)(8) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)))) 
										<< 8)) 
										| ((0x80 
										& ((vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[
										(0x1f 
										& (((IData)(7) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)) 
										>> 5))] 
										>> 
										(0x1f 
										& ((IData)(7) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)))) 
										<< 7)) 
										| ((0x40 
										& ((vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[
										(0x1f 
										& (((IData)(6) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)) 
										>> 5))] 
										>> 
										(0x1f 
										& ((IData)(6) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)))) 
										<< 6)) 
										| ((0x20 
										& ((vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[
										(0x1f 
										& (((IData)(5) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)) 
										>> 5))] 
										>> 
										(0x1f 
										& ((IData)(5) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)))) 
										<< 5)) 
										| ((0x10 
										& ((vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[
										(0x1f 
										& (((IData)(4) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)) 
										>> 5))] 
										>> 
										(0x1f 
										& ((IData)(4) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)))) 
										<< 4)) 
										| ((8 
										& ((vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[
										(0x1f 
										& (((IData)(3) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)) 
										>> 5))] 
										>> 
										(0x1f 
										& ((IData)(3) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)))) 
										<< 3)) 
										| ((4 
										& ((vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[
										(0x1f 
										& (((IData)(2) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)) 
										>> 5))] 
										>> 
										(0x1f 
										& ((IData)(2) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)))) 
										<< 2)) 
										| ((2 
										& ((vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[
										(0x1f 
										& (((IData)(1) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)) 
										>> 5))] 
										>> 
										(0x1f 
										& ((IData)(1) 
										+ 
										((IData)(__PVT__gpr_no) 
										<< 5)))) 
										<< 1)) 
										| (1 
										& (vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[
										(0x1f 
										& (IData)(__PVT__gpr_no))] 
										>> 
										(0x1f 
										& ((IData)(__PVT__gpr_no) 
										<< 5)))))))))))))))))))))))))))))))))));
    // Final
    return (__PVT__get_gpr);
}

void Vor1200_top_or1200_rfram_generic::_sequent__TOP__v__or1200_cpu__or1200_rf__rf_a__1(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"            Vor1200_top_or1200_rfram_generic::_sequent__TOP__v__or1200_cpu__or1200_rf__rf_a__1"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    // ALWAYS at ../or1k-verilog-rtl/or1200_rfram_generic.v:244
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b = 0;
    } else {
	if ((1 & (((vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn 
		    >> 0x1e) & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze))) 
		  | (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid)))) {
	    vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b 
		= (0x1f & (vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn 
			   >> 0xb));
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_rfram_generic.v:155
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0] = 0;
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[1] = 0;
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[2] = 0;
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[3] = 0;
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[4] = 0;
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[5] = 0;
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[6] = 0;
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[7] = 0;
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[8] = 0;
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[9] = 0;
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xa] = 0;
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xb] = 0;
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xc] = 0;
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xd] = 0;
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xe] = 0;
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xf] = 0;
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x10] = 0;
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x11] = 0;
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x12] = 0;
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x13] = 0;
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x14] = 0;
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x15] = 0;
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x16] = 0;
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x17] = 0;
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x18] = 0;
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x19] = 0;
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1a] = 0;
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1b] = 0;
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1c] = 0;
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1d] = 0;
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1e] = 0;
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1f] = 0;
    } else {
	if (vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_we) {
	    if ((0x10 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_addrw))) {
		if ((8 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_addrw))) {
		    if ((4 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_addrw))) {
			if ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_addrw))) {
			    if ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_addrw))) {
				vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1f] 
				    = (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
					& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))
				        ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu
				        : vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw);
			    } else {
				vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1e] 
				    = (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
					& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))
				        ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu
				        : vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw);
			    }
			} else {
			    if ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_addrw))) {
				vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1d] 
				    = (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
					& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))
				        ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu
				        : vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw);
			    } else {
				vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1c] 
				    = (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
					& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))
				        ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu
				        : vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw);
			    }
			}
		    } else {
			if ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_addrw))) {
			    if ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_addrw))) {
				vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1b] 
				    = (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
					& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))
				        ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu
				        : vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw);
			    } else {
				vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1a] 
				    = (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
					& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))
				        ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu
				        : vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw);
			    }
			} else {
			    if ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_addrw))) {
				vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x19] 
				    = (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
					& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))
				        ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu
				        : vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw);
			    } else {
				vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x18] 
				    = (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
					& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))
				        ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu
				        : vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw);
			    }
			}
		    }
		} else {
		    if ((4 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_addrw))) {
			if ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_addrw))) {
			    if ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_addrw))) {
				vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x17] 
				    = (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
					& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))
				        ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu
				        : vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw);
			    } else {
				vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x16] 
				    = (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
					& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))
				        ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu
				        : vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw);
			    }
			} else {
			    if ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_addrw))) {
				vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x15] 
				    = (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
					& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))
				        ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu
				        : vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw);
			    } else {
				vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x14] 
				    = (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
					& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))
				        ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu
				        : vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw);
			    }
			}
		    } else {
			if ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_addrw))) {
			    if ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_addrw))) {
				vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x13] 
				    = (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
					& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))
				        ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu
				        : vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw);
			    } else {
				vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x12] 
				    = (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
					& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))
				        ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu
				        : vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw);
			    }
			} else {
			    if ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_addrw))) {
				vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x11] 
				    = (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
					& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))
				        ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu
				        : vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw);
			    } else {
				vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x10] 
				    = (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
					& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))
				        ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu
				        : vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw);
			    }
			}
		    }
		}
	    } else {
		if ((8 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_addrw))) {
		    if ((4 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_addrw))) {
			if ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_addrw))) {
			    if ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_addrw))) {
				vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xf] 
				    = (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
					& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))
				        ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu
				        : vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw);
			    } else {
				vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xe] 
				    = (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
					& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))
				        ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu
				        : vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw);
			    }
			} else {
			    if ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_addrw))) {
				vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xd] 
				    = (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
					& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))
				        ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu
				        : vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw);
			    } else {
				vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xc] 
				    = (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
					& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))
				        ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu
				        : vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw);
			    }
			}
		    } else {
			if ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_addrw))) {
			    if ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_addrw))) {
				vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xb] 
				    = (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
					& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))
				        ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu
				        : vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw);
			    } else {
				vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xa] 
				    = (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
					& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))
				        ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu
				        : vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw);
			    }
			} else {
			    if ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_addrw))) {
				vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[9] 
				    = (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
					& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))
				        ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu
				        : vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw);
			    } else {
				vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[8] 
				    = (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
					& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))
				        ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu
				        : vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw);
			    }
			}
		    }
		} else {
		    if ((4 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_addrw))) {
			if ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_addrw))) {
			    if ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_addrw))) {
				vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[7] 
				    = (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
					& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))
				        ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu
				        : vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw);
			    } else {
				vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[6] 
				    = (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
					& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))
				        ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu
				        : vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw);
			    }
			} else {
			    if ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_addrw))) {
				vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[5] 
				    = (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
					& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))
				        ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu
				        : vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw);
			    } else {
				vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[4] 
				    = (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
					& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))
				        ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu
				        : vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw);
			    }
			}
		    } else {
			if ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_addrw))) {
			    if ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_addrw))) {
				vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[3] 
				    = (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
					& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))
				        ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu
				        : vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw);
			    } else {
				vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[2] 
				    = (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
					& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))
				        ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu
				        : vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw);
			    }
			} else {
			    if ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__rf_addrw))) {
				vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[1] 
				    = (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
					& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we))
				        ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_dat_cpu
				        : vlSymsp->TOP__v__or1200_cpu.__PVT__rf_dataw);
			    } else {
				vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0] = 0;
			    }
			}
		    }
		}
	    }
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_rfram_generic.v:198
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a = 0;
    } else {
	if ((1 & (((vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn 
		    >> 0x1f) & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze))) 
		  | (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid)))) {
	    vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a 
		= (0x1f & (((IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf.__PVT__spr_valid) 
			    & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__spr_we)))
			    ? vlSymsp->TOP__v__or1200_cpu.__PVT__spr_addr
			    : (vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn 
			       >> 0x10)));
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_rfram_generic.v:251
    vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__do_b 
	= ((0x10 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
	    ? ((8 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
	        ? ((4 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
		    ? ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
		        ? ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1f]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1e])
		        : ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1d]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1c]))
		    : ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
		        ? ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1b]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1a])
		        : ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x19]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x18])))
	        : ((4 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
		    ? ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
		        ? ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x17]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x16])
		        : ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x15]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x14]))
		    : ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
		        ? ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x13]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x12])
		        : ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x11]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x10]))))
	    : ((8 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
	        ? ((4 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
		    ? ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
		        ? ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xf]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xe])
		        : ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xd]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xc]))
		    : ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
		        ? ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xb]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xa])
		        : ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[9]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[8])))
	        : ((4 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
		    ? ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
		        ? ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[7]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[6])
		        : ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[5]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[4]))
		    : ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
		        ? ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[3]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[2])
		        : ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[1]
			    : 0)))));
    // ALWAYS at ../or1k-verilog-rtl/or1200_rfram_generic.v:205
    vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__do_a 
	= ((0x10 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
	    ? ((8 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
	        ? ((4 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
		    ? ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
		        ? ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1f]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1e])
		        : ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1d]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1c]))
		    : ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
		        ? ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1b]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1a])
		        : ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x19]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x18])))
	        : ((4 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
		    ? ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
		        ? ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x17]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x16])
		        : ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x15]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x14]))
		    : ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
		        ? ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x13]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x12])
		        : ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x11]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x10]))))
	    : ((8 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
	        ? ((4 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
		    ? ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
		        ? ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xf]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xe])
		        : ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xd]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xc]))
		    : ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
		        ? ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xb]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xa])
		        : ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[9]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[8])))
	        : ((4 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
		    ? ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
		        ? ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[7]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[6])
		        : ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[5]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[4]))
		    : ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
		        ? ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[3]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[2])
		        : ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[1]
			    : 0)))));
}

void Vor1200_top_or1200_rfram_generic::_settle__TOP__v__or1200_cpu__or1200_rf__rf_a__2(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"            Vor1200_top_or1200_rfram_generic::_settle__TOP__v__or1200_cpu__or1200_rf__rf_a__2"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    // ALWAYS at ../or1k-verilog-rtl/or1200_rfram_generic.v:251
    vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__do_b 
	= ((0x10 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
	    ? ((8 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
	        ? ((4 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
		    ? ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
		        ? ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1f]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1e])
		        : ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1d]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1c]))
		    : ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
		        ? ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1b]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1a])
		        : ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x19]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x18])))
	        : ((4 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
		    ? ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
		        ? ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x17]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x16])
		        : ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x15]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x14]))
		    : ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
		        ? ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x13]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x12])
		        : ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x11]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x10]))))
	    : ((8 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
	        ? ((4 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
		    ? ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
		        ? ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xf]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xe])
		        : ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xd]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xc]))
		    : ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
		        ? ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xb]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xa])
		        : ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[9]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[8])))
	        : ((4 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
		    ? ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
		        ? ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[7]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[6])
		        : ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[5]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[4]))
		    : ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
		        ? ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[3]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[2])
		        : ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_b))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[1]
			    : 0)))));
    // ALWAYS at ../or1k-verilog-rtl/or1200_rfram_generic.v:205
    vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__do_a 
	= ((0x10 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
	    ? ((8 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
	        ? ((4 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
		    ? ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
		        ? ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1f]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1e])
		        : ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1d]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1c]))
		    : ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
		        ? ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1b]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x1a])
		        : ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x19]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x18])))
	        : ((4 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
		    ? ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
		        ? ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x17]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x16])
		        : ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x15]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x14]))
		    : ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
		        ? ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x13]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x12])
		        : ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x11]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0x10]))))
	    : ((8 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
	        ? ((4 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
		    ? ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
		        ? ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xf]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xe])
		        : ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xd]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xc]))
		    : ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
		        ? ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xb]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[0xa])
		        : ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[9]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[8])))
	        : ((4 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
		    ? ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
		        ? ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[7]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[6])
		        : ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[5]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[4]))
		    : ((2 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
		        ? ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[3]
			    : vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[2])
		        : ((1 & (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__intaddr_a))
			    ? vlSymsp->TOP__v__or1200_cpu__or1200_rf__rf_a.__PVT__mem[1]
			    : 0)))));
}
