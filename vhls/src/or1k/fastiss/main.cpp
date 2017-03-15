// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>

//
// $Id: $ standalone testbench (non SystemC) for the ISS
//

#include <assert.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include "orsim.h"
#include "or32.h"

#include "tenos.h"
#include "support/socdam-bdoor-mapping.h"
#include "../spr_defs.h"
#include "argv_backdoor.h"

#include "../../memories/memloaders.h"

#define PFX32 "%08X"

#define MAX_NO_OF_CORES 32

argv_backdoor_setup *argv_backdoor_su = 0;
const char *uart_canned_input = 0;
bool done = false;
bool putchar_backdoor_with_handles = true;
const char *image_lo = "image.out";
int n_cores = 1;
traceregion *traceregions = 0;
int g_tracelevel = 0;
const char *g_name = "noname";
int g_watches = 0;
#define MAXWATCH 32
struct watch_s
{
  u64_t lo;
  u64_t hi;
} Watches[MAXWATCH];

void hup_handler(int p_arg);

// ===========================================================================

// ===========================================================================

class memory : public memloader_if
{
public:
  bool big_endian; // Set if the loaded file is a big-endian file (it is for OpenRISC).
  u32_t mem_size_bytes;
  u32_t mem_base;

  // Data is held in this memory in host workstation endian (usually x86/little).
  u8_t *data;
  
  bool ip_write(long unsigned int a, uint8_t d, bool q)
  {
    assert(a < mem_size_bytes);
    //    printf("load %x %x %c\n", a, d, isprint(d) ? d: ' ');
   if (big_endian) a ^= 3;
    data[a-mem_base] = d;
    return true;
  }


  void set_mem8(long unsigned int a, uint8_t d)
  {
    assert(a < mem_size_bytes);
    if (big_endian) a ^= 3;
    data[a-mem_base] = d;
  }

  void set_mem16(u32_t memaddr, u32_t value)
  { 
    assert(memaddr < mem_size_bytes);
    u16_t *p = (u16_t *)(data);
    assert(big_endian);
    //if (big_endian) a ^= 3;
    p[(memaddr-mem_base)/2] = value;
  }

  void set_mem32(u32_t memaddr, u32_t value)
  { 
    assert(memaddr < mem_size_bytes);
    u32_t *p = (u32_t *)(data);
    assert(big_endian);
    p[(memaddr-mem_base)/4] = value;
  }

  bool ip_read(long unsigned int a, uint8_t &d)
  {
    assert(a < mem_size_bytes);
   if (big_endian) a ^= 3;
    d = data[a-mem_base];
    return true;
  }


#if 0
  u8_t eval_mem8(long unsigned int a)
  {
    u32_t d32 = eval_mem32(a & ~3);
    if (big_endian) a ^= 3;
    return 0xFF & (d32 >> ((a&3)*8));
  }

  u16_t eval_mem16(long unsigned int a)
  {
    assert(a < mem_size_bytes);
    u16_t *p = (u16_t *)(data);
    if (big_endian) a ^= 3;
    return data[(a-mem_base)/2];
  }
#endif

  u32_t eval_mem32(u32_t memaddr)
  {
    memaddr -= mem_base;
    u32_t rdata;
      {
	if (memaddr >= mem_size_bytes)
	  {
	    printf("%s: memaddr too large %x/%x", "mem", memaddr, mem_size_bytes);
	    exit(0);
	  }
	u32_t *p = (u32_t *)(data);
	assert(big_endian);
	//if (big_endian) a ^= 3;
	rdata = p[(memaddr)/4];
      }
    return rdata;

  }
  
  
  memory(const char *filename) // constructor
  {
    u64_t entrypt = 0;
    mem_size_bytes = 1<<26;
    mem_base = 0;
    data = (u8_t *) malloc(mem_size_bytes);
    assert(data);

    cbgelf *eloader = new cbgelf(this);
    u32_t hwm = eloader->reader(big_endian, filename, mem_base, mem_size_bytes, &entrypt);
    printf("%s: loaded elf file %s: hwm=0x" PFX32 "\n", "ormaintest-memory", filename, hwm);
  }

};

class ormaintest : public orsim
{
  u32_t lastfetch;
  const char *names;
  int tracing;
  bool bdoor64(u32_t memaddr, u64_t &rdata);
  int atomics;  // Non zero while attempting atomic bus transaction.
  u32_t atomic_wdata, atomic_addr;

public:
  const char *name() { return names; }
  const char *kind() { return "ormaintest"; }
  int counter_ticks;
  int procID;

  memory *mem;

  ormaintest(int procID, const char *s, memory *mem) : names(s), procID(procID), orsim(s), mem(mem) 
  {
    atomics = 0;
    tracing = 0;
    counter_ticks = 0;
  };

