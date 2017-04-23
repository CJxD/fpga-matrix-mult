// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>

/***********************************************
 * This is the main driver program that is used
 * to start the simulator 
 ***********************************************/

#include <iostream>
#include <csignal>
#include <dlfcn.h>
#include <poll.h>
#include <sys/time.h>
#include <chrono>
#include <thread>
#include <cassert>
#include <iomanip>

#define TEMP_PT_BYPASS 1
#include "tenos/prazor.h"

#ifdef TLM_POWER3
#include <tlm_power>
#endif // TLM_POWER3


#include <tenos/tenos.h>
#include <tenos/ptrace_v1.h>
#include <tenos/argv_backdoor.h>
#include <tenos/io_backdoor.h>
#include <platform/platform.h>
#include <tlm_utils/tlm_quantumkeeper.h>



using namespace std;
using namespace sc_core;
#ifdef TLM_POWER3
using namespace sc_pwr;
#endif // TLM_POWER3

#define MAX_NO_OF_CORES 256



/*
 * GLOBALS
 */
int g_dmi_disablef = 1;
int g_harvardf = 1;
int g_full_traces = false;
int g_cores = 1;
#if TRACECOMM
int g_tracecomm = false;
#endif
#if THREAD_COMM
int g_thread_comm = false;
int g_roi_region = 0;
#endif
int g_self_starting_cores = 1;
int global_qk_ns = 1000;
sc_time global_qk_quantum = sc_time(1000, SC_NS);
int g_no_caches = 0;
bool g_log_energy_vcd_plot = false;
double g_core_frequency = 667e6;
int g_tracelevel = 0;
int g_coremask = -1;
int g_dcache_tracelevel = 0;

int g_watches = 0;
int g_noxterm = 0;

extern struct watch_s Watches[MAXWATCH];

// We may want to stop the processor starting
// until gdb is connected via RSP.
bool g_wait_debugger = false;
bool putchar_backdoor_with_handles = true;
bool use_verilated = false;
int g_confidence = 100;

const char *uart_canned_input = 0;
const char *g_name = "noname";
const char *g_vcd_trace_filename = 0;

argv_backdoor_setup *argv_backdoor_su = 0;
argv_backdoor_setup *stdin_backdoor_su = 0;
io_backdoor_setup *io_backdoor_su = 0;
unsigned char *argv_ram_backdoor = 0;

sc_trace_file* g_Tf = 0;

#ifdef TLM_POWER3
pw_trace_file* g_vcd_pt = NULL;
pw_trace_file* g_txt_pt = NULL;
pw_trace_file* g_slk_pt = NULL;
#endif // TLM_POWER3

// List of callbacks to make under various types of error report.
tenos_diagnostics_s tenos_diagnostics; 

platform* the_benchconfig;

struct timeval g_te;

struct pollfd stdin_poll = {  .fd = STDIN_FILENO
			    , .events = POLLIN | POLLRDBAND | POLLRDNORM | POLLPRI };

prazor_gp_mm_t* prazor_gp_mm_t::_instance = 0;

#ifndef VHLS_STATIC  
typedef platform* (*func_ptr_t)(const sc_module_name&,
				int,
				double,
				bool,
				const char*,
				const char*);
#else
extern "C" {
  platform *maker(
    const sc_module_name& p_name,
    int n_cores,
    double core_frequency,
    bool harvardf,
    const char* device_ini,
    const char* system_ini);
}
#endif

void hup_handler(int p_arg) {
  printf("HUP signal received\n");
  if (the_benchconfig)
    the_benchconfig->hup_handler(p_arg);
  signal (SIGHUP, hup_handler);
}

void close_simulation(int p_arg) {

  cout << endl << endl;
  double l_simulation_time = ((double) clock()) / CLOCKS_PER_SEC;

#ifdef TLM_POWER3
  if(g_vcd_pt) pw_close_vcd_trace_file(g_vcd_pt);
  if(g_txt_pt) pw_close_txt_trace_file(g_txt_pt);
  if(g_slk_pt) pw_close_slk_trace_file(g_slk_pt);
#endif // TLM_POWER3
  
  if(p_arg == 0)
    cout << endl << endl
	 << "[INFO] End of Simulation" << endl;
  else
    cout << endl << endl
	 << "[INFO] Simulation Interrupted" << endl;

  cout << " \t- Simulation Time: " << sc_time_stamp() << endl
       << " \t- Simulation Duration: " << l_simulation_time << " sec"
       << endl;
  
  exit(0);
}

