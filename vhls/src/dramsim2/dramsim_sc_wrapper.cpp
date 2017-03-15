// dramsim_sc_wrapper: A TLM dynamic memory abstraction with controller and power modelling using dramsim2.
// Also with with ELF and Intel Hex load facility.
// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2011-15 D J Greaves
// University of Cambridge, Computer Laboratory.
//

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "tenos.h"
#if TRACECOMM
#include "proc_id_extension.h"
#endif

#include "dramsim_sc_wrapper.h"
#include "dist/SystemConfiguration.h"

// This component cannot be used in other designs without this...
bool dynload_done = true;

#if TRACECOMM
extern int g_tracecomm;
#endif

#if 0
#define DRTRC(X) cout << sc_time_stamp() << "-"; \
  printf("tid=%d-", sc_get_current_process_b()->proc_id); X
#define DRTRC_HELP(X) X
#else
#define DRTRC(X)
#define DRTRC_HELP(X)
#endif

#define MANUAL_DELAY 0

#include "dramsim_sc_wrapper.h"

//int SHOW_SIM_OUTPUT = 1;

using namespace DRAMSim;

void dramsim_sc_wrapper::op_complete(u32_t id, u64_t adr, u64_t done_cycle)
{
  int slot;
  // undesirable search for the correct xaction... 
  // byte lanes and useful id in this ...
  // could be more than one xaction with this adr... oh dear.
  for (slot = 0; slot<MAX_DRAMSIM_THREADS; slot++) 
    if (Queue[slot].active && Queue[slot].trans.address == adr) 
      break;
  assert(slot < MAX_DRAMSIM_THREADS);
  Queue[slot].delay = sc_time((done_cycle - Queue[slot].add_clock_cycle) * tCK, SC_NS);
  Queue[slot].active = 0;
  DRTRC(cout << name() << " op complete " << dec << id << " " << hex << adr 
	<< ":" << dec << done_cycle << "\n");

}

void dramsim_sc_wrapper::update()
{
  ms->update();
  //cout << ms->currentClockCycle << "  bb\n";
}





u32_t dramsim_sc_wrapper::width() // DRAM databus width in bits.
{
  u32_t width = JEDEC_DATA_BUS_BITS;
  return width;
}

u32_t dramsim_sc_wrapper::bl() // DRAM burst length.
{
  assert(ms);
  extern unsigned BL; // not very OO!
  u32_t bl = BL; // ms->BL;
  return bl;
}

void dramsim_sc_wrapper::pw_eng_callback(u32_t id, double stat_energy, double dyn_energy)
{
  assert(0);
}

// Constructor
dramsim_sc_wrapper::dramsim_sc_wrapper(sc_core::sc_module_name names, 
				       u64_t mem_base, 
				       dramsim_ini_t *ini, 
				       uint32_t m_delay,
				       bool dmi_on,
				       sc_time clock_period):
  sc_module(names), 
  port0("port0"), 
  m_peq(this, &dramsim_sc_wrapper::peq_cb),
  ini(ini),
  clock_period(clock_period),  
  llsc_locks(names),
  mem_base(mem_base), 
  m_tracing_on(false), 
  m_qlock("m_qlock"),
  dmi_on(dmi_on),
  manual_delay(m_delay)
#ifdef TLM_POWER3
  , pw_module() 
#if PW_TLM_PAYLOAD > 0
  , read_bus_tracker(this)
