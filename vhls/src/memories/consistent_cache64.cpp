// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2011-14 D J Greaves + M Puzovic
// (C) 2009-11 D J Greaves
// University of Cambridge, Computer Laboratory.
// VHLS/PRAZOR TLM2.0 Simulator

/*
consistent_cache64   A parameterisable set-associative cache with two write strategies and a line buffer
                     to accumulate partially dirty words. Snooping on other caches within a consistent
                     group is enabled by passing each cache an instance of a consisent\_group object.
                     Each way consists of a pair of instances of sram64 to contain data and tags.

*/




// Generic payload blocking transport consistent cache.

#include <stdint.h>
#include "systemc.h"
#include "tenos.h"
#include "consistent_cache64.h"
// include "../mtracer/tracedriven_core.h"
#include "llsc_extension.h"

/*
This is a set-associative cache with one-place write buffer.  It can
cope with reads and writes of burst length up to one cache line.  Read
and write operations on the lower port (here called secondary) are
always complete cache lines.

The write buffer provides dirty flags on a per-byte basis, supporting
byte writes.

Reads use random eviction.  Writes are put in the write buffer unless
the set-associative array is already warm with that cache line. All
writes are only copied back to secondary store on eviction.


Consistency is implemented by calling every other cache in a
consistency group when cache lines change status using the MESI
protocol.  No data is directly passed between caches in a consistent
group: instead dirty lines are evicted to secondary store and then
re-served from that.

TODO: Add bypass for llsc and DMI.

 */

/*If MANUAL_DELAY is 0 then we are using m_sr_latency field on tags and
 * data lines to calculate latency in accessing the cache, otherwise it
 * is using a fixed constant (hit_cycle_time field in consistent cache
 * class) to calculate delay. 
 *
 * I have added MANUAL_DELAY macro in order to be able to
 * experiment with different cache hit times, but this experimentation
 * can be done I believe also with changing factors in formula for m_sr_latency.
 */
#define MANUAL_DELAY 0

using namespace std;

extern bool dynload_done;


// Constructor
cache64::cacheway::cacheway(cache64 *parent, int way):
  parent(parent),
  m_way(way)
{
  char name[64];
  snprintf(name, 64, "Lock_%d", way);
  waylock = new sc_mutex(name);
  snprintf(name, 64, "Data_%d", way);
  Data = new smallram8(name, 
		       8, 
		       parent->geom.dmapping * sizeof(u8_t) * parent->geom.linesize,
		       parent);


  snprintf(name, 64, "Tags_%d", way);
  Tags = new smallram64(name, 
			64-parent->geom.dmap_shift, 
			parent->geom.dmapping, 
			parent);

  Status = (mesi_t *) malloc(parent->geom.dmapping * sizeof(mesi_t));
  for (int i = 0; i<parent->geom.dmapping; i++) 
    Status[i] = invalid;
}

//
// Method called by local cache to update a given line's status.
//
cache64::cacheway::mesi_t cache64::cacheway::operate(mesi_t op, u64_t addr, int dmap, bool &changed)
{
  mesi_t old_state = Status[dmap];

  changed = (Status[dmap] != op);
  switch (op)
    {
    case invalid:
      Status[dmap] = op;
      break;

    case shared:
      Status[dmap] = op;
      break;

    case modified:
      Status[dmap] = op;
      break;
      
    case exclusive:
      Status[dmap] = op;

    case owned:
      Status[dmap] = op;

    default:
      sysc_assert1(parent, 0);

    }

  return old_state;
}


//
// Check whether cache line is changed and if so invalidates entries in all other caches.
//
void cache64::operate(cacheway *cw, cacheway::mesi_t op, u64_t addr, int dmap, sc_time &delay_)
{
  sysc_assert(cw);
  
  cw->waylock->lock();
  cw->Status[dmap] = op;
  cw->waylock->unlock();

} 

// we should only clean one cache line
void cache64::cacheway::clean(sc_time &delay_, 
			      int dmap, 
			      cache_miss_extension* cme, 
			      bool force)
{
  waylock->lock();
  if(Status[dmap] != modified 
     && Status[dmap] != owned
     && !force) {
    waylock->unlock();
    return;
  }

  if(!force) 
    Status[dmap] = invalid;
  waylock->unlock();

  CPTRC((*Tags)[dmap], 
	printf("CACHE64::CACHEWAY(%d) clean() -> sending data ", m_way);	\
       Data->print_content(); \
       printf(" to secondary storage\n"));
    
  // we should set cache line to invalid immediately even before
  // we complete copying to the secondary storage. The reason why
  // we need to do it before is because if we use temporal decoupling
  // it can happen that one core is writing content of cache line to
  // the secondary storage and before it is completed we can have
  // another core reading the same cache line. If it is not set to invalid
  // before start of the transfer the second core will think that
  // line is still valid.

#if 0
  // This is now handled by the queue system.

  // Before we write check if there are other caches that are
  // waiting for this line if there are then their transaction
  // should be aborted and either this way should send transaction
  // on the bus or the core that was waiting needs to reissue... 
  if(parent->grp)
    for(int g = 0; g < parent->grp->target_members; g++) {
      if(parent->grp->Members[g] == parent) continue;
      
      cache64* c = parent->grp->Members[g];
..      
      if(c->secondary_storage_lreq == (*Tags)[dmap])
	c->secondary_storage_lreq = -1;		  
    }
#endif
  

  /*parent->ml.lock();
  u64_t oldAddr = 0;
  if(parent->busyFlag) {
    oldAddr = parent->busyAddr;
    parent->busyAddr = (*Tags)[dmap];
  }
  parent->ml.unlock();*/

  // as we are doing the cleaning (i.e. writting dirty line to secondary
  // storage) we can have some other core trying to access the same
  // cache line so set the status of this line to invalid before we send
  // write request to secondary storage 

  //for (int i=0; i<parent->geom.dmapping; i++)
  //{
  //if (Status[i] == modified)
  //{

  // for know, we should never need to clean instruction cache line
  assert(parent->my_type == CT_DATA || parent->my_type == CT_UNIFORM);

  cache_miss_extension mm;
  PW_TLM_PAYTYPE *trans = parent->cache_miss_mm.allocate();
  trans->set_auto_extension<cache_miss_extension>(&mm);
  trans->acquire();

  // data length will either be secondary block size if it is less then
  // the size of cache line otherwise it is going to be cacheline
  int dl = parent->geom.linesize >= parent->geom.secondary_blocksize_bytes ?
    parent->geom.secondary_blocksize_bytes :
    parent->geom.linesize;
  trans->set_data_length(dl);

  trans->set_byte_enable_length(0);
  trans->set_byte_enable_ptr(0); // All bytes to be operated on if ptr is null.
  trans->set_streaming_width(parent->geom.linesize);
  trans->set_write();

  for(int d=0; d<parent->geom.linesize; d += parent->geom.secondary_blocksize_bytes) {
    trans->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );

    POWER3(parent->pw_module_base::record_energy_use(Tags->m_read_energy_op, trans)); // Read tags
    trans->set_address((*Tags)[dmap] + d);

    int ops = parent->geom.linesize * 8 / Data->width;
    //std::cout << "Tmp debug " << parent->name() << " ops=" << ops << "\n";
    POWER3(parent->pw_module_base::record_energy_use(Data->m_read_energy_op * ops, trans)); // Read data

#if !MANUAL_DELAY
    AUGMENT_LT_DELAY(trans, delay,  Tags->m_sr_latency);
    AUGMENT_LT_DELAY(trans, delay,  Data->m_sr_latency * ops);
#endif

    trans->set_data_ptr(Data->read8p(dmap*parent->geom.linesize)+d);

    POWER3(PW_TLM3(trans->pw_set_origin(parent, PW_TGP_DATA | PW_TGP_ADDRESS | PW_TGP_LANES | PW_TGP_ACCT_SRC, &parent->secondary_bus_tracker))); // Evict write.
    parent->inita_socket->b_transport(*trans, delay);
    POWER3(PW_TLM3(trans->pw_terminus(parent)));
    if (trans->is_response_error()) {
	char txt[256];
	snprintf(txt, 256, 
		 "%s: Error from cacheway secondary write addr=" PFX64 ", response status = %s", 
		 parent->name(), (*Tags)[dmap]+d, trans->get_response_string().c_str());
	SC_REPORT_ERROR(__FILE__, txt);
    } 
  }

  trans->release();

  CPTRC((*Tags)[dmap], 
	printf("CACHE64::CACHEWAY(%d) clean() -> data sent; new cache line state %d\n", m_way, Status[dmap])); 

  /*  parent->ml.lock();
  if(parent->busyFlag) {
    assert(parent->busyAddr == (*Tags)[dmap]);
    parent->busyAddr = oldAddr;
  }
  parent->ml.unlock();*/


  /*if(!force) {
    waylock->lock();
    Status[dmap] = invalid;
    waylock->unlock();
    }*/

}


