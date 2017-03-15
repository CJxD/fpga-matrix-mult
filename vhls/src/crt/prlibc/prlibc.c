/*
 *  prlibc.c - minimal C library - please instead use uLIBC (as in the splash benchmark demo) for all larger examples. 
 *
 * Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
 * (C) 2014 - new hex and decimal output functions.
 * (C) 1995 Tenison Technology.
 * (C) 1995 DJ Greaves.
 *
 */


#include  "prstdio.h"
#include  "archglue.h"
#include "socdam_bdoor_mapping.h"


#define TRC(X)
#define SKIP(X)

# define HUGE		3.40282347e+38F

FILE *stderr = NULL;
__thread FILE *stdout = NULL;
__thread FILE *stdin = NULL;
__thread int errno;  /* provided as part of stdio library */
unsigned int _dl_hwcap;

// variables used by getopt() function
static char *nextchar;
int optind = 0;
int opterr = 1;
int optopt = '?';
char *optarg = NULL;

char *stdin_addr = (char*)SOCDAM_STDIN_BASE_32_LE;

struct drand48_data {
  unsigned short int __x[3];
  unsigned short int __old_x[3];
  unsigned short int __c;
  unsigned short int __init;
  u64_t __a;
};

struct drand48_data __prlibc_drand48_data;

double powten[] = {1e-38, 1e-37, 1e-36, 1e-35, 1e-34, 1e-33,
	1e-32, 1e-31, 1e-30, 1e-29, 1e-28, 1e-27, 1e-26, 1e-25, 1e-24,
	1e-23, 1e-22, 1e-21, 1e-20, 1e-19, 1e-18, 1e-17, 1e-16, 1e-15,
	1e-14, 1e-13, 1e-12, 1e-11, 1e-10, 1e-9, 1e-8, 1e-7, 1e-6, 1e-5,
	1e-4, 1e-3, 1e-2, 1e-1, 1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7,
	1e8, 1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18,
	1e19, 1e20, 1e21, 1e22, 1e23, 1e24, 1e25, 1e26, 1e27, 1e28, 1e29,
	1e30, 1e31, 1e32, 1e33, 1e34, 1e35, 1e36, 1e37, 1e38};

#ifndef SKIP_PRLIBC_CRT
#ifndef USE_DJGTHREAD_CRT
int _argv_main()
{
  //errno = 0;
  int argc =  SOCDAM_READ_ARGC(0);
  int proc_id =  SOCDAM_READ_PID_REG(0);
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

  return main(argc,  argv);
}
#endif
#endif


#ifdef UDP_CONSOLE
extern OutputDevice output_device;
extern void udp_telnet_flush(void);
#else
#define Serial 0
#define output_device Serial
#endif

#define FLOATING_POINT

#define	BUF		513	/* Maximum length of numeric string. */

/*
 * Flags used during conversion.
 */
#define	LONG		0x01	/* l: long or double */
#define	LONGDBL		0x02	/* L: long double; unimplemented */
#define	SHORT		0x04	/* h: short */
#define	SUPPRESS	0x08	/* suppress assignment */
#define	POINTER		0x10	/* weird %p pointer (`fake hex') */
#define	NOSKIP		0x20	/* do not skip blanks */
#define	LONGLONG	0x400	/* ll: long long (+ deprecated q: quad) */
#define	SHORTSHORT	0x4000	/* hh: char */
#define	UNSIGNED	0x8000	/* %[oupxX] conversions */

/*
 * The following are used in numeric conversions only:
 * SIGNOK, NDIGITS, DPTOK, and EXPOK are for floating point;
 * SIGNOK, NDIGITS, PFXOK, and NZDIGITS are for integral.
 */
#define	SIGNOK		0x40	/* +/- is (still) legal */
#define	NDIGITS		0x80	/* no digits detected */

#define	DPTOK		0x100	/* (float) decimal point is still legal */
#define	EXPOK		0x200	/* (float) exponent (e+3, etc) still legal */

#define	PFXOK		0x100	/* 0x prefix is (still) legal */
#define	NZDIGITS	0x200	/* no zero digits detected */

/*
 * Conversion types.
 */
#define	CT_CHAR		0	/* %c conversion */
#define	CT_CCL		1	/* %[...] conversion */
#define	CT_STRING	2	/* %s conversion */
#define	CT_INT		3	/* integer, i.e., strtol or strtoul */
#define	CT_FLOAT	4	/* floating, i.e., strtod */

#define u_char unsigned char
#define u_long unsigned long

int putchar(int c)
{
  switch (output_device)
    {
    case Serial:
      _sa_wrch(c);
      break;
#ifdef UDP_CONSOLE
    case UDPIP:
      _sa_wrch(c);
      udp_telnet_flush();
      break;
#endif
    }
  return c;
}

int getpid(void);

#if 1
extern void _core_ending(), _core_pause(), _kill_sim();
void exit(int c)
{
  // if (getpid() == 0) _core_ending();
  if (getpid() == 0) _kill_sim();
  while (1) _core_pause();
}
#endif



const char *strstr(const char *haystack, const char *needle)
{
  if (!*needle) return haystack;
  while (*haystack)
    {
      const char *bp = haystack, *sp = needle;
      do {
	if (!*sp) return haystack;
      } while (*bp++ == *sp++);
      haystack ++;
    }
  return 0;
}

#define op_t unsigned long int
typedef unsigned char byte;
void *memset(void *dstpp, int c, size_t len)
{
  long int dstp = (long int) dstpp;
  int OPSIZ = sizeof(unsigned long int);
  if (len >= 8) {
    size_t xlen;
    op_t cccc;
  
    cccc = (unsigned char) c;
    cccc |= cccc << 8;
    cccc |= cccc << 16;
    if (OPSIZ > 4)
      cccc |= (cccc << 16) << 16;
  
    /* There are at least some bytes to set.
       No need to test for LEN == 0 in this alignment loop */
    while (dstp % OPSIZ != 0) {
      ((byte *) dstp)[0] = c;
      dstp += 1;
      len -= 1;
    }
  
    /* Write 8 `op_t' per iteration until less than 8 `op_t' remain */
    xlen = len / (OPSIZ * 8);
    while (xlen > 0) {
       ((op_t *) dstp)[0] = cccc;
       ((op_t *) dstp)[1] = cccc;
       ((op_t *) dstp)[2] = cccc;
       ((op_t *) dstp)[3] = cccc;
       ((op_t *) dstp)[4] = cccc;
       ((op_t *) dstp)[5] = cccc;
       ((op_t *) dstp)[6] = cccc;
       ((op_t *) dstp)[7] = cccc;
       dstp += 8 * OPSIZ;
       xlen -= 1;
    }
    len %= OPSIZ * 8;
  
    /* Write 1 `op_t' per iteration until less than OPSIZ bytes remain */
    xlen = len / OPSIZ;
    while (xlen > 0) {
      ((op_t *) dstp)[0] = cccc;
      dstp += OPSIZ;
      xlen -= 1;
    }
    len %= OPSIZ;
  }
  
  /* Write the last few bytes.  */
  while (len > 0) {
    ((byte *) dstp)[0] = c;
    dstp += 1;
    len -= 1;
  }
  
  return dstpp;  
}

void *memchr(const void *s, int c, size_t n) {
  if (n != 0) {
    register const unsigned char *p = s;

    do {
      if (*p++ == c)
	return ((void *)(p - 1));
    } while (--n != 0);
  }
  return (NULL);
}

void *memcpy(void *dest, const void *src, size_t length)
{
  char *d = (char *)dest;
  const char *s = (const char *)src;
  while(--length >= 0) *d++ = *s++;
  return dest;
}



void bcopy(const char *src, char *dest, int length)
{
  while(--length >= 0) *(dest++) = *(src++);
}

int bcmp(char *b1, char *b2, int length)
{
  while(--length >= 0) if ( *(b1++) != *(b2++)) return 1;
  return 0;
}

