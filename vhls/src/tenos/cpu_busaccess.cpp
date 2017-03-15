// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>

//
// A generic bus access unit for processor cores.  This goes between the ISS/etc and the L1 caches.
//
//

#include "cpu_busaccess.h"
#if TRACECOMM
#include "proc_id_extension.h"
#endif


/*
  How to do fork/join in loosely-timed TLM modelling.  Without fork join we put work in series
as follows have:

   while(1)
   {   ... other work
       delay = m_qk.get_local_time() ;
       socket->b_transport( trans, delay );
       m_qk.set( delay );
       ... other work

       m_qk.inc( sc_time(100, SC_NS) ); // Local work penalty.
       if ( m_qk.need_sync() )   m_qk.sync();
   }

The modification for parallel work is
   while(1)
   {   ... other work
       delay1 = m_qk.get_local_time() ;
       delay2 = m_qk.get_local_time() ;

       socket->b_transport( trans1, delay1 );
       socket->b_transport( trans2, delay2 );

       final_delay = MAX2(delay1, delay2);

       m_qk.set( final_delay );
       ... other work

       m_qk.inc( sc_time(100, SC_NS) ); // Local work serial penalty.
       if ( m_qk.need_sync() )   m_qk.sync();
   }

  but this will fail if, say, trans2 has done a resynch - advancing kernel time, since delay1 and delay2 will no longer be related to the same basis.

  The correct coding is 
   while(1)
   {   ... other work

       start_g_time = sc_time_stamp()

       delay1 = m_qk.get_local_time() ;
       delay2 = m_qk.get_local_time() ;

       socket->b_transport( trans1, delay1 );
       socket->b_transport( trans2, delay2 );

       end_g_time = sc_time_stamp();  

       if (start_g_time != end_g_time)   final_delay = 0; // detect aux resynch - if so then perhaps multiple - we have lost the info we need?

       else final_delay = MAX2(delay1, delay2) - gen_g_time; 

       

       m_qk.set( final_delay );
       ... other work

       m_qk.inc( sc_time(100, SC_NS) ); // Local work penalty.
       if ( m_qk.need_sync() )   m_qk.sync();
   }


  Over some unit of execution, which for our x86 model can be a basic block, or at least a number of uops, we can simulate some parallel and  out-of-order operand processing as follows:

  The main ISS thread has a quantum keeper whose value is incremented at the end of the unit of execution by a time estimate for that unit.  When the QK wishes to, it can resynch with SystemC kernel time.

  The instruction fetch, operand load/stores and coprocessor read/writes also take a &lt_delay argument.  Other bus, cache and I/O blocks may tacitly do a resynch on that variable.

  We wish to log an overall penalty for the longest operation in the parallel components of our unit of execution. For each parallel component we must subtract its start and end times, which are its initial value plus the initial kernel time and its final value plus the final kernel time.   Moreover, the initial value of will be the local run ahead of the ISS at the fork time.



 */
#define ATOMICS_TRC(X)

void cpu_busaccess::trace100(const char *msg)
{
  printf("Trace100\n");
  int count = 1000;
  traceregion *itrace = new traceregion(0, (u64_t)-1, count, TENOS_TRACE_IO_READ | TENOS_TRACE_IO_WRITE | TENOS_TRACE_CPU_READ | TENOS_TRACE_CPU_WRITE | TENOS_TRACE_CPU_KEY_REGS |  TENOS_TRACE_CPU_IFETCH | TENOS_TRACE_BUS_OP);
  itrace->add(traceregions);
}


void cpu_busaccess::tracebuffer_s::report_trace(FILE *fd, const char *names)
{
  for (int i=1; i<=TRACEBUFFER_LENGTH; i++)
    {
      int p = in_ptr - i;
      if (p < 0 && hwm != TRACEBUFFER_LENGTH-1) break;
      if (p < 0) p += TRACEBUFFER_LENGTH;
      fprintf(fd, " %s %i pc was %lx\n", names, i, buffer[p]);
    }
  fprintf(fd, " tracebuffer ...\n");
}

cpu_busaccess * g_it = 0; // temporary backdoor for tracing. perhaps delete.


void tmp_trace_bd(const char *msg)  // A backdoor where a uart print (or other such configuration) causes a trace.
{
  FILE *fd = stdout;
  fprintf(fd, "tmp_trace_bd %s %p\n", msg, g_it);
  if (g_it) g_it->tracebuffer.report_trace(fd, msg);
}

// Instruction fetch may go to its own cache in Harvard mode.
bool cpu_busaccess::instr_fetch64(addr_t memaddr, u64_t &rdata, breakpt_t *bp, sc_time &delay)
{
  lastfetch = memaddr;
  bool tf = traceregions && traceregions->check(memaddr, TENOS_TRACE_CPU_READ);
  if (0)
    {
      static sc_time last = SC_ZERO_TIME;
      std::cout << sc_time_stamp() << "  " << sc_time_stamp()-last; printf(" Ffetch a=" PFX64 "\n", memaddr);
      last = sc_time_stamp();
    }

  if (0)
    {
      g_it = this;
      tracebuffer.buffer[tracebuffer.in_ptr] = memaddr;
      tracebuffer.inc();
    }
  if (memaddr == 0xFFFFfffc) sim_done("Branched to PC==0xFFFFfffc"); // Zero was the arm reset vector so use this address now.
  bool ok = mem_read(memaddr, rdata, delay, 0, 1, tf);
  if (tf) printf("%s, wrapper: ok=%i ifetch at " PFX64 " rd=" PFX64 "\n", name(), ok, memaddr, rdata);
  return ok;
}



