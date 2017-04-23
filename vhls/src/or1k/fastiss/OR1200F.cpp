// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>

//
// SystemC TLM wrapper for fast ISS version of OR1200 processor core.
// $Id: $
//
//

#include "systemc.h"
#include "OR1200F.h"
#include "llsc_extension.h"

#include "speedo_reg_mapping.h"


extern int g_full_traces; // please document

const int enable_asym = 5;


#define OR1K_ENDIAN_FLIP (7) // Not the same endian as the underlying workstation (x86).

// Constructor
OR1200F::OR1200F(sc_core::sc_module_name names, u8_t pID, bool harvardf):
  debug_controller(this),
  COREISA_IF(names, OR1K_ENDIAN_FLIP, pID, harvardf, pID*enable_asym),
  ins_fetcher(this),
  orsim(names)
{
#ifdef TLM_POWER3
  //  std::cout << "constructing OR1200F cpu_busaccess this=" << ((void *)&busaccess) << " pw_module_base=" << ((void *)dynamic_cast<pw_module_base *>(&busaccess)) << "\n";
  instruction_energy = pw_energy(200.0, PW_pJ);
  pw_power leakage = pw_power(10.0, PW_mW);
  set_static_power(leakage);
  set_fixed_area(pw_area(0.5, PW_sqmm)); // Without caches
#endif
  build_automaton();
  ext_interrupt=0;
  SC_THREAD(run);
  char txt[20];
  sprintf(txt, "OR1200F_CORE_%d", procID);
  where_last = 0;
  ext_interrupt = 0;
  reset(false);
  printf("Made %s %s\n", txt, name());
  //std::cout << "constructing 2/2 OR1200F cpu_busaccess this=" << ((void *)&busaccess) << " pw_module_base=" << ((void *)dynamic_cast<pw_module_base *>(&busaccess)) << "\n";
  if (g_full_traces) busaccess.start_trace();
}

void OR1200F::reset(bool selfstart)
{
  stall(/*unstall=*/selfstart);
  busaccess.trace_msg("RESET");
  core_reset();
}

// Elaboration phase: connect the interrupt wire to this core using this call.
bool *OR1200F::connect_interrupt(bool *c)
{
  bool *ov = ext_interrupt;
  ext_interrupt = c;
  return ov;
}



// Instruction mini-cache for other half of a 64 bit word.
void OR1200F::ins_fetcher_t::fetch(u32_t adr, u32_t &i, lt_delay &lt_delay)
{ 
  u32_t a1 = adr & ~7;
  if (a1 != cached_adr)
    {
      parent->busaccess.instr_fetch64(a1, data, 0, lt_delay);
      cached_adr = a1;
    }
  // Return big-endian order.
  i = (adr & 4) ? (data >> 0) : (data >> 32); 
}

