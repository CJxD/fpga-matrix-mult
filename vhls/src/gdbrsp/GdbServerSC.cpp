


// 2014: Modified by DJ Greaves for generic use - ARM or OR1K.
// 2013: Modified by DJ Greaves as part of Project spEEDO 
// to be the GDB RSP access port to a multicore CMP SoC for spEEDO-Debug.

// Also (about to be) changed to 64 bit from 32 bit.


// ----------------------------------------------------------------------------

// SystemC GDB RSP server: implementation

// Copyright (C) 2008  Embecosm Limited <info@embecosm.com>

// Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>
// Contributor Julius Baxter <julius@orsoc.se>

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


#include <iostream>
#include <iomanip>
#include <assert.h>

#include "GdbServerSC.h"
#include "MpHash.h"
#include "Utils.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
extern int monitor_to_gdb_pipe[2][2];	// [0][] - monitor to gdb, [1][] - gdb to monitor, [][0] - read, [][1] - write

using std::cerr;
using std::dec;
using std::endl;
using std::hex;

using sc_core::sc_fifo;
using sc_core::sc_module_name;
using sc_core::sc_stop;
using sc_core::sc_time;


SC_HAS_PROCESS(GdbServerSC);

//-----------------------------------------------------------------------------
//! Constructor for the GDB RSP server.

//! We create a SC_THREAD which will act as the listener. Must be a
//! thread, since we need to wait for the actions to complete.

//! The current scan chain is marked as undefined.

//! This makes use of the Embecosm cycle accurate SystemC JTAG interface.

//! @see Embecosm Application Note 5 "Using JTAG with SystemC: Implementation
//!      of a Cycle Accurate Interface"
//!      (http://www.embecosm.com/download/ean5.html)

//! @todo We do not handle a user coded l.trap properly (i.e. one that is not
//!       a breakpoint). Effectively it is ignored, whereas we ought to set up
//!       the exception registers and redirect through the trap vector.

//! @param[in] name             Name of this module, passed to the parent
//!                             constructor. 
//! @param[in] _tapActionQueue  Pointer to fifo of actions to be performed by
//!                             the JTAG interface
//-----------------------------------------------------------------------------
GdbServerSC::GdbServerSC(sc_module_name name,
			 uint32_t _flashStart,
			 uint32_t _flashEnd,
			 int rspPort
			 //sc_fifo <TapAction * >*tapActionQueue
			 ):sc_module(name)

{
	pkt = new RspPacket(RSP_PKT_MAX);
	rsp = new RspConnection(rspPort);


#if 0
	/* Setup the pipes between or1200 monitor module and GDB stub */
	pipe(monitor_to_gdb_pipe[0]);
	pipe(monitor_to_gdb_pipe[1]);

	// Set non-blocking reads - so can poll rather than needing a pthread.
#ifdef O_NONBLOCK		/* The POSIX way */
	fcntl(monitor_to_gdb_pipe[0][0], F_SETFL, O_NONBLOCK);
	fcntl(monitor_to_gdb_pipe[1][0], F_SETFL, O_NONBLOCK);
#elif defined (O_NDELAY)
	fcntl(monitor_to_gdb_pipe[0][0], F_SETFL, O_NDELAY);
	fcntl(monitor_to_gdb_pipe[1][0], F_SETFL, O_NDELAY);
#endif /* O_NONBLOCK */
#endif


	//printf("Creating systemc thread for the server\n");
	SC_THREAD(rspServer); // run on an SC thread ... will need to check it does not block in read systemcall...

	//pthread_create(&rspServer);
}				// GdbServerSC ()

//-----------------------------------------------------------------------------
//! Destructor

//! Free up data structures
//-----------------------------------------------------------------------------
GdbServerSC::~GdbServerSC()
{
  delete debugUnit;
  delete rsp;
  delete pkt;

}				// ~GdbServerSC

//-----------------------------------------------------------------------------
//! SystemC thread to listen for RSP requests

//! JTAG actions will be queued as appropriate. Runs forever

//! We don't allow any actions until the target is out of its startup mode
//! (where the ROM is mapped into RAM space). This is determined by seeing if
//! the PPC is still in flash memory.

//! Have to use a thread, since we will end up waiting for actions to
//! complete.
//-----------------------------------------------------------------------------
void GdbServerSC::rspServer()
{
	// Reset the debug unit, and wait for ORPSoC to be ready, by noting when it
	// accesses an address outside of flash. Note that we use NPC, not PPC since
	// at reset PPC is zero, and would trigger a false positive.

  if (debugUnit)
    {
	debugUnit->resetDebugUnit();
	rsp_sigval = TARGET_SIGNAL_NONE;
	/*
	   uint32_t npc;
	   do
	   {
	   npc = debugUnit->readSpr (SPR_NPC);      
	   }
	   while ((flashStart <= npc) && (npc <= flashEnd));
	 */
	debugUnit->control_cmd(PauseC);
	targetStopped = true;

	// Make sure we are connected.
	while (!rsp->isConnected()) {
		// Reconnect and stall the processor on a new connection
		if (!rsp->rspConnect()) {
			// Serious failure. Must abort execution.
			cerr << "*** Unable to continue: ABORTING" << endl;
			sc_stop();
		}
		// Stall the processor until we get a command to handle.
		if (!debugUnit->isStalled(&rsp_sigval)) {
		  debugUnit->control_cmd(PauseC);
		}

		targetStopped = true;	// Processor now not running
	}
  
    }

  // Loop processing commands forever
  while (true) 
    {
      
#if 0
      if (!rsp->isConnected()) 
	{
	  sc_stop();
	  return;
	}
#endif
      sc_time dally(1, SC_US);
      wait(dally);
      // Wait until the target has stopped. In this simplified implementation,
      // the only reasons for stopping are hitting a breakpoint (l.trap),
      // hardware single stepping or hitting a non-breakpoint l.trap. This
      // last is not cleanly handled at the moment (we ought to redirect the
      // restart through the trap exception vector).
      while (!targetStopped) 
	{
	  /* First check to see if the or1200 monitor module wants us to stall */
	  //if (checkMonitorPipe()) break;
	  rspCheckForException();
	  
	  if (debugUnit->isStalled(&rsp_sigval)) 
	    {
	      targetStopped = true;

#if 0
 This is arch specific and needs to go in the relevant remotecontrol.cpp implementation.
 TODO move me.
	      // If it's a breakpoint, then we need to back up one
	      // instruction, so on restart we execute the actual
	      // instruction.
	      uint32_t ppc = debugUnit->readSpr(SPR_PPC);
	      if (NULL != mpHash->lookup(BP_MEMORY, ppc) && rsp_sigval == TARGET_SIGNAL_TRAP) 
		{
		  writeNpc(ppc);
		}
#endif

	      // Tell the client we've stopped.
	      rspReportException();
	    } 
	  else if (rsp->rspSocketPeek() > 0) 
	    {
	      if (rsp->rspSocketPeek() == 0x03)	// ETX, end of text control char
		{
		  // Got an interrupt command from GDB, this function should
		  // pull the packet off the socket and stall the processor.
		  // and then send a stop reply packet with signal 
		  // TARGET_SIGNAL_NONE.
		  rspInterrupt();
		  targetStopped = true;	// Processor now not running
		}
	    }
	}
      
      // Get a RSP client request
      rspClientRequest();
    }
} // rspServer ()

