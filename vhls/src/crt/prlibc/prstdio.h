/*
 * prstdio.h - a few bare-metal parts of libc.
 *
 * Software Excellence from Mixerton Technology.
 *
 * Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>W
 * (C) 1995-2014 DJ Greaves.
 *
 */

#ifndef __PRSTDIO_H
#define __PRSTDIO_H


typedef int size_t;

#define STDIOLEN 1024
#define NULL ((void *) 0)
#define uchar unsigned char
#define off_t int
#define SYSFDS 32
#define craft_putchar(X) putchar(X)
#define craft_getchar(X) getchar(X)
#define _sa_wrch(X) craft_wrch(X)
#define _sa_rdch(X) craft_rdch(X)
#define EOF -1
#define leaf

/* Types for `void *' pointers.  */
#if __WORDSIZE == 64
# ifndef __intptr_t_defined
typedef long int		intptr_t;
#  define __intptr_t_defined
# endif
typedef unsigned long int	uintptr_t;
#else
# ifndef __intptr_t_defined
typedef int			intptr_t;
#  define __intptr_t_defined
# endif
typedef unsigned int		uintptr_t;
#endif



/* Protections are chosen from these bits, OR'd together.  The
   implementation does not necessarily support PROT_EXEC or PROT_WRITE
   without PROT_READ.  The only guarantees are that no writing will be
   allowed without PROT_WRITE and no access will be allowed for PROT_NONE. */

#define PROT_READ	0x1		/* Page can be read.  */
#define PROT_WRITE	0x2		/* Page can be written.  */
#define PROT_EXEC	0x4		/* Page can be executed.  */
#define PROT_NONE	0x0		/* Page can not be accessed.  */
#define PROT_GROWSDOWN	0x01000000	/* Extend change to start of
					   growsdown vma (mprotect only).  */
#define PROT_GROWSUP	0x02000000	/* Extend change to start of
					   growsup vma (mprotect only).  */

/* Sharing types (must choose one and only one of these).  */
#define MAP_SHARED	0x01		/* Share changes.  */
#define MAP_PRIVATE	0x02		/* Changes are private.  */
#ifdef __USE_MISC
# define MAP_TYPE	0x0f		/* Mask for type of mapping.  */
#endif

/* Other flags.  */
#define MAP_FIXED	0x10		/* Interpret addr exactly.  */
#ifdef __USE_MISC
# define MAP_FILE	0
# define MAP_ANONYMOUS	0x20		/* Don't use a file.  */
# define MAP_ANON	MAP_ANONYMOUS
#endif

/* These are Linux-specific.  */
# define MAP_GROWSDOWN	0x00100		/* Stack-like segment.  */
# define MAP_DENYWRITE	0x00800		/* ETXTBSY */
# define MAP_EXECUTABLE	0x01000		/* Mark it as an executable.  */
# define MAP_LOCKED	0x02000		/* Lock the mapping.  */
# define MAP_NORESERVE	0x04000		/* Don't check for reservations.  */
# define MAP_POPULATE	0x08000		/* Populate (prefault) pagetables.  */
# define MAP_NONBLOCK	0x10000		/* Do not block on IO.  */
# define MAP_STACK	0x20000		/* Allocation is for a stack.  */
# define MAP_UNINITIALIZE 0x4000000     /* For anonymous mmap, memory could
					   be uninitialized. */
union ieee754_double
{
  double d;
  
  /* This is the IEEE 754 double-precision format.  */
  struct
  {
#if BIGE_IONE == 1
    unsigned int negative:1;
    unsigned int exponent:11;
    /* Together these comprise the mantissa.  */
    unsigned int mantissa0:20;
    unsigned int mantissa1:32;
#else
    /* Together these comprise the mantissa.  */
    unsigned int mantissa1:32;
    unsigned int mantissa0:20;
    unsigned int exponent:11;
    unsigned int negative:1;
#endif              /* Little endian.  */
  } ieee;
  
  /* This format makes it easier to see if a NaN is a signalling NaN.  */
  struct
  {
#if BIGE_IONE == 1
    unsigned int negative:1;
    unsigned int exponent:11;
    unsigned int quiet_nan:1;
    /* Together these comprise the mantissa.  */
    unsigned int mantissa0:19;
    unsigned int mantissa1:32;
#else
    /* Together these comprise the mantissa.  */
    unsigned int mantissa1:32;
    unsigned int mantissa0:19;
    unsigned int quiet_nan:1;
    unsigned int exponent:11;
    unsigned int negative:1;
#endif
  } ieee_nan;
};

