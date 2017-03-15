//
// PTLsim: Cycltple Accurate x86-64 Simulator
// Linux Kernel Interface
//
// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// Copyright 2000-2008 Matt T. Yourst <yourst@yourst.com>
//

#include <globals.h>
#include <superstl.h>
#include <mm.h>

#include <elf.h>
#include <asm/ldt.h>
#include <asm/ptrace.h>

#ifdef __x86_64__
#include <asm/prctl.h>
#endif

#include <ptlsim.h>
#include <osbackdoor.h>
#include <config.h>
#include <stats.h>
#include <kernel.h>
#include <loader.h>

#define __INSIDE_PTLSIM__
#include <ptlcalls.h>

#include "socdam_bdoor_mapping.h"

W64 iterations = 0; // what is this DJG?

static inline W64 do_syscall_64bit(W64 syscallid, W64 arg1, W64 arg2, W64 arg3, W64 arg4, W64 arg5, W64 arg6) {
  W64 rc;
  asm volatile ("movq %5,%%r10\n"
                "movq %6,%%r8\n"
                "movq %7,%%r9\n"
                "syscall\n"
                : "=a" (rc)
                : "0" (syscallid),"D" ((W64)(arg1)),"S" ((W64)(arg2)),
                "d" ((W64)(arg3)), "g" ((W64)(arg4)), "g" ((W64)(arg5)),
                "g" ((W64)(arg6)) 
                : "r11","rcx","memory" ,"r8", "r10", "r9" );
  return rc;
}

struct user_desc_32bit {
  W32 entry_number;
  W32 base_addr;
  W32 limit;
  W32 seg_32bit:1;
  W32 contents:2;
  W32 read_exec_only:1;
  W32 limit_in_pages:1;
  W32 seg_not_present:1;
  W32 useable:1;
};


#ifdef __x86_64__
// Parameters in: ebx ecx edx esi edi ebp
static inline W32 do_syscall_32bit(W32 sysid, W32 arg1, W32 arg2, W32 arg3, W32 arg4, W32 arg5, W32 arg6) {
  W32 rc;
  asm volatile ("push %%rbp ; movl %[arg6],%%ebp ; int $0x80 ; pop %%rbp" : "=a" (rc) :
                "a" (sysid), "b" (arg1), "c" (arg2), "d" (arg3),
                "S" (arg4), "D" (arg5), [arg6] "r" (arg6));
  return rc;
}

Waddr get_fs_base() {
  ptlsim_assert(0);
  /*if (ctx.use64) {
    Waddr fsbase;
    ptlsim_assert(sys_arch_prctl(ARCH_GET_FS, &fsbase) == 0);
    return fsbase;
  } else {
    return ldt_seg_base_cache[ctx.seg[SEGID_FS].selector >> 3];
    }*/
}

Waddr get_gs_base() {
  ptlsim_assert(0);
  /*if (ctx.use64) {
    W64 gsbase;
    ptlsim_assert(sys_arch_prctl(ARCH_GET_GS, &gsbase) == 0);
    return gsbase;
  } else {
    return ldt_seg_base_cache[ctx.seg[SEGID_GS].selector >> 3];
    }*/
}

#else
// We need this here because legacy x86 readily runs out of registers:
static W32 tempsysid;

// 32-bit only
static inline W32 do_syscall_32bit(W32 sysid, W32 arg1, W32 arg2, W32 arg3, W32 arg4, W32 arg5, W32 arg6) {
  W32 rc;
  tempsysid = sysid;

  asm volatile ("push %%ebp ; movl %%eax,%%ebp ; movl tempsysid,%%eax ; int $0x80 ; pop %%ebp" : "=a" (rc) :
                "b" (arg1), "c" (arg2), "d" (arg3), 
                "S" (arg4), "D" (arg5), "0" (arg6));
  return rc;
}

Waddr get_fs_base() {
  user_desc_32bit ud;
  memset(&ud, 0, sizeof(ud));
  ud.entry_number = ctx.seg[SEGID_FS].selector >> 3;
  int rc = sys_get_thread_area((user_desc*)&ud);
  return (rc) ? 0 : ud.base_addr;
}

Waddr get_gs_base() {
  user_desc_32bit ud;
  memset(&ud, 0, sizeof(ud));
  ud.entry_number = ctx.seg[SEGID_GS].selector >> 3;
  int rc = sys_get_thread_area((user_desc*)&ud);
  return (rc) ? 0 : ud.base_addr;
}

#endif // !__x86_64__

// Makes it easy to identify which segments PTLsim owns versus the user address space:
bool inside_ptlsim = false;

extern "C" void assert_fail(const char *__assertion, const char *__file, unsigned int __line, const char *__function) {
  stringbuf sb;
  sb << "Assert ", __assertion, " failed in ", __file, ":", __line, " (", __function, ") at iterations, ", iterations;
  //sb << sc_time_stamp();
  sb <<  endl;

  cerr << sb, flush;

  if (logfile) {
    logfile << sb, flush;
    PTLsimMachine* machine = PTLsimMachine::getcurrent();
    if (machine) machine->dump_state(logfile);
    logfile.close();
  }

  // Crash and make a core dump:
  asm("ud2a");
  abort();
}

int Context::write_segreg(unsigned int segid, W16 selector) {
  // Normal userspace PTLsim: assume it's OK
  ptlsim_assert(segid < SEGID_COUNT);

  seg[segid].selector = selector;
  update_shadow_segment_descriptors();
  return 0;
}

void Context::update_shadow_segment_descriptors() {
  W64 limit = (use64) ? 0xffffffffffffffffULL : 0xffffffffULL;

  SegmentDescriptorCache& cs = seg[SEGID_CS];
  cs.present = 1;
  cs.base = 0;
  cs.limit = limit;

  virt_addr_mask = limit;

  SegmentDescriptorCache& ss = seg[SEGID_SS];
  ss.present = 1;
  ss.base = 0;
  ss.limit = limit;

  SegmentDescriptorCache& ds = seg[SEGID_DS];
  ds.present = 1;
  ds.base = 0;
  ds.limit = limit;

  SegmentDescriptorCache& es = seg[SEGID_ES];
  es.present = 1;
  es.base = 0;
  es.limit = limit;
  
  SegmentDescriptorCache& fs = seg[SEGID_FS];
  fs.present = 1;
  fs.base = get_fs_base();
  fs.limit = limit;

  SegmentDescriptorCache& gs = seg[SEGID_GS];
  gs.present = 1;
  gs.base = get_gs_base();
  gs.limit = limit;
}

