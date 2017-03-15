/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

#ifndef ARCHGLUE_H
#define ARCHGLUE_H


typedef volatile int prlibc_mutex_t;


typedef long long unsigned int u64_t;
typedef long int u32_t;


extern int _archglue_test_and_set(prlibc_mutex_t *addr, int wdata);


#define MM_RELEASE(X) _archglue_test_and_set(X, 0)

#define MM_ACQUIRE(X) { while (_archglue_test_and_set(X, 1)) _core_pause(get_core_id()); } // If it returns 1 then another customer had it.



#endif
