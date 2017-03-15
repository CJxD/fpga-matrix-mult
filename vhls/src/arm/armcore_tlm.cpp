// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>


//
// TLM wrapper around the ARM ISS - no caches in here.
//
#include "armcore_tlm.h"
#include "arm_mmu.h"

#include "systemc.h"
#include "armcore_tlm.h"
#include "llsc_extension.h"

#define SHUTDOWN_WAIT 20

extern int g_full_traces;

const int enable_asym = 5;



#define SERVER_TRC(X) 

#define xlat32to64(X) X

#define BDTRC(X) 

#define BACKDOOR_TRACE_ENABLED(X)  (busaccess.traceregions && busaccess.traceregions->check(0, TENOS_TRACE_IO_READ|TENOS_TRACE_IO_WRITE))

uint8_t armcore_tlm::armisa_write1(uint32_t vmemaddr, uint64_t wdata, u2_t size_code, u32_t *scp)  // Return 0 on ok operation
{
  u32_t memaddr = vmemaddr;
  u8_t success = 1;

  sc_time &delay = lt_d_delay;
#include "backdoor_writes.C"

  u64_t a64 = xlat32to64(memaddr);

  // For a store extended, scp denotes the answer place and a 1 indicates success when returned via scp.
  switch (size_code)
    {
    case 0:
      if (scp) 
          *scp = busaccess.cpu_mem_store_conditional(a64, wdata, 8, lt_d_delay);
      else 
          success = busaccess.set_mem8(a64, wdata, 0, lt_d_delay);

      break;      
    case 1:
      if(a64 & 1) {
          // TODO: not implemented, as there were no cases seen so far
          assert(!scp);
          success = busaccess.set_mem8(a64, (wdata & 0xFF), 0, lt_d_delay);
          if(success)
              success = busaccess.set_mem8(a64+1, ((wdata & 0xFF00) >> 8), 0, lt_d_delay);
      }
      else {
          if (scp) 
              *scp = busaccess.cpu_mem_store_conditional(a64, wdata, 16, lt_d_delay);
          else 
              success = busaccess.set_mem16(a64, wdata, 0, lt_d_delay);
      }

      break;

    case 2: // int 32
      if(a64 & 0x3) {
          //printf("Non word-aligned store for address %08x, instruction %08x, data 0x%08x\n", a64, insAddress, wdata);
          // atomic write to non-aligned memory should not happen
          assert(!scp);

          // We are doing copying by byte, this is not efficient,
          // better approach would be to have direct access to mem_write()
          // method in cpu_busaccess do 64bit write using a mask
          success = busaccess.set_mem8(a64, (wdata & 0xFF), 0, lt_d_delay);
          if(success)
              success = busaccess.set_mem8(a64+1, ((wdata & 0xFF00) >> 8), 0, lt_d_delay);
          if(success)
              success = busaccess.set_mem8(a64+2, ((wdata & 0xFF0000) >> 16), 0, lt_d_delay);
          if(success)
              success = busaccess.set_mem8(a64+3, ((wdata & 0xFF000000) >> 24), 0, lt_d_delay);
      } else {
          if (scp) 
              *scp = busaccess.cpu_mem_store_conditional(a64, wdata, 32, lt_d_delay);
          else
              success = busaccess.set_mem32(a64, wdata, 0, lt_d_delay);
      }

      break;

    case 3: //doubleword

      if (a64 & 4) // if unaligned split in two
	{

	  u32_t rdlo = (wdata << 32) >> 32; // Why do this?
	  u32_t rdhi = (wdata >> 32);
	  assert(!(a64 & 3));
	  /*
	    success = armisa_write1(memaddr, rdlo, 2, scp);
	    success = armisa_write1(memaddr+4, rdhi, 2, scp);
	  */
	  if (scp) 
	    *scp = busaccess.cpu_mem_store_conditional(a64, rdlo, 32, lt_d_delay);
	  else 
	    success = busaccess.set_mem32(a64, rdlo, 0, lt_d_delay);
	  if (scp) 
	    *scp = busaccess.cpu_mem_store_conditional(a64+4, rdhi, 32, lt_d_delay);
	  else
	    success = busaccess.set_mem32(a64+4, rdhi, 0, lt_d_delay);
	}
      else
	{
	  if (scp) 
              *scp = busaccess.cpu_mem_store_conditional(a64, wdata, 64, lt_d_delay);
          else
              success = busaccess.set_mem64(a64, wdata, 0, lt_d_delay);
	}

      break;
    default:
      assert(0);
    }

  return success;  // Return 1 on ok ARM write operation - return codes being lost?
}