  void reset()
  {
    core_reset();
  };

  void                corepause(int us, u32_t addr);
  u32_t eval_insn(unsigned int addr, int *bp);
  u32_t            eval_mem32 (oraddr_t memaddr, int *);
  uint16_t            eval_mem16 (oraddr_t memaddr, int *);
  uint8_t             eval_mem8 (oraddr_t memaddr, int *);
  uint8_t             eval_direct8(unsigned int, int, int);

  void                set_mem32 (oraddr_t memaddr, u32_t value, int *);
  void                set_mem16 (oraddr_t memaddr, uint16_t value, int *);
  void                set_mem8 (oraddr_t memaddr, uint8_t value, int *);
  void                set_direct8 (oraddr_t, uint8_t, int, int);
  void                atomic_prefix(); // Prefix following load/store pair as atomic.

  void trace100(const char *msg) { tracing = 100; };
  void sim_done(const char *msg)
  {
    printf("%s: Done %s  ticks=%i\n", name(), msg, counter_ticks);
    done = true;
  }
};



void ormaintest::set_direct8 (oraddr_t, uint8_t, int, int)
{

};


uint8_t ormaintest::eval_direct8(unsigned int, int, int)
{
  return 0;
}



#if 0
  100:       9c 60 ff 10     l.addi r3,r0,0xffffff10
  104:       84 63 00 00     l.lwz r3,0x0(r3)
  108:       b8 63 00 07     l.slli r3,r3,0x7
  10c:       b8 63 00 06     l.slli r3,r3,0x6
  110:       18 20 00 01     l.movhi r1,0x1
  114:       a8 21 00 00     l.ori r1,r1,0x0
  118:       e0 21 18 02     l.sub r1,r1,r3
  11c:       04 00 18 ed     l.jal 64d0 <__main>
  120:       15 00 00 00     l.nop 0x0
#endif

unsigned int canned[] =
    {
      0x9c60ff10,
      //      0x10ff609c,
      0
    };


u32_t ormaintest::eval_insn(unsigned int memaddr, int *bp)
{
  int rdata = eval_mem32(memaddr, 0);
  lastfetch = memaddr;
  if (memaddr < 0x20) sim_done("PC <= 0x20");
  int q = 0;
  if (tracing > 0) { q = 1; tracing -= 1; } 
  if (q || traceregions && traceregions->check(memaddr, TENOS_TRACE_CPU_IFETCH))
    {
      //disassemble_insn(rdata);
      printf("%s: Fetch insn a=%x ins=%x %s\n", name(), memaddr, rdata, disassembled);
    }
   //if (addr >= 0x100) d = canned[(memaddr-0x100)/4];
  return rdata;
}


#define UART_BASE 0xE0009000
#define UART2_SEND    0x10  // Register offsets - one d8 register every 8 bytes
#define UART2_RECEIVE 0x18
#define UART2_CONTROL 0x20
#define UART2_STATUS  0x28

#define UART2_STATUS_RX_EMPTY (0x80)
#define UART2_STATUS_TX_EMPTY (0x40)


#define UART2_CONTROL_RX_INT_ENABLE (0x20)
#define UART2_CONTROL_TX_INT_ENABLE (0x10)

bool ormaintest::bdoor64(oraddr_t memaddr, u64_t &rdata)
{
  if ((memaddr & ~0xff) == UART_BASE)
    {
      //printf("UAR I/O dev %x \n", memaddr);
	{
	  u8_t reg = memaddr & 0xF8;
	  if (reg == UART2_STATUS) 
	    {
	      rdata = UART2_STATUS_TX_EMPTY;
	      return true;
	    }
	  printf("other UART dev read %x reg=%x \n", memaddr, reg);
	}
	return false;
    }
  memaddr &= ~7; // Force A64 for backdoor
#include "../backdoor_reads.C"
}
  
u32_t ormaintest::eval_mem32 (oraddr_t memaddr, int *bp)
{
  u32_t rdata = 0xDEADBEEF;

  if (atomics && memaddr == atomic_addr) // if an atomic exchange is being synthesised, the write info is already stored.
    {
      assert(atomics == 1);
      atomics = 0;
      rdata = mem->eval_mem32(memaddr);   // Read before write atomic operation.
      mem->set_mem32(memaddr, atomic_wdata);
      if (traceregions && traceregions->check(memaddr, TENOS_TRACE_CPU_READ)) printf("Xchg dat32 a=" PFX32 " wdata=" PFX32 " rdata=" PFX32 "\n", memaddr, atomic_wdata, rdata);
    }

  else  if (memaddr >= 0xE0000000)
    {
      u64_t rdata64;
      bdoor64(memaddr, rdata64);
      rdata = (memaddr & 4) ? rdata64: rdata64 >> 32LU;
      //printf("Backdoor %x -> %x\n", memaddr, rdata);
    }
  else 
    {
      rdata = mem->eval_mem32(memaddr);
      // hmmm gets traced twice when a slave.
      if (traceregions && traceregions->check(memaddr, TENOS_TRACE_CPU_READ))
	printf("Read dat32 a=" PFX32 " wdata=" PFX32 " rdata=" PFX32 "\n", memaddr, atomic_wdata, rdata);
    }
  return rdata;
}    

