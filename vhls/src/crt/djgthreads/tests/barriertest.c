//
// barrier test
//
#include <stdio.h>


#define DJGPTHREADS 1

#include "djgthread.h"


int pad0[15];// Best to pad out mutexes so in separate cache lines
pthread_mutex_t mutex1 = 0;
extern void _check_start();

volatile int use_locks = 1;


#define MAXCORES 8
pthread_barrier_t barrier;
pthread_t thrds[MAXCORES];


void *parproc(void *x)
{
  int arg = (int)(x);
  volatile int delay;
  int ll;
  if (use_locks) pthread_mutex_lock(&mutex1);
  printf("Thread %i ticking with %i  sp=%p\n", djgthread_self(), arg, &delay);
  if (use_locks) pthread_mutex_unlock(&mutex1);

  for (ll=0; ll<4; ll++)
    {
      pthread_barrier_wait(&barrier);
      for (delay =0; delay<10 + arg; delay++) continue;
      if (use_locks) pthread_mutex_lock(&mutex1);
      printf("    Thread %i looping with %i loop=%i\n", djgthread_self(), arg, ll);
      if (use_locks) pthread_mutex_unlock(&mutex1);
      pthread_barrier_wait(&barrier);
    }
  if (use_locks) pthread_mutex_lock(&mutex1);
  printf("Thread %i work complete  arg=%i\n", djgthread_self(), arg);
  if (use_locks) pthread_mutex_unlock(&mutex1);
  pthread_barrier_wait(&barrier);
}


int main(int argc, char *argv[])
{
  char *t = malloc(1);
  int i;
    _check_start();
    pthread_barrier_init(&barrier, 0, MAXCORES);
    printf("Heap is roughly at %p\n", t);
  for (i=1; i<MAXCORES; i++)
    {
      pthread_create(&thrds[i], 0, parproc, (void *)i);
    }
  parproc(0);

  _killsim(0); // This makes a nice exit from systemc
}



// eof

