// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>

//
// $Id: mips64_btlm.cpp,v 1.23 2011/08/21 10:57:33 my294 Exp $
//
//
  static int kk = 0;

#include <assert.h>
#include "systemc.h"
#include "mips64_btlm.h"
#include "llsc_extension.h"

//#include "support/mamba_extension.h"


// This needs to be bigendian for mips 64

extern class argv_backdoor_setup *argv_backdoor;

#define MTRC(X) X 


extern int g_tracelevel;
extern int g_dmi_disablef;

// Constructor
mips64_btlm::mips64_btlm(sc_core::sc_module_name names, uint8_t pID, bool harvardf):
  sc_module(names),
  harvardf(harvardf),
  busaccess(this, "busaccess", 0, harvardf, g_dmi_disablef),
  mips64(names, pID, &busaccess.traceregions),
#ifdef TLM_POWER3
  pw_module("power_config_mips64.txt"),
  frontside_bus(this),
#endif
  core_period(2, SC_NS),
  bus_period(5, SC_NS)
{
  SC_HAS_PROCESS(mips64_btlm);

#ifdef TLM_POWER3
  printf("mips64_btlm made %p, bus=%p %p,  pwr=%p\n", this, &busaccess, &frontside_bus, dynamic_cast<pw_module_base *>(this));
#else
  printf("mips64_btlm made %p, bus=%p\n", this, &busaccess);
#endif


  // excluding caches: based on: just made up!
#ifdef TLM_POWER3
  set_excess_area(pw_length(300.0, PW_um), pw_length(250, PW_um)); // Todo correct for number of spokes
  pw_power leakage = pw_power(10.0, PW_mW);
  set_static_power(leakage);
  //set_fixed_area(pw_area(0.5, PW_sqmm)); // Without caches
#endif

  ext_interrupt = 0;
  procID = pID;
  SC_THREAD(run);
  char txt[20];
  sprintf(txt, "MIPS64_CORE_%d", procID);

  where_last = 0;


  // machine endianness
  tlm::tlm_endianness m_endianness = (BIG_ENDIAN) ? tlm::TLM_BIG_ENDIAN : tlm::TLM_LITTLE_ENDIAN;

  // host endianness (ie that of the modelling workstation)
  tlm::tlm_endianness m_host_endianness = tlm::get_host_endianness();
  
  cout << "m_endianness=" << m_endianness << "\n";
  cout << "host_endianness=" << m_host_endianness << "\n";
  eflip = (m_endianness != m_host_endianness) ? 7:0;


}

void mips64_btlm::reset(bool enable_first_to_run)
{
  halted = false;
  core_reset(enable_first_to_run);
}


// Main model process loop
void mips64_btlm::run()
{
  //  sc_time local_quantum(10, SC_NS);
  lt_busdelay = SC_ZERO_TIME;
#ifdef TLM_POWER3
  set_vcc(1.4, PW_VOLT);
//  update_power("ON", "IDLE");
#endif
  m_over = false; // for now
  while(!m_over)
    {
      if (m_qk.need_sync()) m_qk.sync();   // Keeper synchronize when quantum is reached
      if (ext_interrupt && *ext_interrupt) // Level-sensitive interrupt.
	{
	  if (busaccess.traceregions && busaccess.traceregions->check(0, TENOS_TRACE_CPU_INTERRUPTS))
	    printf("%s: %s: INTERRUPT %i\n", name(), kind(), procID); // This is an unmasked version of the interrupt signal.
	  hw_interrupt("mips64_btlm");
	}
      if (halted)
	{
	  m_qk.sync();
	  wait(1, SC_US); // must poll for interrupt in halted TODO make an event
	  continue;
	}
      sc_time ins_start = m_qk.get_current_time();
      lt_busdelay = SC_ZERO_TIME; // This is updated by busaccess.
      step();


      sc_time ext_end = lt_busdelay + sc_time_stamp(); // Bus cycle end time
      sc_time ins_end = ins_start+core_period;  // One instruction per core_period.

      // Retire join: take maximum of internal and external delays at join.
#define TMAX(X, Y) ((X)>(Y)?(X):(Y))
      //cout << "kernel=" << sc_time_stamp() << " start=" << ins_start << " ext_end=" << ext_end << " ins_end=" << ins_end << "\n";
      m_qk.set(TMAX(ext_end, ins_end)-sc_time_stamp());

    }
  m_qk.sync();


}



