//
// 
// $Id: $
// Part of prazor-virtual-platform
// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2010-15 DJ Greaves, University of Cambridge, Computer Laboratory.
//
// General support code - all architecures
//

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "tenos.h"
#include "ptrace_v1.h"


static u64_t prazor_masks[64];
static bool prazor_masks_done=false;

struct watch_s Watches[MAXWATCH];

#if 0  
// temporarily removed because xlk is not defined in tenos.h and so this file won't compile!
volatile bool sim_overp() 
{
  extern bool sc_is_running();
  return !sc_is_running();
}


// temporarily removed because xlk is not defined in tenos.h and so this file won't compile!
void tenos_simcontrol::start() 
{ 
  xlk.lock(); 
  m_active_workers+=1; 
  xlk.unlock();
}

void tenos_simcontrol::stop() 
{ 
  xlk.lock(); 
  m_active_workers-=1; 
  xlk.unlock();
  cout << "No further active workers\n";
    if (m_active_workers == 0) sc_stop();
}
#endif

u64_t prazor_mask(int w)
{
  assert(w > 0 && w < 64);
  if (!prazor_masks_done)
    {
      prazor_masks[1] = 1;
      for (int i=2; i< 64; i++) prazor_masks[i] = prazor_masks[i-1]*2 + 1;
      prazor_masks_done=true;
    }
  return (prazor_masks[w]);
}

int bound_log2(u64_t d) // E.g. if d in 4 to 7 we return 3.
{
  int r = 0;
  while (d > 1LLU) { d /= 2LLU; r++; }
  return r;
}

int pow_2(uint64_t pow) {
  return 1 << pow;
}

#if 0
int bound_log2(u32_t d) // E.g. if d in 4 to 7 we return 3.
{
  int r = 0;
  while (d > 1) { d /= 2; r++; }
  return r;
}
#endif

void tenos_assert_fail(const char *iname, const char *m, const char*file, int line)
{
  printf("%s: assert \"%s\" failed at %s:%i\n", m, iname, file, line);
  exit(1);
}

void tenos_diagnostic_if::diagnostic_report(FILE *fd, int severity, const char *msg)
{
  std::cout << "Missing virtual method: diagnostic report " << msg << "\n";
}

void tenos_diagnostics_s::checkpoint(int severity, const char *where, const char *msg)
{
  std::cout << "diagnostic checkpoint:" << where << "\n";
  for(std::vector< tenos_diagnostic_if *>::const_iterator l_it = m_vec.begin(); l_it != m_vec.end(); ++l_it)
    { 
      (*l_it)->diagnostic_report(stdout, severity, msg);
    }
}


// This code implements a simple performance monitoring database for collecting and writing outputs.  The basic use mode is to grep in a folder of
// report files for CSV and to paste/import the grep output into a spreadsheet in CSV format or read in to R.

static tenos_report_item *g_tenos_report_items = 0;

// constructor 1/6
tenos_report_item::tenos_report_item(const char *name1, const char *name2,  const char *string_value) :
  name1(name1),
  name2(name2),
  string_value(string_value)
{
  u64_value = 0;
  float_value = 0;
  int_value = 0;
  u32_avalue = 0;
  u64_avalue = 0;
  next = g_tenos_report_items;
  g_tenos_report_items = this;
  snapshots = 0;
}

// constructor 2/6
tenos_report_item::tenos_report_item(const char *name1, const char *name2,  float *float_value) :
  name1(name1),
  name2(name2),
  float_value(float_value)
{
  u64_value = 0;
  string_value = 0;
  int_value = 0;
  u32_avalue = 0;
  u64_avalue = 0;
  next = g_tenos_report_items;
  g_tenos_report_items = this;
  snapshots = 0;
}

