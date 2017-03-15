// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) M Puzovic & DJ Greaves - University of Cambridge Computer Laboratory 2011.
// $Id: $


#include "systemc.h"
#include "secondary_cache_with_directory.h"
#include <iostream>
#include <algorithm>

#if 1
#define CDTRC(X) cout << sc_time_stamp() << "-"; \
  printf("tid=%d-", sc_get_current_process_b()->proc_id); X
#else
#define CDTRC(X)
#endif

using namespace std;
using namespace sc_core;

// Directory constructor
secondary_cache_with_directory::directory_ty::directory_ty(sc_module_name name_, 
							   int cores, 
							   int ways,
							   int rows,
							   int cols,
							   int entry_size_bits):
  cores(cores),
  rows(rows),
  cols(cols),
  entry_size_bits(entry_size_bits)
{
  printf("%s: created directory, rows = %d, %llu bytes\n", 
	 name(), rows, entry_size_bits * rows * cols * cores * ways / 8LLU);

  Data = (panel**) malloc(sizeof(panel*)*rows);
  for(int i = 0; i < rows; i++) {
    Data[i] = (panel*) malloc(sizeof(panel)*cols);
    for(int j = 0; j < cols; j++) {
      Data[i][j] = (u64_t**) malloc(sizeof(u64_t*)*cores);
      for(int k = 0; k < cores; k++) {
	Data[i][j][k] = (u64_t*) malloc(sizeof(u64_t)*ways);
	  for(int l = 0; l < ways; l++)
	    Data[i][j][k][l] = -1;
      }
    }
  }
}

secondary_cache_with_directory::directory_ty::panel 
secondary_cache_with_directory::directory_ty::read(int row, int col) {
  assert(row >= 0 && row < rows);
  assert(col >= 0 && col < cols);
  return Data[row][col];
}

void secondary_cache_with_directory::build_instruction_directory(
  int primary_instr_bytes,
  int primary_instr_linesize,
  int primary_instr_ways,
  int core_num) {

  // initialize everything to zero
  idir_geom.mask1 = 0;
  idir_geom.lshift2 = 0;
  idir_geom.mask2 = 0;
  idir_geom.rshift3 = 0;
  idir_geom.lshift3 = 0;
  idir_geom.mask3 = 0;
  idir_geom.rshift4 = 0;
  idir_geom.lshift4 = 0;
  
  // number of entries
  int ne = primary_instr_bytes / primary_instr_ways / primary_instr_linesize;
  // size of directory
  int sd = (ne / geom.banks) * core_num * primary_instr_ways;
  
  int entry_bits = bound_log2(sd);
  
  // find out mask for cpu id
  int cpu_bits = bound_log2(core_num);
  idir_geom.mask1 = core_num - 1;
  if(cpu_bits - entry_bits >= 0) {
    idir_geom.mask1 = pow_2(entry_bits) - 1;
    return;
  }
  else
    idir_geom.mask1 = core_num - 1;
  
  entry_bits -= cpu_bits;

  // find out mask for primary cache way
  idir_geom.lshift2 = cpu_bits;
  idir_geom.mask2;
  int way_bits = bound_log2(primary_instr_ways);
  if(way_bits - entry_bits >= 0) {
    idir_geom.mask2 = pow_2(entry_bits) - 1;
    return;
  }
  else
    idir_geom.mask2 = primary_instr_ways - 1;
    
  entry_bits -= way_bits;
  
  // find out mask for line
  int ls_bits = bound_log2(geom.linesize);
  int pr_ls_bits = bound_log2(primary_instr_linesize);
  assert(pr_ls_bits <= ls_bits);
  int line_bits = ls_bits - pr_ls_bits;
  idir_geom.lshift3 = idir_geom.lshift2 + way_bits;
  idir_geom.rshift3 = pr_ls_bits;
  idir_geom.mask3 = 0;
  if(line_bits != 0) 
    if(line_bits - entry_bits >= 0) {
      idir_geom.mask3 = pow_2(entry_bits) - 1;
      return;
    }
    else
      idir_geom.mask3 = pow_2(line_bits) - 1;
  entry_bits -= line_bits;
  
  // find out mask for the index part of secondary cache
  idir_geom.rshift4 = ls_bits + bound_log2(geom.banks);
  idir_geom.lshift4 = idir_geom.lshift3 + line_bits;
  idir_geom.mask4 = pow_2(entry_bits) - 1;
  
  // Print geometry
  cout << "idir_geom.mask1=" << idir_geom.mask1 << endl;
  cout << "idir_geom.lshift2=" << idir_geom.lshift2 << endl;  
  cout << "idir_geom.mask2=" << idir_geom.mask2 << endl;
  cout << "idir_geom.rshift3=" << idir_geom.rshift3 << endl;  
  cout << "idir_geom.lshift3=" << idir_geom.lshift3 << endl;  
  cout << "idir_geom.mask3=" << idir_geom.mask3 << endl;
  cout << "idir_geom.rshift4=" << idir_geom.rshift4 << endl;  
  cout << "idir_geom.lshift4=" << idir_geom.lshift4 << endl;  
  cout << "idir_geom.mask4=" << idir_geom.mask4 << endl;
}