bool armcore_tlm::armisa_read1(u32_t vmemaddr, u2_t size_code, u1_t instructionf,  bool linkedf, bool mmu)
{
  u32_t memaddr = vmemaddr;
  bool success = true;

  
  if (memaddr == 0xFFFFfffc)
    {

      // spurious read on reset - please tidy up
      read_data = 0;
      return 1;
    }
  if (instructionf)
    {
        success = eval_insn(memaddr, 0, read_data, size_code);
    }
  else
    {
      sc_time &delay = lt_d_delay;
#include "backdoor_reads.C"
      u64_t a64 = xlat32to64(memaddr);
      switch(size_code)
	{
	case 0: // One byte reads.
	  {
	    u8_t r;
	    success = busaccess.eval_mem8(a64, r, 0, lt_d_delay, linkedf);
	    read_data = r;
	    break;
	  }

	case 1: // short 
	  {
              if(a64 & 1) {
                  u8_t r1;
                  success = busaccess.eval_mem8(a64, r1, 0, lt_d_delay, linkedf);
                  u8_t r2;
                  success = busaccess.eval_mem8(a64+1, r2, 0, lt_d_delay, linkedf);
                  
                  read_data = ((u16_t)(r2 << 8)) | r1;
                  
              }
              else { 

                  u16_t r;
                  success = busaccess.eval_mem16(a64, r, 0, lt_d_delay, linkedf);
                  read_data = r;
              }
              break;
	  }
	case 2:// 32 bit reads.
	  {
              // check to see if load is not word-aligned
              if(a64 & 0x3) {
                  // It is not aligned so we are going to do two reads 
                  // and then combine them

                  u32_t part1;
                  success = busaccess.eval_mem32(a64&~3, part1, 0, lt_d_delay, linkedf);
                  u8_t shift1 = (a64&3)*8;
                  part1 = part1 >> shift1;
                  u32_t part2;
                  success = busaccess.eval_mem32((a64&~3)+4, part2, 0, lt_d_delay, linkedf);
                  u8_t shift2 = 32 - 8*(a64&3);
                  part2 = part2 << shift2;
                  read_data = part1 | part2;

                  //printf("Non-word aligned load @ %08x -- data @ %08x is 0x%08x and @ %08x is 0x%08x; combined 0x%08x --> %08x\n",  
                  //     a64, a64&~3, part1, (a64&~3)+4, part2, read_data, insAddress);

              }
              else {
                  // word-aligned read
                  success = busaccess.eval_mem32(a64, read_data, 0, lt_d_delay, linkedf);
              }
              
              break;
	  }
	  

	case 3: // 64 bit reads (doubleword).
	  if (a64 & 4) {// if unaligned 
	    u32_t r0, r1;
	    assert(!(a64 & 3));
	    /*
	    success = armisa_read1(memaddr, 2, instructionf, linkedf, mmu);
	    r0 = read_data;
	    success = armisa_read1(memaddr+4, 2, instructionf, linkedf, mmu);
	    r1 = read_data;
	    */
	    success = busaccess.eval_mem32(a64, r0, 0, lt_d_delay, linkedf);
	    success = busaccess.eval_mem32(a64+4, r1, 0, lt_d_delay, linkedf);
	  
	    read_data64 = r0 | (((u64_t)r1) << 32); // little-endian word combine
	  }
	  else {
	    success = busaccess.eval_mem64(a64, read_data64, 0, lt_d_delay, linkedf);
	  }
	  break;

	default:  // other width?
	  printf("Bad instruction %x size_code=0x%x\n", Reg(PcReg), size_code);
	  assert(0);
	}
    }
  
  if(!success) {
      return 0;
  }

   return 1; // Arm ISS wants a one for an ok READ.  Answer must be in the field 'read_data'.
}

