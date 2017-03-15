//
// PTLsim: Cycle Accurate x86-64 Simulator
// Sequential Core Simulator
//
// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// Copyright 2003-2008 Matt T. Yourst <yourst@yourst.com>
//

#include <globals.h>
#include <ptlsim.h>
#include <seqcore.h>
#include <branchpred.h>
#include <dcache.h>
#include <datastore.h>
#include <stats.h>

#include "speedo_reg_mapping.h"

//#include <limits.h>
#include <tenos.h>

// With these disabled, simulation is faster
#define ENABLE_CHECKS
#define ENABLE_LOGGING

#ifndef ENABLE_CHECKS
#undef assert
#define assert(x) (x)
#endif

#ifndef ENABLE_LOGGING
#undef logable
#define logable(level) (0)
#endif

W64 suppress_total_user_insn_count_updates_in_seqcore;

static const byte archreg_remap_table[TRANSREG_COUNT] = {
  REG_rax,  REG_rcx,  REG_rdx,  REG_rbx,  REG_rsp,  REG_rbp,  REG_rsi,  REG_rdi,
  REG_r8,  REG_r9,  REG_r10,  REG_r11,  REG_r12,  REG_r13,  REG_r14,  REG_r15,

  REG_xmml0,  REG_xmmh0,  REG_xmml1,  REG_xmmh1,  REG_xmml2,  REG_xmmh2,  REG_xmml3,  REG_xmmh3,
  REG_xmml4,  REG_xmmh4,  REG_xmml5,  REG_xmmh5,  REG_xmml6,  REG_xmmh6,  REG_xmml7,  REG_xmmh7,

  REG_xmml8,  REG_xmmh8,  REG_xmml9,  REG_xmmh9,  REG_xmml10,  REG_xmmh10,  REG_xmml11,  REG_xmmh11,
  REG_xmml12,  REG_xmmh12,  REG_xmml13,  REG_xmmh13,  REG_xmml14,  REG_xmmh14,  REG_xmml15,  REG_xmmh15,

  REG_fptos,  REG_fpsw,  REG_fptags,  REG_fpstack,  REG_msr,  REG_dlptr,  REG_trace, REG_ctx,

  REG_rip,  REG_flags,  REG_dlend, REG_selfrip, REG_nextrip, REG_ar1, REG_ar2, REG_zero,

  REG_temp0,  REG_temp1,  REG_temp2,  REG_temp3,  REG_temp4,  REG_temp5,  REG_temp6,  REG_temp7,

  // Notice how these (REG_zf, REG_cf, REG_of) are all mapped to REG_flags in an in-order processor:
  REG_flags,  REG_flags,  REG_flags,  REG_imm,  REG_mem,  REG_temp8,  REG_temp9,  REG_temp10,
};

const char* seqexec_result_names[SEQEXEC_RESULT_COUNT] = {
  "ok",
  "early-exit",
  "smc",
  "check",
  "unaligned",
  "exception",
  "invalidrip",
  "skipblock",
  "barrier",
  "interrupt",
};

template <int N, int setcount>
struct TransactionalMemory {
  W64 addr_list[N];
  W64 data_list[N];
  W16s next_list[N];
  W8  bytemask_list[N];

  W16s sets[setcount];
  int count;

  TransactionalMemory() {
    reset();
  }

  void reset() {
    /*
    foreach (i, N) {
      addr_list[i] = 0;
      data_list[i] = 0;
      next_list[i] = -1;
      bytemask_list[i] = 0;
    }
    */

    count = 0;
    memset(sets, 0xff, sizeof(sets));
  }

  static int setof(W64 addr) {
    W64 set = 0;
    addr >>= 3; // cut off subword bits
    return foldbits<log2(setcount)>(addr);
  }

  int lookup(W64 addr, int& set) const {
    set = setof(addr);
    W16s slot = sets[set];
    while (slot >= 0) {
      if (addr_list[slot] == addr) return slot;
      slot = next_list[slot];
    }
    return -1;
  }

  int lookup(W64 addr) const {
    int dummy;
    return lookup(addr, dummy);
  }

  bool store(W64 addr, W64 data, byte bytemask) {
    int set;
    int slot = lookup(addr, set);
    if likely (slot < 0) {
      slot = count++;
      assert(slot < N);
      addr_list[slot] = addr;
      data_list[slot] = data;
      bytemask_list[slot] = bytemask;
      next_list[slot] = sets[set];
      sets[set] = slot;
      return true;
    }

    W64& d = data_list[slot];
    d = mux64(expand_8bit_to_64bit_lut[bytemask], d, data);
    bytemask_list[slot] |= bytemask;
    return false;
  }

  bool load(W64 addr, W64& data, byte& bytemask) const {
    bytemask = 0;
    int slot = lookup(addr);
    if likely (slot < 0) return false;
    data = data_list[slot];
    bytemask = bytemask_list[slot];
    return true;
  }

  W64 load(W64 addr) const {
    W64 data;
    byte bytemask;

    W64 memdata = loadimpl(addr);
    if likely (!load(addr, data, bytemask)) return memdata;

    if likely (bytemask == 0xff) return data;
    data = mux64(expand_8bit_to_64bit_lut[bytemask], memdata, data);
    return data;
  }

  void rollback() {
    memset(sets, 0xff, sizeof(sets));
    count = 0;
  }

  void commit() {
    foreach (i, count) {
      storeimpl(addr_list[i], data_list[i], bytemask_list[i]);
    }
    memset(sets, 0xff, sizeof(sets));
    count = 0;
  }

  static W64 loadimpl(W64 addr);
  static W64 storeimpl(W64 addr, W64 data, byte bytemask);

  /*

  Sample implementations for straight virtual addresses:

  static W64 loadimpl(W64 addr) {
  return *(const W64*)addr;
  }

  static W64 storeimpl(W64 addr, W64 data, byte bytemask) {
  addr = signext64(addr, 48);
  W64& mem = *(W64*)(Waddr)addr;
  mem = mux64(expand_8bit_to_64bit_lut[bytemask], mem, data);
  return mem;
  }

  */

  int update(CommitRecord& cmtrec) {
    foreach (i, count) {
      SFR& sfr = cmtrec.stores[i];
      setzero(sfr);
      sfr.data = data_list[i];
      sfr.physaddr = addr_list[i] >> 3;
      sfr.bytemask = bytemask_list[i];
    }
    cmtrec.store_count = count;
    return count;
  }

  superstl::ostream& print(superstl::ostream& os) const {
    os << "TransactionalMemory containing ", count, " stores:", superstl::endl;
    foreach (i, count) {
      W64 data = data_list[i];
      os << "  ", intstring(i, 4), ": 0x", hexstring(W64(addr_list[i]), 64), " <= ", bytemaskstring((byte*)&data_list[i], bytemask_list[i], 8), superstl::endl;
    }
    os << "  Hash chains:", superstl::endl;
    foreach (set, setcount) {
      if (sets[set] < 0) continue;
      os << "  Set ", intstring(set, 2), ":";
      W16s slot = sets[set];
      while (slot >= 0) {
        os << ' ', slot;
        slot = next_list[slot];
      }
      os << superstl::endl;
    }
    return os;
  }
};

template <int N, int setcount>
superstl::ostream& operator <<(superstl::ostream& os, const TransactionalMemory<N, setcount>& tm) {
  return tm.print(os);
}

template <int N, int setcount>
W64 TransactionalMemory<N, setcount>::loadimpl(W64 physaddr) {
  return loadphys(physaddr);
}

template <int N, int setcount>
W64 TransactionalMemory<N, setcount>::storeimpl(W64 physaddr, W64 data, byte bytemask) {
  logfile << "Before storeimpl: physaddr ", (void*)physaddr, " => mfn ", (physaddr >> 12), ", data ", bytemaskstring(data, 8, bytemask), superstl::endl;
  W64 rc = storemask(physaddr, data, bytemask);
  logfile << "After storeimpl: physaddr ", (void*)physaddr, " => mfn ", (physaddr >> 12), ", data ", bytemaskstring(data, 8, bytemask), " => rc ", (void*)rc, superstl::endl;
  return rc;
}

enum {
  EVENT_INVALID = 0,
  EVENT_TRANSLATE,
  EVENT_EXECUTE_BB,
  EVENT_ISSUE,
  EVENT_BRANCH,
  EVENT_LOAD,
  EVENT_STORE,
  EVENT_LOAD_STORE_UNALIGNED,
  EVENT_LOAD_ANNUL,
  EVENT_SKIPBLOCK,
  EVENT_ALIGNMENT_FIXUP,
  EVENT_PTE_UPDATE,
  EVENT_ASSIST,
  EVENT_SMC,
  EVENT_COUNT,
};

