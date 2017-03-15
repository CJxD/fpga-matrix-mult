// Verilated -*- C++ -*-
#include "Vor1200_top_or1200_ctrl.h" // For This
#include "Vor1200_top__Syms.h"

//--------------------
// STATIC VARIABLES


//--------------------

VL_CTOR_IMP(Vor1200_top_or1200_ctrl) {
    // Reset internal values
    // Reset structure values
    __PVT__clk = VL_RAND_RESET_I(1);
    __PVT__rst = VL_RAND_RESET_I(1);
    __PVT__id_freeze = VL_RAND_RESET_I(1);
    __PVT__ex_freeze = VL_RAND_RESET_I(1);
    wb_freeze = VL_RAND_RESET_I(1);
    __PVT__flushpipe = VL_RAND_RESET_I(1);
    __PVT__if_insn = VL_RAND_RESET_I(32);
    __PVT__ex_insn = VL_RAND_RESET_I(32);
    __PVT__branch_op = VL_RAND_RESET_I(3);
    __PVT__branch_taken = VL_RAND_RESET_I(1);
    __PVT__rf_addrw = VL_RAND_RESET_I(5);
    __PVT__rf_addra = VL_RAND_RESET_I(5);
    __PVT__rf_addrb = VL_RAND_RESET_I(5);
    __PVT__rf_rda = VL_RAND_RESET_I(1);
    __PVT__rf_rdb = VL_RAND_RESET_I(1);
    __PVT__alu_op = VL_RAND_RESET_I(4);
    __PVT__mac_op = VL_RAND_RESET_I(2);
    __PVT__shrot_op = VL_RAND_RESET_I(2);
    __PVT__rfwb_op = VL_RAND_RESET_I(3);
    __PVT__wb_insn = VL_RAND_RESET_I(32);
    __PVT__simm = VL_RAND_RESET_I(32);
    __PVT__branch_addrofs = VL_RAND_RESET_I(30);
    __PVT__lsu_addrofs = VL_RAND_RESET_I(32);
    __PVT__sel_a = VL_RAND_RESET_I(2);
    __PVT__sel_b = VL_RAND_RESET_I(2);
    __PVT__lsu_op = VL_RAND_RESET_I(4);
    __PVT__comp_op = VL_RAND_RESET_I(4);
    __PVT__multicycle = VL_RAND_RESET_I(2);
    __PVT__cust5_op = VL_RAND_RESET_I(5);
    __PVT__cust5_limm = VL_RAND_RESET_I(6);
    __PVT__spr_addrimm = VL_RAND_RESET_I(16);
    __PVT__wbforw_valid = VL_RAND_RESET_I(1);
    __PVT__du_hwbkpt = VL_RAND_RESET_I(1);
    __PVT__sig_syscall = VL_RAND_RESET_I(1);
    __PVT__sig_trap = VL_RAND_RESET_I(1);
    __PVT__force_dslot_fetch = VL_RAND_RESET_I(1);
    __PVT__no_more_dslot = VL_RAND_RESET_I(1);
    __PVT__ex_void = VL_RAND_RESET_I(1);
    __PVT__id_macrc_op = VL_RAND_RESET_I(1);
    __PVT__ex_macrc_op = VL_RAND_RESET_I(1);
    __PVT__rfe = VL_RAND_RESET_I(1);
    __PVT__except_illegal = VL_RAND_RESET_I(1);
    __PVT__pre_branch_op = VL_RAND_RESET_I(3);
    __PVT__id_insn = VL_RAND_RESET_I(32);
    __PVT__wb_rfaddrw = VL_RAND_RESET_I(5);
    __PVT__sel_imm = VL_RAND_RESET_I(1);
}

void Vor1200_top_or1200_ctrl::__Vconfigure(Vor1200_top__Syms* vlSymsp, bool first) {
    if (0 && first) {}  // Prevent unused
    this->__VlSymsp = vlSymsp;
}

Vor1200_top_or1200_ctrl::~Vor1200_top_or1200_ctrl() {
}

//--------------------
// Internal Methods

uint32_t Vor1200_top_or1200_ctrl::get_wb_insn() {
    VL_DEBUG_IF(cout<<"          Vor1200_top_or1200_ctrl::get_wb_insn"<<endl; );
    // Variables
    VL_OUT(__PVT__get_wb_insn,31,0);
    Vor1200_top__Syms* __restrict vlSymsp = this->__VlSymsp;
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    __PVT__get_wb_insn = vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__wb_insn;
    // Final
    return (__PVT__get_wb_insn);
}