#endif
#endif
{
  traceregions = 0;
  m_watchpoint = -1;//0xe8018;
  m_queued_items = 0;
  m_big_endian = tlm::TLM_LITTLE_ENDIAN; // This gets set by memloader.
  assert(ini);
  //  local_quantum = clock_period; // for now  - this forces approximately timed!
  mem_size_bytes = ini->megsOfMemory * (1LLU<<20LLU);

  new tenos_report_item(name(), "read_ops", &m_stats.m_read_ops);
  new tenos_report_item(name(), "write_ops", &m_stats.m_write_ops);
  new tenos_report_item(name(), "read_bytes", &m_stats.m_read_bytes);
  new tenos_report_item(name(), "write_bytes", &m_stats.m_write_bytes);
  new tenos_report_item(name(), "row_activations", &m_stats.m_row_activations);



#ifdef TLM_POWER3
  // Area does not matter since off chip.
  set_excess_area(pw_length(150, PW_um), pw_length(150,  PW_um));
  set_chip_region("DRAMBANKS");
#endif
  m_systemID = 105; // Arbitrary for now. Passing in atoi(name()) would make sense if name were an integer.
  ms = new DRAMSim::MemorySystem(m_systemID, 
				 ini->deviceIniFilename, 
				 ini->systemIniFilename, 
				 ini->pwdString, 
				 ini->traceFileName, 
				 ini->megsOfMemory);
  
  /*DRTRC(printf("%i Megs %s %i ya\n", ini->megsOfMemory, __FILE__, __LINE__));*/
  Callback_t *read_cb = new Callback<dramsim_sc_wrapper, void, u32_t, u64_t, u64_t>(this, &dramsim_sc_wrapper::op_complete);
  Callback_t *write_cb = new Callback<dramsim_sc_wrapper, void, u32_t, u64_t, u64_t>(this, &dramsim_sc_wrapper::op_complete);
  ms->RegisterCallbacks(read_cb, write_cb, NULL);

  stat_energy_sofar =0.0; 
  dyn_energy_sofar =0.0; 
  //energy_callback = new Callback<dramsim_sc_wrapper, void, u32_t, double, double>(this, &dramsim_sc_wrapper::pw_eng_callback));

  mem_blocksize_bytes = width() / 8 * bl();
  //DRTRC(printf("%s: bytes=" PFI64 ", block size = " PFI64 "\n", name(), mem_size_bytes, mem_blocksize_bytes));


  // Register TLM callback for incoming blocking transport calls.
  port0.register_b_transport(this, &dramsim_sc_wrapper::b_access);
  port0.register_nb_transport_fw(this, &dramsim_sc_wrapper::nb_transport_fw);

  //printf("%s %i ya\n", __FILE__, __LINE__);
  if (dmi_on) 
    {
      port0.register_get_direct_mem_ptr(this, &dramsim_sc_wrapper::get_direct_mem_ptr);
    }
  traceregions = 0;
#if TRACECOMM
  for(int i = 0; i < 255; ++i)
    m_hit_reads[i] = 0;
#endif
}

//
// Transfer energy quanta from dramsim model to TLM_POWER3 framework.
//
#ifdef TLM_POWER3
void dramsim_sc_wrapper::energy_accounter(sc_pwr::pw_agent_record *p_agent, 
					  PW_TLM_PAYTYPE *trans,
					  tlm::tlm_command cmd) 
// NB call this at end of simulation if not called often. Called at end of every xaction at the moment.
{
  //p_agent = 0; // for now
  double stat_energy = 0.0;
  double dyn_energy = 0.0;
  ms->collectEnergy(stat_energy, dyn_energy);
  stat_energy *= clock_period.to_seconds();
  dyn_energy *= clock_period.to_seconds();
  if (stat_energy < stat_energy_sofar)
    {
      DRTRC(printf("%s: stat_energy decreased: Assume model was reset\n", name()));
      stat_energy_sofar = 0.0;
    }
  if (dyn_energy < dyn_energy_sofar)
    {
      DRTRC(printf("%s: dyn_energy decreased: Assume model was reset\n", name()));
      dyn_energy_sofar = 0.0;
    }

  //#ifdef TLM_POWER3
  // These have already been converted to Joules by divides.
  pw_energy spe = pw_energy(stat_energy-stat_energy_sofar, pw_energy_unit::PW_JOULE);
  pw_energy dpe = pw_energy(dyn_energy-dyn_energy_sofar, pw_energy_unit::PW_JOULE);
  //(std::cout << sc_time_stamp() << name() << " Record energies s=" << spe <<  "  d=" << dpe << "\n");
  DRTRC(std::cout << name() << " " << sc_time_stamp() << " Record energies s=" 
	<< spe.round3sf() <<  "  d=" << dpe.round3sf() << "\n");
  if (p_agent) 
    {
      // Must log the energy on the p_agent so that the DMI mechanism gets an estimate.
      p_agent -> record_energy_use(spe, PW_ACCT_STATIC_DEFAULT, true);
      p_agent -> record_energy_use(dpe, trans, PW_ACCT_DYNAMIC_DEFAULT, true);
    }
  else
    {
      pw_module_base::record_energy_use(spe, PW_ACCT_STATIC_DEFAULT, true);
      pw_module_base::record_energy_use(dpe, trans, PW_ACCT_DYNAMIC_DEFAULT, true);
    }
  //#endif
  stat_energy_sofar = stat_energy;
  dyn_energy_sofar = dyn_energy;
}
#endif