//
// Event that gets written to the trace buffer
//
// In the interest of minimizing space, the cycle counters
// and uuids are only 32-bits; in practice wraparound is
// not likely to be a problem.
//
struct SequentialCoreEvent {
  W32 cycle;
  W32 uuid;
  W64 rip;
  sc_time eomid;
  byte type;
  byte coreid;
  byte threadid;
  byte uopid;
  TransOp uop;

  union {
    struct {
      IssueState state;
    } issue;
    struct {
      SFR sfr;
      W64 virtaddr;
      W64 origaddr;
      W64 pteused;
      W32 pfec;
    } loadstore;
    struct {
      int uopindex;
    } alignfixup;
    struct {
      W64 chk_recovery_rip;
      byte bytes_in_current_insn;
    } skipblock;
    struct {
      W64 virtaddr;
      W64 pteupdate;
    } pteupdate;
    struct {
      RIPVirtPhysBase rvp;
      void* bb;
      byte bbcount;
    } bb;
    struct {
      W64 rip;
      W64 ptl_pip;
      W64 next_rip;
      W64 real_target_rip;
      W16 id;
    } assist;
  };

  superstl::ostream& print(superstl::ostream& os) const;
};

PrintOperator(SequentialCoreEvent);

superstl::ostream& SequentialCoreEvent::print(superstl::ostream& os) const {
  if (uuid > 0)
    os << intstring(uuid, 20);
  else os << padstring("-", 20);
  os << " c", coreid, " t", threadid, " ";

  bool st = isstore(uop.opcode);
  bool ld = isload(uop.opcode);
  bool br = isbranch(uop.opcode);
  stringbuf sb;
  sb << uop;

  //os << "[type ", type, "]", superstl::flush;

  switch (type) {
  case EVENT_ISSUE:
  case EVENT_BRANCH: {
    stringbuf rdstr;
    print_value_and_flags(rdstr, issue.state.reg.rddata, issue.state.reg.rdflags);
    os << ((issue.state.reg.rdflags & FLAG_INV)
           ? ((br) ? "brxcpt" : "except")
           : ((br) ? "branch" : "issue "));
    os << " rip ", (void*)rip, ":", intstring(uopid, -2), "  ", padstring(sb, -60), " ", rdstr;
    break;
  }
  case EVENT_LOAD:
  case EVENT_STORE: {
    os << ((loadstore.sfr.invalid)
           ? ((st) ? "stxcpt" : "ldxcpt")
           : ((st) ? "store " : "load  "));
    os << " rip ", (void*)rip, ":", intstring(uopid, -2), "  ", padstring(sb, -60), " ", loadstore.sfr,
      " (virt 0x", hexstring(loadstore.virtaddr, 48), ")";
    if (loadstore.origaddr != loadstore.virtaddr) os << " (orig 0x", hexstring(loadstore.origaddr, 48), ")";
    if (loadstore.sfr.invalid) os << " (PFEC ", PageFaultErrorCode(loadstore.pfec), ", PTE ", Level1PTE(loadstore.pteused), ")";
    break;
  }
  case EVENT_LOAD_ANNUL: {
    os << "ldanul", " rip ", (void*)rip, ":", intstring(uopid, -2), "  ", padstring(sb, -60), " ", loadstore.sfr,
      " (virt 0x", hexstring(loadstore.virtaddr, 48), ")";
    if (loadstore.origaddr != loadstore.virtaddr) os << " (orig 0x", hexstring(loadstore.origaddr, 48), ")";
    os << " was annulled (high unaligned load)";
    break;
  }
  case EVENT_LOAD_STORE_UNALIGNED: {
    os << ((st) ? "stalgn" : "ldalgn");
    os << " rip ", (void*)rip, ":", intstring(uopid, -2), "  ", padstring(sb, -60),
      " virt 0x", hexstring(loadstore.virtaddr, 48), " (size ", (1<<uop.size), ")";
    break;
  }
  case EVENT_SKIPBLOCK: {
    os << "skip   rip ", (void*)rip, ":", intstring(uopid, -2), "  ", padstring(sb, -60), ": advance by ",
      skipblock.bytes_in_current_insn,  " bytes to ", (void*)skipblock.chk_recovery_rip;
    break;
  }
  case EVENT_ALIGNMENT_FIXUP: {
    os << "algnfx", " rip ", rip, ":", intstring(uopid, -2), " ", padstring(sb, -60),
      " set unaligned bit for uop index ", alignfixup.uopindex;
    break;
  }
  case EVENT_TRANSLATE: {
    os << "xlate  rip ", (void*)rip, " (rvp ", bb.rvp, "): BB of ", bb.bbcount, " uops";
    break;
  }
  case EVENT_EXECUTE_BB: {
    os << "execbb rip ", (void*)rip, " (rvp ", bb.rvp, "): BB of ", bb.bbcount, " uops";
    break;
  }
  case EVENT_PTE_UPDATE: {
    os << "pteupd 0x", hexstring(pteupdate.virtaddr, 48), ": ", PTEUpdate(pteupdate.pteupdate);
    break;
  }
  default: {
    os << "Unknown event type ", type, superstl::endl;
    break;
  }
  }

  //if (uop.eom) os << " [EOM #" << eomid.cstr() << "]";  
  os << superstl::endl;

  return os;
}

struct SequentialCoreEventLog {
  SequentialCoreEvent* start;
  SequentialCoreEvent* end;
  SequentialCoreEvent* tail;
  superstl::ostream* logfile;

  SequentialCoreEventLog() { start = null; end = null; tail = null; logfile = null; }

  bool init(size_t bufsize);
  void reset();

  SequentialCoreEvent* add() {
    if unlikely (tail >= end) {
      tail = start;
      if likely ((config.loglevel >= 6) || config.flush_event_log_every_cycle) flush();
    }
    SequentialCoreEvent* event = tail;
    tail++;
    return event;
  }

  void flush(bool only_to_tail = false);

  void clear() { tail = start; }

  SequentialCoreEvent* add(u64_t cycle, int type, int coreid, const TransOp& uop, W64 rip, int uopid, W64 uuid, sc_time eomid) {
    SequentialCoreEvent* event = add();
    event->type = type;
    event->cycle = cycle;
    event->uuid = uuid;
    event->rip = rip;
    event->eomid = eomid;
    event->uopid = uopid;
    event->coreid = coreid;
    event->threadid = 0;
    event->uop = uop;
    return event;
  }

  superstl::ostream& print(superstl::ostream& os, bool only_to_tail = false, W64 bbcount = limits<W64>::max);

  superstl::ostream& print_n_basic_blocks(superstl::ostream& os, W64 bbcount) {
    return print(os, false, bbcount);
  }

  superstl::ostream& print_one_basic_block(superstl::ostream& os) {
    return print_n_basic_blocks(os, 1);
  }
};

bool SequentialCoreEventLog::init(size_t bufsize) {
  reset();
  size_t bytes = bufsize * sizeof(SequentialCoreEvent);
  start = (SequentialCoreEvent*)malloc(bytes); //ptl_mm_alloc_private_pages(bytes);
  if unlikely (!start) return false;
  end = start + bufsize;
  tail = start;
  
  foreach (i, bufsize) start[i].type = EVENT_INVALID;
  return true;
}

void SequentialCoreEventLog::reset() {
  if (!start) return;

  size_t bytes = (end - start) * sizeof(SequentialCoreEvent);
  ptl_mm_free_private_pages(start, bytes);
  start = null;
  end = null;
  tail = null;
}

void SequentialCoreEventLog::flush(bool only_to_tail) {
  if unlikely (!logfile) return;
  if unlikely (!logfile->ok()) return;
  print(*logfile, only_to_tail);
  tail = start;
}

superstl::ostream& SequentialCoreEventLog::print(superstl::ostream& os, bool only_to_tail, W64 bbcount) {
  if (tail >= end) tail = start;
  if (tail < start) tail = end;
  size_t bufsize = end - start;

  SequentialCoreEvent* p = (only_to_tail) ? start : tail;

  int limit = (only_to_tail ? (tail - start) : bufsize);

  if (bbcount < limits<W64>::max) {
    limit = 0;
    p = tail - 1;
    if (p < start) p = end-1;
    foreach (i, bufsize) {
      limit++;
      if unlikely (p->type == EVENT_EXECUTE_BB) bbcount--;
      if (!bbcount) break;
      p--;
      if (p < start) p = end-1;
    }
  }

  if (!config.flush_event_log_every_cycle) os << "#-------- Start of event log --------", superstl::endl;

  W64 cycle = limits<W64>::max;
  foreach (i, limit) {
    if unlikely (p >= end) p = start;
    if unlikely (p < start) p = end-1;

    if unlikely (p->type == EVENT_INVALID) {
      p++;
      continue;
    }

    if (p->type == EVENT_EXECUTE_BB) {
      foreach (i, 24) os << "--------";
      os << superstl::endl;
    }

    if unlikely (p->cycle != cycle) {
      cycle = p->cycle;
      os << "Cycle ", cycle, ":", superstl::endl;
    }

    p->print(os);
    p++;
  }

  if (!config.flush_event_log_every_cycle) os << "#-------- End of event log --------", superstl::endl;

  return os;
}