bool cpu_busaccess::eval_mem64(addr_t memaddr, u64_t &rdata, breakpt_t *bp, sc_time &delay, bool lock)
{
  bool ok = false;
  bool tf = traceregions && traceregions->check(memaddr, TENOS_TRACE_CPU_READ);
  if (atomics)
    {
      if ((memaddr & ~7LLU) == atomic_addr) // if an atomic exchange is being synthesised, the write info is already stored.
	{
	  assert(atomics == 1);
	  assert(64 == atomic_width);
	  atomics = 0;
	  int ok_eval_xchg = eval_xchg(memaddr, atomic_wdata, rdata, 64, delay, tf);
          switch(ok_eval_xchg) {
            case 0:
              ok = false;
              break;
            case 1:
              ok = true;
              break;
            case 2:
              ok = false;
              break;
            default:
              ok = false;
              break;
          }

	  ATOMICS_TRC(printf("%s, ok=%i atomic xchg data64 adr=" PFX64 " wd=" PFX64 " rd=" PFX64 "\n", name(), ok, memaddr, atomic_wdata, rdata));
	  if (tf) printf("%s, ok=%i xchg data64 adr=" PFX64 " wd=" PFX64 " rd=" PFX64 "\n", name(), ok, memaddr, atomic_wdata, rdata);
	}
      ATOMICS_TRC(else printf("atomics: strange: second part of xchg address is different " PFX64  "\n", memaddr));
    }
  else
    {
      ok = mem_read(memaddr, rdata, delay, lock, 0, tf, 3, 8);
      if (tf) printf("%s, ok=%i Read data64 at " PFX64 " rd=" PFX64 "\n", name(), ok, memaddr, rdata);
    }    

  return ok;
}



bool cpu_busaccess::eval_mem32(addr_t memaddr, u32_t &r, breakpt_t *bp, sc_time &delay, bool linked)
{
  u64_t rdata;
  bool ok, tf;
  u32_t d = 0;
  bool uc = UNCACHED_ADDRESS_SPACE64(memaddr); // For now - if an I/O device then use a 32 bit load to avoid upsetting neighbouring I/O read-sensitive locations.
  tf = traceregions && traceregions->check(memaddr, TENOS_TRACE_CPU_READ);
  if (atomics && (memaddr & ~7LLU) == atomic_addr) // if an atomic exchange is being synthesised, the write info is already stored.
    {
      assert(atomics == 1);
      assert(32 == atomic_width);
      atomics = 0;
      int ok_eval_xchg = eval_xchg(memaddr, atomic_wdata, rdata, 32, delay, tf);
      switch(ok_eval_xchg) {
        case 0:
          ok = false;
          break;
        case 1:
          ok = true;
          break;
        case 2:
          ok = false;
          break;
        default:
          ok = false;
          break;
      }
      d = rdata;
      ATOMICS_TRC(printf("%s: atomic Xchg dat32  at " PFX64 " wdata=" PFX64 " rdata=" PFX64 " ans=%x\n", name(), memaddr, atomic_wdata, rdata, d));
      if (tf)
	printf("%s: Xchg dat32  at " PFX64 " wdata=" PFX64 " rdata=" PFX64 " ans=%x\n", name(), memaddr, atomic_wdata, rdata, d);
    }
  else if (uc)
    {
      int not64 = 2; // Set not64 to 2 for a 32 bit read. - This is used to avoid reading volatile registers in the adjacent int32.
      u64_t bus_address = (memaddr & ~3LLU) ^ (ENDIAN_FLIP & 4);
      ok = mem_read(bus_address, rdata, delay, linked, 0, tf, not64, 4);
      d = rdata;
    }
  else
    {
      u64_t bus_address = memaddr & ~7LLU;
      ok = mem_read(bus_address, rdata, delay, linked, 0, tf, 3, 4);
      d = rdata >> (8 *((ENDIAN_FLIP ^ memaddr) & 4));
    }
  if (ok)
    {
      if (tf) printf("%s: Read dat32 at " PFX64 " rdata=" PFX64 " ans=%x\n", name(), memaddr, rdata, d);
      r = d;
    }    
  return ok;
}



bool cpu_busaccess::eval_mem16(addr_t memaddr, u16_t &d, breakpt_t *bp, sc_time &delay, bool linkedf)
{
  u64_t rdata;
  bool tf = traceregions && traceregions->check(memaddr, TENOS_TRACE_CPU_READ);
  // check to see if we are on the word boundary and need a split load.
  if((((memaddr & ~7LLU)+8) - memaddr) == 1) {
    u64_t rdata1;
    u64_t rdata2;

    bool ok = mem_read(memaddr & ~7LLU, rdata1, delay, linkedf, 0, tf, 3, 2);
    if(!ok) return false;
    ok = mem_read((memaddr & ~7LLU) + 8, rdata2, delay, linkedf, 0, tf, 3, 2);
    if(!ok) return false;

    // Optional sign extension should be in individual cores ISS code.
#ifndef BIG_ENDIAN // TODO ... DO NOT USE GLOBAL FLAGS NOW WE SUPPORT HETEROGENOUS COMPILATIONS
    rdata1 = rdata1 >> 56;
    rdata2 = rdata1 << 56 >> 56;
#else
    rdata1 = rdata1 << 56 >> 56;
    rdata2 = rdata2 >> 56;
#endif
    u16_t k = rdata1 << 8 | rdata2;
    if (tf) printf("Short load memaddr=" PFX64 " rdata=" PFX64 " %d\n", memaddr, rdata, k);
    d = k;
    return ok;
  }
  else {
    bool ok = mem_read(memaddr & ~7LLU, rdata, delay, linkedf, 0, tf, 3, 2);
    if (ok)
    {
      uint64_t shift = ((memaddr ^ ~ENDIAN_FLIP) & 7LLU) - (~ENDIAN_FLIP & 1);
      //uint16_t r = rdata >> ((8*(((memaddr & ~7LLU)+8) - memaddr)) - 16);
      uint16_t r = (rdata << (shift*8)) >> 48; // 64 - 16
      if (traceregions && traceregions->check(memaddr, TENOS_TRACE_CPU_READ)) printf("Short load " PFX64 " " PFX64 "%d\n", memaddr, rdata, r);
      d = r;
    }
    return ok;
  }
}    