u1_t armcore_tlm::wbuf_logged()
{
  return false;
}

 uint32_t armcore_tlm::copro_read(u4_t cpn, u4_t cpr, u4_t cpm, u3_t op1, u3_t op2)
{
  if(cpn != 15 && cpn != 14) {
      printf("Coprocessor %d is not yet implemented! Read from instruction %08x -- %08x\n", cpn, insAddress, instruction);
      assert(0);
  }

  cp_request_extension ext(cpr, cpm, op1, op2);
  u32_t data;

  PW_TLM_PAYTYPE trans;
  trans.set_extension(&ext);
  trans.set_byte_enable_length(0);
  trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
  trans.set_read();
  trans.set_address(0);
  trans.set_data_length(4);
  trans.set_data_ptr((unsigned char*) &data);

  sc_time delay = SC_ZERO_TIME;
  switch(cpn) {
  case 14:
      cp14_port->b_transport(trans, delay);
      break;
    case 15:
      cp15_port->b_transport(trans, delay);
      break;
    default:
      assert(0); // should not reach here
  }

  trans.clear_extension<cp_request_extension>(&ext);

  return data;
}

void armcore_tlm::copro_write(u4_t cpn, u4_t cpr, u4_t cpm, u32_t data, u3_t op1, u3_t op2)
{
  if(cpn != 15 && cpn != 14) {
    printf("Coprocessor %d is not yet implemented!\n", cpn);
    assert(0);
  }

  cp_request_extension ext(cpr, cpm, op1, op2);

  PW_TLM_PAYTYPE trans;
  trans.set_extension(&ext);
  trans.set_byte_enable_length(0);
  trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
  trans.set_write();
  trans.set_address(0);
  trans.set_data_length(4);
  trans.set_data_ptr((unsigned char*) &data);

  sc_time delay = SC_ZERO_TIME;
  switch(cpn) {
    case 14:
      cp14_port->b_transport(trans, delay);
      break;
    case 15:
      cp15_port->b_transport(trans, delay);
      break;
    default:
      assert(0); // should not hit this
  }
  trans.clear_extension<cp_request_extension>(&ext);
}



void armcore_tlm::recompute_pvt_parameters() // This callback invoked by POWER3 when supply voltage changed.
{

#ifdef TLM_POWER3
  //  std::cout << "constructing armcore_tlm cpu_busaccess this=" << ((void *)&busaccess) << " pw_module_base=" << ((void *)dynamic_cast<pw_module_base *>(&busaccess)) << "\n";
  m_instruction_energy = pw_energy(200.0, PW_pJ); // 817 core total as measured on parallella card at 1 volt.

  m_mispredict_energy_penalty = 5 * m_instruction_energy; // Wild guess for now.

  pw_power leakage = pw_power(10.0, PW_mW);
  set_static_power(leakage);
#endif
}