void bzero(char *b1, int length)
{
  while(--length >= 0) *(b1++) = (char) 0;
}


// nasty - non-reentrant implementation: cannot be used with threads.
static char *spf;
static FILE *fspf;
static int ffile;

void locked_printf(const char *s, VA_DECL_T poi);
void puthex32(int leadingz, unsigned int d0, int f);
void putdec32(int leadingz, int signedf, unsigned int d0, int f);
void puthex64(int leadingz, unsigned long long int d0, int f);
void putdec64(int leadingz, int signedf, unsigned long long int d0, int f);
void putdouble(int force_scientific_form, double d0, int f);

int __sflush(FILE* fp) {
  int r = _syscall_flush(fp->handle, &(fp->data[0]), STDIOLEN);
  bzero(&(fp->data[0]), STDIOLEN);  
  fp->_r = STDIOLEN;
  fp->_p = &(fp->data[0]);

  return r;
}

void pfputc(int X) {
  if (ffile) {
    if(fspf->_r == 0)
      __sflush(fspf);

    *fspf->_p++ = ((char)(X));
    --fspf->_r;
  } else if (!ffile && spf) {
    *spf++ = ((char)(X));
  } else {
    putchar(X);
  }
}

int fprintf(FILE *f, const char *s,  ...)
{
  ffile = 0;
  spf = NULL;
  if(f != NULL) {
    ffile = 1;
    fspf = f;
  }
  VA_DECL_T poi;
  STARTPOI(poi, s);

  locked_printf(s, poi);
  // todo f is ignored
  return 0;
}

int vfprintf(FILE* f, const char *s, VA_DECL_T poi) {
  locked_printf(s, poi);

  return 0;
}

int getpid()
{
  return SOCDAM_READ_PID_REG(0);
}

int get_core_id()
{
  return SOCDAM_READ_PID_REG(0);
}

#ifdef OR1K

// These prims are now in the archglue file for each arch

//#define MM_RELEASE(X) _archglue_test_and_set(X, 0)
//#define MM_ACQUIRE(X) { while (_archglue_test_and_set(X, 1)) _core_pause(get_core_id()); } // If it returns 1 then another customer had it so try again.

#else
#ifdef ARM32

// implementations are in ... ?

#else
typedef volatile long long int prlibc_mutex_t;
#define MM_RELEASE(X) _simple_mutex_unlock(X)
#define MM_ACQUIRE(X) _simple_mutex_lock(X)
#endif
#endif


int printf_q = 12345678;

void prstring(char *s, int prefield, int field)
{
  if (prefield >= 0)  /* prefield positive implies field is MAX length */
  {
    while(*s && field>0) 
    { 
      pfputc(*s++);
      field--; 
      prefield--; 
    }
    while(prefield>0)
    {
      pfputc(' '); prefield--; 
    }
  }
  else
  {
    while(*s)
    {
      pfputc(*s++); field--;
    }
    while(field>0) 
    {
      pfputc(' '); field--;
    }
  }
}

void locked_printf(const char * __restrict format, VA_DECL_T poi)
{
  int i = 0;
  int zeropad = 0;
  char c = format[0];
  int argsused = 0;
  int  prefield;
  int field;
  char longf;
  //  puts("HERE"); putchar(c ? '1':'0'); putchar ('\n');
  while (c)
   { 
     if (c !='%')
     {
       pfputc(c);
       c = format[++i];
       continue;
     }
     field = 0;
     prefield = -1;
     longf=0; 
 sol:
     {
       uchar t1, t;
       int tc;
       
       t = format[++i];
       if (t >= 'a') t = t - 32;//toupper.

       if (t == '%')
	 {
	   pfputc('%');
	 }
       else
	 {switch(t)
	    { 
	    case 'L': longf ++; goto sol;

	    case 'C': tc = PRLIBC_VA_ARG(poi, int); pfputc(tc);
	      break;

	    case 'S':
	      {
		char *s = PRLIBC_VA_ARG(poi, char *);
		//puthex32(0, prefield, 0);
		prstring(s, prefield, field);
	      }
	      break;

	    case 'F':
	      {
		double f = PRLIBC_VA_ARG(poi, double); 
		putdouble(0, f, field);
		break;
	      }
	    case 'E':
	      {
		double f = PRLIBC_VA_ARG(poi, double); 
		putdouble(1, f, field);
		break;
	      }
	    case 'X': 
	      if (longf>1)
		puthex64(zeropad, PRLIBC_VA_ARG(poi, long long int), field);
	      else puthex32(zeropad, PRLIBC_VA_ARG(poi, int), field);
	      break;

	    case 'P': 
	      pfputc('0'); pfputc('x');
	      uintptr_t address = (uintptr_t)(PRLIBC_VA_ARG(poi, void *));
	      if (sizeof(void *) == 64)	
		puthex64(0, address, 1);
	      else
		puthex32(0, address, 1);
	      break;
	      
	    case '*': field = PRLIBC_VA_ARG(poi, int); goto sol;
	      
	    case 'I':
	    case 'D':
	      if (longf>1) putdec64(zeropad, 1, PRLIBC_VA_ARG(poi, long long int), field);
	      else 
		{
		  int vv = PRLIBC_VA_ARG(poi, int);
		  putdec32(zeropad, 1, vv, field);
		}
	      break;
#if 0
	    case 'Q': // delete me - used for debuging varargs.  make sure you have a proper prototype/definition for a varadic function like printf in scope before you call it!
		{
		  int vv = printf_q;
		  debhex8(vv);
		  putdec32(zeropad, 1, vv, field);
		}
	      break;
#endif

	    case 'U':
	      if (longf>1) putdec64(zeropad, 0, PRLIBC_VA_ARG(poi, long long unsigned int), field);
	      else putdec32(zeropad, 0, PRLIBC_VA_ARG(poi, unsigned int), field);
	      break;
	      
	    case '0':
	      if (field==0) zeropad = 1;
	    case '1': case '2': case '3': case '4':
	    case '5': case '6': case '7': case '8': case '9':
	      field = field*10 + t - '0';
	      goto sol;
	      
	    case '.': prefield = field; field = 0; goto sol;
	       
	    default: 
	      pfputc('?'); 	      pfputc('?');
	      break;
	      //printf("Bad printf selector '%c' \n", t);
	      
	    }
	  c = format[++i];
	}
     }
   }
}


volatile long int dof_mutex_spacer = 0;
prlibc_mutex_t dof_mutex = 0;

int printf(const char *__restrict format,  ...)
{
  MM_ACQUIRE(&dof_mutex);
  ffile = 0;
  spf = NULL;
  VA_DECL_T poi;
  STARTPOI(poi, format);
  locked_printf(format, poi);
  MM_RELEASE(&dof_mutex);
  return 0;
}

int vsnprintf(char *str, int size, const char *format, VA_DECL_T poi)
{
  MM_ACQUIRE(&dof_mutex);
  ffile = 0;
  spf = str;
  locked_printf(format, poi);
  *spf = (char) 0;
  MM_RELEASE(&dof_mutex);
  return 0;
}

int sprintf(char *string, const char *s, ...)
{
  MM_ACQUIRE(&dof_mutex);
  ffile = 0;
  spf = string;
  VA_DECL_T poi;
  STARTPOI(poi, s);
  locked_printf(s, poi);
  *spf = (char) 0;
  MM_RELEASE(&dof_mutex);
  return 0;
}

int snprintf(char *string, int ignored_, const char *s, ...)
{
  MM_ACQUIRE(&dof_mutex);
  ffile = 0;
  spf = string;
  VA_DECL_T poi;
  STARTPOI(poi, s);
  locked_printf(s, poi);
  *spf = (char) 0;
  MM_RELEASE(&dof_mutex);
  return 0;
}




char printi_zeroflag;

void hex1(int x)
{
  x = x & 15;
  pfputc( x>9 ? x+('A'-10): x+'0');
}