//-----------------------------------------------------------------------------
//! Deal with a request from the GDB client session

//! In general, apart from the simplest requests, this function relies on
//! other functions to implement the functionality.

//! @note It is the responsibility of the recipient to delete the packet when
//!       it is finished with. It is permissible to reuse the packet for a
//!       reply.

//! @todo Is this the implementation of the 'D' packet really the intended
//!       meaning? Or does it just mean that only vAttach will be recognized
//!       after this?

//! @param[in] pkt  The received RSP packet
//-----------------------------------------------------------------------------
void GdbServerSC::rspClientRequest()
{
	if (!rsp->getPkt(pkt)) {
		rsp->rspClose();	// Comms failure
		return;
	}
	//Uncomment the next line for the RSP client to print out every packet it gets from GDB
	//cerr << "rspClientRequest: " << pkt->data/*[0]*/ << endl;
	switch (pkt->data[0]) {
	case '!':
		// Request for extended remote mode
		pkt->packStr("OK");
		rsp->putPkt(pkt);
		return;

	case '?':
		// Return last signal ID
		rspReportException();
		return;

	case 'A':
		// Initialization of argv not supported
		cerr << "Warning: RSP 'A' packet not supported: ignored" <<
		    endl;
		pkt->packStr("E01");
		rsp->putPkt(pkt);
		return;

	case 'b':
		// Setting baud rate is deprecated
		cerr << "Warning: RSP 'b' packet is deprecated and not "
		    << "supported: ignored" << endl;
		return;

	case 'B':
		// Breakpoints should be set using Z packets
		cerr << "Warning: RSP 'B' packet is deprecated (use 'Z'/'z' "
		    << "packets instead): ignored" << endl;
		return;

	case 'c':
	  // Continue
	  rspContinue(0 /*EXCEPT_NONE - reset vector - arch specific? */);
	  return;

	case 'C':
	  // Continue with signal (in the packet)
	  rspContinue();
	  return;

	case 'd':
		// Disable debug using a general query
		cerr << "Warning: RSP 'd' packet is deprecated (define a 'Q' "
		    << "packet instead: ignored" << endl;
		return;

	case 'D':
		// Detach GDB. Do this by closing the client. The rules say that
		// execution should continue, so unstall the processor.
		pkt->packStr("OK");
		rsp->putPkt(pkt);
		rsp->rspClose();
		//debugUnit->unstall ();      
		return;

	case 'F':
		// File I/O is not currently supported
		cerr << "Warning: RSP file I/O not currently supported: 'F' "
		    << "packet ignored" << endl;
		return;

	case 'g':
		rspReadAllRegs();
		return;

	case 'G':
		rspWriteAllRegs();
		return;

	case 'H':
	  // Set the thread number of subsequent operations.
	  printf("Thread H cmd >%s<\n", pkt->data);
	  //Thread T cmd >T100<
	  //Thread H cmd >Hg100<   set current thread - decimal 256.
	  // Letter is c or g - c is for cont and step commands. g is for others.
	  // -1 means pick any thread.
	  {
	    int id = -1000;
	    int letter = 0;
	    if (1 == sscanf(pkt->data, "Hc%d", &id))
	      {
		letter = 'c';
	      }
	    else if (1 == sscanf(pkt->data, "Hg%x", &id))
	      {
		letter = 'g';
	      }
	    else
	      {
		cerr << "Warning: Failed to recognize RSP set thread command: " << pkt->data << endl;
		pkt->packStr("E01");
		rsp->putPkt(pkt);
		return;
	      }
	    printf ("rspSetThread: new id letter=%c %i\n", letter, id);
	    debugUnit->setThread(letter, id); // use also for customer number/core when debugging hardware CMP.
	    pkt->packStr("OK");
	    rsp->putPkt(pkt);
	  }
	  return;

	case 'i':
	case 'I':
		// Single cycle step not currently supported. Mark the target as
		// running, so that next time it will be detected as stopped (it is
		// still stalled in reality) and an ack sent back to the client.
		cerr << "Warning: RSP cycle stepping not supported: target "
		    << "stopped immediately" << endl;
		targetStopped = false;
		return;

	case 'k':
		// Kill request. Do nothing for now.
		return;

	case 'm':
		// Read memory (symbolic)
		rspReadMem();
		return;

	case 'M':
		// Write memory (symbolic)
		rspWriteMem();
		return;

	case 'p':
		// Read a register
		rspReadReg();
		return;

	case 'P':
		// Write a register
		rspWriteReg();
		return;

	case 'q':
		// Any one of a number of query packets
		rspQuery();
		return;

	case 'Q':
		// Any one of a number of set packets
		rspSet();
		return;

	case 'r':
		// Reset the system. Deprecated (use 'R' instead)
		cerr << "Warning: RSP 'r' packet is deprecated (use 'R' "
		    << "packet instead): ignored" << endl;
		return;

	case 'R':
		// Restart the program being debugged.
		rspRestart();
		return;

	case 's':
	  // Single step one machine instruction.
	  rspStep(0 /*EXCEPT_NONE*/);
	  return;
	  
	case 'S':
	  // Single step one machine instruction.
	  rspStep();
	  return;

	case 't':
		// Search. This is not well defined in the manual and for now we don't
		// support it. No response is defined.
		cerr << "Warning: RSP 't' packet not supported: ignored"
		    << endl;
		return;

	case 'T':
		// Is the thread alive. We are bare metal, so don't have a thread
		// context. The answer is always "OK".
	  printf("Thread T cmd >%s<\n", pkt->data);
	  
		pkt->packStr("OK");
		rsp->putPkt(pkt);
		return;

	case 'v':
		// Any one of a number of packets to control execution
		rspVpkt();
		return;

	case 'X':
		// Write memory (binary)
		rspWriteMemBin();
		return;

	case 'z':
		// Remove a breakpoint/watchpoint.
		rspRemoveMatchpoint();
		return;

	case 'Z':
		// Insert a breakpoint/watchpoint.
		rspInsertMatchpoint();
		return;

	default:
		// Unknown commands are ignored
		cerr << "Warning: Unknown RSP request" << pkt->data << endl;
		return;
	}
}				// rspClientRequest ()