// Based on /usr/include/asm-i386/unistd.h:
#define __NR_32bit_mmap 90
#define __NR_32bit_mmap2 192
#define __NR_32bit_munmap 91
#define __NR_32bit_mprotect 125
#define __NR_32bit_mremap 163
#define __NR_32bit_brk 45
#define __NR_32bit_exit 1
#define __NR_32bit_exit_group 252
#define __NR_32bit_mremap 163
#define __NR_32bit_set_thread_area 243
#define __NR_32bit_rt_sigaction 174
#define __NR_32bit_alarm 27

#define __NR_64bit_mmap 9
#define __NR_64bit_munmap 11
#define __NR_64bit_mprotect 10
#define __NR_64bit_brk 12
#define __NR_64bit_rt_sigaction 13
#define __NR_64bit_mremap 25
#define __NR_64bit_arch_prctl 158
#define __NR_64bit_exit 60
#define __NR_64bit_gettimeofday 96
#define __NR_64bit_exit_group 231

#define __NR_64bit_alarm 37

//
// Get the processor core frequency in cycles/second:
//
static W64 core_freq_hz = 0;

W64 get_core_freq_hz() {
  if likely (core_freq_hz) return core_freq_hz;

  W64 hz = 0;

  istream cpufreqis("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");
  if (cpufreqis) {
    char s[256];
    cpufreqis >> readline(s, sizeof(s));      
    
    int khz;
    int n = sscanf(s, "%d", &khz);
    
    if (n == 1) {
      hz = (W64)khz * 1000;
      core_freq_hz = hz;
      return hz;
    }
  }
  
  istream is("/proc/cpuinfo");
  
  if (!is) {
    cerr << "get_core_freq_hz(): warning: cannot open /proc/cpuinfo. Is this a Linux machine?", endl;
    core_freq_hz = hz;
    return hz;
  }
  
  while (is) {
    char s[256];
    is >> readline(s, sizeof(s));
    
    int mhz;
    int n = sscanf(s, "cpu MHz : %d", &mhz);
    if (n == 1) {
      hz = (W64)mhz * 1000000;
      core_freq_hz = hz;
      return hz;
    }
  }

  // Can't read either of these procfiles: abort
  ptlsim_assert(false);
  return 0;
}

AddressSpace asp;

W64 ldt_seg_base_cache[LDT_SIZE];

void Context::propagate_x86_exception(byte exception, W32 errorcode, Waddr virtaddr) {
  cerr << "Exception ", exception, " (", x86_exception_names[exception], ") ec=", errorcode, endl, flush;
  ptlsim_assert(0);
  /*
Waddr rip = ctx.commitarf[REG_selfrip];

  logfile << "Exception ", exception, " (", x86_exception_names[exception], ") @ rip ", (void*)(Waddr)commitarf[REG_rip], " (";
  //logfile << sc_time_stamp << endl;
  cerr << "Exception ", exception, " (", x86_exception_names[exception], ") @ rip ", (void*)(Waddr)commitarf[REG_rip], " ", endl, flush;



  if (config.dumpcode_filename.set()) {
    byte insnbuf[1024];
    PageFaultErrorCode insn_pfec;
    Waddr insn_faultaddr;
    int valid_byte_count = copy_from_user(insnbuf, rip, sizeof(insnbuf), insn_pfec, insn_faultaddr);

    logfile << "Writing ", valid_byte_count, " bytes from rip ", (void*)rip, " to ", ((char*)config.dumpcode_filename), "...", endl, flush;
    odstream("dumpcode.dat").write(insnbuf, sizeof(insnbuf));
  }

  logfile << "Aborting...", endl, flush;
  cerr << "Aborting...", endl, flush;
  ptlsim_assert(false);*/
}

//
// Shadow page accessibility table format (x86-64 only): 
// Top level:  1048576 bytes: 131072 64-bit pointers to chunks
//
// Leaf level: 65536 bytes per chunk: 524288 bits, one per 4 KB page
// Total: 131072 chunks x 524288 pages per chunk x 4 KB per page = 48 bits virtual address space
// Total: 17 bits       + 19 bits                + 12 bits       = 48 bits virtual address space
//
// In 32-bit version, SPAT is a flat 131072-byte bit vector.
//

byte& AddressSpace::pageid_to_map_byte(spat_t top, Waddr pageid) {
#ifdef __x86_64__
  W64 chunkid = pageid >> log2(SPAT_PAGES_PER_CHUNK);

  if (!top[chunkid]) {
    top[chunkid] = (SPATChunk*)ptl_mm_alloc_private_pages(SPAT_BYTES_PER_CHUNK);
  }
  SPATChunk& chunk = *top[chunkid];
  W64 byteid = bits(pageid, 3, log2(SPAT_BYTES_PER_CHUNK));
  ptlsim_assert(byteid <= SPAT_BYTES_PER_CHUNK);
  return chunk[byteid];
#else
  return top[pageid >> 3];
#endif
}

void AddressSpace::make_accessible(void* p, Waddr size, spat_t top) {
  Waddr address = lowbits((Waddr)p, ADDRESS_SPACE_BITS);
  Waddr firstpage = (Waddr)address >> log2(PAGE_SIZE);
  Waddr lastpage = ((Waddr)address + size - 1) >> log2(PAGE_SIZE);
  if (logable(1)) {
    logfile << "SPT: Making byte range ", (void*)(firstpage << log2(PAGE_SIZE)), " to ",
      (void*)(lastpage << log2(PAGE_SIZE)), " (size ", size, ") accessible for ", 
    ((top == readmap) ? "read" : (top == writemap) ? "write" : (top == execmap) ? "exec" : "UNKNOWN"),
      endl, flush;
  }
  ptlsim_assert(ceil((W64)address + size, PAGE_SIZE) <= ADDRESS_SPACE_SIZE);
  for (W64 i = firstpage; i <= lastpage; i++) { setbit(pageid_to_map_byte(top, i), lowbits(i, 3)); }
}