void hex2(int x)
{
  x = x & 255;
  hex1(x >> 4);
  hex1(x);
}


void debhex8(int x)
{
  hex2(x >> 24);
  hex2(x >> 16);
  hex2(x >> 8);
  hex2(x >> 0);
  pfputc(' ');
}


int isupper (int c)
{
  if ((c >='A') && (c <= 'Z')) return 1;
  return 0;
}

int isdigit (int c)
{
  if ((c >='0') && (c <= '9')) return 1;
  return 0;
}

int isprint (int c)
{
  if ((c >=' ') && (c <= 126)) return 1;
  return 0;
}



int tolower (int c)
 { if ((c >='A') && (c <= 'Z')) return c+32;
   return c;
 }

int toupper (int c)
{
  if ((c >='a') && (c <= 'z')) return c-32;
   return c;
}

int islower(int c)
 { if ((c >= 'a') && (c <= 'z')) return 1;
   return 0;
 }

int isalnum(int c)
{
   if (c >= 'a' && c <= 'z') return 1;
   if (c >= 'A' && c <= 'Z') return 1;
   if (c >= '0' && c <= '9') return 1; 
   return 0;
}

int isalpha(int c)
{
   if (c >= 'a' && c <= 'z') return 1;
   if (c >= 'A' && c <= 'Z') return 1;
   return 0;
}

int strlen(const char *s)
{
  int r = 0;
  while (*s) { s++; r++; }
  return r;
}



char *strcpy(char *s1, const char *s2)
{
  char *r = s1;
  while (*s2) { *s1++ = *s2++; }
  *s1 = (char) 0;
  return r;
}

char *strdup(const char *s)
{
  char * r = (char *)malloc(strlen(s)+1);
  strcpy(r, s);
  return r;
}

char *strcat(char *s1, const char *s2) {
  strcpy(&s1[strlen(s1)], s2);
  return s1;
}

char *strncpy(char *dest, const char *src, size_t n)
{
  size_t i;
  
    for (i = 0 ; i < n && src[i] != '\0' ; i++)
      dest[i] = src[i];
    for ( ; i < n ; i++)
      dest[i] = '\0';
    
    return dest;
}


int strcmp(const char *s1, const char *s2)
{
  while(1)
    {
      char a = *s1++;
      char b = *s2++;
      //char * s = "comparing %p %c %c %p %p\n";       printf(s, s, a, b, s1, s2);
      if (a == (char) 0 && b == (char) 0) return 0;
      if (a == (char) 0 || b == (char) 0 || a!=b) return a-b;
    }
}

int strcasecmp(const char *s1, const char *s2)
{
  while(1)
    {
      char a = toupper(*s1++);
      char b =  toupper(*s2++);
      if (a == (char) 0 && b == (char) 0) return 0;
      if (a == (char) 0 || b == (char) 0 || a!=b) return a-b;
    }
}

int strncmp(const char *s1, const char *s2, int n)
{
  while(1)
    {
      if (n--==0) return 0;
      char a = *s1++;
      char b = *s2++;
      if (a == (char) 0 && b == (char) 0) return 0;
      if (a == (char) 0 || b == (char) 0 || a!=b) return a-b;
    }
  return 0;
}

int strncasecmp(const char *s1, const char *s2, int n)
{
  while(1)
    {
      if (n--==0) return 0;
      char a = toupper(*s1++);
      char b = toupper(*s2++);
      if (a == (char) 0 && b == (char) 0) return 0;
      if (a == (char) 0 || b == (char) 0 || a!=b) return a-b;
    }

}

char *strchr(const char* src, int chr) {
    while(*src) {
        if(*src == chr)
            return (char*) src;
        src++;
    }

    return 0;
}

int atoi(const char *s)
{
  int r = 0;
  int base = 10;
  if (s == NULL) return 0;
  while (*s == ' ' || *s == 0) s++;  /* Skip leading whities */
  while (1)
   { char c = *(s++);
     if (c >= '0' && c <= '9')
      { r = r*base + c - '0';
        continue;
      }
     if (c >= 'a') c = c - 32;
     if (c =='X')
      { base = 16;
        continue;
      }
     if (base >= 10 && c >= 'A' && c <= 'F')
      {
	r = r*base + c - 55;
        continue;
      }
     break;
   }
  return r;
}

long atol(const char *s) {
  long r = 0;
  int base = 10;
  if(s == NULL) return 0;
  while(*s == ' ' || *s == 0) s++;
  while(1) {
    char c = *(s++);
    if(c >= '0' && c <= '9') {
      r = r*base + c - '0';
      continue;
    }
    if(c >= 'a')
      c = c - 32;
    if (c == 'X' || c == 'x') {
      base = 16;
      continue;
    }

    if(base >= 10 && c >= 'A' && c <= 'F') {
      r = r*base + c - 55;
      continue;
    }
    break;
  }

  return r;
}

#ifdef NOT_USED
int write(int fd, const char *buf, int len)
{
  TRC(_sa_wrch(*buf));
  while (len >0)
    {
      _sa_wrch(*buf++);
      len -= 1;
    }
  return 0;
}
#endif

int fwrite(const char *buf, int size, int nmemb, FILE *stream)
{
  while (*buf)
    {
      _sa_wrch(*buf++);
    }
  return 0;
}



int sa_write(char *buf, int len)
{
  unsigned char checksum='O';
  TRC(_sa_wrch(*buf));
  while (len >0)
    {
      char c=*buf++;
      if (c == '\n')
	{
	  _sa_wrch(13);
	}
      _sa_wrch(c);
      len -= 1;
    }
#ifdef UDP_CONSOLE
  if (output_device==UDPIP)
    udp_telnet_flush();
#endif
  
  return 0;
}


#ifdef NOT_USED
int read(int f, char *buf, int len)
{
  char c = _polled_sa_rdch();
  *buf = c;
  return 1;
}
#endif


char getchar()
{
  return _sa_rdch(0);
}

// Note. gcc will call puts instead of printf sometimes. so we need the printf mutex
// around this as well to prevent within-the-line interleaving.
int puts(const char *s)
{
  MM_ACQUIRE(&dof_mutex);
  while (*s)
    {
      char c = *s++;
      putchar(c);
    }
  putchar('\n');
  MM_RELEASE(&dof_mutex);
}


void writes(char *s)
{
  while(*s) 
    {
      if (*s == ' ') craft_wrch('!');
      craft_wrch(*s++);
    }
}


prlibc_mutex_t malloc_mutex = 0;

unsigned char *heapbase = (unsigned char *)HEAPBASE;

void *malloc(size_t size)
{
  MM_ACQUIRE(&malloc_mutex);
  size = (size + 3) & (~3);
  size += 4;
  /* Put size in the first four bytes */
  *((size_t*)heapbase) = size - 4;
  void *r = heapbase + 4;
  if (r > (void *)HEAPEND)
    {
      printf("prlibc: Ran out of heap memory!\n");
      exit(1);
      return 0;
    }
  heapbase += size;
  MM_RELEASE(&malloc_mutex);
  return (signed char *) r;
}

void *calloc(size_t num, size_t size) {
  int i = 0;
  signed char* r = malloc(num*size);
  for(i = 0; i < num*size; ++i) {
    r[i] = 0;
  }

  return r;
			       
}

void *realloc(void* ptr, size_t size) {
  size_t cur;
  char* newp;
  
  if(ptr == NULL)
    return malloc(size);

  cur = *((size_t*)(ptr-4));
  if(cur <= size)
    return ptr;

  newp = malloc(size);
  bcopy(ptr, newp, cur);
  free(ptr);

  return newp;
}

void free(void *x)
{

}

int getpagesize() 
{
  return 4096;
}


void _assert_failed(int code, const char *filename, int linno)
{
  exit(126);
}


