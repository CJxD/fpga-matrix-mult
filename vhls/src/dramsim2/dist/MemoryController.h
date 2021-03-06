/*********************************************************************************
*  Copyright (c) 2010-2011, Elliott Cooper-Balis
*                             Paul Rosenfeld
*                             Bruce Jacob
*                             University of Maryland 
*                             dramninjas [at] umd [dot] edu
*  All rights reserved.
*  
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions are met:
*  
*     * Redistributions of source code must retain the above copyright notice,
*        this list of conditions and the following disclaimer.
*  
*     * Redistributions in binary form must reproduce the above copyright notice,
*        this list of conditions and the following disclaimer in the documentation
*        and/or other materials provided with the distribution.
*  
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
*  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
*  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
*  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
*  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
*  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*********************************************************************************/








#ifndef MEMORYCONTROLLER_H
#define MEMORYCONTROLLER_H

//MemoryController.h
//
//Header file for memory controller object
//

#include "SimulatorObject.h"
#include "Transaction.h"
#include "SystemConfiguration.h"
#include "CommandQueue.h"
#include "BusPacket.h"
#include "BankState.h"
#include "Rank.h"
#include <map>
#include "Callback.h"

using namespace std;
using namespace DRAMSim;

namespace DRAMSim
{
  typedef CallbackBase<void,unsigned,double, double> Callback1_t; // added by djg/cambridge

class MemorySystem;
class MemoryController : public SimulatorObject
{

public:
	//functions
	MemoryController(MemorySystem* ms, std::ofstream *outfile);
	virtual ~MemoryController();
	void resetStats(); // Moved into separate function djg/cambridge.
	bool addTransaction(Transaction &trans);
	bool WillAcceptTransaction();
	void returnReadData(const Transaction &trans);
	void receiveFromBus(BusPacket *bpacket);
	void attachRanks(vector<Rank> *ranks);
	void update();
	void printStats(bool finalStats = false);
	void collectEnergy(double &stat_engy, double &dyn_engy);//added by djg/cambridge

	//fields
	vector<Transaction> transactionQueue;
	vector< vector <BankState> > bankStates;


private:
	//functions
	void addressMapping(uint64_t physicalAddress, unsigned &rank, unsigned &bank, unsigned &row, unsigned &col);
	void insertHistogram(unsigned latencyValue, unsigned rank, unsigned bank);

	//fields
	MemorySystem *parentMemorySystem;

	CommandQueue commandQueue;
	BusPacket *poppedBusPacket;
	vector<unsigned>refreshCountdown;
	vector<BusPacket *> writeDataToSend;
	vector<unsigned> writeDataCountdown;
	vector<Transaction> returnTransaction;
	vector<Transaction> pendingReadTransactions;
	map<unsigned,unsigned> latencies; // latencyValue -> latencyCount
	vector<bool> powerDown;

	vector<Rank> *ranks;

	//output file
	std::ofstream *visDataOut;

	// these packets are counting down waiting to be transmitted on the "bus"
	BusPacket *outgoingCmdPacket;
	unsigned cmdCyclesLeft;
	BusPacket *outgoingDataPacket;
	unsigned dataCyclesLeft;
 public:
	uint64_t activations;
	uint64_t totalTransactions;
 private:
	vector<uint64_t> grandTotalBankAccesses; 
	vector<uint64_t> totalReadsPerBank;
	vector<uint64_t> totalWritesPerBank;

	vector<uint64_t> totalReadsPerRank;
	vector<uint64_t> totalWritesPerRank;

	// energy values are per rank
	vector< uint64_t > backgroundEnergy;
	vector< uint64_t > burstEnergy;
	vector< uint64_t > actpreEnergy;
	vector< uint64_t > refreshEnergy;

	vector< uint64_t > totalEpochLatency;

	unsigned channelBitWidth;
	unsigned rankBitWidth;
	unsigned bankBitWidth;
	unsigned rowBitWidth;
	unsigned colBitWidth;
	unsigned byteOffsetWidth;


	unsigned refreshRank;
	
};
}

#endif