// constructor 3/6
tenos_report_item::tenos_report_item(const char *name1, const char *name2,  int *int_value) :
  name1(strdup(name1)),
  name2(strdup(name2)),
  int_value(int_value)
{
  u64_value = 0;
  float_value = 0;
  string_value = 0;
  u32_avalue = 0;
  u64_avalue = 0;
  next = g_tenos_report_items;
  g_tenos_report_items = this;
  snapshots = 0;
}

// constructor 4/6
tenos_report_item::tenos_report_item(const char *name1, const char *name2,  u64_t *u64_t_value) :
  name1(strdup(name1)),
  name2(strdup(name2)),
  u64_value(u64_t_value)
{
  int_value = 0;
  float_value = 0;
  string_value = 0;
  u32_avalue = 0;
  u64_avalue = 0;
  next = g_tenos_report_items;
  g_tenos_report_items = this;
  snapshots = 0;
}

  // constructor 5/6
tenos_report_item::tenos_report_item(const char *name1, const char *name2,  std::atomic<u32_t > *u32_avalue)
:
  name1(strdup(name1)),
  name2(strdup(name2)),
  u32_avalue(u32_avalue)
{
  int_value = 0;
  float_value = 0;
  string_value = 0;
  u64_avalue = 0;
  next = g_tenos_report_items;
  g_tenos_report_items = this;
  snapshots = 0;
}


  // constructor 6/6
tenos_report_item::tenos_report_item(const char *name1, const char *name2,  std::atomic<u64_t > *u64_avalue)
:
  name1(strdup(name1)),
  name2(strdup(name2)),
  u64_avalue(u64_avalue)
{
  int_value = 0;
  float_value = 0;
  string_value = 0;
  u32_avalue = 0;
  next = g_tenos_report_items;
  g_tenos_report_items = this;
  snapshots = 0;
}

void tenos_report_item::take_snapshot(int no)
{
  snapshot_s *n = new snapshot_s;
  n->snapshot_number = no;
  if (string_value)     n->v.string_value = string_value;
  else if (int_value)   n->v.int_value = *int_value;
  else if (u32_avalue)  n->v.u32_value = u32_avalue->load();
  else if (float_value) n->v.float_value = *float_value;
  else if (u64_value)   n->v.u64_value = *u64_value;
  else if (u64_avalue)  n->v.u64_value = u64_avalue->load();

  n->previous_snapshots = snapshots; // Insert on head of linked list of snapshots.
  snapshots = n;

  if (next) next->take_snapshot(no);
}

void tenos_report_item::print_csv_title(FILE *fd)
{
  if (name2) fprintf(fd, ",\"%s.%s\"", name1, name2); // Include quotes in title entries since these can easily then be imported using R's read.csv.
  else fprintf(fd, ",\"%s\"", name1);
}


tenos_report_item::snapshot_s *tenos_report_item::find_snapshot(int no)
{
  if (no < 0) return 0;
  for (snapshot_s *r=snapshots; r; r=r->previous_snapshots)
    {
      if (r->snapshot_number == no) return r;
    }
  tenos_diagnostics.checkpoint(2, "tenos_report_item", "missing snapshot number");
  printf("Cannot find snapshot number %i\n", no);
  assert(0); 
  return 0;
}

// A shot number of -1 indicates to print the current value.
// A relative_shot_number gives a baseline to subtract from.
void tenos_report_item::print_value(FILE *fd, bool humanForm, int shot_number, int relative_shot_number)
{
  snapshot_s *toprint = find_snapshot(shot_number);
  snapshot_s *relative_to = find_snapshot(relative_shot_number);
	 
  if (string_value) 
    {
      fprintf(fd, ",%s", toprint ? toprint->v.string_value: string_value); // Subtract of a baseline does not apply to strings.
    }
  else if (int_value) 
    {
      fprintf(fd, ",%i", (toprint ? toprint->v.int_value: *int_value) - ((relative_to ? relative_to->v.int_value:0)));
    }
  else if (u32_avalue)
    {
      fprintf(fd, ",%u", (toprint ? toprint->v.u32_value:u32_avalue->load()) - ((relative_to ? relative_to->v.u32_value:0)));
    }
  else if (float_value) 
    {
      fprintf(fd, ",%f", (toprint ? toprint->v.float_value:*float_value) - ((relative_to ? relative_to->v.float_value:0.0)));
    }
  else if (u64_value) 
    {
      fprintf(fd, "," PFI64, (toprint ? toprint->v.u64_value:*u64_value) - ((relative_to ? relative_to->v.u64_value:0)));
    }
  else if (u64_avalue)
    {
      fprintf(fd, "," PFI64, (toprint ? toprint->v.u64_value:u64_avalue->load()) - ((relative_to ? relative_to->v.u64_value:0)));
    }
  else fprintf(fd, ",<null>");

}