bool cpu_busaccess::eval_mem8(addr_t memaddr, u8_t &d, breakpt_t *bp, sc_time &delay, bool linkedf)
{
  u64_t r0;
  bool tf = traceregions && traceregions->check(memaddr, TENOS_TRACE_CPU_READ);
  bool uc = UNCACHED_ADDRESS_SPACE64(memaddr); // For now - if an I/O device then use a 32 bit load to avoid upsetting neighbouring I/O read-sensitive locations.

  if (uc)
    {
      int not64 = 2; // Do a 32-bit load and then adjust for byte here. 
      u64_t bus_address = (memaddr & ~3LLU) ^ (ENDIAN_FLIP & 4);
      bool ok = mem_read(bus_address, r0, delay, linkedf, 0, tf, not64, 1);
      if (ok)
	{
	  // For a big-endian CPU, modelled on x86, ENDIAN_FLIP will be 7, meaning byte zero is at the top and needs the most shifting down. You might think the endianness would not effect byte loads and stores, and on a little-endian CPU like the x86 to get byte zero we should just mask and not shift, but this is wrong. The data has been installed in RAM from elf or other stores with the byte lanes within a 64 bit word permuted.
	  uint8_t rdata =  r0 >> (8*((ENDIAN_FLIP ^ memaddr) & 3)); 
	  if (tf)
	    printf("%s:IO  Byte load at " PFX64 " r0=" PFX64 " rdata=0x%02X %c\n", name(), memaddr, r0, rdata, isprint(rdata)? rdata:' ');
	  
	  // printf("%s: DB %p Byte load at " PFX64 " r0=" PFX64 " rdata=0x%02X %c\n", name(), traceregions, memaddr, r0, rdata, isprint(rdata)? rdata:' ');
	  d = rdata;
	}
      return ok;
    }    
  else
   {
      bool ok  = mem_read(memaddr & ~7LLU, r0, delay, linkedf, 0, tf, 3, 1);
      if (ok)
	{
	  // For a big-endian CPU, modelled on x86, ENDIAN_FLIP will be 7, meaning byte zero is at the top and needs the most shifting down. You might think the endianness would not effect byte loads and stores, and on a little-endian CPU like the x86 to get byte zero we should just mask and not shift, but this is wrong. The data has been installed in RAM from elf or other stores with the byte lanes within a 64 bit word permuted.
	  uint8_t rdata =  r0 >> (8*((ENDIAN_FLIP ^ memaddr) & 7llu)); 
	  if (tf)
	    printf("%s:Q  Byte load at " PFX64 " r0=" PFX64 " rdata=0x%02X %c\n", name(), memaddr, r0, rdata, isprint(rdata)? rdata:' ');
	  
	  // printf("%s: DB %p Byte load at " PFX64 " r0=" PFX64 " rdata=0x%02X %c\n", name(), traceregions, memaddr, r0, rdata, isprint(rdata)? rdata:' ');
	  d = rdata;
	}
      return ok;
    }    
}


void cpu_busaccess::atomic_prefix() // Prefix following load/store pair as atomic: makes them into an exchange instruction.
{
  if (traceregions && traceregions->check(0, TENOS_TRACE_CPU_WRITE))
    printf("%s:%s: atomic prefix\n", name(), kind());
  ATOMICS_TRC(printf("%s:%s: atomic prefix\n", name(), kind()));
  atomics = 2; // Trigger atomic transaction state machine: nb there may be instr fetches in the meantime.
}



bool cpu_busaccess::set_mem64(addr_t memaddr, u64_t wdata, breakpt_t *bp, sc_time &delay, bool mamba_swp)
{
  assert(!mamba_swp);
  if (atomics) 
    {
      assert(atomics==2);
      atomics -= 1;
      atomic_addr = memaddr & ~7LLU;
      atomic_wdata = wdata;
      atomic_width = 64;
      ATOMICS_TRC(printf("%s:%s: atomic store save a=%lx d64=%lx\n", name(), kind(), memaddr, wdata));
    }
  else
    {
      if (traceregions && traceregions->check(memaddr, TENOS_TRACE_CPU_WRITE)) printf("Int64 store at " PFX64 " wd=" PFX64 "\n", memaddr, wdata);
      return mem_write(memaddr,  0/*means 64 bits*/,  wdata, delay);
    }

  return true;
}




