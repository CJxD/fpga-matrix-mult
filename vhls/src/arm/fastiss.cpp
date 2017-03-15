// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2015 M Puzovic + D J Greaves


// Standalone testbench (non SystemC) for 
// ARM instruction set simulator

#include <assert.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <map>
#include "armisa.h"

#include "tenos.h"
#include "speedo_reg_mapping.h"
#include "socdam_bdoor_mapping.h"
#include "argv_backdoor.h"

#include "../memories/memloaders.h"

#include "boost/date_time/posix_time/posix_time.hpp"

#define PFX32 "%08X"

// Used for tracing information from backdoor - a feature of the simulator that is not present in the real hardware.
#define BDTRC(X) X
#define BACKDOOR_TRACE_ENABLED(X) false

#undef WITH_SPEEDO

using namespace std;
using namespace boost::posix_time;

unsigned char *argv_ram_backdoor = 0;
argv_backdoor_setup *argv_backdoor_su = 0;

bool done = false;

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

map<u32_t, int> llsc_addrs;

class memory : public memloader_if {

  public:
    // Set if the loaded file is a big-endian file
    bool big_endian; 
    u32_t mem_size_bytes;
    u32_t mem_base;

    // Data is held in this memory in host workstation endian (usually x86/little).
    u8_t *data;
  
    bool ip_write(long unsigned int a, uint8_t d, bool q) {
        assert(a < mem_size_bytes);
        //printf("load %x %x %c\n", a, d, isprint(d) ? d: ' ');
        data[a-mem_base] = d;
        return true;
    }


    void set_mem8(long unsigned int a, uint8_t d) {
        assert(a < mem_size_bytes);
        data[a-mem_base] = d;
    }

    void set_mem16(u32_t memaddr, u32_t value) {
        assert(memaddr < mem_size_bytes);
        u16_t *p = (u16_t *)(data);
        p[(memaddr-mem_base)/2] = value;
    }

    void set_mem32(u32_t memaddr, u32_t value) {
        assert(memaddr < mem_size_bytes);
        u32_t *p = (u32_t *)(data);
        p[(memaddr-mem_base)/4] = value;
    }

    bool ip_read(long unsigned int a, uint8_t &d) {
        assert(a < mem_size_bytes);
        d = data[a-mem_base];
        return true;
    }

    u8_t eval_mem8(long unsigned int a) {
        assert(a < mem_size_bytes);
        return data[a-mem_base];
    }

    u16_t eval_mem16(long unsigned int a) {
        assert(a < mem_size_bytes);
        u16_t *p = (u16_t *)(data);
        
        u16_t rdata;
        rdata = p[(a)/2];
        
        return rdata;
    }

    u32_t eval_mem32(u32_t memaddr) {
        assert(memaddr < mem_size_bytes);
        memaddr -= mem_base;
        
        u32_t rdata;
	u32_t *p = (u32_t *)(data);
	rdata = p[(memaddr)/4];
        
        return rdata;
    }
    
    // Constructor
    memory(const char *filename) {
        u64_t entrypt = 0;
        mem_size_bytes = 1<<26;
        mem_base = 0;
        data = (u8_t *) malloc(mem_size_bytes);
        assert(data);
        
        cbgelf *eloader = new cbgelf(this);
        u32_t hwm = eloader->reader(big_endian, filename, mem_base, mem_size_bytes, &entrypt);
        printf("%s: loaded elf file %s: hwm=0x" PFX32 ", endianness=%d\n", "armmaintest-memory", filename, hwm, big_endian);
        assert(!big_endian);
    }
};

class armmaintest : public armisa {

  private:
    u32_t lastfetch;
    const char *names;

    // UART read and writes
    u32_t uart_read(u32_t memaddr);
    void uart_write(u32_t memaddr, u32_t data);

  public:
    const char *name() { 
        return names; 
    }

    const char *kind() { 
        return "armmaintest"; 
    }

    int counter_ticks;
    int procID;

    memory *mem;

    // Constructor
    armmaintest(int procID, const char *s, memory *mem) 
        : names(s), 
          procID(procID), 
          armisa(procID, s),
          mem(mem) {
        counter_ticks = 0;
    }

    void reset() {
        iss_reset();
    }

    // Virtual methods from armisa class
    u1_t armisa_write1(u32_t addr, u64_t data, u2_t sf, u32_t *llsc_scp);
    bool armisa_read1(u32_t addr, u2_t sf, u1_t opf, bool linkedf, bool mmu);
    
    u1_t wbuf_logged();