void mips64_btlm::corepause(int us)  // Pause CPU for this time interval
{
  // We need to implement the halt and interrupt material as well...
//  update_power("ON", "IDLE");
#ifdef TLM_POWER3
  pw_voltage l_vcc = get_vcc();
  set_vcc(0.8, PW_VOLT);
#endif
  sc_time delay(us, SC_US);
  lt_busdelay += delay;
#ifdef TLM_POWER3
  set_vcc(l_vcc);
//  update_power("ON", "COMPUTE");
#endif
}


#ifdef TLM_POWER3
void mips64_btlm::energy_event(pw_energy pj)
{
  // cout << name() << " energy log " << pj << "\n";
  pw_module_base::record_energy_use(pj);
}
#endif



void mips64_btlm::hw_interrupt(const char *)
{
  assert(0); // todo - send to spoke 0
}


void mips64_btlm::sim_done()
{
  m_over = true;
  cout << name() << " sim_done\n";
  if (procID == 0) sc_stop(); // sc_end_of_simulation("sim_done")
#ifdef TLM_POWER3
    {set_vcc(0.2, PW_VOLT);  wait(100, SC_US);}
#endif
}

// Elaboration phase: connect the interrupt wire to this core using this call.
bool *mips64_btlm::connect_interrupt(bool *c)
{
  bool *ov = ext_interrupt;
  ext_interrupt = c;
  return ov;
}


void mips64_btlm::atomic_prefix() // Prefix following load/store pair as atomic.
{
  busaccess.atomic_prefix(); // Trigger atomic transaction state machine.
}


int mips64_btlm::cpu_mem_store_conditional32(uint64_t memaddr, uint32_t value)
{
  return busaccess.cpu_mem_store_conditional(memaddr, value, 32, lt_busdelay);
}


bool mips64_btlm::eval_mem64(mips_addr_t memaddr, u64_t &d, breakpt_t *bp, bool update_presence_flag, bool new_polarity)
{
  if (update_presence_flag)
    {
      assert(0);
      // You should be using mamba_btlm not mips64_btlm!
    }
  return busaccess.eval_mem64(memaddr, d, bp, lt_busdelay);
}


bool mips64_btlm::eval_mem32(mips_addr_t memaddr, u32_t &rd, breakpt_t*bp, bool linked)
{
  return busaccess.eval_mem32(memaddr, rd, bp, lt_busdelay, linked);
}

 
bool mips64_btlm::eval_mem16(mips_addr_t memaddr,   u16_t &rd, breakpt_t *bp)
{
  return busaccess.eval_mem16((memaddr), rd, bp, lt_busdelay);
}

bool mips64_btlm::eval_mem8(mips_addr_t memaddr, u8_t &rd, breakpt_t *bp)
{
  return busaccess.eval_mem8((memaddr), rd, 0, lt_busdelay);
}


void mips64_btlm::set_direct8(mips_addr_t a, u8_t d, int, breakpt_t *bp)
{
  assert(0);
}

u8_t mips64_btlm::eval_direct8(mips_addr_t a, int, int, breakpt_t *bp)
{
  assert(0);
}


void mips64_btlm::set_mem64(u64_t memaddr, u64_t wd, breakpt_t *bp, bool mamba_swp)
{
  if (mamba_swp)
    {
      assert(0);
      // You should be using mamba_btlm not mips64_btlm!
    }
  busaccess.set_mem64(memaddr, wd, bp, lt_busdelay);
}

void mips64_btlm::set_mem32(mips_addr_t memaddr, u32_t wd, breakpt_t *bp)
{
  busaccess.set_mem32(memaddr, wd, bp, lt_busdelay);
}

void mips64_btlm::set_mem16(mips_addr_t memaddr, u16_t wd, breakpt_t *bp)
{
  busaccess.set_mem16(memaddr, wd, bp, lt_busdelay);
}

void mips64_btlm::set_mem8(mips_addr_t memaddr, uint8_t wd, breakpt_t *bp)
{
  busaccess.set_mem8(memaddr, wd, bp, lt_busdelay);
}

void mips64_btlm::stat_report(const char *msg, FILE *fd, bool resetf)
{

  if (fd) fprintf(fd, "%s %s  \n", name(), msg);
  busaccess.stat_report(msg, fd, resetf);
  if (resetf) stats.reset();
  
}



// eof