//
// Adds a new line to a cache way.
//
bool cache64::cacheway::insert(u64_t addr, 
			       int dmap, 
			       u8_t *cline, 
			       sc_time &delay_,
			       cache_miss_extension* cme,
			       mesi_t new_state)
{
  CPTRC(addr, printf("CACHE64::CACHEWAY(%d) insert() -> dmap %d\n", m_way, dmap));
  
  switch (Status[dmap])
    {
    case invalid:
    case exclusive: 
    case shared:
      break;

    case modified:
    case owned:
      // if address is different need to evict
      POWER3(parent->log_energy_use(Tags->m_read_energy_op)); // Read tags - double counted?
#if !MANUAL_DELAY
      AUGMENT_LT_DELAY(trans, delay,  Tags->m_sr_latency);
#endif
      if (addr != (*Tags)[dmap])
	{
	  CPTRC(addr, 
		printf("CACHE64::CACHEWAY(%d) insert() -> collision, evicting data ", m_way);\
		Data->print_content();\
		printf(" to secondary storage\n"));
	  
	  u8_t* dd = Data->read8p(dmap * parent->geom.linesize);
	  parent->evict_lower_level_line((*Tags)[dmap], 
					 parent->geom.linesize,
					 dd);

	  dd = Data->read8p(dmap * parent->geom.linesize);
	  clean(delay, dmap, cme);
	  // stats.evict(... log this ... );
	  break;
	}

      else 
	{
	  printf("End of world: %s way=%i insert already present " PFX64 " for " PFX64 " - dmap alias?\n", parent->name(), m_way, (*Tags)[dmap], addr);
	  sysc_assert1(parent, 0); // Should never happen.
	}

    default:
      sysc_assert1(parent, 0);
    }

  POWER3(parent->log_energy_use(Tags->m_write_energy_op)); // Write tags
#if !MANUAL_DELAY
  AUGMENT_LT_DELAY(trans, delay,  Tags->m_sr_latency);
#endif
  Tags->write(dmap, addr);

  int ops = parent->geom.linesize * 8 / Data->width;
  //std::cout << "Tmp debug " << parent->name() << " ops=" << ops << "\n";
  POWER3(parent->log_energy_use(Data->m_write_energy_op * ops)); // Write data
#if !MANUAL_DELAY
  AUGMENT_LT_DELAY(trans, delay,  Data->m_sr_latency * ops);
#endif
 
  memcpy(Data->read8p(dmap * parent->geom.linesize), cline, parent->geom.linesize);

  waylock->lock(); // This write non atomic?
  Status[dmap] = new_state; 
  waylock->unlock();

  CPTRC(addr, 
	printf("CACHE64::CACHEWAY(%d) insert(): new data with state %d: ", m_way, Status[dmap]);	\
	Data->print_content(dmap*parent->geom.linesize, parent->geom.linesize));

    { // cw = Cont[w];
    }

  return true;
}

// Return true if the address is present.
bool cache64::cacheway::lookup(u64_t addr, int dmap, u8_t **clinep, sc_time &max_delay)
{
  // It is wrong to add the latency for each way's lookup to delay since these are done in parallel (should do a max join over them).
  POWER3(parent->log_energy_use(Tags->m_read_energy_op)); // Read tags - being double counted?

#if !MANUAL_DELAY
  max_delay = max(max_delay, Tags->m_sr_latency);
#endif

  if (Status[dmap] != invalid && (*Tags)[dmap] == addr) // Should this use the smallcache read function for delay and energy.?
    {
      if (clinep)
	{ // Not every lookup leads to data being read
	  int ops = parent->geom.linesize * 8 / Data->width;
	  //std::cout << "Tmp debug " << parent->name() << " ops=" << ops << "\n";
	  POWER3(parent->log_energy_use(Data->m_read_energy_op * ops)); // Read data
#if !MANUAL_DELAY
	  max_delay = max(max_delay, Data->m_sr_latency * ops);
#endif
          *clinep = Data->read8p(dmap * parent->geom.linesize);
	}
      return true;
    }
  else return false;
}


// clean: write out dirty contents: only some lanes of a burst may be dirty and some bursts may not need issuing.
void cache64::write_buffer::clean(sc_time &delay, cache_miss_extension* ext)
{

  WBTRC(printf("WRITE BUFFER clean() -> initiating sending of data ");	\
	cline.print_content();
	printf(" to secondary storage.\n"));

  wbl.lock();
  // Secondary memory word width may be less than linesize.
  if (m_addr != -1)
    {

      u64_t addr_m = m_addr;
      evict_addr = m_addr;
      m_addr = -1;
      wbl.unlock();

      /*parent->ml.lock();
      u64_t oldAddr = 0;
      if(parent->busyFlag) {
	oldAddr = parent->busyAddr;
	parent->busyAddr = addr_m;
      }
      parent->waylock->unlock();*/

      parent->ml.lock();
      if(parent->secondary_reqs.find(addr_m) != parent->secondary_reqs.end())
	parent->invalid_reqs.insert(addr_m);
      parent->ml.unlock();

      PW_TLM_PAYTYPE* trans = parent->cache_miss_mm.allocate(); 
      trans->set_auto_extension<cache_miss_extension>(ext);
      trans->acquire();

      u8_t data[MAX_LINESIZE];
      u8_t lanemask[MAX_LINESIZE]; 
      for(int i = 0; i < MAX_LINESIZE; i++) {
	lanemask[i] = 0x00;
	data[i] = 0;
      }
      
      // data length will either be secondary block size if it is less then
      // the size of cache line otherwise it is going to be cacheline
      int dl = linesize >= parent->geom.secondary_blocksize_bytes ?
	parent->geom.secondary_blocksize_bytes :
	linesize;
      trans->set_data_length(dl);

      trans->set_byte_enable_length(linesize);
      trans->set_byte_enable_ptr((u8_t *)&lanemask); // All bytes to be operated on if ptr is null.
      trans->set_streaming_width(linesize);//??
      
      trans->set_write();
      for (int d=0; d<linesize; d += parent->geom.secondary_blocksize_bytes)       // linesize read in one go?
	{
	  bool w = false;
	  // Irrespective of the system being modelled, the cache and main memory use simulator workstation endianness. This is little for x86.  
	  for (int i=0; i<min(parent->geom.secondary_blocksize_bytes, linesize); i++) 
	    if (Dirty[d+i]) 
	      {
		lanemask[i] = 0xFF;
		//Dirty[d+i] = false;
		w = true;
	      }
	    else 
	      {
		lanemask[i] = 0x00;
	      }
	  if (w)
	    {
	      
	      // before writting buffer to secondary memory we need to
	      // invalidate cache line from consistency group, but by
	      // MESI definition we should not have cache line in 
	      // other consistency group

	      trans->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );
	      trans->set_address(addr_m + d);
	      
	      // we need to make a copy of the write buffer content as
	      // we have already released the entry to be overwrittent
	      // by someone else
	      for(int k = 0; k < parent->geom.secondary_blocksize_bytes; k++)
		data[k] = *(cline.read8p(d)+k);
	      // Reference to a byte view of internal data.
	      trans->set_data_ptr(&data[0])
;
	      POWER3(PW_TLM3(trans->pw_set_origin(parent, PW_TGP_DATA | PW_TGP_LANES | PW_TGP_ADDRESS | PW_TGP_ACCT_SRC, &parent->secondary_bus_tracker))); // Write out
	      parent->inita_socket->b_transport(*trans, delay);
	      POWER3(PW_TLM3(trans->pw_terminus(parent)));
	      if (trans->is_response_error())
		{
		  char txt[256];
		  snprintf(txt, 256, "%s: Error from secondary write addr=" PFX64 ", response status = %s", 
			   parent->name(), addr_m, trans->get_response_string().c_str());
		  SC_REPORT_ERROR(__FILE__, txt);
		} 
	    }
	}
      trans->release();

      wbl.lock();
      evict_addr = -1;
      /*for(int d = 0; d < linesize; d++)
	Dirty[d] = false;*/
      wbl.unlock();
      
      /*parent->ml.lock();
      if(parent->busyFlag) {
	if(parent->busyAddr != addr_m) {
	  fprintf(stdout, "0x%lx vs 0x%lx\n", parent->busyAddr, addr_m);
	}
	assert(parent->busyAddr == addr_m);
	parent->busyAddr = oldAddr;
      }
      parent->ml.unlock();*/
    }
  else
    wbl.unlock();

  WBTRC(printf("WRITE BUFFER clean() -> data sent to secondary storage\n"));
}

void cache64::clean(sc_time &delay)
{
  for (int w=0; w<geom.ways; w++) 
    for(int l=0; l<geom.dmapping; l++)
      Cont[w]->clean(delay, l, 0);

  // Todo: some of this might be done in parallel, so accumulating a total in delay is not correct - but this is only used on IPL (initial program load).
  if(buf0)
    buf0->clean(delay, 0);
}