bool cpu_busaccess::set_mem32(addr_t memaddr, u32_t wdata, breakpt_t *bp, sc_time &delay)
{
  bool tf = traceregions && traceregions->check(memaddr, TENOS_TRACE_CPU_WRITE);
  if (atomics) 
    {
      assert(atomics==2);
      if (tf) printf("%s: Int32 llsc prestore at " PFX64 " wd=%x\n", name(), memaddr, wdata);
      //printf("sw 1a %i\n", atomics);
      atomics -= 1;
      atomic_addr = memaddr & ~7LLU; // Only supports u64 aligment. 
      atomic_wdata = wdata;
      atomic_width = 32;
      ATOMICS_TRC(printf("%s:%s: atomic store save a=%lx d32=%lx\n", name(), kind(), memaddr, wdata));
    }
  else
    {
      if (tf) printf("%s: Int32 store at " PFX64 " wd=%x\n", name(), memaddr, wdata);
      // If bit2 of address set then shift mask to top four bytes because the modelling
      // workstation (x86) is little endian (irrespective for modelled architecture (which might involve both)). But this is swapped again in mem_write!
      return mem_write(memaddr&~4, 15 << (( memaddr) & 4), ((u64_t)wdata << 32)|(wdata << 0), delay);
    }

  return true;
}



bool cpu_busaccess::set_mem16(addr_t memaddr, u16_t wdata, breakpt_t *bp, sc_time &delay)
{
  if (traceregions && traceregions->check(memaddr, TENOS_TRACE_CPU_WRITE)) printf("%s: Short store at " PFX64 " wd=%x\n", name(), memaddr, wdata);
  u64_t wd = ((u64_t)wdata << 16)|((u64_t)wdata << 0);
  wd |= wd << 32;
  return mem_write(memaddr&~6,  3 << ((memaddr) & 6), wd, delay);
}    


bool cpu_busaccess::set_mem8 (addr_t memaddr, uint8_t wdata, breakpt_t *bp, sc_time &delay)
{
  u64_t wd = (wdata << 8)|(wdata << 0);
  wd |= wd << 16;
  wd |= wd << 32;
  if (traceregions && traceregions->check(memaddr, TENOS_TRACE_CPU_WRITE)) printf("%s: Byte store at " PFX64 " wd=%x  %c\n", name(), memaddr, wdata, isprint(wdata) ? wdata:' ');
  return mem_write(memaddr&~7,  1 << ((memaddr) & 7), wd, delay);
}    

// little debug code...
//static int stat_c = 0;
//static pw_energy stat_e = PW_ZERO_ENERGY;


// rc=true if all ok
// Perform TLM read transaction on the main socket (excludes I-fetch if that is using its own socket).
// This can be re-entrant but separate DMI records are kept to stop crosstalk between I and D when separate.
// Set not64 to 2 for 32bit reads. We need this for reading some 32-bit I/O devices where me must not read the neighbouring register by accident.
bool cpu_busaccess::mem_read(addr_t memaddr, u64_t &rdata, sc_time &delay, bool linked, int skt, bool trace_flag, int not64, int size)
{
  assert(skt >= 0 && skt < NO_CPU_BUSACCESS_SKTS);
  //if (tf) printf("%s: Read mem linked=%i skt=%i at " PFX64 "\n", name(), linked, skt, memaddr);
  stats.reads += 1;
#ifdef CYCLE_ACCURATE
  if (ea_mon) ea_mon->write(memaddr);
#endif

  bool measure_needed = DMI_records[skt]->read_latency.measurement_needed();// this needs be needed disjunction over all
#ifdef TLM_POWER3
  measure_needed |= DMI_records[skt]->read_energy.measurement_needed();// this needs be needed disjunction over all
#endif

  unsigned char* dmi_ptr = 0;
#ifdef DMI_STATS
  
  if (measure_needed) DMI_records[skt]->dmi_stats.measure_needed += 1;
  else if (linked) DMI_records[skt]->dmi_stats.linked += 1;
  else if (!DMI_records[skt]->data_valid) DMI_records[skt]->dmi_stats.not_valid += 1;
  else if (!(memaddr >= DMI_records[skt]->data.get_start_address())) 
    {
      //printf("%s dmi[%i] too low   " PFX64 "    " PFX64 " .. "  PFX64 "\n", name(), skt, memaddr,  DMI_records[skt]->data.get_start_address(), DMI_records[skt]->data.get_end_address()); 
      DMI_records[skt]->dmi_stats.too_low += 1;
    }
  else if (!(memaddr <= DMI_records[skt]->data.get_end_address() -8LLU)) 
    {
      //      printf("%s dmi[%i] too high ip  " PFX64 "    " PFX64 " .. "  PFX64 "\n", name(), skt, memaddr,  DMI_records[skt]->data.get_start_address(), DMI_records[skt]->data.get_end_address()); 
      DMI_records[skt]->dmi_stats.too_high_margin += (memaddr - (DMI_records[skt]->data.get_end_address() -8LLU)); 
      DMI_records[skt]->dmi_stats.too_high += 1;
    }
  else DMI_records[skt]->dmi_stats.aok += 1;
#endif

  if (!measure_needed && !linked && DMI_records[skt]->data_valid && memaddr >= DMI_records[skt]->data.get_start_address()  && memaddr <= DMI_records[skt]->data.get_end_address() -8LLU)
    {
      dmi_ptr = DMI_records[skt]->data.get_dmi_ptr();
    }

  if (dmi_ptr)
    {
      u64_t *bo = ((u64_t *)(dmi_ptr));
      //      BUSACCESS_MEMTRC(for (int i =0; i<32; i++) { if ((i&3)==0) printf("%04X: ", i*8); printf("%016llx ", bo[i]); if ((i&3)==3) printf("\n"); })
      rdata = bo [ ((memaddr - DMI_records[skt]->data.get_start_address()) >> 3LLU) ];

      if (trace_flag) printf("DMI - Can get " PFX64 " from %p %llx to %llx at offset %llx  rdata=%lx\n", memaddr, dmi_ptr, DMI_records[skt]->data.get_start_address(), DMI_records[skt]->data.get_end_address(), memaddr - DMI_records[skt]->data.get_start_address(), rdata);

      delay += DMI_records[skt]->read_latency.provide_estimate();
      //TODO scale by length in bytes

      stats.dmi_reads += 1;
      POWER3(pw_energy ee = DMI_records[skt]->read_energy.provide_estimate());
      //std::cout << "DMI estimates (not scaled) " << ee << " e=" << DMI_records[skt]->read_energy.provide_estimate() << "\n";

      //TODO scale by length in bytes
      //stat_e += ee;
      //stat_c += 1;
      //if ((stat_c & 0xFFFFF)==0) std::cout << "stat_e is now " << stat_e << "\n";

      POWER3(record_energy_use(ee)); // should default to dynamic. DMI-1A - log dmi power on originator.
      return true;
    }

  PW_TLM_PAYTYPE  *trans, nt;

  if (!linked)
    {
      trans = &nt; // Use stack allocated payload when no extensions needed.
      trans->set_dmi_allowed( false ); // We set this to false and it is set true by the target if we can later ask for a DMI ptr.?
    }
  else
    {
      trans = llsc_payload_alloc();
      trans->set_dmi_allowed( false ); // We set this to false and it is set true by the target if we can later ask for a DMI ptr.?
      trans->acquire();
      trans->set_dmi_allowed( false );
      llsc_extension *llsc;
      trans->get_extension(llsc);
      assert(llsc);
      llsc->id = I2V(procID);
    }

  PW_TLM3(trans->set_customer_acct(*Customer_ids[current_alias]));

 //  printf("Ref count %i  %i %i\n", linked, trans->has_mm(), trans->get_ref_count());
  trans->set_byte_enable_length(0); // No byte lanes used on a read - defaults to 64 bits but different data_length fields can also be used for some targets.
  trans->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );
  trans->set_read();
  trans->set_address( (sc_dt::uint64) memaddr);
  trans->set_data_length(not64 != 3 ? 4: sizeof(rdata)); // 8 bytes for size 3, 4 bytes for size 2.
  trans->set_data_ptr( (unsigned char *) &rdata);

  // This is not used, so when tracing communication
  // we use it to indicate size of data