static SequentialCoreEventLog eventlog;

struct SequentialCore {
  Context& ctx;
  CommitRecord* cmtrec;
  x86core* parent;

  //SequentialCore(): ctx(contextof(0)), cmtrec(null), parent(null), load_linked(false), load_linked_uopindex(-1) { }
  /*SequentialCore(x86core* parent_) : ctx(contextof(0)), cmtrec(null), parent(parent_), load_linked(false), load_linked_uopindex(-1) {
    if unlikely(config.event_log_enabled && (!eventlog.start)) {
	eventlog.init(config.event_log_ring_buffer_size);
	eventlog.logfile = &logfile;
     }

    external_to_core_state(ctx);
    logfile << "VCPU initial state:", superstl::endl;
    logfile << ctx, superstl::endl;
    }*/

  SequentialCore(Context& ctx_, x86core* parent_ = null, CommitRecord* cmtrec_ = null)
      : ctx(ctx_), cmtrec(cmtrec_), parent(parent_), load_linked(false), load_linked_uopindex(-1), failureNum(1) { 
    if unlikely(config.event_log_enabled && (!eventlog.start)) {
	eventlog.init(config.event_log_ring_buffer_size);
	eventlog.logfile = &logfile;
    }

    current_icache_block = 0;
    external_to_core_state(ctx);
    logfile << "VCPU initial state:", superstl::endl;
    logfile << ctx, superstl::endl;

  }

  BasicBlock* current_basic_block;
  int bytes_in_current_insn;
  int current_uop_in_macro_op;
  W64 current_uuid;

  // (n/a):
  W64 fetch_blocks_fetched;
  W64 fetch_uops_fetched;
  W64 fetch_user_insns_fetched;

  W64 bbcache_inserts;
  W64 bbcache_removes;

  CycleTimer ctseq;

  W64 seq_total_user_insns_committed;

  // MP 31May13 - used by our infastructure to query instruction cache
  W64 current_icache_block;

  //
  // Shadow flags are maintained for each archreg to simulate renaming,
  // since the x86 decoder assumes renaming will be done and hence may
  // specify some uops as "don't update user flags".
  //
  W64 arf[TRANSREG_COUNT];
  W16 arflags[TRANSREG_COUNT];

  // data used by load-linked/store-conditional
  // MP - 13Jun13
  W64 arfllsc[TRANSREG_COUNT];
  W16 arflagsllsc[TRANSREG_COUNT];
  bool load_linked;
  int load_linked_uopindex;
  W64 ll_sc_data;
  Waddr ll_sc_address;

  // used for exponential backoff
  int failureNum;

  void save_state_after_load_linked(int current_index, Waddr addr, W64 data) {
    for(int i = 0; i < TRANSREG_COUNT; i++) {
      arfllsc[i] = arf[i];
      arflagsllsc[i] = arflags[i];
    }
    
    load_linked = true;
    load_linked_uopindex = current_index;

    ll_sc_data = data;
    ll_sc_address = addr;
  }

  void reset_state_after_succesfull_store_conditional(bool same) {
    load_linked = false;
    load_linked_uopindex = -1;

    ll_sc_data = -1;
    ll_sc_address = -1;

    if(!same)
        failureNum = 1;
  }

  int load_state_after_failed_store_conditional() {
    assert(load_linked);
    assert(load_linked_uopindex >= 0);

    for(int i = 0; i < TRANSREG_COUNT; i++) {
      arf[i] = arfllsc[i];
      arflags[i] = arflagsllsc[i];
    }

    load_linked = false;
    int ret = load_linked_uopindex;
    load_linked_uopindex = -1;

    ll_sc_data = -1;
    ll_sc_address = -1;

    failureNum++;

    return ret;
  }

  

  TransactionalMemory<MAX_STORES_IN_COMMIT_RECORD, 16> transactmem;

  superstl::ostream& print_state(superstl::ostream& os) {
    os << "General state:", parent->name(), superstl::endl;
    os << "  RIP:                ", (void*)(Waddr)arf[REG_rip], superstl::endl;
    os << "  Flags:              ", hexstring(arf[REG_flags], 16), " ", flagstring(arf[REG_flags]), superstl::endl;
    os << "  UUID:               ", current_uuid, superstl::endl;
    os << "  Bytes in macro-op:  ", bytes_in_current_insn, superstl::endl;
    os << "  Uop in macro-op:    ", current_uop_in_macro_op, superstl::endl;
    os << "Basic block state:", superstl::endl;
    os << "  BBcache block:      ", current_basic_block, superstl::endl;
    os << "  uop count in block: ", (current_basic_block) ? current_basic_block->count : 0, superstl::endl;
    os << "Register state:       ", superstl::endl;

    static const int width = 4;
    foreach (i, TRANSREG_COUNT) {
      stringbuf flagsb; flagsb << flagstring(arflags[i]);
      os << "    ", padstring(arch_reg_names[i], -6), " 0x", hexstring(arf[i], 64), "|", padstring(flagsb, -6), "  ";
      if ((i % width) == (width-1)) os << superstl::endl;
    }

    return os;
  }

  void reset_fetch(W64 realrip) {
    arf[REG_rip] = realrip;
    current_basic_block = null;
  }

  enum {
    ISSUE_COMPLETED = 1,
    ISSUE_REFETCH = 0,
    ISSUE_EXCEPTION = -1,
  };

  //
  // Address generation common to both loads and stores
  //
  template <int STORE>
  Waddr addrgen(const TransOp& uop, SFR& state, Waddr& origaddr, W64 ra, W64 rb, W64 rc, PTEUpdate& pteupdate, Waddr& addr, int& exception, PageFaultErrorCode& pfec, Level1PTE& pteused, bool& annul) {
    int sizeshift = uop.size;
    int aligntype = uop.cond;
    bool internal = uop.internal;
    bool signext = (uop.opcode == OP_ldx);

    Waddr rip = arf[REG_rip];

    addr = (STORE) ? (ra + rb) : ((aligntype == LDST_ALIGN_NORMAL) ? (ra + rb) : ra);


    //
    // x86-64 requires virtual addresses to be canonical: if bit 47 is set, 
    // all upper 16 bits must be set. If this is not true, we need to signal
    // a general protection fault.
    //
    addr = (W64)signext64(addr, 48);
    // MP 22May13 - commented out for now as we are not
    // setting 
    //addr &= ctx.virt_addr_mask;
    origaddr = addr;
    annul = 0;

    switch (aligntype) {
    case LDST_ALIGN_NORMAL:
      break;
    case LDST_ALIGN_LO:
      addr = floor(addr, 8); break;
    case LDST_ALIGN_HI:
      //
      // Is the high load ever even used? If not, don't check for exceptions;
      // otherwise we may erroneously flag page boundary conditions as invalid
      //
      addr = floor(addr, 8);
      annul = (floor(origaddr + ((1<<sizeshift)-1), 8) == addr);
      addr += 8; 
      break;
    }

    state.physaddr = addr >> 3;
    state.invalid = 0;
    state.addrvalid = 1;
    state.datavalid = 0;

    //
    // Special case: if no part of the actual user load/store falls inside
    // of the high 64 bits, do not perform the access and do not signal
    // any exceptions if that page was invalid.
    //
    // However, we must be extremely careful if we're inheriting an SFR
    // from an earlier store: the earlier store may have updated some
    // bytes in the high 64-bit chunk even though we're not updating
    // any bytes. In this case we still must do the write since it
    // could very well be the final commit to that address. In any
    // case, the SFR mismatch and LSAT must still be checked.
    //
    // The store commit code checks if the bytemask is zero and does
    // not attempt the actual store if so. This will always be correct
    // for high stores as described in this scenario.
    //

    exception = 0;

    // MP 22May13 - Commented out because we are not setting ctx yet
    //W64 physaddr = (annul) ? INVALID_PHYSADDR : ctx.check_and_translate(addr, uop.size, STORE, uop.internal, exception, pfec, pteupdate, pteused);
    //return physaddr;
    return addr;
  }