//-----------------------------------------------------------------------------
//! Check if processor is stalled. If it is, read the DRR and return the
//! target signal code.
//-----------------------------------------------------------------------------
void GdbServerSC::rspCheckForException()
{
  uint32_t drr;
  
  if (!debugUnit->isStalled(&rsp_sigval)) 
    {
      // Processor not stalled. Just return;
      return;
    }
  
  // Updates the local copy of rsp_sigval with the reason for stalling if stalled.

    
  return;
}

//-----------------------------------------------------------------------------
//! Send a packet acknowledging an exception has occurred

//! The only signal we ever see in this implementation is TRAP.
//-----------------------------------------------------------------------------
void GdbServerSC::rspReportException()
{
	// Construct a signal received packet
	pkt->data[0] = 'S';
	pkt->data[1] = Utils::hex2Char(rsp_sigval >> 4);
	pkt->data[2] = Utils::hex2Char(rsp_sigval % 16);
	pkt->data[3] = '\0';
	pkt->setLen(strlen(pkt->data));

	rsp->putPkt(pkt);

}				// rspReportException ()

//-----------------------------------------------------------------------------
//! Handle a RSP continue request

//! This version is typically used for the 'c' packet, to continue without
//! signal, in which case EXCEPT_NONE is passed in as the exception to use.

//! At present other exceptions are not supported

//! @param[in] except  The OpenRISC 1000 exception to use
//-----------------------------------------------------------------------------
void GdbServerSC::rspContinue(uint32_t except)
{
  uint32_t addr;		// Address to continue from.

  // Reject all except 'c' packets
  if ('c' != pkt->data[0]) 
    {
      cerr <<
	"Warning: Continue with signal not currently supported: " <<
	"ignored" << endl;
      return;
    }
  // Get an address if we have one
  if (0 == strcmp("c", pkt->data)) 
    {
      addr = readNpc();	// Default uses current NPC
    } 
  else if (1 != sscanf(pkt->data, "c%x", &addr)) 
    {
      cerr << "Warning: RSP continue address " << pkt->data
	   << " not recognized: ignored" << endl;
      addr = readNpc();	// Default uses current NPC
    }
  
  rspContinue(addr, 0 /*EXCEPT_NONE*/);
  debugUnit->control_cmd(RunC);
  targetStopped = false;
}	// rspContinue ()

//-----------------------------------------------------------------------------
//! Generic processing of a continue request

//! The signal may be EXCEPT_NONE if there is no exception to be
//! handled. Currently the exception is ignored.

//! The single step flag is cleared in the debug registers and then the
//! processor is unstalled.

//! @param[in] addr    Address from which to step
//! @param[in] except  The exception to use (if any)                         
//-----------------------------------------------------------------------------
void GdbServerSC::rspContinue(u64_t addr, uint32_t except)
{
  debugUnit->control_cmd(RunC, except, addr);
}

//-----------------------------------------------------------------------------
//! Handle a RSP continue with signal request

//! @todo Currently does nothing. Will use the underlying generic continue
//!       function. 
//-----------------------------------------------------------------------------
void GdbServerSC::rspContinue()
{
  cerr << "RSP continue with signal '" << pkt->data
       << "' received" << endl;
  
}				// rspContinue ()

//------------------------------------------------------------------------------
//!Handle an interrupt from GDB

//! Detect an interrupt from GDB and stall the processor                        
//------------------------------------------------------------------------------
void GdbServerSC::rspInterrupt()
{
	unsigned char c;
	c = rsp->getRspChar();
	if (c < 0) {
		// Had issues, just return
		return;
	}
	// Ensure this is a ETX control char (0x3), currently, we only call this
	// function when we've peeked and seen it, otherwise, ignore, return and pray
	// things go back to normal...
	if (c != 0x03) {
		cerr <<
		    "* Warning: Interrupt character expected but not found on socket"
		    << endl;
		return;
	}
	// Otherwise, it's an interrupt packet, stall the processor, and upon return
	// to the main handle_rsp() loop, it will inform GDB.

	debugUnit->control_cmd(PauseC, -1, 0);

	// Send a stop reply response, manually set rsp.sigval to TARGET_SIGNAL_NONE
	rsp_sigval = TARGET_SIGNAL_NONE;
	rspReportException();

	return;

}


//-----------------------------------------------------------------------------
//! Handle a RSP read all registers request

//! OR1K: The registers follow the GDB sequence for OR1K: GPR0 through GPR31, PPC
//! (i.e. SPR PPC), NPC (i.e. SPR NPC) and SR (i.e. SPR SR). Each register is
//! returned as a sequence of bytes in target endian order.

//! Each byte is packed as a pair of hex digits.                             
//-----------------------------------------------------------------------------
void GdbServerSC::rspReadAllRegs()  // TODO or1k specific
{	// The GPRs
  int m = debugUnit->max_gprs();
  int len = 0;
  for (int r = 0; r < m; r++) 
    {
      Utils::reg2Hex(readGpr(r), &(pkt->data[r * 8]));
      len += 8;
    }

#if 0
  // or1k stuff
  // PPC, NPC and SR
  Utils::reg2Hex(debugUnit->readSpr(VHLS_SPR_PC), &(pkt->data[PPC_REGNUM * 8]));

  Utils::reg2Hex(readNpc(), &(pkt->data[NPC_REGNUM * 8]));

  Utils::reg2Hex(debugUnit->readSpr(SPR_SR), &(pkt->data[SR_REGNUM * 8]));
#endif  
  // Finalize the packet and send it
  pkt->data[len] = 0;
  pkt->setLen(len);
  rsp->putPkt(pkt);
}	// rspReadAllRegs ()