    void copro_write(u4_t cpn, u4_t cp, u4_t cpr, u32_t data, u3_t op1, u3_t op2);
    u32_t copro_read(u4_t cpn, u4_t cp, u4_t cpr, u3_t op1, u3_t op2);

    int arm_sim_command(int, int, int, u32_t *);

    u32_t check_tenos_trace(u32_t mask, u32_t pc);

    void trace100(const char *msg) { 
        // Nothing to do 
    };

    void sim_done(const char *msg);

    int simulator_cmd(int cmd, int arg0, int arg1);
};

#define UART_BASE     0xE0001000
#define UART2_SEND    0x10  
#define UART2_RECEIVE 0x18
#define UART2_CONTROL 0x20
#define UART2_STATUS  0x2C
#define UART2_WRITE   0x30

#define UART2_STATUS_RX_EMPTY (0x80)
#define UART2_STATUS_TX_EMPTY (0x40)

#define UART2_CONTROL_RX_INT_ENABLE (0x20)
#define UART2_CONTROL_TX_INT_ENABLE (0x10)

u32_t armmaintest::uart_read(u32_t memaddr) {
    if((memaddr & ~0xFF) == UART_BASE) {
        u8_t reg = memaddr & 0xFF;
        if(reg == UART2_STATUS) {
            return UART2_STATUS_TX_EMPTY;
        }
        assert(0);
    }

    // Unknown address
    printf("Unknown address is %08x\n", memaddr & ~0xFF);
    assert(0);
}

void armmaintest::uart_write(u32_t memaddr, u32_t data) {
    if((memaddr & ~0xFF) == UART_BASE) {
        u8_t reg = memaddr & 0xFF;
        if(reg == UART2_WRITE) {
            char c = data & 0xFF;
            printf("%c", c);
            return;
        }
        assert(0);
    }

    // Unknown address
    printf("Unknown address is %08x\n", memaddr & ~0xFF);
    assert(0);    
}


u1_t armmaintest::armisa_write1(u32_t memaddr, u64_t wdata, u2_t sf, u32_t *llsc_scp) {
    if(llsc_scp) {
        if(llsc_addrs.find(memaddr) != llsc_addrs.end() &&
           llsc_addrs[memaddr] == procID) {
            *llsc_scp = true;
            llsc_addrs.erase(memaddr);
        } else {
            *llsc_scp = false;
        }
    }

    if(traceregions && 
       traceregions->check(memaddr, TENOS_TRACE_MEM_WRITE)) {
        printf("%s: Writes %llx at main memory address %08x --> size %d, store conditional (%d, %d)\n", 
               name(), wdata, memaddr, sf, llsc_scp != NULL, llsc_scp ? *llsc_scp : 0);
    }

    if(memaddr >= 0xE0000000) {
      assert(0);
      // DJG temporary patch here - this was not compiling for me owing to use for busaccess and lt sc_time delay variable.
      //#include "backdoor_writes.C"
        // uart write
        uart_write(memaddr, wdata);
        return true;
    }

    switch(sf) {
      case 0: {
          // writing one byte
          u8_t data = wdata & 0xFF;
          mem->set_mem8(memaddr, data);
          break;
      }

      case 1: {
          // writing two bytes
          if(memaddr & 1) {
              // unaligned access
              u8_t data = wdata & 0xFF;
              mem->set_mem8(memaddr, data);
              data = (wdata >> 8) & 0xFF;
              mem->set_mem8(memaddr+1, data);
          } else {
              // aligned access
              u16_t data = wdata & 0xFFFF;
              mem->set_mem16(memaddr, data);
          }
          break;
      }

      case 2: {
          // writing 4 bytes
          if(memaddr & 3) {
              // unaligned access
              u8_t data = wdata & 0xFF;
              mem->set_mem8(memaddr, data);
              data = (wdata >> 8) & 0xFF;
              mem->set_mem8(memaddr+1, data);
              data = (wdata >> 16) & 0xFF;
              mem->set_mem8(memaddr+2, data);
              data = (wdata >> 24) & 0xFF;
              mem->set_mem8(memaddr+3, data);              
          } else {
              u32_t data = wdata & 0xFFFFFFFF;
              mem->set_mem32(memaddr, data);
          }
          break;
      }

      case 3: {
          // writing 8 bytes
          // unaligned and aligned access
          u32_t lo = wdata & 0xFFFFFFFF;
          u32_t hi = wdata >> 32;
          mem->set_mem32(memaddr, lo);
          mem->set_mem32(memaddr+4, hi);

          break;
      }

      default:
        assert(0);
    }

    return true;
}