  //
  // Handle exceptions common to both loads and stores
  //
  template <bool STORE>
  int handle_common_exceptions(const TransOp& uop, SFR& state, Waddr& origaddr, Waddr& addr, int& exception, PageFaultErrorCode& pfec, Level1PTE& pteused) {
    if likely (!exception) return ISSUE_COMPLETED;

    int aligntype = uop.cond;

    state.invalid = 1;
    state.data = exception | ((W64)pfec << 32);
    state.datavalid = 1;

    if likely (exception == EXCEPTION_UnalignedAccess) {
      //
      // If we have an unaligned access, mark all loads and stores at this 
      // macro-op's rip as being unaligned and remove the basic block from
      // the bbcache so it gets retranslated with properly split loads
      // and stores after we resume fetching.
      //
      // As noted elsewhere, the bbcache is for simulator purposes only;
      // the real hardware would detect unaligned uops in the fetch stage
      // and split them up on the fly. For simulation, it's more efficient
      // to just split them once in the bbcache; this has no performance
      // effect on the cycle accurate results.
      //
      if unlikely (config.event_log_enabled) {
	  SequentialCoreEvent* event = eventlog.add(parent->cycles, EVENT_LOAD_STORE_UNALIGNED, ctx.vcpuid, uop, arf[REG_rip], current_uop_in_macro_op, current_uuid, sc_time_stamp());
        event->loadstore.virtaddr = origaddr;
      }

      return ISSUE_REFETCH;
    }

    if unlikely (((exception == EXCEPTION_PageFaultOnRead) | (exception == EXCEPTION_PageFaultOnWrite)) && (aligntype == LDST_ALIGN_HI)) {
      //
      // If we have a page fault on an unaligned access, and this is the high
      // half (ld.hi / st.hi) of that access, the page fault address recorded
      // in CR2 must be at the very first byte of the second page the access
      // overlapped onto (otherwise the kernel will repeatedly fault in the
      // first page, even though that one is already present.
      //
      origaddr = addr;
    }

    if unlikely (config.event_log_enabled) {
	SequentialCoreEvent* event = eventlog.add(parent->cycles, (STORE) ? EVENT_STORE : EVENT_LOAD, ctx.vcpuid, uop, arf[REG_rip], current_uop_in_macro_op, current_uuid, sc_time_stamp());
      event->loadstore.sfr = state;
      event->loadstore.virtaddr = addr;
      event->loadstore.origaddr = origaddr;
      event->loadstore.pfec = pfec;
      event->loadstore.pteused = pteused;
    }

    return ISSUE_EXCEPTION;
  }

  int issuestore(const TransOp& uop, SFR& state, Waddr& origaddr, W64 ra, W64 rb, W64 rc, PTEUpdate& pteupdate) {
    int status;
    Waddr rip = arf[REG_rip];
    int sizeshift = uop.size;
    int aligntype = uop.cond;

    Waddr addr;
    int exception = 0;
    PageFaultErrorCode pfec;
    Level1PTE pteused;
    bool annul;

    W64 physaddr = addrgen<1>(uop, state, origaddr, ra, rb, rc, pteupdate, addr, exception, pfec, pteused, annul);
  
    if unlikely ((status = handle_common_exceptions<1>(uop, state, origaddr, addr, exception, pfec, pteused)) != ISSUE_COMPLETED) return status;

    //
    // At this point all operands are valid, so merge the data and mark the store as valid.
    //    
    state.physaddr = (annul) ? INVALID_PHYSADDR : (physaddr >> 3);

    bool ready;
    byte bytemask;

    switch (aligntype) {
    case LDST_ALIGN_NORMAL:
    case LDST_ALIGN_LO:
      bytemask = ((1 << (1 << sizeshift))-1) << (lowbits(origaddr, 3));
      rc <<= 8*lowbits(origaddr, 3);
      break;
    case LDST_ALIGN_HI:
      bytemask = ((1 << (1 << sizeshift))-1) >> (8 - lowbits(origaddr, 3));
      rc >>= 8*(8 - lowbits(origaddr, 3));
    }

    state.invalid = 0;
    state.data = rc;
    state.bytemask = bytemask;
    state.datavalid = !annul;

    if unlikely (config.event_log_enabled) {
      SequentialCoreEvent* event = eventlog.add(parent->cycles, EVENT_STORE, ctx.vcpuid, uop, rip, current_uop_in_macro_op, current_uuid, sc_time_stamp());
      event->loadstore.sfr = state;
      event->loadstore.virtaddr = addr;
      event->loadstore.origaddr = origaddr;
      event->loadstore.pfec = pfec;
      event->loadstore.pteused = pteused;
    }

    if unlikely (inrange(W64(addr), config.log_trigger_virt_addr_start, config.log_trigger_virt_addr_end)) {
      W64 mfn = physaddr >> 12;
      logfile << "Trigger hit for virtual address range: STORE virt ", (void*)addr, ", phys mfn ", mfn, "+0x", hexstring(addr, 12), " <= 0x", hexstring(rc, (1 << sizeshift)*8),
        " (SFR ", state, ") by insn @ rip ", (void*)arf[REG_rip], " at cycle ", parent->cycles, ", uuid ", current_uuid, ", commits ";
      //logfile << sc_time_stamp() << superstl::endl;
      eventlog.print_one_basic_block(logfile);
    }

    return ISSUE_COMPLETED;
  }

  static inline W64 extract_bytes(void* target, int SIZESHIFT, bool SIGNEXT) {
    W64 data;
    switch (SIZESHIFT) {
    case 0:
      data = (SIGNEXT) ? (W64s)(*(W8s*)target) : (*(W8*)target); break;
    case 1:
      data = (SIGNEXT) ? (W64s)(*(W16s*)target) : (*(W16*)target); break;
    case 2:
      data = (SIGNEXT) ? (W64s)(*(W32s*)target) : (*(W32*)target); break;
    case 3:
      data = *(W64*)target; break;
    }
    return data;
  }

  int issueload(const TransOp& uop, SFR& state, Waddr& origaddr, W64 ra, W64 rb, W64 rc, PTEUpdate& pteupdate, W64& data) {
    int status;
    Waddr rip = arf[REG_rip];
    int sizeshift = uop.size;
    int aligntype = uop.cond;
    bool signext = (uop.opcode == OP_ldx);
    Waddr addr;
    int exception = 0;
    PageFaultErrorCode pfec;
    Level1PTE pteused;
    bool annul;

    W64 physaddr = addrgen<0>(uop, state, origaddr, ra, rb, rc, pteupdate, addr, exception, pfec, pteused, annul);

    if unlikely ((status = handle_common_exceptions<0>(uop, state, origaddr, addr, exception, pfec, pteused)) != ISSUE_COMPLETED) return status;

    state.physaddr = (annul) ? 0xffffffffffffffffULL : (physaddr >> 3);

    if likely (!annul) {
      if unlikely (cmtrec) {
        data = transactmem.load(state.physaddr << 3);
	// MP 29May13 - we should never be here?
	assert(0);
      } else {
        logfile << "[cycle ", parent->cycles, "] load from physaddr ", (void*)physaddr, " for virtaddr ", (void*)origaddr, superstl::endl;



        //data = loadphys(physaddr);
	// MP 29May13 - replacing load with out cache infrastructure
	Waddr addr = floor(signext64(physaddr, 48), 8);
	u64_t mydata;
	
	bool res = false;
	if (eval_bdoor_read(addr, mydata)) { res = true; }
	else res = parent->busaccess.eval_mem64(addr, mydata, NULL, parent->lt_d_delay, uop.locked);
        assert(res != false);
	data = mydata;       
      }
    }

    if unlikely (aligntype == LDST_ALIGN_HI) {
      //
      // Concatenate the aligned data from a previous ld.lo uop provided in rb
      // with the currently loaded data D as follows:
      //
      // rb | D
      //
      // Example:
      //
      // floor(a) floor(a)+8
      // ---rb--  --DD---
      // 0123456701234567
      //    XXXXXXXX
      //    ^ origaddr
      //
      if likely (!annul) {
        struct {
          W64 lo;
          W64 hi;
        } aligner;

        aligner.lo = rb;
        aligner.hi = data;

        W64 offset = lowbits(origaddr - floor(origaddr, 8), 4);

        data = extract_bytes(((byte*)&aligner) + offset, sizeshift, signext);
      } else {
        //
        // annulled: we need no data from the high load anyway; only use the low data
        // that was already checked for exceptions and forwarding:
        //
        W64 offset = lowbits(origaddr, 3);
        state.data = extract_bytes(((byte*)&rb) + offset, sizeshift, signext);
        state.invalid = 0;
        state.datavalid = 1;

        if unlikely (config.event_log_enabled) {
          SequentialCoreEvent* event = eventlog.add(parent->cycles, EVENT_LOAD_ANNUL, ctx.vcpuid, uop, rip, current_uop_in_macro_op, current_uuid, sc_time_stamp());
          event->loadstore.sfr = state;
          event->loadstore.virtaddr = addr;
          event->loadstore.origaddr = origaddr;
          event->loadstore.pfec = 0;
        }

        return ISSUE_COMPLETED;
      }
    } else {
        data = extract_bytes(((byte*)&data) + lowbits(addr, 3), sizeshift, signext);

    }

    //
    // NOTE: Technically the data is valid right now for simulation purposes
    // only; in reality it may still be arriving from the cache.
    //
    state.data = data;
    state.invalid = 0;
    state.datavalid = 1;
    state.bytemask = 0xff;

    if unlikely (config.event_log_enabled) {
      SequentialCoreEvent* event = eventlog.add(parent->cycles, EVENT_LOAD, ctx.vcpuid, uop, rip, current_uop_in_macro_op, current_uuid, sc_time_stamp());
      event->loadstore.sfr = state;
      event->loadstore.virtaddr = addr;
      event->loadstore.origaddr = origaddr;
      event->loadstore.pfec = pfec;
      event->loadstore.pteused = pteused;
    }

    if unlikely (inrange(W64(addr), config.log_trigger_virt_addr_start, config.log_trigger_virt_addr_end)) {
      W64 mfn = physaddr >> 12;
      logfile << "Trigger hit for virtual address range: LOAD virt ", (void*)addr, ", phys mfn ", mfn, "+0x", hexstring(addr, 12), " => 0x", hexstring(state.data, 64), " (SFR ", state, ") at cycle ",
        " by insn @ rip ", (void*)arf[REG_rip], " at cycle ", parent->cycles, ", uuid ", current_uuid, ", commits ";
      //logfile << sc_time_stamp(), superstl::endl;
      eventlog.print_one_basic_block(logfile);
    }

    return ISSUE_COMPLETED;
  }

