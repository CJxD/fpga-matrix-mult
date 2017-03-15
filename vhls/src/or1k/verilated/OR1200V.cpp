//
// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2009-10 DJ Greaves (TLM 2.0 Version Arturs Prieditis) 
// Blocking TLM wrapper for the verilated version of the OR1200 core.
// $Id: OR1200V.cpp,v 1.1 2011/02/01 10:52:03 djg11 Exp $
//
#include "OR1200V.h"


#include "speedo_reg_mapping.h"





#include <iostream>

#ifdef TLM_POWER3
#include "tlm_power.h"
#define POWER3(X) X
#else
#define POWER3(X)
#endif

#define OR1K_ENDIAN_FLIP (7) // Not the same endian as the underlying workstation (x86).

// Constructor for verilated core
OR1200V::OR1200V(sc_core::sc_module_name name, u8_t pID, bool harvardf):	 
  COREISA_IF(name, OR1K_ENDIAN_FLIP, pID, harvardf),
  ins_fetcher(this),
  or1200_cpu("or1k2"),
  accessor(&or1200_cpu),
  over(false)
{
  ext_interrupt = 0;
  char txt[20];
  sprintf(txt, "OR1200V_CORE_%d", procID);
  SC_THREAD(run);
#ifdef TLM_POWER3
  // Power figures: ARM 9 1.4mm2  47 mW 200 MHz - 235 pJ per instruction.
  // 200 MHz x 250 pJ = 50 mW (no caches included).
  instruction_energy = pw_energy(200.0, PW_pJ);
  pw_power leakage = pw_power(10.0, PW_mW);
  set_static_power(leakage);
  set_fixed_area(pw_area(0.5, PW_sqmm)); // Without caches
#endif
  extern sc_trace_file* g_Tf;
  if (g_Tf)
    {
      int i = pID;
#if 1
      sprintf(txt, "clk_%d", i);
      sc_trace(g_Tf, or1200_cpu.clk_i, txt, 1);
      sprintf(txt, "icyc_o_%d", i);
      sc_trace(g_Tf, or1200_cpu.iwb_cyc_o,txt, 1); 
      sprintf(txt, "istb_o_%d", i);
      sc_trace(g_Tf, or1200_cpu.iwb_stb_o,txt );  
      sprintf(txt, "icab_o_%d", i);
      sc_trace(g_Tf, or1200_cpu.iwb_cab_o, txt, 1);
      sprintf(txt, "iadr_o_%d", i);
      sc_trace(g_Tf, or1200_cpu.iwb_adr_o, txt, 32);
      sprintf(txt, "iack_i_%d", i);
      sc_trace(g_Tf, or1200_cpu.iwb_ack_i, txt, 1);
      sprintf(txt, "idat_i_%d", i);
      sc_trace(g_Tf, or1200_cpu.iwb_dat_i, txt, 1);
      sprintf(txt, "ierr_i_%d", i);
      sc_trace(g_Tf, or1200_cpu.iwb_err_i, txt, 1);
      sprintf(txt, "idat_o_%d", i);
      sc_trace(g_Tf, or1200_cpu.iwb_dat_o, txt, 32);
      sprintf(txt, "iwe_o_%d", i);
      sc_trace(g_Tf, or1200_cpu.iwb_we_o, txt, 1);
      sprintf(txt, "irty_i_%d", i);
      sc_trace(g_Tf, or1200_cpu.iwb_rty_i, txt, 1);
      sprintf(txt, "isel_o_%d", i);
      sc_trace(g_Tf, or1200_cpu.iwb_sel_o, txt, 4);
      
      //or1200_cpu data
      sprintf(txt, "dcyc_o_%d", i);
      sc_trace(g_Tf, or1200_cpu.dwb_cyc_o, txt, 1); 
      sprintf(txt, "dstb_o_%d", i);
      sc_trace(g_Tf, or1200_cpu.dwb_stb_o, txt, 1);  
      sprintf(txt, "dcab_o_%d", i);
      sc_trace(g_Tf, or1200_cpu.dwb_cab_o, txt, 1);
      sprintf(txt, "dadr_o_%d", i);
      sc_trace(g_Tf, or1200_cpu.dwb_adr_o, txt, 32);
      sprintf(txt, "dack_i_%d", i);
      sc_trace(g_Tf, or1200_cpu.dwb_ack_i, txt, 1);
      sprintf(txt, "ddat_i_%d", i);
      sc_trace(g_Tf, or1200_cpu.dwb_dat_i, txt, 32);
      sprintf(txt, "derr_i_%d", i);
      sc_trace(g_Tf, or1200_cpu.dwb_err_i, txt, 1);
      sprintf(txt, "ddat_o_%d", i);
      sc_trace(g_Tf, or1200_cpu.dwb_dat_o, txt, 32);
      sprintf(txt, "dwe_o_%d", i);
      sc_trace(g_Tf, or1200_cpu.dwb_we_o, txt, 1);
      sprintf(txt, "drty_i_%d", i);
      sc_trace(g_Tf, or1200_cpu.dwb_rty_i, txt, 1);
      sprintf(txt, "dsel_o_%d", i);
      sc_trace(g_Tf, or1200_cpu.dwb_sel_o, txt, 4);
#else
      sprintf(txt, "clk_%d", i);
      sc_trace(g_Tf, or1200_cpu.clk,txt );
      //or1200_cpu instructions
      
      sprintf(txt, "icyc_o_%d", i);
      sc_trace(g_Tf, or1200_cpu.icyc_o,txt ); 
      sprintf(txt, "istb_o_%d", i);
      sc_trace(g_Tf, or1200_cpu.istb_o,txt );  
      sprintf(txt, "icab_o_%d", i);
      sc_trace(g_Tf, or1200_cpu.icab_o,txt);
      sprintf(txt, "iadr_o_%d", i);
      sc_trace(g_Tf, or1200_cpu.iadr_o,txt);
      sprintf(txt, "iack_i_%d", i);
      sc_trace(g_Tf, or1200_cpu.iack_i,txt);
      sprintf(txt, "idat_i_%d", i);
      sc_trace(g_Tf, or1200_cpu.idat_i,txt);
      sprintf(txt, "ierr_i_%d", i);
      sc_trace(g_Tf, or1200_cpu.ierr_i,txt);
      sprintf(txt, "idat_o_%d", i);
      sc_trace(g_Tf, or1200_cpu.idat_o,txt);
      sprintf(txt, "iwe_o_%d", i);
      sc_trace(g_Tf, or1200_cpu.iwe_o,txt);
      sprintf(txt, "irty_i_%d", i);
      sc_trace(g_Tf, or1200_cpu.irty_i,txt);
      sprintf(txt, "isel_o_%d", i);
      sc_trace(g_Tf, or1200_cpu.isel_o,txt);
      
      //or1200_cpu data
      sprintf(txt, "dcyc_o_%d", i);
      sc_trace(g_Tf, or1200_cpu.dcyc_o,txt); 
      sprintf(txt, "dstb_o_%d", i);
      sc_trace(g_Tf, or1200_cpu.dstb_o, txt );  
      sprintf(txt, "dcab_o_%d", i);
      sc_trace(g_Tf, or1200_cpu.dcab_o,txt);
      sprintf(txt, "dadr_o_%d", i);
      sc_trace(g_Tf, or1200_cpu.dadr_o,txt);
      sprintf(txt, "dack_i_%d", i);
      sc_trace(g_Tf, or1200_cpu.dack_i,txt);
      sprintf(txt, "ddat_i_%d", i);
      sc_trace(g_Tf, or1200_cpu.ddat_i,txt);
      sprintf(txt, "derr_i_%d", i);
      sc_trace(g_Tf, or1200_cpu.derr_i,txt);
      sprintf(txt, "ddat_o_%d", i);
      sc_trace(g_Tf, or1200_cpu.ddat_o,txt);
      sprintf(txt, "dwe_o_%d", i);
      sc_trace(g_Tf, or1200_cpu.dwe_o,txt);
      sprintf(txt, "drty_i_%d", i);
      sc_trace(g_Tf, or1200_cpu.drty_i,txt);
      sprintf(txt, "dsel_o_%d", i);
      sc_trace(g_Tf, or1200_cpu.dsel_o, txt);
#endif
    }


//or1200.............................................................//
  or1200_cpu.rst_i = 1;
  or1200_cpu.dwb_rst_i	= 1;
  or1200_cpu.iwb_rst_i      = 1;
 
  or1200_cpu.clmode_i = 0;
 // Debug i/f
  or1200_cpu.dbg_ewt_i = 0;
  or1200_cpu.dbg_stall_i = 0;
 
  update(0);

}