// Constructor
armcore_tlm::armcore_tlm(sc_core::sc_module_name names, u8_t pID, bool harvardf):
  COREISA_IF(names, 0 /*endian flip*/, pID, harvardf, pID*enable_asym), // INTRODUCE CORE ASYM
  armisa(pID, names),
  ins_fetcher(this)
{
  gdb_warning_issued = false;

#ifdef TLM_POWER3
  set_fixed_area(pw_area(0.25, PW_sqmm)); // Area of core without caches
#endif
  recompute_pvt_parameters();
  iss_reset();
  over = false;
  lt_i_delay = SC_ZERO_TIME;
  lt_d_delay = SC_ZERO_TIME;

#ifdef BSYSTEM
  pthreadDelegate<armcore_tlm> *delegate = new pthreadDelegate<armcore_tlm> (this, &armcore_tlm::run);
  pthread_create(&pth, 0, pthread_delegate_routine<pthreadDelegate<armcore_tlm> >, (void*)delegate);
  SC_THREAD(run_proxy);
#else
  SC_THREAD(run);
#endif

  char txt[20];
  sprintf(txt, "armcore_tlm_CORE_%d", procID);
  printf("Made %s %s\n", txt, name());
  //std::cout << "constructing 2/2 armcore_tlm cpu_busaccess this=" << ((void *)&busaccess) << " pw_module_base=" << ((void *)dynamic_cast<pw_module_base *>(&busaccess)) << "\n";


  if (busaccess.traceregions || g_full_traces) notify_tracing_might_be_needed(true); // TODO -this is temporary - change traceregions to be an interface where this behaviour is automatic
  if (g_full_traces) busaccess.start_trace();  
}


// Unimplemented instructions and so on should cause a diagnostic report over all cores
// where report entries are generated by invoking this method.
void /*override*/armcore_tlm::diagnostic_report(FILE *fd, int severity, const char *msg)
{
  printf("report %p %i %p\n", fd, severity, msg);
  printf("%s: %s: diagnostic dumpregs\n", name(), msg);
  PrintRegs(fd, 1, true);
}

void armcore_tlm::reset(bool selfstart)
{
  if (busaccess.traceregions || g_full_traces) notify_tracing_might_be_needed(true); // TODO -this is temporary - change traceregions to be an interface where this behaviour is automatic
  iss_reset();
  halted = !selfstart;
  busaccess.trace_msg("RESET");

  printf("%s: Reset. selfstart=%i, CPSR=%08x\n", name(), selfstart, Reg(CpsrReg));
}


#if OLDER

// We now use an sc_in<bool>

// Elaboration phase: connect the interrupt wire to this core using this call.
bool *armcore_tlm::connect_interrupt(bool *c)
{
  bool *ov = ext_interrupt;
  ext_interrupt = c;
  return ov;
}
#endif


// Instruction mini-cache for other half of a 64 bit word.
bool armcore_tlm::ins_fetcher_t::fetch(u32_t adr, u32_t &i, sc_time &lt_delay)
{ 
  bool ok = true;
  u32_t a1 = adr & ~7;
  if (a1 != cached_adr)
    {
      ok = parent->busaccess.instr_fetch64(a1, cached_ins, 0, lt_delay);
      cached_adr = a1;
    }
#if 0
  // Return big-endian order.
  i = (adr & 4) ? (cached_ins >> 0) : (cached_ins >> 32); 
#else
  // Return little-endian order. (Same as underlying x86).
  i = (adr & 4) ? (cached_ins >> 32) : (cached_ins >> 0); 
#endif

  return ok;
}


void armcore_tlm::lt_all_sync() // Roll all loosely-timed metrics down to accounting substrates.
{
  if (m_branch_predictor)
    {
      u64_t penalties = m_branch_predictor->collect_penalties();

      if (penalties)
	{
	  m_qk.inc(penalties * m_mispredict_time_penalty);
	  POWER3(record_energy_use(m_mispredict_energy_penalty * penalties));
	}
    }
  m_qk.sync();
}