void AddressSpace::make_inaccessible(void* p, Waddr size, spat_t top) {
  Waddr address = lowbits((Waddr)p, ADDRESS_SPACE_BITS);
  Waddr firstpage = (Waddr)address >> log2(PAGE_SIZE);
  Waddr lastpage = ((Waddr)address + size - 1) >> log2(PAGE_SIZE);
  if (logable(1)) {
    logfile << "SPT: Making byte range ", (void*)(firstpage << log2(PAGE_SIZE)), " to ",
      (void*)(lastpage << log2(PAGE_SIZE)), " (size ", size, ") inaccessible for ", 
    ((top == readmap) ? "read" : (top == writemap) ? "write" : (top == execmap) ? "exec" : "UNKNOWN"),
      endl, flush;
  }
  ptlsim_assert(ceil((W64)address + size, PAGE_SIZE) <= ADDRESS_SPACE_SIZE);
  for (Waddr i = firstpage; i <= lastpage; i++) { clearbit(pageid_to_map_byte(top, i), lowbits(i, 3)); }
}

AddressSpace::AddressSpace() { }

AddressSpace::~AddressSpace() { }

AddressSpace::spat_t AddressSpace::allocmap() {
#ifdef __x86_64__
  return (spat_t)ptl_mm_alloc_private_pages(SPAT_TOPLEVEL_CHUNKS * sizeof(SPATChunk*));
#else 
  return (spat_t)ptl_mm_alloc_private_pages(SPAT_BYTES);
#endif
}
void AddressSpace::freemap(AddressSpace::spat_t top) {
#ifdef __x86_64__
  if (top) {
    foreach (i, SPAT_TOPLEVEL_CHUNKS) {
      if (top[i]) ptl_mm_free_private_pages(top[i], SPAT_BYTES_PER_CHUNK);
    }
    ptl_mm_free_private_pages(top, SPAT_TOPLEVEL_CHUNKS * sizeof(SPATChunk*));
  }
#else
  if (top) {
    ptl_mm_free_private_pages(top, SPAT_BYTES);
  }
#endif
}

void AddressSpace::reset() {
  brkbase = sys_brk(0);
  brk = brkbase;

  freemap(readmap);
  freemap(writemap);
  freemap(execmap);
  freemap(dtlbmap);
  freemap(itlbmap);
  freemap(transmap);
  freemap(dirtymap);

  readmap  = allocmap();
  writemap = allocmap();
  execmap  = allocmap();
  dtlbmap  = allocmap();
  itlbmap  = allocmap();
  transmap = allocmap();
  dirtymap = allocmap();
}

void AddressSpace::setattr(void* start, Waddr length, int prot) {
  //
  // Check first if it's been assigned a non-stdin (> 0) filehandle,
  // since this may get called from ptlsim_preinit_entry before streams
  // have been set up.
  //
  if (logfile.filehandle() > 0) {
    logfile << "setattr: region ", start, " to ", (void*)((char*)start + length), " (", length >> 10, " KB) has user-visible attributes ",
      ((prot & PROT_READ) ? 'r' : '-'), ((prot & PROT_WRITE) ? 'w' : '-'), ((prot & PROT_EXEC) ? 'x' : '-'), endl;
  }

  if (prot & PROT_READ)
    allow_read(start, length);
  else disallow_read(start, length);

  if (prot & PROT_WRITE)
    allow_write(start, length);
  else disallow_write(start, length);

  if (prot & PROT_EXEC)
    allow_exec(start, length);
  else disallow_exec(start, length);
}

int AddressSpace::getattr(void* addr) {
  Waddr address = lowbits((Waddr)addr, ADDRESS_SPACE_BITS);

  Waddr page = pageid(address);

  int prot = 
    (bit(pageid_to_map_byte(readmap, page), lowbits(page, 3)) ? PROT_READ : 0) |
    (bit(pageid_to_map_byte(writemap, page), lowbits(page, 3)) ? PROT_WRITE : 0) |
    (bit(pageid_to_map_byte(execmap, page), lowbits(page, 3)) ? PROT_EXEC : 0);

  return prot;
}
 
int AddressSpace::mprotect(void* start, Waddr length, int prot) {
  length = ceil(length, PAGE_SIZE);
  int rc = sys_mprotect(start, length, prot);
  if (rc) return rc;
  setattr(start, length, prot);
  return 0;
}

int AddressSpace::munmap(void* start, Waddr length) {
  length = ceil(length, PAGE_SIZE);
  int rc = sys_munmap(start, length);
  if (rc) return rc;
  setattr(start, length, PROT_NONE);
  return 0;
}

void* AddressSpace::mmap(void* start, Waddr length, int prot, int flags, int fd, W64 offset) {
  // Guarantee enough room will be available post-alignment:
  length = ceil(length, PAGE_SIZE);
  start = sys_mmap(start, length, prot, flags, fd, offset);
  if (mmap_invalid(start)) return start;
  setattr(start, length, prot);
  if (!(flags & MAP_ANONYMOUS)) {
    //
    // Linux has strange semantics w.r.t. memory mapped files
    // when the mapped region is larger than the file itself.
    // The process should get SIGBUS when we access memory
    // beyond the end of the file, however we need a special
    // check here to emulate this behavior in the SPT bitmaps.
    // Otherwise in extremely rare cases speculative execution
    // may attempt to access memory that looks valid but isn't.
    //
    W64 origoffs = sys_seek(fd, 0, SEEK_CUR);
    W64 filesize = sys_seek(fd, 0, SEEK_END);
    sys_seek(fd, origoffs, SEEK_SET);
    if ((W64s)filesize < 0) return (void*)-EINVAL; // can't access the file?
    W64 last_page_in_file = ceil(filesize, PAGE_SIZE);
    W64 last_page_to_map = offset + length;
    W64s delta_bytes = last_page_to_map - last_page_in_file;
    if (delta_bytes <= 0) return start; // OK

    logfile << "mmap(", start, ", ", length, ", ", prot, ", ", flags, ", ", fd, ", ", offset, "): ",
      "mapping beyond end of file: file ends at byte ", last_page_in_file, " but mapping ends at byte ",
      last_page_to_map, " (", delta_bytes, " bytes marked invalid starting at ",
      ((byte*)start + length - delta_bytes), ")", endl;

    setattr((byte*)start + length - delta_bytes, delta_bytes, PROT_NONE);
  }
  return start;
}

void* AddressSpace::mremap(void* start, Waddr oldlength, Waddr newlength, int flags) {
  int oldattr = getattr(start);

  void* p = sys_mremap(start, oldlength, newlength, flags);
  if (mmap_invalid(p)) return p;

  setattr(start, oldlength, 0);
  setattr(p, newlength, oldattr);
  return p;
}