#if TRACECOMM
  proc_id_extension* pid = new proc_id_extension;
  pid->proc_id = procID;
  pid->length = size;
  trans->set_extension(pid);
  trans->set_streaming_width( size );
#else
  trans->set_streaming_width(8);
#endif  

  // We want to maintain two DMI records even if non-harvard and sharing one socket or L1 cache.
  int out_skt = (harvardf) ? skt: 0; // 

  POWER3(PW_TLM3(trans->pw_set_origin(m_module,  //read
				      PW_TGP_ADDRESS |
				      PW_TGP_ACCT_SRC |
				      ((false)?0:PW_TGP_DATA|PW_TGP_LANES), 
				      out_skt ? &bus_tracker1: &bus_tracker0
				      )));

  sc_time start_time = sc_time_stamp() + delay;
  if (out_skt > 0) { (*ifetch_socket)->b_transport(*trans, delay); }
  else initiator_socket->b_transport(*trans, delay);

  //if (tf) printf("%s: %s mem at " PFX64 " done RD = " PFX64 " ... \n", name(), linked? "load-linked" : "load", memaddr, rdata);

#if TRACECOMM
  trans->clear_extension(pid);
  delete(pid);
#endif  
  
  if (trans->is_response_error())
    {
      //char txt[1024];
      //snprintf(txt, 1024, "Error from b_mem_access, mem_read, at " PFX64 " response status=%s lastfetchPC=" PFX64, memaddr, trans->get_response_string().c_str(), lastfetch);
      //SC_REPORT_ERROR("busaccess", txt);
      if (linked)   trans->release();
      if (linked) payload_mm.free(trans);

      return false;
    } 

  sc_time latency = sc_time_stamp() + delay - start_time;

  DMI_records[skt]->read_latency.record_measurement(latency);
  if (!disable_dmi /* && !DMI_records[skt]->data_valid */&& trans->is_dmi_allowed()) 
    {
      // See: http://blogtlm.mariusmonton.com/en/2009/03/dmi-example
      DMI_records[skt]->data_valid = initiator_socket->get_direct_mem_ptr(*trans, DMI_records[skt]->data);

      //std::cout <<  "DMI e meas=" << trans->get_dmi_energy_record() << " logged\n";
      POWER3(PW_TLM3(DMI_records[skt]->read_energy.record_measurement(trans->get_dmi_energy_record()))); // PUTS IN READ DYN ACCT BY DEF. TODO.
    }
  POWER3(PW_TLM3(trans->pw_terminus(m_module)));

  if (linked) payload_mm.free(trans);
  if (trace_fd)
    {
      fprintf(trace_fd, "%s at " PFX64 " D=" PFX64 "\n", (linked? "RL": "R"), memaddr, rdata);
    }
  return true;
}