void dramsim_sc_wrapper::loaddtb(const char* blob, u64_t offset) {
  cbgdtb* loader = new cbgdtb(this);
  printf("%s: loading device tree blob %s at offset " PFX64 "\n", name(), blob, offset);
  bool big_endian = false;
  u64_t hwm = loader->reader(big_endian, blob, offset, mem_size_bytes);
  printf("%s: loaded device tree blob %s from " PFX64 " to " PFX64 "\n", name(), blob, offset, hwm);

  delete loader;
}

void dramsim_sc_wrapper::loadme(const char *filename, bool elff, u64_t base, u64_t *entrypt) {
    eloader = new cbgelf(this);
    printf("%s: loading elf file %s: base=" PFX64 " size=" PFX64 "\n", name(), filename, base, mem_size_bytes);
    bool big_endian = false;
    u64_t hwm = eloader->reader(big_endian, filename, base, mem_size_bytes, entrypt);
    printf("%s: loaded elf file %s: hwm=" PFX64 "\n", name(), filename, hwm);
}
//
//
//
void dramsim_sc_wrapper::loadme(const char *filename, bool elff, u64_t *entrypt)
{
  bool big_endian = false; // Is set by loader.
  if (elff)
    {
      eloader = new cbgelf(this);
      printf("%s: loading elf file %s: base=" PFX64 " size=" PFX64 "\n", name(), filename, mem_base, mem_size_bytes);
      u64_t hwm = eloader->reader(big_endian, filename, mem_base, mem_size_bytes, entrypt);
      printf("%s: loaded elf file %s: hwm=" PFX64 "\n", name(), filename, hwm);
    }
  else 
    {
      iloader = new cbgihex(this);
      u64_t end = iloader->reader(big_endian, filename, mem_base, mem_size_bytes);
      DRTRC(printf("%s: loaded ihex file %s: " PFU64 " bytes\n", name(), filename, end));
    }
  m_big_endian = (big_endian) ? tlm::TLM_BIG_ENDIAN: tlm::TLM_LITTLE_ENDIAN;
  u64_t base = 0x0;
  if (base)
    {
      // Little print out: uses ip_read8 but the ip_write used for load has had ES applied. 
      DRTRC(printf("%s: little printout (%s)\n", name(),
	     tlm::has_host_endianness(m_big_endian) ? "host endian matches loaded file endian":
		   "as stored on modelling workstation: models should see other endian"));

      for(int i=0;i<40;i++)
	{
	  u64_t b = i*16LLU + base;
	  DRTRC(printf("%s " PFX64 ": ", name(), b));
	  for (u64_t j=0;j<16;j++)
	    {
	      u8_t d;
	      bool present = ip_read(b+j, d);
	      DRTRC(if (present) \
		      printf("%02X ", d);\
		    else printf("-- ")); // print dashes for not present memory
	    }
	  DRTRC(printf("\n"));
	}
    }
}