//-----------------------------------------------------------------------------
//! Handle a RSP write all registers request

//! The registers follow the GDB sequence for OR1K: GPR0 through GPR31, PPC
//! (i.e. SPR PPC), NPC (i.e. SPR NPC) and SR (i.e. SPR SR). Each register is
//! supplied as a sequence of bytes in target endian order.

//! Each byte is packed as a pair of hex digits.

//! @todo There is no error checking at present. Non-hex chars will generate a
//!       warning message, but there is no other check that the right amount
//!       of data is present. The result is always "OK".
//-----------------------------------------------------------------------------
void GdbServerSC::rspWriteAllRegs() // TODO or1k specific
{
  // The GPRs
  int len=0, m = debugUnit->max_gprs();
  for (int r = 0; r < m; r++) 
    {
    writeGpr(r, Utils::hex2Reg(&(pkt->data[r * 8])));
    len += 8;
  }
#if 0  
  // PPC, NPC and SR
  debugUnit->writeSpr(SPR_PPC,  Utils::hex2Reg(&(pkt->data[PPC_REGNUM * 8])));
  debugUnit->writeSpr(SPR_SR,   Utils::hex2Reg(&(pkt->data[SR_REGNUM * 8])));
  writeNpc(Utils::hex2Reg(&(pkt->data[NPC_REGNUM * 8])));
#endif
  // Acknowledge (always OK for now).
  pkt->packStr("OK");
  rsp->putPkt(pkt);
} // rspWriteAllRegs ()

//-----------------------------------------------------------------------------
//! Handle a RSP read memory (symbolic) request

//! Syntax is:

//!   m<addr>,<length>:

//! The response is the bytes, lowest address first, encoded as pairs of hex
//! digits.

//! The length given is the number of bytes to be read.
//-----------------------------------------------------------------------------
void GdbServerSC::rspReadMem()
{
	unsigned int addr;	// Where to read the memory
	int len;		// Number of bytes to read
	int off;		// Offset into the memory

	if (2 != sscanf(pkt->data, "m%x,%x:", &addr, &len)) {
		cerr << "Warning: Failed to recognize RSP read memory command: "
		    << pkt->data << endl;
		pkt->packStr("E01");
		rsp->putPkt(pkt);
		return;
	}
	// Make sure we won't overflow the buffer (2 chars per byte)
	if ((len * 2) >= pkt->getBufSize()) {
		cerr << "Warning: Memory read " << pkt->data
		    << " too large for RSP packet: truncated" << endl;
		len = (pkt->getBufSize() - 1) / 2;
	}
	//cerr << "rspReadMem: " << len << " bytes@0x"<< hex << addr << endl;
	// Refill the buffer with the reply
	for (off = 0; off < len; off++) {
		unsigned char ch = debugUnit->readMem8(addr + off);

		pkt->data[off * 2] = Utils::hex2Char(ch >> 4);
		pkt->data[off * 2 + 1] = Utils::hex2Char(ch & 0xf);
	}

	pkt->data[off * 2] = '\0';	// End of string
	pkt->setLen(strlen(pkt->data));
	rsp->putPkt(pkt);

}				// rsp_read_mem ()

//-----------------------------------------------------------------------------
//! Handle a RSP write memory (symbolic) request

//! Syntax is:

//!   m<addr>,<length>:<data>

//! The data is the bytes, lowest address first, encoded as pairs of hex
//! digits.

//! The length given is the number of bytes to be written.
//-----------------------------------------------------------------------------
void GdbServerSC::rspWriteMem()
{
	uint32_t addr;		// Where to write the memory
	int len;		// Number of bytes to write

	if (2 != sscanf(pkt->data, "M%x,%x:", &addr, &len)) {
		cerr << "Warning: Failed to recognize RSP write memory "
		    << pkt->data << endl;
		pkt->packStr("E01");
		rsp->putPkt(pkt);
		return;
	}
	// Find the start of the data and check there is the amount we expect.
	char *symDat = (char *)(memchr(pkt->data, ':', pkt->getBufSize())) + 1;
	int datLen = pkt->getLen() - (symDat - pkt->data);

	// Sanity check
	if (len * 2 != datLen) {
		cerr << "Warning: Write of " << len *
		    2 << "digits requested, but " << datLen <<
		    " digits supplied: packet ignored" << endl;
		pkt->packStr("E01");
		rsp->putPkt(pkt);
		return;
	}
	// Write the bytes to memory (no check the address is OK here)
	for (int off = 0; off < len; off++) {
		uint8_t nyb1 = Utils::char2Hex(symDat[off * 2]);
		uint8_t nyb2 = Utils::char2Hex(symDat[off * 2 + 1]);

		if (!debugUnit->writeMem8(addr + off, (nyb1 << 4) | nyb2)) {
			pkt->packStr("E01");
			rsp->putPkt(pkt);
			return;
		}
	}

	pkt->packStr("OK");
	rsp->putPkt(pkt);

}				// rspWriteMem ()

//-----------------------------------------------------------------------------
//! Read a single register

//! OR1K: The registers follow the GDB sequence for OR1K: GPR0 through GPR31, PC
//! (i.e. SPR NPC) and SR (i.e. SPR SR). The register is returned as a
//! sequence of bytes in target endian order.

//! Each byte is packed as a pair of hex digits.
//-----------------------------------------------------------------------------
void GdbServerSC::rspReadReg()
{
  unsigned int regNum;
  // Break out the fields from the data
  if (1 != sscanf(pkt->data, "p%x", &regNum)) 
    {
      cerr <<
	"Warning: Failed to recognize RSP read register command: "
	   << pkt->data << endl;
      pkt->packStr("E01");
		rsp->putPkt(pkt);
		return;
    }
  // Get the relevant register
  if (regNum < debugUnit->max_gprs()) 
    {
      Utils::Utils::reg2Hex(debugUnit->readSpr(VHLS_SPR_GPR0+regNum), pkt->data);
    } 
  else
    {
      Utils::Utils::reg2Hex(debugUnit->readSpr(regNum), pkt->data);
    }
#if 0
 or1k mapping ... move me too
 if (PPC_REGNUM == regNum) {
    Utils::Utils::reg2Hex(debugUnit->readSpr(SPR_PPC), pkt->data);
  } else if (NPC_REGNUM == regNum) {
    Utils::Utils::reg2Hex(readNpc(), pkt->data);
  } else if (SR_REGNUM == regNum) {
    Utils::Utils::reg2Hex(debugUnit->readSpr(SPR_SR), pkt->data);
  } else {
    // Error response if we don't know the register
    cerr << "Warning: Attempt to read unknown register" << regNum
	 << ": ignored" << endl;
    pkt->packStr("E01");
    rsp->putPkt(pkt);
    return;
	}
#endif  
  pkt->setLen(strlen(pkt->data));
  rsp->putPkt(pkt);
} // rspReadReg ()