// Main ISS process loop for the ARM model.
void armcore_tlm::run()
{
  int shutdown_requests = 0;
  over = false;
  while(!over)
    {

      if (debug_op_requested()) serve_debugger();

      //printf("%s: Tick pc=0x%x  state=%i %s\n", myname(), get_pc(), runstate, state_string(runstate));

      if (m_qk.need_sync()) lt_all_sync();   // Keeper synchronize when quantum is reached
      if (IRQ.read()) // Level-sensitive interrupt.
	{
            if(wait_for_interrupt) {
                //printf("Going to enable core %d now after WFI\n", m_core_no);
                wait_for_interrupt = false;
            }

            if(wait_for_event && !IRQFlag()) {
                wait_for_event = false;
                //printf("Going to enable core %d now after WFE\n", m_core_no);
            }
            //printf("%s: %s: INTERRUPT %i\n", name(), kind(), procID); // This is an unmasked version of the interrupt signal.
	  hw_interrupt("armcore");
	}

      if(wait_for_event || !event_register) {
          u32_t value = inter_core_event->load();
          if((value & (MAX_NO_OF_CORES-1)) != (last_event & (MAX_NO_OF_CORES-1))
             || ((value >> LOG_MAX_NO_OF_CORES) != (last_event >> LOG_MAX_NO_OF_CORES))) {
              //printf("Core %d received event %d\n", m_core_no, event_in.read());
              if(wait_for_event) {
                  wait_for_event = false;
                  //printf("Going to enable core %d now after WFE\n", m_core_no);
              }
              else {
                  event_register = true;
              }
          }
      }

      if (halted || wait_for_interrupt || wait_for_event)
	{
	  lt_all_sync();
	  wait(1, SC_US); // must poll for interrupt or debugger command while halted.  TODO make an sc_event ?
	  continue;
	}

      sc_time ins_start = m_qk.get_current_time(); // This is local_time+sc_time_stamp()
      lt_i_delay = SC_ZERO_TIME; //
      lt_d_delay = SC_ZERO_TIME; //ins_start; // SC_ZERO_TIME; //
      if (reset_or_yield_countdown > 0) reset_or_yield_countdown -= 1;
      else 
	{
	  execute(); // Main ISS call.
	  POWER3(record_energy_use(m_instruction_energy));
	}      

      if(shutdown_sequence) {
          if(shutdown_requests > SHUTDOWN_WAIT) {
              // if there was no change in execution pattern
              // for SHUTDOWN_WAIT instructions we are going
              // to finish the simulation
              printf("%s: Shutting down core %d\n", name(), m_core_no);
              over = true;
          }
          shutdown_requests++;
      } else {
          shutdown_requests = 0;
      }
      sc_time d_end = lt_d_delay + sc_time_stamp(); // Bus cycle end time
      sc_time i_end = lt_i_delay + sc_time_stamp(); // Bus cycle end time
      sc_time ins_end = ins_start + m_effective_instruction_period;  //core_period modified by nominal IPC.

#if 0
      cout << name () << " ktime=" << sc_time_stamp() << "\n";
      cout << name () << " lt_busdelay was i=" << lt_i_delay << " d=" << lt_d_delay << "\n";
      cout << name () << " ext_end was d=" << d_end << " and i=" << i_end << "\n";
      //cout << name () << " ins_end was " << ins_end << "\n";
      cout << name () << " ins_start was " << ins_start << "\n";
      cout << name() << " iss: kernel=" << sc_time_stamp() << " start=" << ins_start << " d_del=" << (d_end-ins_start) << " i_del=" << (i_end-ins_start) << " core_del=" << (ins_end-ins_start) <<  " icount=" << stats.m_Instructions << "\n";
#endif

      // Retire join: take maximum of internal and external delays at join. - This is not modelling out of order execution within the Quantum or basic block.
#define TMAX(X, Y) ((X)>(Y)?(X):(Y))


      m_qk.set(TMAX(ins_end, TMAX(d_end, i_end))-sc_time_stamp());

      if (1 && busaccess.traceregions) // all tracing is inside the armisa for now. this does it every clock.
	{
	  u32_t pc = get_pc(); 
	  int m = busaccess.traceregions->check(pc, TENOS_TRACE_CPU_KEY_REGS|TENOS_TRACE_CPU_ALL_REGS);
	  if (m) 
	    {
	      //printf("%s site %i - debugmask=%x\n", __FILE__, __LINE__, m);
	      PrintRegs(stdout, m & TENOS_TRACE_CPU_ALL_REGS, true);
	    }
	}
    }
  lt_all_sync();

  if(m_core_no == 0)
      sim_done("Finished simulation\n");
}