// Conduct TLM operation on next level memory down.
// This socket should have width equal to secondary_width?
// Needs better be passthrough for passing DMI and uncached/LLSC transactions straight thru ?
// Return 0 on ok, -ve on bus error etc.
int cache64::secondary_lookup(u64_t line_addr, 
					 u8_t *cline, 
					 sc_time &delay, 
					 int w,
			      cache_miss_extension* cme,
			      pw_customer_acct *customer_id)
{
  PW_TLM_PAYTYPE *trans;

  CTRC(line_addr, "CACHE64 secondary_lookup() -> request to secondary storage\n"); 

  // pick up port where to send it
  // depends on the number of banks secondary storage has
  int bank = geom.secondary_banks == 1 
    ? 0 
    : ((line_addr >> geom.secondary_linesize_bits) & (geom.secondary_banks - 1));

  trans = cache_miss_mm.allocate(); 
  PW_TLM3(trans->set_customer_acct(customer_id));
  trans->set_auto_extension<cache_miss_extension>(cme);
  trans->acquire();

  // data length will either be secondary block size if it is less then
  // the size of cache line otherwise it is going to be cacheline
  int dl = geom.linesize >= geom.secondary_blocksize_bytes ?
    geom.secondary_blocksize_bytes :
    geom.linesize;
  trans->set_data_length(dl);
  trans->set_byte_enable_length(0);
  trans->set_byte_enable_ptr(0); // All bytes to be operated on if ptr is null.
  trans->set_streaming_width(geom.linesize);
  trans->set_read();
  for (int d=0; d<geom.linesize; d += geom.secondary_blocksize_bytes)       // linesize read in one go?
    {
      trans->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );
      trans->set_address(line_addr + d);

      trans->set_data_ptr(cline+d);
//define DETAILED_LOOK
#ifdef DETAILED_LOOK
      sc_time start_t = COLLECT_LT_DELAY(delay_ + sc_time_stamp(), trans.ltd.point());
#endif
      POWER3(PW_TLM3(trans->pw_set_origin(this, PW_TGP_ADDRESS | PW_TGP_ACCT_SRC, &secondary_bus_tracker))); // Read a line
      if(inita_socket.size() > 0) // is this IF really needed?
	inita_socket[bank]->b_transport(*trans, delay);
      else
	inita_socket->b_transport(*trans, delay);
#ifdef DETAILED_LOOK
      sc_time end_t =  COLLECT_LT_DELAY(delay + sc_time_stamp(), trans.ltd.point());
      cout << name() << " backside service time " << end_t-start_t << " (delay=" << trans.ltd << ")\n"; 
#endif

      POWER3(PW_TLM3(trans->pw_terminus(this)));
      if (trans->is_response_error())
	{
	  char txt[256];
	  snprintf(txt, 256, "%s: Error from secondary access addr=" PFX64 ", response status = %s", 
		   name(), line_addr, trans->get_response_string().c_str());
	  SC_REPORT_ERROR(__FILE__, txt);
	} 
    }


  CTRC(line_addr, cout << "CACHE64 secondary lookup() -> delay: " << delay << endl);

  trans->release();

  return 0;
}


bool cache64::write_buffer::word_present(int offset)
{
  int i;
  for (i=0; i<8; i++) if (!Dirty[offset*8+i]) break;
  bool present = (i==8); // All bytes are dirty.
  if(present) {
    WBTRC(printf("WRITE BUFFER word_present() -> word that starts at address 0x%lx is present\n", m_addr+offset*8));
  }
    
  return present; 
}

void cache64::cacheway::print(FILE *fd, int dmap)
{
  fprintf(fd, "Cacheway %i mesi=%i " PFX64 ":\n", dmap, Status[dmap], (*Tags)[dmap]);
  for (int i=0;i<parent->geom.linesize;i++)
    fprintf(fd, "%02X", (*Data)[dmap * parent->geom.linesize+i]);
  fprintf(fd, "\n");
}


// Lookup function (not for load-linked).
// Servicef is read/not-write for local work and false for snoop operations.
cache64::cacheway *cache64::lookup(bool servicef, 
				   u64_t line_addr, 
				   int dmap, 
				   int loffset, 
				   u64_t * &datap1, 
				   sc_time &delay, 
				   cache_miss_extension* cme,
				   pw_customer_acct *customer_acct,
				   bool nested,
				   int second_trip) {
  bool foundf = 0;
  cacheway *cw = 0;
  u8_t *cline = 0;
  sc_time max_lookup_delay = SC_ZERO_TIME;
  CTRC(line_addr, printf("CACHE64 lookup() -> trip number %i\n", second_trip));

  if(do_i_lookup(line_addr))
      for (int w=0; w<geom.ways; w++)
      {
          cw = Cont[w];
          bool present = cw->lookup(line_addr, dmap, &cline, max_lookup_delay);
          if (present) {
	    CTRC(line_addr,
		 printf("CACHE64 lookup() -> cache line present in way %d\n", w));
	    break;
          }
      }
#if !MANUAL_DELAY
  AUGMENT_LT_DELAY(trans, delay,  max_lookup_delay);
#endif

  if (!cline && !servicef) 
    {
      return 0;
    }
  if (cline) 
    { // Read hit
      stats.hits += 1;
      datap1 = ((u64_t*)(cline)) + loffset; // loffset in words
#if MANUAL_DELAY
      AUGMENT_LT_DELAY(trans, delay,  clock_period * hit_cycle_time);
#endif

#if 0
      // already accounted in lookup
      int ops = parent->geom.linesize * 8 / Data->width;
      //std::cout << "Tmp debug " << parent->name() << " ops=" << ops << "\n";
      POWER3(parent->pw_module_base::record_energy_use(Data->m_read_energy_op * ops, trans)); // Read data
      AUGMENT_LT_DELAY(trans, delay,  Data->m_sr_latency * ops);
#endif

      operate_miss_type(cw, dmap, cme);

      return cw;
    }


  u8_t snooped_data[MAX_LINESIZE];
  int exist = do_a_snoop(nested, servicef, line_addr, delay, cme, customer_acct, &snooped_data[0]);

  lru = (lru == geom.ways - 1) ? 0 : lru + 1;
  cw = Cont[lru];

  bool ins = true;
  if(exist > 0) {
    assert(snooped_data != 0);
    stats.snooped_reads += 1;
    ins = insert_line(cw, line_addr, dmap, snooped_data, delay, cme, cacheway::mesi_t::shared);
    assert(ins);
  }
  else {
    assert(exist == 0);
    // Service a read miss from main memory
    u8_t cline10[MAX_LINESIZE];
    ml.lock();
    bool requested = true;
    if(secondary_reqs.find(line_addr) == secondary_reqs.end()) 
      {
	secondary_reqs.insert(line_addr);
	requested = false;
      }
    ml.unlock();
    if(!requested) {
      do
	{
	  int rc = secondary_lookup(line_addr, cline10, delay, lru, cme, customer_acct);
	  if (rc<0) return 0;
	  ml.lock();
	  if(invalid_reqs.find(line_addr) != invalid_reqs.end()) {
	    invalid_reqs.erase(line_addr);
	  }
	  else 
	    {
	      ml.unlock();
	      break;
	    }
	  ml.unlock();
	} while(1);

      cacheway::mesi_t ns = cacheway::mesi_t::invalid;

      if(cc_protocol == MOESI_CC) {
        assert(cme != NULL);
	if(cme->resp == CT_MISS_SHARED)
	  ns = cacheway::mesi_t::shared;
	else if(cme->resp == CT_MISS_EXCLUSIVE)
	  ns = cacheway::mesi_t::exclusive;
	else if(cme->resp == CT_MISS_DIRTY)
	  ns = cacheway::mesi_t::modified;
	else if(cme->resp == CT_MISS_OWNED)
	  ns = cacheway::mesi_t::owned;
	else assert(0);
	
	assert(ns != cacheway::mesi_t::invalid);
      }
      else
	ns = cacheway::mesi_t::exclusive;

      ins = insert_line(cw, line_addr, dmap, cline10, delay, cme, ns);
      ml.lock();
      secondary_reqs.erase(line_addr);
      secondary_req_event.notify();
      ml.unlock();
    }
    else 
      {
	bool w = true;
	do {
	  wait(secondary_req_event);
	  ml.lock();
	  w = !(secondary_reqs.find(line_addr) == secondary_reqs.end());
	  ml.unlock();
	} while(w);
	// search again in case the data has been loaded in the meantime. find the way
	for (int w=0; w<geom.ways; w++) {
	  cacheway *cw = Cont[w];
	  bool present = cw->lookup(line_addr, dmap, &cline, delay);
	  if (present) break;
	}
	assert(cline);
	
      }
  }
  
  if(!ins)
    {
      if (second_trip > 3) // This is blowing the stack on uncontrolled recursion in some error state... 
	{
	  printf("%s: Likely to loop indefinitely looking up " PFX64 " \n", name(), line_addr);
	  assert(0); 
	}
      return lookup(servicef, line_addr, dmap, loffset, datap1, delay, cme,  customer_acct, nested, second_trip+1); // do it again
    }
  else {
    bool present = cw->lookup(line_addr, dmap, &cline, delay); // should hit this time!
    sysc_assert(present);
    //for(int i=0;i<geom.linesize;i++) printf("%02X", cline[i]); printf(" B loffset=%i\n", loffset);
    datap1 = ((u64_t*)cline) + loffset; // loffset in words
    //printf("Miss serviced line_addr=" PFX64 ", loffset=%i %p\n", line_addr, loffset, datap1);
    stats.misses += 1;
    CTRC(line_addr, printf("CACHE64 lookup() -> miss serviced to way %i at dmap %i\n", cw->m_way, dmap));
    if(cme) 
      cme->miss[level][my_type] = true;
    return cw;
  }
}

void cache64::stat_header(const char *msg, FILE *fd) {
  fprintf(fd, "\n\n"); 
  fprintf(fd, "   Hits    Misses   Sharing Evictions  Ratio   Writes   Reads   Id\n");
  fprintf(fd, "------------------------------------------------------------------\n");
}