//-----------------------------------------------------------------------------
//! Write a single register
//! The register is specified as a sequence of bytes in target endian order.
//! Each byte is packed as a pair of hex digits.
//-----------------------------------------------------------------------------
void GdbServerSC::rspWriteReg()
{
  unsigned int regNum;
  char valstr[9];		// Allow for EOS on the string
  // Break out the fields from the data
  if (2 != sscanf(pkt->data, "P%x=%8s", &regNum, valstr)) {
    cerr <<
      "Warning: Failed to recognize RSP write register command "
	 << pkt->data << endl;
    pkt->packStr("E01");
    rsp->putPkt(pkt);
    return;
  }

  int rc = -2;
  // Set the relevant register
  if (regNum < debugUnit->max_gprs()) 
    {
      rc = writeGpr(regNum, Utils::hex2Reg(valstr));
    } 
  else 
    {
      rc = debugUnit->writeSpr(regNum, Utils::hex2Reg(valstr));
    }
#if 0
 OR1K mappings ... moveme
  else if (PPC_REGNUM == regNum) 
    {
      debugUnit->writeSpr(VHLS_SPR_PC, Utils::hex2Reg(valstr));
    } 
  else if (NPC_REGNUM == regNum) 
    {
      writeNpc(Utils::hex2Reg(valstr));
    } 
  else if (SR_REGNUM == regNum) 
    {
      debugUnit->writeSpr(VHLS_SPR_PSR, Utils::hex2Reg(valstr));
    } 
  else {
    // Error response if we don't know the register
    cerr << "Warning: Attempt to write unknown register " << regNum
	 << ": ignored" << endl;
    pkt->packStr("E01");
    rsp->putPkt(pkt);
    return;
  }
#endif
  pkt->packStr(rc == 0 ? "OK": "E01");
  rsp->putPkt(pkt);

}				// rspWriteReg ()

//-----------------------------------------------------------------------------
//! Handle a RSP query request
//-----------------------------------------------------------------------------
void GdbServerSC::rspQuery()
{
  static int index = 0;
  if (!strcmp("qAttached", pkt->data)) 
    {
      // We are always attaching to an existing process with the bare metal
      // embedded system. 
      pkt->packStr("1");
      rsp->putPkt(pkt);
    } 
  else if (!strncmp("qCRC", pkt->data, strlen("qCRC"))) {
    // Return CRC of memory area
    cerr << "Warning: RSP CRC query not supported" << endl;
    pkt->packStr("E01");
    rsp->putPkt(pkt);
  } 

  else if (!strcmp("qC", pkt->data))
    {
      // Return the current thread ID (unsigned hex). A null response
      // indicates to use the previously selected thread.
      //sprintf(pkt->data, "QC%x", current_thread);
      //sprintf(pkt->data, "");
      pkt->setLen(strlen(pkt->data));
      rsp->putPkt(pkt);
    } 
  
  else if (!strcmp("qfThreadInfo", pkt->data))  // first
    {
      // Return info about active threads. 
      // We map this to info on our cpu cores.
      index = 0;
      sprintf(pkt->data, "m%x", 1001 * index);
      pkt->setLen(strlen(pkt->data));
      rsp->putPkt(pkt);
    } 
  else if (!strcmp("qsThreadInfo", pkt->data))  // subsequent 
    {
      extern int g_cores;
      if (index < g_cores)
	{
	  sprintf(pkt->data, "m%x",  1001 * index);
	  pkt->setLen(strlen(pkt->data));
	  index ++;
	}
      // Return info about more active threads. We have no more, so return the end of list marker, 'l'
      else pkt->packStr("l");
      rsp->putPkt(pkt);
    } 
  else if (!strncmp("qGetTLSAddr:", pkt->data, strlen("qGetTLSAddr:"))) 
    {
      // We don't support this feature
      pkt->packStr("");
      rsp->putPkt(pkt);
    }
  else if (!strncmp("qL", pkt->data, strlen("qL"))) 
    {
      // Deprecated and replaced by 'qfThreadInfo'
      cerr << "Warning: RSP qL deprecated: no info returned" << endl;
      pkt->packStr("qM001");
      rsp->putPkt(pkt);
    } 
  else if (!strcmp("qOffsets", pkt->data)) 
    {
      // Report any relocation
      pkt->packStr("Text=0;Data=0;Bss=0");
      rsp->putPkt(pkt);
    } 
  else if (!strncmp("qP", pkt->data, strlen("qP"))) 
    {
      // Deprecated and replaced by 'qThreadExtraInfo'
      cerr << "Warning: RSP qP deprecated: no info returned" << endl;
      pkt->packStr("");
      rsp->putPkt(pkt);
    }
  else if (!strncmp("qRcmd,", pkt->data, strlen("qRcmd,")))
    {
      // This is used to interface to commands to do "stuff"
      rspCommand();
    }
  else if (!strncmp("qSupported", pkt->data, strlen("qSupported"))) 
    {
      // Report a list of the features we support. For now we just ignore any
      // supplied specific feature queries, but in the future these may be
      // supported as well. Note that the packet size allows for 'G' + all the
		// registers sent to us, or a reply to 'g' with all the registers and an
		// EOS so the buffer is a well formed string.
      sprintf(pkt->data, "PacketSize=%x", pkt->getBufSize());
      pkt->setLen(strlen(pkt->data));
      rsp->putPkt(pkt);
    } 
  else if (!strncmp("qSymbol:", pkt->data, strlen("qSymbol:")))
    {
      // Offer to look up symbols. Nothing we want (for now). TODO. This just
      // ignores any replies to symbols we looked up, but we didn't want to
      // do that anyway!
      pkt->packStr("OK");
      rsp->putPkt(pkt);
    } else if (!strncmp("qThreadExtraInfo,", pkt->data, strlen("qThreadExtraInfo,"))) 
    {
      // Report that we are runnable, but the text must be hex ASCI
      // digits. For now do this by steam, reusing the original packet
      sprintf(pkt->data, "%02x%02x%02x%02x%02x%02x%02x%02x%02x",
	      'R', 'u', 'n', 'n', 'a', 'b', 'l', 'e', 0);
      pkt->setLen(strlen(pkt->data));
      rsp->putPkt(pkt);
    }
  else if (!strncmp("qTStatus", pkt->data, strlen("qTstatus"))) 
    {
      // We don't current support tracing, return empty packet
      pkt->packStr("");
      rsp->putPkt(pkt);
    }
  else if (!strncmp("qXfer:", pkt->data, strlen("qXfer:"))) 
    {
      // For now we support no 'qXfer' requests, but these should not be
      // expected, since they were not reported by 'qSupported'
      cerr << "Warning: RSP 'qXfer' not supported: ignored" << endl;
      pkt->packStr("");
      rsp->putPkt(pkt);
    }
  else {
    cerr << "Unrecognized RSP query: ignored" << endl;
  }
}				// rspQuery ()

