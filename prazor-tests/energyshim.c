/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

//
// PRAZOR energyshim - wrapper around a benchmark containing instrumentation.
//
//
// When running NATIVE we generally need a lot of loops - this shim provides these loops as well as some control for energy probes.
// It takes its args from the middle group of args separated with -- -- marks.
//
//    vhls-sim  <simulator args> -cores nn -- <energyshim args> -- args passed to the application benchmark.
//
#ifdef NATIVE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>

#else

#ifdef USE_PRLIBC
#include "prstdio.h"
#else
#include "stdio.h"
#include "string.h"
#include "sys/time.h"
#include <stdlib.h>
#endif
#endif

#ifdef USE_VHLS_BACKDOORS
#include "socdam_bdoor_mapping.h"
#else
#define SOCDAM_KILL_SIM(X) exit(X)
#endif
#ifdef USE_SPEEDO
#include "speedo_reg_mapping.h"
#endif

#ifdef GET_ENERGY_FROM_PROBE
#include "currentprobe-client.h"
#endif

#define MAX_PROCESSORS 64

#include "energyshim.h"

const char *g_es_title = "noname";
const char *g_es_mode = "";
int g_es_verbosef = 1;
int g_es_num_keys = 1000;
int cores = 1;
int g_es_loops = 1;

int start_shot_no;



// Capture prazor stats counters to a snapshot and return its number.
unsigned int arm_backdoor_stats_take_snapshot()
{
  unsigned int r_snapshot_no = 0;
#ifdef USE_VHLS_BACKDOORS
  asm volatile ("  mov r0,#4\n swi #207\n mov %0,r0 \n": "=r" (r_snapshot_no): );
#else
#ifndef GET_ENERGY_FROM_PROBE
  printf("take_snapshot: Energyshim compiled without VHLS backdoors\n");
#endif
#endif
  return r_snapshot_no;
}

// Print prazor stats counters.  Supply a snapshot number and a second snapshot number to print relative to.
void arm_backdoor_print_csv_stats(int shot_no, int rel_shot_no)
{
#ifdef USE_VHLS_BACKDOORS
  //asm volatile (" nop \n nop \n");
  asm volatile (" mov r0,%0 \n mov r1,%1 \n swi #207": : "r" ((shot_no << 8) | 1), "r" (rel_shot_no));
#else
#ifndef GET_ENERGY_FROM_PROBE
  printf("print_csv: Energyshim compiled without VHLS backdoors\n");
#endif
#endif
}

unsigned int arm_pmu_backdoor_cycle_count()
{
  unsigned int r_cycle_count = 0;
  // This shim only collects bottom 32 bits of the counters.
#ifdef USE_VHLS_BACKDOORS
// ARM PMU PRAZOR BACKDOOR
  asm volatile (" swi #205": "=r" (r_cycle_count): );
#else
#ifndef GET_ENERGY_FROM_PROBE
  printf("cycle_count: Energyshim compiled without VHLS backdoors\n");
#endif
#endif
  return r_cycle_count;
}

unsigned int arm_pmu_backdoor_icount()
{
  unsigned int r_icount = 0;
  // This shim only collects bottom 32 bits of the counters.
#ifdef USE_VHLS_BACKDOORS
// ARM PMU PRAZOR BACKDOOR
  asm volatile (" swi #206": "=r" (r_icount): );
#else
#ifndef GET_ENERGY_FROM_PROBE
  printf("icount: Energyshim compiled without VHLS backdoors\n");
#endif
#endif
  return r_icount;
}


typedef unsigned int energy_t;
#ifndef N_ENERGIES
#define N_ENERGIES 2
#endif
#if defined(USE_SPEEDO) | defined(GET_ENERGY_FROM_PROBE)
energy_t energies[N_ENERGIES];
unsigned int start_time = 0; 
#endif

/*
 * Energy meter - millijoules
 */
void energy_meters(energy_t out[], energy_t baseline[])
{
  int en;
  int energies[N_ENERGIES];
  for (en=0; en<N_ENERGIES;en++) energies[en] = 0;
#ifdef  USE_VHLS_BACKDOORS
  return; //... read from spEEDO interface on tlm_power3
#endif
#if GET_ENERGY_FROM_PROBE
  char *d =  "localhost";
  char *e = getenv("CURRENTPROBE");
  if (!e) printf("using localhost.  Please set probe IP address using e.g.  export CURRENTPROBE=192.168.1.1 or bognor.sm\n");
  //else printf("Using %s\n", e);
  int rc = currentprobe_operate(energies, e ? e:d);
  for (en=0; en<N_ENERGIES;en++)
    {
      out[en] = energies[en] - (baseline ? baseline[en]:0);
    }
#endif
}

/*
 * Clock - microseconds - system time and elapsed time.
 */