void cache64::stat_report(const char *msg, FILE *fd, bool resetf, bool no_header)
{
  const char *f1 = write_policy ==  WRITE_THROUGH ? "WRITE_THROUGH": "WRITE-BACK"; 
  const char *f2 = 
    write_miss == READ_ALLOCATE ? "READ_ALLOCATE":
    write_miss == NO_READ_ALLOCATE ? "NO_READ_ALLOCATE":
    write_miss == WRITE_ALLOCATE ? "WRITE_ALLOCATE": "NO_WRITE_ALLOCATE";
  const char *f3 = 
    store_miss == READ_ALLOCATE ? "READ_ALLOCATE":
    store_miss == NO_READ_ALLOCATE ? "NO_READ_ALLOCATE":
    store_miss == WRITE_ALLOCATE ? "WRITE_ALLOCATE": "NO_WRITE_ALLOCATE";

  int s = stats.hits + stats.misses;
  if (fd) {
    if(no_header)
      fprintf(fd, "%s: %s: Hits=%i  Misses=%i ratio=%1.3f. SharingEvictions=%i SnoopedReads=%i  Writes=%i, Reads=%i %s/%s/%s\n", 
	      name(), 
	      msg, 
	      stats.hits, 
	      stats.misses, 
	      (s) ? float(stats.hits) / float(s):0.0,  
	      stats.sharing_evictions, 
	      stats.snooped_reads, 
	      stats.writes, stats.reads, f1, f2, f3);
    else
      fprintf(fd, "%7d%10d%20d%7.3f%9d%8d %s/%s/%s  %s\n",
	      stats.hits,
	      stats.misses,
	      stats.sharing_evictions,
	      (s) ? float(stats.hits) / float(s) : 0.0,
	      stats.writes,
	      stats.reads,
	      f1, f2, f3,
	      name() 
	      );
  }
    
    
  if (resetf) {
      //printf("%s stats reset\n", name());
      stats.reset();
  }
}

void cache64::stat_accumulate(stats_t& total) {
  total.hits += stats.hits;
  total.misses += stats.misses;
  total.sharing_evictions += stats.sharing_evictions;
  total.writes += stats.writes;
  total.reads += stats.reads;
}

void cache64::stats_t::totals(FILE* fd) {
  int s = hits + misses;
  fprintf(fd, "------------------------------------------------------------------\n");
  fprintf(fd, "%7d%10d%20d%7.3f%9d%8d\n",
	  hits,
	  misses,
	  sharing_evictions,
	  (s) ? float(hits) / float(s) : 0.0,
	  writes,
	  reads);

}


void cache64::end_of_simulation()
{
  stat_report("End of simulation", stdout);
}

// constructor
cache64::write_buffer::write_buffer(cache64 * parent, int linesize, int secondary_width):
  parent(parent),
  cline("writebuffer_cline", 8, linesize, parent),
  linesize(linesize),
  secondary_width_bytes(secondary_width/8),
  wbl("write_buffer_lock")
{
  m_addr = -1; // invalid
  evict_addr = -1;

  for (int i=0; i<linesize; i++) 
    Dirty[i] = false;
}




u64_t *cache64::write_buffer::hit(u64_t addr, int loffset) 
{ 
  //  return (addr == m_addr) ? &((u64_t *)cline)[loffset]:0; 
  // TODO  POWER3(parent->pw_module_base::record_energy_use( ); // Write buffer energy read.
  // m_AUGMENT_LT_DELAY(trans, delay,    .. read Data)
  if (addr == m_addr) {
    WBTRC(printf("WRITE BUFFER hit() -> present\n"));
    return (u64_t *)(cline.read8p(loffset*8));
  } else {
    return 0;
  }
}



void cache64::write_buffer::mark_dirty(int len, int loffset, u8_t *lanes, int bel)
{
  for (int i=0; i<len; i++)
    {
      if (!lanes || !bel || lanes[i % bel]) 
	{
	  Dirty[i+8*loffset] = true;
	}
    }

  WBTRC(printf("WRITE BUFFER mark_dirty() -> marked bytes from address 0x%lx to 0x%lx as dirty\n", m_addr+8*loffset, m_addr+8*loffset+len-1));
}

u64_t *cache64::write_buffer::init(u64_t addr, 
				   int loffset, 
				   int dmap,
				   cache_miss_extension* ext,
				   sc_time& delay)
{
  sysc_assert1(parent, m_addr == -1); // must be empty  

  if(parent->cc_protocol == MOESI_CC) {
    // also send the invalid message to all other cores
    PW_TLM_PAYTYPE *trans;
    
    trans = parent->cache_miss_mm.allocate();
    trans->set_auto_extension<cache_miss_extension>(ext);
    trans->acquire();
    
    trans->set_data_length(parent->geom.linesize);
    trans->set_data_ptr(0);
    trans->set_write();
    trans->set_byte_enable_ptr(0);
    trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    trans->set_address(addr);
    
    parent->inita_socket->b_transport(*trans, delay);
    assert(trans->is_response_error() == false);


    trans->release();
  }
  wbl.lock();
  m_addr = addr;
  m_dmap = dmap;
  // clean dirty flags
  for(int i = 0; i < linesize; i++) {
    Dirty[i] = false;
  }
  wbl.unlock();

  WBTRC(printf("WRITE BUFFER init() -> about to write data\n"));

  // TODO initial power and delay need accounting here.
  return (u64_t *)(cline.read8p(loffset*8));
}



// Constructor
cache64::cache64(sc_module_name mname,  
		 int bytes, 
		 int ways, 
		 int linesize, 
		 int secondary_width,
		 sc_time clock_period,
		 int cache_level,
		 cache_ty ty,
		 uint32_t hct,
		 int secondary_bl, 
		 on_miss_ty store_miss,
		 on_miss_ty write_miss,
		 write_policy_ty write_policy,
		 bool use_write_buffer,
		 int secondary_linesize,
		 int secondary_banks, 
		 cc_protocol_ty protocol,
		 bool big_endian // ?
		 ):
  sc_core::sc_module(mname), 
#ifdef TLM_POWER3
  pw_module() ,
#endif
  targ_socket("targ_socket"), 
  inita_socket("inita_socket"),
  buf0(0),
  level(cache_level),
  secondary_storage_lreq(-1),
  store_miss(store_miss),
  write_miss(write_miss),
  write_policy(write_policy),
  use_write_buffer(use_write_buffer),
  my_type(ty),
  hit_cycle_time(hct),
  clock_period(clock_period),
  busyFlag(false),
  busy_way(NULL),
  busyAddr(0),
#ifdef TLM_POWER3
#if PW_TLM_PAYLOAD > 0
  primary_bus_tracker(this),
  secondary_bus_tracker(this),  
#endif
#endif
  ml("cache_lock"),
  cc_protocol(protocol)
  //, busy_flag(false)
{
  traceregions = 0;
  geom.big_endian_ = big_endian;
  geom.ways = ways;
  geom.linesize = linesize;
  geom.bytes = bytes;
  geom.secondary_blocksize_bytes= secondary_width/8 * secondary_bl;
  geom.secondary_bl = secondary_bl;
  // Input sizes are all in bytes.
  sysc_assert((secondary_width % 8) == 0);
  sysc_assert(geom.linesize > 0 && (geom.linesize % 8)==0); // Each line must be a multiple of 64 bits.
  sysc_assert(geom.ways > 0); // When ways=1 we are directly mapped
  sysc_assert(bytes > 0 && (bytes % (8 * geom.ways * geom.linesize)) == 0); // total size needs to be a valid multiple.
  sysc_assert(geom.linesize <= MAX_LINESIZE);
  sysc_assert(secondary_width >= 64);
  sysc_assert(geom.secondary_blocksize_bytes >= geom.linesize);
  sysc_assert((geom.secondary_blocksize_bytes % geom.linesize)==0);
  // number of cache lines that are in one way
  geom.dmapping = bytes / (geom.ways * geom.linesize);
  sysc_assert(bound_log2(geom.dmapping) > bound_log2(geom.dmapping-1)); // ie dmapping is a power of 2.

  // size of the cache lines in words - 1 word = 8 bytes
  geom.word64s_per_line = geom.linesize / 8;

  /*printf("%s: ways=%i, linesize=%i bytes, dmapping=%i (totalsize=%i bytes).\n", 
	 name(), 
	 geom.ways, 
	 geom.linesize, 
	 geom.dmapping, 
	 geom.bytes));*/

  // Need to shift by: (3 for bytes per word) + log words_per_line;
  geom.dmap_shift =  3 + bound_log2(geom.word64s_per_line);

  geom.loffset_shift = 3; // Always 3 for 64 bit words when byte addressed.

  if(secondary_linesize < linesize)
    geom.secondary_linesize_bits = bound_log2(linesize);
  else
    geom.secondary_linesize_bits = bound_log2(secondary_linesize);
  geom.secondary_banks = secondary_banks;

  // End of geometry computations.
  Cont = (cacheway **) malloc(geom.ways * sizeof(cacheway*));
  for (int w=0; w<geom.ways; w++)
    {
      Cont[w] = new cacheway(this, w);
    }
  lru = 0;
  //clean(delay); // should power up as clean!ss

#ifdef TLM_POWER3
  std_energy_op = pw_energy((double) (0.1 * 64), pw_energy_unit::PW_pJ); 
  // based on: just made up!
  //POWER3(set_excess_area(pw_length(150 * geom.ways, PW_um), pw_length(150,  PW_um)));

#endif

  // Register callbacks for incoming interface method calls
  targ_socket.register_b_transport(this, &cache64::b_transport);
  targ_socket.register_nb_transport_fw(this, &cache64::nb_transport_fw);
  targ_socket.register_get_direct_mem_ptr(this, &cache64::get_direct_mem_ptr);

  // write buffer
  if(use_write_buffer)
    buf0 = new write_buffer(this, linesize, secondary_width);

  new tenos_report_item(name(), "snooped_reads", &stats.snooped_reads);
  new tenos_report_item(name(), "hits", &stats.hits);
  new tenos_report_item(name(), "misses", &stats.misses);
  new tenos_report_item(name(), "sharing_evictions", &stats.sharing_evictions);

}

