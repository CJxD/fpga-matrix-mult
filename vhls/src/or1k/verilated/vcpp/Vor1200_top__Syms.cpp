// Verilated -*- C++ -*-

#include "Vor1200_top__Syms.h"
#include "Vor1200_top.h"
#include "Vor1200_top_or1200_top.h"
#include "Vor1200_top_or1200_cpu.h"
#include "Vor1200_top_or1200_rf.h"
#include "Vor1200_top_or1200_ctrl.h"
#include "Vor1200_top_or1200_rfram_generic.h"

// FUNCTIONS
Vor1200_top__Syms::Vor1200_top__Syms(Vor1200_top* topp, const char* namep)
	// Setup locals
	: __Vm_namep(namep)
	, __Vm_activity(false)
	, __Vm_didInit(false)
	// Setup submodule names
	, TOP__v                         (Verilated::catName(topp->name(),".v"))
	, TOP__v__or1200_cpu             (Verilated::catName(topp->name(),".v.or1200_cpu"))
	, TOP__v__or1200_cpu__or1200_ctrl (Verilated::catName(topp->name(),".v.or1200_cpu.or1200_ctrl"))
	, TOP__v__or1200_cpu__or1200_rf  (Verilated::catName(topp->name(),".v.or1200_cpu.or1200_rf"))
	, TOP__v__or1200_cpu__or1200_rf__rf_a (Verilated::catName(topp->name(),".v.or1200_cpu.or1200_rf.rf_a"))
{
    // Pointer to top level
    TOPp = topp;
    // Setup each module's pointers to their submodules
    TOPp->v                         = &TOP__v;
    TOPp->v->or1200_cpu             = &TOP__v__or1200_cpu;
    TOPp->v->or1200_cpu->or1200_ctrl  = &TOP__v__or1200_cpu__or1200_ctrl;
    TOPp->v->or1200_cpu->or1200_rf  = &TOP__v__or1200_cpu__or1200_rf;
    TOPp->v->or1200_cpu->or1200_rf->rf_a  = &TOP__v__or1200_cpu__or1200_rf__rf_a;
    // Setup each module's pointer back to symbol table (for public functions)
    TOPp->__Vconfigure(this, true);
    TOP__v.__Vconfigure(this, true);
    TOP__v__or1200_cpu.__Vconfigure(this, true);
    TOP__v__or1200_cpu__or1200_ctrl.__Vconfigure(this, true);
    TOP__v__or1200_cpu__or1200_rf.__Vconfigure(this, true);
    TOP__v__or1200_cpu__or1200_rf__rf_a.__Vconfigure(this, true);
}