void tenos_report_items_report(const char *msg, FILE *fd, int shot_no, int rel_shot_no)
{
  static int titles_printed = 0;
  
  for (int pass = titles_printed; pass<2; pass++) // Zeroth pass prints the titles - first time only.
    {
      if (pass == 0)
	{
	  fprintf(fd, "\"TCSV\",\"%s\",\"shot\",\"rshot\"", msg);
	}
      else if (pass == 1) fprintf(fd, "CSV,\%s,%i,%i", msg, shot_no, rel_shot_no); // NB: This line is duplicated code
      else fprintf(fd, "HUMAN,\%s,%i,%i", msg, shot_no, rel_shot_no); // NB: This line is duplicated code

      for (tenos_report_item *p = g_tenos_report_items; p; p=p->next)
	{
	  if (pass == 0) p->print_csv_title(fd);
	  else if (pass == 1) p->print_csv_title(fd);
	  else 
	    {
	      p->print_value(fd, false, shot_no, rel_shot_no);
	      fprintf(fd, "/");
	      p->print_csv_title(fd);
	    }
	}
      fprintf(fd, "\n");
    }
  // titles_printed = 1;
}

void tenos_report_items_report(const char *msg, FILE *fd)
{
  tenos_report_items_report(msg, fd, -1, -1);
}

u32_t tenos_stats_checkpoint_cmd(const char *msg, int subcmd, int arg0, int arg1)
{
  static int next_checkpoint_number = 1;
  int rc = -2;

  if (subcmd & (TENOS_STATS_CHECKPOINT_FLAG_PRINT_CSV | TENOS_STATS_CHECKPOINT_FLAG_PRINT_HUMAN))
    {
      int humanf = (subcmd & TENOS_STATS_CHECKPOINT_FLAG_PRINT_HUMAN);
      rc = 0;
      FILE *fd=stdout;
      fprintf(fd, "CSV,\%s,%i,%i", msg, arg0, arg1); // NB: This line is duplicated code
      for (tenos_report_item *p = g_tenos_report_items; p; p=p->next)
	{
	  p->print_value(fd, humanf, arg0, arg1);
	}
      if (!humanf) fprintf(fd, "\n");

      if (1)
	{
	  fprintf(fd, "HUMAN,\%s,%i,%i", msg, arg0, arg1); // NB: This line is duplicated code
	  for (tenos_report_item *p = g_tenos_report_items; p; p=p->next)
	    {
	      p->print_value(fd, humanf, arg0, arg1);
	      fprintf(fd, "/");
	      p->print_csv_title(fd);
	    }
	  fprintf(fd, "\n");
	}
      



    }

  
  if (subcmd & TENOS_STATS_CHECKPOINT_FLAG_PRINT_TAKE_SNAPSHOT)
    {
      int no = next_checkpoint_number++;
      //printf("Taking stats shot no %i\n", no);
      g_tenos_report_items->take_snapshot(no);
      rc = no;
    }
  
  if (rc == -2)
    printf("tenos_stats_checkpoint_cmd: %s: ignoring unrecognised sub-command code %i.\n", msg, subcmd);
  return rc;
  
}



static int expected_magic_no = 0; // This is just the endianness and width setting ... should check ABI word too. (0x3e for x86_64) // TODO will not support hetrogeneous system.
static const char *expected_magic_name = 0;