unsigned long int tenos_clock(int *elapsed_microseconds)
{
  int r = 0;
#ifdef USE_VHLS_BACKDOORS
  // Here we are returning the same elapsed time answer as the user time, which is wrong when multiple cores are used.
  r = SOCDAM_READ_TICKS(0);
  if (elapsed_microseconds) *elapsed_microseconds = r;

#else
  int always = 1 ;
  if (always || elapsed_microseconds)
    {
      struct timeval tv;
      gettimeofday(&tv, 0);
      r = tv.tv_usec + (1000 * 1000) * tv.tv_sec;
      if (elapsed_microseconds) *elapsed_microseconds = r;
    }
#if 0
  struct rusage resources;
  int rc = getrusage(RUSAGE_SELF, &resources);
  r = resources.ru_utime.tv_usec + (1000 * 1000) * resources.ru_utime.tv_sec;
#endif
#endif
  return r;
}


#if 0
void clib_test()
 // Test of 64 bit ops - multiply and divide seem to be missing on OR1K C compiler so implement long style below.
{
  volatile long long int x = 23;
  volatile long long int y = 3;
  volatile long long int z;
  int i;
  extern const unsigned long long int dec64_table[20];
  printf("Magic fractions %f %f %e %e\n", 22.0/7, 1/0.81, 22.0/7, 1/0.81);
  for (i=0; i<sizeof(dec64_table)/sizeof(unsigned long long int); i++) printf("%i %lld %llx %i\n", i, dec64_table[i], dec64_table[i], i);
  
  double f = 0.0001;
  for (i=0; i<sizeof(dec64_table)/sizeof(unsigned long long int); i++) 
    {
      f = f * 10.0;
      printf("%i %f   %e\n", i, f, f);
    }
  





  printf("Hello World %10x %llx\n", 0xC0, 0x12345678deadbeefULL);
  printf(" times  %llx %llx -> %llx\n", x, y, x*y);
  printf(" plus  %llx %llx -> %llx\n", x, y, x+y);
  printf(" minus  %llx %llx -> %llx\n", x, y, x-y);
  printf(" sub    %llx %llx -> %llx\n", x, y, x-y);
  printf(" div    %llx %llx -> %llx\n", x, y, x/y);
  printf(" mod    %llx %llx -> %llx\n", x, y, x%y);
  printf(" >  %llx %llx -> %d\n",x, y, x>y);
  printf(" <  %llx %llx -> %d\n",x, y, x<y);
  printf("Hello World %i %i\n", sizeof(long int), sizeof(long long int));
}
#endif

void snap_start()
{
  start_shot_no = arm_backdoor_stats_take_snapshot();
#if GET_ENERGY_FROM_PROBE
  energy_meters(energies, 0);
  start_time = tenos_clock(0);
#endif
#if USE_SPEEDO
  SPEEDO_CHKPT(0);
  energies[0] = READ_SPEEDO_CTX1(SPEEDO_CTX_REG_GLOBAL_ENERGY);
  energies[1] = READ_SPEEDO_CTX1(SPEEDO_CTX_REG_LOCAL_ENERGY);
  printf("Current energies are %i,%i\n", energies[0], energies[1]);
  start_time = READ_SPEEDO_CTX1(SPEEDO_CTX_REG_LOCAL_TIME);
#endif

}

void snap_end()
{
     int end_shot_no = arm_backdoor_stats_take_snapshot();
   //printf("energyshim: Start/End shot nos %i/ %i\n", start_shot_no, end_shot_no);
   arm_backdoor_print_csv_stats(end_shot_no, start_shot_no);


#if GET_ENERGY_FROM_PROBE
   energy_meters(energies, energies);
   unsigned int end_time = tenos_clock(0);

   energy_t ee0 = energies[0];
   energy_t ee1 = energies[1];
   unsigned int tt = end_time-start_time;
   printf("CSV,%s,%s,%i,%i,%i,%i,%u,%u\n", g_es_title, g_es_mode, cores, g_es_loops, g_es_num_keys, tt, ee0, ee1);
   printf("                 TIMING + ENERGY INFORMATION\n");
   printf("Total time                        : %i us (%i .. %i)\n",
	  tt, start_time, end_time);
   printf("Core energy                       : %f J\n",
	  (ee1)/1000.0);
   printf("Total energy                      : %f J\n",
	  (ee0)/1000.0);
   printf("Average power                     : %f W\n",
	  (ee0/1000.0)/(tt/1e6));
   printf("\n"); 
#elif USE_SPEEDO
   SPEEDO_CHKPT(0);
   unsigned int end_time = READ_SPEEDO_CTX1(SPEEDO_CTX_REG_LOCAL_TIME);
   energy_t ee0 = READ_SPEEDO_CTX1(SPEEDO_CTX_REG_GLOBAL_ENERGY)-energies[0];
   energy_t ee1 = READ_SPEEDO_CTX1(SPEEDO_CTX_REG_LOCAL_ENERGY)-energies[1];
   unsigned int units = READ_SPEEDO(SPEEDO_REG_ENERGY_UNITS);
   double core = ee1/((double)units);
   double total = ee0/((double)units);
   unsigned int tt = end_time-start_time;
   double power = total/((double)tt/(double)1e6);
   printf("CSV,%s,%s,%i,%i,%i,%i,%u,%u\n", g_es_title, g_es_mode, cores, g_es_loops, g_es_num_keys, tt, ee0, ee1);
   printf("                 TIMING + ENERGY INFORMATION\n");
   printf("Total time                        : %i us (%i .. %i)\n",
	  tt, start_time, end_time);
   printf("Core energy                       : %lf J\n",
	  core);
   printf("Total energy                      : %f J\n",
	  total);
   printf("Average power                     : %f W\n",
	  power);
   printf("\n");
#endif

}