void* AddressSpace::setbrk(void* reqbrk) {
  Waddr oldsize = ceil(((Waddr)brk - (Waddr)brkbase), PAGE_SIZE);

  if (!reqbrk) {
    ptlsim_assert(brk == sys_brk(0));
    logfile << "setbrk(0): returning current brk ", brk, endl;
    return brk;
  }

  // Remove old brk
  setattr(brkbase, oldsize, PROT_NONE);

  logfile << "setbrk(", reqbrk, "): old range ", brkbase, "-", brk, " (", oldsize, " bytes); new range ", brkbase, "-", reqbrk, " (delta ", ((Waddr)reqbrk - (Waddr)brk), ", size ", ((Waddr)reqbrk - (Waddr)brkbase), ")", endl;

  void* newbrk = sys_brk(reqbrk);

  if (newbrk < brkbase) {
    // Contracting memory
    Waddr clearsize = (Waddr)brkbase - (Waddr)newbrk;
    logfile << "setbrk(", reqbrk, "): contracting: new range ", newbrk, "-", brkbase, " (clearsize ", clearsize, ")", endl, flush;
    brk = newbrk;
    brkbase = newbrk;
    setattr(brkbase, clearsize, PROT_NONE);
  } else {
    // Expanding memory
    Waddr newsize = (Waddr)newbrk - (Waddr)brkbase;
    logfile << "setbrk(", reqbrk, "): expanding: new range ", brkbase, "-", newbrk, " (size ", newsize, ")", endl, flush;
    brk = newbrk;
    setattr(brkbase, newsize, PROT_READ|PROT_WRITE|PROT_EXEC);
  }

  return newbrk;
}

#ifdef __x86_64__

const char* syscall_names_64bit[] = {
  "read", "write", "open", "close", "stat", "fstat", "lstat", "poll", "lseek", "mmap", "mprotect", "munmap", "brk", "rt_sigaction", "rt_sigprocmask", "rt_sigreturn", "ioctl", "pread64", "pwrite64", "readv", "writev", "access", "pipe", "select", "sched_yield", "mremap", "msync", "mincore", "madvise", "shmget", "shmat", "shmctl", "dup", "dup2", "pause", "nanosleep", "getitimer", "alarm", "setitimer", "getpid", "sendfile", "socket", "connect", "accept", "sendto", "recvfrom", "sendmsg", "recvmsg", "shutdown", "bind", "listen", "getsockname", "getpeername", "socketpair", "setsockopt", "getsockopt", "clone", "fork", "vfork", "execve", "exit", "wait4", "kill", "uname", "semget", "semop", "semctl", "shmdt", "msgget", "msgsnd", "msgrcv", "msgctl", "fcntl", "flock", "fsync", "fdatasync", "truncate", "ftruncate", "getdents", "getcwd", "chdir", "fchdir", "rename", "mkdir", "rmdir", "creat", "link", "unlink", "symlink", "readlink", "chmod", "fchmod", "chown", "fchown", "lchown", "umask", "gettimeofday", "getrlimit", "getrusage", "sysinfo", "times", "ptrace", "getuid", "syslog", "getgid", "setuid", "setgid", "geteuid", "getegid", "setpgid", "getppid", "getpgrp", "setsid", "setreuid", "setregid", "getgroups", "setgroups", "setresuid", "getresuid", "setresgid", "getresgid", "getpgid", "setfsuid", "setfsgid", "getsid", "capget", "capset", "rt_sigpending", "rt_sigtimedwait", "rt_sigqueueinfo", "rt_sigsuspend", "sigaltstack", "utime", "mknod", "uselib", "personality", "ustat", "statfs", "fstatfs", "sysfs", "getpriority", "setpriority", "sched_setparam", "sched_getparam", "sched_setscheduler", "sched_getscheduler", "sched_get_priority_max", "sched_get_priority_min", "sched_rr_get_interval", "mlock", "munlock", "mlockall", "munlockall", "vhangup", "modify_ldt", "pivot_root", "_sysctl", "prctl", "arch_prctl", "adjtimex", "setrlimit", "chroot", "sync", "acct", "settimeofday", "mount", "umount2", "swapon", "swapoff", "reboot", "sethostname", "setdomainname", "iopl", "ioperm", "create_module", "init_module", "delete_module", "get_kernel_syms", "query_module", "quotactl", "nfsservctl", "getpmsg", "putpmsg", "afs_syscall", "tuxcall", "security", "gettid", "readahead", "setxattr", "lsetxattr", "fsetxattr", "getxattr", "lgetxattr", "fgetxattr", "listxattr", "llistxattr", "flistxattr", "removexattr", "lremovexattr", "fremovexattr", "tkill", "time", "futex", "sched_setaffinity", "sched_getaffinity", "set_thread_area", "io_setup", "io_destroy", "io_getevents", "io_submit", "io_cancel", "get_thread_area", "lookup_dcookie", "epoll_create", "epoll_ctl_old", "epoll_wait_old", "remap_file_pages", "getdents64", "set_tid_address", "restart_syscall", "semtimedop", "fadvise64", "timer_create", "timer_settime", "timer_gettime", "timer_getoverrun", "timer_delete", "clock_settime", "clock_gettime", "clock_getres", "clock_nanosleep", "exit_group", "epoll_wait", "epoll_ctl", "tgkill", "utimes", "vserver", "vserver", "mbind", "set_mempolicy", "get_mempolicy", "mq_open", "mq_unlink", "mq_timedsend", "mq_timedreceive", "mq_notify", "mq_getsetattr", "kexec_load", "waitid"};

//
// SYSCALL instruction from x86-64 mode
//

