/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */


// $Id: $
// djgthread.c
// DJG threads for bare metal - roughly pthreads compliant.
// 

#include "djgthread.h"
#include "socdam_bdoor_mapping.h"

#ifdef SPEEDO_ACCOUNTING
#include "speedo_reg_mapping.h"
#endif


//#include <assert.h>


int kprintf(const char *format, ...);
void _check_start();

#define TRC(X)

#define MAXTHREADS 128

volatile int _djg_nexttid = 1; // Zeroth is already running!
volatile int _djg_core_hwm = 0;


#define MAXCPUS 32
volatile int _djg_cpu_current[MAXCPUS]; // CPU core to thread mapping.


#define MAP_THREADS 0
#if MAP_THREADS
int t2c_map[] = {4, 5, 6, 0, 2, 1, 7, 3};
#endif

// User should perhaps define USE_DJGTHREAD_CRT to enable this.
// Or at least turn off the one in prlibc with -DSKIP_PRLIBC_CRT
int _argv_main()
{
  // Todo provide a better way of init uClibc and prlibc.
  extern __thread int errno;
  errno = 0; // 

  int argc = SOCDAM_READ_ARGC(0);
  int core_id = SOCDAM_READ_PID_REG(0);
  TRC(kprintf("entering core id %d\n", core_id));
#if MAP_THREADS  
  if (core_id != t2c_map[0]) {
#else
  if(core_id > 0) {
#endif      
#ifdef ARM32
    // Turn on caches for cores higher than core 0
    asm("mov r0,#0x1000"); // Turn on L1 Cache
    asm("orr r0,r0,#4");
    asm("mcr  p15, 0, r0, c1, c0, 0"); //  (r0 = 0x1004)
#endif
  
#if MAP_THREADS   
    if(core_id != t2c_map[0]) {
#else
    if(core_id > 0) {
#endif	
      return _extra_cores();
    }
  }

#if MAP_THREADS    
  if(core_id == t2c_map[0]) {
#else
  if(core_id == 0) {
#endif      
#ifdef ARM32
    // Turn on caches for cores higher than core 0
    asm("mov r0,#0x1000"); // Turn on L1 Cache
    asm("orr r0,r0,#4");
    asm("mcr  p15, 0, r0, c1, c0, 0"); //  (r0 = 0x1004)
#endif
  }

  _check_start();
#ifdef M32
#if BIGE_IONE == 1
  char * argv = (char *)(SOCDAM_ARGV_BASE_32_BE);
#else
  char * argv = (char *)(SOCDAM_ARGV_BASE_32_LE);
#endif
#else
#if BIGE_IONE == 1
  char * argv = (char *)(SOCDAM_ARGV_BASE_64_BE);
#else
  char * argv = (char *)(SOCDAM_ARGV_BASE_64_LE);
#endif
#endif
  int rc = main(argc, argv);
  //printf("main is back at djgthread.c\n");
  return rc;
}


struct _djg_thread_control_block
{
  djgthread_key_t key0, key1;
  void *specific0, *specific1;
#ifndef SKIP_ACCOUNTING
  int accounting_stopped;
  spEEDO_pet2 last_start;
  spEEDO_pet2 accounting;
#endif
  void *arg;
  void *(*entryp)(void*);
  volatile short core;
  // To declare the value of the pointer — that is, the actual address stored in the pointer — as const or volatile, use a declaration of the form:
  djgthread_cond_t volatile *waiting;
} _djg_Threads[MAXTHREADS];


#define LASSERT(X) if (!(X)) kprintf("ASSERT failed %i %s \n",  __LINE__ ,  __FILE__);

// Do not use the _nonatomic suffix one since it is not atomic!
// #define TAS(X, Y)  _test_and_set_nonatomic(X, Y)

#ifdef OR1K

#define MM_RELEASE(X) _archglue_test_and_set(X, 0)
#define MM_ACQUIRE(X) { while (_archglue_test_and_set(X, 1)) _core_pause(SOCDAM_READ_PID_REG(0)); }  // If it returns 1 then another customer had it.

#else
#define MM_RELEASE(X) _simple_mutex_unlock(X)
#define MM_ACQUIRE(X) _simple_mutex_lock(X)
#endif

volatile int _djg_started = 0;


static djgthread_mutex_t hwm_mutex = 0;

static djgthread_mutex_t threads_mm = 0;

// Should call this check_start function on main core early on.
void _check_start()
{
  //static djgthread_mutex_t threads_mm = 0;
  //simulator_trace100();
    //kprintf("_check_start entered\n"); //-- causes an infinite recurse?
  if (_djg_started) return;
  _djg_started = 1;

  MM_ACQUIRE(&threads_mm);
  int i;
  for(i = 0; i < MAXTHREADS; ++i)
    _djg_Threads[i].core = -1;
#if MAP_THREADS  
  _djg_Threads[0].core = t2c_map[0];
#else
  _djg_Threads[0].core = 0;
#endif  
  
  if (!_djg_started)
    {
      TRC(kprintf("djgthread package start\n"));
      _djg_cpu_current[0] = 0; // not a thread! its the main!  for now
      _djg_Threads[0].waiting = 0; 
      _djg_Threads[0].key0 = 0; 
      _djg_Threads[0].key1 = 0;
    }
  MM_RELEASE(&threads_mm);
}


extern int _mystack();


// Processor cores beyond the first one start execution with this...
int _extra_cores()
{
  // Don't start threads package on cores greater than zero: they will get there first generally and we want to start when zero does something.
  int core_id = SOCDAM_READ_PID_REG(0);
  TRC(kprintf("Core %i is alive sp=%x\n", core_id, _mystack()));
  MM_ACQUIRE(&hwm_mutex);
  _djg_cpu_current[core_id] = -1;
  MM_RELEASE(&hwm_mutex);

  while (!_djg_started) 
    {
      djgthread_yield();
      continue;
    }
  MM_ACQUIRE(&hwm_mutex);
  if (core_id >  _djg_core_hwm) _djg_core_hwm = core_id;
  MM_RELEASE(&hwm_mutex);
  TRC(kprintf("Core %i a is ready to serve sp=%p\n", core_id, _mystack()));

  while(1)
    {
      int i;
      // Should yield here for 10..100 us or so to save power at the expense
      // of slightly slower thread starting. Or better, halt and wait for intercore interrupt.
      //kprintf("p %i nexttid=%i hwm=%i %p\n", core_id, _djg_nexttid, _djg_core_hwm, &_djg_core_hwm);
      for (i=0; i<_djg_nexttid; i++)
	{
	  //kprintf("Poll %i %i %i\n",i, _djg_Threads[i].core, core_id);
            if (_djg_Threads[i].core == core_id) 
	    {
	     TRC(kprintf("work found for %i\n", core_id));
             MM_ACQUIRE(&hwm_mutex);
	     _djg_cpu_current[core_id] = i;
             MM_RELEASE(&hwm_mutex);

             TRC(kprintf("Start111 thread %i on core %i ep=%p\n", i, core_id, _djg_Threads[i].entryp));

#ifdef SPEEDO_ACCOUNTING
	      _djg_Threads[i].accounting_stopped = 0;
	      SPEEDO_CHKPT(0);
	      _djg_Threads[i].last_start.energy = READ_SPEEDO_CTX1(SPEEDO_CTX_REG_LOCAL_ENERGY);
	      _djg_Threads[i].last_start.ticks = READ_SPEEDO_CTX1(SPEEDO_CTX_REG_LOCAL_TIME);
	      _djg_Threads[i].accounting.energy = 0;
	      _djg_Threads[i].accounting.ticks = 0;
#endif
	      //TRACE_REG(0) = 1000;
	      _djg_Threads[i].arg = _djg_Threads[i].entryp(_djg_Threads[i].arg);
	      TRC(kprintf("Finished thread %i on core %i\n", i, core_id));
	      _djg_Threads[i].core = -1;
	    }
	}
      djgthread_yield();
    }
}

// Can find out how many cores are available by calling this 
int processor_info(int cpu, void *dest)
{
  if (cpu > _djg_core_hwm) return -1;
  return 0;
}

void processor_bind(int *wot )
{
  LASSERT(0);
}



int djgthread_self_real() // Return current thread tid.
{
  _check_start();
  int core_id = SOCDAM_READ_PID_REG(0);
  MM_ACQUIRE(&hwm_mutex);
  int tid = _djg_cpu_current[core_id];
  MM_RELEASE(&hwm_mutex);

  TRC(kprintf("djgthread_self: core =  %i, tid = %i, addr = 0x%lx\n", core_id, tid, &(_djg_cpu_current[core_id])));
  return tid;
}


int djgthread_self() // Return current thread tid.
{
  return djgthread_self_real(); // Return current thread tid.
}


// Alternative definition without using h/w prims: suitable for single core only.
int _test_and_set_nonatomic(djgthread_mutex_t *mutex, int nv)
{
  // INTS_OFF
  int ov = *mutex;
  *mutex = nv;
  // INTS_ON()
  return ov;
}


#define ACCT_STOP 1
#define ACCT_RESUME 2

#ifdef SPEEDO_ACCOUNTING


/*
Ideally, the debugger should virtually perform the operations of djgthread_accounting_update
to get is values up to date - i.e. combine the current run on in the
h/w registers with that already logged in the datastructure.
*/

int djgthread_accounting_update(int tid)       // Update info from hardware registers into tid control block accounts
{
  if (_djg_Threads[tid].accounting_stopped) return;
  SPEEDO_CHKPT(0);
  unsigned int new_ticks = READ_SPEEDO_CTX1(SPEEDO_CTX_REG_LOCAL_TIME);
  unsigned int new_energy = READ_SPEEDO_CTX1(SPEEDO_CTX_REG_LOCAL_ENERGY);

  _djg_Threads[tid].accounting.energy +=  new_energy -  _djg_Threads[tid].last_start.energy;
  _djg_Threads[tid].accounting.ticks  += new_ticks -  _djg_Threads[tid].last_start.ticks;
  
  _djg_Threads[tid].last_start.energy = new_energy;
  _djg_Threads[tid].last_start.ticks = new_ticks;
}


int djgthread_accounting_ctrl(int cmd)
{
  int tid = djgthread_self();
  switch(cmd)
    {
    case ACCT_STOP:
      // When accounting is stopped we must read the h/w registers and store them.
      if (_djg_Threads[tid].accounting_stopped) return;
      djgthread_accounting_update(tid);
      _djg_Threads[tid].accounting_stopped = 1;

      break;

    case ACCT_RESUME:
      if (!_djg_Threads[tid].accounting_stopped) return;
      {
	SPEEDO_CHKPT(0);
	unsigned int new_ticks = READ_SPEEDO_CTX1(SPEEDO_CTX_REG_LOCAL_TIME);
	unsigned int new_energy = READ_SPEEDO_CTX1(SPEEDO_CTX_REG_LOCAL_ENERGY);
	_djg_Threads[tid].last_start.energy = new_energy;
	_djg_Threads[tid].last_start.ticks = new_ticks;
      }
      _djg_Threads[tid].accounting_stopped = 0;
      break;

    }
}

// User function to read out account info from a thread.
int djgthread_accounting_into(int tid, spEEDO_pet2 *record) 
{ 
  *record = _djg_Threads[tid].accounting;
}

#else
int djgthread_accounting_ctrl(int cmd) { }; // Null function if accounting is disabled
//int djgthread_accounting_into(int tid, spEEDO_pet2 *record) { }; // Null function if accounting is disabled
#endif

int djgthread_mutex_lock(djgthread_mutex_t *mutex)
{
  _check_start();
  djgthread_accounting_ctrl(ACCT_STOP);
  MM_ACQUIRE(mutex);
  djgthread_accounting_ctrl(ACCT_RESUME);
  //djgthread_yield();
  return 0;
}

int djgthread_mutex_unlock(djgthread_mutex_t *mutex)
{
  TRC(kprintf("%li: mutex %p released\n", djgthread_self(), mutex));
  MM_RELEASE(mutex);
}

int djgthread_mutex_init(djgthread_mutex_t *mutex, djgthread_mutexattr_t *att)
{
  *mutex = 0;
  return 0;
  //MM_RELEASE(mutex);
}

int djgthread_cond_init(djgthread_cond_t *cond, djgthread_condattr_t *attr)
{
  // The value in a cond variable is never looked at!
  *cond = 0;
  return 0;
}


//
// The condition variable itself does not contain any data: we just use its unique address as a handle
// on a virtual queue of threads.   If we had a scheduler this would know which threads are blocked on which condition variables, which would tell more of the story...
int djgthread_cond_wait(djgthread_cond_t *cond, djgthread_mutex_t *mutex)
{
  int tid = djgthread_self();
  _djg_Threads[tid].waiting = cond;
  djgthread_mutex_unlock(mutex); // unlock while waiting
  djgthread_accounting_ctrl(ACCT_STOP);
  TRC(kprintf("Tid %i: Waiting for cv %p, mutex %p %i\n", tid, cond, mutex, *mutex));
  int count = 0;
  while (_djg_Threads[tid].waiting == cond) 
    {
      _core_pause(tid);
      //if (!(++count  & 0x3FFF))
      //  kprintf("%i cond_wait: still waiting %p =? %p\n", tid, _djg_Threads[tid].waiting, cond) ;
      //continue;
    }
  TRC(kprintf("Tid %i: received cv signal\n", tid));
  djgthread_mutex_lock(mutex);
  djgthread_accounting_ctrl(ACCT_RESUME);
  TRC(kprintf("Tid %i: reaquired cv mutex\n", tid));
}


// missing: djgthread_cond_signal to unblock just one thread - unblocking them all should always be safe.
int djgthread_cond_signal(djgthread_cond_t *cond)
{
  djgthread_cond_broadcast(cond);
}


int djgthread_cond_broadcast(djgthread_cond_t *cond)
{
  int i;
  static djgthread_mutex_t bcast_mutex = 0;
  MM_ACQUIRE(&bcast_mutex);
  TRC(kprintf("%i: Broadcasting cond %p\n", djgthread_self(), cond));
  for (i=0; i<_djg_nexttid; i++) 
    if (_djg_Threads[i].waiting == cond) 
      {
	TRC(kprintf("Broadcasting cond %p unblocked thread %i\n", cond, i));
	MM_RELEASE(&(_djg_Threads[i].waiting));
	//_djg_Threads[i].waiting = 0;
      }
  MM_RELEASE(&bcast_mutex);
}


// 
// Store a pointer to void against a key in the current thread-specific area.
//
void djgthread_setspecific(djgthread_key_t key, void *arg)
{
  _check_start();

  int tid = djgthread_self();
  //if (key== 0) simulator_trace100();
  TRC(kprintf("set L216 djgthread_setspecific tid=%i key=%p arg=%p\n", tid, key, arg));

  if (_djg_Threads[tid].key0 == key ||  !(_djg_Threads[tid].key0))
    {
      _djg_Threads[tid].key0 = key;
      _djg_Threads[tid].specific0 = arg;
    }
  else if (_djg_Threads[tid].key1 == key ||  !(_djg_Threads[tid].key1))
    {
      _djg_Threads[tid].key1 = key;
      _djg_Threads[tid].specific1 = arg;
    }
  else
    {
        TRC(kprintf("Too many keys in use on tid %i: %p and %p\n",
	      _djg_Threads[tid].key0,
	      _djg_Threads[tid].key1,
                    tid));
      LASSERT(0);
    }
}


//
// Should return fresh unique key value on each call?
//
int djgthread_key_create(djgthread_key_t *key, void (*dispose)())
{
  _check_start();
  static int keyid = 100;
  TRC(kprintf("Key created %p %i\n", key, keyid));
  *key = (void *)42 + keyid++; // The magic key - contents do not matter, only the numeric identity of the key matters.
}


void *djgthread_getspecific(djgthread_key_t *k)
{
  int tid = djgthread_self();
  // Only allow max of one key per thread in this current implementation!

  if (_djg_Threads[tid].key0 == k)   return _djg_Threads[tid].specific0;
  else if (_djg_Threads[tid].key1 == k)   return _djg_Threads[tid].specific1;
  else
    {
        TRC(kprintf("Key value not found on thread block %i,  %i or %i != %i (%p)\n", tid, 
	      _djg_Threads[tid].key0,
                    _djg_Threads[tid].key1, *k, k));
      LASSERT(0);
    }
  return 0;
}

int djgthread_exit(void *retval) //  __attribute__ ((__noreturn__))
{
  int tid = djgthread_self();
  _djg_Threads[tid].arg = retval;
  _djg_Threads[tid].core = -1;

  // ideally need longjmp back to scheduler ... TODO - otherwise assume user has it at the end of the function passed to create.
}

#if 0
void hex8(int x)
{
  hex2(x >> 24);
  hex2(x >> 16);
  hex2(x >> 8);
  hex2(x >> 0);
  putchar(' ');
}

void hex2(int x)
{
  x = x & 255;
  hex1(x >> 4);
  hex1(x);
}

void hex1(int x)
{
  x = x & 15;
  putchar(x>9 ? x+('A'-10): x+'0');
}
#endif


//
// We restrict to one thread per core, at the moment, so need to hang in here
// if all cores are currently busy, hoping one will exit.
//
int djgthread_create(djgthread_t *newt, djgthread_attr_t *attr, void *(*entryp) (void *), void *arg)
{
  TRC(kprintf("create: caller sp=%x   ep=%x %p\n", _mystack(), entryp, entryp));
  TRC(kprintf("create hwm=%i %p\n", _djg_core_hwm,  &_djg_core_hwm));
  int tid;
#if MAP_THREADS
  tid = _djg_nexttid++;
  int core = t2c_map[tid];
#else
  while(1)
    {
      for (tid = 0; tid< _djg_nexttid; tid ++)
	{
	  if (_djg_Threads[tid].core < 0) break; // Found a spare slot in table.
	}
      if (tid < _djg_nexttid) break;
      if (tid == _djg_nexttid && tid <= _djg_core_hwm) 
	{
	  _djg_nexttid +=1;
	  break;
	}
      if (tid > _djg_core_hwm)
	{
            //kprintf("Core %i (>%i) does not exist or has not started yet!\n", tid, _djg_core_hwm);
	}
      // Yield, hoping a core will become free.
      djgthread_yield();
    }
  int core =  tid; // =cpu for now!
#endif    
  _djg_Threads[tid].key0 = 0;
  _djg_Threads[tid].key1 = 0;
  _djg_Threads[tid].waiting = 0;
#ifdef SPEEDO_ACCOUNTING
  _djg_Threads[tid].accounting.energy = 0;
  _djg_Threads[tid].accounting.ticks = 0;
#endif
  _djg_Threads[tid].arg = arg;

  _djg_Threads[tid].entryp = entryp;
  _djg_Threads[tid].core = core;

  *newt = tid;

  LASSERT(core <= _djg_core_hwm);
  TRC(kprintf("Queued thread %i on core %i (hwm=%i) at ep=%p\n", tid, core, _djg_core_hwm, entryp));
  LASSERT(tid < MAXTHREADS);
  return 0; // ok return code.
}


int djgthread_join(djgthread_t tid, void **thread_return)
{
    TRC(kprintf("djgthread_join(): tid is %d\n", tid));
  while(1)
    {
      if (_djg_Threads[tid].core < 0) break;
      djgthread_yield();
    }
  if (thread_return) *thread_return = _djg_Threads[tid].arg;
  return 0;
}


int djgthread_yield()
{
  // Need to save power here by pausing the processor core fetch execute.
  // ... or halt waiting for an intercore interrupt.
  //  LASSERT(0);
  int value = 1; // Delay in microseconds
   _core_pause(value);
}

int djgthread_sched_yield(void)
{
  int value = 10; // Delay in microseconds
  _core_pause(value);
}


int djgthread_barrier_init(djgthread_barrier_t *barrier, void *atts, int customers)
{
  djgthread_mutex_init(&(barrier->m), 0);
  djgthread_cond_init(&(barrier->c), 0);
  barrier->n = 0;
  barrier->cycle = 0;
  barrier->customers = customers;
  return 0;
}



int djgthread_barrier_wait(djgthread_barrier_t *barrier)
{
  djgthread_mutex_lock(&(barrier->m));
  int tid = djgthread_self();
  int rc = 0;
  TRC(kprintf("%i Enter djg barrier %p with %i out of %i present\n", tid, barrier, barrier->n, barrier->customers));
  if (++(barrier->n) < barrier->customers) 
    {
      int cycle = barrier->cycle;
      while (cycle == barrier->cycle) djgthread_cond_wait(&(barrier->c), &(barrier->m));
    } 
  else
    {
        TRC(kprintf("Last thread %i entered barrier\n", tid));
      rc = PTHREAD_BARRIER_SERIAL_THREAD; // One should return this code
      barrier->n = 0;
      barrier->cycle = barrier->cycle + 1;
      djgthread_cond_broadcast(&(barrier->c));
    }
  TRC(kprintf("%i Barrier %p  passed\n", tid, barrier));
  djgthread_mutex_unlock(&(barrier->m));
  return rc;
}



#include <stdarg.h>
int kprintf(const char *format, ...)
{
  char buf[4096];
  // cannot use a full-blooded mutex_lock in here since infinite regress... but the low-level macos should be fine.
  va_list arg;
  int rv;
  static djgthread_mutex_t djgthread_kprintf_mutex = 0;
  va_start(arg, format);
  MM_ACQUIRE(&djgthread_kprintf_mutex);
  rv = vsnprintf(buf, 4096, format, arg);
  puts(buf);
  MM_RELEASE(&djgthread_kprintf_mutex);
  va_end(arg);
  return rv;
}

int djgthread_mutexattr_init(djgthread_mutexattr_t* attr) {
  // do nothing for now, assume that attribute
  // is always successfully initialised
  return 0;
}

int djgthread_attr_init(djgthread_attr_t* attr) {
  // do nothing for now, assume that attribute
  // is always successfully initialised
  return 0;
}

int djgthread_attr_setdetachstate (djgthread_attr_t *attr, int detachstate)
{
  // does not do anything
}


int djgthread_attr_destroy(djgthread_attr_t* attr) {
  return 0;
}

int djgthread_mutex_destroy(djgthread_mutex_t *mutex) {
  return 0;
}

int djgthread_setcancelstate(int state, int *oldstate) {
  // does not to anything
}

// eof