//-----------------------------------------------------------------------------
//! Handle a RSP qRcmd request

//! The actual command follows the "qRcmd," in ASCII encoded to hex
//-----------------------------------------------------------------------------
void GdbServerSC::rspCommand()
{
	char cmd[RSP_PKT_MAX];

	Utils::hex2Ascii(cmd, &(pkt->data[strlen("qRcmd,")]));

	// Work out which command it is
	if (!strncmp("readspr ", cmd, strlen("readspr"))) {
		unsigned int sprNum;

		// Parse and return error if we fail
		if (1 != sscanf(cmd, "readspr %4x", &sprNum)) {
			cerr << "Warning: qRcmd " << cmd
			    << "not recognized: ignored" << endl;
			pkt->packStr("E01");
			rsp->putPkt(pkt);
			return;
		}
		// SPR out of range
		if (sprNum > VHLS_MAX_SPRS) {
			cerr << "Warning: qRcmd readspr " << hex << sprNum
			    << dec << " too large: ignored" << endl;
			pkt->packStr("E01");
			rsp->putPkt(pkt);
			return;
		}
		// Construct the reply
		sprintf(cmd, "%8lx", debugUnit->readSpr(sprNum));
		Utils::ascii2Hex(pkt->data, cmd);
		pkt->setLen(strlen(pkt->data));
		rsp->putPkt(pkt);
	} else if (!strncmp("writespr ", cmd, strlen("writespr"))) {
		unsigned int sprNum;
		uint32_t val;

		// Parse and return error if we fail
		if (2 != sscanf(cmd, "writespr %4x %8x", &sprNum, &val)) {
			cerr << "Warning: qRcmd " << cmd <<
			    " not recognized: ignored" << endl;
			pkt->packStr("E01");
			rsp->putPkt(pkt);
			return;
		}
		// SPR out of range
		if (sprNum > VHLS_MAX_SPRS) {
			cerr << "Warning: qRcmd writespr " << hex << sprNum
			    << dec << " too large: ignored" << endl;
			pkt->packStr("E01");
			rsp->putPkt(pkt);
			return;
		}
		// Update the SPR and reply "OK"
		debugUnit->writeSpr(sprNum, val);
		pkt->packStr("OK");
		rsp->putPkt(pkt);
	}

}				// rspCommand ()

//-----------------------------------------------------------------------------
//! Handle a RSP set request
//-----------------------------------------------------------------------------
void GdbServerSC::rspSet()
{
	if (!strncmp("QPassSignals:", pkt->data, strlen("QPassSignals:"))) {
		// Passing signals not supported
		pkt->packStr("");
		rsp->putPkt(pkt);
	} else if ((!strncmp("QTDP", pkt->data, strlen("QTDP"))) ||
		   (!strncmp("QFrame", pkt->data, strlen("QFrame"))) ||
		   (!strcmp("QTStart", pkt->data)) ||
		   (!strcmp("QTStop", pkt->data)) ||
		   (!strcmp("QTinit", pkt->data)) ||
		   (!strncmp("QTro", pkt->data, strlen("QTro")))) {
		// All tracepoint features are not supported. This reply is really only
		// needed to 'QTDP', since with that the others should not be
		// generated.
		pkt->packStr("");
		rsp->putPkt(pkt);
	} else {
		cerr << "Unrecognized RSP set request: ignored" << endl;
		delete pkt;
	}
}				// rspSet ()

//-----------------------------------------------------------------------------
//! Handle a RSP restart request

//! For now we just put the program counter back to the reset vector. If we
//! supported the vRun request, we should use the address specified
//! there. There is no point in unstalling the processor, since we'll never
//! get control back.
//-----------------------------------------------------------------------------
void GdbServerSC::rspRestart()
{
  writeNpc(0/*EXCEPT_RESET*/);

} // rspRestart ()




//-----------------------------------------------------------------------------
//! Handle a RSP step request

//! This version is typically used for the 's' packet, to continue without
//! signal, in which case EXCEPT_NONE is passed in as the exception to use.

//! @param[in] except  The exception to use. Only EXCEPT_NONE should be set
//!                    this way.
//-----------------------------------------------------------------------------
void GdbServerSC::rspStep(uint32_t except)
{
	uint32_t addr;		// The address to step from, if any

	// Reject all except 's' packets
	if ('s' != pkt->data[0]) {
		cerr << "Warning: Step with signal not currently supported: "
		    << "ignored" << endl;
		return;
	}

	if (!strcmp("s", pkt->data)) {
		addr = readNpc();	// Default uses current NPC
	} else if (1 != sscanf(pkt->data, "s%x", &addr)) {
		cerr << "Warning: RSP step address " << pkt->data
		    << " not recognized: ignored" << endl;
		addr = readNpc();	// Default uses current NPC
	}

	rspStep(addr, 0/*EXCEPT_NONE*/);

} // rspStep ()

//-----------------------------------------------------------------------------
//! Handle a RSP step with signal request

//! @todo Currently null. Will use the underlying generic step function.
//-----------------------------------------------------------------------------
void GdbServerSC::rspStep()
{
	cerr << "RSP step with signal '" << pkt->data << "' received" << endl;

}				// rspStep ()