template<typename Rep,typename Period>
void print_duration(std::chrono::duration<Rep,Period> t) {
    assert(0<=t.count() && "t must be >= 0");

    typedef chrono::duration<int,std::ratio<60*60*24>> days;

    auto d = chrono::duration_cast<days>(t);
    auto h = chrono::duration_cast<chrono::hours>(t - d);
    auto m = chrono::duration_cast<chrono::minutes>(t - d - h);
    auto s = chrono::duration_cast<chrono::seconds>(t - d - h - m);
    auto ms = chrono::duration_cast<chrono::milliseconds>(t - d - h - m - s);

    cout << "Simulation took: ";
    cout << setw(2) << setfill('0') << d.count() << "d:";
    cout << setw(2) << setfill('0') << h.count() << "h:";
    cout << setw(2) << setfill('0') << m.count() << "m:";
    cout << setw(2) << setfill('0') << s.count() << "s:";
    cout << setw(3) << setfill('0') << ms.count() << "ms" << endl;
}

int sc_main(int argc, char *argv[]) {
  
  const char* boot = 0;
  const char* dtb = 0;
  const char* kernel = 0;
  const char* vdd = 0;

  const char *device_ini = 0;
  const char *system_ini = 0;
  const char *libplatform = 0;

  // Setup the signal handlers
  signal(SIGINT, close_simulation);  
  signal(SIGHUP, hup_handler);
  
  while (argc > 1) {
    // Disable backdoor access to RAMs.
    if (!strcmp(argv[1], "-dmi-disable")) {
      g_dmi_disablef = 1;
      argc -= 1; argv += 1;
      continue;
    } else if (!strcmp(argv[1], "-dmi-enable")) {
      g_dmi_disablef = 0;
      argc -= 1; argv += 1;
      continue;
      // Have separate I+D caches
    } else if (!strcmp(argv[1], "-no-harvard")) {
      g_harvardf = 0;
      argc -= 1; argv += 1;
      continue;
    } else if (!strcmp(argv[1], "-full-traces")) {
      g_full_traces = 1;
      argc -= 1; argv += 1;
      continue;
    } else if (!strcmp(argv[1], "-ptracer")) {
#ifdef PTRACE_V1_ENABLE
      g_ptracer_v1 = new ptrace_v1(argv[2]);
#else
      printf("This version compiled without PTRACE_V1_ENABLE.\n"
	     "Ptrace request is ignored\n");
#endif
      argc -= 2; argv += 2;
      continue;
    } else if (!strcmp(argv[1], "-cores")) {
      g_cores = atoi(argv[2]);
      argc -= 2; argv += 2;
      continue;
    } else if (!strcmp(argv[1], "-self-starting-cores")) {
      g_self_starting_cores = atoi(argv[2]);
      if (g_self_starting_cores > g_cores)
	g_cores = g_self_starting_cores;
      argc -= 2; argv += 2;
      continue;
    } else if (!strcmp(argv[1], "-global-qk-ns")) {
      global_qk_ns = atoi(argv[2]);
      global_qk_quantum = sc_time(global_qk_ns, SC_NS);
      argc -= 2; argv += 2;
      continue;
    } else if (!strcmp(argv[1], "-no-caches")) {
      g_no_caches = 1;
      argc -= 1; argv += 1;
      continue;
    } else if (!strcmp(argv[1], "-vcd-power-plot")) {
      g_log_energy_vcd_plot = 1;
      argc -= 1; argv += 1;
      continue;
    } else if (!strcmp(argv[1], "-core-frequency")) {
      // User supplies value in MHz.
      g_core_frequency = atof(argv[2]) * 1.0e6; 
      argc -= 2; argv +=2;
      continue;
    } else if (!strcmp(argv[1], "-tracelevel")) {
      g_tracelevel = atoi(argv[2]);
      //printf("Set tracelevel to %i\n", g_tracelevel);
      argc -= 2; argv += 2;
      continue;
    } else if (!strcmp(argv[1], "-coremask")) {
      g_coremask = atoi(argv[2]);
      argc -= 2; argv += 2;
      continue;
    } else if (!strcmp(argv[1], "-dcache-tracelevel")) {
      g_dcache_tracelevel = atoi(argv[2]);
      argc -= 2; argv += 2;
      continue;
      //e.g. -watch c001b428 +128
    } else if (!strcmp(argv[1], "-watch")) {
      u64_t lo, hi;
      int c = sscanf(argv[2], "%lx", &lo);
      // +n for second value is a relative offset.
      if (argv[3][0] == '+') {
	c += sscanf(argv[3]+1, "%lx", &hi);
	hi += lo;
      }
      else
	c += sscanf(argv[3], "%lx", &hi);
	  
      if (c != 2 || hi < lo || g_watches==MAXWATCH) {
	printf("Watch range numbers malformed\n");
	exit(1);
      }
      
      Watches[g_watches].level = g_tracelevel;
      Watches[g_watches].lo = lo;
      Watches[g_watches].hi = hi;
      g_watches += 1;
      argc -= 3; argv += 3; continue;
      // Do not start an xterm for console output.
      // Note an alternative and useful approach would be to
      // have just one xterm that each new run of the simulator
      // connects to - this is quite easy to do but this comment
      // is too small...
    } else if (!strcmp(argv[1], "-noxterm")) {
      g_noxterm = true;
      argc -= 1; argv += 1;
      continue;
      // Wait for the debugger to connect before starting program
    } else if (!strcmp(argv[1], "-wait-debugger")) {
      g_wait_debugger = true;
      argc -= 1; argv += 1;
      continue;
      // Wait for the debugger to connect before starting program
    } else if (!strcmp(argv[1], "-putchar-backdoor-nohandles")) {
      putchar_backdoor_with_handles = false;
      argc -= 1; argv += 1;
      continue;
    } else if (!strcmp(argv[1], "-verilated")) {
      use_verilated = true;
      argc -= 1; argv += 1;
      continue;
    } else if (argc > 2 && !strcmp(argv[1], "-dram-device")) {
      device_ini = argv[2];
      argc -= 2;
      argv += 2;
      continue;
    } else if(argc > 2 && !strcmp(argv[1], "-dram-system-ini")) {
      system_ini = argv[2];
      argc -= 2;
      argv += 2;
      continue;
    } else if(argc > 2 && !strcmp(argv[1], "-platform")) {
      libplatform = argv[2];
      argc -= 2;
      argv += 2;
    } else if (argc > 2 && !strcmp(argv[1], "-name")) {
      g_name = argv[2];
      new tenos_report_item("name", "", g_name);
#ifdef TLM_POWER3
      sc_pwr::simulation_title(g_name);
#endif // TLM_POWER3
      argc -= 2; argv += 2;
      continue;
    } else if (argc > 2 && !strcmp(argv[1], "-confidence")) {
      g_confidence = atoi(argv[2]);
      assert(g_confidence >= 2);
      argc -= 2; argv += 2;
      continue;
    } else if (argc > 2 &&
	       (!strcmp(argv[1], "-boot") ||
		!strcmp(argv[1], "-image"))) {
      boot = argv[2];
      argc -= 2; argv += 2;
      continue;
    } else if (argc > 2 && !strcmp(argv[1], "-devicetree")) {
      dtb = argv[2];
      argc -= 2; argv += 2;
      continue;
    } else if (argc > 2 && !strcmp(argv[1], "-kernel")) {
      kernel = argv[2];
      argc -= 2; argv += 2;
      continue;
    }
    else if (argc > 2 && !strcmp(argv[1], "-vdd")) {
      vdd = argv[2];
      argc -= 2; argv +=2;
      continue;
    } 
    else if (argc > 2 && !strcmp(argv[1], "-uart-input")) {
      // Note: Immediate string or file data for uart model input.
      // Note: the uart model will handle input starting with a
      // dot or a slash as a file to read from
      uart_canned_input = argv[2];
      argc -= 2; argv += 2;
      continue;
    } else if (argc > 2 && !strcmp(argv[1], "-vcd-trace-filename")) {
      g_vcd_trace_filename = argv[2];
      argc -= 2; argv += 2;
      continue;
      // All further args passed by backdoor to simulated environment.
    } else if (!strcmp(argv[1], "--")) {
      // Copy these ready for backdoor access
      argv_backdoor_su = new argv_backdoor_setup(boot, argc-2, argv+2);
      break;
    }
    else {
      fprintf(stderr, "Bad cmdline argument '%s'\n"
	      "To see supported command line args please vhls.cpp\n",
	      argv[1]);
      SC_REPORT_FATAL("vhls.cpp", "bad cmdline argument");
    }
  }

  /* Check to see if we have anything in stdin.
   * We will assume that if there was redirection to 
   * the simulator then it should be passed down.
   */
  if(poll(&stdin_poll, 1, 0) == 1) {
    stdin_backdoor_su = new argv_backdoor_setup(boot, stdin);
  }

  io_backdoor_su = new io_backdoor_setup(boot, 4, 'l');
  
  if (global_qk_ns) {
    tlm_utils::tlm_quantumkeeper::set_global_quantum(
      sc_time(global_qk_ns, SC_NS));
  }
  
  std::cout
    << "Global quantum is: "
    << tlm_utils::tlm_quantumkeeper::get_global_quantum()
    << std::endl;

  if (!argv_backdoor_su) {
    // needed for symbolic disassemble
    argv_backdoor_su = new argv_backdoor_setup(boot, 0, 0);
  }

  assert (g_cores <= MAX_NO_OF_CORES);
  
  if (g_vcd_trace_filename) {
    g_Tf = sc_create_vcd_trace_file(g_vcd_trace_filename);
    // NS resolution - not in Systemc 3.2 ?
    // ((vcd_trace_file*)g_Tf)->sc_set_vcd_time_unit(-9);
    }
  
#ifdef TLM_POWER3
  char line0[132], line1[132];
  snprintf(line0, 132, "%s", g_name);
  snprintf(line1, 132, "%s.plt", g_name);
  if (g_log_energy_vcd_plot)
    g_vcd_pt = pw_create_vcd_trace_file(
      line0,
      0 ? line1 : 0,
      PlotStyleDecaying);

  std::cout << "Constructing " << line0 << "\n";
  snprintf(line0, 132, "%s.power.txt", g_name);
  g_txt_pt = pw_create_txt_trace_file(line0);
  // g_slk_pt = pw_create_slk_trace_file(g_name);
#endif // TLM_POWER3

#ifdef TLM_POWER3
  //! Set Power and Energy value for dealing with overflow
  pw_set_power_resolution(1, PW_fW);
  pw_set_energy_resolution(1, PW_fJ);
#endif // TLM_POWER3
  
  if(system_ini == 0 || device_ini == 0) {
    std::cout
      << "Need to specify DRAMIS2 device and system files"
      << std::endl;
    return 0;
  }

#ifndef VHLS_STATIC  
  if(libplatform == 0) {
    std::cout << "Need to specify platform configuration" << endl;
    return 0;
  }
  
  void *hndl = dlopen(libplatform, RTLD_NOW);
  if(hndl == NULL) {
    std::cerr << dlerror() << endl;
    return 0;
  }
  
  // Create the bench  
  func_ptr_t maker = (func_ptr_t)dlsym(hndl, "maker");
#endif  
  the_benchconfig = maker("the_top",
			g_cores,
			g_core_frequency,
			g_harvardf,
			device_ini,
			system_ini);
  the_benchconfig->reset(boot, dtb, kernel, vdd);

#ifdef TLM_POWER3
  std::cout<< "child mode =" << also_trace_children << __LINE__ << "\n";
  // pw_trace(g_txt_pt,
  //          *the_benchconfig,
  //          "NEWTOPNAME",
  //          also_trace_children);
  //pw_trace(g_txt_pt, *the_benchconfig, "NEWTOPTOTAL", sum_children);
  
  //pw_trace(g_slk_pt, *the_benchconfig, "BENCHCONFIG");
  //pw_trace(g_slk_pt, (the_benchconfig->memory0), "Memory 0");
  //pw_trace(g_slk_pt, (the_benchconfig->memory1), "Memory 1");
  //pw_trace(g_slk_pt, (the_benchconfig->busmux0), "BusMux 0");
  //pw_trace(g_slk_pt, (the_benchconfig->busmux1), "BusMux 1");
  //pw_trace(g_slk_pt, (the_benchconfig->uart0),   "Uart 0");
#endif
    
  for(int i = 0; i < g_cores; i++) {
    char txt[20];
    sprintf(txt, "core_%d", i);
#ifdef TLM_POWER3
    pw_trace(g_vcd_pt, *(the_benchconfig->get_Core(i)), txt);
    pw_trace(g_txt_pt, *(the_benchconfig->get_Core(i)), txt);
    pw_trace(g_slk_pt, *(the_benchconfig->get_Core(i)), txt);
#endif
  }

  struct timeval te;
  gettimeofday(&te, NULL);

  int millisec = lrint(te.tv_usec/1000.0);
  if(millisec >= 1000) {
    millisec -= 1000;
    te.tv_sec++;
  }

  struct tm* tm_info = localtime(&te.tv_sec);
  char buffer[26];
  strftime(buffer, 26, "%d/%m/%Y %H:%M:%S", tm_info);
  printf("Simulation starts at %s.%03d\n", buffer, millisec);
  
  gettimeofday(&g_te, NULL);

  // Start simulation
  auto start = chrono::steady_clock::now();
  sc_start();
  sc_stop();
  auto finish = std::chrono::steady_clock::now();
  print_duration(finish-start);
  
  if (1) {
    char line0[232];
    snprintf(line0, 232, "%s.stat_rpt.txt", g_name);
    FILE *d = fopen(line0, "w");
    if (d) {
      the_benchconfig->stat_report("eos spooled", d);
      tenos_report_items_report("eos spooled", d);
      fclose(d);
    }
  }

  tenos_report_items_report("eos stdout", stdout);
  the_benchconfig->stat_report("eos stdout", stdout);
  if (g_Tf)
    sc_close_vcd_trace_file(g_Tf);

#ifdef PTRACE_V1_ENABLE
  if (g_ptracer_v1)
    g_ptracer_v1->close();
#endif

#ifdef TLM_POWER3
  if (g_vcd_pt) pw_close_vcd_trace_file(g_vcd_pt);
  if (g_txt_pt) pw_close_txt_trace_file(g_txt_pt);
  if (g_slk_pt) pw_close_slk_trace_file(g_slk_pt);
#endif // TLM_POWER3

  unsigned long t = lrint(sc_time_stamp().to_seconds()*1e6L);
  long int s = lrint(t/(long int)1e6);
  long int us = t - s*1e6;

  te = g_te;
  te.tv_sec += s;
  te.tv_usec += us;

  millisec = lrint(te.tv_usec/1000.0);
  if(millisec >= 1000) {
    millisec -= 1000;
    te.tv_sec++;
  }

  tm_info = localtime(&te.tv_sec);
  strftime(buffer, 26, "%d/%m/%Y %H:%M:%S", tm_info);
  printf("Simulation ends at %s.%03d\n", buffer, millisec);

  cout << " \t- Simulation Time: " << sc_time_stamp() << endl;

  return 0;
}