void OR1200V::update(bool c)
{
  or1200_cpu.clk_i 	= c;
  or1200_cpu.dwb_clk_i	= c;
  or1200_cpu.iwb_clk_i 	= c;  
  or1200_cpu.eval();
}

void OR1200V::reset(bool selfstart)
{
  // Selfsart - we want only one core to start running.

  // TODO selfstart ignored at the moment
  busaccess.reset();
  or1200_cpu.rst_i     	= 1;
  or1200_cpu.dwb_rst_i	= 1 ;
  or1200_cpu.iwb_rst_i	= 1;
  update(0), update(1);
  update(0), update(1);
  or1200_cpu.rst_i     	= 0;
  or1200_cpu.dwb_rst_i	= 0 ;
  or1200_cpu.iwb_rst_i	= 0;
}


// Instruction mini-cache for other half of a 64 bit word.
void OR1200V::ins_fetcher_t::fetch(u32_t memadr, u32_t &i, sc_time &lt_busdelay)
{ 
  u32_t a1 = memadr & ~7;
  if (a1 != cached_adr)
    {
      parent->busaccess.instr_fetch64(a1, data, 0, lt_busdelay);
      cached_adr = a1;
    }
  // Return big-endian order.
  i = (memadr & 4) ? (data >> 0) : (data >> 32); 
}