//
// 1 of 2 write operations.
//
void cache64::write_through(int id,
				       PW_TLM_PAYTYPE &trans, 
				       sc_time &delay,
				       u64_t addr) 
{
  assert(0);
  // find out to which memory bank we need to send the transaction
  int mask = geom.secondary_banks == 1 ? 0x1 : geom.secondary_banks - 1;
  int bank = (addr >> geom.secondary_linesize_bits) & mask;

  PW_TLM_PAYTYPE fwd; 

  fwd.set_write();
  fwd.set_address(trans.get_address());
  fwd.set_data_length(trans.get_data_length());
  fwd.set_data_ptr(trans.get_data_ptr());
  fwd.set_byte_enable_length(trans.get_byte_enable_length());
  fwd.set_byte_enable_ptr(trans.get_byte_enable_ptr());
  fwd.set_streaming_width(trans.get_streaming_width());
  fwd.set_dmi_allowed(false); 
  fwd.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

  POWER3(PW_TLM3(fwd.pw_set_origin(this, PW_TGP_DATA | PW_TGP_ADDRESS | PW_TGP_LANES | PW_TGP_ACCT_SRC, &secondary_bus_tracker))); // Write thru.
  inita_socket[bank]->b_transport(fwd, delay);
  POWER3(PW_TLM3(fwd.pw_terminus(this)));

  if(fwd.is_response_error()) {
    char txt[100];
    sprintf(txt, "Error from b_write_through addr=" PFX64 ", response status = %s",
	     addr, trans.get_response_string().c_str());
    SC_REPORT_ERROR("cache64", txt);
  }
  
}

//
// 2 of 2 write operations.
//
void cache64::write_back(PW_TLM_PAYTYPE &trans, 
				    sc_time &delay,
				    u64_t line_addr,
				    int loffset,
				    u64_t addr,
				    u32_t len,
				    int dmap,
				    u8_t* ptr,
				    cache_miss_extension* cme) 
{
  pw_customer_acct *customer_acct = 0;
  PW_TLM3(customer_acct = trans.get_customer_id());

  if(cc_protocol == MOESI_CC && trans.is_write() && trans.get_data_ptr() == 0) {
    // this is invalidation message broadcasted from lower level
    // caches, as there is no data we just need to forward it
    trans.set_data_length(geom.linesize);
    trans.set_address(line_addr);

    inita_socket->b_transport(trans, delay);
    assert(trans.is_response_error() == false);

    return;    
  }

  stats.writes += 1;
  u8_t *lanes = trans.get_byte_enable_ptr();
  u64_t *wdatap = 0;
  int bel = trans.get_byte_enable_length();
  if(buf0) {
    buf0->wbl.lock();
    wdatap = buf0->hit(line_addr, loffset);
    buf0->wbl.unlock();
  }
 
  bool warm_write = false;
  cacheway *rp = NULL;
  if(wdatap) // Write coalesc with existing dirty data?
    {
      CTRC(addr, printf("CACHE64 write_back() -> warm buffered write\n"));
      buf0->mark_dirty(len, loffset, lanes, bel);
      buf0->wbl.unlock();
      
      AUGMENT_LT_DELAY(trans, delay,  buf0->data_latency(len));
    }
  else  // If the line is in the cache then update that, else use write buffer.
    {
      rp = lookup(false, line_addr, dmap, loffset, wdatap, delay, cme, customer_acct);
      if (wdatap)
	{
	  CTRC(addr, printf("CACHE64 write_back() -> warm cache line write\n"));

	  operate(rp, cacheway::mesi_t::modified, line_addr, dmap, delay);
          warm_write = true;
	}
      else
	{	      
	  if(!buf0) {
	    inita_socket->b_transport(trans, delay);
	    return;
	  }
	  else {
	    CTRC(addr, printf("CACHE64 write_back() -> cold buffered write\n"));

	    buf0->clean(delay, cme);
	    line_invalidation(line_addr, loffset, dmap, cme, delay);

	    u8_t* data; u8_t* flags;
	    tie(data, flags) = check_lower_level_lines(line_addr, 
						       buf0->linesize,
						       cacheway::invalid);
	    wdatap = buf0->init(line_addr, loffset, dmap, cme, delay);	      
	    if(data != NULL) {
	      assert(flags != NULL);
	      u8_t* wb_data = buf0->cline.read8p(0);
	      for(int i = 0; i < buf0->linesize; i++) {
		if(flags[i] == 0xFF) {
		  wb_data[i] = data[i];
		  buf0->Dirty[i] = true;
		}
	      }
	    }
	   	  
	    buf0->mark_dirty(len, loffset, lanes, bel);	      
	  }
	}
    }
  sysc_assert(wdatap); // We should, by here, know where we are writing.
  for (int dd =0; dd<len; dd+=8)
    {
      assert(dd+loffset < geom.linesize);
      if (!lanes || bel==0 || ((u64_t *)lanes)[(dd % bel)/8] == 0xFFFFffffFFFFffffLLU)
	{
	  //C1TRC(addr, printf("%s Write64 0x%lx\n", name(), ((u64_t *)ptr)[dd/8]));
	  wdatap[dd/8] = ((u64_t *)ptr)[dd/8];
	}
      else
	{
	  u8_t *ipw = (u8_t *)wdatap;
	  //C1TRC(addr, printf("%s Write  (lanes=" PFX64 "...)\n", name(), *(u64_t *)lanes));
	  for (int l=0;l<8;l++) 
	    {
              //C1TRC(addr, printf("%i %i  %i   %p %p g=%i data=0x%02X\n", dd, l, bel, ipw, ptr, lanes[(dd+l)%bel], ptr[l+dd]));
	      if (lanes[(dd+l)%bel]) {
		ipw[l+dd] = ptr[l+dd]; 
	      }
	    }
	}
    }
  if(warm_write) {
      rp->waylock->lock();
      busy_way = NULL;
      rp->waylock->unlock();
  }

}

void cache64::b_transport(int id, PW_TLM_PAYTYPE &trans, sc_time &delay) {
  u64_t addr = trans.get_address();
  u64_t line_addr = addr & ~(geom.linesize-1);

  int tid = sc_get_current_process_b()->proc_id;
  ml.lock();
  requests.push(tid);
  bool isTop = requests.front() == tid;
  if(busyFlag || !isTop) {
      if(!isTop && !busyFlag)
          busyWait.notify();

      wait(delay);
      delay = SC_ZERO_TIME;
      do {
          ml.unlock();
          wait(busyWait);
          ml.lock();
          isTop = requests.front() == tid;
      } while(busyFlag || !isTop);
  }

  busyFlag = true;
  busyAddr = line_addr;
  ml.unlock();
  //printf("%s: start %x " PFX64 "\n", name(), addr);
  b_access(id, trans, delay, addr);
  //printf("%s: end %x " PFX64 "\n", name(), addr);
  ml.lock();
  busyFlag = false;
  busyAddr = 0;
  int top = requests.front();
  assert(top == tid);
  requests.pop();
  busyWait.notify();
  ml.unlock();
}