//
//
void dramsim_sc_wrapper::blockwhile(TransactionType transType, u64_t adr, sc_time &delay)
{
  // If we are the first/only then we must clock the model, otherwise, wait for other thread to
  // advance the subsystem until we are serviced.
  assert(m_queued_items < MAX_DRAMSIM_THREADS);
   
  m_qlock.lock();
  int slot;
  for (slot = 0; slot<MAX_DRAMSIM_THREADS; slot++) if (!Queue[slot].active) break;
  assert(slot < MAX_DRAMSIM_THREADS);
  Queue[slot].active = true;
  extern void* operator new(size_t s, Transaction_mm *mm);

  //Transaction_mm MM(&Queue[slot].trans);
  Transaction *tx; // = new (&MM) Transaction(transType, adr, 0); // To use positional new would require us to add to the transaction class
  // printf(" wo %p %p\n", tx, &Queue[slot].trans);
  tx = &Queue[slot].trans;

  // make sure that addres is aligned to the request size of DRAM
  u64_t aligned_addr = adr;
  if(adr % mem_blocksize_bytes != 0) 
    aligned_addr = (adr >> bound_log2(mem_blocksize_bytes)) << bound_log2(mem_blocksize_bytes);

  tx->transactionType = transType; tx->address = aligned_addr; tx->data = 0;
  Queue[slot].prim = (m_queued_items == 0);
  m_queued_items += 1;
  ms->addTransaction(*tx);
  DRTRC(cout << name() << ": blockwhile() current memory system clock is: " 
	<< dec << ms->currentClockCycle << endl);
  Queue[slot].add_clock_cycle = ms->currentClockCycle;

  assert(!m_qlock.lock());
  while(Queue[slot].active)
    {
#if !MANUAL_DELAY
      //delay += clock_period;
      /*DRTRC(cout << name() << ": blockwhile(): adding delay: " << clock_period 
	<< " queued_items = " << m_queued_items << endl);*/
#endif
      if (Queue[slot].prim) update(); // Clock the DRAM subsystem while it is busy.
      //keeper(delay);
    }

  delay += Queue[slot].delay;
  Queue[slot].delay = SC_ZERO_TIME;
  assert(!m_qlock.lock());
  m_queued_items -= 1; 
  if (Queue[slot].prim && m_queued_items >0)
    {
      // Need to pass prim to another thead if we were the holder.
      int slot1 = 0;
      for (slot1 = 0; slot1<MAX_DRAMSIM_THREADS; slot1++) if (Queue[slot1].active) break;
      assert(slot1 < MAX_DRAMSIM_THREADS);
      Queue[slot1].active = true;
    }
  m_qlock.unlock();

  DRTRC(cout << name() << ": blockwhile() end time = " << sc_time_stamp() 
	<< " delay = " << delay << endl);
}

#if 0
void dramsim_sc_wrapper::keeper(sc_time &delay)
{
  if(delay >= local_quantum) // need global q really
    {
      wait(delay);
      delay = SC_ZERO_TIME; // Reset timing annotation after waiting
    }
}
#endif


void dramsim_sc_wrapper::end_of_simulation()
{
#ifdef TLM_POWER3
  energy_accounter(0, 0);
#endif
  ms->printStats(true); // What's  the difference ? final or otherwise?  we
  stat_report("end_of_simulation", stdout, 0);
}

tlm::tlm_sync_enum dramsim_sc_wrapper::nb_transport_fw(PW_TLM_PAYTYPE& trans,
						       tlm::tlm_phase& phase,
						       sc_time& delay) {
  m_peq.notify(trans, phase, delay);
  return tlm::TLM_ACCEPTED;
}

void dramsim_sc_wrapper::peq_cb(PW_TLM_PAYTYPE& trans, const tlm::tlm_phase& ph) {
  sc_time delay = SC_ZERO_TIME;

  switch(ph) {
  case tlm::BEGIN_REQ:
    break;
  default:
    assert(0); // anything else is error (for now)
  }
  
  b_access(trans, delay);

  // send the message back using backwards path
  tlm::tlm_phase bw_phase = tlm::END_REQ;
  port0->nb_transport_bw(trans, bw_phase, delay);
}