#define PZERO 38			/* index of 1e0 in powten[]	*/
#define PMAX 76				/* highest index of powten[]	*/
#define HUGE_NOR 1.70141183460469228		/* normalised HUGE	*/
double atof(const char s[])
{
  extern double powten[];
  double val;
  int i, j, sign, esign, ex, ex1;

  /* skip white space */
  for (i=0; s[i]==' ' || s[i]=='\n' || s[i]=='\t'; i++);
  /* set sign to be +1 or -1 */
  sign = 1;
  if (s[i] == '+' || s[i] == '-')    /* sign */
    sign = (s[i++]=='+') ? 1 : -1;
  /* read digits and build value for as long as there are digits */
  ex = PZERO - 1;
  for (val = 0.0, j = PZERO; s[i] >= '0' && s[i] <= '9'; i++) {
    if (j >= 0)
      val += powten[j] * (s[i] - '0');
    j--;
    ex++;
  }
  /* if first non-digit is decimal point skip it and keep going */
  /* if it is not a decimal point we fall through to test for exponent */
  if (s[i] == '.')
    i++;
  /* continue to build value while the digits keep coming */
  for (; s[i] >= '0' && s[i] <= '9'; i++){
    if (j >= 0)
      val += powten[j] * (s[i] - '0');
    j--;
  }
  /* if non-digit was an exponent flag deal with exponent */
  if (s[i]=='e' || s[i]=='E' || s[i]=='d' || s[i]=='D') {
    i++;
    esign = 1;
    /* check for explicit + or - sign in exponent and deal with it */
    if (s[i] == '+' || s[i] == '-')
      esign = (s[i++]=='+') ? 1 : -1;
	for (ex1 = 0; s[i] >= '0' && s[i] <= '9'; i++)
	    ex1 = 10 * ex1 + s[i] - '0';
	ex += ex1 * esign;
  }
  /* incorporate exponent into val */
  if (ex < 0)
    val = 0.0;
  else if (ex < PMAX || (ex == PMAX && val < HUGE_NOR))
    val *= powten[ex];
  else {
    val = HUGE;
  }

  double n = sign*val;
  return(sign * val);
}

const char *perror(int n)
{
  return "perror missing";
}

/* TODO: Implement below calls */

int creat(const char *pathname, unsigned int mode) {
  return -1;
}

int open(const char *fn, int mode)
{
  return -1;
}

int fstat(int fd, struct stat* buf) {
  return -1;
}

int close(int fd)
{
  return -1;
}

int fsync()
{

}

void* mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
  return (void *) -1; 
}

int munmap(void *addr, size_t length) {
  return -1;
}

static int m_z = 23423342, m_w = 3232342;
int srand(int m)
{
  m_z = m;
  m_w = 3;
}

int sysconf(int arg)
{
  //  return SOCDAM_BACKDOOR_CALL(SOCDAM_SYSCONF_CALL, arg);
  return -2;
}

int rand()
{
  m_z = 36969 * (m_z & 65535) + (m_z >> 16);
  m_w = 18000 * (m_w & 65535) + (m_w >> 16);
  int r = (m_z << 16) + m_w;
  if(r < 0)
    r *= -1;
  
  return r;
}


int write(int fd, const char *buffer, int len)
{
  return 0;
}


int read(int fd, const char *buffer, int len)
{
  return 0;
}

int lseek(int fd, int whence)
{
  return 0;
}

void __assert_fail(int code, const char *filename, int linno)
{
  exit(126);
}

void __assert_func(int code, const char *filename, int linno)
{
  exit(126);
}

const unsigned int hex32_table[] =
  {
    0x1,
    0x10,
    0x100,
    0x1000,
    0x10000,
    0x100000,
    0x1000000,
    0x10000000
  };

const u64_t hex64_table[] =
  {
    0x1ull,
    0x10ull,
    0x100ull,
    0x1000ull,
    0x10000ull,
    0x100000ull,
    0x1000000ull,
    0x10000000ull,
    0x100000000ull,
    0x1000000000ull,
    0x10000000000ull,
    0x100000000000ull,
    0x1000000000000ull,
    0x10000000000000ull,
    0x100000000000000ull,
    0x1000000000000000ull
  };

const u32_t dec32_table[] =
  {
    1,
    10,
    100,
    1000,
    10000,
    100000,
    1000000,
    10000000,
    100000000,
    1000000000,
  };

const u64_t dec64_table[20] =
  {
    1ull,
    10ull,
    100ull,
    1000ull,
    10000ull,
    100000ull,
    1000000ull,
    10000000ull,
    100000000ull,
    1000000000ull,
    10000000000ull,
    100000000000ull,
    1000000000000ull,
    10000000000000ull,
    100000000000000ull,
    1000000000000000ull,
    10000000000000000ull,
    100000000000000000ull,
    1000000000000000000ull,
    10000000000000000000ull, // Note - this last entry is -ve: -8446744073709551616 8ac7230489e80000.
  };
// Special cases when printing number.
//  a) The number zero can vanish under leading zero supression.
//
//



#define N_BIN_POWERS_OF_TEN 6
static const long double bin_exponents_table[N_BIN_POWERS_OF_TEN] =
 { 1.0e1, 1.0e2, 1.0e4, 1.0e8, 1.0e16, 1.0e32 };
static const long double bin_fracs_table[N_BIN_POWERS_OF_TEN] =
 { 1.0e-1, 1.0e-2, 1.0e-4, 1.0e-8, 1.0e-16, 1.0e-32 };




void putdouble(int scientific_form, double d0, int f)
{
  if (d0 < 0.0)
    {
      d0 = -d0;
      pfputc('-'); // Will print -ve NaNs !
    }
  union tobits
  {
    double df;
    unsigned long long int di;
  } to;
  to.df = d0;
  int in_exp = (to.di >> 52) & 0x7FF;
  int mant = to.di & 0xFFFFFffffFFFFllu;


  if (in_exp == 0x7FF)
    {
      if (mant == 0)
	{
	  pfputc('I');
	  pfputc('n');
	  pfputc('f');
	}
      else
	{
	  pfputc('N');
	  pfputc('a');
	  pfputc('N');
	}
      return;
    }

  if (in_exp == 0) // A common path for 0 and for all denormals!
    {
      pfputc('0');
      pfputc('.');
      pfputc('0');
      return;
    }

  int exponent = 0;
  unsigned int i, bitpos = 1U<<(N_BIN_POWERS_OF_TEN-1);
  int precision = 7;
  const int lower_bound = dec32_table[precision];
  const int upper_bound = dec32_table[precision+1];
  if (d0 < lower_bound)
    {
      for(i=N_BIN_POWERS_OF_TEN-1, bitpos = 1U<<(N_BIN_POWERS_OF_TEN-1); bitpos; i--, bitpos>>=1)
	{
	  double q = d0 * bin_exponents_table[i];
	  if (q < upper_bound) 
	    {
	      d0 = q;
	      exponent -= bitpos;
	    }
	}
    }
  else
    {
      for(i=N_BIN_POWERS_OF_TEN-1, bitpos = 1U<<(N_BIN_POWERS_OF_TEN-1); bitpos; i--, bitpos>>=1)
	{
	  double q = d0 * bin_fracs_table[i];
	  if (q >= lower_bound)
	    {
	      d0 = q;
	      exponent += bitpos;
	    }
	}
    }
  exponent += precision;
  int imant = (int)d0;

  // Decimal point will only move a certain distance: outside that range force scientific form.
  if (exponent> precision || exponent <0) scientific_form = 1;  
  
  int p = precision;
  int enable_tz_supress = 0;
  for (p = precision; p >= 0; p--)
    {
      int d = 0;
      while (imant>=dec32_table[p])
	{
	  imant -= dec32_table[p];
	  d++;
	}
      pfputc(d + '0');
      if (enable_tz_supress && imant == 0) break;
      if (p == precision + (scientific_form ? 0: -exponent)) 
	{
	  pfputc('.');
	  enable_tz_supress = 1;
	}
    }
  if (scientific_form)
    {
      pfputc('e');
      putdec32(0, 1, exponent, 1);
    }
}