int main(int argc, const char **argv)
{
#ifndef NATIVE
#ifndef USE_DJGTHREAD_CRT
  if (1)
    {
      printf("Turn on caches in main.cpp of energyshim.c\n");
      asm("mov r0,#0x1000"); // Turn on L1 Cache
      asm("orr r0,r0,#4");
      asm("mcr  p15, 0, r0, c1, c0, 0"); //  (r0 = 0x1004)
      //  ((int *)0xF8F02100)[0] = 1; // Zynq: turn on L2 cache
    }
#endif  
#endif

  //  clib_test();

  while (argc > 1)
     {
       const char *optarg = argv[1];
       if (!strcmp(optarg, "m=icpu") ||  !strcmp(optarg, "m=fcpu") || !strcmp(optarg, "m=privmem") || !strcmp(optarg, "m=dsharedmem") || !strcmp(optarg, "m=rsharedmem"))
	 {
	   g_es_mode = strdup(optarg+2);
	   //printf("Set mode to %s\n", mode);
	   argc --; argv ++; continue;
	 }
       else if (optarg[0] == 'c' && optarg[1] == '=')
       {
	 cores = atoi(optarg+2);
	 if (cores < 1)
	   {
	     printf("cNNN (no of cores) must be >= 1\n");
	     exit(-1);
	   }
	 if (cores > MAX_PROCESSORS)
	   {
	     printf("Maximum processors (MAX_PROCESSORS) exceeded\n");
	     exit(-1);
	   }
	 argc --; argv ++; continue;
       }
       else if (optarg[0] == 't' && optarg[1] == '=')
	 {
	   g_es_title = optarg+2;
	   argc --; argv ++; continue;
	 }
       else if (optarg[0] == 'v')
	 {
	   g_es_verbosef = 1;
	   argc --; argv ++; continue;
	 }
       else if (optarg[0] == 'n' && optarg[1] == '=')
	 {
	   g_es_num_keys = atoi(optarg+2);
	   if (g_es_num_keys < 1) 
	     {
	       printf("Arg 'n' %i\n", g_es_num_keys);
	       printf("Number of keys must be >= 1\n");
	       exit(-1);
	     }
	   argc --; argv ++; continue;
	 }
       else if (optarg[0] == 'l' && optarg[1] == '=')
	 {
	   g_es_loops = atoi(optarg+2);
	   if (g_es_loops < 1) 
	     {
	       printf("Arg 'l' %i\n", g_es_loops);
	       printf("Number of g_es_loops must be >= 1\n");
	       exit(-1);
	     }
	   argc --; argv ++; continue;
	 }
       else if (optarg[0] == '-' && optarg[1] == '-')
	 {
	   argc --; argv++;
	   // pass remaining args to the program under test... it will see '--' as its argv[0]...
	   break;
	 }
       else
	 {
	   printf("Shim: bad arg '%s' in middle section of args:  simargs -- shim args - app args\n", optarg);
	   printf("m=icpu/fcpu/privmem/rsharedmem/dsharedmem  v n1000 c6 l1000 t=title\n");
	   exit(1);
	 }
     }

  int vd = 1;
  if (vd) printf("START WARMUP\n");
  bm_init(argc, argv);

  snap_start();
   int llx;
   if (vd) printf("START MAIN LOOPS\n");
   for (llx=0; llx<g_es_loops; llx++) bm_main(argc, argv);
   if (vd) printf("DONE\n");
   snap_end();

  if (vd) printf("TIDY UP\n");
  bm_deinit();

  SOCDAM_KILL_SIM(0); // Cause PRAZOR simulator to exit - may need to wait for interrupt output to drain before exiting.
  return 0;
}


#ifdef WITH_PRINTF_CHK
__printf_chk(int x_)
{

}

void __stack_chk_fail()
{

}
#endif


// eof