void OR1200V::ifetch_step(sc_time &delay)
{
  //Service instruction fetch port
  if (or1200_cpu.iwb_cyc_o && or1200_cpu.iwb_stb_o)
    {
      u32_t memaddr = or1200_cpu.iwb_adr_o; 
      u32_t i;
      ins_fetcher.fetch(or1200_cpu.iwb_adr_o, i, delay);
      if (busaccess.traceregions && busaccess.traceregions->check(memaddr, TENOS_TRACE_CPU_IFETCH))  
	{
	  char line[132];
	  if (argv_backdoor_su) argv_backdoor_su->symbolic_dis64(line, 132, memaddr);
	  else snprintf(line, 132, " %X ", memaddr);
	  //disassemble_insn(i); // Verilated code has a disassembler we could call.
	  printf("%s:%s: Fetch insn a=%s ins=%x\n", name(), kind(), line, i);
	}

      or1200_cpu.iwb_dat_i = i;
      or1200_cpu.iwb_ack_i = true; 
      
      if (busaccess.traceregions)
	{
	  int m = busaccess.traceregions->check(memaddr, TENOS_TRACE_CPU_KEY_REGS|TENOS_TRACE_CPU_ALL_REGS);
	  dumpregs(stdout, m & TENOS_TRACE_CPU_ALL_REGS);
	}
    }
}


int OR1200V::eval_bdoor_read(oraddr_t memaddr, u32_t &rdata, sc_time &delay)
{
  #include "../backdoor_reads.C"
}

int OR1200V::eval_bdoor_write(oraddr_t memaddr, u32_t wdata,   sc_time &delay)
{
  #include "../backdoor_writes.C"
}


