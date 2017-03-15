// Verilated -*- C++ -*-
#ifndef _Vor1200_top__Syms_H_
#define _Vor1200_top__Syms_H_


// INCLUDE MODULE CLASSES
#include "Vor1200_top.h"
#include "Vor1200_top_or1200_top.h"
#include "Vor1200_top_or1200_cpu.h"
#include "Vor1200_top_or1200_rf.h"
#include "Vor1200_top_or1200_ctrl.h"
#include "Vor1200_top_or1200_rfram_generic.h"

static inline IData  VL_UNARYMIN_I(IData data) { return -data; }
static inline QData  VL_UNARYMIN_Q(QData data) { return -data; }

// SYMS CLASS
class Vor1200_top__Syms {
  public:
    
    // LOCAL STATE
    const char* __Vm_namep;
    bool	__Vm_activity;		///< Used by trace routines to determine change occurred
    bool	__Vm_didInit;
    char	__VpadToAlign10[6];
    
    // SUBCELL STATE
    Vor1200_top*                   TOPp;
    Vor1200_top_or1200_top         TOP__v;
    Vor1200_top_or1200_cpu         TOP__v__or1200_cpu;
    Vor1200_top_or1200_ctrl        TOP__v__or1200_cpu__or1200_ctrl;
    Vor1200_top_or1200_rf          TOP__v__or1200_cpu__or1200_rf;
    Vor1200_top_or1200_rfram_generic TOP__v__or1200_cpu__or1200_rf__rf_a;
    
    // COVERAGE
    
    // CREATORS
    Vor1200_top__Syms(Vor1200_top* topp, const char* namep);
    ~Vor1200_top__Syms() {};
    
    // METHODS
    inline const char* name() { return __Vm_namep; }
    inline bool getClearActivity() { bool r=__Vm_activity; __Vm_activity=false; return r;}
    
} VL_ATTR_ALIGNED(64);
#endif  /*guard*/