u8_t mask_left[8]  = { 0xFF, 0xFF, 0xFF, 0xFF,    0, 0, 0, 0 };
u8_t mask_right[8] = { 0, 0, 0, 0,                0xFF, 0xFF, 0xFF, 0xFF };

struct byte_mask_s { u8_t a,b,c,d,e,f,g,h; };

struct byte_mask_s short_write_masks[4]  =
  {
    { 0xFF, 0xFF, 0, 0,    0, 0, 0, 0 },
    { 0, 0, 0xFF, 0xFF,    0, 0, 0, 0 },
    { 0, 0, 0, 0,          0xFF, 0xFF, 0, 0 },
    { 0, 0, 0, 0,          0, 0, 0xFF, 0xFF }
};

struct byte_mask_s byte_write_masks[8]  =
  {
    { 0xFF, 0, 0, 0,    0, 0, 0, 0 },
    { 0, 0xFF, 0, 0,    0, 0, 0, 0 },
    { 0, 0, 0xFF, 0,    0, 0, 0, 0 },
    { 0, 0, 0, 0xFF,    0, 0, 0, 0 },
    { 0, 0, 0, 0,       0xFF, 0, 0, 0 },
    { 0, 0, 0, 0,       0, 0xFF, 0, 0 },
    { 0, 0, 0, 0,       0, 0, 0xFF, 0 },
    { 0, 0, 0, 0,       0, 0, 0, 0xFF }
};




// A 1, indicating success, is written into GPR rt.  Otherwise, memory is not modified and a 0, indicating failure, is written into GPR rt.
int cpu_busaccess::cpu_mem_store_conditional(addr_t memaddr, u64_t value, int width, sc_time &delay)
{
  stats.writes += 1;
#ifdef CYCLE_ACCURATE
  if (ea_mon) ea_mon->write(memaddr);
#endif
  PW_TLM_PAYTYPE *trans = llsc_payload_alloc();
  PW_TLM3(trans->set_customer_acct(*Customer_ids[current_alias]));
  trans->acquire();
  trans->set_write();
  trans->set_address( (sc_dt::uint64) memaddr & ~7); // For all widths we do a word64 aligned operation.
  trans->set_data_length(8);
  trans->set_streaming_width( 8 );
  u64_t wdata;
  if (width == 64)
    {
      trans->set_byte_enable_ptr(0);
      wdata = value;
    }
  else
    {
      trans->set_byte_enable_length(8);
      switch (width)
	{
	case 8:
	  value &= 0xff;
	  wdata = (value<<0LLU) | (value << 8LLU);
	  wdata = (wdata<<0LLU) | (wdata << 16LLU);
	  wdata = (wdata<<0LLU) | (wdata << 32LLU);
	  trans->set_byte_enable_ptr(&byte_write_masks[(memaddr ^ ENDIAN_FLIP) & 7].a);
	  break;

	case 16:
	  value &= 0xffff;
	  wdata = (value<<0LLU) | (value << 16LLU);
	  wdata = (wdata<<0LLU) | (wdata << 32LLU);
	  trans->set_byte_enable_ptr(&short_write_masks[((memaddr ^ ENDIAN_FLIP)>>1) & 3].a);
	  trans->set_byte_enable_ptr((memaddr ^ ENDIAN_FLIP)& 4 ? &mask_right[0]: &mask_left[0]);
	  break;

	case 32:
	  value &= 0xFFFFffff;
	  wdata = (value<<0LLU) | (value << 32LLU);
	  trans->set_byte_enable_ptr((memaddr ^ ENDIAN_FLIP)& 4 ? &mask_right[0]: &mask_left[0]);
	  break;
	}
    }
  trans->set_dmi_allowed( false );
  llsc_extension *llsc;
  trans->get_extension(llsc);
  assert(llsc);
  llsc->id = I2V(procID);
  trans->set_data_ptr( (unsigned char *) &wdata);
  trans->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );

  int skt = 0;
  POWER3(PW_TLM3(trans->pw_set_origin(m_module,  //write
				      PW_TGP_ADDRESS |
				      PW_TGP_ACCT_SRC |
				      ((false)?0:PW_TGP_DATA|PW_TGP_LANES), 
				      &bus_tracker0)));
  
  initiator_socket->b_transport(*trans, delay);
  
  if (trans->is_response_error()) // Gives address error on failed src
    {
      //trans->release();
      payload_mm.free(trans);

      if(trans->get_response_status() == tlm::TLM_GENERIC_ERROR_RESPONSE)
          return 2;
      else {
          if (traceregions && traceregions->check(memaddr, TENOS_TRACE_CPU_WRITE))
              printf("Store conditional at " PFX64 " failed: %s\n", memaddr, trans->get_response_string().c_str());
          return 0;
      }
    }

  if (traceregions && traceregions->check(memaddr, TENOS_TRACE_CPU_WRITE))
    printf("%s: Store conditional w=%i at " PFX64 " ok\n", name(), width, memaddr);  

  POWER3(PW_TLM3(trans->pw_terminus(m_module)));
  //trans->release();
  payload_mm.free(trans);

  if (trace_fd)
    {
      fprintf(trace_fd, "WC at " PFX64 " D=" PFX64 "\n", memaddr, wdata);
    }
  return 1;
}