// TLM-2 blocking transport method
void cache64::b_access(int id, 
		       PW_TLM_PAYTYPE &trans, 
		       sc_time &delay_, 
		       u64_t addr)
{
  // we can call this method with id either equal to zero or equal to one.
  // when it is equal to zero it means that request is coming from the local core
  // while if it is equal to one it means that it is coming from remote node

  // the requests from remote node can come at any time so we need to order them
  // if they are trying to access the same cache line that the local core
  // is accessing at the moment we would need to wait for that event to finish
  // before we process the cache line
  u8_t *ptr = trans.get_data_ptr();
  int dmap = (addr >> (u64_t)geom.dmap_shift) & (geom.dmapping-1LLU);
  int loffset  = (addr >> geom.loffset_shift) & (geom.word64s_per_line-1LLU); // loffset in words
  u64_t line_addr = addr & ~(geom.linesize-1);
  u32_t len = trans.get_data_length();

  sc_time start_time = COLLECT_LT_DELAY(delay_ + sc_time_stamp(), trans.ltd.point());
  tlm::tlm_command cmd = trans.get_command();
  pw_customer_acct *customer_acct = 0;
  PW_TLM3(customer_acct = trans.get_customer_id());  
  llsc_extension* linked = 0;
  trans.get_extension(linked); 
				 
  cache_miss_extension* c_ext = 0;
  trans.get_extension(c_ext);
  if(c_ext == 0) {
      c_ext = new cache_miss_extension;
      trans.set_extension(c_ext);
  }
  
  if (linked) 
    { // Do an invalidate (on all caches in grp even if we do not have it! ).
      buf0->clean(delay, c_ext);
      line_invalidation(line_addr, loffset, dmap, c_ext, delay);
      //printf("%s .. cache llsc invalidate\n", name());
    }
  if (/*UNCACHED_ADDRESS_SPACE64(addr) ||*/ linked) 
    {
        //  fprintf(stdout, "addr=%lx\n", addr);
        //  assert(0); 
      bool tf = traceregions && traceregions->check(addr, TENOS_TRACE_MEM_READ|TENOS_TRACE_MEM_WRITE);
      int bank = geom.secondary_banks == 1 
	? 0 
	: ((addr >> geom.secondary_linesize_bits) & (geom.secondary_banks - 1));
      
#ifdef TLM_POWER3
      PW_TLM3(trans.pw_log_hop(this,  0,  &secondary_bus_tracker)); // It will pass to the secondary bus.
#endif
      inita_socket[bank]->b_transport(trans, delay);
      if (tf) printf("%s uncached or linked op pass at " PFX64 " op=%s\n", name(), addr, cmd == tlm::TLM_READ_COMMAND ?"read": "write");
#ifdef TLM_POWER3
      PW_TLM3(trans.pw_log_hop(this,  0,  &primary_bus_tracker)); 
#endif
      return;
    }
  
  
  POWER3(PW_TLM3(pw_agent_record l_agent =
		 trans.pw_log_hop(this,  
				  (cmd==tlm::TLM_READ_COMMAND ? 
				   PW_TGP_DATA: PW_TGP_NOFIELDS) 
				  | PW_TGP_ACCT_CKP,  
				  &primary_bus_tracker))); // Read or write but not passthrough.
  
  sysc_assert(trans.get_data_length() <= 8);    
  
#if 0
  // if id is greater then zero then message wasn't recieved from core
  // but from other module (in this case from L2). This message consists
  // of address and way that we need to invalidate
  if(id > 0) {
    way_extension* ext = 0;
    trans.get_extension(ext);
    assert(ext);
    ml->lock();
    // for now we let messages to race to lock but should consider
    // implementing order based on the timestamp at which msg was received
    invq.push(make_pair(dmap, ext->way));
    ml->unlock();
    trans.set_response_status(tlm::TLM_OK_RESPONSE);
    return;
  }
  // message received form the core
  else {
    ml->lock();
    // invalidate all entries first
    while(!invq.empty()) {
      pair<int, int> e;
      e = invq.front();
      C1TRC(addr, printf("%s: Need to invalidate way %d whose dmap is %lx\n", 
		  name(), e.second, e.first));
      Cont[e.second]->Status[e.first] = cacheway::invalid;
      invq.pop();
    }
    ml->unlock();
  }
#endif
  
  if (len % 8)  {
    printf("%s: Illegal len or wid: len=%i wid=%i\n", name(), len, 64);
    trans.set_response_status(tlm::TLM_BURST_ERROR_RESPONSE);
    return;
  }
  
  
  if (cmd == tlm::TLM_READ_COMMAND) {
    bool tf = traceregions && traceregions->check(addr, TENOS_TRACE_MEM_READ);
    if (tf) printf("%s: cache read op at " PFX64 " len=%i dmap=0x%x, dmap_shift=%i,delay=%s\n", 
		       name(), addr, len, dmap, geom.dmap_shift, delay.to_string().c_str());
    stats.reads+=1;
    u64_t *rdatap1 = 0;
    if(buf0 && do_i_lookup(line_addr)) {
      rdatap1 = buf0->hit(line_addr, loffset);
    }
      
    if(rdatap1) {
      // We can service from write buffer if a complete word is present, otherwise clean write buffer
      // and miss as usual.
      // An improvement would be to check only for the requested read lanes.

      if (buf0->word_present(loffset)) {
	//xprintf("%s: read op addr=" PFX64 " dmap=0x%x    dmap_shift=%i\n", 
	//name(), addr, dmap, geom.dmap_shift);
	// Ignore byte lanes on read and return complete word.
	
	AUGMENT_LT_DELAY(trans, delay,  buf0->data_latency(len));
	// TODO power...

	if (c_ext) c_ext->resp = CT_MISS_DIRTY;
      }
      else {
	rdatap1 = 0; // Discard partial hit
	buf0->clean(delay, c_ext); // Write it all out and then lookup using reload on miss.
      }
    }
    
    if (!rdatap1) {
      cacheway *rc = lookup(true, line_addr, dmap, loffset, rdatap1, delay, c_ext, customer_acct);
      if (rc) {
	// Ignore byte lanes on read and return complete word.
      }
      else 
	sysc_assert(0); // incomplete response
    }
    
    sysc_assert(rdatap1);
    u64_t ans =  *rdatap1;
    *((u64_t *)ptr) = ans; 
    //Data copy: one word (burst length words) from cache to buffer pointed at in payload.
    assert(len == 8);
  }
  else if (cmd == tlm::TLM_WRITE_COMMAND) {

    bool tf = traceregions && traceregions->check(addr, TENOS_TRACE_MEM_WRITE);
    if (tf) 
      printf("%s: cache write op at " PFX64 " len=%i dmap=0x%x, dmap_shift=%i,delay=%s\n", 
	     name(), addr, len, dmap, geom.dmap_shift, delay.to_string().c_str());


    switch(write_policy) {
    case WRITE_THROUGH:
      write_through(id, trans, delay, addr);
      break;
    case WRITE_BACK:
      write_back(trans, delay, line_addr, loffset, addr, len, dmap, ptr, c_ext);
      break;
    default:
      assert(0); // Unknown write policy
      assert(len == 8);
    }
  }
  
  
  trans.set_response_status(tlm::TLM_OK_RESPONSE);
  
  sc_time end_time = COLLECT_LT_DELAY(delay_ + sc_time_stamp(), trans.ltd.point());
// end of transact: pass down dynamic energy use.
  POWER3(log_energy_use(std_energy_op));
#if PW_TLM_PAYLOAD > 0 
  POWER3(l_agent.record_energy_use(m_running_op_energy, &trans));
#else
  POWER3(record_energy_use(m_running_op_energy, &trans));
#endif
  POWER3(m_running_op_energy = PW_ZERO_ENERGY);  
}



bool cache64::get_direct_mem_ptr(int n, PW_TLM_PAYTYPE &trans, tlm::tlm_dmi &dmi_data)
{
  inita_socket/*[n]*/->get_direct_mem_ptr(trans, dmi_data);
}

void cache64::set_sr_latencies(int tag_words,
			       int tag_width,
			       int data_words,
			       int data_width,
			       double factor) {

  for(int i = 0; i < geom.ways; i++) {
    cacheway* cw = Cont[i];
    cw->Data->set_sr_latency(data_words, data_width, factor);
    cw->Tags->set_sr_latency(tag_words, tag_width, factor);
  }

  return;
}

// constructor
cache64::smallram64::smallram64(sc_core::sc_module_name name_, int width, int words, cache64 *parent):
  sc_module(name_),
#ifdef TLM_POWER3
  pw_module(),
#endif
  words(words),
  width(width),
  parent(parent)
{
  sysc_assert(width <= 64);
  sysc_assert(words > 0);
  u64_t bits = ((u64_t)words) * ((u64_t)width);
  u64_t l_bits = words * width;
#ifdef TLM_POWER3
  pw_power leakage = pw_power(82.0 * l_bits, PW_nW);
  set_static_power(leakage);

  set_fixed_area(pw_area(13359.0 + 4.93/8 * l_bits, PW_squm)); // or from tech file
#endif

  m_sr_latency =  sc_time(0.21 + 3.8e-4 *sqrt(float(l_bits)), SC_NS);

  recompute_pvt_parameters();
  Data = (u64_t *)malloc(words * sizeof(u64_t));
};


u64_t cache64::smallram64::read(u64_t idx)
{
  POWER3(parent->log_energy_use(m_read_energy_op));
  sysc_assert(idx >= 0 && idx < words);
  return Data[idx];
}

void cache64::smallram64::write(u64_t idx, u64_t d)
{
  POWER3(parent->log_energy_use(m_write_energy_op));
  sysc_assert(idx >= 0 && idx < words);
  Data[idx] = d;
}

// constructor
cache64::smallram8::smallram8(sc_core::sc_module_name name_, int width, int words, cache64 *parent):
  sc_module(name_),
#ifdef TLM_POWER3
  pw_module(),
#endif
  words(words),
  width(width),
  parent(parent)
{
  sysc_assert(width <= 8);
  sysc_assert(words > 0);
  u64_t bits = ((u64_t)words) * ((u64_t)width);

  u64_t l_bits = words * width;
#ifdef TLM_POWER3
  pw_power leakage = pw_power(82.0 * l_bits, PW_nW);
  set_static_power(leakage);

  set_fixed_area(pw_area(13359.0 + 4.93/8 * l_bits, PW_squm)); // or from tech file
#endif

  m_sr_latency =  sc_time(0.21 + 3.8e-4 *sqrt(float(l_bits)), SC_NS);

  recompute_pvt_parameters();  
  Data = (u8_t *)malloc(words * sizeof(u8_t));
};