// Constructor
secondary_cache_with_directory::secondary_cache_with_directory(
  sc_module_name mname,
  int idx,
  int bytes,
  int linesize,
  int ways,
  int secondary_width,
  int secondary_bl,
  int banks,
  int primary_bytes,
  int primary_linesize,
  int primary_ways,
  int primary_instr_bytes,
  int primary_instr_linesize,
  int primary_instr_ways,
  int core_num
) : 
  sc_module(mname),
  targ_socket("targ_socket"),
  inita_socket("inita_socket"),
  ml("secondary_cache_lock")
#ifdef TLM_POWER3
  , pw_module() 
#if PW_TLM_PAYLOAD > 0
  , primary_bus_tracker(this)
  , secondary_bus_tracker(this)
  , cache_to_processor_xbus(this)
#endif
#endif
{
  POWER3(m_running_op_energy = PW_ZERO_ENERGY);
  geom.idx = idx;
  geom.ways = ways;
  geom.linesize = linesize;
  geom.bytes = bytes;
  geom.secondary_blocksize_bytes = secondary_width / 8 * secondary_bl;
  geom.secondary_bl = secondary_bl;
  geom.banks = banks;
  // number of cache lines that are in one way
  geom.dmapping = bytes / (geom.banks * geom.ways * geom.linesize);

  geom.word64s_per_line = geom.linesize / 8;

  printf("%s ways=%i, linesize=%i bytes, dmapping=%i (totalsize=%i bytes).\n", 
	 name(), geom.ways, geom.linesize, geom.dmapping, geom.bytes/banks);
  
  // Need to shift by: (3 for bytes per word) + log words_per_line + log banks
  geom.dmap_shift =  3 + bound_log2(geom.word64s_per_line) + bound_log2(geom.banks);

  // Always 3 for 64 bit words when byte addressed
  geom.loffset_shift = 3;

  // caulcate number of bits to be used as index in directory table
  int primary_offset = bound_log2(primary_linesize);
  int primary_index = bound_log2(primary_bytes/(primary_linesize*primary_ways));
  
  geom.directory_panel_row_shift = bound_log2(primary_linesize);
  geom.directory_panel_row_map = 
    (1 << (bound_log2(linesize) - bound_log2(primary_linesize))) - 1;

  geom.directory_panel_col_shift = geom.dmap_shift;
  int col_size_bits = primary_index + primary_offset - geom.dmap_shift;
  geom.directory_panel_col_map = (1 << col_size_bits) - 1;
  
  /*printf("%s directory_panel_row_shift=0x%x, directory_panel_row_map=0x%x, "
	 "directory_panel_col_shift=0x%x, directory_panel_col_map=0x%x\n",
	 name(), geom.directory_panel_row_shift, geom.directory_panel_row_map,
	 geom.directory_panel_col_shift, geom.directory_panel_col_map);*/

  // allocate directory
  int dir_entry_size = 0;
  // in directory we store L2 index 
  dir_entry_size += bound_log2(geom.dmapping);
  // L2 way
  dir_entry_size += bound_log2(geom.ways);
  // and if line is valid or not
  dir_entry_size += 1;
  
  directory = new directory_ty("directory", 
			       8, // number of cores
			       primary_ways,
			       geom.directory_panel_row_map + 1,
			       geom.directory_panel_col_map + 1,
			       dir_entry_size);

  geom.primary_tag_shift = (primary_index + primary_offset);
  geom.primary_ways = primary_ways;

  build_instruction_directory(primary_instr_bytes,
			      primary_instr_linesize,
			      primary_instr_ways,
			      core_num);

  // End of geometry computation

  Cont = (cacheway**) malloc(geom.ways * sizeof(cacheway*));
  for(int w = 0; w < geom.ways; w++)
    Cont[w] = new cacheway(this, w);

  lru = 0;
#ifdef TLM_POWER3
  std_energy_op = pw_energy((double) (0.1 * 64), pw_energy_unit::PW_pJ); 
  // based on: just made up!
  //  POWER3(set_excess_area(pw_length(150 * geom.ways, PW_um), pw_length(150,  PW_um)));
#endif

  // Register callback for incoming interface method calls
  targ_socket.register_b_transport(this,
				   &secondary_cache_with_directory::b_transport);
}