// Atomic exchange operation turns into bus-level loadlinked/store conditional.
int cpu_busaccess::eval_xchg(addr_t memaddr, u64_t wdata, u64_t &rdata, int width, sc_time &delay, bool tf)
{
#define XCHG(X)  if (tf) X
  XCHG(printf("%s: xchg start at " PFX64 " wdata=" PFX64 "\n", name(), memaddr, wdata));
  int ok2;
  //trace100("xchg ok");
  while(1) // Implement xchg using ll/sc.  Keep going until success.
    {
      // First read old value with load linked/locked.     
      bool ok1 = mem_read(memaddr & ~7LLU, rdata, delay, true, 0, tf);
      XCHG(printf("%s: xchg load done at " PFX64 " rdata=" PFX64 "\n", name(), memaddr, rdata));
      if (!ok1)
	{
	  char txt[1024];
	  snprintf(txt, 1024, "%s:%s: Load linked as past of xchange failed", name(), kind());
	  SC_REPORT_ERROR("busaccess", txt);
	  return 0; //  Failed outright.
	} 

      // Then store conditional
      ok2 = cpu_mem_store_conditional(memaddr, wdata, width, delay);
      if (ok2) break; // Failure here means try again...
      XCHG(printf("xchg try again\n"));
      // Hold-off according to processor id!
      delay += sc_time(procID, SC_US);
    }
  if (width == 32 && ((memaddr ^ ENDIAN_FLIP) & 4)) rdata = rdata >> 32;
  XCHG(printf("%s: xchg end at " PFX64 " wdata=" PFX64 " rdata=" PFX64 "\n", name(), memaddr, wdata, rdata));
  return ok2;
}

//
// correct ? : The lane mask is a byte u8_t with lsb denoting the modelled-cpu's names (bit 0 is its ls-byte).
//
bool cpu_busaccess::mem_write(addr_t memaddr, u8_t mask, u64_t wdata, sc_time &delay)
{
  stats.writes += 1;
#ifdef CYCLE_ACCURATE
  if (ea_mon) ea_mon->write(memaddr);
#endif
  PW_TLM_PAYTYPE trans;
  PW_TLM3(trans.set_customer_acct(*Customer_ids[current_alias]));
  u8_t bmask[8];  
  char bmask_txt[132];
  if (mask == 0)
    {
      trans.set_byte_enable_length(0);
      trans.set_byte_enable_ptr(0);
    }
  else if (0 && mask == 0xF0) // TODO  - perhaps precomputed ones in byte_write_masks for common patterns - but need to include endian FLIP.
    {
      trans.set_byte_enable_ptr(mask_left);
      trans.set_byte_enable_length(8);
    }
  else if (0 && mask == 0x0F)
    {
      trans.set_byte_enable_ptr(mask_right);
      trans.set_byte_enable_length(8);
    }
  else 
    {
      for (int i=0;i<8;i++) bmask[i ^ ENDIAN_FLIP] = (mask & (1<<i)) ? 0xFF:00; 
      trans.set_byte_enable_ptr(bmask);
      trans.set_byte_enable_length(8);
    }
  if (traceregions && traceregions->check(memaddr, TENOS_TRACE_CPU_WRITE)) printf("[%s: Writing at " PFX64 " d=" PFX64 " mask=%02X lanes=%s]\n", name(), memaddr, wdata, mask, mask ? (snprintf(bmask_txt, 132, PFX64, ((u64_t *)(bmask))[0]), bmask_txt): "ALL");
  trans.set_write();
  trans.set_address((u64_t) memaddr);
  trans.set_data_length(8);
  trans.set_data_ptr((u8_t *) & wdata);
  trans.set_streaming_width(8);
  trans.set_dmi_allowed( false ); // TODO... use for writes as well?
  trans.set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );

  int skt = 0;
  POWER3(PW_TLM3(trans.pw_set_origin(m_module, // write
				     PW_TGP_ADDRESS |
				     PW_TGP_ACCT_SRC |
				     ((true)?0:PW_TGP_DATA|PW_TGP_LANES), 
				     &bus_tracker0)));

  initiator_socket->b_transport(trans, delay);
  POWER3(PW_TLM3(trans.pw_terminus(m_module)));
  if (trans.is_response_error())
    {
        //char txt[1024];
        //snprintf(txt, 1024, "%s:%s: Error from b_mem_write addr=" PFX64 ", mask=0x%x, response status=%s lastfetchPC=" PFX64, name(), kind(), memaddr, mask, trans.get_response_string().c_str(), lastfetch);
        //SC_REPORT_ERROR("busaccess", txt);

        return false;
    } 

  if (trace_fd)
    {
      fprintf(trace_fd, "W at " PFX64 " L=%X D=" PFX64 "\n", memaddr, mask, wdata);
    }

  return true;

}


void cpu_busaccess::invalidate_direct_mem_ptr0(sc_dt::uint64 start, sc_dt::uint64 end)
{
  printf("DMI Invalidated 0\n");
  DMI_records[0]->data_valid = false;
  if (!harvardf) DMI_records[1]->data_valid = false;
}

void cpu_busaccess::invalidate_direct_mem_ptr1(sc_dt::uint64 start, sc_dt::uint64 end)
{
  printf("DMI Invalidated 1\n");
  DMI_records[1]->data_valid = false;
}