u8_t cache64::smallram8::read(u64_t idx)
{
  POWER3(parent->log_energy_use(m_read_energy_op));
  sysc_assert(idx >= 0 && idx < words);
  return Data[idx];
}


u64_t cache64::smallram8::read64(u64_t idx) // idx is a byte offset, as always.
{
  POWER3(parent->log_energy_use(m_read_energy_op * 8));
  sysc_assert(idx >= 0 && idx < words-8);
  return ((u64_t *)Data)[idx>>3LLU];
}

u8_t *cache64::smallram8::read8p(u64_t idx) // idx is a byte offset, as always.
{
  POWER3(parent->log_energy_use(m_read_energy_op * 8)); // log power, assuming it will be used.
  sysc_assert(idx >= 0 && idx < words);
  return &(Data[idx]);
}

void cache64::smallram8::print_content() {
  for(int i = 0; i < words; i++)
    fprintf(stdout, "%02x", Data[i]);
}

void cache64::smallram8::print_content(u64_t idx, u64_t length) {
    sysc_assert(idx >= 0 && (idx + length - 1) < words);
    for(int i = 0; i < length; i++)
        fprintf(stdout, "%02x", Data[idx+i]);
}



void cache64::smallram8::write(u64_t idx, int blen, u8_t *d, u8_t *lanes)
{
  for (int w=0;w<blen;w++)
    {
      POWER3(parent->log_energy_use(m_write_energy_op));
      sysc_assert(idx+w >= 0 && idx+w < words);
      if (lanes[w]) Data[idx+w] = d[w]; // lanes: todo.
    }
}


void cache64::smallram8::recompute_pvt_parameters() // Called when Vcc is changed and so on.
{ 
#ifdef TLM_POWER3
  u64_t l_bits = words * width;
  m_read_energy_op = pw_energy(5.0 + 1.2e-4 / 8.0 *l_bits, pw_energy_unit::PW_pJ);
  m_write_energy_op = 2.0 * m_read_energy_op; // rule of thumb!
  pw_voltage vcc = get_vcc();
  m_sr_latency = m_sr_latency / vcc.to_volts();
#endif
}

void cache64::smallram8::set_sr_latency(int _words,
					int _width,
					double factor) {

  m_sr_latency = sc_time((0.21 + 3.8e-4 * sqrt(float(_words * _width)))/factor, SC_NS);
}

void cache64::smallram64::recompute_pvt_parameters() // Called when Vcc is changed and so on.
{
#ifdef TLM_POWER3
  u64_t l_bits = words * width;
  m_read_energy_op = pw_energy(5.0 + 1.2e-4 / 8.0 *l_bits, pw_energy_unit::PW_pJ);

  pw_voltage vcc = get_vcc();
  m_sr_latency = m_sr_latency / vcc.to_volts();
#endif

}

void cache64::smallram64::set_sr_latency(int _words,
					 int _width,
					 double factor) {
  m_sr_latency = sc_time((0.21 + 3.8e-4 * sqrt(float(_words * _width)))/factor, SC_NS);
}


sc_time cache64::write_buffer::data_latency(int len_in_bytes)
{

  return sc_time ((16 + len_in_bytes) / 16 * 200, SC_PS); // Made up figure.
}



PW_TLM_PAYTYPE* cache_miss_mm_t::allocate() {
  PW_TLM_PAYTYPE* ptr;

  lck.lock();
  if(free_list) {
    ptr = free_list->trans;
    empties = free_list;
    free_list = free_list->next;
  }
  else
    ptr = new (PW_TLM_PAYTYPE)(this);
  lck.unlock();

  if(ptr->get_ref_count() < 0)
    while(ptr->get_ref_count() != 0)
      ptr->acquire();

  assert(ptr->get_ref_count() == 0);

  return ptr;
}


void cache_miss_mm_t::free(PW_TLM_PAYTYPE* trans) 
{
  // do not need to release extension as they are all allocated on stack
  // so clearing should be enough
  cache_miss_extension* cme = 0;
  trans->get_extension(cme);
  if(cme)
    trans->clear_extension<cache_miss_extension>(cme);

  // clears the extension pointers for sure
  trans->reset();

  lck.lock();
  if(!empties) {
    empties = new access;
    empties->next = free_list;
    empties->prev = 0;
    if(free_list)
      free_list->prev = empties;
  }
  free_list = empties;
  free_list->trans = trans;
  empties = free_list->prev;
  lck.unlock();
}

consistent_cache64::consistent_cache64(sc_module_name name, 
				       consistent_group* grp,
				       int bytes, 
				       int ways, 
				       int linesize, 
				       int secondary_width, 
				       int cache_level, 
				       cache_ty ty,
				       cc_protocol_ty protocol,
				       sc_time clock_period,
				       uint32_t hit_cycles,
				       int bl,
				       on_miss_ty store_miss,
				       on_miss_ty write_miss,
				       write_policy_ty write_policy,
				       bool use_write_buffer,
				       int secondary_linesize,
				       int secondary_banks,
				       bool big_endian)

  : cache64(name, bytes, ways, linesize, secondary_width, clock_period, cache_level,
	    ty, hit_cycles, bl, store_miss, write_miss, write_policy, use_write_buffer,
	    secondary_linesize, secondary_banks, protocol, big_endian),
    grp(grp)
{
  if(grp) grp->add(this);  
}

int consistent_cache64::do_a_snoop(bool nested,
				   bool servicef,
				   u64_t addr,
				   sc_time& delay,
				   cache_miss_extension* cme,
				   pw_customer_acct *customer_acct,
				   u8_t* snooped_data) {
  int exist = 0;
  set<cacheway::mesi_t> states;
  if (grp && !nested) {  // Enquire for this cache line in my consistent neighbours if any.
    int g;
    // This for loop is done in parallel on snoopy systems:
    for (g = 0; g < grp->target_members; g++) {
      if (grp->Members[g] == this) continue;
      bool exclude = !servicef; // For a write we will want to evict others on a share.
      cacheway::mesi_t state;
      bool present = grp->Members[g]->grp_snoop(addr, 
						exclude, 
						delay, 
						cme, 
						customer_acct,
						state, 
						snooped_data);
      // Need to be careful in accumulating the delays from parallel activities
      // since there may be some serialised delay included, such as contention in 
      // access to next-level store.
      if (present) {
	exist++;
	states.insert(state);
      }
    }
     
    if (g != grp->target_members) {
      // something happened, e.g. remote evicted - we don't care 
      //at the moment since we read from next level.  Do we ? 
    }
  }

#if 0 
  // safety check
  if(exist == 1) {
    assert(states.size() == 1);
    assert(*states.begin() == cacheway::mesi_t::exclusive 
	   || *states.begin() == cacheway::mesi_t::modified
	   || *states.begin() == cacheway::mesi_t::owned
	   // although there is only one copy of the line in cache
	   // it can be in shared state if a line from consistent cache(s)
	   // that was shared with have been replaced by some other line
	   || *states.begin() == cacheway::mesi_t::shared);
  }
  else if(exist > 1) {
    assert(states.size() == 1 || states.size() == 2);
    for(set<cacheway::mesi_t>::iterator it = states.begin(); it != states.end(); ++it) 
      assert(*it == cacheway::mesi_t::shared || *it == cacheway::mesi_t::owned);
  }
#endif


  return exist;
}