void dramsim_sc_wrapper::b_access(PW_TLM_PAYTYPE& trans, sc_time& delay) {

  DRTRC(cout << name() << ": start time = " << sc_time_stamp()
	<< " and delay = " << delay << endl);

  tlm::tlm_command cmd = trans.get_command();
  POWER3(pw_agent_record l_agent = trans.pw_log_hop(this,  (cmd==tlm::TLM_READ_COMMAND ? PW_TGP_DATA: PW_TGP_NOFIELDS) | PW_TGP_ACCT_CKP,  &read_bus_tracker));  // TODO cannot pass in tracker if missing when PAYTYPE<3

  u64_t adr = ((u64_t)trans.get_address()) - mem_base;
  //printf("%s addr= " PFX64 " - " PFX64 " = " PFX64 "  < " PFX64 "\n",  name(), ((u64_t)trans.get_address()), mem_base, adr, mem_size_bytes);
  u8_t* ptr = trans.get_data_ptr();
  u32_t len = trans.get_data_length();
  u8_t* lanes = trans.get_byte_enable_ptr();
  //printf("%s b_access " PFX64 "\n", name(), adr);
#if !MANUAL_DELAY
  //delay += clock_period; // One cycle overhead always added.
  // adding clock boundary crossing between controller and DRAM channel
  // assuming very optimisic implementation where it takes 3 cycles
  // at the source and 5 cycles at the destination (MP - 31 Jul 12)
  delay += 3 * clock_period + 5 * sc_time(tCK, SC_NS);
#endif

  bool not64 = false;
  bool nonaligned = false;
  
  // Obliged to check address range and check for unsupported features, i.e. byte enables, streaming, and bursts

  if (adr >= mem_size_bytes)
      {
          DRTRC(printf("%s: address 0x" PFX64 " out of range : 0x" PFX64 " .. 0x" PFX64 "\n", name(), adr+mem_base, mem_base, mem_base+mem_size_bytes));
	trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
	return;
      }
  
  if (adr & 7) // non-aligned reads not supported.
      {
	// This is a workaround for now when we want to 
	// read 32 bits, at one point we are going to remove
	// this and parameterise creation of dramsim module
	// with the burst size that we want to read. DDR support bursts of 4 or 8 words.
	if((adr & 4) && len == 4) {
	  not64 = true;
	  nonaligned = true;
	  
	  adr &= ~0x7;
	  len = 8;
	}
	else {
	  DRTRC(printf("%s: address 0x" PFX64 " unaligned access\n", name(), adr+mem_base));
	  trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
	  return;
	}

      }
  
  if (len % bl()) // Must be a multiple of DRAMBURST
    {
      // workaround when trying to read 32bit address
      if(len == 4) {
	not64 = true;

	len = 8;
      }
      else {
        DRTRC(printf("%s: Illegal len or wid: len=%i wid=%i (burstlen=%i)\n", name(), len, 64, bl()));
	trans.set_response_status(tlm::TLM_BURST_ERROR_RESPONSE);
	return;
      }
    }
  
  llsc_extension * llsc;
  trans.get_extension(llsc);
  // Obliged to implement read and write commands
  if (cmd == tlm::TLM_READ_COMMAND)
    {
        bool tf = traceregions && traceregions->check(adr, TENOS_TRACE_MEM_READ);
	m_stats.m_read_ops += 1;
#if TRACECOMM
	proc_id_extension *pid = NULL;
	trans.get_extension(pid);
	assert(pid != NULL);
	if(g_tracecomm && pid->proc_id != -1) {
	  m_hit_reads[pid->proc_id] += pid->length;
	}
#endif	
	
      if(tf && llsc) {
	printf("%s,%s: RAM load linked for addr " PFX64 "\n", name(), kind(), adr);
      }

      if (llsc && llsc->loadlocked_fail(&llsc_locks, adr & ~7LLU)) // The load linked phase should never normally fail.
	{
	  //if (adr & 7LLU) printf("%s: Store conditional unaligned 0x" PFX64 "\n", name(), adr);
	  if (tf) printf("%s,%s: Ram load linked read missed at " PFX64 "\n", name(), kind(), adr);
	  trans.set_response_status( tlm::TLM_ADDRESS_ERROR_RESPONSE );
	  return;
	}
      if (dynload_done) blockwhile(DATA_READ, adr, delay);
      m_stats.m_read_bytes += len*8;
      for (int dd=0; dd<len; dd+=8)
	{
	  u64_t temp;
	  bool ok = ip_read(adr+dd, temp);
	  if(!ok)
	    {
	      // memory hasn't been written so return nothing
	      // Obliged to set response status to indicate successful completion
	      trans.set_data_length(0);
	      trans.set_data_ptr(0);
	      trans.set_response_status(tlm::TLM_OK_RESPONSE);
	      POWER3(energy_accounter(&l_agent, &trans, cmd));
	      if (tf) printf("%s: read-64 (unset) at " PFX64 " RD=" PFX64 "\n", name(), temp, adr);
	      temp = 0;//Could put 0xdead_beef here etc...
	    }
	  else
	    {
	      if (tf) printf("%s: %s: Read %s at " PFX64 " " PFX64 "\n", name(), kind(), llsc? "linked":"", adr+dd, temp);
	    }
	  ((u64_t *)ptr)[dd/8] = temp;
	  if ((adr & ~7LLU) == m_watchpoint) 
	    {
	      printf("%s: read-64 (watchpoint) d=0x" PFX64 " at " PFX64 " + %x\n", name(), temp, adr, dd);
	    }
	}
    }
  else if (cmd == tlm::TLM_WRITE_COMMAND)
    {
      bool tf = traceregions && traceregions->check(adr, TENOS_TRACE_MEM_WRITE);
      m_stats.m_write_ops += 1;
      if (llsc)
	{
	  if (adr & 7LLU) printf("%s: Store conditional unaligned at " PFX64 "\n", name(), adr);
	  if (tf) printf("%s: llsc DRAM store conditional write at " PFX64 "\n", name(), adr);
	  if (llsc->storeconditional_fail(&llsc_locks, adr & ~7LLU))
	    {
              if (tf) printf("%s: llsc DRAM store conditional write missed at " PFX64 "\n", name(), adr);
	      trans.set_response_status( tlm::TLM_ADDRESS_ERROR_RESPONSE );
	      return; // Return without doing the store: client will try again.
	    }
	  POWER3(pw_module_base::record_energy_use(llsc_energy_op, &trans));
	}
      else {
          // check if we are writing to a memory location for which we are
          // holding a lock, if we are then we should remove lock

	  // Example of such operation is that if we have two cores C1 and C2 and
	  // C1 does load-linked read for location A and then C2 does a normal write
	  // to a location A then C1's store-conditional should fail.
	  u64_t start_addr = adr;
	  u64_t end_addr = adr + len;
          if(llsc_locks.llsc0.a >= start_addr && llsc_locks.llsc0.a < end_addr) {
              //printf("Writing to region [%lx, %lx) for which there is a lock\n", start_addr, end_addr);
              llsc_locks.llsc0.a = -1;
          }
          else if(llsc_locks.llsc1.a >= start_addr && llsc_locks.llsc1.a < end_addr) {
              //printf("Writing to region [%lx,%lx) for which there is a lock\n", start_addr, end_addr);
              llsc_locks.llsc1.a = -1;
          }
      }

      // TODO: can write post - make a selectable mode
      if(dynload_done) blockwhile(DATA_WRITE, adr, delay);
      int bel = trans.get_byte_enable_length();
      m_stats.m_write_bytes += len*8; // We currently count a byte as written even if not byte enabled.
      for (int dd=0; dd<len; dd+=8)
	{
	  u64_t *tmp = (u64_t *)(ptr);
	  if ((adr+dd & ~7LLU) == m_watchpoint) 
	    {
	      printf("%s: write-64 (watchpoint) d=0x" PFX64 " at " PFX64 " + %x/%x (bel=%x)", name(), tmp[dd/8], adr, dd, len, bel);
	      if (!lanes) printf(" all lanes\n");
	      if (lanes) printf(" lanemask=" PFX64 "\n", ((u64_t *)lanes)[(dd % bel)/8]);
	    }

	  if (!lanes || bel==0 || ((u64_t *)lanes)[(dd % bel)/8] == 0xFFFFffffFFFFffffLLU)
	    {
	      if (tf)
                  printf("%s: %s: Write64 adr=" PFX64 " %02x%02x%02x%02x  %02x%02x%02x%02lx\n", name(), kind(), adr+dd, ptr[(dd/8)+7], ptr[(dd/8)+6], ptr[(dd/8)+5], ptr[(dd/8)+4], ptr[(dd/8)+3], ptr[(dd/8)+2], ptr[(dd/8)+1], tmp[(dd/8)+0]); 
	      //	  printf("%s b_access w64 " PFX64 "\n", name(), adr);
	      ip_write(adr+dd, tmp[dd/8]);
	      //XDRTRC(fprintf(stdout, "%s: llsc wrote ok at " PFX64 " : " PFX64 "\n", name(), adr+dd, tmp[dd/8]));
              if(tf)
                  fprintf(stdout, "%s: wrote at " PFX64 " : " PFX64 "\n", name(), adr+dd, tmp[dd/8]);
	    }
	  else
	    {
	      if (tf)
		{
		  u64_t temp;
		  bool ok = ip_read(adr+dd, temp);

		  printf("%s %s: Write byte(s) adr=" PFX64 "  %02x %02x %02x %02x  %02x %02x %02x %02x lanes=" PFX64 " bel=%i (old val ok=%i data=" PFX64 ")\n", name(), kind(), adr+dd, ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7], ((u64_t*)lanes)[0], bel, ok, temp);
		}
	      // We put data in the RAM model in the endianness of the modelling workstation (little for x86).
	      // This requires a swap in the processor model (not here) when the workstation and the system being modelled are different.
	      DRTRC(fprintf(stdout, "%s: wrote at 0x%lx: ", name(), adr));
	      for (int l=0;l<8;l++) 
		if (lanes[(dd+l)%bel]) {
		  ip_write(adr+l+dd, ptr[l+dd]); 
		  DRTRC(fprintf(stdout, "%02x", ptr[l+dd])); 
		}
	      
	    }
	  
	}
    }
  
