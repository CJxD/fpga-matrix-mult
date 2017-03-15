/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */


#ifndef DJGTHREAD_H
#define DJGTHREAD_H

// The djgthread prefix is now totally eliminated by default.
// Enable the following macro to preserve previous behaviour which did not alias with another pthread definition.
// 
#ifndef SKIP_RENAME_DJGTHREAD



#define djgthread_attr_t     pthread_attr_t

#define djgthread_t          pthread_t 
#define djgthread_key_t      pthread_key_t
#define djgthread_cond_t     pthread_cond_t
#define djgthread_mutex_t    pthread_mutex_t
#define djgthread_thread_t   pthread_thread_t
#define djgthread_barrier_t  pthread_barrier_t


// Methods
//#define times(X)    SOCDAM_READ_TICKS(0)

#define djgthread_key_create(KEY, DF)  pthread_key_create(KEY, DF)
#define djgthread_getspecific(KEY)     pthread_getspecific(KEY)
#define djgthread_setspecific(KEY, Y)  pthread_setspecific(KEY, Y)

#define djgthread_mutex_init(MM, Y)  pthread_mutex_init(MM, Y)
#define djgthread_mutex_lock(X)      pthread_mutex_lock(X)
#define djgthread_mutex_unlock(X)    pthread_mutex_unlock(X)
#define djgthread_mutex_destroy(X)   pthread_mutex_destroy(X)

#define djgthread_cond_init(CV, VAL) pthread_cond_init(CV, VAL)
#define djgthread_cond_broadcast(X)  pthread_cond_broadcast(X)
#define djgthread_cond_signal(X)     pthread_cond_signal(X)
#define djgthread_cond_wait(X, M)    pthread_cond_wait(X, M)

#define djgthread_exit(M)            pthread_exit(M)
#define djgthread_create(A,B,C,D)    pthread_create(A,B,C,D)
#define djgthread_join(A,B)          pthread_join(A,B)


#define djgthread_barrier_init(X, AT, CUSTS)   pthread_barrier_init(X, AT, CUSTS)
#define djgthread_barrier_wait(X)              pthread_barrier_wait(X)

#define djgthread_self(X)            pthread_self()
#define djgthread_yield(X)           pthread_yield(X)

#define djgthread_mutexattr_t        pthread_mutexattr_t

#define djgthread_mutexattr_init(X)  pthread_mutexattr_init(X)
#define djgthread_attr_init(X)       pthread_attr_init(X)
#define djgthread_attr_destroy(X)    pthread_attr_destroy(X)
#define djgthread_attr_setdetachstate(X, Y) pthread_attr_setdetachstate(X, Y)
#define djgthread_setcancelstate(X, Y) pthread_setcancelstate(X, Y)
                                       

#endif

/* Detach state.  */
enum
{
  /*
   * pthread_attr_{get_det}detachstate
   */
  PTHREAD_CREATE_JOINABLE = 0,
#define PTHREAD_CREATE_JOINABLE PTHREAD_CREATE_JOINABLE
  PTHREAD_CREATE_DETACHED = 1,
#define PTHREAD_CREATE_DETACHED PTHREAD_CREATE_DETACHED
  /*
   * pthread_setcancelstate paramters
   */
  PTHREAD_CANCEL_ENABLE = 0, /* Default */
#define PTHREAD_CANCEL_ENABLE PTHREAD_CANCEL_ENABLE
  PTHREAD_CANCEL_DISABLE = 1
#define PTHREAD_CANCEL_DISABLE PTHREAD_CANCEL_DISABLE
};


typedef int djgthread_t;


typedef long long int djgthread_machineword_t;
typedef volatile djgthread_machineword_t djgthread_cond_t;
typedef int djgthread_attr_t;
typedef long djgthread_mutexattr_t;
typedef int djgthread_condattr_t;
typedef void * djgthread_key_t;

typedef volatile djgthread_machineword_t djgthread_mutex_t;


#define PTHREAD_BARRIER_SERIAL_THREAD (1000) // One barrier exiter should return this code

typedef struct
{
  djgthread_mutex_t m;
  djgthread_cond_t c;
  volatile int n, cycle;
  int customers;
} djgthread_barrier_t;


// This definition has a problem?
// extern int processor_info(int cpu, void *dest);   // Find out how many cores are available

extern int djgthread_self(); // Return current thread tid.