bool armmaintest::armisa_read1(u32_t memaddr, u2_t sf, u1_t opf, bool linkedf, bool mmu) {
    if(linkedf) {
        llsc_addrs[memaddr] = procID;
    }

    if(memaddr == 0xFFFFfffc) {
        // Spurious read on startup
        read_data = 0;
        return true;
    } else if(memaddr >= 0xE0000000) {
      assert(0);
      // DJG temporary patch here - this was not compiling for me owing to use for busaccess and lt sc_time delay variable.
      //#include "backdoor_reads.C"
        // accessing uart
        read_data = uart_read(memaddr);
        return true;
    }

    // Check to see if it is instruction
    if(opf) {
        read_data = mem->eval_mem32(memaddr);
        if(sf == 1) {
            if(memaddr & 2)
                read_data >>= 16;
            read_data &= 0xFFFF;
        }

        if(traceregions && 
           traceregions->check(memaddr, TENOS_TRACE_CPU_IFETCH)) {
            char line[1024];
            char disassembled[1024];

            snprintf(line, 1024, "%X", memaddr);
            bool print = dis.dis(memaddr, read_data, disassembled, thumb_mode);
            if(print)
                printf("%s: Fetch insn a=%s %s\n",
                       kind(), line, disassembled);
        }
        
        return true;
    }

    // Otherwise it is data read
    switch(sf) {
      case 0: {
          // one byte read
          read_data = mem->eval_mem8(memaddr);
          break;
      }

      case 1: {
          // two bytes read
          if(memaddr & 1) {
              // unaligned read
              u8_t data1 = mem->eval_mem8(memaddr);
              u8_t data2 = mem->eval_mem8(memaddr+1);

              read_data = ((u16_t)(data2 << 8)) | data1;
          } else {
              read_data = mem->eval_mem16(memaddr);
          }
          break;
      }

      case 2: {
          // four bytes read
          if(memaddr & 3) {
              u32_t part1 = mem->eval_mem32(memaddr & ~3);
              u8_t shift1 = (memaddr & 3) * 8;
              part1 = part1 >> shift1;

              u32_t part2 = mem->eval_mem32((memaddr & ~3) + 4);
              u8_t shift2 = 32 - 8 * (memaddr & 3);
              part2 = part2 << shift2;

              read_data = part1 | part2;
          } else {
              read_data = mem->eval_mem32(memaddr);
          }
          break;
      }

      case 3: {
          // 8 bytes read
          u32_t lo = mem->eval_mem32(memaddr);
          u32_t hi = mem->eval_mem32(memaddr + 4);
          read_data64 = lo | (((u64_t)hi) << 32);

          break;
      }

      default: 
        assert(0);
    }

    if(traceregions && 
       traceregions->check(memaddr, TENOS_TRACE_MEM_READ)) {
        if(sf < 3) {
            printf("%s: Reads %08x at main memory address %08x whose size was %d (load linked=%d)\n", 
                   name(), read_data, memaddr, sf, linkedf);
        } else {
            printf("%s: Reads %016llx at main memory address %08x whose size was %d (load linked=%d)\n", 
                   name(), read_data, memaddr, sf, linkedf);
        }
    }

    return true;   
}

u1_t armmaintest::wbuf_logged() {
    assert(0);
}

void armmaintest::copro_write(u4_t cpn, u4_t cp, u4_t cpr, u32_t data, u3_t op1, u3_t op2) {
    assert(0);
}

u32_t armmaintest::copro_read(u4_t cpn, u4_t cp, u4_t cpr, u3_t op1, u3_t op2) {
    assert(0);
}

void armmaintest::sim_done(const char *msg) {
    printf("%s: Done %s  ticks=%i\n", name(), msg, counter_ticks);
    done = true;
}

int armmaintest::simulator_cmd(int cmd, int arg0, int arg1, u32_t *rp) {
    const char *msg = "simulator cmd from core";
    switch(cmd) {
      case TENOS_CMD_SIM_DONE:
        sim_done(msg);
        break;

      case TENOS_CMD_TRACE100:
        trace100(msg);
        break;

      default:
        assert(0); // Command not implemented
    }
}


int armmaintest::arm_sim_command(int cmd, int arg0, int arg1, u32_t *rp) {
  return simulator_cmd(cmd, arg0, arg1, rp); 
}

u32_t armmaintest::check_tenos_trace(u32_t mask, u32_t pc) {
    assert(0);
}

class armmaintest *cores[MAX_NO_OF_CORES];