void handle_syscall_64bit(Context& ctx) {

  bool DEBUG = 0; //analyze_in_detail();
  //
  // Handle an x86-64 syscall:
  // (This is called from the assist_syscall ucode assist)
  //

  int syscallid = ctx.commitarf[REG_rax];
  W64 arg1 = ctx.commitarf[REG_rdi];
  W64 arg2 = ctx.commitarf[REG_rsi];
  W64 arg3 = ctx.commitarf[REG_rdx];
  W64 arg4 = ctx.commitarf[REG_r10];
  W64 arg5 = ctx.commitarf[REG_r8];
  W64 arg6 = ctx.commitarf[REG_r9];

#define SYSC_MESSAGE(X)  cout << X << " -> (#", syscallid, " ", ((syscallid < lengthof(syscall_names_64bit)) ? syscall_names_64bit[syscallid] : "???"),	\
      ") from ", (void*)ctx.commitarf[REG_rcx], " args ", " (", (void*)arg1, ", ", (void*)arg2, ", ", (void*)arg3, ", ", (void*)arg4, ", ", \
      (void*)arg5, ", ", (void*)arg6, ") at iteration ", iterations, endl, flush; \

  if (DEBUG) logfile << "handle_syscall -> (#", syscallid, " ", ((syscallid < lengthof(syscall_names_64bit)) ? syscall_names_64bit[syscallid] : "???"), 
      ") from ", (void*)ctx.commitarf[REG_rcx], " args ", " (", (void*)arg1, ", ", (void*)arg2, ", ", (void*)arg3, ", ", (void*)arg4, ", ",
      (void*)arg5, ", ", (void*)arg6, ") at iteration ", iterations, endl, flush;


  switch (syscallid) {
  case __NR_read: {
    char *buf = (char *)malloc(arg3);
    int rc = do_syscall_64bit(syscallid, arg1,(W64)buf, arg3, arg4, arg5, arg6);
    //printf("syscall read rc = %i for %lli bytes '%c'\n", rc, arg3, *buf);
    if (rc>0) ctx.bdoor->os_cmd('w', buf, arg2, rc);
    free (buf);
    ctx.commitarf[REG_rax] = rc;
    break;
  }

  case __NR_write: {
    char *buf = (char *)malloc(arg3);
    ctx.bdoor->os_cmd('r', buf, arg2, arg3);
    int rc = do_syscall_64bit(syscallid, arg1,(W64)buf, arg3, arg4, arg5, arg6);
    //printf("syscall write rc = %i for %lli bytes '%c'\n", rc, arg3, *buf);
    free (buf);
    ctx.commitarf[REG_rax] = rc;
    break;
  }
  case __NR_close: {
    int rc = do_syscall_64bit(syscallid, arg1, arg2, arg3, arg4, arg5, arg6);
    //printf("syscall close rc = %i for %lli bytes \n", rc, arg3);
    ctx.commitarf[REG_rax] = rc;
    break;
  }

  case __NR_open: {
    int l1 = 4096;
    char *buf1 = (char *)malloc(l1);
    ctx.bdoor->os_cmd('s', buf1, arg1, l1);
    int rc = do_syscall_64bit(syscallid, (W64)buf1, arg2, arg3, arg4, arg5, arg6);
    printf("syscall open rc = %i for %s \n", rc, buf1);
    free(buf1);
    ctx.commitarf[REG_rax] = rc;
    break;
  }

  case __NR_64bit_exit: {
    logfile << "handle_syscall at iteration ", iterations, ": exit(): exiting with arg ", (W64s)arg1, "...", endl, flush;
    ctx.bdoor->os_svc_exit((int)arg1);
    user_process_terminated((int)arg1);
  }


  case __NR_64bit_gettimeofday: {
      printf("gettimeofday on  %llX ignored for now\n", arg1);
      int rc = 0;
      ctx.commitarf[REG_rax] = rc;
      break;

  }

  case __NR_ioctl:
    {
      printf("Ioctl on fd %lli ignored for now\n", arg1);
      int rc = 0;
      ctx.commitarf[REG_rax] = rc;
      break;
    }
    /*
  case __NR_64bit_mmap:
    ctx.commitarf[REG_rax] = (W64)asp.mmap((void*)arg1, arg2, arg3, arg4, arg5, arg6);
    break;
  case __NR_64bit_munmap:
    ctx.commitarf[REG_rax] = asp.munmap((void*)arg1, arg2);
    break;
  case __NR_64bit_mprotect:
    ctx.commitarf[REG_rax] = asp.mprotect((void*)arg1, arg2, arg3);
    break;
  case __NR_64bit_brk:
    ctx.commitarf[REG_rax] = (W64)asp.setbrk((void*)arg1);
    break;
  case __NR_64bit_mremap: {
    ctx.commitarf[REG_rax] = (W64)asp.mremap((void*)arg1, arg2, arg3, arg4);
    break;
  }
  case __NR_64bit_arch_prctl: {
    // We need to trap this so we can virtualize ARCH_SET_FS and ARCH_SET_GS:
    ctx.commitarf[REG_rax] = sys_arch_prctl(arg1, (void*)arg2);
    ctx.update_shadow_segment_descriptors();
    switch (arg1) {
    case ARCH_SET_FS:
      ctx.seg[SEGID_FS].base = arg2;
      logfile << "arch_prctl: set FS base to ", (void*)ctx.seg[SEGID_FS].base, endl;
      break;
    case ARCH_SET_GS:
      ctx.seg[SEGID_GS].base = arg2;
      logfile << "arch_prctl: set GS base to ", (void*)ctx.seg[SEGID_GS].base, endl;
      break;
    }
    break;
  }

  case __NR_64bit_exit_group: {
    logfile << "handle_syscall at iteration ", iterations, ": exit_group(): exiting with arg ", (W64s)arg1, "...", endl, flush;
    user_process_terminated((int)arg1);
  }
  case __NR_64bit_rt_sigaction: {
    // This is only so we receive SIGSEGV on our own:
#if 1
    logfile << "handle_syscall: signal(", arg1, ", ", (void*)arg2, ")", endl, flush;
    ctx.commitarf[REG_rax] = 0;
#else
    ctx.commitarf[REG_rax] = do_syscall_64bit(syscallid, arg1, arg2, arg3, arg4, arg5, arg6);
#endif
    break;
  }
  case __NR_64bit_alarm: {
    // Do not allow SIGALRM (we cannot handle it properly inside PTLsim and the timing is wrong anyway)
    // NOTE: This may break some programs!
#if 1
    logfile << "handle_syscall: alarm(", arg1, ")", endl, flush;
    ctx.commitarf[REG_rax] = 0;
#else
    ctx.commitarf[REG_rax] = do_syscall_64bit(syscallid, arg1, arg2, arg3, arg4, arg5, arg6);
#endif
    break;
  }
    */

  default:
    SYSC_MESSAGE("default");
    ptlsim_assert(0);
    ctx.commitarf[REG_rax] = do_syscall_64bit(syscallid, arg1, arg2, arg3, arg4, arg5, arg6);
    break;
  }
  //ctx.commitarf[REG_rax] = -EINVAL;
  ctx.commitarf[REG_rip] = ctx.commitarf[REG_rcx];

  if (DEBUG) logfile << "handle_syscall: result ", ctx.commitarf[REG_rax], " (", (void*)ctx.commitarf[REG_rax], "); returning to ", (void*)ctx.commitarf[REG_rip], endl, flush;
}