// constructor
cpu_busaccess::cpu_busaccess(sc_module *module, sc_module_name names, int endian_flip, int idx, int harvardf, int disable_dmi) : 
  disable_dmi(disable_dmi),
  sc_module(names),
  ENDIAN_FLIP(endian_flip),
  m_module(module),
  procID(idx),
#ifdef TLM_POWER3
  pw_module(),
  n_customer_ids_in_use(0),
  speedo(*this),
#if PW_TLM_PAYLOAD > 0
  bus_tracker0(module), // Used if counting wire transitions. 
  bus_tracker1(module),
#endif
#endif
  harvardf(harvardf)
{
  trace_fd = 0;
  DMI_records[0] = new dmi_record((const char *)names);
  DMI_records[1] = (1)  ? new dmi_record((const char *)names): 0;
  ifetch_socket = (harvardf) ? new tlm_utils::simple_initiator_socket<cpu_busaccess, 64, PW_TLM_TYPES> ("ifetch_socket"): 0;
  traceregions = 0;
  atomics = 0; // should clear on reset too!
  initiator_socket.register_invalidate_direct_mem_ptr(this, &cpu_busaccess::invalidate_direct_mem_ptr0);
  if (ifetch_socket) ifetch_socket->register_invalidate_direct_mem_ptr(this, &cpu_busaccess::invalidate_direct_mem_ptr1);
  new tenos_report_item(name(), "bus-writes", &stats.writes);
  new tenos_report_item(name(), "bus-reads", &stats.reads);
  //std::cout << "constructed cpu_busaccess this=" << ((void *)this) << " pw_module=" << ((void *)dynamic_cast<pw_module *>(this)) << "\n";
  //std::cout << "constructed cpu_busaccess this=" << ((void *)this) << " pw_module_base=" << ((void *)dynamic_cast<pw_module_base *>(this)) << "\n";

#ifdef TLM_POWER3
  char line[132];
  snprintf(line, 132, "%s_%i", (const char *)names, idx);
  printf("registering customer %s\n", line);
  Customer_ids[0] = new pw_customer_id(m_module, line, idx);
  n_customer_ids_in_use = 1;
  current_alias = 0;
  resume_accounting_alias(current_alias);
  sc_pwr::trace_t m  = no_children;

#endif
}

#ifdef TLM_POWER3
void cpu_busaccess::resume_accounting_alias(int n) // Change token put in my initiated traffic.
{
  current_alias = n % n_customer_ids_in_use;
  a_customer_observer = Customer_ids[current_alias]->get_observer();
}

int cpu_busaccess::local_energy()
{
  pw_energy l_sum = a_customer_observer->get_energy();
#if 0
  for (int a=0; a<PW_ACCT_MAX_NO_ACCOUNTS; a++) 
    {
      pw_energy l_e = (pw_stat_observer_base::global_inuse(a)) ? parent.a_customer_observer->get_energy(a): PW_ZERO_ENERGY; // The library code here called is in accounts.cpp
      l_sum += l_e;
    }
#endif
  float joules = l_sum.to_joules();
  int ival = (int)(joules * 1.0e6);
  //std::cout << "Local energy read by cpu_busaccess " << l_sum << " i=" << ival << "\n";
  return ival;
}
#endif


// An allocater that ensures the llsc extension is present.
PW_TLM_PAYTYPE *cpu_busaccess::llsc_payload_alloc()
{
  PW_TLM_PAYTYPE *r = payload_mm.allocate();
  llsc_extension *ext;
  r->get_extension(ext);
  if (!ext)
    {
      ext = new llsc_extension;
      r->set_extension(ext);
    }
  return r;
}


void cpu_busaccess::sim_done(const char *msg)
{
  std::cout <<  "procID=" << (procID + '0') << ", sim_done " << msg << " at simulation time " << sc_time_stamp() << "\n";
  sc_stop();
}

void cpu_busaccess::trace_msg(const char *msg)
{
  if (trace_fd) fprintf(trace_fd, "M %s\n", msg);
}

void cpu_busaccess::start_trace(const char *sname)
{
  if (!sname) sname = name();
  trace_fd = fopen(sname, "w");
}



void cpu_busaccess::stat_report(const char *msg, FILE *fd, bool resetf)
{

#ifdef DMI_STATS
  for (int skt=0; skt<NO_CPU_BUSACCESS_SKTS; skt++)
    if (fd) fprintf(fd, "%s %s DMI[%i] too_low=%i, too_high=%i, not_valid=%i, linked=%i, measure_needed=%i aok=%i, too_hi_margin=%lli\n",
		    name(), msg, skt,
		    DMI_records[skt]->dmi_stats.too_low, 
		    DMI_records[skt]->dmi_stats.too_high,
		    DMI_records[skt]->dmi_stats.not_valid, 
		    DMI_records[skt]->dmi_stats.linked, 
		    DMI_records[skt]->dmi_stats.measure_needed,
		    DMI_records[skt]->dmi_stats.aok,
		    DMI_records[skt]->dmi_stats.too_high ? DMI_records[skt]->dmi_stats.too_high_margin/DMI_records[skt]->dmi_stats.too_high: 0
		    ); 
#endif


  if (fd) fprintf(fd, "%s %s  reads=%i dmi_reads=%i writes=%i\n", name(), msg, stats.reads, stats.dmi_reads, stats.writes);
  if (resetf) stats.reset();
  
}


void cpu_busaccess::diagnostic_report(FILE *fd, int severity, const char *msg)
{
  tracebuffer.report_trace(fd, name());
}

// eof
