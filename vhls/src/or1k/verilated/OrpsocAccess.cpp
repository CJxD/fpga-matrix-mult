// ----------------------------------------------------------------------------

// Access functions for the ORPSoC Verilator model: implementation

// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// Copyright (C) 2008  Embecosm Limited <info@embecosm.com>

// Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

// This file is part of the cycle accurate model of the OpenRISC 1000 based
// system-on-chip, ORPSoC, built using Verilator.

// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.

// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.

// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// ----------------------------------------------------------------------------

// $Id: OrpsocAccess.cpp 303 2009-02-16 11:20:17Z jeremy $

#include "OrpsocAccess.h"

#include "Vor1200_top.h"
#include "Vor1200_top_or1200_cpu.h"
#include "Vor1200_top_or1200_ctrl.h"
#include "Vor1200_top_or1200_rf.h"
#include "Vor1200_top_or1200_rfram_generic.h"
#include "Vor1200_top_or1200_top.h"


//! Constructor for the ORPSoC access class

//! Initializes the pointers to the various module instances of interest
//! within the Verilator model.

//! @param[in] orpsoc  The SystemC Verilated ORPSoC instance

OrpsocAccess::OrpsocAccess (Vor1200_top *or1200_top)
{
  or1200_ctrl = or1200_top->v->or1200_cpu->or1200_ctrl;
  rf_a        = or1200_top->v->or1200_cpu->or1200_rf->rf_a;
  cpu_pc      = &or1200_top->v->or1200_cpu->__PVT__or1200_genpc__DOT__pcreg;
  //    VL_SIG(__PVT__or1200_genpc__DOT__pcreg,31,2);
  //  VL_SIG(__PVT__or1200_genpc__DOT__pc,31,0);

}	// OrpsocAccess ()

uint32_t OrpsocAccess::getPc ()
{
  return *cpu_pc << 2;
    //or1200_top->TOP__v__or1200_cpu.__PVT__id_pc;
  //cpu->__PVT__id_pc;
}	// getPc ()



//! Access for the wb_freeze signal

//! @return  The value of the or1200_ctrl.wb_freeze signal

bool OrpsocAccess::getWbFreeze ()
{
  return  or1200_ctrl->wb_freeze;

}	// getWbFreeze ()


//! Access for the wb_insn register

//! @return  The value of the or1200_ctrl.wb_insn register

uint32_t OrpsocAccess::getWbInsn ()
{
  return  (or1200_ctrl->get_wb_insn) ();

}	// getWbInsn ()


//! Access for the OR1200 GPRs

//! These are extracted from memory using the Verilog function

//! @param[in] regNum  The GPR whose value is wanted

//! @return            The value of the GPR

uint32_t OrpsocAccess::getGpr (uint32_t  regNum)
{
  return  (rf_a->get_gpr) (regNum);

}	// getGpr ()



// eof