//
// A debugger server for a CPU core.  Customised for the OR1K.
//
void OR1200F::serve_debugger()
{
  u32_t rd;
  switch (debug_cmd.cmd)
    {
    case Ack:
      break;
      
#define SERVER_TRC(X)
    case Control:
      switch(debug_cmd.data)
	{
	case ResetC:
	  printf("%s: Debugger 'reset' command\n", name());
	  core_reset();
	  stall(/*unstall=*/false); // ????
	  debug_cmd.cmd = Ack;
	  break;

	case RunC:
	  printf("%s: Debugger 'run' command\n", name());
	  stall(/*unstall=*/true);
	  debug_cmd.cmd = Ack;
	  break;

	case PauseC:
	  printf("%s: Debugger 'pause' command\n", name());
	  stall(/*unstall=*/false);
	  debug_cmd.cmd = Ack;
	  break;

	case SingleStepC:
	  assert(0); // for now - need to call remotecontrol...

	default: 
	  printf("%s: Ignored debugger control command %i\n", name(), (int)debug_cmd.data);
	  debug_cmd.cmd = Ack;
	  break;
	}
      break;

      
    case ReadMem:
      {
	u32_t rd32 = eval_mem32(debug_cmd.addr, 0);
	u8_t rd8 = rd32; // GDB does byte reads only?
	SERVER_TRC(printf("%s: Debug read mem 0x%x  %02X\n", name(), debug_cmd.addr, rd8));
	debug_cmd.data = rd8;
	debug_cmd.cmd = Ack;
      }
      break;
      
    case WriteMem:
      busaccess.set_mem32(xlat32to64(debug_cmd.addr), debug_cmd.data, 0, lt_d_delay);
      SERVER_TRC(printf("%s: Debugger write mem command a=%x d=%x\n", name(), (int)debug_cmd.addr, (int)debug_cmd.data));
      debug_cmd.cmd = Ack;
      break;

    case WriteReg:
      printf("%s: write reg command GPR$%x = 0x%x\n", name(), (int)debug_cmd.addr, (int)debug_cmd.data);
      set_reg(debug_cmd.addr, debug_cmd.data);
      debug_cmd.cmd = Ack;
      break;


    case ReadReg:
      debug_cmd.data = get_reg(debug_cmd.addr);
      SERVER_TRC(printf("%s: ReadReg GPR$%x = 0x%08x\n", name(), (int)debug_cmd.addr, (int)debug_cmd.data));
      debug_cmd.cmd = Ack;
      break;


    case Status:
      switch(debug_cmd.data)
	{
	case RunC:
	  SERVER_TRC(printf("%s: Debugger 'run' get status\n", name(), debug_cmd.data));
	  debug_cmd.addr = debug_controller.get_sigval(); // When stalled (halted) we want to return the signal that stopped us as r2.
	  debug_cmd.data = is_halted(); // Return non-zero if stalled waiting for debug
	  debug_cmd.cmd = Ack;
	  break;

	default: 
	  printf("%s: Ignored debugger status command %i\n", name(), (int)debug_cmd.data);
	  debug_cmd.cmd = Ack;
	  break;
	}
      break;
      
      
    case WriteSpr:
      {
	uorreg_t nv = (int)debug_cmd.data;
	printf("debugger SPR spr$%x write  data=0x%x\n", (int)debug_cmd.addr, nv); 
	set_spr(debug_cmd.addr, nv);
	debug_cmd.cmd = Ack;
      }
      break;

    case AndSpr:
      {
	uorreg_t nv = (int)debug_cmd.data & get_spr(debug_cmd.addr);
	printf("debugger SPR spr$%x AND update  data=0x%x\n", (int)debug_cmd.addr, nv); 
	set_spr(debug_cmd.addr, nv);
	debug_cmd.cmd = Ack;
      }
      break;
	
    case OrSpr:
      {
	uorreg_t nv = (int)debug_cmd.data | get_spr(debug_cmd.addr);
	printf("debugger SPR spr$%x OR update  data=0x%x\n", (int)debug_cmd.addr, nv); 
	set_spr(debug_cmd.addr, nv);
	debug_cmd.cmd = Ack;
      }
      break;

    case ReadSpr:
      debug_cmd.data = get_spr(debug_cmd.addr);
      SERVER_TRC(printf("debugger ReadSpr spr$%x gave %x\n", (int)debug_cmd.addr, (int)debug_cmd.data));
      debug_cmd.cmd = Ack;
      break;

    case ReadMeta:
      debug_cmd.data = 0x00010201; //  "ELF 32-bit MSB executable, OpenRISC, version 1 (SYSV), statically linked, not stripped"
      SERVER_TRC(printf("debugger read OR1K meta %x gave %x\n", (int)debug_cmd.addr, (int)debug_cmd.data));
      debug_cmd.cmd = Ack;
      break;

    default:
      printf("unexpected cmd %i\n", debug_cmd.cmd);
      assert(0); // for now
    }

  assert(debug_cmd.cmd == Ack);
}