  void external_to_core_state(const Context& ctx) {
    foreach (i, ARCHREG_COUNT) {
      arf[i] = ctx.commitarf[i];
      arflags[i] = 0;
    }
    for (int i = ARCHREG_COUNT; i < TRANSREG_COUNT; i++) {
      arf[i] = 0;
      arflags[i] = 0;
    }

    arflags[REG_flags] = ctx.commitarf[REG_flags];
  }

  void core_to_external_state(Context& ctx) {
    foreach (i, ARCHREG_COUNT) {
      ctx.commitarf[i] = arf[i];
    }
  }

  bool handle_barrier() {
    core_to_external_state(ctx);

    int assistid = ctx.commitarf[REG_rip];
    assist_func_t assist = (assist_func_t)(Waddr)assistid_to_func[assistid];

    if (logable(5)) {
      logfile << "[vcpu ", ctx.vcpuid, "] Barrier (#", assistid, " -> ", (void*)assist, " ", assist_name(assist), " called from ",
        (RIPVirtPhys(ctx.commitarf[REG_selfrip]).update(ctx)), "; return to ", (void*)(Waddr)ctx.commitarf[REG_nextrip],
        ") at ", parent->cycles, " cycles, ";
      // logfile << sc_time_stamp(), " commits", superstl::endl, superstl::flush;
    }

    if (logable(6)) logfile << "Calling assist function at ", (void*)assist, "...", superstl::endl, superstl::flush; 

    update_assist_stats(assist);
    if (logable(6)) {
      logfile << "Before assist:", superstl::endl, ctx, superstl::endl;
#ifdef PTLSIM_HYPERVISOR
      logfile << sshinfo, superstl::endl;
#endif
    }

    assist(ctx);

    if (logable(6)) {
      logfile << "Done with assist", superstl::endl;
      logfile << "New state:", superstl::endl;
      logfile << ctx;
#ifdef PTLSIM_HYPERVISOR
      logfile << sshinfo;
#endif
    }

    reset_fetch(ctx.commitarf[REG_rip]);
    external_to_core_state(ctx);
#ifndef PTLSIM_HYPERVISOR
    if (requested_switch_to_native) {
      logfile << "PTL call requested switch to native mode at rip ", (void*)(Waddr)ctx.commitarf[REG_rip], superstl::endl;
      return false;
    }
#endif
    return true;
  }

  bool handle_exception() {
    core_to_external_state(ctx);

#ifdef PTLSIM_HYPERVISOR
    if (logable(4)) {
      logfile << "PTL Exception ", exception_name(ctx.exception), " called from rip ", (void*)(Waddr)ctx.commitarf[REG_rip], 
        " at ", parent->cycles, " cycles, ";
      //logfile <<sc_time_stamp(), " commits", superstl::endl, superstl::flush;
    }

    //
    // Map PTL internal hardware exceptions to their x86 equivalents,
    // depending on the context. The error_code field should already
    // be filled out.
    //
    switch (ctx.exception) {
    case EXCEPTION_PageFaultOnRead:
    case EXCEPTION_PageFaultOnWrite:
    case EXCEPTION_PageFaultOnExec:
      ctx.x86_exception = EXCEPTION_x86_page_fault; break;
    case EXCEPTION_FloatingPointNotAvailable:
      ctx.x86_exception = EXCEPTION_x86_fpu_not_avail; break;
    case EXCEPTION_FloatingPoint:
      ctx.x86_exception = EXCEPTION_x86_fpu; break;
    default:
      logfile << "Unsupported internal exception type ", exception_name(ctx.exception), superstl::endl, superstl::flush;
      assert(false);
    }

    if unlikely ((ctx.x86_exception == EXCEPTION_x86_page_fault) && (ctx.cr2 == 0xffffffff00000018)) eventlog.print(logfile);

    if (logable(4)) {
      logfile << ctx;
      logfile << sshinfo;
    }

    ctx.propagate_x86_exception(ctx.x86_exception, ctx.error_code, ctx.cr2);

    external_to_core_state(ctx);

    return true;
#else
    if (logable(6)) 
      {
	logfile << "Exception (", exception_name(ctx.exception), " called from ", (void*)(Waddr)ctx.commitarf[REG_rip], 
	  ") at ", parent->cycles, " cycles, ";
	//logfile << sc_time_stamp();
	logfile << " commits", superstl::endl, superstl::flush;
      }

    stringbuf sb;
    logfile << exception_name(ctx.exception), " detected at fault rip ", (void*)(Waddr)ctx.commitarf[REG_rip], " @ ";
    //logfile << sc_time_stamp();
    logfile << " commits : genuine user exception (",
      exception_name(ctx.exception), "); aborting", superstl::endl;
    logfile << ctx, superstl::endl;
    logfile << superstl::flush;

    logfile << "Aborting...", superstl::endl, superstl::flush;
    superstl::cerr << "Aborting...", superstl::endl, superstl::flush;

    assert(false);
    return false;
#endif
  }

#ifdef PTLSIM_HYPERVISOR
  bool handle_interrupt() {
    core_to_external_state(ctx);

    if (logable(6)) {
      logfile << "Interrupts pending at ", parent->cycles, " cycles, ";
      //logfile << sc_time_stamp();
      logfile << " commits", superstl::endl, superstl::flush;
      logfile << "Context at interrupt:", superstl::endl;
      logfile << ctx;
      logfile << sshinfo;
      logfile.superstl::flush();
    }

    ctx.event_upcall();

    if (logable(6)) {
      logfile << "After interrupt redirect:", superstl::endl;
      logfile << ctx;
      logfile << sshinfo;
      logfile.flush();
    }

    reset_fetch(ctx.commitarf[REG_rip]);
    external_to_core_state(ctx);

    return true;
  }
#endif


  int simulator_cmd(int cmd, int a0, int a1, u32_t *rp)
  {

    printf("simulator cmd %i ignored\n", cmd);
    return 0;
  }

  bool eval_bdoor_read(u64_t memaddr, u64_t &rdata)
  {
    sc_time &delay = parent->lt_d_delay;
#include "backdoor_reads.C"
  }

  bool eval_bdoor_write(u64_t memaddr, u64_t wdata)
  {
#include "backdoor_writes.C"

  }


  BasicBlock* fetch_or_translate_basic_block(Waddr rip) {
    //assert(rip < 0x4000);
    RIPVirtPhys rvp(rip);

    rvp.update(ctx);

    BasicBlock* bb = bbcache(rvp);

    if likely (bb) {
      current_basic_block = bb;
    } else {
      current_basic_block = bbcache.translate(ctx, rvp);
      assert(current_basic_block);
      synth_uops_for_bb(*current_basic_block);

      if unlikely (config.event_log_enabled) {
        TransOp dummyuop; setzero(dummyuop);
        SequentialCoreEvent* event = eventlog.add(parent->cycles, EVENT_TRANSLATE, ctx.vcpuid, dummyuop, rip, 0, 0, sc_time_stamp());
        event->bb.rvp = rvp;
        event->bb.bb = current_basic_block;
        event->bb.bbcount = current_basic_block->count;
      }

      bbcache_inserts++;
    }

    current_basic_block->use(parent->cycles);
    return current_basic_block;
  }

  //
  // Execute one basic block sequentially
  //