//-----------------------------------------------------------------------------
//! Generic processing of a step request

//! The signal may be EXCEPT_NONE if there is no exception to be
//! handled. Currently the exception is ignored.

//! The single step flag is set in the debug registers and then the processor
//! is unstalled.

//! @todo There appears to be a bug in the ORPSoC debug unit, whereby multiple
//!       single steps make a mess of the pipeline, leading to multiple
//!       executions of the same instruction. A fix would be to use l.trap (as
//!       for continue) for any consecutive calls to step.

//! @param[in] addr    Address from which to step
//! @param[in] except  The exception to use (if any)                         
//-----------------------------------------------------------------------------
void GdbServerSC::rspStep(uint32_t addr, uint32_t except)
{
	// Set the address as the value of the next program counter
	writeNpc(addr);

	/*
	   // If we're continuing from a breakpoint, replace that instruction in the memory
	   // ... actually no, I was wrong about this.
	   if (NULL != mpHash->lookup (BP_MEMORY, addr) && rsp_sigval == TARGET_SIGNAL_TRAP)
	   {
	   MpEntry *entry = mpHash->lookup (BP_MEMORY, addr);
	   debugUnit->writeMem32(entry->addr, entry->instr);
	   }
	 */
	debugUnit->step(false); // Step one instruction and then stop.
	targetStopped = false; // We think it is running until it tells us.
} // rspStep ()

//-----------------------------------------------------------------------------
//! Handle a RSP 'v' packet

//! These are commands associated with executing the code on the target
//-----------------------------------------------------------------------------
void GdbServerSC::rspVpkt()
{
	if (!strncmp("vAttach;", pkt->data, strlen("vAttach;"))) {
		// Attaching is a null action, since we have no other process. We just
		// return a stop packet (using TRAP) to indicate we are stopped.
		pkt->packStr("S05");
		rsp->putPkt(pkt);
		return;
	} else if (!strcmp("vCont?", pkt->data)) {
		// For now we don't support this.
		pkt->packStr("");
		rsp->putPkt(pkt);
		return;
	} else if (!strncmp("vCont", pkt->data, strlen("vCont"))) {
		// This shouldn't happen, because we've reported non-support via vCont?
		// above
		cerr << "Warning: RSP vCont not supported: ignored" << endl;
		return;
	} else if (!strncmp("vFile:", pkt->data, strlen("vFile:"))) {
		// For now we don't support this.
		cerr << "Warning: RSP vFile not supported: ignored" << endl;
		pkt->packStr("");
		rsp->putPkt(pkt);
		return;
	} else if (!strncmp("vFlashErase:", pkt->data, strlen("vFlashErase:"))) {
		// For now we don't support this.
		cerr << "Warning: RSP vFlashErase not supported: ignored" <<
		    endl;
		pkt->packStr("E01");
		rsp->putPkt(pkt);
		return;
	} else if (!strncmp("vFlashWrite:", pkt->data, strlen("vFlashWrite:"))) {
		// For now we don't support this.
		cerr << "Warning: RSP vFlashWrite not supported: ignored" <<
		    endl;
		pkt->packStr("E01");
		rsp->putPkt(pkt);
		return;
	} else if (!strcmp("vFlashDone", pkt->data)) {
		// For now we don't support this.
		cerr << "Warning: RSP vFlashDone not supported: ignored" <<
		    endl;;
		pkt->packStr("E01");
		rsp->putPkt(pkt);
		return;
	} else if (!strncmp("vRun;", pkt->data, strlen("vRun;"))) {
		// We shouldn't be given any args, but check for this
		if (pkt->getLen() > strlen("vRun;")) {
			cerr << "Warning: Unexpected arguments to RSP vRun "
			    "command: ignored" << endl;
		}
		// Restart the current program. However unlike a "R" packet, "vRun"
		// should behave as though it has just stopped. We use signal 5 (TRAP).
		rspRestart();
		pkt->packStr("S05");
		rsp->putPkt(pkt);
	} else {
		cerr << "Warning: Unknown RSP 'v' packet type " << pkt->data
		    << ": ignored" << endl;
		pkt->packStr("E01");
		rsp->putPkt(pkt);
		return;
	}
}				// rspVpkt ()

//-----------------------------------------------------------------------------
//! Handle a RSP write memory (binary) request

//! Syntax is:

//!   X<addr>,<length>:

//! Followed by the specified number of bytes as raw binary. Response should be
//! "OK" if all copied OK, E<nn> if error <nn> has occurred.

//! The length given is the number of bytes to be written. The data buffer has
//! already been unescaped, so will hold this number of bytes.

//! The data is in model-endian format, so no transformation is needed.
//-----------------------------------------------------------------------------
void GdbServerSC::rspWriteMemBin()
{
	uint32_t addr;		// Where to write the memory
	int len;		// Number of bytes to write

	if (2 != sscanf(pkt->data, "X%x,%x:", &addr, &len)) {
		cerr <<
		    "Warning: Failed to recognize RSP write memory command: %s"
		    << pkt->data << endl;
		pkt->packStr("E01");
		rsp->putPkt(pkt);
		return;
	}
	// Find the start of the data and "unescape" it. Bindat must be unsigned, or
	// all sorts of horrible sign extensions will happen when val is computed
	// below!
	uint8_t *bindat = (uint8_t *) (memchr(pkt->data, ':',
					      pkt->getBufSize())) + 1;
	int off = (char *)bindat - pkt->data;
	int newLen = Utils::rspUnescape((char *)bindat, pkt->getLen() - off);

	// Sanity check
	if (newLen != len) {
		int minLen = len < newLen ? len : newLen;

		cerr << "Warning: Write of " << len << " bytes requested, but "
		    << newLen << " bytes supplied. " << minLen <<
		    " will be written" << endl;
		len = minLen;
	}
	// Write the bytes to memory. More efficent to do this in 32-bit chunks
	int startBytes = addr & 0x3;
	int endBytes = (addr + len) & 0x3;

	// First partial word. Access bindat in an endian independent fashion.
	for (off = 0; off < startBytes; off++) {
		if (!debugUnit->writeMem8(addr + off, bindat[off])) {
			pkt->packStr("E01");
			rsp->putPkt(pkt);
			return;
		}
	}

	// The bulk as words. Convert to model endian before writing.
	for (off = startBytes; off < len; off += 4) {
		uint32_t val = *((uint32_t *) (&(bindat[off])));

		if (!debugUnit->writeMem32(addr + off, Utils::htotl(val))) {
			pkt->packStr("E01");
			rsp->putPkt(pkt);
			return;
		}
	}

	// Last partial word. Access bindat in an endian independent fashion.
	for (off = len - endBytes; off < len; off++) {
		uint32_t base = (addr + len) & 0xfffffffc;

		if (!debugUnit->writeMem8(base + off, bindat[off])) {
			pkt->packStr("E01");
			rsp->putPkt(pkt);
			return;
		}
	}

	pkt->packStr("OK");
	rsp->putPkt(pkt);

}				// rspWriteMemBin ()