void putdec64(int leadingz, int signedf, unsigned long long int d0, int f)
{
  int p;
  char lzchar = leadingz? '0': ' ';
  char lzf = 0;
  if (signedf && (d0 & (1ULL<<63)))
    {
      pfputc('-');
      f --;
      d0 = -d0;
    }
  for (p=0;1; p++)
    {
      if (p==sizeof(dec64_table)/sizeof(u64_t)-1) break;
      if (dec64_table[p] > d0) { p--; break; }
    }
  f--;
  while (f > p) { pfputc(lzchar); f--; }
  while (p >= 0)
    {
      int d = 0;
      while (d0>=dec64_table[p])
	{
	  d0 -= dec64_table[p];
	  d++;
	}
      if (d) lzf = 1;
      if (lzf || p==0)
	{
	  char c = (d > 9) ? 'A'+ (d) - 10: '0'+ (d);
	  pfputc(c);
	}
      p --;
    }
}

void puthex64(int leadingz, unsigned long long int d0, int f)
{
  int p;
  char lzchar = leadingz? '0': ' ';
  char lzf = 0;
  for (p=0;1; p++)
    {
      if (p==sizeof(hex64_table)/sizeof(u64_t)-1) break;
      if (hex64_table[p] > d0) { p--; break; }
    }
  f--;
  while (f > p) { pfputc(lzchar); f--; }
  for( ;p >= 0; p--)
    {
      int d = 0;
      while (d0>=hex64_table[p])
	{
	  d0 -= hex64_table[p];
	  d++;
	}
      if (d) lzf = 1;
      if (lzf || p==0)
	{
	  char c = (d > 9) ? 'A'+ (d) - 10: '0'+ (d);
	  pfputc(c);
	}
    }
}

void putdec32(int leadingz, int signedf, unsigned int d0, int f)
{
  int p;
  char lzchar = leadingz? '0': ' ';
  char lzf = 0;
  if (signedf && (d0 & (1ULL<<31)))
    {
      pfputc('-');
      f --;
      d0 = -d0;
    }
  for (p=0;1; p++)
    {
      if (p==sizeof(dec32_table)/sizeof(u32_t)-1) break;
      if (dec32_table[p] > d0) { if (p>0) p--; break; }
    }
  f--;
  while (f > p) { pfputc(lzchar); f--; } // field pad with leading character
  for( ; p >= 0; p--)
    {
      int d = 0;
      while (d0>=dec32_table[p])
	{
	  d0 -= dec32_table[p];
	  d++;
	}
      if (d) lzf = 1;
      if (lzf || p==0)
	{
	  char c = (d > 9) ? 'A'+ (d) - 10: '0'+ (d);
	  pfputc(c);
	}
    }
}

void puthex32(int leadingz, unsigned int d0, int f)
{
  int p;
  char lzchar = leadingz? '0': ' ';
  char lzf = 0;
  for (p=0;1; p++)
    {
      if (p==sizeof(hex32_table)/sizeof(u32_t)-1) break;
      if (hex32_table[p] > d0) { p--; break; }
    }
  f--;
  while (f > p) { pfputc(lzchar); f--; }
  for (; p >= 0; p--)
    {
      int d = 0;
      while (d0>=hex32_table[p])
	{
	  d0 -= hex32_table[p];
	  d++;
	}
      if (d) lzf = 1;
      if (lzf || p==0)
	{
	  char c = (d > 9) ? 'A'+ (d) - 10: '0'+ (d);
	  pfputc(c);
	}
    }
}

static const char * _getopt_initialize(const char* optstring) {
    optind = 1;

    nextchar = NULL;

    return optstring;
}

int getopt(int argc, char *const *argv, const char *optstring) {
    optarg = NULL;
    if(optind == 0)
        optstring = _getopt_initialize(optstring);

    if(nextchar == NULL || *nextchar == '\0') {
        if(optind == argc) {
            return EOF;
        }

        if((argv[optind][0] != '-' || argv[optind][1] == '\0')) {
            optarg = argv[optind++];
            return 1;
        }

        nextchar = (argv[optind] + 1);
    }
    

    {

        char c = *nextchar++;
        char *temp = strchr(optstring, c);

        if(*nextchar == '\0')
            ++optind;

        if(temp == NULL || c == ':') {
            if(opterr)
                printf("%s: invalid option %c\n", argv[0], c);
            optopt = c;
            return '?';
        }

        if(temp[1] == ':') {
            if(temp[2] == ':') {
                if(*nextchar != '\0') {
                    optarg = nextchar;
                    optind++;
                }
                else
                    optarg = NULL;
                nextchar = NULL;
            }
            else {
                if(*nextchar != '\0') {
                    optarg = nextchar;
                    optind++;
                }
                else if(optind == argc) {
                    if(opterr) {
                        printf("%s: option requires an argument -- %c\n", argv[0], c);
                    }
                    optopt = c;
                    if(optstring[0] == ':')
                        c = ':';
                    else
                        c = '?';
                }
                else
                    optarg = argv[optind++];
                nextchar = NULL;
            }
        }
        return c;
    }
}


int fputs(const char *str, FILE *stream) {
  char c;
  int i = 0;
  
  if(!stream)
    return EOF;

  while((c = *str++) != '\0') {
    if(stream->_r == 0)
      __sflush(stream);

    *stream->_p++ = c;
    --stream->_r;
    ++i;
  }
  
  return i;
}

int fflush(FILE *stream) {
  // do nothing for now as we always immediately
  // write data to a stream
}

char *gets(char * str) {
  char* begin = str;

  while(*stdin_addr != '\n') {
    *begin++ = *stdin_addr++;
  }
  stdin_addr++;
  *begin = '\0';
  
  return str;
}

int __srefill(FILE* fp) {
  fp->_p = &(fp->data[0]);
  return _syscall_refill(fp->handle, &(fp->_r), &(fp->data[0]), STDIOLEN);
}

char* fgets(char *buf, int n, FILE *fp) {
  register size_t len;
  register char *s;
  register unsigned char *p, *t;
  
  if (n == 0)		/* sanity check */
    return (NULL);

  s = buf;
  n--;			/* leave space for NUL */
  while (n != 0) {
    /*
     * If the buffer is empty, refill it.
     */
    if ((len = fp->_r) <= 0) {
      if (__srefill(fp)) {
	/* EOF/error: stop with partial or no line */
	if (s == buf)
	  return (NULL);
	break;
      }
      len = fp->_r;
    }
    p = fp->_p;
    
    /*
     * Scan through at most n bytes of the current buffer,
     * looking for '\n'.  If found, copy up to and including
     * newline, and stop.  Otherwise, copy entire chunk
     * and loop.
     */
    if (len > n)
      len = n;
    t = memchr((void *)p, '\n', len);
    if (t != NULL) {
      len = ++t - p;
      fp->_r -= len;
      fp->_p = t;
      (void)memcpy((void *)s, (void *)p, len);
      s[len] = 0;
      return (buf);
    }
    fp->_r -= len;
    fp->_p += len;
    (void)memcpy((void *)s, (void *)p, len);
    s += len;
    n -= len;
  }
  *s = 0;
  return (buf);
}

int fgetc(FILE *fp) {
  if(fp->_r == 0) {
    __srefill(fp);
  }
  int c = *fp->_p++;
  fp->_r--;

  return c;
}

int getc(FILE *fp) {
  return fgetc(fp);
}


