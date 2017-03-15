/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */
#ifndef ARCHGLUE_H
#define ARCHGLUE_H

// ARM32 version

typedef volatile int prlibc_mutex_t;


typedef long long unsigned int u64_t;
typedef long int u32_t;


extern int _archglue_test_and_set(prlibc_mutex_t *addr, int wdata);


#define MM_RELEASE(X) _archglue_test_and_set(X, 0)

#define MM_ACQUIRE(X) { while (_archglue_test_and_set(X, 1)) _core_pause(get_core_id()); } // If it returns 1 then another customer had it.

extern int _syscall_fopen(const char* filename, const char* mode);
extern int _syscall_refill(int handle, int* len, char* data, int max);
extern int _syscall_fclose(int handle);
extern int _syscall_flush(int handle, char* data, int len);
extern unsigned long _syscall_time(long int* s, long int* us);

extern void flip_tracecomm(void);


#define __WORDSIZE 32

#endif