  int execute(BasicBlock* bb, W64 insnlimit) {
    arf[REG_rip] = bb->rip;



    parent->corestats.basicblocks += 1; // incremented in two places?
    //
    // Fetch
    //
    int traceflags = 0;
    if (parent->busaccess.traceregions)
    {
	u64_t pc = arf[REG_rip];
	//assert(pc < 0x4000);
        traceflags = parent->busaccess.traceregions->check(pc, TENOS_TRACE_CPU_IDECODE|TENOS_TRACE_CPU_KEY_REGS|TENOS_TRACE_CPU_ALL_REGS|TENOS_TRACE_CPU_IFETCH);
	if (traceflags & TENOS_TRACE_CPU_IFETCH)
        {
	    char line[132];
	    if (argv_backdoor_su) argv_backdoor_su->symbolic_dis64(line, 132, pc);
	    else snprintf(line, 132, " %lX ", pc);
	    //disassemble_insn(rdata);
	    printf("%s: FETCH  %s (RIP: 0x%lx)\n",   parent->name(), line, pc);    
	    printf("%s: AX=" PFLX64 "  BX=" PFLX64 "  CX=" PFLX64 "  DX=" PFLX64 "\n", parent->name(), arf[REG_rax], arf[REG_rbx], arf[REG_rcx], arf[REG_rdx]);
	    printf("%s: SP=" PFLX64 "  BP=" PFLX64 "  SI=" PFLX64 "  DI=" PFLX64 "\n", parent->name(), arf[REG_rsp], arf[REG_rbp], arf[REG_rsi], arf[REG_rdi]);
        }
    }


    //std::cout <<    ctx.vcpuid << " Fetch block " << bb->rip << std::endl;
    bool barrier = 0;

    if (logable(5)) logfile << "[vcpu ", ctx.vcpuid, "] Sequentially executing basic block ", bb->rip, " (", bb->count, " uops), insn limit ", insnlimit, ", stack pointer = ", hexstring(arf[REG_rsp], 64), superstl::endl;

    if unlikely (config.event_log_enabled) {
      TransOp dummyuop; setzero(dummyuop);
      SequentialCoreEvent* event = eventlog.add(parent->cycles, EVENT_EXECUTE_BB, ctx.vcpuid, dummyuop, bb->rip, 0, 0, sc_time_stamp());
      event->bb.rvp = bb->rip;
      event->bb.bb = bb;
      event->bb.bbcount = bb->count;
    }

    if unlikely (!bb->synthops) synth_uops_for_bb(*bb);
    bb->hitcount++;

    TransOpBuffer unaligned_ldst_buf;
    unaligned_ldst_buf.index = -1;

    int uopindex = 0;
    int current_uop_in_macro_op = 0;

    int user_insns = 0;

    parent->corestats.basicblocks++;

    RIPVirtPhys rvp(arf[REG_rip]);

    assert(bb->rip.rip == arf[REG_rip]);

    // See comment below about idempotent updates
    W64 saved_flags = 0;

    while ((uopindex < bb->count)/* & (user_insns < insnlimit)*/) {
      TransOp uop;
      uopimpl_func_t synthop = null;

      if (parent->m_qk.need_sync()) parent->m_qk.sync();
      sc_time ins_start = parent->m_qk.get_current_time();
      parent->lt_i_delay = SC_ZERO_TIME;
      parent->lt_d_delay = SC_ZERO_TIME;

      // MP - 31May13 = check if instruction cache has a line
      // that contains the instruction that we are about to execute
      W64 instr_addr = arf[REG_rip];

      // instruction granularity is 8 bytes
      const int ICACHE_FETCH_GRANULARITY = 8;
      W64 req_icache_block = floor(instr_addr, ICACHE_FETCH_GRANULARITY);
      if(current_icache_block != req_icache_block) {
	// query the instruction cache
	u64_t data;
	parent->busaccess.instr_fetch64
	  (req_icache_block, 
	   data, 
	   NULL, 
	   parent->lt_i_delay);

	// MP - 31May13 = here if we want we can check if data
	// is the same as one kept in the TraceDecoder buffer
	// that was retrieved by reading image
	current_icache_block = req_icache_block;
      }

      if unlikely (arf[REG_rip] == config.stop_at_rip) {
        return SEQEXEC_EARLY_EXIT;
      }

      if unlikely (arf[REG_rip] == config.start_log_at_rip) {
        config.start_log_at_iteration = 0;
        logenable = 1;
      }

      if likely (!unaligned_ldst_buf.get(uop, synthop)) {
        uop = bb->transops[uopindex];
        synthop = bb->synthops[uopindex];
      }

      assert(uopindex < bb->count);

      if unlikely (uop.unaligned) {
        // if (logable(6)) logfile << padstring("", 20), " fetch  rip 0x", (void*)(Waddr)arf[REG_rip], ": split unaligned load or store ", uop, superstl::endl;
        split_unaligned(uop, unaligned_ldst_buf);
        assert(unaligned_ldst_buf.get(uop, synthop));
      }
  
      if likely (uop.som) {
        current_uop_in_macro_op = 0;
        bytes_in_current_insn = uop.bytes;
        fetch_user_insns_fetched++;
        // Update the span of bytes to watch for SMC:
        rvp.update(ctx, uop.bytes);
        //
        // Save the flags at the start of this x86 insn in
        // case an ALU uop inside the macro-op updates the
        // flags before all exceptions (i.e. from stores)
        // can be detected. All other registers are updated
        // idempotently.
        //
        saved_flags = arf[REG_flags];
      }

      //
      // Check for self modifying code (SMC) by checking if any previous
      // instruction has dirtied the page(s) on which the current instruction
      // resides. The SMC check is done first since it's perfectly legal for a
      // store to overwrite its own instruction bytes, but this update only
      // becomes visible after the store has committed.
      //
      /*if unlikely (smc_isdirty(rvp.mfnlo) | (smc_isdirty(rvp.mfnhi))) {
        logfile << "Self-modifying code at rip ", rvp, " detected: mfn was dirty (invalidate and retry)", superstl::endl;
        bbcache.invalidate_page(rvp.mfnlo, INVALIDATE_REASON_SMC);
        if (rvp.mfnlo != rvp.mfnhi) bbcache.invalidate_page(rvp.mfnhi, INVALIDATE_REASON_SMC);
        return SEQEXEC_SMC;
	}*/

      fetch_uops_fetched++;

      //
      // Issue
      //
      IssueState state;
      state.reg.rdflags = 0;
      ctx.exception = 0;

      IssueInput input;
      W64 radata = arf[archreg_remap_table[uop.ra]];
      W64 rbdata = (uop.rb == REG_imm) ? uop.rbimm : arf[archreg_remap_table[uop.rb]];
      W64 rcdata = (uop.rc == REG_imm) ? uop.rcimm : arf[archreg_remap_table[uop.rc]];

      W16 raflags = arflags[archreg_remap_table[uop.ra]];
      W16 rbflags = arflags[archreg_remap_table[uop.rb]];
      W16 rcflags = arflags[archreg_remap_table[uop.rc]];

      bool ld = isload(uop.opcode);
      bool st = isstore(uop.opcode);
      bool br = isbranch(uop.opcode);
      bool pause = uop.opcode == OP_pause;

      SFR sfr;
      
      bool refetch = 0;

      PTEUpdate pteupdate = 0;
      Waddr origvirt = 0;
      PageFaultErrorCode pfec = 0;

      bool force_fpu_not_avail_fault = 0;

      Waddr rip = arf[REG_rip];
      if (traceflags && TENOS_TRACE_CPU_IDECODE)
          printf("%s:%i:  uopcode=0x%03X %s\n", parent->name(), uopindex, uop.opcode, opinfo[uop.opcode].name);
      if ((traceflags & TENOS_TRACE_CPU_ALL_REGS)  
	  //||(traceflags & TENOS_TRACE_CPU_KEY_REGS)   // TODO - want to print fewer in key mode!
	  )
        print_state(superstl::cout);  // This does not interleave with stdout printfs correctly! TODO.


#ifdef PTLSIM_HYPERVISOR
      if unlikely (uop.is_sse|uop.is_x87) {
        force_fpu_not_avail_fault = ctx.cr0.ts | (uop.is_x87 & ctx.cr0.em);
      }
#endif
      if unlikely (force_fpu_not_avail_fault) {
        if unlikely (config.event_log_enabled) {
          SequentialCoreEvent* event = eventlog.add(parent->cycles, EVENT_ISSUE, ctx.vcpuid, uop, rip, current_uop_in_macro_op, current_uuid, sc_time_stamp());
          IssueState state;
          state.reg.rdflags = FLAG_INV;
          state.reg.rddata = EXCEPTION_FloatingPointNotAvailable;
        }

        ctx.exception = EXCEPTION_FloatingPointNotAvailable;
        ctx.error_code = 0;
        arf[REG_flags] = saved_flags;
        return SEQEXEC_EXCEPTION;
      } else if unlikely (ld|st) {
        int status;


        if likely (ld) {
            W64 load_data = 0;

	    status = issueload(uop, sfr, origvirt, radata, rbdata, rcdata, pteupdate, load_data);

	    if (uop.locked && traceflags && TENOS_TRACE_CPU_EXECUTE)
                printf("tid=%d-%s: ISSUING linked load 0x%llx\n",
                       sc_get_current_process_b()->proc_id, parent->name(), origvirt);


	    if(uop.locked) {
	      // load linked - save the state of registers at this point
	      // and also the index of load so if store conditional fails
	      // we are going to repeat instructions from that index
              save_state_after_load_linked(uopindex, origvirt, load_data);
	    }
        } else if unlikely (uop.opcode == OP_mf) {
          // Memory fences are NOPs on the in-order core:
          status = ISSUE_COMPLETED;
          sfr.data = 0;
 
        } else {
          assert(st);
          status = issuestore(uop, sfr, origvirt, radata, rbdata, rcdata, pteupdate);

          /*std::cout << sc_time_stamp() << "-"; */
          if (traceflags && TENOS_TRACE_CPU_EXECUTE)
              printf("tid=%d-%s: ISSUED store to addr 0x%llx; locked %d; 0x%llx, 0x%llx, 0x%llx\n",
                     sc_get_current_process_b()->proc_id, parent->name(), origvirt, uop.locked, radata, rbdata, rcdata);
        }

        state.reg.rddata = sfr.data;
        state.reg.rdflags = 0;

        if (status == ISSUE_EXCEPTION) {
          ctx.exception = LO32(state.reg.rddata);
          ctx.error_code = HI32(state.reg.rddata); // page fault error code
#ifdef PTLSIM_HYPERVISOR
          ctx.cr2 = origvirt;
#endif
          arf[REG_flags] = saved_flags;
          return SEQEXEC_EXCEPTION;
        } else if (status == ISSUE_REFETCH) {
          if unlikely (config.event_log_enabled) {
            SequentialCoreEvent* event = eventlog.add(parent->cycles, EVENT_ALIGNMENT_FIXUP, ctx.vcpuid, uop, rip, current_uop_in_macro_op, current_uuid, sc_time_stamp());
            event->alignfixup.uopindex = uopindex;
          }
          bb->transops[uopindex].unaligned = 1;
          continue;
        }

      } else if unlikely (br) {
        state.brreg.riptaken = uop.riptaken;
        state.brreg.ripseq = uop.ripseq;
        assert((void*)synthop);
        synthop(state, radata, rbdata, rcdata, raflags, rbflags, rcflags); 

        if unlikely (config.event_log_enabled) {
          SequentialCoreEvent* event = eventlog.add(parent->cycles, EVENT_BRANCH, ctx.vcpuid, uop, rip, current_uop_in_macro_op, current_uuid, sc_time_stamp());
          event->issue.state = state;
        }

        bb->predcount += (uop.opcode == OP_jmp) ? (state.reg.rddata == bb->lasttarget) : (state.reg.rddata == uop.riptaken);
        bb->lasttarget = state.reg.rddata;
      } else {
        assert((void*)synthop);
        synthop(state, radata, rbdata, rcdata, raflags, rbflags, rcflags);
        if unlikely (state.reg.rdflags & FLAG_INV) ctx.exception = LO32(state.reg.rddata);

        if unlikely (config.event_log_enabled) {
          SequentialCoreEvent* event = eventlog.add(parent->cycles, EVENT_ISSUE, ctx.vcpuid, uop, rip, current_uop_in_macro_op, current_uuid, sc_time_stamp());
          event->issue.state = state;
        }

        if unlikely (ctx.exception) {
          if (isclass(uop.opcode, OPCLASS_CHECK) & (ctx.exception == EXCEPTION_SkipBlock)) {
            W64 chk_recovery_rip = arf[REG_rip] + bytes_in_current_insn;
            if unlikely (config.event_log_enabled) {
              SequentialCoreEvent* event = eventlog.add(parent->cycles, EVENT_SKIPBLOCK, ctx.vcpuid, uop, rip, current_uop_in_macro_op, current_uuid, sc_time_stamp());
              event->skipblock.bytes_in_current_insn = bytes_in_current_insn;
              event->skipblock.chk_recovery_rip = chk_recovery_rip;
            }
            current_uuid++;
            arf[REG_rip] = chk_recovery_rip;

	    seq_total_user_insns_committed++;
            user_insns++;
            return SEQEXEC_OK;
          } else {
            arf[REG_flags] = saved_flags;
            return SEQEXEC_EXCEPTION;
          }
        }
      }

      //
      // Commit
      //

      assert(!ctx.exception);

      bool failed_sc = false;

      if unlikely (uop.opcode == OP_st) {
	if (sfr.bytemask) {
          if unlikely (cmtrec) {
	      transactmem.store(sfr.physaddr << 3, sfr.data, sfr.bytemask);
	      // MP 29May13 - we should never reach this?
	      ptlsim_assert(0);
          } else {	 
            //storemask(sfr.physaddr << 3, sfr.data, sfr.bytemask);
	    // MP 29May13 - replacing store mask with our cache infrastructure
	    Waddr addr = floor(signext64((sfr.physaddr << 3), 48), 8);
	    if (uop.locked) {
	      // assert sfr.bytemask TODO
              
              // MP 12Aug13 - if we are under lock doing store conditional but the
              // content of memory at the address to be written is the same we
              // do not write to memory, otherwise CMPXCHG semantics breaks
              assert(ll_sc_address == addr);
              int rc = 1;
              if(sfr.data != ll_sc_data) {
		  if (traceflags && TENOS_TRACE_CPU_EXECUTE) {
		    printf("tid=%d-%s: DOING store conditional to addr 0x%llx with data 0x%llx; result = %d\n", 
			   sc_get_current_process_b()->proc_id, parent->name(), addr, sfr.data, rc);
                  }

                  rc = parent->busaccess.cpu_mem_store_conditional(addr, sfr.data, 64, parent->lt_d_delay);
              }

	      if(rc)
		reset_state_after_succesfull_store_conditional(sfr.data == ll_sc_data);
	      else {                  
                  if(traceflags && TENOS_TRACE_CPU_EXECUTE) 
                      printf("tid=%d-%s: FAILED store conditional to addr 0x%llx\n", sc_get_current_process_b()->proc_id, parent->name(), addr);
		uopindex = load_state_after_failed_store_conditional();
		failed_sc = true;
	      }
	    }
	    else {
	       /*std::cout << sc_time_stamp() << "-"; 
                printf("tid=%d-%s: DOING memory write to addr 0x%lx\n", 
                sc_get_current_process_b()->proc_id, parent->name(), addr);*/
                parent->busaccess.mem_write(addr, sfr.bytemask, sfr.data, parent->lt_d_delay);
            }
          }

          Waddr mfn = (sfr.physaddr << 3) >> 12;
	  // MP 29May13 - we do not have self modified code so we do not
	  // need to set page to dirty
          //smc_setdirty(mfn); // why is this being passed zero?
	}
      } else if likely (uop.rd != REG_zero) {
        arf[uop.rd] = state.reg.rddata;
        arflags[uop.rd] = state.reg.rdflags;
        
        if (!uop.nouserflags) {
          W64 flagmask = setflags_to_x86_flags[uop.setflags];
          arf[REG_flags] = (arf[REG_flags] & ~flagmask) | (state.reg.rdflags & flagmask);
          arflags[REG_flags] = arf[REG_flags];
        }
     }


      if unlikely (pteupdate) {
        if unlikely (config.event_log_enabled) {
          SequentialCoreEvent* event = eventlog.add(parent->cycles, EVENT_PTE_UPDATE, ctx.vcpuid, uop, rip, current_uop_in_macro_op, current_uuid, sc_time_stamp());
          event->pteupdate.virtaddr = origvirt;
          event->pteupdate.pteupdate = pteupdate;
        }

        if unlikely (cmtrec) {
          /*
          assert(cmtrec->pte_update_count < lengthof(cmtrec->pte_update_list));
          cmtrec->pte_update_list[cmtrec->pte_update_count] = pteupdate;
          cmtrec->pte_update_virt[cmtrec->pte_update_count] = origvirt;
          cmtrec->pte_update_count++;
          */
        } else {
          ctx.update_pte_acc_dirty(origvirt, pteupdate);
        }
      }

      barrier = isclass(uop.opcode, OPCLASS_BARRIER);

      if unlikely ((arf[REG_rip] == config.log_backwards_from_trigger_rip) && (config.event_log_enabled)) {
        logfile << "Hit trigger rip ", (void*)(Waddr)config.log_backwards_from_trigger_rip, "; printing event ring buffer:", superstl::endl, superstl::flush;
        eventlog.print(logfile);
        logfile << "End of triggered event dump", superstl::endl;
      }

      if likely (uop.eom) {
        arf[REG_rip] = (uop.rd == REG_rip) ? state.reg.rddata : (arf[REG_rip] + bytes_in_current_insn);
        // Do not commit transactional memory: that's up to the caller:
        // if unlikely (cmtrec) transactmem.commit();
      }

      if unlikely (barrier) {
        if unlikely (config.event_log_enabled) {
          int assistid = arf[REG_rip];
          assist_func_t assist = (assist_func_t)(Waddr)assistid_to_func[assistid];
          TransOp dummyuop; setzero(dummyuop);
          SequentialCoreEvent* event = eventlog.add(parent->cycles, EVENT_ASSIST, ctx.vcpuid, dummyuop, rip, current_uop_in_macro_op, current_uuid, sc_time_stamp());
          event->assist.id = assistid;
          event->assist.rip = arf[REG_selfrip];
          event->assist.ptl_pip = (W64)assist;
          event->assist.next_rip = arf[REG_nextrip];
          event->assist.real_target_rip = arf[REG_rip];
        }
      }

      seq_total_user_insns_committed += uop.eom;
      user_insns += uop.eom;
      parent->corestats.insns += uop.eom;
      parent->corestats.uops++;

      current_uuid++;
      // Don't advance on cracked loads/stores:
      if (!failed_sc) uopindex += unaligned_ldst_buf.empty();
      current_uop_in_macro_op++;

      if (logable(9)) {
        if unlikely (br) {
          core_to_external_state(ctx);
          logfile << "Core State after branch:", superstl::endl;
          logfile << ctx;
        }
      }

      sc_time d_end = parent->lt_d_delay + sc_time_stamp();
      sc_time i_end = parent->lt_i_delay + sc_time_stamp();
      sc_time ins_end = ins_start;

      if unlikely (pause || failed_sc)
	{
	  parent->corestats.pauses += 1;
          if(pause)
              failureNum++;
	  // MP - Need to find out what is the optimal multiplier???
          // random number between 0 and 2^failureNo - 1
          if(failureNum > 24)
              failureNum = 1;
          int k = rand() % ((int)(pow(2, failureNum) - 1));
          //fprintf(stdout, "failure for core %d, factor is %d, failure is %d\n", parent->procID, k, failureNum);
          ins_end += k*1000*parent->m_core_period; // Hard-coded clocks pause.
	}
      else {
          ins_end += parent->m_effective_instruction_period;
      }


#define TMAX(X, Y) ((X)>(Y)?(X):(Y))
      sc_time mx = TMAX(ins_end, TMAX(d_end, i_end));
      //std::cout << "Advancing time to " << mx << " from " <<  sc_time_stamp() << "\n";
      parent->m_qk.set(mx-sc_time_stamp());
      //std::cout << "Time now " << sc_time_stamp() << "\n";
    }

    // after the end of the basic block we should never have just
    // load-linked that was not followed by store-conditional
    assert(!load_linked);
    assert(load_linked_uopindex == -1);

    if (barrier) {
      return SEQEXEC_BARRIER;
    }

    return (insnlimit < bb->user_insn_count) ? SEQEXEC_EARLY_EXIT : SEQEXEC_OK;
  }