#define IEEE754_DOUBLE_BIAS 0x3ff /* Added to exponent.  */

/* Memory allocations */

extern void *malloc(size_t);
extern void *calloc(size_t num, size_t size);
extern void free(void *x);

/* ARM : */
/*#define UPDATEPOI_ARM(X) { argsused += 1; if (argsused <3) poi-=1; else if (argsused==3) poi = poi+8; else poi += 1; }*/
#define VA_DECL_T_ARM  __builtin_va_list 
#define STARTPOI_ARM(V, L)    __builtin_va_start(V, L); /* V = (int)((&L)+3) */

/* OR1K */
#define STARTPOI_OR1K(V, L)    __builtin_va_start(V, L); /* V = (int)((&L)+3) */
#define VA_DECL_T_OR1K __builtin_va_list 

/* X86_64 */
#define STARTPOI_X86_64(V, L)    __builtin_va_start(V, L); /* V = (VA_DECL_T*)((&L)+4) */
#define VA_DECL_T_X86_64  __builtin_va_list 

#ifdef OR1K
#define STARTPOI(V, L)  STARTPOI_OR1K(V, L)
#define VA_DECL_T            VA_DECL_T_OR1K
#else
#ifdef ARM32
#define STARTPOI(V, L)  STARTPOI_ARM(V, L)
#define VA_DECL_T            VA_DECL_T_ARM
#else
#define STARTPOI(V, L)  STARTPOI_X86_64(V, L)
#define VA_DECL_T            VA_DECL_T_X86_64
#endif
#endif

#define PRLIBC_VA_ARG(P, TY)  __builtin_va_arg(P, TY) 

extern int printf(const char *, ...);
extern int sprintf(char *, const char *, ...);
/*extern int vsnprintf(char *str, int size, const char *format, VA_DECL_T poi);*/

extern char console_flags, deb;
extern void craft_wrch(char);
extern short int craft_testch();
extern char craft_rdch();
extern void exit(int);
extern int strlen(const char *src);
extern char *strdup(const char *src);
extern char *strcpy(char *dest, const char *src);
extern char *strchr(const char *src, int);
extern int strcmp(const char *s1, const char *s2);

extern void *memset(void *str, int c, size_t n);
extern const char *strstr(const char *haystack, const char *needle);

int atoi(const char*);
double atof(const char*);
long atol(const char*);

enum FileFlags {
  _F_READ = 0x0001,
  _F_WRIT = 0x0002,
  _F_RDWR = 0x0003,
  _F_ERR  = 0x0010,
  _F_EOF  = 0x0020,
  _F_BIN  = 0x0040
};

/* open/fcntl - O_SYNC is only implemented on blocks devices and on files
   located on an ext2 file system */
#define O_ACCMODE	   0003
#define O_RDONLY	     00
#define O_WRONLY	     01
#define O_RDWR		     02
#define O_CREAT		   0100	/* not fcntl */
#define O_EXCL		   0200	/* not fcntl */
#define O_NOCTTY	   0400	/* not fcntl */
#define O_TRUNC		  01000	/* not fcntl */
#define O_APPEND	  02000
#define O_NONBLOCK	  04000
#define O_NDELAY	O_NONBLOCK
#define O_SYNC		 010000
#define O_FSYNC		 O_SYNC
#define O_ASYNC		 020000


typedef struct stdio
{
  int _r; /* bytes left to read */
  char* _p; /* pointer to next character to read */
  unsigned short handle; /* File handle */
  short flags; /* FileFlags */
  char data[STDIOLEN];
} FILE;

extern int fprintf(FILE*, const char *, ...);

extern FILE *stderr;
extern __thread FILE *stdout;
extern __thread FILE *stdin;
extern __thread int errno;

#ifndef assert
#define assert(X) do { if (!(X)) _assert_failed(X, __FILE__, __LINE__); } while (0)
#endif

extern char *optarg;
extern int getopt(int, char *const *, const char*);

extern int fputs(const char *str, FILE *stream);

/* TODO: MP */
extern int fflush(FILE *stream);
extern char *gets(char * str);
extern FILE *fopen(const char* filename, const char* mode);
extern int fclose(FILE* stream);
extern int fscanf(FILE* stream, const char* format, ...);

struct stat  {
  long st_size;			/* Size of file, in bytes.  */
};

struct timeval {
  unsigned long tv_sec;
  unsigned long tv_usec;
};

struct timezone {
  int tz_minuteswest;
  int tz_dsttime;
};

int gettimeofday(struct timeval* tv, struct timezone* tz);

/* eof */

#endif
