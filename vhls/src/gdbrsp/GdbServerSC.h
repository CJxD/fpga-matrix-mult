//
// 2013: Modified by DJ Greaves as part of Project spEEDO 
// to be the GDB RSP access port to a multicore CMP SoC.


// ----------------------------------------------------------------------------

// SystemC GDB RSP server: definition

// Copyright (C) 2008  Embecosm Limited <info@embecosm.com>

// Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

// This file is part of the GDB interface to the cycle accurate model of the
// OpenRISC 1000 based system-on-chip, ORPSoC, built using Verilator.

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

// $Id$

#ifndef GDB_SERVER_SC__H
#define GDB_SERVER_SC__H

#include <stdint.h>

#include "systemc"

// #include "JtagSC_includes.h"

#include "SpeedoDebug.h"
#include "RspConnection.h"
#include "RspPacket.h"

//! Module implementing a GDB RSP server.

//! A thread listens for RSP requests, which are converted to requests to read
//! and write registers, memory or control the CPU in the debug unit

class GdbServerSC:public sc_core::sc_module {
public:

	// Constructor and destructor
	GdbServerSC(sc_core::sc_module_name name,
		    uint32_t _flashStart,
		    uint32_t _flashEnd,
		    int rspPort
		    //sc_core::sc_fifo < TapAction * >*tapActionQueue
		    );
	~GdbServerSC();
	void set_debugUnit(vhls_debug_port *debugUnit)
	{
	  this->debugUnit = debugUnit;
	}

private:




	//! Maximum size of a GDB RSP packet
	//static const int  RSP_PKT_MAX  = NUM_REGS * 8 + 1;
	static const int RSP_PKT_MAX = 1024 * 16;

	//! The packet pointer. There is only ever one packet in use at one time, so
	//! there is no need to repeatedly allocate and delete it.
	RspPacket *pkt;



	//! Our associated Debug Unit
	vhls_debug_port *debugUnit;


	//! Our associated RSP interface (which we create)
	RspConnection *rsp;

	// SystemC thread to listen for and service RSP requests
	void rspServer();

	//! RSP Signal value - reason for a fault or stall.
	int rsp_sigval;

	// Check if we got a message from the or1200 monitor module telling us
	// to stall
	bool checkMonitorPipe();


	// ALL OR1K-specific code needs removing from here...

	//! Is the target stopped
	bool targetStopped;


	// Main RSP request handler
	void rspClientRequest();

	// Handle the various RSP requests
	void rspCheckForException();
	void rspReportException();
	void rspContinue();
	void rspContinue(uint32_t except);
	void rspContinue(u64_t addr, uint32_t except);
	void rspInterrupt();
	void rspReadAllRegs();
	void rspWriteAllRegs();
	void rspReadMem();
	void rspWriteMem();
	void rspReadReg();
	//void rspSetThread();
	void rspWriteReg();
	void rspQuery();
	void rspCommand();
	void rspSet();
	void rspRestart();
	void rspStep();
	void rspStep(uint32_t except);
	void rspStep(uint32_t addr, uint32_t except);
	void rspVpkt();
	void rspWriteMemBin();
	void rspRemoveMatchpoint();
	void rspInsertMatchpoint();


	uint32_t readGpr(int regNum);
	int writeGpr(int regNum, uint32_t value);


	uint32_t readNpc();
	int writeNpc(u32_t a);


  //! Thread ID used by Or1ksim - needs be generic.
  static const int OR1KSIM_TID = 1;


};				// GdbServerSC ()

#endif // GDB_SERVER_SC__H