u32_t armcore_tlm::check_tenos_trace(u32_t mask, u32_t pc) // See if debug tracing to the console is turned on.
{
  u32_t m = busaccess.traceregions ? busaccess.traceregions->check(pc, mask): 0; // Note order of args swaps here.
  return m;
}

bool armcore_tlm::eval_insn(u32_t memaddr, int *bp, u32_t& read_data, u8_t size_code)
{
  bool ok = ins_fetcher.fetch(memaddr, read_data, harvardf ? lt_i_delay: lt_d_delay);  //Service instruction fetch charging delay to appropriate account.x
  if(size_code == 1) {
      if(memaddr & 2)
          read_data >>= 16;
      read_data &= 0xFFFF;
  }
  if (ok && busaccess.traceregions && busaccess.traceregions->check(memaddr, TENOS_TRACE_CPU_IFETCH))  
    {
      char line[1024], disassembled[1024];
      if (argv_backdoor_su) argv_backdoor_su->symbolic_dis64(line, 1024, insAddress);
      else snprintf(line, 1024, " %X ", memaddr);
      
      bool print = dis.dis(memaddr, read_data, disassembled, thumb_mode);
      if(print)
          printf("%s:%s: Fetch insn a=%s %s\n", name(), kind(), line, disassembled);
    }
  return ok;
}


void armcore_tlm::sim_done(const char *msg)
{
  busaccess.sim_done(msg);
  over = 1;
}


int armcore_tlm::simulator_cmd(int cmd, int arg0, int arg1, u32_t *rp)
{
  //printf("simulator cmd %i\n", cmd);
  const char *msg = "simulator cmd from core";
  switch (cmd)
    {
    case TENOS_CMD_STATS_CHECKPOINT:
      {
	u32_t result_ = tenos_simulator_cmd(msg, cmd, arg0, arg1, rp);
      }
      break;

    case TENOS_CMD_SIM_DONE:
      sim_done(msg);
      break;

    case TENOS_CMD_TRACE100:
      busaccess.trace100(msg);
      break;	

    case TENOS_CMD_COREPAUSE: // Not a backdoor - need to find ARM proper instruction for this which is yield.
      iss_yield(-1); 
      break;

    case TENOS_CMD_CORE_HALT:
      halted = true; // reason for stopping stored in sigval in iss.
      break;
    
    case TENOS_CMD_WRCH: // A backdoor console write - not using UARTs etc..
      {
	char c = arg0;
	extern bool putchar_backdoor_with_handles;
	if (putchar_backdoor_with_handles)
	  {
	    cout << "Putchar backdoor char '" << c << "'\n";
	  }
	else std::cout << c << std::flush; // puts a character in GPR3 lower 8 bits
      }
      break;

    default:
      return -1;
    }

  return 0; // ok return code.
}

int armcore_tlm::arm_sim_command(int cmd, int arg0, int arg1, u32_t *rp)
{
  //printf ("arm_sim_command: %s: cmd=%i arg0=0x%x arg1=0x%x rp=%p (start)\n", name(), cmd, arg0, arg1, rp);
  int rc = simulator_cmd(cmd, arg0, arg1, rp); // plumb the two abstract interfaces
  //printf ("arm_sim_command: %s: cmd=%i arg0=0x%x arg1=0x%x rp=%p rc=%i rv=%i\n", name(), cmd, arg0, arg1, rp, rc, (rp)?*rp:-202);
  return rc;
}