void tenos_check_correct_magic_no(int w, int magic_no, bool stop_on_error)
{
  //printf("Magic no bits=%i %08X\n", w, magic_no);
  if (expected_magic_no && expected_magic_no != magic_no)
    {
      printf("Loaded magic number %08X did not match expected magic number (%08X,%s) - did you use the wrong compiler?\n", magic_no, expected_magic_no, expected_magic_name); 
      if (stop_on_error) assert(expected_magic_no == magic_no);
    }
}


void install_expected_magic_no(int n, const char *name) // make a method of CORISA_IF please.
{
  expected_magic_no = n;
  expected_magic_name = name;
}


// Placeholder for BSYSTEM - multicore SystemC experiments.
// 
template <class T> void *pthread_delegate_routine(void *ptr)
{
  T *arg = (reinterpret_cast<T*>(ptr));
  //return (*arg)();
  (*arg)();
  return 0;
}


template <class T> class pthreadDelegate
{
  T *m_ptr;
  void (T::*m_func)();
public:
  pthreadDelegate(T *ptr, void (T::*func)()) : m_ptr(ptr), m_func(func) {}

  void operator ()()
  {
    (*m_ptr.*m_func)();
  }
};


#ifdef PTRACE_V1_ENABLE
// We have on static instance of this tracer and all cores write to it.
// Interleaving will depend on the lt quantum.
ptrace_v1 *g_ptracer_v1 = 0;
#endif

ptrace_v1::ptrace_v1(const char *filename) : fn(filename) // constructor
{
  ptrace_fd = 0;
  ptrace_count = 0;
  ptrace_dirty = false;
  printf("Created ptracer_v1\n");
}

void ptrace_v1::operand(u32_t operand)
{
  pblock[3] = operand;
}

void ptrace_v1::instruction(int set_no, int core_no, bool executedf, u32_t pc, u32_t ins32)
{
    if (!ptrace_fd)
    {
      if (!fn) fn = "/tmp/ptrace_v1.bin";
      ptrace_fd = fopen(fn, "wb");
      printf("Opened ptracer_v1 log file %s\n", fn);
    }
  flush();

  pblock[0] = (set_no & 0xFF) | ((executedf & 1)<<8) | (PTRACE_FMT_ID << 10) | (core_no << 20);
  pblock[1] = pc;
  pblock[2] = ins32;
  pblock[3] = 0;
  ptrace_dirty= true; // Do not write out immediately in case an operand is inserted.

}

void ptrace_v1::close()
{
  if (ptrace_fd)
    {
      flush();
      fclose(ptrace_fd);
      printf("closed ptrace %s : traced instruction count =  %i \n", fn, ptrace_count);
    }
}

void ptrace_v1::flush()
{
  if (ptrace_dirty)
    {
      assert(ptrace_fd);
      fwrite (&pblock , sizeof(char), sizeof(pblock), ptrace_fd);
      //printf("bin32 instruction trace: set=%i, pc=%08X  executed=%i, ins=%08x\n", set_no, pc, executedf, ins32);
      ptrace_count += 1;
      if ((ptrace_count & 0x1FFFFF)==0) 
	{
	  printf("ptrace log file : %s %ld MByte\n", fn, ptrace_count * sizeof(pblock) / (1000 * 1000));
	}
      ptrace_dirty = 0;
    }

}

u32_t tenos_simulator_cmd(const char *msg, int cmd, int arg0, int arg1, u32_t *rp)
{
  switch (cmd)
    {
    case TENOS_CMD_STATS_CHECKPOINT:
      {
	u32_t rr = tenos_stats_checkpoint_cmd(msg, arg0 & 0xFF, arg0>>8, arg1);
	if (rp) *rp = rr;
	return 0;
      }

    default:
      printf("tenos_simulator_cmd: ignoring unrecognised command code %i\n", cmd);
      return -1;
    }
  return 0;
}

// eof