FILE *fopen(const char* filename, const char* mode) {
  int fid;
  int i;

  short flags;
  if(!strcmp(mode, "r")) {
    flags = _F_READ;
  } else if(!strcmp(mode, "w")) {
    flags = _F_WRIT;
  } else {
    // TODO: Add other modes
    printf("Only r and w modes are supported\n");
    return 0;
  }
  
  FILE *f = (FILE*) malloc(sizeof(FILE));

  f->_r = 0;
  f->_p = &(f->data[0]);
  f->flags = flags;
  fid = _syscall_fopen(filename, mode);
  if(fid < 0) {
    free(f);
    return 0;
  }
  f->handle = fid;

  if(f->flags == _F_READ) {
    __srefill(f);
  } else if(f->flags == _F_WRIT) {
    bzero(&(f->data[0]), STDIOLEN);
    f->_r = STDIOLEN;
  }

  return f;
}

int fclose(FILE* fp) {
  int r;
  
  if(fp->flags | _F_WRIT) {
    __sflush(fp);
  }
  r = _syscall_fclose(fp->handle);
  if(!r)
    free(fp);

  return r;
}

/*
 * Fill in the given table from the scanset at the given format
 * (just after `[').  Return a pointer to the character past the
 * closing `]'.  The table has a 1 wherever characters should be
 * considered part of the scanset.
 */
static const u_char * __sccl(register char *tab, register const u_char *fmt) {

  register int c, n, v;
  
  /* first `clear' the whole table */
  c = *fmt++;		/* first char hat => negated scanset */
  if (c == '^') {
    v = 1;		/* default => accept */
    c = *fmt++;	/* get new first char */
  } else
    v = 0;		/* default => reject */
  /* should probably use memset here */
  for (n = 0; n < 256; n++)
    tab[n] = v;
  if (c == 0)
    return (fmt - 1);/* format ended before closing ] */
  
  /*
   * Now set the entries corresponding to the actual scanset
   * to the opposite of the above.
   *
   * The first character may be ']' (or '-') without being special;
   * the last character may be '-'.
   */
  v = 1 - v;
  for (;;) {
    tab[c] = v;		/* take character c */
  doswitch:
    n = *fmt++;		/* and examine the next */
    switch (n) {
      case 0:			/* format ended too soon */
	return (fmt - 1);

      case '-':
	/*
	 * A scanset of the form
	 *	[01+-]
	 * is defined as `the digit 0, the digit 1,
	 * the character +, the character -', but
	 * the effect of a scanset such as
	 *	[a-zA-Z0-9]
	 * is implementation defined.  The V7 Unix
	 * scanf treats `a-z' as `the letters a through
	 * z', but treats `a-a' as `the letter a, the
	 * character -, and the letter a'.
	 *
	 * For compatibility, the `-' is not considerd
	 * to define a range if the character following
	 * it is either a close bracket (required by ANSI)
	 * or is not numerically greater than the character
	 * we just stored in the table (c).
	 */
	n = *fmt;
	if (n == ']' || n < c) {
	  c = '-';
	  break;	/* resume the for(;;) */
	}
	fmt++;
	do {		/* fill in the range */
	  tab[++c] = v;
	} while (c < n);
#if 1	/* XXX another disgusting compatibility hack */
	/*
	 * Alas, the V7 Unix scanf also treats formats
	 * such as [a-c-e] as `the letters a through e'.
	 * This too is permitted by the standard....
	 */
	goto doswitch;
#else
	c = *fmt++;
	if (c == 0)
	  return (fmt - 1);
	if (c == ']')
	  return (fmt);
#endif
	break;
	
    case ']':		/* end of scanset */
      return (fmt);
      
    default:		/* just another character */
      c = n;
      break;
    }
  }
  /* NOTREACHED */
}

int ungetc(int c, FILE *fp) {
  c = (unsigned char)c;
  *--fp->_p = c;
  fp->_r++;

  return c;
}

size_t fread(void *buf, size_t size, size_t count, register FILE* fp) {
  register size_t resid;
  register char *p;
  register int r;
  size_t total;
  
  if ((resid = count * size) == 0)
    return (0);
  if (fp->_r < 0)
    fp->_r = 0;
  total = resid;
  p = buf;
  while (resid > (r = fp->_r)) {
    (void)memcpy((void *)p, (void *)fp->_p, (size_t)r);
    fp->_p += r;
    /* fp->_r = 0 ... done in __srefill */
    p += r;
    resid -= r;
    if (__srefill(fp)) {
      /* no more input: return partial result */
      return ((total - resid) / size);
    }
  }
  (void)memcpy((void *)p, (void *)fp->_p, resid);
  fp->_r -= resid;
  fp->_p += resid;
  return (count);
}

#include "strtol.c"
#include "strtod.c"

