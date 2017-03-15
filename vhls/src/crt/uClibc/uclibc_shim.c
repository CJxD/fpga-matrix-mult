//
// This file contains the minimal amount of material to link with uClibc (a version of libc) so that it runs bare metal.
//
// 
//
#include "socdam_bdoor_mapping.h"

#include <stdio.h>
#include <stdint.h>
#include <sys/syscall.h>



int uClibc_crt0() {
  int argc = SOCDAM_READ_ARGC(0);
  int core_id = SOCDAM_READ_PID_REG(0);

  //kprintf("core_id is %d\n", core_id);

  // define the UNICORE flag to disable all code associated with futher SMP cores
#ifndef UNICORE
#ifdef KILL_HIGHER_CORE_THREADS
  if (core_id > 0) while(1) _core_pause();
#else
  if (core_id > 0) return _extra_cores();
#endif
  _check_start();
#endif

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

  return main(argc, argv);
}


size_t __stdio_WRITE(register FILE *stream, register const unsigned char *buf, size_t bufsize) 
{
  int fd = fileno(stream);
  if (fd < 3)
    {
      int i;
      for(i = 0; i < bufsize; i++) craft_wrch(buf[i]);
      return bufsize;
    }
  else
    {
#if 0      
      craft_wrch('@' + ((fd>>12) & 15));
      craft_wrch('@' + ((fd>>8) & 15));
      craft_wrch('@' + ((fd>>4) & 15));
      craft_wrch('@' + ((fd>>0) & 15));
      craft_wrch('\n');
#endif
      int rc = syscall(__NR_write, fd, buf, bufsize);
      return rc;
    }
}


size_t __stdio_READ(register FILE* stream,
		    unsigned char *buf, 
		    size_t bufsize) {
  int fd = fileno(stream);
#if 0
  craft_wrch('@' + ((fd>>12) & 15));
  craft_wrch('@' + ((fd>>8) & 15));
  craft_wrch('@' + ((fd>>4) & 15));
  craft_wrch('@' + ((fd>>0) & 15));
  craft_wrch('\n');
  craft_wrch('\n');
#endif
  int rc = syscall(__NR_read, fd, buf, bufsize);
  return rc;
}

#define HEAPBASE 0x1000000 
#define HEAPEND  0x1FF0000

unsigned char *heapbase = (unsigned char *)HEAPBASE;

void *malloc(size_t size)
{
  size = (size + 3) & (~3);
  void *r = heapbase;
  if (r > (void *)HEAPEND)
    {
      printf("prlibc: Ran out of heap memory!\n");
      exit(1);
      return 0;
    }
  heapbase += size;
  return (signed char *) r;
}

void writes(const char *s)
{
  while(*s) putchar(*s++);
}


void free(void *x)
{

}

void __assert_fail(const char *__assertion, const char *__file,
			   unsigned int __line, const char *__function)
{
  printf("uclibc_shim: assert failed: %s:%i %s in %s\n", __file, __line, __assertion, __function);
  exit(-2);
}

// eof

		    
		     