#endif // __x86_64__

struct old_mmap32_arg_struct {
  W32 addr;
  W32 len;
  W32 prot;
  W32 flags;
  W32 fd;
  W32 offset;
};

const char* syscall_names_32bit[] = {"restart_syscall", "exit", "fork", "read", "write", "open", "close", "waitpid", "creat", "link", "unlink", "execve", "chdir", "time", "mknod", "chmod", "lchown", "break", "oldstat", "lseek", "getpid", "mount", "umount", "setuid", "getuid", "stime", "ptrace", "alarm", "oldfstat", "pause", "utime", "stty", "gtty", "access", "nice", "ftime", "sync", "kill", "rename", "mkdir", "rmdir", "dup", "pipe", "times", "prof", "brk", "setgid", "getgid", "signal", "geteuid", "getegid", "acct", "umount2", "lock", "ioctl", "fcntl", "mpx", "setpgid", "ulimit", "oldolduname", "umask", "chroot", "ustat", "dup2", "getppid", "getpgrp", "setsid", "sigaction", "sgetmask", "ssetmask", "setreuid", "setregid", "sigsuspend", "sigpending", "sethostname", "setrlimit", "getrlimit", "getrusage", "gettimeofday", "settimeofday", "getgroups", "setgroups", "select", "symlink", "oldlstat", "readlink", "uselib", "swapon", "reboot", "readdir", "mmap", "munmap", "truncate", "ftruncate", "fchmod", "fchown", "getpriority", "setpriority", "profil", "statfs", "fstatfs", "ioperm", "socketcall", "syslog", "setitimer", "getitimer", "stat", "lstat", "fstat", "olduname", "iopl", "vhangup", "idle", "vm86old", "wait4", "swapoff", "sysinfo", "ipc", "fsync", "sigreturn", "clone", "setdomainname", "uname", "modify_ldt", "adjtimex", "mprotect", "sigprocmask", "create_module", "init_module", "delete_module", "get_kernel_syms", "quotactl", "getpgid", "fchdir", "bdflush", "sysfs", "personality", "afs_syscall", "setfsuid", "setfsgid", "_llseek", "getdents", "_newselect", "flock", "msync", "readv", "writev", "getsid", "fdatasync", "_sysctl", "mlock", "munlock", "mlockall", "munlockall", "sched_setparam", "sched_getparam", "sched_setscheduler", "sched_getscheduler", "sched_yield", "sched_get_priority_max", "sched_get_priority_min", "sched_rr_get_interval", "nanosleep", "mremap", "setresuid", "getresuid", "vm86", "query_module", "poll", "nfsservctl", "setresgid", "getresgid", "prctl", "rt_sigreturn", "rt_sigaction", "rt_sigprocmask", "rt_sigpending", "rt_sigtimedwait", "rt_sigqueueinfo", "rt_sigsuspend", "pread64", "pwrite64", "chown", "getcwd", "capget", "capset", "sigaltstack", "sendfile", "getpmsg", "putpmsg", "vfork", "ugetrlimit", "mmap2", "truncate64", "ftruncate64", "stat64", "lstat64", "fstat64", "lchown32", "getuid32", "getgid32", "geteuid32", "getegid32", "setreuid32", "setregid32", "getgroups32", "setgroups32", "fchown32", "setresuid32", "getresuid32", "setresgid32", "getresgid32", "chown32", "setuid32", "setgid32", "setfsuid32", "setfsgid32", "pivot_root", "mincore", "madvise", "madvise1", "getdents64", "fcntl64", "<unused>", "<unused>", "gettid", "readahead", "setxattr", "lsetxattr", "fsetxattr", "getxattr", "lgetxattr", "fgetxattr", "listxattr", "llistxattr", "flistxattr", "removexattr", "lremovexattr", "fremovexattr", "tkill", "sendfile64", "futex", "sched_setaffinity", "sched_getaffinity", "set_thread_area", "get_thread_area", "io_setup", "io_destroy", "io_getevents", "io_submit", "io_cancel", "fadvise64", "<unused>", "exit_group", "lookup_dcookie", "epoll_create", "epoll_ctl", "epoll_wait", "remap_file_pages", "set_tid_address", "timer_create", "statfs64", "fstatfs64", "tgkill", "utimes", "fadvise64_64", "vserver", "mbind", "get_mempolicy", "set_mempolicy", "mq_open", "sys_kexec_load", "waitid"};

W32 sysenter_retaddr = 0;

W32 get_sysenter_retaddr(W32 end_of_sysenter_insn) {
  if (!sysenter_retaddr) {
    byte* p = (byte*)(Waddr)end_of_sysenter_insn;
    logfile << "First sysenter call: finding return point starting from ", p, endl, flush;
    while (*p == 0x90) p++;
    ptlsim_assert(*p == 0xeb); // short jump
    p += 2;
    ptlsim_assert(*p == 0x5d); // "pop %ebp" instruction
    logfile << "Found sysenter return address at ", p, endl, flush;
    sysenter_retaddr = (W32)(Waddr)p;
  }
  return sysenter_retaddr;
}