  int execute() {
    Waddr rip = arf[REG_rip];
    if (rip == 0x0) {// MP 29May13 - special instruction to stop simulation 
      core_to_external_state(ctx);
      if (logable(9)) {
        logfile << "Core State at end:", superstl::endl;
        logfile << ctx;
	logfile << superstl::endl, superstl::flush;
      }
      return 2;
    }

    current_basic_block = fetch_or_translate_basic_block(rip);

    bool exiting = 0;

    int result = execute(current_basic_block, (uint64_t)-1);

    switch (result) {
    case SEQEXEC_OK:
    case SEQEXEC_SMC:
    case SEQEXEC_SKIPBLOCK:
      // no action required
      break;
    case SEQEXEC_EARLY_EXIT:
      exiting = 1;
      break;
    case SEQEXEC_EXCEPTION:
    case SEQEXEC_INVALIDRIP:
      ctseq.stop();
      exiting = (!handle_exception());
      ctseq.start();
      break;
    case SEQEXEC_BARRIER:
      ctseq.stop();
      exiting = (!handle_barrier());
      ctseq.start();
      break;
#ifdef PTLSIM_HYPERVISOR
    case SEQEXEC_INTERRUPT:
      ctseq.stop();
      handle_interrupt();
      ctseq.start();
      break;
#endif
    default:
      assert(false);
    }

    return exiting;
  }

#ifdef PTLSIM_HYPERVISOR
  int execute_in_place(W64 bbcount = limits<W64>::max, W64s insncount = limits<W64s>::max) {
    external_to_core_state(ctx);
    int result = SEQEXEC_OK;

    W64 user_insns_at_start = seq_total_user_insns_committed;

    if unlikely (config.event_log_enabled && (!eventlog.start)) {
      eventlog.init(config.event_log_ring_buffer_size);
      eventlog.logfile = &logfile;
    }

    foreach (i, bbcount) {
      Waddr rip = arf[REG_rip];
      
      TraceDecoder trans(ctx, rip);
      trans.split_basic_block_at_locks_and_fences = 1;
      trans.split_invalid_basic_blocks = 1;
      
      byte byte_buffer[MAX_BB_BYTES];
      int valid_byte_count = trans.fillbuf(ctx, byte_buffer, lengthof(byte_buffer));
      assert(valid_byte_count <= lengthof(byte_buffer));
      
      for (;;) { if (!trans.translate()) break; }
      
      if likely (trans.ptelo.p) smc_cleardirty(trans.ptelo.mfn);
      if likely (trans.ptehi.p) smc_cleardirty(trans.ptehi.mfn);
      
      W64 user_insns_at_start = seq_total_user_insns_committed;
      result = execute(&trans.bb, insncount);
      W64 delta_insns = seq_total_user_insns_committed - user_insns_at_start;
      insncount -= delta_insns;
      
      if (trans.bb.synthops) delete[] trans.bb.synthops;
      
      if unlikely (config.event_log_enabled) {
        if unlikely (config.flush_event_log_every_cycle) {
          eventlog.flush(true);
        }
      }

      if unlikely (result == SEQEXEC_SMC) continue;
      if unlikely (result != SEQEXEC_OK) break;
      if unlikely (insncount <= 0) break;
    }

    if likely (cmtrec) {
      transactmem.update(*cmtrec);
      cmtrec->exit_reason = result;
    }

    core_to_external_state(ctx);

    return result;
  }