secondary_cache_with_directory::cacheway::cacheway(
  secondary_cache_with_directory *parent,
  int way):
  parent(parent),
  m_way(way)
{
  char name[64];
  snprintf(name, 64, "Data_%d", way);
  Data = new smallram8(name, 
		       8, 
		       parent->geom.dmapping * sizeof(u8_t) * parent->geom.linesize,
		       parent);

  snprintf(name, 64, "Tags_%d", way);
  Tags = new smallram64(name, 64-parent->geom.dmap_shift, parent->geom.dmapping, parent);

  Status = (state_t*) malloc(parent->geom.dmapping * sizeof(state_t));
  for(int i = 0; i < parent->geom.dmapping; i++)
    Status[i] = invalid;
}

bool secondary_cache_with_directory::cacheway::lookup(u64_t addr, 
						      int dmap, 
						      u8_t **clinep,
						      sc_time &max_delay)
{
  POWER3(parent->pw_module_base::record_energy_use(Tags->m_read_energy_op));
  max_delay = max(max_delay, Tags->m_sr_latency);
  if (Status[dmap] != invalid && (*Tags)[dmap] == addr) {
    if(clinep) {
      // not every lookup leads to data being read
      int ops = parent->geom.linesize * 8 / Data->width;
      POWER3(parent->log_energy_use(Data->m_read_energy_op*ops));
      max_delay = max(max_delay, Data->m_sr_latency * ops);
      *clinep = Data->read8p(dmap * parent->geom.linesize); // >> (loffset);
    }
    return true;
  }
  else
    return false;
}

void secondary_cache_with_directory::cacheway::clear(sc_time& delay,
						     int dmap,
						     u64_t addr) {
  if(Status[dmap] != dirty) 
    return;

  PW_TLM_PAYTYPE nt, *trans =&nt; 
  trans->set_data_length(parent->geom.secondary_blocksize_bytes); 
  trans->set_byte_enable_length(0);
  trans->set_byte_enable_ptr(0); // All bytes to be operated on if ptr is null.
  trans->set_streaming_width(parent->geom.linesize);
  trans->set_write();

  // on eviction from L2 we also need to invalidate all potentional
  // lines from L1, by picking up column and doing test for tag on the rest
  POWER3(parent->pw_module_base::record_energy_use(Tags->m_read_energy_op, trans));
  delay += Tags->m_sr_latency;
  int laddr = (*Tags)[dmap]; // line addr
  int dir_col = 
    (laddr >> parent->geom.directory_panel_col_shift) 
    & parent->geom.directory_panel_col_map;
  int nrows = parent->geom.directory_panel_row_map + 1;
  
  if(addr != -1) {
    u64_t tag = parent->get_dir_entry_content(dmap, m_way);
    for(int r = 0; r < nrows; r++) {
      directory_ty::panel dir_entry = parent->directory->read(r, dir_col); 
      for(int i = 0; i < 8; i++) 
	for(int j = 0; j < parent->geom.primary_ways; j++) {
	  if(dir_entry[i][j] == tag) {
	    CDTRC(printf("%s: invalidate processor %d, way %d addr %lx; %lx\n", 
			 parent->name(), i, j, addr, tag));
	    parent->invalidate_line_in_primary(i, j, addr, delay);
	    dir_entry[i][j] = -1;
	  }
	}
    }
  }
  

  // as memory is banked then bits we are using to select bank are not
  // to be used as part of address
  u64_t real_addr = 
    ((*Tags)[dmap] >> parent->geom.dmap_shift) << (bound_log2(parent->geom.linesize));
  CDTRC(printf("%s: real address for 0x%lx is 0x%lx\n", 
	       parent->name(), (*Tags)[dmap], real_addr));
  for(int d=0; d<parent->geom.linesize; d += parent->geom.secondary_blocksize_bytes)
  {
    trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    trans->set_address(real_addr + d);
    trans->set_data_ptr(Data->read8p(dmap*parent->geom.linesize)+d);

    CDTRC(printf("%s: for address 0x%lx sends ", parent->name(), real_addr+d);\
	 for(int k = 0; k < parent->geom.secondary_blocksize_bytes; k++) \
	   printf("%02x", *(Data->read8p(dmap*parent->geom.linesize)+d+k)); \
	 printf("\n"));

    // 1-to-1 mapping btw memory controller and L2 bank
    POWER3(PW_TLM3(trans->pw_set_origin(parent, PW_TGP_DATA | PW_TGP_ADDRESS | PW_TGP_LANES | PW_TGP_ACCT_SRC, &parent->secondary_bus_tracker))); // Write out.
    parent->inita_socket[0]->b_transport(*trans, delay);
    POWER3(PW_TLM3(trans->pw_terminus(parent)));

    if(trans->is_response_error()) {
      char txt[256];
      snprintf(txt, 256, 
	       "%s: Error from cacheway secondary write addr=" PFX64 ", response status = %s", 
	       parent->name(), real_addr+d, trans->get_response_string().c_str());
      SC_REPORT_ERROR(__FILE__, txt);
    } 
  }

  Status[dmap] = invalid;
}