void Vor1200_top_or1200_ctrl::_sequent__TOP__v__or1200_cpu__or1200_ctrl__1(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"          Vor1200_top_or1200_ctrl::_sequent__TOP__v__or1200_cpu__or1200_ctrl__1"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    // ALWAYS at ../or1k-verilog-rtl/or1200_ctrl.v:534
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__sel_imm = 0;
    } else {
	if ((1 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze)))) {
	    vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__sel_imm 
		= (1 & ((0x80000000 & vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn)
			 ? ((0x40000000 & vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn)
			     ? ((0x20000000 & vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn)
				 ? ((vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn 
				     >> 0x1c) | (vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn 
						 >> 0x1b))
				 : ((0x10000000 & vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn)
				     ? ((~ (vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn 
					    >> 0x1b)) 
					& (~ (vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn 
					      >> 0x1a)))
				     : (vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn 
					>> 0x1b))) : 
			    ((~ (vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn 
				 >> 0x1d)) | ((~ (vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn 
						  >> 0x1c)) 
					      | ((vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn 
						  >> 0x1b) 
						 | (~ 
						    (vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn 
						     >> 0x1a))))))
			 : ((0x40000000 & vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn)
			     ? ((vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn 
				 >> 0x1d) | ((vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn 
					      >> 0x1c) 
					     | ((0x8000000 
						 & vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn)
						 ? 
						(vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn 
						 >> 0x1a)
						 : 
						(~ 
						 (vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn 
						  >> 0x1a)))))
			     : ((0x20000000 & vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn)
				 ? ((vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn 
				     >> 0x1c) | (vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn 
						 >> 0x1b))
				 : ((~ (vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn 
					>> 0x1c)) | 
				    ((vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn 
				      >> 0x1b) | (~ 
						  (vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn 
						   >> 0x1a))))))));
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_ctrl.v:316
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_macrc_op = 0;
    } else {
	if ((((~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)) 
	      & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze)) 
	     | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__flushpipe))) {
	    vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_macrc_op = 0;
	} else {
	    if ((1 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)))) {
		vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_macrc_op 
		    = vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_macrc_op;
	    }
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_ctrl.v:774
    vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__mac_op 
	= (3 & ((IData)(vlTOPp->rst_i) ? 0 : ((((~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)) 
						& (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze)) 
					       | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__flushpipe))
					       ? 0 : 
					      ((IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)
					        ? 0
					        : (
						   (0x13 
						    == 
						    (0x3f 
						     & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
							>> 0x1a)))
						    ? 1
						    : 
						   ((0x31 
						     == 
						     (0x3f 
						      & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
							 >> 0x1a)))
						     ? vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn
						     : 0))))));
    // ALWAYS at ../or1k-verilog-rtl/or1200_ctrl.v:1036
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__sig_syscall = 0;
    } else {
	if ((((~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)) 
	      & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze)) 
	     | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__flushpipe))) {
	    vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__sig_syscall = 0;
	} else {
	    if ((1 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)))) {
		vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__sig_syscall 
		    = (0x40 == (0x1ff & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
					 >> 0x17)));
	    }
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_ctrl.v:1055
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__sig_trap = 0;
    } else {
	if ((((~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)) 
	      & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze)) 
	     | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__flushpipe))) {
	    vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__sig_trap = 0;
	} else {
	    if ((1 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)))) {
		vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__sig_trap 
		    = (0x42 == (0x1ff & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
					 >> 0x17)));
	    }
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_ctrl.v:806
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__shrot_op = 0;
    } else {
	if ((((~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)) 
	      & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze)) 
	     | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__flushpipe))) {
	    vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__shrot_op = 0;
	} else {
	    if ((1 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)))) {
		vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__shrot_op 
		    = (3 & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
			    >> 6));
	    }
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_ctrl.v:819
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rfwb_op = 0;
    } else {
	if ((((~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)) 
	      & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze)) 
	     | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__flushpipe))) {
	    vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rfwb_op = 0;
	} else {
	    if ((1 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)))) {
		vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rfwb_op 
		    = ((0x80000000 & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
		        ? ((0x40000000 & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
			    ? ((0x20000000 & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
			        ? ((0x10000000 & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
				    ? 0 : ((0x8000000 
					    & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
					    ? 0 : (
						   (0x4000000 
						    & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
						    ? 0
						    : 1)))
			        : 0) : ((0x20000000 
					 & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
					 ? ((0x10000000 
					     & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
					     ? ((0x8000000 
						 & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
						 ? 
						((0x4000000 
						  & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
						  ? 0
						  : 1)
						 : 
						((0x4000000 
						  & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
						  ? 5
						  : 1))
					     : 1) : 
					((0x10000000 
					  & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
					  ? ((0x8000000 
					      & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
					      ? ((0x4000000 
						  & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
						  ? 1
						  : 3)
					      : 3) : 
					 ((0x8000000 
					   & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
					   ? ((0x4000000 
					       & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
					       ? 3 : 0)
					   : ((0x4000000 
					       & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
					       ? 3 : 0)))))
		        : ((0x40000000 & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
			    ? ((0x20000000 & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
			        ? 0 : ((0x10000000 
					& vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
				        ? 0 : ((0x8000000 
						& vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
					        ? (
						   (0x4000000 
						    & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
						    ? 0
						    : 7)
					        : 0)))
			    : ((0x20000000 & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
			        ? 0 : ((0x10000000 
					& vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
				        ? ((0x8000000 
					    & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
					    ? ((0x4000000 
						& vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
					        ? 0
					        : 1)
					    : 0) : 
				       ((0x8000000 
					 & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
					 ? 0 : ((0x4000000 
						 & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
						 ? 7
						 : 0))))));
	    }
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_ctrl.v:475
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__wb_rfaddrw = 0;
    } else {
	if ((1 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.wb_freeze)))) {
	    vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__wb_rfaddrw 
		= vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rf_addrw;
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_ctrl.v:613
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__except_illegal = 0;
    } else {
	if ((((~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)) 
	      & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze)) 
	     | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__flushpipe))) {
	    vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__except_illegal = 0;
	} else {
	    if ((1 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)))) {
		vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__except_illegal 
		    = (1 & ((0x80000000 & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
			     ? ((0x40000000 & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
				 ? ((0x20000000 & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
				     ? ((vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
					 >> 0x1c) | 
					(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
					 >> 0x1b)) : 
				    ((0x10000000 & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
				      ? ((~ (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
					     >> 0x1b)) 
					 & (~ (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
					       >> 0x1a)))
				      : (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
					 >> 0x1b)))
				 : ((~ (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
					>> 0x1d)) & 
				    ((~ (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
					 >> 0x1c)) 
				     & (~ (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
					   >> 0x1a)))))
			     : ((0x40000000 & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
				 ? ((vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
				     >> 0x1d) | ((vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
						  >> 0x1c) 
						 | ((~ 
						     (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
						      >> 0x1b)) 
						    & (~ 
						       (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
							>> 0x1a)))))
				 : ((0x20000000 & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
				     ? ((vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
					 >> 0x1c) | 
					(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
					 >> 0x1b)) : 
				    ((0x10000000 & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
				      ? ((vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
					  >> 0x1b) 
					 & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
					    >> 0x1a))
				      : ((vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
					  >> 0x1b) 
					 & (~ (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
					       >> 0x1a))))))));
	    }
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_ctrl.v:1024
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__comp_op = 0;
    } else {
	if ((((~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)) 
	      & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze)) 
	     | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__flushpipe))) {
	    vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__comp_op = 0;
	} else {
	    if ((1 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)))) {
		vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__comp_op 
		    = (0xf & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
			      >> 0x15));
	    }
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_ctrl.v:962
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__branch_op = 0;
    } else {
	if ((((~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)) 
	      & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze)) 
	     | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__flushpipe))) {
	    vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__branch_op = 0;
	} else {
	    if ((1 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)))) {
		vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__branch_op 
		    = vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__pre_branch_op;
	    }
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_ctrl.v:521
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__wb_insn = 0x14410000;
    } else {
	if (vlSymsp->TOP__v__or1200_cpu.__PVT__flushpipe) {
	    vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__wb_insn = 0x14410000;
	} else {
	    if ((1 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.wb_freeze)))) {
		vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__wb_insn 
		    = vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn;
	    }
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_ctrl.v:331
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__spr_addrimm = 0;
    } else {
	if ((((~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)) 
	      & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze)) 
	     | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__flushpipe))) {
	    vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__spr_addrimm = 0;
	} else {
	    if ((1 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)))) {
		vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__spr_addrimm 
		    = (0xffff & ((0x2d == (0x3f & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
						   >> 0x1a)))
				  ? vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn
				  : ((0xf800 & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
						>> 0xa)) 
				     | (0x7ff & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn))));
	    }
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_ctrl.v:676
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op = 4;
    } else {
	if ((((~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)) 
	      & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze)) 
	     | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__flushpipe))) {
	    vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op = 4;
	} else {
	    if ((1 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)))) {
		vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__alu_op 
		    = (0xf & ((0x80000000 & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
			       ? ((0x40000000 & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
				   ? ((0x20000000 & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
				       ? ((0x10000000 
					   & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
					   ? 4 : ((0x8000000 
						   & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
						   ? 4
						   : 
						  ((0x4000000 
						    & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
						    ? 0xd
						    : vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)))
				       : ((0x10000000 
					   & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
					   ? 4 : ((0x8000000 
						   & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
						   ? 4
						   : 
						  ((0x4000000 
						    & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
						    ? 4
						    : 0xe))))
				   : ((0x20000000 & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
				       ? ((0x10000000 
					   & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
					   ? ((0x8000000 
					       & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
					       ? ((0x4000000 
						   & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
						   ? 0xd
						   : 8)
					       : ((0x4000000 
						   & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
						   ? 0xf
						   : 6))
					   : ((0x8000000 
					       & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
					       ? ((0x4000000 
						   & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
						   ? 5
						   : 4)
					       : ((0x4000000 
						   & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
						   ? 3
						   : 1)))
				       : ((0x10000000 
					   & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
					   ? ((0x8000000 
					       & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
					       ? ((0x4000000 
						   & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
						   ? 0
						   : 4)
					       : 4)
					   : 4))) : 
			      ((0x40000000 & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
			        ? 4 : ((0x20000000 
					& vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
				        ? 4 : ((0x10000000 
						& vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
					        ? (
						   (0x8000000 
						    & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
						    ? 
						   ((0x4000000 
						     & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
						     ? 4
						     : 0xc)
						    : 4)
					        : (
						   (0x8000000 
						    & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
						    ? 4
						    : 0xb))))));
	    }
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_ctrl.v:458
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rf_addrw = 0;
    } else {
	if (((~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)) 
	     & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze))) {
	    vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rf_addrw = 0;
	} else {
	    if ((1 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)))) {
		vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__rf_addrw 
		    = (0x1f & (((2 == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__pre_branch_op)) 
				| (3 == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__pre_branch_op)))
			        ? 9 : (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
				       >> 0x15)));
	    }
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_ctrl.v:503
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn = 0x14410000;
    } else {
	if ((((~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)) 
	      & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze)) 
	     | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__flushpipe))) {
	    vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn = 0x14410000;
	} else {
	    if ((1 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)))) {
		vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn 
		    = vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn;
	    }
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_ctrl.v:915
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__pre_branch_op = 0;
    } else {
	if (vlSymsp->TOP__v__or1200_cpu.__PVT__flushpipe) {
	    vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__pre_branch_op = 0;
	} else {
	    if ((1 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze)))) {
		vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__pre_branch_op 
		    = ((0x80000000 & vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn)
		        ? 0 : ((0x40000000 & vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn)
			        ? ((0x20000000 & vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn)
				    ? 0 : ((0x10000000 
					    & vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn)
					    ? 0 : (
						   (0x8000000 
						    & vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn)
						    ? 
						   ((0x4000000 
						     & vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn)
						     ? 0
						     : 2)
						    : 
						   ((0x4000000 
						     & vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn)
						     ? 2
						     : 0))))
			        : ((0x20000000 & vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn)
				    ? ((0x10000000 
					& vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn)
				        ? 0 : ((0x8000000 
						& vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn)
					        ? 0
					        : (
						   (0x4000000 
						    & vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn)
						    ? 6
						    : 0)))
				    : ((0x10000000 
					& vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn)
				        ? ((0x8000000 
					    & vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn)
					    ? 0 : (
						   (0x4000000 
						    & vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn)
						    ? 0
						    : 4))
				        : ((0x8000000 
					    & vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn)
					    ? ((0x4000000 
						& vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn)
					        ? 5
					        : 0)
					    : 3)))));
	    }
	}
    }
}

void Vor1200_top_or1200_ctrl::_sequent__TOP__v__or1200_cpu__or1200_ctrl__2(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"          Vor1200_top_or1200_ctrl::_sequent__TOP__v__or1200_cpu__or1200_ctrl__2"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    // ALWAYS at ../or1k-verilog-rtl/or1200_ctrl.v:973
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op = 0;
    } else {
	if ((((~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)) 
	      & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze)) 
	     | (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__flushpipe))) {
	    vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op = 0;
	} else {
	    if ((1 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__wb_freeze)))) {
		vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op 
		    = ((0x80000000 & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
		        ? ((0x40000000 & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
			    ? ((0x20000000 & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
			        ? 0 : ((0x10000000 
					& vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
				        ? ((0x8000000 
					    & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
					    ? ((0x4000000 
						& vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
					        ? 0xc
					        : 0xa)
					    : ((0x4000000 
						& vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
					        ? 0xe
					        : 0))
				        : 0)) : ((0x20000000 
						  & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
						  ? 0
						  : 
						 ((0x10000000 
						   & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
						   ? 
						  ((0x8000000 
						    & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
						    ? 
						   ((0x4000000 
						     & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
						     ? 0
						     : 5)
						    : 
						   ((0x4000000 
						     & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
						     ? 4
						     : 3))
						   : 
						  ((0x8000000 
						    & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
						    ? 
						   ((0x4000000 
						     & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
						     ? 2
						     : 0)
						    : 
						   ((0x4000000 
						     & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn)
						     ? 6
						     : 0)))))
		        : 0);
	    }
	}
    }
    // ALWAYS at ../or1k-verilog-rtl/or1200_ctrl.v:445
    vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_addrofs 
	= ((0xfffff800 & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_addrofs) 
	   | (0x7ff & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn));
    vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_addrofs 
	= ((((0xe == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op)) 
	     | (0xc == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op))) 
	    | (0xa == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op)))
	    ? ((0x7ff & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_addrofs) 
	       | ((0xffff0000 & (VL_UNARYMIN_I((IData)(
						       (1 
							& (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn 
							   >> 0x19)))) 
				 << 0x10)) | (0xf800 
					      & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn 
						 >> 0xa))))
	    : ((0x7ff & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_addrofs) 
	       | ((0xffff0000 & (VL_UNARYMIN_I((IData)(
						       (1 
							& (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn 
							   >> 0xf)))) 
				 << 0x10)) | (0xf800 
					      & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn))));
    // ALWAYS at ../or1k-verilog-rtl/or1200_ctrl.v:485
    if (vlTOPp->rst_i) {
	vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn = 0x14410000;
    } else {
	if (vlSymsp->TOP__v__or1200_cpu.__PVT__flushpipe) {
	    vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn = 0x14410000;
	} else {
	    if ((1 & (~ (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__id_freeze)))) {
		vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
		    = vlSymsp->TOP__v__or1200_cpu.__PVT__if_insn;
	    }
	}
    }
    vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_macrc_op 
	= ((6 == (0x3f & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
			  >> 0x1a))) & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
					>> 0x10));
}

void Vor1200_top_or1200_ctrl::_settle__TOP__v__or1200_cpu__or1200_ctrl__3(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"          Vor1200_top_or1200_ctrl::_settle__TOP__v__or1200_cpu__or1200_ctrl__3"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    // ALWAYS at ../or1k-verilog-rtl/or1200_ctrl.v:445
    vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_addrofs 
	= ((0xfffff800 & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_addrofs) 
	   | (0x7ff & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn));
    vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_addrofs 
	= ((((0xe == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op)) 
	     | (0xc == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op))) 
	    | (0xa == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_op)))
	    ? ((0x7ff & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_addrofs) 
	       | ((0xffff0000 & (VL_UNARYMIN_I((IData)(
						       (1 
							& (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn 
							   >> 0x19)))) 
				 << 0x10)) | (0xf800 
					      & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn 
						 >> 0xa))))
	    : ((0x7ff & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__lsu_addrofs) 
	       | ((0xffff0000 & (VL_UNARYMIN_I((IData)(
						       (1 
							& (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn 
							   >> 0xf)))) 
				 << 0x10)) | (0xf800 
					      & vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__ex_insn))));
    vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_macrc_op 
	= ((6 == (0x3f & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
			  >> 0x1a))) & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
					>> 0x10));
}

void Vor1200_top_or1200_ctrl::_combo__TOP__v__or1200_cpu__or1200_ctrl__4(Vor1200_top__Syms* __restrict vlSymsp) {
    VL_DEBUG_IF(cout<<"          Vor1200_top_or1200_ctrl::_combo__TOP__v__or1200_cpu__or1200_ctrl__4"<<endl; );
    Vor1200_top* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    // Body
    vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__no_more_dslot 
	= ((((0 != (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__branch_op)) 
	     & (~ ((5 == (0x3f & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
				  >> 0x1a))) & (vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__id_insn 
						>> 0x10)))) 
	    & (IData)(vlSymsp->TOP__v__or1200_cpu.__PVT__branch_taken)) 
	   | (6 == (IData)(vlSymsp->TOP__v__or1200_cpu__or1200_ctrl.__PVT__branch_op)));
}