int main(int argc, char *argv[]) {
    const char *progname = argv[0];

    while (argc > 1) {
        if(!strcmp(argv[1], "-cores")) {
            n_cores = atoi(argv[2]);
            argc -= 2; argv += 2;
            continue;
	}
        else if(!strcmp(argv[1], "-watch")) {
            u64_t lo, hi;
            int c = sscanf(argv[2], "%lx", &lo);
            if (argv[3][0] == '+') { // +n for second value is a relative offset.
                c += sscanf(argv[3]+1, "%lx", &hi);
                hi += lo;
	    }
            else 
                c += sscanf(argv[3], "%lx", &hi);
	  
            if (c != 2 || hi < lo || g_watches == MAXWATCH) {
                printf("Watch range numbers malformed\n");
                exit(1);
	    }

            Watches[g_watches].lo = lo;
            Watches[g_watches].hi = hi;
            g_watches += 1;
            argc -= 3; argv += 3; continue;
	}
        else if(argc > 2 && !strcmp(argv[1], "-image")) {
            image_lo =argv[2];
            argc -= 2; argv += 2;
            continue;
	}
        else if (argc > 2 && !strcmp(argv[1], "-name")) {
            g_name = argv[2];
            argc -= 2; argv += 2;
            continue;
	}
        else if(!strcmp(argv[1], "-tracelevel")) {
            g_tracelevel = atoi(argv[2]);
            argc -= 2; argv += 2;
            continue;
	}
        else if(!strcmp(argv[1], "--")) { // All further args passed by backdoor to simulated environment.
            // Copy these ready for backdoor access
            argv_backdoor_su = new argv_backdoor_setup(image_lo, argc-2, argv+2); 
            break;
	}
        else {
            fprintf(stderr, "Bad cmdline argument %s\nTo see supported command line args please read fastiss.cpp\n", argv[1]);
            printf("fastiss.cpp", "bad argument");
            exit(1);
	}
    }

  assert (n_cores <= MAX_NO_OF_CORES);

  u32_t flags = 0;
  if (g_tracelevel > 0) {
      flags |= TENOS_TRACE_CPU_INTERRUPTS
          | ((g_tracelevel > 1) ? TENOS_TRACE_IO_READ | TENOS_TRACE_IO_WRITE : 0)
          | ((g_tracelevel > 2) ? TENOS_TRACE_CPU_IFETCH : 0)
          | ((g_tracelevel > 3) ? TENOS_TRACE_MEM_READ | TENOS_TRACE_MEM_WRITE : 0)
          | ((g_tracelevel > 3) ? TENOS_TRACE_CPU_KEY_REGS : 0)
          | ((g_tracelevel > 4) ? TENOS_TRACE_CPU_READ | TENOS_TRACE_CPU_WRITE : 0)
          | ((g_tracelevel > 6) ? TENOS_TRACE_CPU_ALL_REGS : 0)
          | ((g_tracelevel > 7) ? TENOS_TRACE_BUS_OP : 0);
  }

  if (flags) {
      for (int k=0; k==0 || k < g_watches; k++) {
          u64_t lo = k>=g_watches ? 0: Watches[k].lo;
          u32_t hi = k>=g_watches ? (u64_t)-1: Watches[k].hi;
          //printf("%i/%i Watch %lx to %lx\n", k, g_watches, lo, hi);
          (new traceregion(lo, hi, INT_MAX, flags))->add(traceregions);
      }
  }

  memory *mem = new memory(image_lo);
  for (int k =0; k<n_cores; k++) {
      cores[k] = new armmaintest(k, "top", mem);
  }

  printf("%s: Start\n", progname);

  for (int k =0; k<n_cores; k++) {
      cores[k]->reset();
  }

  printf("%s: Reset\n", progname);

  // Start simulation
  ptime start = microsec_clock::local_time();
  
  done = false;
  for(int i=0; !done; i++) {
      for (int k =0; k<n_cores; k++) {
	  armmaintest *core = cores[k];
          if(traceregions &&
             traceregions->check(core->insAddress, 
                                 TENOS_TRACE_CPU_KEY_REGS | TENOS_TRACE_CPU_ALL_REGS))
              core->PrintRegs(stdout);

          core->execute();
          core->counter_ticks++;
      }
  }
  ptime end = microsec_clock::local_time();
  time_duration diff = end - start;

  cout << "Results: " << diff.total_microseconds();
  for(int i = 0; i < n_cores; i++) {
      printf(",%d", cores[i]->stats.Instructions);
      printf(",%d",  cores[i]->stats.Cycles);
  }
  printf("\n");

  printf("%s: Done\n", progname);

  return 0;
}

// eof