void secondary_cache_with_directory::cacheway::insert(u64_t addr, 
						      int dmap, 
						      u8_t *cline, 
						      sc_time& delay,
						      u64_t full_addr)
{
  CDTRC(printf("%s: cacheway=%i, insert line dmap=0x%x\n", 
	       parent->name(), m_way, dmap));
  
  switch (Status[dmap]) {
    case invalid:
    case clean:
      break;
      
    case dirty: {
      // if address is different need to evict
      POWER3(parent->log_energy_use(Tags->m_read_energy_op));
      delay += Tags->m_sr_latency;
      if(addr != (*Tags)[dmap]) {
	CDTRC(printf("%s way=%i Evict " PFX64 " for " PFX64 "\n", 
		     parent->name(), m_way, (*Tags)[dmap], addr));
	clear(delay, dmap, full_addr);
	break;
      }
      else 
	assert(0); // Should never happen
    }
  }


  POWER3(parent->log_energy_use(Tags->m_write_energy_op));  // Is this being double counted?
  delay += Tags->m_sr_latency;
  Tags->write(dmap, addr);
  int ops = parent->geom.linesize * 8 / Data->width;
  POWER3(parent->log_energy_use(Data->m_write_energy_op * ops));  // Is this being double counted?
  delay += Data->m_sr_latency * ops;
  memcpy(Data->read8p(dmap * parent->geom.linesize), cline, parent->geom.linesize);
  CDTRC(printf("%s: wrote data to address 0x%lx: ", parent->name(), addr); \
       for(int k = 0; k < parent->geom.linesize; k++)			\
	 printf("%02x", *(Data->read8p(dmap*parent->geom.linesize)+k)); \
       printf("\n"));

  Status[dmap] = clean;
}

// Return 0 on ok, -ve on bus error etc.
int secondary_cache_with_directory::main_memory_lookup(u64_t line_addr, 
						       u8_t *cline, 
						       sc_time &delay)
{
  // as memory is banked then bits we are using to select bank are not
  // to be used as part of address
  u64_t real_addr = (line_addr >> geom.dmap_shift) << (bound_log2(geom.linesize));
  CDTRC(printf("%s: real address for 0x%lx is 0x%lx\n", 
	       name(), line_addr, real_addr));

  PW_TLM_PAYTYPE nt, *trans = &nt; 
  trans->set_data_length(geom.secondary_blocksize_bytes);
  trans->set_byte_enable_length(0);
  trans->set_byte_enable_ptr(0); // All bytes to be operated on if ptr is null.
  trans->set_streaming_width(geom.linesize);
  trans->set_read();
  int st = geom.linesize;
  // linesize read in one go?
  for (int d=0; d<geom.linesize; d += geom.secondary_blocksize_bytes) 
    {
      printf("secondary_blocksize_bytes=%d\n", geom.secondary_blocksize_bytes);
      trans->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );
      trans->set_address(real_addr + d);

      trans->set_data_ptr(cline+d);
      // 1-to-1 mapping btw memory controller and L2 bank
      POWER3(PW_TLM3(trans->pw_set_origin(this, PW_TGP_ADDRESS | PW_TGP_ACCT_SRC, &secondary_bus_tracker))); // Read 
      inita_socket[0]->b_transport(*trans, delay);
      POWER3(PW_TLM3(trans->pw_terminus(this)));

      if (trans->is_response_error())
	{
	  char txt[256];
	  snprintf(txt, 256, "%s: Error from secondary access addr=" PFX64 ", response status = %s", 
		   name(), real_addr, trans->get_response_string().c_str());
	  SC_REPORT_ERROR(__FILE__, txt);
	} 
      CDTRC(printf("%s: for address 0x%lx receives ", name(), real_addr+d); \
	 for(int k = 0; k < geom.secondary_blocksize_bytes; k++) \
	   printf("%02x", *(cline+d+k));			 \
	 printf("\n"));

    }

  return 0;
}