// For LT should add on the run-ahead 'delay'
int backdoor_counter_ticks(lt_delay runahead) {
  int r = runahead.point()/ sc_time(1, SC_US);
  // printf("Backdoor_counter_ticks returns %i\n", r);
  return r;
}


prazor_gp_mm_t* prazor_gp_mm_t::instance() {
  if(_instance == 0) {
    _instance = new prazor_gp_mm_t;
  }

  return _instance;
}

PRAZOR_GP_T *prazor_gp_mm_t::allocate() {
  PRAZOR_GP_T *ptr;

  lck.lock();
  if(free_list) {
    ptr = free_list->trans;

    access *empties_head = empties;
    empties = free_list;
    free_list = free_list->next;
    empties->prev = 0;
    empties->next = empties_head;
  }
  else {
    ptr = new PRAZOR_GP_T(this);
  }
  lck.unlock();

  if(ptr->get_ref_count() < 0) {
    while(ptr->get_ref_count() != 0)
      ptr->acquire();
  }

  assert(ptr->get_ref_count() == 0);

  return ptr;
}


void prazor_gp_mm_t::free(PRAZOR_GP_T *trans)
{
  // clears the extension pointers for sure
  trans->reset();
  
  lck.lock();
  if(!empties) {
    empties = new access;
    empties->prev = 0;
    empties->next = 0;
    if(free_list) 
      free_list->prev = empties;
  }
  access* free_list_head = free_list;
  free_list = empties;
  empties = empties->next;
  free_list->next = free_list_head;
  free_list->trans = trans;
  lck.unlock();
}

void prazor_gp_mm_t::free(tlm::tlm_generic_payload* disposeme) {
  free(dynamic_cast<prazor_gp_t *>(disposeme));
}


// EOF