//-----------------------------------------------------------------------------
//! Handle a RSP remove breakpoint or matchpoint request

//! For now only memory breakpoints are implemented, which are implemented by
//! substituting a breakpoint at the specified address. The implementation must
//! cope with the possibility of duplicate packets.

//! @todo This doesn't work with icache/immu yet
//-----------------------------------------------------------------------------
void GdbServerSC::rspRemoveMatchpoint()
{

	MpType type;		// What sort of matchpoint
	uint32_t addr;		// Address specified
	int len;		// Matchpoint length (not used)

	// Break out the instruction
	if (3 != sscanf(pkt->data, "z%1d,%x,%1d", (int *)&type, &addr, &len)) {
		cerr << "Warning: RSP matchpoint deletion request not "
		    << "recognized: ignored" << endl;
		pkt->packStr("E01");
		rsp->putPkt(pkt);
		return;
	}
	// Sanity check that the length is 4
	if (4 != len) {
		cerr << "Warning: RSP matchpoint deletion length " << len
		    << "not valid: 4 assumed" << endl;
		len = 4;
	}

	const char *rcs = debugUnit->MatchpointControl("D", type, addr);
	pkt->packStr(rcs);
	rsp->putPkt(pkt);
	return;
}


//---------------------------------------------------------------------------*/
//! Handle a RSP insert breakpoint or matchpoint request

//! For now only memory breakpoints are implemented, which are implemented by
//! substituting a breakpoint at the specified address. The implementation must
//! cope with the possibility of duplicate packets.

//! @todo This doesn't work with icache/immu yet
//---------------------------------------------------------------------------*/
void GdbServerSC::rspInsertMatchpoint()
{
  MpType type;		// What sort of matchpoint
  uint32_t addr;		// Address specified
  int len;		// Matchpoint length (not used)

  // Break out the instruction
  if (3 != sscanf(pkt->data, "Z%1d,%x,%1d", (int *)&type, &addr, &len)) {
    cerr << "Warning: RSP matchpoint insertion request not "
	 << "recognized: ignored" << endl;
    pkt->packStr("E01");
    rsp->putPkt(pkt);
    return;
  }
  // Sanity check that the length is 4
  if (4 != len) {
    cerr << "Warning: RSP matchpoint insertion length " << len
	 << "not valid: 4 assumed" << endl;
    len = 4;
  }
  
  const char *rcs = debugUnit->MatchpointControl("I", type, addr);
  
  pkt->packStr(rcs);
  rsp->putPkt(pkt);
} // rspInsertMatchpoint ()

  
//-----------------------------------------------------------------------------
//! Read the value of the Next Program Counter (a SPR)

//! A convenience routine.   OR1K:

//! Setting the NPC flushes the pipeline, so subsequent reads will return
//! zero until the processor has refilled the pipeline. This will not be
//! happening if the processor is stalled (as it is when GDB had control).

//! However for debugging we always want to know what the effective value of
//! the NPC will be (i.e. the value that will be used once the pipeline has
//! refilled). Fortunately SPR cacheing in the debug unit silently solves this
//! for us.

//! @return  The value of the NPC
//-----------------------------------------------------------------------------
uint32_t GdbServerSC::readNpc()
{
  return debugUnit->readSpr(VHLS_SPR_PC);
} // readNpc ()

int GdbServerSC::writeNpc(u32_t addr)
{
  return debugUnit->writeSpr(VHLS_SPR_PC, addr);
}


//-----------------------------------------------------------------------------
//! Read the value of a General Purpose Register

//! A convenience function. This is just a wrapper for reading a SPR, since
//! the GPR's are mapped into SPR space

//! @param[in]  regNum  The GPR to read
//! @return  The value of the GPR
//-----------------------------------------------------------------------------
uint32_t GdbServerSC::readGpr(int regNum)
{
  //return debugUnit->readGpr(regNum);
  return debugUnit->readSpr(VHLS_SPR_GPR0 + regNum);
  
} // readGpr ()

//-----------------------------------------------------------------------------
//! Write the value of an OpenRISC 1000 General Purpose Register

//! A convenience function. This is just a wrapper for writing a SPR, since
//! the GPR's are mapped into SPR space

//! @param[in]  regNum  The GPR to read

//! @return  The value of the GPR
//-----------------------------------------------------------------------------
int GdbServerSC::writeGpr(int regNum, uint32_t value)
{
  return debugUnit->writeSpr(VHLS_SPR_GPR0 + regNum, value);

}				// writeGpr ()

//-----------------------------------------------------------------------------
//! Check if we received anything via the pipe from the or1200 monitor

//! We stall the processor, and behave in a manner similar to if an interrupt
//! had been received. Perhaps the sigval should be set differently/more
//! more appropriately.
//! Read from the pipe should be NON-blocking.

//! @return  false if nothing received, else true
//-----------------------------------------------------------------------------
bool GdbServerSC::checkMonitorPipe()
{
	char readChar;
	assert(0);
#if 0
	int n = read(monitor_to_gdb_pipe[0][0], &readChar, sizeof(char));
	if (!(((n < 0) && (errno == EAGAIN)) || (n == 0)) && !targetStopped) {
		debugUnit->stall(false);
		// Send a stop reply response, manually set rsp.sigval to TARGET_SIGNAL_NONE
		rsp_sigval = TARGET_SIGNAL_NONE;
		rspReportException();
		targetStopped = true;	// Processor now not running
		write(monitor_to_gdb_pipe[1][1], &readChar, sizeof(char));
		return true;
	}
#endif
	return false;

}				// checkMonitorPipe ()

// eof