secondary_cache_with_directory::cacheway *secondary_cache_with_directory::lookup(
  bool service,							      	 
  u64_t line_addr,
  int dmap,
  int loffset,
  u64_t* &datap1,
  sc_time& delay,
  u64_t full_addr)
{
  cacheway *cw = 0;
  u8_t* cline = 0;

  // look at all ways
  sc_time max_lookup_delay = SC_ZERO_TIME;
  for(int w = 0; w < geom.ways; w++) {
    cw = Cont[w];
    CDTRC(printf("%s: looking for line %lx in way %d with dmap %x\n",
		 name(), line_addr, w, dmap));
    bool present = cw->lookup(line_addr, 
			      dmap, 
			      &cline,
			      max_lookup_delay);
    if(present)
      break;
  }

  delay += max_lookup_delay;

  // do not service the miss
  if(!cline && !service)
    return 0;

  // we have a read hit
  if(cline) {
    stats.hits++;
    datap1 = ((u64_t*)(cline)) + loffset; // loffset in words
    CDTRC(printf("%s: Read hit, line_addr=" PFX64 " + %x 0x" PFX64 "\n", 
		name(), line_addr, loffset * 8, *datap1));
    return cw;
  }

  // service a miss 
  u8_t cline10[MAX_LINESIZE];
  int rc = main_memory_lookup(line_addr, cline10, delay);
  if(rc < 0) return 0;

  cw = Cont[lru];
  lru = (lru == geom.ways - 1) ? 0 : lru + 1;
  cw->insert(line_addr, dmap, cline10, delay, full_addr);
  
  // should hit this time
  cw->lookup(line_addr, dmap, &cline, delay);
  sysc_assert(cline);
  datap1 = ((u64_t*)cline) + loffset;
  stats.misses++;
  CDTRC(printf("%s: Miss serviced (way=%i) addr=" PFX64 ", dmap=%x, loffset=%i, 1stdata=0x" PFX64 "\n",
	       name(), cw->m_way, line_addr, dmap, loffset, *datap1));

  return cw;
}

void secondary_cache_with_directory::clean(sc_time &delay) {
  for(int w = 0; w < geom.ways; w++)
    for(int l = 0; l < geom.dmapping; l++) {
      Cont[w]->clear(delay, l, -1);
    }
}

void secondary_cache_with_directory::invalidate_line_in_primary(int core,
								int w,
								int addr,
								sc_time &delay) 
{
  PW_TLM_PAYTYPE *trans = way_mm.allocate();
  way_extension way;
  trans->set_auto_extension<way_extension>(&way);
  way.way = w;

  trans->set_data_length(8); // size of address
  trans->set_byte_enable_length(0);
  trans->set_byte_enable_ptr(0);
  trans->set_streaming_width(8);
  trans->set_read(); // this will be ignored
  trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
  trans->set_address(addr);
  trans->set_data_ptr(0);
  POWER3(PW_TLM3(trans->pw_set_origin(this, PW_TGP_ADDRESS | PW_TGP_ACCT_SRC, &cache_to_processor_xbus))); // Invalidate
  inita_socket[core + 1]->b_transport(*trans, delay);
  POWER3(PW_TLM3(trans->pw_terminus(this)));

  if(trans->is_response_error()) {
    char txt[256];
    snprintf(txt, 256, "%s: Error from sending invalidate msg, addr=%x, core=%d\n",
	     name(), addr, core);
    SC_REPORT_ERROR(__FILE__, txt);
  }
  
  return;
}
							       