int vfscanf(FILE* fp, const char* fmt0, VA_DECL_T ap) {
  register const u_char *fmt = (const u_char *)fmt0;
  register int c;	/* character from format, or conversion */
  register size_t width;/* field width, or 0 */
  register char *p;	/* points into all kinds of strings */
  register int n;	/* handy integer */
  register int flags;	/* flags as defined above */
  register char *p0;	/* saves original value of p when necessary */
  int nassigned;	/* number of fields assigned */
  int nread;		/* number of characters consumed from fp */
  int base;		/* base argument to strtol/strtoul */
  u_long (*ccfn)();	/* conversion function (strtol/strtoul) */
  char ccltab[256];	/* character class table for %[...] */
  char buf[BUF];	/* buffer for numeric conversions */
  
  /* `basefix' is used to avoid `if' tests in the integer scanner */
  static short basefix[17] =
    { 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };

  nassigned = 0;
  nread = 0;
  base = 0;		/* XXX just to keep gcc happy */
  ccfn = NULL;		/* XXX just to keep gcc happy */
  for (;;) {
    c = *fmt++;
    if (c == 0)
      return (nassigned);
    if (isspace(c)) {
      for (;;) {
	if (fp->_r <= 0 &&  __srefill(fp))
	  return (nassigned);
	if (!isspace(*fp->_p))
	  break;
	nread++, fp->_r--, fp->_p++;
      }
      continue;
    }
    if (c != '%')
      goto literal;
    width = 0;
    flags = 0;
    /*
     * switch on the format.  continue if done;
     * break once format type is derived.
     */
again:
    c = *fmt++;
    switch (c) {
      case '%':
literal:
	if (fp->_r <= 0 && __srefill(fp))
	  goto input_failure;
	if (*fp->_p != c)
	  goto match_failure;
	fp->_r--, fp->_p++;
	nread++;
	continue;

      case '*':
	flags |= SUPPRESS;
	goto again;
	
      case 'l':
	flags |= LONG;
	goto again;
	
      case 'L':
	flags |= LONGDBL;
	goto again;
	
      case 'h':
	flags |= SHORT;
	goto again;

      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
	width = width * 10 + c - '0';
	goto again;

      /*
       * Conversions.
       * Those marked `compat' are for 4.[123]BSD compatibility.
       *
       * (According to ANSI, E and X formats are supposed
       * to the same as e and x.  Sorry about that.)
       */
      case 'D':	/* compat */
	flags |= LONG;
	/* FALLTHROUGH */
      case 'd':
	c = CT_INT;
	ccfn = (u_long (*)())strtol;
	base = 10;
	break;

      case 'i':
	c = CT_INT;
	ccfn = (u_long (*)())strtol;
	base = 0;
	break;
	
      case 'O':	/* compat */
	flags |= LONG;
	/* FALLTHROUGH */
	
      case 'o':
	c = CT_INT;
	ccfn = strtoul;
	base = 8;
	break;

      case 'u':
	c = CT_INT;
	ccfn = strtoul;
	base = 10;
	break;

      case 'X':	/* compat   XXX */
	flags |= LONG;
	/* FALLTHROUGH */
      case 'x':
	flags |= PFXOK;	/* enable 0x prefixing */
	c = CT_INT;
	ccfn = strtoul;
	base = 16;
	break;

#ifdef FLOATING_POINT
      case 'E':	/* compat   XXX */
      case 'F':	/* compat */
	flags |= LONG;
	/* FALLTHROUGH */
	
      case 'e': case 'f': case 'g':
	c = CT_FLOAT;
	break;
#endif

      case 's':
	c = CT_STRING;
	break;

      case '[':
	fmt = __sccl(ccltab, fmt);
	flags |= NOSKIP;
	c = CT_CCL;
	break;

      case 'c':
	flags |= NOSKIP;
	c = CT_CHAR;
	break;

      case 'p':	/* pointer format is like hex */
	flags |= POINTER | PFXOK;
	c = CT_INT;
	ccfn = strtoul;
	base = 16;
	break;

      case 'n':
	if (flags & SUPPRESS)	/* ??? */
	  continue;
	if (flags & SHORT)
	  *PRLIBC_VA_ARG(ap, short *) = nread;
	else if (flags & LONG)
	  *PRLIBC_VA_ARG(ap, long *) = nread;
	else
	  *PRLIBC_VA_ARG(ap, int *) = nread;
	continue;

	/*
	 * Disgusting backwards compatibility hacks.	XXX
	 */
      case '\0':	/* compat */
	return (EOF);
	
      default:	/* compat */
	if (isupper(c))
	  flags |= LONG;
	c = CT_INT;
	ccfn = (u_long (*)())strtol;
	base = 10;
	break;
    }

    /*
     * We have a conversion that requires input.
     */
    if (fp->_r <= 0 && __srefill(fp))
      goto input_failure;
    
    /*
     * Consume leading white space, except for formats
     * that suppress this.
     */
    if ((flags & NOSKIP) == 0) {
      while (isspace(*fp->_p)) {
	nread++;
	if (--fp->_r > 0)
	  fp->_p++;
	else if (__srefill(fp))
	  goto input_failure;
      }
      /*
       * Note that there is at least one character in
       * the buffer, so conversions that do not set NOSKIP
       * ca no longer result in an input failure.
       */
    }
    
    /*
     * Do the conversion.
     */
    switch (c) {
      
    case CT_CHAR:
      /* scan arbitrary characters (sets NOSKIP) */
      if (width == 0)
	width = 1;
      if (flags & SUPPRESS) {
	size_t sum = 0;
	for (;;) {
	  if ((n = fp->_r) < width) {
	    sum += n;
	    width -= n;
	    fp->_p += n;
	    if (__srefill(fp)) {
	      if (sum == 0)
		goto input_failure;
	      break;
	    }
	  } else {
	    sum += width;
	    fp->_r -= width;
	    fp->_p += width;
	    break;
	  }
	}
	nread += sum;
      } else {
	size_t r = fread((void *)PRLIBC_VA_ARG(ap, char *), 1,
			 width, fp);
	
	if (r == 0)
	  goto input_failure;
	nread += r;
	nassigned++;
      }
      break;
      
    case CT_CCL:
      /* scan a (nonempty) character class (sets NOSKIP) */
      if (width == 0)
	width = ~0;	/* `infinity' */
			/* take only those things in the class */
      if (flags & SUPPRESS) {
	n = 0;
	while (ccltab[*fp->_p]) {
	  n++, fp->_r--, fp->_p++;
	  if (--width == 0)
	    break;
	  if (fp->_r <= 0 && __srefill(fp)) {
	    if (n == 0)
	      goto input_failure;
	    break;
	  }
	}
	if (n == 0)
	  goto match_failure;
      } else {
	p0 = p = PRLIBC_VA_ARG(ap, char *);
	while (ccltab[*fp->_p]) {
	  fp->_r--;
	  *p++ = *fp->_p++;
	  if (--width == 0)
	    break;
	  if (fp->_r <= 0 && __srefill(fp)) {
	    if (p == p0)
	      goto input_failure;
	    break;
	  }
	}
	n = p - p0;
	if (n == 0)
	  goto match_failure;
	*p = 0;
	nassigned++;
      }
      nread += n;
      break;
      
    case CT_STRING:
      /* like CCL, but zero-length string OK, & no NOSKIP */
      if (width == 0)
	width = ~0;
      if (flags & SUPPRESS) {
	n = 0;
	while (!isspace(*fp->_p)) {
	  n++, fp->_r--, fp->_p++;
	  if (--width == 0)
	    break;
	  if (fp->_r <= 0 && __srefill(fp))
	    break;
	}
	nread += n;
      } else {
	p0 = p = PRLIBC_VA_ARG(ap, char *);
	while (!isspace(*fp->_p)) {
	  fp->_r--;
	  *p++ = *fp->_p++;
	  if (--width == 0)
	    break;
	  if (fp->_r <= 0 && __srefill(fp))
	    break;
				}
	*p = 0;
	nread += p - p0;
	nassigned++;
      }
      continue;
      
    case CT_INT:
      /* scan an integer as if by strtol/strtoul */
#ifdef hardway
      if (width == 0 || width > sizeof(buf) - 1)
	width = sizeof(buf) - 1;
#else
      /* size_t is unsigned, hence this optimisation */
      if (--width > sizeof(buf) - 2)
	width = sizeof(buf) - 2;
      width++;
#endif
      flags |= SIGNOK | NDIGITS | NZDIGITS;
      for (p = buf; width; width--) {
	c = *fp->_p;
	/*
	 * Switch on the character; `goto ok'
	 * if we accept it as a part of number.
	 */
	switch (c) {
	  
	  /*
	   * The digit 0 is always legal, but is
	   * special.  For %i conversions, if no
	   * digits (zero or nonzero) have been
	   * scanned (only signs), we will have
	   * base==0.  In that case, we should set
	   * it to 8 and enable 0x prefixing.
	   * Also, if we have not scanned zero digits
	   * before this, do not turn off prefixing
	   * (someone else will turn it off if we
	   * have scanned any nonzero digits).
	   */
	case '0':
	  if (base == 0) {
	    base = 8;
	    flags |= PFXOK;
	  }
	  if (flags & NZDIGITS)
	    flags &= ~(SIGNOK|NZDIGITS|NDIGITS);
	  else
	    flags &= ~(SIGNOK|PFXOK|NDIGITS);
	  goto ok;
	  
	  /* 1 through 7 always legal */
	case '1': case '2': case '3':
	case '4': case '5': case '6': case '7':
	  base = basefix[base];
	  flags &= ~(SIGNOK | PFXOK | NDIGITS);
	  goto ok;
	  
	  /* digits 8 and 9 ok iff decimal or hex */
	case '8': case '9':
	  base = basefix[base];
	  if (base <= 8)
	    break;	/* not legal here */
	  flags &= ~(SIGNOK | PFXOK | NDIGITS);
	  goto ok;
	  
	  /* letters ok iff hex */
	case 'A': case 'B': case 'C':
	case 'D': case 'E': case 'F':
	case 'a': case 'b': case 'c':
	case 'd': case 'e': case 'f':
	  /* no need to fix base here */
	  if (base <= 10)
	    break;	/* not legal here */
	  flags &= ~(SIGNOK | PFXOK | NDIGITS);
	  goto ok;
	  
	  /* sign ok only as first character */
	case '+': case '-':
	  if (flags & SIGNOK) {
	    flags &= ~SIGNOK;
	    goto ok;
	  }
	  break;
	  
	  /* x ok iff flag still set & 2nd char */
	case 'x': case 'X':
	  if (flags & PFXOK && p == buf + 1) {
	    base = 16;	/* if %i */
	    flags &= ~PFXOK;
	    goto ok;
	  }
	  break;
	}
	
	/*
	 * If we got here, c is not a legal character
	 * for a number.  Stop accumulating digits.
	 */
	break;
      ok:
	/*
	 * c is legal: store it and look at the next.
	 */
	*p++ = c;
	if (--fp->_r > 0)
	  fp->_p++;
	else if (__srefill(fp))
	  break;		/* EOF */
      }
      /*
       * If we had only a sign, it is no good; push
       * back the sign.  If the number ends in `x',
       * it was [sign] '0' 'x', so push back the x
       * and treat it as [sign] '0'.
       */
      if (flags & NDIGITS) {
	if (p > buf)
	  (void) ungetc(*(u_char *)--p, fp);
	goto match_failure;
			}
      c = ((u_char *)p)[-1];
      if (c == 'x' || c == 'X') {
	--p;
	(void) ungetc(c, fp);
      }
      if ((flags & SUPPRESS) == 0) {
	u_long res;
	
	*p = 0;
	res = (*ccfn)(buf, (char **)NULL, base);
	if (flags & POINTER)
	  *PRLIBC_VA_ARG(ap, void **) = (void *)res;
	else if (flags & SHORT)
					*PRLIBC_VA_ARG(ap, short *) = res;
	else if (flags & LONG)
					*PRLIBC_VA_ARG(ap, long *) = res;
	else
	  *PRLIBC_VA_ARG(ap, int *) = res;
	nassigned++;
      }
      nread += p - buf;
      break;
      
#ifdef FLOATING_POINT
    case CT_FLOAT:
      /* scan a floating point number as if by strtod */
#ifdef hardway
      if (width == 0 || width > sizeof(buf) - 1)
	width = sizeof(buf) - 1;
#else
      /* size_t is unsigned, hence this optimisation */
      if (--width > sizeof(buf) - 2)
	width = sizeof(buf) - 2;
      width++;
#endif
      flags |= SIGNOK | NDIGITS | DPTOK | EXPOK;
      for (p = buf; width; width--) {
	c = *fp->_p;
	/*
	 * This code mimicks the integer conversion
	 * code, but is much simpler.
	 */
	switch (c) {
	  
	case '0': case '1': case '2': case '3':
	case '4': case '5': case '6': case '7':
	case '8': case '9':
	  flags &= ~(SIGNOK | NDIGITS);
	  goto fok;
	  
	case '+': case '-':
	  if (flags & SIGNOK) {
	    flags &= ~SIGNOK;
	    goto fok;
	  }
	  break;
	case '.':
	  if (flags & DPTOK) {
	    flags &= ~(SIGNOK | DPTOK);
	    goto fok;
	  }
	  break;
	case 'e': case 'E':
	  /* no exponent without some digits */
	  if ((flags&(NDIGITS|EXPOK)) == EXPOK) {
	    flags =
	      (flags & ~(EXPOK|DPTOK)) |
	      SIGNOK | NDIGITS;
	    goto fok;
	  }
					break;
	}
	break;
      fok:
	*p++ = c;
	if (--fp->_r > 0)
	  fp->_p++;
	else if (__srefill(fp))
	  break;	/* EOF */
      }
      /*
       * If no digits, might be missing exponent digits
       * (just give back the exponent) or might be missing
       * regular digits, but had sign and/or decimal point.
       */
      if (flags & NDIGITS) {
	if (flags & EXPOK) {
	  /* no digits at all */
	  while (p > buf)
	    ungetc(*(u_char *)--p, fp);
	  goto match_failure;
	}
	/* just a bad exponent (e and maybe sign) */
	c = *(u_char *)--p;
	if (c != 'e' && c != 'E') {
	  (void) ungetc(c, fp);/* sign */
	  c = *(u_char *)--p;
	}
	(void) ungetc(c, fp);
      }
      if ((flags & SUPPRESS) == 0) {
	double res;
	
	*p = 0;
	res = strtod(buf,(char **) NULL);
	if (flags & LONG)
	  *PRLIBC_VA_ARG(ap, double *) = res;
	else
	  *PRLIBC_VA_ARG(ap, float *) = res;
	nassigned++;
      }
      nread += p - buf;
      break;
#endif /* FLOATING_POINT */
    }
  }
 input_failure:
  return (nassigned ? nassigned : -1);
 match_failure:
  return (nassigned);
}