void handle_syscall_32bit(int semantics) {
  ptlsim_assert(0);
  /*  bool DEBUG = 1; //analyze_in_detail();
  //
  // Handle a 32-bit syscall:
  // (This is called from the assist_syscall ucode assist)
  //

  static const char* semantics_name[] = {"int80", "syscall", "sysenter"};

  int syscallid;
  W32 arg1, arg2, arg3, arg4, arg5, arg6;
  W32 retaddr;

  if (semantics == SYSCALL_SEMANTICS_INT80) {
    //
    // int 0x80 can be called from either 32-bit or 64-bit mode,
    // but in 64-bit mode, its semantics exactly match the 32-bit
    // semantics, i.e. x86 syscall IDs, truncates addresses to 32
    // bits, etc.
    //
    syscallid = ctx.commitarf[REG_rax];
    arg1 = ctx.commitarf[REG_rbx];
    arg2 = ctx.commitarf[REG_rcx];
    arg3 = ctx.commitarf[REG_rdx];
    arg4 = ctx.commitarf[REG_rsi];
    arg5 = ctx.commitarf[REG_rdi];
    arg6 = ctx.commitarf[REG_rbp];
    retaddr = ctx.commitarf[REG_nextrip];
  } else if (semantics == SYSCALL_SEMANTICS_SYSENTER) {
    //
    // SYSENTER is just like int 0x80, but it only works in 32-bit
    // mode. Its semantics are identical to int 0x80, except that
    // %ebp contains the stack pointer to restore, and *(%ebp)
    // is the sixth argument. It always returns to a fixed address
    // in the VDSO page, so there's no need to store the address.
    // We do need to dynamically find that address though.
    //
    retaddr = get_sysenter_retaddr(ctx.commitarf[REG_nextrip]);

    ptlsim_assert(!ctx.use64);
    syscallid = ctx.commitarf[REG_rax];
    arg1 = ctx.commitarf[REG_rbx];
    arg2 = ctx.commitarf[REG_rcx];
    arg3 = ctx.commitarf[REG_rdx];
    arg4 = ctx.commitarf[REG_rsi];
    arg5 = ctx.commitarf[REG_rdi];

    W32* arg6ptr = (W32*)(Waddr)LO32(ctx.commitarf[REG_rbp]);

    if (!asp.check(arg6ptr, PROT_READ)) {
      ctx.commitarf[REG_rax] = (W64)(-EFAULT);
      ctx.commitarf[REG_rip] = retaddr;
      if (DEBUG) logfile << "handle_syscall (#", syscallid, " ", ((syscallid < lengthof(syscall_names_32bit)) ? syscall_names_32bit[syscallid] : "???"), 
                   " via ", semantics_name[semantics], ") from ", (void*)(Waddr)retaddr, " args ", " (", (void*)(Waddr)arg1, ", ", (void*)(Waddr)arg2, ", ", 
                   (void*)(Waddr)arg3, ", ", (void*)(Waddr)arg4, ", ", (void*)(Waddr)arg5, ", ???", ") at iteration ", iterations, ": arg6 @ ", arg6ptr,
                   " inaccessible via SYSENTER; returning -EFAULT", endl, flush;
    }

    arg6 = *arg6ptr;

  } else if (semantics == SYSCALL_SEMANTICS_SYSCALL) {
    ptlsim_assert(!ctx.use64);
    //
    // SYSCALL can also be used from 32-bit mode when the vsyscall
    // kernel page is used. The semantics are then as follows:
    //
    // Arguments:
    // %eax System call number.
    // %ebx Arg1
    // %ecx return EIP 
    // %edx Arg3
    // %esi Arg4
    // %edi Arg5
    // %ebp Arg2    [note: not saved in the stack frame, should not be touched]
    // %esp user stack 
    // 0(%esp) Arg6
    //
    syscallid = LO32(ctx.commitarf[REG_rax]);
    arg1 = LO32(ctx.commitarf[REG_rbx]);
    arg2 = LO32(ctx.commitarf[REG_rbp]);
    arg3 = LO32(ctx.commitarf[REG_rdx]);
    arg4 = LO32(ctx.commitarf[REG_rsi]);
    arg5 = LO32(ctx.commitarf[REG_rdi]);
    retaddr = ctx.commitarf[REG_rcx];

    W32* arg6ptr = (W32*)(Waddr)LO32(ctx.commitarf[REG_rsp]);

    if (!asp.check(arg6ptr, PROT_READ)) {
      ctx.commitarf[REG_rax] = (W64)(-EFAULT);
      ctx.commitarf[REG_rip] = retaddr;
      if (DEBUG) logfile << "handle_syscall (#", syscallid, " ", ((syscallid < lengthof(syscall_names_32bit)) ? syscall_names_32bit[syscallid] : "???"), 
                   " via ", semantics_name[semantics], ") from ", (void*)(Waddr)retaddr, " args ", " (", (void*)(Waddr)arg1, ", ", (void*)(Waddr)arg2, ", ", 
                   (void*)(Waddr)arg3, ", ", (void*)(Waddr)arg4, ", ", (void*)(Waddr)arg5, ", ???", ") at iteration ", iterations, ": arg6 @ ", arg6ptr,
                   " inaccessible; returning -EFAULT", endl, flush;
    }

    arg6 = *arg6ptr;
  } else {
    ptlsim_assert(false);
  }

  if (DEBUG) 
    logfile << "handle_syscall (#", syscallid, " ", ((syscallid < lengthof(syscall_names_32bit)) ? syscall_names_32bit[syscallid] : "???"), 
      " via ", semantics_name[semantics], ") from ", (void*)(Waddr)retaddr, " args ", " (", (void*)(Waddr)arg1, ", ", 
      (void*)(Waddr)arg2, ", ", (void*)(Waddr)arg3, ", ", (void*)(Waddr)arg4, ", ", (void*)(Waddr)arg5, ", ", (void*)(Waddr)arg6,
      ") at iteration ", iterations, endl, flush;

  switch (syscallid) {
  case __NR_32bit_mmap2:
    // mmap2 specifies the 4KB page number to allow mapping 2^(32+12) = 2^44 bit
    // files; x86-64 mmap doesn't have this silliness:
    ctx.commitarf[REG_rax] = (Waddr)asp.mmap((void*)(Waddr)arg1, arg2, arg3, arg4, arg5, arg6 << log2(PAGE_SIZE));
    break;
  case __NR_32bit_munmap:
    ctx.commitarf[REG_rax] = asp.munmap((void*)(Waddr)arg1, arg2);
    break;
  case __NR_32bit_mprotect:
    ctx.commitarf[REG_rax] = asp.mprotect((void*)(Waddr)arg1, arg2, arg3);
    break;
  case __NR_32bit_brk:
    ctx.commitarf[REG_rax] = (Waddr)asp.setbrk((void*)(Waddr)arg1);
    break;
  case __NR_32bit_mremap:
    ctx.commitarf[REG_rax] = (Waddr)asp.mremap((void*)(Waddr)arg1, arg2, arg3, arg4);
    break;
  case __NR_32bit_exit: {
    logfile << "handle_syscall at iteration ", iterations, ": exit(): exiting with arg ", (W64s)arg1, "...", endl, flush;
    user_process_terminated((int)arg1);
  }
  case __NR_32bit_exit_group: {
    logfile << "handle_syscall at iteration ", iterations, ": exit_group(): exiting with arg ", (W64s)arg1, "...", endl, flush;
    user_process_terminated((int)arg1);
  }
  case __NR_32bit_set_thread_area: {
    user_desc_32bit* desc = (user_desc_32bit*)(Waddr)arg1;
    ctx.commitarf[REG_rax] = do_syscall_32bit(syscallid, arg1, 0, 0, 0, 0, 0);
    if (!ctx.commitarf[REG_rax]) {
      logfile << "handle_syscall at iteration ", iterations, ": set_thread_area: LDT desc 0x", 
        hexstring(((desc->entry_number << 3) + 3), 16), " now has base ", (void*)(Waddr)desc->base_addr, endl, flush;
      ldt_seg_base_cache[desc->entry_number] = desc->base_addr;
      ctx.update_shadow_segment_descriptors();
    }
    break;
  }
  case __NR_32bit_rt_sigaction: {
    //++MTY This is only so we receive SIGSEGV on our own:
#if 1
    logfile << "handle_syscall: signal(", arg1, ", ", (void*)(Waddr)arg2, ")", endl, flush;
    ctx.commitarf[REG_rax] = 0;
#else
    ctx.commitarf[REG_rax] = do_syscall_32bit(syscallid, arg1, arg2, arg3, arg4, arg5, arg6);
#endif
    break;
  }
  case __NR_32bit_alarm: {
    // Do not allow SIGALRM (we cannot handle it properly inside PTLsim and the timing is wrong anyway)
    // NOTE: This may break some programs!
#if 1
    logfile << "handle_syscall: alarm(", arg1, ")", endl, flush;
    ctx.commitarf[REG_rax] = 0;
#else
    ctx.commitarf[REG_rax] = do_syscall_64bit(syscallid, arg1, arg2, arg3, arg4, arg5, arg6);
#endif
    break;
  }
  case __NR_32bit_mmap: {
    old_mmap32_arg_struct* mm = (old_mmap32_arg_struct*)(Waddr)arg1;
    ctx.commitarf[REG_rax] = (Waddr)asp.mmap((void*)(Waddr)mm->addr, mm->len, mm->prot, mm->flags, mm->fd, mm->offset);
    break;
  }
  default:
    ctx.commitarf[REG_rax] = do_syscall_32bit(syscallid, arg1, arg2, arg3, arg4, arg5, arg6);
    break;
  }
  ctx.commitarf[REG_rip] = retaddr;

  if (DEBUG) logfile << "handle_syscall: result ", ctx.commitarf[REG_rax], " (", (void*)(Waddr)ctx.commitarf[REG_rax], "); returning to ", (void*)(Waddr)ctx.commitarf[REG_rip], endl, flush;*/
}