u64_t secondary_cache_with_directory::get_dir_entry_content(int dmap, int way) {
  return (dmap << (bound_log2(geom.ways))) | way;
}

u64_t secondary_cache_with_directory::calculate_idir_index(u64_t line_addr,
							   u64_t way,
							   u64_t cpuid)
{
  u64_t p1 = cpuid & idir_geom.mask1;
  u64_t p2 = way & idir_geom.mask2;
  u64_t p3 = (line_addr >> idir_geom.rshift3) & idir_geom.mask3;
  u64_t p4 = (line_addr >> idir_geom.rshift4) & idir_geom.mask4;

  u64_t entry = (p4 << idir_geom.lshift4)
    | (p3 << idir_geom.lshift3)
    | (p2 << idir_geom.lshift2)
    | p1;

  return entry;
}

// TLM-2 blocking transport method
// if id is even then request has been sent by instruction cache
// id if is odd then request has been sent by data cache
void secondary_cache_with_directory::b_transport(
  int id,
  PW_TLM_PAYTYPE &trans,
  sc_time &delay)
{
  sysc_assert(trans.get_data_length() <= 8);

  int tid = sc_get_current_process_b()->proc_id;
  ml.lock();
  requests.push_back(tid);
  bool isTop = requests.front() == tid;
  if(busy_flag || !isTop) {
    if(!isTop && !busy_flag)
      // resend the wake event
      wake_event.notify();

    CDTRC(printf("tid=%i: Start wait core %d\n", tid, id));
    wait(delay);
    delay = SC_ZERO_TIME;

    do {
      CDTRC(printf("tid=%i: spin for core %i\n", tid, id));
      ml.unlock();
      wait(wake_event);
      ml.lock();
      isTop = requests.front() == tid;
    } while(busy_flag || !isTop);
  }
  busy_flag = true;
  ml.unlock();

  u64_t addr = ((u64_t)trans.get_address());
  u8_t *ptr = trans.get_data_ptr();

  u8_t *lanes = trans.get_byte_enable_ptr();
  int bel = trans.get_byte_enable_length();

  int dmap = (addr >> (u64_t)geom.dmap_shift) & (geom.dmapping - 1LLU);
  u64_t line_addr = addr & ~(geom.linesize - 1);
  u32_t len = trans.get_data_length();
  /// loffest in words
  int loffset = (addr >> geom.loffset_shift) & (geom.word64s_per_line-1LLU);

  // obtain entry from directory
  int dir_row = 
    (addr >> geom.directory_panel_row_shift) & geom.directory_panel_row_map;
  int dir_col = 
    (addr >> geom.directory_panel_col_shift) & geom.directory_panel_col_map;
  //CDTRC(printf("%s: directory entry = [%d,%d]\n", name(), dir_row, dir_col));
  
  tlm::tlm_command cmd = trans.get_command();

  POWER3(PW_TLM3(trans.pw_log_hop(this,  (cmd==tlm::TLM_READ_COMMAND ? PW_TGP_DATA: PW_TGP_NOFIELDS) | PW_TGP_ACCT_CKP,  &primary_bus_tracker))); // Read or write but not passthrough.

  if(cmd == tlm::TLM_READ_COMMAND) {
    way_extension* ext = 0;
    trans.get_extension(ext);
    assert(ext);
    u64_t *rdatap1;

    cacheway* cw = lookup(true, line_addr, dmap, loffset, rdatap1, delay, addr);
    
    if(id % 2 == 1) {
      directory_ty::panel dir_entry = directory->read(dir_row, dir_col);
      // replicate L1 tag
      u64_t tag = get_dir_entry_content(dmap, cw->m_way);
      dir_entry[(id-1)/2][ext->way] = tag;
      CDTRC(printf("%s: directory[%d][%d][%d][%d] has tag %lx\n",
		   name(), dir_row, dir_col, (id-1)/2, ext->way, tag));
    } 
    sysc_assert(rdatap1);
    u64_t ans = *rdatap1;
    // Data copy: one word (burst length words) from cache to 
    // buffer pointed at in payload
    *((u64_t *)ptr) = ans;
    assert(len == 8);
  }

  else if(cmd == tlm::TLM_WRITE_COMMAND) {
    // for now, we should never have a write request from instruction cache
    assert(id % 2 == 1);

    directory_ty::panel dir_entry = directory->read(dir_row, dir_col);
    stats.writes++;

    u64_t *wdatap = 0;
    cacheway *rp = lookup(false, line_addr, dmap, loffset, wdatap, delay, -1);

    // it is not in the cache so we need to find a place where we will write
    if(!rp) {
      cacheway* cw = Cont[lru]; // pick up way
      lru = (lru == geom.ways - 1) ? 0 : lru + 1;
      // if line is dirty clean it to main memory
      POWER3(log_energy_use(cw->Tags->m_read_energy_op));  // Is this being double counted?
      delay += cw->Tags->m_sr_latency;
      if(cw->Status[dmap] != cacheway::invalid && (*(cw->Tags))[dmap] == line_addr)
	cw->clear(delay, dmap, addr);
      // set the correct tag
      POWER3(log_energy_use(cw->Tags->m_read_energy_op));  // Is this being double counted?
      delay += cw->Tags->m_sr_latency;
      cw->Tags->write(dmap, line_addr);
      // initalize the whole line to zeros (could be unnecessary?)
      //memset(cw->Data->read8p(dmap * geom.linesize), 0, geom.linesize);
      int ops = geom.linesize * 8 / cw->Data->width;
      POWER3(log_energy_use(cw->Data->m_read_energy_op*ops));  // Is this being double counted?
      wdatap = ((u64_t*)(cw->Data->read8p(dmap * geom.linesize))) + loffset;

      cw->Status[dmap] = cacheway::dirty;

      CDTRC(printf("%s: cold write op addr= " PFX64 " dmap=0x%x dmap_shift=%i, way=%d\n",
		   name(), addr, dmap, geom.dmap_shift, lru));
    }
    else {
      CDTRC(printf("%s: warm write op addr= " PFX64 " dmap=0x%x dmap_shift=%i\n",
		   name(), addr, dmap, geom.dmap_shift));

      // need to invalidate processor that holds it
      u64_t tag = get_dir_entry_content(dmap, rp->m_way);
      for(int i = 0; i < 8; i++) 
	for(int j = 0; j < geom.primary_ways; j++) 
	  if(dir_entry[i][j] == tag) {
	    CDTRC(printf("%s: invalidate processor %d, way %d addr %lx; %lx\n", 
			 name(), i, j, addr, tag));
	    
	    invalidate_line_in_primary(i, j, addr, delay);
	    dir_entry[i][j] = -1;
	  }
    }
    
    sysc_assert(wdatap); // We should, by here, know where we are writing.
    for (int dd = 0; dd < len; dd += 8) {
      assert(dd + loffset < geom.linesize);
      if (!lanes 
	  || bel==0 
	  || ((u64_t *)lanes)[(dd % bel)/8] == 0xFFFFffffFFFFffffLLU)
	  wdatap[dd/8] = ((u64_t *)ptr)[dd/8];
      else {
	u8_t *ipw = (u8_t *)wdatap;
	CDTRC(printf("%s Write  (lanes=" PFX64 "...)\n", name(), *(u64_t *)lanes));
	for (int l=0;l<8;l++) 
	  if (lanes[(dd+l)%bel]) ipw[l+dd] = ptr[l+dd]; 
      }
    }
  }
  else
    assert(0);

  trans.set_response_status(tlm::TLM_OK_RESPONSE);
  POWER3(log_energy_use(std_energy_op));

  ml.lock();
  busy_flag = false;
  int top = requests.front();
  assert(top == tid);
  requests.pop_front();
  wake_event.notify();
  ml.unlock();


// end of transact: pass down dynamic energy use.
  POWER3(sc_pwr::pw_module_base::record_energy_use(m_running_op_energy, &trans));
  POWER3(m_running_op_energy = PW_ZERO_ENERGY);  
}