#if MANUAL_DELAY
  delay += manual_delay * clock_period;
#else
  // crossing from DRAM channel to memory controller
  delay += 3 * sc_time(tCK, SC_NS) + 5 * clock_period;
#endif

  // Set DMI hint to indicated whether DMI is encouraged.  
  if (dmi_on) trans.set_dmi_allowed(dmi_on);

  // if read was 32bit we need to select correct bits
  if(not64) {
    u64_t tmp = *(u64_t*)ptr;
    if(nonaligned)
      tmp >>= 32;
    else
      tmp = (tmp << 32) >> 32;
    memcpy(ptr, (u8_t*)(&tmp), 8);
  }

  // Obliged to set response status to indicate successful completion
  trans.set_response_status(tlm::TLM_OK_RESPONSE);

  m_stats.m_row_activations = ms->memoryController->activations; // upcopy the activation count. - reset of this metric wont work correctly but we will use checkpointing instead of resets going forward.
  //printf ("upcopy  adr=" PFX64 " activations=%llu \n", adr, m_stats.m_row_activations );
  POWER3(energy_accounter(&l_agent, &trans, cmd));

  DRTRC(cout << name() << ": end time = " << sc_time_stamp()
	     << " and delay = " << delay);
}


bool dramsim_sc_wrapper::get_direct_mem_ptr(PW_TLM_PAYTYPE&, tlm::tlm_dmi &dmi_data)
{
  if (last_page)
    {
      dmi_data.allow_read_write();
      dmi_data.set_start_address(last_page->base);
      dmi_data.set_end_address(last_page->base + INTERNALPAGESIZE/8LLU - 1);
      dmi_data.set_dmi_ptr(reinterpret_cast<unsigned char *>(last_page->data));

      dmi_data.set_read_latency( sc_time(10, SC_NS) ); // latency per byte TODO and only once !
      dmi_data.set_write_latency( sc_time(10, SC_NS) ); // RANDOM AND WRONG VALUE - DMI CANNOT REALLY MANAGE DRAM's complex details.
    }
  return true;
}