#if 1
// get rid of me - TODO
void user_process_terminated(int rc) {
  x86_set_mxcsr(MXCSR_DEFAULT);
  logfile << "user_process_terminated(rc = ", rc, "): initiating shutdown at ";
  //logfile << sc_time_stamp() << endl;
  capture_stats_snapshot("final");
  flush_stats();
  logfile << "PTLsim exiting...", endl, flush;
  //shutdown_subsystems();
  logfile.close();
  sys_exit(rc);
}
#endif

void print_sysinfo(ostream& os) {
  // Nothing special on userspace PTLsim
}

const char* ptlcall_names[PTLCALL_COUNT] = {"nop", "marker", "switch_to_sim", "switch_to_native", "capture_stats"};

bool requested_switch_to_native = 0;

W64 handle_ptlcall(W64 rip, W64 callid, W64 arg1, W64 arg2, W64 arg3, W64 arg4, W64 arg5) {
  logfile << "PTL call from userspace (", (void*)(Waddr)rip, "): callid ", callid, " (", ((callid < PTLCALL_COUNT) ? ptlcall_names[callid] : "UNKNOWN"), 
    ") args (", (void*)(Waddr)arg1, ", ", (void*)(Waddr)arg2, ", ", (void*)(Waddr)arg3, ", ", (void*)(Waddr)arg4, ", ", (void*)(Waddr)arg5, ")", endl, flush;
  if (callid >= PTLCALL_COUNT) return (W64)(-EINVAL);

  switch (callid) {
  case PTLCALL_NOP: {
    logfile << "  (no operation)", endl;
    break;
  }
  case PTLCALL_MARKER: {
    logfile << "  MARKER: iteration ", iterations;
      //logfile << sc_time_stamp() << endl;
    break;
  };
  case PTLCALL_SWITCH_TO_SIM: {
    logfile << "  WARNING: already running in simulation mode", endl;
    return (W64)(-EINVAL);
  }
  case PTLCALL_CAPTURE_STATS: {
    const char* snapshotname = (const char*)(Waddr)arg1;
    if (asp.check((void*)snapshotname, PROT_READ)) {
      capture_stats_snapshot(snapshotname);
    } else {
      logfile << "WARNING: invalid snapshotname pointer (", snapshotname, "); using default snapshot ID", endl;
      capture_stats_snapshot(null);
    }
    break;
  }
  case PTLCALL_SWITCH_TO_NATIVE: {
    logfile << "  Switching to native mode at rip ", (void*)(Waddr)rip, endl;
    requested_switch_to_native = 1;
    break;
  }
  }
  return 0;
}

// This is where we end up after issuing opcode 0x0f37 (undocumented x86 PTL call opcode)
void assist_ptlcall(Context& ctx) {
  ctx.commitarf[REG_rax] = handle_ptlcall(ctx.commitarf[REG_nextrip], ctx.commitarf[REG_rdi], ctx.commitarf[REG_rsi], ctx.commitarf[REG_rdx], ctx.commitarf[REG_rcx], ctx.commitarf[REG_r8], ctx.commitarf[REG_r9]);
  ctx.commitarf[REG_rip] = ctx.commitarf[REG_nextrip];
}

int inject_events() {
  // No events or interrupts to inject in userspace PTLsim
  return 0;
}

bool check_for_async_sim_break() {
  if unlikely (
	       //(parent->cycles >= config.stop_at_cycle) |
               (iterations >= config.stop_at_iteration) 
               //(| sc_time_stamp() >= config.stop_at_user_insns)
	       ) {
      logfile << "Stopping simulation loop at specified limits (", iterations, " iterations, ";
      //logfile << sc_time_stamp() << endl;
      return true;
    }
  
  return false;
}