// constructor
secondary_cache_with_directory::smallram64::smallram64(
  sc_module_name name_, 
  int width, 
  int words,
    secondary_cache_with_directory* parent):
  sc_module(name_),
#ifdef TLM_POWER3
  pw_module("power_config_smallram.txt"),
#endif
  words(words),
  width(width),
  parent(parent)
{
  sysc_assert(width <= 64);
  sysc_assert(words > 0);
  u64_t bits = ((u64_t)words) * ((u64_t)width);
  printf("%s created, %llu bytes\n", name(), bits / 8LLU);
  
  recompute_ptv_parameters();
  Data = (u64_t *)malloc(words * sizeof(u64_t));
};


u64_t secondary_cache_with_directory::smallram64::read(u64_t idx)
{
  POWER3(parent->log_energy_use(m_read_energy_op));
  sysc_assert(idx >= 0 && idx < words);
  return Data[idx];
}

void secondary_cache_with_directory::smallram64::write(u64_t idx, u64_t d)
{
  POWER3(parent->log_energy_use(m_write_energy_op));
  sysc_assert(idx >= 0 && idx < words);
  Data[idx] = d;
}

// constructor
secondary_cache_with_directory::smallram8::smallram8(
  sc_module_name name_, 
  int width, 
  int words,
  secondary_cache_with_directory* parent):
  sc_module(name_),
