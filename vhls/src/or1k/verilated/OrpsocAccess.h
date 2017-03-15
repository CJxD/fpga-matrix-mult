// ----------------------------------------------------------------------------

// Access functions for the ORPSoC Verilator model: definition

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

// $Id: OrpsocAccess.h 303 2009-02-16 11:20:17Z jeremy $


#ifndef ORPSOC_ACCESS__H
#define ORPSOC_ACCESS__H

#include <stdint.h>

class Vor1200_top;
class Vor1200_top_or1200_top;
class Vor1200_top_or1200_ctrl;
class Vor1200_top_or1200_rfram_generic;


//! Access functions to the Verilator model

//! This class encapsulates access to the Verilator model, allowing other
//! Classes to access model state, without needing to be built within the
//! Verilator environment.
class OrpsocAccess
{
public:

  // Constructor
  OrpsocAccess (Vor1200_top *or1200_top);

  // Accessor functions
  bool      getWbFreeze ();
  uint32_t  getWbInsn ();
  uint32_t  getGpr (uint32_t regNum);
  uint32_t  getPc ();

private:

  // Pointers to modules with accessor functions
  Vor1200_top_or1200_ctrl          *or1200_ctrl;
  Vor1200_top_or1200_rfram_generic *rf_a;
  unsigned int /*IData*/ *cpu_pc;

};	// OrpsocAccess ()

#endif	// ORPSOC_ACCESS__H
