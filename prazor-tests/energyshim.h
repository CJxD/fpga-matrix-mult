/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

#ifndef ENERGYSHIM_H
#define ENERGYSHIM_H

// PRAZOR energyshim.h - wrapper around a benchmark containing instrumentation.

extern void snap_start(void);
extern void snap_end(void);
extern unsigned int arm_backdoor_stats_take_snapshot();
extern void arm_backdoor_print_csv_stats(int shot_no, int rel_shot_no);
extern unsigned int arm_pmu_backdoor_cycle_count();
extern unsigned int arm_pmu_backdoor_icount();
extern unsigned long int tenos_clock(int *elapsed_microseconds);
extern int bm_init (int argc, const char *argv[]);  // This is the init function that should parse argc and argv and do a warm up.
extern int bm_main (int argc, const char *argv[]);  // This is the entry point to the application core code for measurement.
extern int bm_deinit ();  // This is the cleanup function to release any acquired resources after measurement.
extern int g_es_verbosef;
extern int g_es_loops;
extern int g_es_num_keys;
extern const char *g_es_mode;
extern const char *g_es_title;




#endif