#ifdef TLM_POWER3
  pw_module("power_config_smallram.txt"),
#endif
  words(words),
  width(width),
  parent(parent)
{
  sysc_assert(width <= 8);
  sysc_assert(words > 0);
  u64_t bits = ((u64_t)words) * ((u64_t)width);
  printf("%s created, %llu bytes\n", name(), bits / 8LLU);
  
  recompute_ptv_parameters();
  Data = (u8_t *)malloc(words * sizeof(u8_t));
};


u8_t secondary_cache_with_directory::smallram8::read(u64_t idx)
{
  POWER3(parent->log_energy_use(m_read_energy_op));
  sysc_assert(idx >= 0 && idx < words);
  return Data[idx];
}


u64_t secondary_cache_with_directory::smallram8::read64(u64_t idx)
// idx is a byte offset, as always.
{
  POWER3(parent->log_energy_use(m_read_energy_op * 8));
  sysc_assert(idx >= 0 && idx < words-8);

  return ((u64_t *)Data)[idx>>3LLU];
}

u8_t *secondary_cache_with_directory::smallram8::read8p(u64_t idx)
// idx is a byte offset, as always.
{
  // log power, assuming it will be used.
  POWER3(parent->log_energy_use(m_read_energy_op * 8)); 
  sysc_assert(idx >= 0 && idx < words);
  return &(Data[idx]);
}

void secondary_cache_with_directory::smallram8::write(u64_t idx, 
						      int blen, 
						      u8_t *d, 
						      u8_t *lanes)
{
  for (int w=0;w<blen;w++)
    {
      POWER3(parent->log_energy_use(m_write_energy_op));
      sysc_assert(idx+w >= 0 && idx+w < words);
      if (lanes[w]) Data[idx+w] = d[w]; // lanes: todo.
    }
}


void secondary_cache_with_directory::smallram8::recompute_ptv_parameters() // Called when Vcc is changed and so on.
{ 
  u64_t l_bits = words * width;
  m_sr_latency =  sc_time(0.21 + 3.8e-4 *sqrt(float(l_bits)), SC_NS);
#ifdef TLM_POWER3

  pw_power leakage = pw_power(82.0 * l_bits, PW_nW);
  set_static_power(leakage);

  set_fixed_area(sc_pwr::pw_area(13359.0 + 4.93 * l_bits, PW_squm)); // or from tech file

  m_read_energy_op = pw_energy(5.0 + 1.2e-4 * 8.0 *l_bits, pw_energy_unit::PW_pJ);
  m_write_energy_op = 2.0 * m_read_energy_op; // rule of thumb!
  pw_voltage vcc = get_vcc();
  m_sr_latency = m_sr_latency / vcc.to_volts();
#endif
  cout << name() << " basic latency = " << m_sr_latency << "\n";
}

void secondary_cache_with_directory::smallram64::recompute_ptv_parameters() // Called when Vcc is changed and so on.
{
  u64_t l_bits = words * width;
  m_sr_latency =  sc_time(0.21 + 3.8e-4 *sqrt(float(l_bits)), SC_NS);
#ifdef TLM_POWER3

  pw_power leakage = pw_power(82.0 * l_bits, PW_nW);
  set_static_power(leakage);

  set_fixed_area(sc_pwr::pw_area(13359.0 + 4.93 * l_bits, PW_squm)); // or from tech file

  m_read_energy_op = pw_energy(5.0 + 1.2e-4 * 8.0 *l_bits, pw_energy_unit::PW_pJ);

  pw_voltage vcc = get_vcc();
  m_sr_latency = m_sr_latency / vcc.to_volts();
#endif
  cout << name() << " basic latency = " << m_sr_latency << "\n";
}

// eof