void dramsim_sc_wrapper::stat_report(const char *msg, FILE *fd, bool resetf)
{
  if (fd) fprintf(fd, "%s:%s DRAM %i MByte stats: read_ops=" PFI64 ", write_ops=" PFI64 ", read_bytes=" PFI64 ", write_bytes=" PFI64 " row_activations=" PFI64 "\n", name(), msg, 
		  ini->megsOfMemory,
		  m_stats.m_read_ops, m_stats.m_write_ops, m_stats.m_read_bytes, m_stats.m_write_bytes, m_stats.m_row_activations);
  assert(ms);
  ms->printStats(false); // What's  the difference ? final or otherwise?  we need a reset api on this.
  //  ms->printStats(true);
  if (resetf) ms->memoryController->resetStats();
  if (resetf) m_stats.reset();
}


bool dramsim_sc_wrapper::ip_clear_(u64_t a)
{
  u64_t a0 = a & ~0xFFF;
  u32_t a1 = a & 0xFFF;
  internal_page *x = memory_hash_map[a0];
  last_page = x;
  bool ov = (x != 0);
  return ov;
}

bool dramsim_sc_wrapper::ip_write(u64_t a, u64_t d) // 64 bit write (fast path).
{
  u64_t a0 = a & ~0xFFF;
  u32_t a1 = a & 0xFFF;
  assert(!(a1 & 7)); // check aligned access
  internal_page *x = memory_hash_map[a0];
  bool ov = (x!=0);
  if (!x)
    {
      x = new internal_page(a0);
      memory_hash_map[a0] = x;
      DRTRC(printf("Memory ip_write to 0x" PFX64 ", \n", mem_base + a0));
    }
  u64_t *ap = (u64_t *) (&(x->data[a1 & ~7]));
  *ap = d;
  last_page = x;
  DRTRC(printf("%s: Write64 " PFX64 " to " PFX64 "\n", name(), d, a));
  return true;
}