  int execute_transactional(W64 bbcount = limits<W64>::max, W64s insncount = limits<W64s>::max) {
    external_to_core_state(ctx);
    int result = SEQEXEC_OK;

    W64 user_insns_at_start = seq_total_user_insns_committed;

    foreach (i, bbcount) {
      Waddr rip = arf[REG_rip];

      BasicBlock* bb = fetch_or_translate_basic_block(rip);
      assert(bb);
      
      result = execute(bb, insncount);
      insncount -= bb->user_insn_count;

      if unlikely (result != SEQEXEC_OK) break;
      if unlikely (insncount <= 0) break;
    }

    if likely (cmtrec) {
      transactmem.update(*cmtrec);
      cmtrec->exit_reason = result;
    }

    core_to_external_state(ctx);

    return result;
  }
#endif
};

struct SequentialMachine: public PTLsimMachine {
  SequentialCore* cores[MAX_CONTEXTS];
  bool init_done;

  SequentialMachine(const char* name) {
    // Add to the list of available core types
    addmachine(name, this);
    init_done = 0;
  }

  //
  // Construct all the structures necessary to configure
  // the cores. This function is only called once, after
  // all other PTLsim subsystems are brought up.
  //
  virtual bool init(PTLsimConfig& config) {
    assert(0); // we should not be calling this - MP 15Jun13
    /*if (init_done) return true;

    foreach (i, contextcount) {
      cores[i] = new SequentialCore(contextof(i));
      //
      // Note: in a real cycle accurate model, config may
      // specify various ways of slicing contextcount up
      // into threads, cores and sockets; the appropriate
      // interconnect and cache hierarchy parameters may
      // be specified here.
      //
    }

    init_done = 1;
    return true;*/
  }



};


#ifdef PTLSIM_HYPERVISOR
int execute_sequential(Context& ctx, CommitRecord* cmtrec, W64 bbcount, W64 insncount) {
  if (config.flush_event_log_every_cycle) {
    assert(config.event_log_enabled);
    logfile << "execute_sequential(", insncount, " insns): clear event log and flush every cycle", superstl::endl;
    eventlog.clear();
  }
  if unlikely (cmtrec) {
    cmtrec->reset();
    *(Context*)cmtrec = ctx;
    SequentialCore core(*cmtrec, cmtrec);
    return core.execute_in_place(bbcount, insncount);
  } else {
    SequentialCore core(ctx);
    return core.execute_in_place(bbcount, insncount);
  }
}
#endif

superstl::ostream& CommitRecord::print(superstl::ostream& os) const {
  os << "CommitRecord: ", store_count, " stores, ", pte_update_count, " PTE updates", superstl::endl;
  foreach (i, store_count) {
    os << "  Store ", intstring(i, 3), ": ", stores[i], superstl::endl;
  }
  return os;
}