// Process for the core.
void OR1200V::run()
{
  //m_qk.set_global_quantum(m_core_period); // Wrong place to set this
  m_qk.reset();
  update(0);
  while(!over)
    {
      if (m_qk.need_sync()) m_qk.sync();  // Keeper synchronize when quantum is reached
      //busaccess.tick();
      sc_time ins_start = m_qk.get_current_time();
      //cout << ins_start << " " << name() << " tick\n";
      sc_time lt_d_delay = SC_ZERO_TIME; //
      sc_time lt_i_delay = SC_ZERO_TIME; //

      ifetch_step(harvardf ? lt_i_delay: lt_d_delay);  //Service instruction fetch charging delay to appropriate account.

      // Service data read/write port (in serial or parallel depending on harvardf).
      if (or1200_cpu.dwb_cyc_o && or1200_cpu.dwb_stb_o)
	{
	  u32_t a32 = or1200_cpu.dwb_adr_o;
	  u64_t a64 = xlat32to64(a32);
	  //printf("Write %x sel=%x  a64=%lx\n", or1200_cpu.dwb_adr_o, or1200_cpu.dwb_sel_o, a64); 
	  if (!or1200_cpu.dwb_we_o) 
	    {
	      u32_t rd32;
	      if (eval_bdoor_read(a32, rd32, lt_d_delay))
		{
		  or1200_cpu.dwb_dat_i = rd32;
		  printf("bdoor read %x size=%i gave %x \n", a32, or1200_cpu.dwb_dat_o, rd32);
		}
	      else busaccess.eval_mem32(a64, or1200_cpu.dwb_dat_i, 0, lt_d_delay); 
	    }
	  else
	    {
	      u32_t wd32 = or1200_cpu.dwb_dat_o;
	      switch(or1200_cpu.dwb_sel_o)
		{
		case 0xF:
		  if (eval_bdoor_write(a32, wd32, lt_d_delay)) { /* */} // only support word writes to backdoor space.
		  else busaccess.set_mem32(a64, wd32, 0, lt_d_delay);
		  break;
		  
		  
		case 0x3: // big endian byte addressing.
		  busaccess.set_mem16((a64 & ~1)+2LLU, or1200_cpu.dwb_dat_o, 0, lt_d_delay);
		  break;

		case 0xC: // big endian byte addressing.
		  busaccess.set_mem16((a64 & ~1)+0LLU, or1200_cpu.dwb_dat_o, 0, lt_d_delay);
		break;
		
		case 0x1: // big endian byte addressing.
		  busaccess.set_mem8((a64 & ~3)+3LLU, or1200_cpu.dwb_dat_o, 0, lt_d_delay);
		  break;
		case 0x2:
		  busaccess.set_mem8((a64 &~3)+2LLU, or1200_cpu.dwb_dat_o, 0, lt_d_delay);
		  break;
		case 0x4:
		  busaccess.set_mem8((a64 & ~3)+1LLU, or1200_cpu.dwb_dat_o, 0, lt_d_delay);
		  break;
		case 0x8:
		  busaccess.set_mem8((a64 & ~3)+0LLU, or1200_cpu.dwb_dat_o, 0, lt_d_delay);
		  break;
		  
		default:
		  printf("%s: Unsupported byte lanes = %i\n", name(), or1200_cpu.dwb_sel_o);
		  assert(0); // for now
		}
	    }
	  or1200_cpu.dwb_ack_i = true; 
	}

      sc_time d_end = lt_d_delay + sc_time_stamp(); // D Bus cycle end time
      sc_time i_end = lt_i_delay + sc_time_stamp(); // I Bus cycle end time 
      // Retire join: take maximum of internal and external delays at join.
#define TMAX(X, Y) ((X)>(Y)?(X):(Y))
      sc_time run_until = TMAX(ins_start + m_effective_instruction_period, TMAX(i_end, d_end));
      sc_time retire_time = ins_start;
      while (retire_time < run_until && !over)
	{
	  retire_time += m_effective_instruction_period;  // 
	  if (reset_or_yield_countdown > 0)
	    {
	      reset_or_yield_countdown -= 1;
	    }
	  else
	    {
	      update(0);
	      update(1);
	      monitor_or_decode(); // Moved to before update call (Apr 2014) owing to atomic prefix sometimes being processed after the first bus op of an atomic pair.
	      POWER3(record_energy_use(instruction_energy));      // this is energy per clock - not energy per instruction!
	    }
	  or1200_cpu.iwb_ack_i = false; //clear previous ack
	  or1200_cpu.dwb_ack_i = false; //clear previous ack
	}
      //       cout << "kernel=" << sc_time_stamp() << " start=" << ins_start << " ext_end=" << ext_end << " ins_end=" << ins_end << "\n";
      if (0)
	{
	  cout << "kernel=" << sc_time_stamp() << " start=" << ins_start << "\n";
	  cout << name () << " lt_busdelay was i=" << lt_i_delay << "  d=" << lt_d_delay << "\n";
	  cout << name () << " core_period=" << m_core_period << "  retire_time=" << retire_time << "  new delay=" << retire_time-sc_time_stamp() << "\n";
	  //std::cout << "Retire at " << retire_time << "\n";
	}
      m_qk.set(retire_time-sc_time_stamp());      // Accumulate local time
    } 
  m_qk.sync();
}