bool dramsim_sc_wrapper::ip_write(u64_t a, u8_t d, bool es)
{
  u64_t a0 = a & ~0xFFF;
  u32_t a1 = a & 0xFFF;
  internal_page *x = memory_hash_map[a0];
  if (!x)
    {
      x = new internal_page(a0);
      memory_hash_map[a0] = x;
      DRTRC(printf("\n Memory ip_write a0== " PFX64 ", x== %p", a0, x));
    }
  x -> data[a1 ^ (es ? 7:0)] = d;
  last_page = x;
  DRTRC(printf("%s: Write8 " PFX64 " to %x\n", name(), d, a));
  return true;
}



bool dramsim_sc_wrapper::ip_read(u64_t a, u8_t &d)
{
  u64_t a0 = a & ~0xFFF;
  u32_t a1 = a & 0xFFF;
  internal_page *x = memory_hash_map[a0];
  last_page = x;
  d = x -> data[a1 ^ (0 /*AFLIP*/) ];
  DRTRC(printf("%s: Read8 %x from " PFX64 "\n", name(), d, a));
  return true;
}

bool dramsim_sc_wrapper::ip_read(u64_t a, u64_t &d) // 64 bit read (fast path).
{
  u64_t a0 = a & ~0xFFF;
  u32_t a1 = a & 0xFFF;
  assert(!(a1 & 7)); // check aligned access
  internal_page *x = memory_hash_map[a0];
  bool ov = (x!=0);
  if (!ov)
    {
      DRTRC(printf("%s: Read before write to " PFX64 "\n", name(), a));
      //assert(x);
      return false;
    }
  d = ((u64_t *)(x -> data + (a1 & ~7)))[0];
  
  last_page = x;
  DRTRC(printf("%s: Read64 " PFX64 " from " PFX64 "\n", name(), d, a));
  return true;
}

TENOS_KIND_DEFINITION(dramsim_sc_wrapper)


//
//
void dramsim_ini_t::set_as_an_example(int no, int sizeInMegs)
{
  deviceIniFilename = "../dramsim2/dist/system.ini.example";
  //systemIniFilename = "../dramsim2/dist/ini/DDR2_micron_16M_8b_x8_sg3E.ini";
  systemIniFilename = "../dramsim2/dist/ini/DDR3_micron_32M_8B_x4_sg125.ini";
  pwdString = "";
  traceFileName = "tracefile";
  megsOfMemory = sizeInMegs;
}



#if 0

//
// A unit test.
//
void dramsim_sc_wrapper::tmpmain()
{
  bool q;
  char buf0[4096];
  char buf1[4096];
  char buf2[4096];
  for (int i=0; i<4096; i++) buf0[i] = i;
  for (int i=0; i<4096; i++) buf1[i] = i | 4;
  DRTRC(cout << " buf addr " << &buf0 << " " << &buf1 << "\n");
  Transaction *trans0 = new Transaction(DATA_WRITE, 0x1234000, &buf0);
  ms->addTransaction(*trans0);
  Transaction *trans = new Transaction(DATA_WRITE, 0x1235000, &buf1);
  DRTRC(cout << " trans addr " << trans0 << " " << trans << "\n");
  ms->addTransaction(*trans);
  trans->print();
  for (int i =0; i<2000; i++) update();
  for (int i=0; i<4096; i++) buf2[i] = 0;  
  trans = new Transaction(DATA_READ, 0x1234000, &buf2);
  q = ms->addTransaction(*trans);
  assert(q);
  trans = new Transaction(DATA_READ, 0x1234020, &buf2);
  ms->addTransaction(*trans);
  trans = new Transaction(DATA_READ, 0x1334000, &buf2);
  ms->addTransaction(*trans);
  trans->print();
  for (int i =0; i<1000; i++) update();
  for (int i=0; i<4096; i++) printf("%02X ", buf2[i]);
}

int main()
{
  dramsim_ini_t *ini = new dramsim_ini_t;
  ini->set_as_an_example(0);
  dramsim_sc_wrapper k("testname", 0, ini);
  k.tmpmain();
  DRTRC(cout << "ENDDDD\n");
  return 0;
 
}
#endif


// eof