// Main model process loop
void OR1200F::run()
{
  over = false;
  while(!over)
    {
      if (debug_op_requested()) serve_debugger();

      //if (m_qk.need_sync()) m_qk.sync();   // Keeper synchronize when quantum is reached
      if (ext_interrupt && *ext_interrupt) // Level-sensitive interrupt.
	{
	  if (busaccess.traceregions && busaccess.traceregions->check(0, TENOS_TRACE_CPU_INTERRUPTS))
	    printf("%s: %s: INTERRUPT %i\n", name(), kind(), procID); // This is an unmasked version of the interrupt signal.
	  hw_interrupt("orsim_sc");
	}
      if (is_halted())
	{
	  //old m_qk.sync();
	  wait(1, SC_US); // must poll for interrupt in halted TODO make an event
	  continue;
	}
      //busaccess.tick();
      lt_i_delay = master_runahead;
      lt_d_delay = master_runahead;
      if (reset_or_yield_countdown > 0) reset_or_yield_countdown -= 1;
      else 
	{
	  stall(!step());
	  POWER3(record_energy_use(instruction_energy));
	}

      master_runahead += m_effective_instruction_period;  //core_period modified by nominal IPC.
      master_runahead << lt_d_delay; // Join with any external operations on the d-cache
      master_runahead << lt_i_delay; // Join with any external operations on the i-cache

      // Retire join: take maximum of internal and external delays at join.
#if 0
      if (0)
	{
	  cout << "kernel=" << sc_time_stamp() << " start=" << ins_start << "  ins_end=" << ins_end << "\n";
	  cout << name () << " lt_busdelay was i=" << lt_i_delay << "  d=" << lt_d_delay << "\n";
	  cout << name () << " core_period=" << m_core_period << " ins_end was " << ins_end << " retire_time=" << retire_time << "  new delay=" << retire_time-sc_time_stamp() << "\n";
	  //std::cout << "Retire at " << retire_time << "\n";
	}
#endif

      //m_qk.set(retire_time-sc_time_stamp());


      oraddr_t pc = get_pc(); // peek_into_itlb(cpu_state.iqueue.insn_addr);
      if (busaccess.traceregions)
	{
	  int m = busaccess.traceregions->check(pc, TENOS_TRACE_CPU_KEY_REGS|TENOS_TRACE_CPU_ALL_REGS);
	  if (m) dumpreg(stdout, m & TENOS_TRACE_CPU_ALL_REGS);
	}
    }
  //m_qk.sync();
}






uint32_t OR1200F::eval_insn(unsigned int memaddr, int *bp)
{
  uint32_t rdata;
  ins_fetcher.fetch(memaddr, rdata, harvardf ? lt_i_delay: lt_d_delay);  //Service instruction fetch charging delay to appropriate account.x
  if (busaccess.traceregions && busaccess.traceregions->check(memaddr, TENOS_TRACE_CPU_IFETCH))  
    {
      char line[132];
      if (argv_backdoor_su) argv_backdoor_su->symbolic_dis64(line, 132, memaddr);
      else snprintf(line, 132, " %X ", memaddr);

      disassemble_insn(rdata);
      printf("%s:%s: Fetch insn a=%s ins=%x %s\n", name(), kind(), line, rdata, disassembled);
    }
  where_last = memaddr;
  return rdata;
}


void OR1200F::sim_done(const char *msg)
{
  busaccess.sim_done(msg);
  over = 1;
}

