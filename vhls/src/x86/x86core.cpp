// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>

#include "x86core.h"

#include "systemc.h"
#include <kernel.h>
#include <decode.h>
#include <osbackdoor.h>


// ?? do we need it? (MP)
const int enable_asym = 5;

extern int g_full_traces;
extern BasicBlockCache bbcache;

// USE A STATIC ARRAY REQUIRES ALL CPUS TO HAVE SAME MEMORY MAP FOR IFETCH SURELY?
// for not maximum image size is 2MB but no particular
// reason for this and it can be easily extended (MP - 21May13)
extern const int MAX_IMAGE;
const int MAX_IMAGE = 1024 * 1024 * 2;
byte executable_image[MAX_IMAGE];

#include <seqcore.cpp>

bool exec_image::ip_write(u64_t a, u8_t d, bool es) {
  executable_image[a] = d;

  return true;
}

bool exec_image::ip_read(u64_t a, u8_t& d) {
  // do nothing
  return true;
}

#define X86_ENDIAN_FLIP (0) // Same as modelling workstation: both are x86.

// Constructor
x86core::x86core(sc_core::sc_module_name names,
		 u8_t pID,
		 int stopctrl)
  : COREISA_IF(names, pID, X86_ENDIAN_FLIP, true, pID * enable_asym),
  bb_transop_index(0),
  current_bb(null)
 {
   sc_time fsb_period(1.0/2.7e9, SC_SEC);
   set_clock_frequencies(fsb_period, fsb_period);
   cycles = 0;
   corestats.reset();
   SC_THREAD(run);

  char txt[20];
  sprintf(txt, "x86core_%d", procID);
  char *stxt = strdup(txt);
  
  // Create a new context
  Context *ctx = new Context;
  ctx->reset();
  ctx->use64 = true;//djg temp start in 64 bit mode
  ctx->vcpuid = pID;
  ctx->mycore = this;
  ctx->bdoor = new osbackdoor(this);
  if(g_full_traces)
    busaccess.start_trace();

  stop_sim_on_exit = (stopctrl == 1) ? 1: (stopctrl == 0) ? (procID == 0) : 0;
  printf("Made %s %s (stop_sim_on_exit=%i)\n", txt, name(), stop_sim_on_exit);
  me = new SequentialCore(*ctx, this);
  im = new exec_image(this);

  new tenos_report_item(stxt, "pauses", &corestats.pauses);
  new tenos_report_item(stxt, "uops", &corestats.uops);
  new tenos_report_item(stxt, "insns", &corestats.insns);
}

bool* x86core::connect_interrupt(bool* c) {
  // Empty for now (MP)
  return c;
}

void x86core::stat_report(const char *msg, FILE *fd, bool resetf) {

  const char *p = m_core_period.to_string().c_str();
  if (fd) fprintf(fd, "%s %s  period=%s ipc=%f\n", name(), msg, p, m_average_ipc);
  fprintf(fd, "%s  %lli seq_total_user_insns_committed\n", name(), me->seq_total_user_insns_committed);
  fprintf(fd, "%s  %li cycles\n", name(), cycles);
  fprintf(fd, "%s  %li pauses\n", name(), corestats.pauses);
  fprintf(fd, "%s  %li insns\n", name(), corestats.insns);
  fprintf(fd, "%s  %li uops\n", name(), corestats.uops);
  fprintf(fd, "%s  %li basicblocks\n", name(), corestats.basicblocks);
  busaccess.stat_report(msg, fd, resetf);

  if (resetf) corestats.reset();

}


u64_t x86core::install_image(const char* image) {
  cbgelf* eloader = new cbgelf(im);
  bool big_endian;
  u64_t hwm = eloader->reader(big_endian, image, 0, MAX_IMAGE, 0);
  printf("%s: image_hwm 0x%lx\n", name(), hwm);
  return hwm;
}

void x86core::reset(bool selfstart) {
  halted = !selfstart;
  busaccess.trace_msg("RESET");

  me->reset_fetch(0x100);
}

void osbackdoor::os_svc_exit(int arg)
{
  x86core *core = (x86core *)_x86core;
  printf("Core %s os_svc_exit\n", core->name());
  core->busaccess.core_finished = true; // no setter for this ? should be simulator_cmd.
}

int osbackdoor::os_cmd(char cmd, char *data, long long int addr, int len)
{
  x86core *core = (x86core *)_x86core;
  switch (cmd)
    {
    case 'w':
      {
	for (int i=0; i<len; i++) core->busaccess.set_mem8(addr+i, data[i], 0, core->lt_d_delay);
	break;
      }

    case 'r':
      {
	for (int i=0; i<len; i++) 
	  {
	    u8_t d;
	    core->busaccess.eval_mem8(addr+i, d, 0, core->lt_d_delay);
	    data[i] = d;
	  }
	break;
      }
    case 's': // Copy a string
      {
	for (int i=0; i<len; i++) 
	  {
	    u8_t d;
	    core->busaccess.eval_mem8(addr+i, d, 0, core->lt_d_delay);
	    data[i] = d;
	    if (!d) break;
	  }
	break;
      }

    }
  return -1;
}


int x86core::simulator_cmd(int cmd, int arg0, int arg1, u32_t *rp)
{
  if (cmd == TENOS_CMD_SIM_DONE)
    {
      printf("EXIT %i\n", procID);
      master_runahead.force_sync();
      //if (sc_time_stamp() == sc_core::sc_time_stamp::sc_time_max()) std::cout <<  name() << ": Core exited at max SystemC time\n";
      std::cout <<  name() << ": Core exited at " << sc_time_stamp() << std::endl;
      if (this->stop_sim_on_exit) busaccess.sim_done("Primary/boot core exited"); // Stop simulation on this core exit
    }

  return 0;
}


// Main model process loop
void x86core::run() 
{
  POWER3(pw_energy uop_energy = pw_energy(200.0, PW_pJ));
  busaccess.core_finished = false;
  while (!busaccess.core_finished) {
    if (halted)
      {
	//m_qk.set(sc_time(1.0, SC_US)); // for now
	master_runahead += sc_time(1.0, SC_US);
	//m_qk.sync();
	master_runahead.perhaps_sync();
	continue;
      }

    cycles += 1;
    u64_t su = corestats.uops;
    //std::cout <<  name() << ": Core exec start at " << sc_time_stamp() + m_qk.get_current_time() << std::endl;

    int ret = me->execute();
    int uops = corestats.uops - su;
    //std::cout <<  name() << ": Core exec end at " << sc_time_stamp() + m_qk.get_current_time() << " after " << uops << std::endl;

    POWER3(record_energy_use(uops * uop_energy));
    if(ret == 2)
      busaccess.core_finished = true;
  }
  simulator_cmd(TENOS_CMD_SIM_DONE, 0, 0, 0);
}

// eof