uint16_t ormaintest::eval_mem16 (oraddr_t memaddr, int *bp)
{
  u32_t rdata = eval_mem32(memaddr & ~3, bp);
  memaddr ^= 3;
  u16_t k = 0xFFFF & (rdata >> ((memaddr&2)*8));
  if (traceregions && traceregions->check(memaddr, TENOS_TRACE_CPU_READ)) printf("Short load memaddr=" PFX32 " rdata=" PFX32 " %d\n", memaddr, rdata, k);
  /*if (big_endian) */memaddr ^= 3;
  return k;
}    

uint8_t ormaintest::eval_mem8 (oraddr_t memaddr, int *bp)
{
  u32_t r0 = eval_mem32(memaddr & ~3, bp);
  memaddr ^= 3;
  u8_t rdata = r0 >> ((memaddr&3)*8);
  if (traceregions && traceregions->check(memaddr, TENOS_TRACE_CPU_READ))

    printf("%s: Byte load a=" PFX32 " r0=" PFX32 " rdata=0x%02X %c\n", name(), memaddr, r0, rdata, isprint(rdata)? rdata:' ');
  return rdata;
}    


void ormaintest::set_mem32 (oraddr_t memaddr, u32_t wdata, int *bp)
{
  if (atomics) 
    {
      assert(atomics==2);
      //printf("sw 1a %i\n", atomics);
      atomics -= 1;
      atomic_addr = memaddr;
      atomic_wdata = wdata;
    }
  else
    {
      if (traceregions && traceregions->check(memaddr, TENOS_TRACE_CPU_WRITE)) printf("Int32 store a=" PFX32 " d=%x\n", memaddr, wdata);
      
      if (memaddr >= 0xE0000000)
	{
	  memaddr &= ~7; // Force A64 for backdoor
#include "../backdoor_writes.C"
	  
	}
      
      else mem->set_mem32(memaddr, wdata);
    }    
}


void ormaintest::set_mem16 (oraddr_t memaddr, uint16_t wdata, int *bp)
{
  if (traceregions && traceregions->check(memaddr, TENOS_TRACE_CPU_WRITE)) printf("Short store " PFX32 " %x\n", memaddr, wdata);
  mem->set_mem16(memaddr, wdata);
}    

void ormaintest::set_mem8 (oraddr_t memaddr, uint8_t wdata, int *bp)
{
  if (traceregions && traceregions->check(memaddr, TENOS_TRACE_CPU_WRITE)) printf("Byte store " PFX32 " %x  %c\n", memaddr, wdata, isprint(wdata) ? wdata:' ');
  if ((memaddr & ~0xff) == UART_BASE)
    {
      u8_t reg = memaddr & 0xF8;
      if (reg == UART2_SEND) 
	{
	  //printf("Uart write %x %c\n", wdata, isprint(wdata) ? wdata:' ');
	  putchar(wdata);
	  fflush(stdout);
	  return; 
	}
      printf("other UART dev write %x reg=%x \n", memaddr, reg);
    }

  mem->set_mem8(memaddr, wdata);
}    


void ormaintest::atomic_prefix()
{
  if (traceregions && traceregions->check(0, TENOS_TRACE_CPU_WRITE))
    printf("%s:%s: atomic prefix\n", name(), kind());
  atomics = 2; // Trigger atomic transaction state machine: nb there may be instr fetches in the meantime.

}

void ormaintest::corepause(int us, u32_t addr)
{

}


class ormaintest *Ors[MAX_NO_OF_CORES];