void OR1200V::dumpregs(FILE *fd, int allregs)
{
  if (!fd) return;
  fprintf(fd, "PC:%08X: ", accessor.getPc());
  if (allregs)
    {
      for (int i=0; i<32; i++)
	{
	  fprintf(fd, "GPR%02i: %08X     ", i, accessor.getGpr(i));
	  if ((i & 3)==3) fprintf(fd, "\n");
	}

    }
  else
    {
      static int mapp[8] = { 1, 2, 3, 4, 5, 9, 30, 31 }; 
      for (int j=0; j<8; j++)
	{
	  int i = mapp[j];
	  fprintf(fd, "GPR%02i: %08X     ", i, accessor.getGpr(i));
	  if ((i & 3)==3) fprintf(fd, "\n");
	}

    }
}

void OR1200V::monitor_or_decode()
{
  u32_t  r3;
  u32_t instr;
  // Check the instruction when the freeze signal is low.
  if (!accessor.getWbFreeze())
   {
     instr=accessor.getWbInsn();
     
     // atomic_prefix and wrch etc are sometimes not working - getting called too late!
     // atomic_prefix is not a nop any longer - it is a write to a backdoor memory register.
     #include "../backdoor_nops.C"
    }
}


void OR1200V::atomic_prefix() // Prefix following load/store pair as atomic.
{
  //printf("%s: Atomic prefix\n", name());
  busaccess.atomic_prefix(); // Trigger atomic transaction state machine.
}



void OR1200V::corepause(int us)  // Pause CPU for this time interval
{
  // We need to implement the halt and interrupt material as well...
  sc_time l_delay(us, SC_US);
  m_qk.sync();
  wait(l_delay);
}


// Elaboration phase: connect the interrupt wire to this core using this call.
bool *OR1200V::connect_interrupt(bool *c)
{
  bool *ov = ext_interrupt;
  ext_interrupt = c;
  return ov;
}



void OR1200V::sim_done(const char *msg)
{
  busaccess.sim_done(msg);
  over = 1;
}

int OR1200V::simulator_cmd(int cmd, int arg0, int arg1, u32_t *rp)
{
  if (cmd == TENOS_CMD_SIM_DONE) 
    {
     sim_done("simulator cmd from core");
   }

  else if (cmd == TENOS_CMD_WRCH)
    {
      char c = (char) arg0;
      extern bool putchar_backdoor_with_handles;
      if (putchar_backdoor_with_handles)
	{
	  cout << "Putchar backdoor char '" << c << "'\n";
	}
      else std::cout << c << std::flush; // puts a character in GPR3 lower 8 bits
    }

  else if (cmd == TENOS_CMD_TRACE100)
    {
     busaccess.trace100("simulator cmd from core");
   }

  return 0;
}


void OR1200V::stat_report(const char *msg, FILE *fd, bool resetf)
{
  const char *p = m_core_period.to_string().c_str();
  if (fd) fprintf(fd, "%s %s  verilated: or1k core clock period=%s ipc=%f\n", name(), msg, p, m_average_ipc);
  busaccess.stat_report(msg, fd, resetf);
  if (resetf) stats.reset();
}

//TENOS_KIND_DEFINITION(OR1200V)

// eof