extern int djgthread_create(djgthread_t *newt, djgthread_attr_t *attr, void *(*entryp) (void *), void *arg);
extern int djgthread_exit(void *retval);
extern int djgthread_self();                      // Return current thread tid.
extern int djgthread_join(djgthread_t tid, void **thread_return);



extern int djgthread_mutex_init(djgthread_mutex_t *mutex, djgthread_mutexattr_t *att);
extern int djgthread_mutex_lock(djgthread_mutex_t *mutex);
extern int djgthread_mutex_unlock(djgthread_mutex_t *mutex);
extern int djgthread_mutex_destroy(djgthread_mutex_t *mutex);


extern int djgthread_cond_init(djgthread_cond_t *cond, djgthread_condattr_t *attr);
extern int djgthread_cond_wait(djgthread_cond_t *cond, djgthread_mutex_t *mutex);
extern int djgthread_cond_broadcast(djgthread_cond_t *cond);


extern int djgthread_barrier_init(djgthread_barrier_t *barrier, void *atts, int customers);
extern int djgthread_barrier_wait(djgthread_barrier_t *barrier);

// Store a pointer to void against a key in the current thread-specific area.
int djgthread_key_create(djgthread_key_t *key, void (*dispose)());
extern void djgthread_setspecific(djgthread_key_t key, void *arg);
extern void *djgthread_getspecific(djgthread_key_t *k);

extern int djgthread_mutexattr_init(djgthread_mutexattr_t* attr);

extern int djgthread_setcancelstate(int state, int *oldstate);


// We used to enable the following macros definitions to get the djgthread implementation of pthreads
// But now this is the default behaviour.
#if 0

// /usr/include/bits/sched.h:130:3: note: previous declaration of ‘cpu_set_t’ was here 
// typedef unsigned long long int cpu_set_t;


#define pthread_attr_t     djgthread_attr_t

#define pthread_t          djgthread_t 
#define pthread_key_t      djgthread_key_t
#define pthread_cond_t     djgthread_cond_t
#define pthread_mutex_t    djgthread_mutex_t
#define pthread_thread_t   djgthread_thread_t
#define pthread_barrier_t  djgthread_barrier_t


// Methods
#define times(X)    SOCDAM_READ_TICKS(0)

#define pthread_key_create(KEY, DF)  djgthread_key_create((djgthread_key_t) KEY, DF)
#define pthread_getspecific(KEY)     djgthread_getspecific((djgthread_key_t) KEY)
#define pthread_setspecific(KEY, Y)  djgthread_setspecific((djgthread_key_t) KEY, Y)

#define pthread_mutex_init(MM, Y)  djgthread_mutex_init(MM, Y)
#define pthread_mutex_lock(X)      djgthread_mutex_lock(X)
#define pthread_mutex_unlock(X)    djgthread_mutex_unlock(X)

#define pthread_cond_init(CV, VAL) djgthread_cond_init(CV, VAL)
#define pthread_cond_broadcast(X)  djgthread_cond_broadcast(X)
#define pthread_cond_signal(X)     djgthread_cond_signal(X)
#define pthread_cond_wait(X, M)    djgthread_cond_wait(X, M)

#define pthread_exit(M)            djgthread_exit(M)
#define pthread_create(A,B,C,D)    djgthread_create(A,B,C,D)
#define pthread_join(A,B)          djgthread_join(A,B)


#define pthread_barrier_init(X, AT, CUSTS)   djgthread_barrier_init(X, AT, CUSTS)
#define pthread_barrier_wait(X)   djgthread_barrier_wait(X)

#define pthread_self(X)            djgthread_self()
#define pthread_yield(X)           djgthread_yield(X)

#define pthread_mutexattr_t        djgthread_mutexattr_t

#define pthread_mutexattr_init(X)  djgthread_mutexattr_init(X)
#define pthread_attr_init(X)       djgthread_attr_init(X)
#define pthread_attr_destroy(X)    djgthread_attr_destroy(X)
#define pthread_attr_setdetachstate(X, Y) djgthread_attr_setdetachstate(X, Y)

#endif


#ifndef SKIP_ACCOUNTING

#ifndef  spEEDO_energy_t
#define spEEDO_energy_t unsigned int
#endif

typedef struct spEEDO_pet2
{ 
  spEEDO_energy_t energy;
  unsigned int ticks;
} spEEDO_pet2;


// User function to read out account info from a thread.
int djgthread_accounting_into(int tid, struct spEEDO_pet2 *record);

#endif


#endif