int main(int argc, char *argv[])
{
  const char *progname = argv[0];
  signal(SIGHUP, hup_handler);


  while (argc > 1)
    {
      if (!strcmp(argv[1], "-cores"))
	{
	  n_cores = atoi(argv[2]);
	  argc -= 2; argv += 2;
	  continue;
	}
      else if (!strcmp(argv[1], "-watch"))
	{ 
	  u64_t lo, hi;
	  int c = sscanf(argv[2], "%lx", &lo);
	  if (argv[3][0] == '+') // +n for second value is a relative offset.
	    {
	      c += sscanf(argv[3]+1, "%lx", &hi);
	      hi += lo;
	    }
	  else c += sscanf(argv[3], "%lx", &hi);
	  
	  if (c != 2 || hi < lo || g_watches==MAXWATCH)
	    {
	      printf("Watch range numbers malformed\n");
	      exit(1);
	    }
	  Watches[g_watches].lo = lo;
	  Watches[g_watches].hi = hi;
	  g_watches += 1;
	  argc -= 3; argv += 3; continue;
	}


      else if (argc > 2 && !strcmp(argv[1], "-uart-input"))
	{
	  // Note: Immediate string or file data for uart model input.
	  // Note: the uart model will handle input starting with a dot or a slash as a file to read from
	  uart_canned_input = argv[2];
	  argc -= 2; argv += 2;
	  continue;
	}
      else if (argc > 2 && !strcmp(argv[1], "-image"))
	{
	  image_lo =argv[2];
	  argc -= 2; argv += 2;
	  continue;
	}
      else if (argc > 2 && !strcmp(argv[1], "-name"))
	{
	  g_name = argv[2];
	  argc -= 2; argv += 2;
	  continue;
	}

      else if (argc > 2 && !strcmp(argv[1], "-image"))
	{
	  image_lo =argv[2];
	  argc -= 2; argv += 2;
	  continue;
	}
      else if (!strcmp(argv[1], "-tracelevel"))
	{
	  g_tracelevel = atoi(argv[2]);
	  argc -= 2; argv += 2;
	  continue;
	}
      else if (argc > 2 && !strcmp(argv[1], "-name"))
	{
	  g_name = argv[2];
	  argc -= 2; argv += 2;
	  continue;
	}
      else if (!strcmp(argv[1], "--")) // All further args passed by backdoor to simulated environment.
	{
	  argv_backdoor_su = new argv_backdoor_setup(image_lo, argc-2, argv+2); // Copy these ready for backdoor access
	  break;
	}

      else
	{
	  fprintf(stderr, "Bad cmdline argument %s\nTo see supported command line args please read main.cpp\n", argv[1]);
	  printf("main.cpp", "bad argument");
	  exit(1);
	}
    }

  assert (n_cores <= MAX_NO_OF_CORES);


  u32_t flags = 0;
  if (g_tracelevel >0)
    {
      flags |=
           TENOS_TRACE_CPU_INTERRUPTS
	| ((g_tracelevel > 1) ? TENOS_TRACE_IO_READ | TENOS_TRACE_IO_WRITE: 0)
	| ((g_tracelevel > 2) ? TENOS_TRACE_CPU_IFETCH : 0)
	| ((g_tracelevel > 3) ? TENOS_TRACE_MEM_READ|TENOS_TRACE_MEM_WRITE: 0)
	
	| ((g_tracelevel > 3) ? TENOS_TRACE_CPU_KEY_REGS: 0)
	| ((g_tracelevel > 4) ? TENOS_TRACE_CPU_READ|TENOS_TRACE_CPU_WRITE : 0)
	    | ((g_tracelevel > 6) ? TENOS_TRACE_CPU_ALL_REGS: 0)
	| ((g_tracelevel > 7) ? TENOS_TRACE_BUS_OP: 0)
	;
      //printf("trace flags 0x%X\n", flags);
    
    }

  if (flags) for (int k=0; k==0 || k < g_watches; k++)
    {
      u64_t lo = k>=g_watches ? 0: Watches[k].lo;
      u32_t hi = k>=g_watches ? (u64_t)-1: Watches[k].hi;
      //printf("%i/%i Watch %lx to %lx\n", k, g_watches, lo, hi);
      (new traceregion(lo, hi, INT_MAX, flags))->add(traceregions);
    }
  memory *mem = new memory(image_lo);
  for (int k =0; k<n_cores; k++)
    {
      Ors[k] = new ormaintest(k, "top", mem);
    }

  printf("%s: Start\n", progname);
  for (int k =0; k<n_cores; k++)
    {
      Ors[k]->reset();
    }
  //printf("%s: Reset\n", progname);
  done = false;
  for(int i=0; !done; i++)
    {
      for (int k =0; k<n_cores; k++)
	{
	  ormaintest *Or = Ors[k];
	  oraddr_t pc = Or->get_pc();
	  if (traceregions)                                                                                                                                               
	    {
	      int m = traceregions->check(pc, TENOS_TRACE_CPU_KEY_REGS|TENOS_TRACE_CPU_ALL_REGS);
	      //if (m) Or->dumpreg();
	    }
	  if (Or->step()) ;
	  Or->counter_ticks += 1;
	}
    }

  printf("%s: Done\n", progname);

  return 0;
}

void orsim::trace100(const char *msg)
{
  printf("Ignored\n");
}


void hup_handler(int p_arg)
{
  printf("HUP signal received\n");
  for (int k =0; k<n_cores; k++)
    {
      Ors[k]->trace100("HUP handler");
    }
  signal (SIGHUP, hup_handler);
}


// eof