void armcore_tlm::iss_yield(int cycles)  // Pause (yield) CPU for this time interval: this makes it take less static(?) and dynamic power for this interval and is used inside futex spins etc.
{
  if (cycles < 0) // negative denotes the default number - roughly a pipeline depth worth
    cycles = 4;
  reset_or_yield_countdown = cycles;

  if (busaccess.traceregions && busaccess.traceregions->check(busaccess.lastfetch, TENOS_TRACE_CPU_IFETCH))  
    {
      printf("%s:%s: Pause (%i core periods)\n", name(), kind(), cycles);
    }
  //printf("%s:%s: Pause %i microseconds (%i core cycles)\n", name(), kind(), cycles);
  //  stats.pauses += new_del .. log_delperiods(new_delperiods, addr);

  // TODO - drop static power while yielding to simulate clock gating.
}




void armcore_tlm::stat_report(const char *msg, FILE *fd, bool resetf)
{
  const char *p = m_core_period.to_string().c_str();
  if (fd) fprintf(fd, "%s %s  period=%s ipc=%f\n", name(), msg, p, m_average_ipc);
  stats.report(msg, fd, resetf);
  if (m_branch_predictor)
    {
      m_branch_predictor->stats.report(msg, fd, resetf);
    }
  if (m_abtb) m_abtb->stats.report(msg, fd, resetf);
  busaccess.stat_report(msg, fd, resetf);
  if (resetf) stats.reset();
}


void armcore_tlm::end_of_simulation(void) // Called by SystemC on exit.
{
  printf("%s: end of simulation PC=%x\n", name(), get_pc());
}

#define SERVER_DEBUGGER_TRC(X) X