int OR1200F::simulator_cmd(int cmd, int arg0, int arg1, u32_t *rp)
{
  if (cmd == TENOS_CMD_SIM_DONE) sim_done("simulator cmd from core");

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

void OR1200F::corepause(int microseconds, u32_t addr)  // Pause CPU for this time interval: this makes it take less static and dynamic power for this interval.
{
  if (microseconds < 0 || microseconds > 2) microseconds = 2;
  sc_time pi = sc_time(microseconds, SC_US);
  u64_t new_delperiods = pi / m_core_period;

  if (busaccess.traceregions && busaccess.traceregions->check(busaccess.lastfetch, TENOS_TRACE_CPU_IFETCH))  
    {
      printf("%s:%s: Pause %i microseconds (%li core periods)\n", name(), kind(), microseconds, new_delperiods);
    }
  //printf("%s:%s: Pause %i microseconds (%i core periods)\n", name(), kind(), microseconds, new_delperiods);
  log_delperiods(new_delperiods, addr);
  // We need to implement the halt and interrupt material as well...
  //m_qk.sync();
  wait(pi);
}




void OR1200F::atomic_prefix() // Prefix following load/store pair as atomic.
{
  //printf("%s: atomic\n", name());
  busaccess.atomic_prefix(); // Trigger atomic transaction state machine.
}

int OR1200F::eval_bdoor_read(oraddr_t memaddr, u32_t &rdata)
{
  lt_delay &delay = lt_d_delay;
  #include "../backdoor_reads.C"
}

int OR1200F::eval_bdoor_write(oraddr_t memaddr, u32_t wdata)
{
  lt_delay &delay = lt_d_delay;
  #include "../backdoor_writes.C"
}


u32_t OR1200F::eval_mem32(oraddr_t memaddr, int*)
{
  u32_t rdata;
  if (eval_bdoor_read(memaddr, rdata)) return rdata;
  busaccess.eval_mem32(xlat32to64(memaddr), rdata, 0, lt_d_delay);
  return rdata;
}

 
u16_t OR1200F::eval_mem16(oraddr_t memaddr, int*)
{
  u32_t rd;
  u16_t rdata;
  if (eval_bdoor_read(memaddr, rd)) 
    {
      rdata = rd;
      rdata >>= ((memaddr & 2) ^ 2) * 8;
    }
  else busaccess.eval_mem16(xlat32to64(memaddr), rdata, 0, lt_d_delay);
  return rdata;
}

u8_t OR1200F::eval_mem8(oraddr_t memaddr, int*)
{
  u32_t rd; u8_t rdata;
  if (eval_bdoor_read(memaddr, rd)) 
    {
      u32_t rd1 = rd;
      rd1 >>= ((memaddr & 3) ^ 3) * 8;
      rdata = rd1;
    }
  else busaccess.eval_mem8(xlat32to64(memaddr), rdata, 0, lt_d_delay);
  return rdata;
}


void OR1200F::set_direct8(oraddr_t a, u8_t d, int, int)
{
  assert(0);
}

u8_t OR1200F::eval_direct8(oraddr_t a, int, int)
{
  assert(0);
}

void OR1200F::set_mem32(oraddr_t memaddr, u32_t wd, int*)
{
  if (eval_bdoor_write(memaddr, wd)) { /* .. */ }
  else busaccess.set_mem32(xlat32to64(memaddr), wd, 0, lt_d_delay);
}

void OR1200F::set_mem16(oraddr_t memaddr, u16_t wd, int*)
{
  busaccess.set_mem16(xlat32to64(memaddr), wd, 0, lt_d_delay);
}

void OR1200F::set_mem8(oraddr_t memaddr, uint8_t wd, int*)
{
  busaccess.set_mem8(xlat32to64(memaddr), wd, 0, lt_d_delay);

}


void OR1200F::stat_report(const char *msg, FILE *fd, bool resetf)
{
  const char *p = m_core_period.to_string().c_str();
  if (fd) fprintf(fd, "%s %s  fastiss: or1k core clock period=%s ipc=%f\n", name(), msg, p, m_average_ipc);
  core_stat_report(msg, fd, resetf);
  busaccess.stat_report(msg, fd, resetf);
  if (resetf) stats.reset();
  
}

// TENOS_KIND_DEFINITION(OR1200F)

// eof