int fscanf(FILE* stream, const char* format, ...) {
  VA_DECL_T args;
  STARTPOI(args, format);

  return vfscanf(stream, format, args);
}

int __isoc99_fscanf(FILE* stream, const char* format, ...) {
  VA_DECL_T args;
  STARTPOI(args, format);

  return vfscanf(stream, format, args);
}

#include "scanf.c"

int scanf(const char * format, ...) {
  VA_DECL_T args;
  STARTPOI(args, format);

  return vsscanf(stdin_addr, format, args);
}

int sscanf(const char *s, const char * format, ...) {
  VA_DECL_T args;
  STARTPOI(args, format);

  return vsscanf(s, format, args);
}

int __isoc99_sscanf(const char *stream, const char* format, ...) {
  VA_DECL_T args;
  STARTPOI(args, format);

  return vsscanf(stream, format, args);
}

int  __drand48_iterate (unsigned short int xsubi[3],
			struct drand48_data *buffer) {
  u64_t X;
  u64_t result;

  /* Initialize buffer, if not yet done.  */
  if (!buffer->__init)
    {
      buffer->__a = 0x5deece66dull;
      buffer->__c = 0xb;
      buffer->__init = 1;
    }

  /* Do the real work.  We choose a data type which contains at least
     48 bits.  Because we compute the modulus it does not care how
     many bits really are computed.  */

  X = (u64_t) xsubi[2] << 32 | (u32_t) xsubi[1] << 16 | xsubi[0];

  result = X * buffer->__a + buffer->__c;

  xsubi[0] = result & 0xffff;
  xsubi[1] = (result >> 16) & 0xffff;
  xsubi[2] = (result >> 32) & 0xffff;

  return 0;
}

int __erand48_r(unsigned short int xsubi[3],
		struct drand48_data *buffer,
		double *result) {
  
  union ieee754_double temp;
  
  if(__drand48_iterate(xsubi, buffer) < 0)
    return -1;

  temp.ieee.negative = 0;
  temp.ieee.exponent = IEEE754_DOUBLE_BIAS;
  temp.ieee.mantissa0 = (xsubi[2] << 4) | (xsubi[1] >> 12);
  temp.ieee.mantissa1 = ((xsubi[1] & 0xfff) << 20) | (xsubi[0] << 4);

  *result = temp.d - 1.0;
}

double drand48(void) {
  double result;

  (void) __erand48_r(__prlibc_drand48_data.__x, &__prlibc_drand48_data, &result);

  return result;
}


int __srand48_r (long int seedval, struct drand48_data* buffer) {
  /* The standards say we only have 32 bits.  */
  if (sizeof (long int) > 4)
    seedval &= 0xffffffffl;

  buffer->__x[2] = seedval >> 16;
  buffer->__x[1] = seedval & 0xffffl;
  buffer->__x[0] = 0x330e;

  buffer->__a = 0x5deece66dull;
  buffer->__c = 0xb;
  buffer->__init = 1;

  return 0;
}

int  __nrand48_r (unsigned short int xsubi[3],
		  struct drand48_data *buffer,
		  long int *result) {
  /* Compute next state.  */
  if (__drand48_iterate (xsubi, buffer) < 0)
    return -1;

  /* Store the result.  */
  if (sizeof (unsigned short int) == 2)
    *result = xsubi[2] << 15 | xsubi[1] >> 1;
  else
    *result = xsubi[2] >> 1;

  return 0;
}

long int lrand48() {
  long int result;

  (void) __nrand48_r(__prlibc_drand48_data.__x,
		     &__prlibc_drand48_data,
		     &result);

  return result;
}

void srand48(long seedval) {
  (void) __srand48_r(seedval, &__prlibc_drand48_data);
}

int gettimeofday(struct timeval* tv, struct timezone* tz) {
  long int s;
  long int us;
  // for now we ignore timezone argument
  long int ctime = _syscall_time(&s, &us);
  tv->tv_sec = s;
  tv->tv_usec = us;

  return 0;
}

//
// Calling this function makes the SystemC simulation to exit.
//
// void _kill_sim(char value)
// Now in the crt.S verysimple.S


// end of cbg prlibc.c