void armcore_tlm::serve_debugger()
{
  u32_t d, rd, nv;
  switch (debug_cmd.cmd)
    {
    case Ack:
      break;
      
    case Control:
      switch(debug_cmd.data)
	{
	case ResetC:
	  SERVER_DEBUGGER_TRC(printf("%s: Debugger 'reset' command\n", name()));
	  iss_reset();
	  halted = true; // ? see self_start...
	  debug_cmd.cmd = Ack;
	  break;

	case RunC:
	  SERVER_DEBUGGER_TRC(printf("%s: Debugger 'run' command\n", name()));
	  halted = false;
	  debug_cmd.cmd = Ack;
	  break;

	case PauseC:
	  SERVER_DEBUGGER_TRC(printf("%s: Debugger 'pause' command\n", name()));
	  halted = true;
	  debug_cmd.cmd = Ack;
	  break;

	case SingleStepC:
	  SERVER_DEBUGGER_TRC(printf("%s: Debugger 'SingleStep' command\n", name()));
	  debug_cmd.cmd = Ack;
	  set_stop_after_count(1);
	  //set_breakpoint(get_pc(), SingleStep);
	  halted = false;
	  //printf("%s: set_breakpoint after one instruction\n", name());
	  break;

	default: 
	  printf("%s: Ignored debugger control command %i\n", name(), (int)debug_cmd.data);
	  debug_cmd.cmd = Ack;
	  break;
	}
      break;

      
    case ReadMem:
      {
	u64_t rd64;
	u32_t a = debug_cmd.addr;
	if ((a & 0xFFFFff00) == 0xFFFFff00)
	  {
	    rd64 = 0xDEADBEEF12345678; // 
	    if (!gdb_warning_issued)
	      {
		  gdb_warning_issued = true;
		  printf("Temporary bypass gdb's reads of these high addresses until we understand better addr=$%08x\n", a);
	      }
	  }
	else
	  {
	    busaccess.mem_read(xlat32to64(a & ~7), rd64, lt_d_delay);
	  }
	u8_t rd8 = rd64 >> (((debug_cmd.addr & 7))*8); // NOTE Litte ENDIAN CODE FOR ARM.
	SERVER_TRC(printf("%s: Debug read mem " PFX64 "  %02X\n", name(), debug_cmd.addr, rd8));
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
      d = (u32_t)debug_cmd.data;
      d = ((d>>24) & 0xFF) | ((d>>8) & 0xFF00) | ((d<<8) & 0xFF0000) | ((d<<24) & 0xFF000000);  // Litte endian convert
      // The RSP parser assumes bigendian so swap here.
      printf("%s: write reg command GPR$%x = 0x%x\n", name(), (int)debug_cmd.addr, d);
      set_reg(debug_cmd.addr, d);
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
	  debug_cmd.addr = get_sigval(); // When stalled (halted) we want to return the signal that stopped us as r2.
	  debug_cmd.data = halted; // Return non-zero if stalled waiting for debug
	  debug_cmd.cmd = Ack;
	  break;

	default: 
	  printf("%s: Ignored debugger status command %i\n", name(), (int)debug_cmd.data);
	  debug_cmd.cmd = Ack;
	  break;
	}
      break;
      
      
    case WriteSpr:
      d = (u32_t)debug_cmd.data;
      d = ((d>>24) & 0xFF) | ((d>>8) & 0xFF00) | ((d<<8) & 0xFF0000) | ((d<<24) & 0xFF000000);  // Litte endian convert
      // The RSP parser assumes bigendian so swap here.
      printf("debugger SPR spr$%x write  data=0x%x\n", (int)debug_cmd.addr, d); 
      set_spr(debug_cmd.addr, d);
      debug_cmd.cmd = Ack;
      break;

    case AndSpr:
      d = (u32_t)debug_cmd.data;
      d = ((d>>24) & 0xFF) | ((d>>8) & 0xFF00) | ((d<<8) & 0xFF0000) | ((d<<24) & 0xFF000000);  // Litte endian convert
      // The RSP parser assumes bigendian so swap here.
      nv = (int)debug_cmd.data & get_spr(debug_cmd.addr);
      printf("debugger SPR spr$%x AND update  data=0x%x\n", (int)debug_cmd.addr, nv); 
      set_spr(debug_cmd.addr, nv);
      debug_cmd.cmd = Ack;
      break;
	
    case OrSpr:
      d = (u32_t)debug_cmd.data;
      d = ((d>>24) & 0xFF) | ((d>>8) & 0xFF00) | ((d<<8) & 0xFF0000) | ((d<<24) & 0xFF000000);  // Litte endian convert
      // The RSP parser assumes bigendian so swap here.
      nv = (int)debug_cmd.data | get_spr(debug_cmd.addr);
      printf("debugger SPR spr$%x OR update  data=0x%x\n", (int)debug_cmd.addr, nv); 
      set_spr(debug_cmd.addr, nv);
      debug_cmd.cmd = Ack;
      break;

    case ReadSpr:
      debug_cmd.data = get_spr(debug_cmd.addr);
      SERVER_TRC(printf("debugger ReadSpr spr$%x gave %x\n", (int)debug_cmd.addr, (int)debug_cmd.data));
      debug_cmd.cmd = Ack;
      break;


    case ReadMeta:
      debug_cmd.data = 0x00010101; // "ELF 32-bit LSB executable, ARM, version 1 (SYSV), statically linked, not stripped"
      SERVER_TRC(printf("debugger read ARM meta %x gave %x\n", (int)debug_cmd.addr, (int)debug_cmd.data));
      debug_cmd.cmd = Ack;
      break;

    case InsertMatchpoint:
      {
	int mtype =  debug_cmd.data;
	char cmd = debug_cmd.lanes;
	u32_t addr = debug_cmd.addr;
	printf("Insert Matchpoint at %x mtype=%i cmd='%c'\n", addr, mtype, cmd);

	if (cmd == 'D')
	  {
	    delete_breakpoint(addr);
	  }
	else set_breakpoint(addr); // TODO mtype ignored - what about data watchpoints?

	debug_cmd.cmd = Ack;
	break;

      }

    default:
      printf("%s: unexpected debug cmd %i\n", name(), debug_cmd.cmd);
      assert(0); // for now
    }

  assert(debug_cmd.cmd == Ack);
}


// TENOS_KIND_DEFINITION(armcore_tlm)

// eof