//
// Called over consistency bus to serve a neighbour's request for a given line.
// Returns true if the neighbour still has a copy after the snoop operations.
//
bool consistent_cache64::grp_snoop(u64_t addr, 
				   bool exclude, 
				   sc_time &delay, 
				   cache_miss_extension* cme,
				   pw_customer_acct *customer_acct,
				   cacheway::mesi_t& state,
				   u8_t* data)
{
  CTRC(addr, printf("CACHE64 grp_snoop() -> consistency request\n"));
  int dmap = (addr >> (u64_t)geom.dmap_shift) & (geom.dmapping-1LLU);
  int loffset  = (addr >> geom.loffset_shift) & (geom.word64s_per_line-1LLU);
  u64_t line_addr = addr & ~(geom.linesize-1);

  if (buf0) {
    buf0->wbl.lock();
    if(buf0->hit(line_addr, loffset)) {
      CTRC(addr, printf("CACHE64 grp_snoop() -> write buffer has the cache line and will evict it\n"));
      stats.sharing_evictions += 1;
      buf0->wbl.unlock();
      buf0->clean(delay, cme); // turf out of write buffer. Will be dirty no doubt.
      return false;
    }
    else 
      buf0->wbl.unlock();
  }

  u64_t *rdatap1=0;
  cacheway *rc = lookup(false, line_addr, dmap, loffset, rdatap1, delay, cme, customer_acct, true);

  if (rc && (//cc_protocol == MOESI_CC &&   
	     rc != busy_way)) {
    CTRC(addr, printf("CACHE64 grp_snoop() -> snoop has a copy of cache line in way %d\n", rc->m_way));
    // check if cache lines at the lower level have been modified
    u8_t* ll_data; u8_t* flags;
    cacheway::mesi_t new_state = cacheway::mesi_t::shared;
    if(cc_protocol == MOESI_CC)
      new_state = cacheway::mesi_t::owned;

    tie(ll_data, flags) = check_lower_level_lines(line_addr, 
					       geom.linesize,
					       new_state);

    u8_t* upper_level_data = rc->Data->read8p(dmap * geom.linesize);
    
    if(ll_data != NULL) {
      assert(flags != NULL);
      for(int i = 0; i < geom.linesize; i++) 
	if(flags[i] == 0xFF)
	  upper_level_data[i] = ll_data[i];

      state = cacheway::mesi_t::modified;
      memcpy(data, upper_level_data, geom.linesize);

      switch(cc_protocol) {
        case MESI_CC:
	  rc->clean(delay, dmap, cme); // turf it out!
	  stats.sharing_evictions += 1;
	  
	  rc->waylock->lock();
	  rc->Status[dmap] = cacheway::mesi_t::shared;
	  rc->waylock->unlock();
	  return true;

        case MOESI_CC:
	  rc->waylock->lock();
	  rc->Status[dmap] = cacheway::mesi_t::owned;
	  rc->waylock->unlock();
	  return true;

        default:
	  assert(0);
      }
    }
    else {
      switch(rc->Status[dmap]) {
        case cacheway::mesi_t::exclusive: 
	  CTRC(addr, 
	       printf("CACHE64 grp_snoop() -> cache line in EXCLUSIVE state. To be changed to SHARED"));
	  // Do not need to write back if exclusive
	  //rc->clean(delay, dmap, cme); // turf it out!
	  //stats.sharing_evictions += 1;
	  state = cacheway::mesi_t::exclusive;
	  memcpy(data, rc->Data->read8p(dmap * geom.linesize), geom.linesize);
	  rc->waylock->lock();
	  rc->Status[dmap] = cacheway::mesi_t::shared;
	  rc->waylock->unlock();
	  return true;
	       
        case cacheway::mesi_t::shared: { 
	  if (exclude) {
	    
	    assert(0); // not sure if we ever get in here?
	    // Snooping a write, shared is no longer an option here, 
	    // so a sharing eviction.
	    stats.sharing_evictions += 1;
	    rc->clean(delay, dmap, cme); // turf it out!
	    return false; // Do not stop - other caches may have it in shared mode.
	  }
	  else {
	    CTRC(addr, 
		 printf("CACHE64 grp_snoop() -> cache line in SHARED state\n"));
	    // We have it, and might serve from here in the future. 
	    // Stop since can't be exclusive elsewhere.
	    state = cacheway::mesi_t::shared;
	    memcpy(data, rc->Data->read8p(dmap * geom.linesize), geom.linesize);
	    
	    return true;
	  }
	}

        case cacheway::mesi_t::modified: {
	  CTRC(addr, 
	       printf("CACHE64 grp_snoop() -> cache line in MODIFIED state. To be written to secondary storage and changed to SHARED"));
	  rc->clean(delay, dmap, cme); // turf it out!
	  stats.sharing_evictions += 1;
	  
	  state = cacheway::mesi_t::modified;
	  
	  memcpy(data, rc->Data->read8p(dmap * geom.linesize), geom.linesize);
	  
	  rc->waylock->lock();
	  rc->Status[dmap] = cacheway::mesi_t::shared;
	  rc->waylock->unlock();
	  
	  return true;
	}

        case cacheway::mesi_t::invalid:
	  printf("%s: invalid should not happen\n", name());
	  sysc_assert(0);
	  break;

        case cacheway::mesi_t::owned:
	  CTRC(addr, 
	       printf("CACHE64 grp_snoop() -> cache line in OWNED state. To be changed to SHARED"));

	  state = cacheway::mesi_t::owned; // stays in the same state
	  memcpy(data, rc->Data->read8p(dmap * geom.linesize), geom.linesize);

	  return true;

        default:
	  printf("%s: Should not happen\n", name());
	  sysc_assert(0);
      }
    }
  }

  state = cacheway::mesi_t::invalid;
  return false;
}

void consistent_cache64::operate(cacheway *cw, 
				 cacheway::mesi_t op, 
				 u64_t addr, 
				 int dmap, 
				 sc_time &delay)
{
  sysc_assert(cw);
  
  cw->waylock->lock();
  //bool changed=false;
  //cacheway::mesi_t os = cw->operate(op, addr, dmap, changed); 
  cacheway::mesi_t os = cw->Status[dmap];
  CTRC(addr, printf("CACHE64 operate() -> state changed from %d to %d\n", os, op));
  cw->waylock->unlock();

  // if the previous state was shared then it needs to invalidate
  // the same cache line from its consistent peers
  if(os != op
     && (os == cacheway::shared || os == cacheway::owned)
     && grp)
    for(int g = 0; g < grp->target_members; g++) {
      if(grp->Members[g] == this) continue;

      cache64* c = grp->Members[g];
      
      for(int w = 0; w < c->geom.ways; w++) {
	cacheway* g_cw = c->Cont[w];
	
	g_cw->waylock->lock();
	bool present = g_cw->lookup(addr, dmap, 0, delay);
	if(present) {
	  g_cw->Status[dmap] = cacheway::invalid;
	  CTRC(addr, printf("CACHE64 operate() -> invalidated line\n"));
	}
	g_cw->waylock->unlock();
      }
    }
  
  // call the method from the base class
  cache64::operate(cw, op, addr, dmap, delay);
} 

bool consistent_cache64::insert_line(cacheway* me,
				     u64_t addr, 
				     int dmap, 
				     u8_t* data,
				     sc_time &delay,
				     cache_miss_extension* cme,
				     cacheway::mesi_t ns) {
  
  bool ins = me->insert(addr, dmap, data, delay, cme, ns);

  // MP noticed this sequence of operations
  // C3 has cache line L1 that is in modified state
  // C0 wants line L1, hence L1 from C3 is sent to secondary storage
  // before C0 gets reply C2 also requsts line L1
  // line L1 is still in modified state as C0 hasn't recived reply so it sent to 
  // secondary storage (again)
  // strangely C2 gets reply before C0 
  // before C0 recieves reply C2 writes to L1 making whatever C0 recives invalid

   // This happens because we are using temporal decoupling where each thread
  // keeps its own local view of time and SystemC kernel keeps a single 
  // synchronized view of time therefore thread can run ahead in simulation
  // time until it needs to synchronize with another thread. Calls to wait()
  // function need to be inserted carefully for cycle accurate simulation
  // such that above case doesn't happen

  // Safety check to test if there is a core that modified the cache line
  // between cache line request and now.
  // For debugging only - no delay/power consumption to model.
  if(grp)
    for(int g = 0; g < grp->target_members; g++) {
      if(grp->Members[g] == this) continue;

      consistent_cache64* c = grp->Members[g];
      
      cacheway* cw = 0;
      for(int w = 0; w < c->geom.ways; w++) {
	cacheway* cw = c->Cont[w];
	cw->waylock->lock();

	bool present = cw->lookup(addr, dmap, 0, delay);
	if(present 
	   && cw->Status[dmap] == cacheway::mesi_t::modified 
	   && ns == cacheway::mesi_t::exclusive) 
	  {
	    CTRC(addr, printf("CACHE64 insert_line() -> remote invalidation\n"));
	    me->Status[dmap] = cacheway::mesi_t::invalid;
	    cw->waylock->unlock();
	    return false;
	  }
	cw->waylock->unlock();
      }
    }

  return ins;
}

void consistent_cache64::line_invalidation(u64_t addr, 
					   int loffset, 
					   int dmap, 
					   cache_miss_extension* ext, 
					   sc_time& delay) {
  // check to see if other caches in the consistent group have this line
  // if they have it, then they need to be invalidated
  bool traverse = true;
  if(grp) 
    for(int g = 0; g < grp->target_members && traverse; g++) {
      sc_time max_lookup_delay = SC_ZERO_TIME;
      cache64* c = grp->Members[g];

      // check first write buffer
      if(c->buf0) {
	c->buf0->wbl.lock();
	if(c->buf0->hit(addr, loffset)) {
	  c->buf0->wbl.unlock();
	  c->buf0->clean(max_lookup_delay, ext);
	  break;
	}
	else c->buf0->wbl.unlock();
      }
      for(int w = 0; w < c->geom.ways && traverse; w++) {
	// check to see if the line exists
	cacheway* cw = c->Cont[w];
	cw->waylock->lock();
	bool present = cw->lookup(addr, dmap, 0, max_lookup_delay);
	//printf("%s: check grp member %i, present=%i\n", name(), g, present);
	if(present) 
	  switch(cw->Status[dmap]) {
	  case cacheway::modified: 
	    cw->Status[dmap] = cacheway::invalid;
	    cw->waylock->unlock();
	    cw->clean(max_lookup_delay, dmap, ext, true);
	    traverse = false;
	    break;
	  case cacheway::shared:
	  case cacheway::exclusive: 
	    cw->Status[dmap] = cacheway::invalid;
	    cw->waylock->unlock();
	    break;
	  case cacheway::owned:
	    cw->Status[dmap] = cacheway::invalid;
	    cw->waylock->unlock();
	    cw->clean(max_lookup_delay, dmap, ext, true);
	    break;
	  default:
	    assert(0);
	  }
	else
	  cw->waylock->unlock();
      }
    }
  
  // done
}


TENOS_KIND_DEFINITION(cache64)

// eof
