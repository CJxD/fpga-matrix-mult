// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2014 M Puzovic + D J Greaves

#include "tenos.h"
#if TRACECOMM
#include "proc_id_extension.h"
#endif
#include "ccache.h"

#include <algorithm>

using namespace sc_core;
using namespace std;
using namespace tlm;
using namespace tlm_utils;

#if TRACECOMM
extern int g_cores;
extern int g_tracecomm;
#endif

ccache::way::way(ccache* _parent, u8_t _way, u32_t _columns)
    : m_parent(_parent),
      m_way(_way)
{
    char name[64];

    // cache lines
    snprintf(name, 64, "Data_%d", m_way);
    m_data = new smallram<u8_t>(name, 
                                m_parent->m_geometry.linesize / sizeof(u8_t),
                                _columns,
                                m_parent);

    // tags
    snprintf(name, 64, "Tags_%d", m_way);
    m_tags = new smallram<u64_t>(name,
                                 1,
                                 _columns,
                                 m_parent);

    // status of lines
    m_status = (atomic<u64_t>*)malloc(_columns*(sizeof(atomic<u64_t>)));
    for(int i = 0; i < _columns; i++)
        // all zeros
        m_status[i] = 0;

#if TRACECOMM    
    // owner of the line
    m_owner = (u32_t*)malloc(_columns*(sizeof(u32_t)));
    for(int i = 0; i < _columns; i++)
      m_owner[i] = -1; // no owner
#endif    
}

ccache::state_t ccache::way::lookup(
    u64_t addr,
    u64_t index, 
    u64_t tag, 
    u8_t** cline, 
    u64_t& ts, 
    sc_time& delay) {
    ts = m_status[index].load();

    state_t clinestate = (state_t)((ts >> 1) & 0x7);

    POWER3(m_parent->log_energy_use(m_tags->m_read_energy_op)); 
    //delay = max(delay, m_tags->m_sr_latency);
    delay += m_tags->m_sr_latency;

#if 0
    CWTRC(addr, printf("CCACHE::WAY::lookup() -> address 0x%lx in state %d, tag %lx -- ts %ld\n",
                       addr, clinestate, (*m_tags)[index], ts));
#endif

    state_t ret = Invalid;
    if(clinestate != Invalid && (*m_tags)[index] == tag) {
        // energy access is going to be access
        // for the whole of cache line
        POWER3(m_parent->log_energy_use(m_data->m_read_energy_op));
	//delay = max(delay, m_data->m_sr_latency);
	delay += m_data->m_sr_latency;
	  
        *cline = m_data->readp(index);

        ret = clinestate;
    }

    return ret;
}

u64_t ccache::way::incts(u64_t oldts, state_t s) {
    return (((oldts >> 4) + 1) << 4) | (s << 1);
}

u64_t ccache::way::incts(u64_t oldts) {
    return (((oldts >> 4) + 1) << 4) | (oldts & 0xE);
}

bool ccache::way::acquirets(u64_t &oldts, u64_t index) {
    oldts &= ~0x1;
    if(!m_status[index].compare_exchange_strong(oldts, (oldts|0x1)))
        return false;

    oldts |= 1;
    return true;
}

bool ccache::way::acquirets(u64_t &oldts, u64_t index, state_t s) {
    u64_t newts = (((oldts >> 4) + 1) << 4) | (s << 1);
    newts |= 0x1;
    oldts &= ~0x1;
    if(!m_status[index].compare_exchange_strong(oldts, newts))
        return false;

    oldts = newts;
    return true;
}


bool ccache::way::updatets(u64_t index, u64_t &oldts, u64_t newts) {
    u64_t ts = oldts & ~0x1;
    return m_status[index].compare_exchange_strong(ts, newts);
}

bool ccache::way::releasets(u64_t index, u64_t oldts, u64_t newts) {
    return m_status[index].compare_exchange_strong(oldts, newts);
}

bool ccache::way::clean(
    u64_t addr,
    u64_t index,
    u64_t tag,
    state_t olds, 
    sc_time &delay,
    int rsize) {

    assert((*m_tags)[index] == tag);
    
    ccache_state_extension* c_ext = NULL;
    if(m_parent->snooped_socket) {
        c_ext = new ccache_state_extension;
        c_ext->state = olds;
    }
    u8_t* data = m_data->readp(index);
    m_parent->trans_to_secondary(
        c_ext,
        WriteBack,
        m_parent->m_geometry.linesize,
        addr,
        data,
        false,
        delay,
	rsize);
    
    if(c_ext)
        delete c_ext;

    return true;
}

u64_t ccache::way::rebuild_address(
    u64_t index) {
    u64_t oaddr = 0;
    oaddr |= (((*m_tags)[index] << (m_parent->m_geometry.indexb + m_parent->m_geometry.boffsetb)) 
              | (index << (m_parent->m_geometry.boffsetb)));

    return oaddr;
}

bool ccache::way::insert(
    u64_t addr, 
    u64_t index, 
    u64_t tag, 
    u8_t* cline, 
    state_t news,
    state_t olds,
    sc_time& delay,
    int rsize) {

    // check to see if line is dirty and tag is
    // different then one we want to insert
    // in the case we have a conflict and need to write
    // data to secondary storaga
    if((olds == UniqueDirty) && (*m_tags)[index] != tag) {
        CWTRC(addr, printf("CCACHE::WAY::insert() -> CONFLICT for address 0x%lx: expected tag 0x%lx != found tag 0x%lx\n",
                          addr, tag, (*m_tags)[index]));

        // rebuild the address
        u64_t oaddr = rebuild_address(index);
        m_parent->m_stats.conflicts_evictions.fetch_add(1);
#ifdef CMISSTY
        m_parent->m_stats.caddrs.insert(oaddr);
#endif
        clean(oaddr, index, (*m_tags)[index], olds, delay, rsize);
    }


    // write tags
    POWER3(m_parent->log_energy_use(m_tags->m_write_energy_op));
    m_tags->write(index, 1, &tag);

    // write data
    POWER3(m_parent->log_energy_use(m_data->m_write_energy_op));
    memcpy(m_data->readp(index), cline, m_parent->m_geometry.linesize);

    CWTRC(addr, printf("CCACHE::WAY::insert() -> inserted for address %lx line with state %d at index %ld and tag %lx\n",
                       addr, news, index, tag));


    return true;
}

ccache::ccache( // constructor
    sc_module_name _name,
    u32_t _myid, 
    u32_t _addrsize,
    u32_t _size,
    u32_t _linesize,
    u8_t _ways,
    bool snooped)
  : sc_module(_name),
    myid(_myid), 
    lru(0),
#ifdef TLM_POWER3
#if PW_TLM_PAYLOAD > 0
    primary_bus_tracker(this),
    secondary_bus_tracker(this),
#endif
#endif
    m_peq(this, &ccache::peq_cb),
    m_active(true)
{

    // cache geometry
    m_geometry.addrsize = _addrsize;
    m_geometry.size = _size;
    m_geometry.linesize = _linesize;
    m_geometry.ways = _ways;

    u32_t cache_line_num = m_geometry.size / m_geometry.linesize;
    u32_t cache_lines_per_way = cache_line_num / m_geometry.ways;

    m_geometry.boffsetm = m_geometry.linesize - 1;
    m_geometry.boffsetb = bound_log2(m_geometry.linesize);
    m_geometry.indexm = cache_lines_per_way - 1;
    m_geometry.indexb = bound_log2(cache_lines_per_way);
    m_geometry.tagm = pow(2, m_geometry.addrsize * 8 - m_geometry.indexb - m_geometry.boffsetb) - 1;

    // crate cache ways
    m_ways = (way**) malloc(m_geometry.ways * sizeof(way*));
    for(u8_t i = 0; i < m_geometry.ways; i++) {
        m_ways[i] = new way(this, i, cache_lines_per_way);
    }

#if TRACECOMM
    m_total_reads = 0;

    m_hit_reads = (u64_t*)malloc(g_cores*sizeof(u64_t));
    for(int i = 0; i < g_cores; ++i)
      m_hit_reads[i] = 0;
    
    m_comm_reads = (u64_t*)malloc(g_cores*sizeof(u64_t));
    for(int i = 0; i < g_cores; ++i)
      m_comm_reads[i] = 0;
#endif    


    target_socket.register_b_transport(this, &ccache::b_transport);
    if(snooped) {
        snooped_socket = new multi_passthrough_target_socket<ccache, 64, PW_TLM_TYPES>;
        snooped_socket->register_nb_transport_fw(this, &ccache::nb_transport_fw);
    }

    //Tenos CSV report
  new tenos_report_item(name(), "snoop_evictions", &m_stats.snoop_evictions);
  new tenos_report_item(name(), "conflicts_evictions", &m_stats.conflicts_evictions);
  new tenos_report_item(name(), "reads", &m_stats.reads);
  new tenos_report_item(name(), "read_hits", &m_stats.read_hits);
  new tenos_report_item(name(), "read_miss", &m_stats.read_miss);
  new tenos_report_item(name(), "writes", &m_stats.writes);
  new tenos_report_item(name(), "write_hits", &m_stats.write_hits);
  new tenos_report_item(name(), "write_miss", &m_stats.write_miss);

#ifdef CMISSTY
  new tenos_report_item(name(), "capacity", &m_stats.capacity);
#endif

    // Energy
#ifdef TLM_POWER3
    std_energy_op = pw_energy((double) (0.1 * 64), pw_energy_unit::PW_pJ); 
    // based on: just made up!
#endif

}

// Simple invalidation
bool ccache::invalidate() {
  for(int i = 0; i < m_geometry.ways; i++) {
    way* w = m_ways[i];
    for(int j = 0; j <= m_geometry.indexm; j++) {
      w->m_status[j] = 0;
    }
  }
}

bool ccache::trans_to_secondary(
    ccache_state_extension* &cse,
    transaction type,
    u32_t length,
    u64_t addr,
    u8_t* &data,
    bool is_read,
    sc_time& delay,
    int rsize,
    int pid) {

    PW_TLM_PAYTYPE trans;// = ccache_state_mm.allocate(); 
    if(cse) {
        cse->trans = type;
        //trans.set_auto_extension<ccache_state_extension>(cse);
        trans.set_extension(cse);
    }
    //trans->acquire();

    trans.set_data_length(length);
    trans.set_byte_enable_length(0);
    trans.set_byte_enable_ptr(0);
    trans.set_streaming_width(rsize);
#if TRACECOMM    
    proc_id_extension* pid_ext = NULL;
#endif    
    if(is_read) {
#if TRACECOMM
      pid_ext = new proc_id_extension;
      pid_ext->proc_id = pid;
      pid_ext->length = rsize;
      trans.set_extension(pid_ext);
#endif	
      trans.set_read();
    } else {
        trans.set_write();
    }
    
    // TODO: are there any examples where it might
    // not be possible for secondary storage to
    // read linesize in one go??
    trans.set_response_status(TLM_INCOMPLETE_RESPONSE);
    trans.set_address(addr);
    trans.set_data_ptr(data);

    sc_time secondary_delay = SC_ZERO_TIME;
    initiator_socket[0]->b_transport(trans, secondary_delay);
    delay += secondary_delay;
    if (trans.is_response_error()) {
        //trans->release();
        
        // we should not receive error
        // transaction from secondary
        assert(0);
        
        return false;
    } 

    //trans->release();
    trans.clear_extension<ccache_state_extension>(cse);
#if TRACECOMM    
    if(pid_ext != NULL) {
      trans.clear_extension<proc_id_extension>(pid_ext);
      delete pid_ext;
    }
#endif    
    return true;
}

void ccache::b_transport(int id, PW_TLM_PAYTYPE &trans, sc_time &delay) {
    // check to see if cache is active, if it is not
    // active then simply forward transaction to the next
    // unit in line

    if(!m_active) {
        return initiator_socket->b_transport(trans, delay);
    }

    // assumption here is that memory has been
    // allocated for data where transactions was
    // created and that size is at least
    // equal to number of bytes in data_length
    // otherwise we get segmentation fault
    u8_t* data_ptr = trans.get_data_ptr();
    u32_t data_length = trans.get_data_length();

    u64_t addr = trans.get_address();

    // offset within the cache line
    u64_t offset = addr & m_geometry.boffsetm;
    // index within the cache way
    u64_t index = (addr >> m_geometry.boffsetb) & m_geometry.indexm;
    // tag
    u64_t tag = (addr >> (m_geometry.boffsetb + m_geometry.indexb));

    //sc_time start_time = delay + sc_time_stamp();
    pw_customer_acct *customer_acct = 0;

    PW_TLM3(customer_acct = trans.get_customer_id());  
    llsc_extension* linked = 0;
    trans.get_extension(linked); 

    ccache_state_extension* c_ext = 0;
    trans.get_extension(c_ext);
    if(c_ext == 0 && snooped_socket) {
        c_ext = new ccache_state_extension;
        trans.set_extension(c_ext);
    }

#if TRACECOMM
    int proc_id = -1;
    proc_id_extension* pid_ext = 0;
    trans.get_extension(pid_ext);
    if(pid_ext != 0)
      proc_id = pid_ext->proc_id;
#endif
    

    // what is command
    tlm_command cmd = trans.get_command();

    if(cmd == TLM_WRITE_COMMAND) 
      {
	//printf("Foreign write %llx\n", addr);
	foreign_write(addr);
      }
    if (UNCACHED_ADDRESS_SPACE64(addr) || linked) {
        tlm_response_status rstatus = TLM_OK_RESPONSE;

        if(cmd == TLM_READ_COMMAND && linked) {
            m_stats.reads.fetch_add(1);
            m_stats.llsc_reads.fetch_add(1);

            while(true) {
                bool success = locked_op(
                    addr,
                    offset, 
                    index, 
                    tag, 
                    data_ptr, 
                    data_length, 
                    c_ext, 
                    linked,
                    delay,
		    trans.get_streaming_width());

                if(success) {
                    break;
                }

                CTRC(addr, printf("CCACHE::b_transport() -> FAILED LOCKED READ for address 0x%lx due to CONTENTION\n",
                                  addr));
                m_stats.read_contentions.fetch_add(1);

                // we shouldn't hit this, in case of hitting this
                // assertion let MP know how to reproduce it
                assert(0);
            }
            CTRC(addr, printf("CCACHE::b_transport() -> SUCCESSFUL LOCKED READ for address 0x%lx\n",
                              addr));
            // forward transaction
            if(c_ext)
                c_ext->trans = Read;
            initiator_socket->b_transport(trans, delay);

        } else if(cmd == TLM_WRITE_COMMAND && linked) {
            while(true) {
                bool success = locked_op(
                    addr, 
                    offset, 
                    index, 
                    tag, 
                    data_ptr, 
                    data_length, 
                    c_ext, 
                    linked,
                    delay,
		    trans.get_streaming_width());

                if(success) {
                    break;
                }

                CTRC(addr, printf("CCACHE::b_transport() -> FAILED LOCKED READ for address 0x%lx due to CONTENTION\n",
                                  addr));



                // we shouldn't hit this, in case of hitting this
                // assertion let MP know how to reproduce it
                assert(0);
            }
            if(c_ext)
                c_ext->trans = Write;
            initiator_socket->b_transport(trans, delay);            
        } else {
            if(c_ext)
                c_ext->trans = (cmd == TLM_READ_COMMAND) ? Read : Write;
            
            initiator_socket->b_transport(trans, delay);

            return;
        }

        return;
    }

    POWER3(PW_TLM3(pw_agent_record l_agent =
                   trans.pw_log_hop(
                       this,  
                       (cmd==tlm::TLM_READ_COMMAND ? 
                        PW_TGP_DATA: PW_TGP_NOFIELDS) 
                       | PW_TGP_ACCT_CKP,  
                       &primary_bus_tracker))); // Read or write but not passthrough.

    switch(cmd) {
      case TLM_READ_COMMAND: 
	{
          m_stats.reads.fetch_add(1);
	  while(true) {
	    bool success = read(
                addr, 
                offset, 
                index, 
                tag, 
                data_ptr, 
                data_length, 
                c_ext, 
                delay,
		trans.get_streaming_width()
#if TRACECOMM
		, proc_id
#endif
		);

            if(success)
                break;

            CTRC(addr, printf("CCACHE::b_transport() -> FAILED READ for address 0x%lx due to CONTENTION\n",
                              addr));
            m_stats.read_contentions.fetch_add(1);
	  }
#if TRACECOMM
	  if(g_tracecomm) {
	    m_total_reads += trans.get_streaming_width();
	  }
#endif	  
	  break;
	}
      case TLM_WRITE_COMMAND: 
        {
            u8_t* benable_ptr = trans.get_byte_enable_ptr();
            u32_t benable_length = trans.get_byte_enable_length();

            m_stats.writes.fetch_add(1);
            while(true) {
                bool success = write(
                    addr, 
                    offset, 
                    index, 
                    tag, 
                    data_ptr, 
                    data_length, 
                    benable_ptr, 
                    benable_length, 
                    c_ext,
                    delay,
		    trans.get_streaming_width());
                if(success) 
                    break;
                
                CTRC(addr, printf("CCACHE::b_transport() -> FAILED WRITE for address 0x%lx due to CONTENTION\n",
                                  addr));

                m_stats.write_contentions.fetch_add(1);
            }

            break;
        }
      default:
        // unknown command
        assert(0);
    }

    trans.set_response_status(tlm::TLM_OK_RESPONSE);
  
    // end of transaction: pass down dynamic energy use.
    POWER3(log_energy_use(std_energy_op));
#if PW_TLM_PAYLOAD > 0 
    POWER3(l_agent.record_energy_use(m_running_op_energy, &trans));
#else
    POWER3(record_energy_use(m_running_op_energy, &trans));
#endif
    POWER3(m_running_op_energy = PW_ZERO_ENERGY);     
}

// TODO: mp need to add power and timing calculation
void ccache::secondary_lookup(
    u64_t addr,
    u8_t* &clinep,
    ccache_state_extension* &cse,
    sc_time& delay,
    int rsize,
    int pid) {

    PW_TLM_PAYTYPE* trans;
    u64_t line_addr = addr & ~(m_geometry.linesize - 1);
    CTRC(addr, printf("CCACHE::secondary_lookup() -> doing a secondary lookup for address 0x%lx; line address 0x%lx\n",
		      addr, line_addr));

    trans_to_secondary(
        cse,
        ReadShared,
        m_geometry.linesize,
        line_addr,
        clinep,
        true,
        delay,
	rsize,
        pid);
}

ccache::way* ccache::lookup(
    u64_t addr, 
    u64_t index, 
    u64_t tag, 
    u8_t** clinep, 
    u64_t& oldts, 
    state_t& retlstate,
    sc_time& delay) {

    retlstate = Invalid;
    for(int i = 0; i < m_geometry.ways; i++) {
        way* w = m_ways[i];
        state_t lstate = w->lookup(addr, index, tag, clinep, oldts, delay);
        if(lstate != Invalid) {
            CTRC(addr, printf("CCACHE::lookup() -> hit for address 0x%lx in way %d with cache line state %d\n", 
                              addr, i, lstate));
	    CTRC(addr, printf("CACHE::lookup() -> content of cache line is: "); \
		 for(int i = 0; i < m_geometry.linesize; i++)		\
		   printf("%x ", (*clinep)[i]);				\
		 printf("\n"));

            retlstate = lstate;
            return w;
        }
    }

    return NULL;

}

bool ccache::warm_write(
    ccache::way* cway,
    u64_t addr,
    u64_t index,
    u64_t offset,
    u8_t* &data,
    u32_t dlength,
    u8_t* benable,
    u32_t blength,
    u64_t& oldts,
    bool update) {

    u64_t line_addr = addr & ~(m_geometry.linesize - 1);
    if(data == NULL)
        assert(0);

    // prepare data
    u8_t* cdata = cway->m_data->readp(index, offset);
    if(benable) {
        for(int i = 0; i < blength; i++)
            if(!benable[i])
                data[i] = cdata[i];
    }

    CTRC(addr, 
         printf("CCACHE::warm_write() -> about to write for address 0x%lx ; line address 0x%lx: ",
                addr, line_addr);                                \
         for(int i = 0; i < dlength; i++)                               \
             printf("%x ", data[i]);                                    \
         printf("\n"));

    // write data to cache line but no need to change
    // the state of the line
    if(!cway->acquirets(oldts, index))
        return false;
    memcpy(cdata, data, dlength);

    u64_t newts = update ? cway->incts(oldts, UniqueDirty) : cway->incts(oldts);
    CTRC(addr, 
         printf("CCACHE::warm_write() -> succesfull warm write 0x%lx; line address 0x%lx; newts = %ld\n",
                addr, line_addr, newts);                                      \
         u8_t* toprint = cway->m_data->readp(index, 0);                 \
         CTRC(addr, printf("CACHE::warm_write() -> content of cache line now is: "); \
              for(int i = 0; i < m_geometry.linesize; i++)		\
                  printf("%02x ", toprint[i]);				\
              printf("\n")));


    assert(cway->releasets(index, oldts, newts));

    return true;
}

bool ccache::cold_write(
    u64_t addr,
    u64_t offset, 
    u64_t index,
    u64_t tag, 
    u8_t* data,
    u32_t dlength,
    u8_t* benable,
    ccache_state_extension* &cse, 
    sc_time& delay,
    int rsize) {

    u64_t line_addr = addr & ~(m_geometry.linesize - 1);
    CTRC(addr, printf("CCACHE::cold_write() -> doing a secondary write for address 0x%lx; line address 0x%lx\n",
		      addr, line_addr));
    CTRC(addr, printf("CCACHE::cold_write() -> planning to write data: "); \
         for(int i = 0; i < dlength; i++)                               \
             printf("%02x ", data[i]);                                  \
         printf("\n"));

    CTRC(addr, if(benable) {                                            \
            printf("CCACHE::cold_write() -> bytenable: ");              \
            for(int i = 0; i < dlength; i++)                            \
                printf("%02x ", benable[i]);                            \
            printf("\n");                                               \
        });



    // we didn't find line in the cache so need to
    // do a secondary write first and bring the
    // line back to us
    
    // send either ReadUnique or
    // MakeUnique transaction to secondary storage
    // depending whether we are writing a full cache line
    // or only partial. With partial cache line we bring
    // data back with full we do not have to
    transaction ttype;
    if(dlength == m_geometry.linesize)
        ttype = MakeUnique;
    else
        ttype = ReadUnique;

    u8_t newdata[m_geometry.linesize];
    u8_t* topass = &newdata[0];
    // allocate transaction
    trans_to_secondary(
        cse,
        ttype,
        m_geometry.linesize,
        line_addr,
        topass,
        true,
        delay,
	rsize);


    for(int i = 0; i < m_geometry.linesize; i++) {
        if(i >= offset && i < (offset + dlength)) {
            if(!benable || (benable && benable[i-offset]))
                newdata[i] = data[i-offset];
        }
    }

    u64_t oldts;
    way* cway = NULL;
    bool success = insert(addr, index, tag, &newdata[0], UniqueDirty, oldts, cway, delay, rsize);
    if(success) {
        CTRC(addr, 
             printf("CCACHE::cold_write() -> succesfull cold write 0x%lx; line address 0x%lx\n",
                addr, line_addr);                                       \
         CTRC(addr, printf("CCACHE::cold_write() -> content of cache line now is: "); \
              for(int i = 0; i < m_geometry.linesize; i++)		\
                  printf("%02x ", newdata[i]);				\
              printf("\n")));
    }


    return success;
}

bool ccache::write_locked(
    u64_t addr,
    u64_t offset, 
    u64_t index,
    u64_t tag,
    u8_t* data,
    u32_t dlength,
    u8_t* benable,
    u32_t blength,
    ccache_state_extension* &cse, 
    sc_time& delay) {
    CTRC(addr,
         printf("CCACHE::write_locked() -> for address 0x%lx; offset %ld, index %ld, tag 0x%lx, dlength=%d\n",
                addr, offset, index, tag, dlength));

    if(benable) {
        // if byte enabled then length
        // must be the same as data length
        assert(dlength == blength);
    }

    // line must be present in the cache and state must be UniqueDirty
    u8_t* clinep = 0;
    u64_t oldts;
    state_t clstate;
    way* cway = lookup(addr, index, tag, &clinep, oldts, clstate, delay);

    if(clstate != UniqueDirty) {
        CTRC(addr,
             printf("CCACHE::write_locked() -> FAILED LOCKED WRITE for address 0x%lx; offset %ld, index %ld, tag 0x%lx because state of line is %d != UniqueDirty\n",
                    addr, offset, index, tag, clstate));
        return false;
    }
    
    bool success = warm_write(cway, addr, index, offset, data, dlength, benable, blength, oldts, false);
    if(success)
        m_stats.write_hits.fetch_add(1);

    return success;
}

bool ccache::write(
    u64_t addr,
    u64_t offset, 
    u64_t index,
    u64_t tag,
    u8_t* data,
    u32_t dlength,
    u8_t* benable,
    u32_t blength,
    ccache_state_extension* &cse, 
    sc_time& delay,
    int rsize) {
    CTRC(addr,
         printf("CCACHE::write() -> for address 0x%lx; offset %ld, index %ld, tag 0x%lx, dlength=%d\n",
                addr, offset, index, tag, dlength));

    if(benable) {
        // if byte enabled then length
        // must be the same as data length
        assert(dlength == blength);
    }

    // 1. try to see if line is already present in the cache
    u8_t* clinep = 0;
    u64_t oldts;
    state_t clstate;
    way* cway = lookup(addr, index, tag, &clinep, oldts, clstate, delay);

    bool success;
    
    switch(clstate) {
      case UniqueDirty: {
          success = warm_write(cway, addr, index, offset, data, dlength, benable, blength, oldts, false);
          if(success)
              m_stats.write_hits.fetch_add(1);

	  break;
      }
      case UniqueClean: {
          success = warm_write(cway, addr, index, offset, data, dlength, benable, blength, oldts, true);
          if(success)
              m_stats.write_hits.fetch_add(1);

	  break;
      }
      case SharedClean: {
	  // invalidate first all lines in snooping caches
          u64_t line_addr = addr & ~(m_geometry.linesize - 1);
          u8_t* edatap = NULL;
          trans_to_secondary(
              cse,
              MakeInvalid,
              0,
              line_addr,
              edatap,
              false,
              delay,
	      rsize);

	  success = warm_write(cway, addr, index, offset, data, dlength, benable, blength, oldts, true);
          if(success)
              m_stats.write_hits.fetch_add(1);

	  break;
      }
      case Invalid: {
	bool success = cold_write(addr, offset, index, tag, data, dlength, benable, cse, delay, rsize);
          if(success) {
#ifdef CMISSTY
              u64_t line_addr = addr & ~(m_geometry.linesize - 1);
              if(m_stats.alladdrs.find(line_addr) != m_stats.alladdrs.end()) {
                  m_stats.alladdrs.insert(line_addr);
                  m_stats.compulsory.fetch_add(1);
              } else if(m_stats.caddrs.find(line_addr) != m_stats.caddrs.end()) {
                  m_stats.conflict.fetch_add(1);
              } else {
                  m_stats.capacity.fetch_add(1);
              }
#endif
              m_stats.write_miss.fetch_add(1);
          }

          break;
      }
      case SharedDirty:
      {
          // TODO: not implemented (MOESI protocol)
          assert(0);
          break;
      }

    }

#if TRACECOMM    
    if(success && cway)
      cway->m_owner[index] = myid;
#endif
    
    return success;
}

ccache::way* ccache::lruw() {
    // LRU algorithm
    // TODO: consider whether we should implement some more algorithms
    lru = (lru == m_geometry.ways - 1) ? 0 : lru + 1;
    way* cway = m_ways[lru];

    return cway;
}

bool ccache::insert(
    u64_t addr,
    u64_t index,
    u64_t tag,
    u8_t* clinep,
    state_t news,
    u64_t& oldts, 
    way* &cway,
    sc_time& delay,
    int rsize) {

    cway = lruw();
    // as we are modifying internal state of the cache
    // we are only going to do that if we can get there first
    u64_t ts = cway->m_status[index].load();
    state_t olds = (state_t)((ts & 0xE) >> 1);
    if(!cway->acquirets(ts, index)) {
        return false;
    }

    cway->insert(addr, index, tag, clinep, news, olds, delay, rsize);

    u64_t newts = cway->incts(ts, news);
    // this must succeed otherwise we are going to have
    // inconsistent internal state because we did write 
    assert(cway->releasets(index, ts, newts));

    oldts = newts;

    return true;
}

bool ccache::locked_op(
    u64_t addr, 
    u64_t offset, 
    u64_t index, 
    u64_t tag, 
    u8_t* &destination,
    u32_t length,
    ccache_state_extension* &cse, 
    llsc_extension* &linked,
    sc_time& delay,
    int rsize) {
    
    CTRC(addr, 
	   printf("CCACHE::locked_op() -> for address 0x%lx; offset %ld, index %ld, tag 0x%lx\n",
		    addr, offset, index, tag));

    u64_t line_addr = addr & ~(m_geometry.linesize - 1);
    u8_t* clinep = 0;
    u64_t oldts;
    state_t clstate;
    way* cway;
    cway = lookup(addr, index, tag, &clinep, oldts, clstate, delay);
    
    bool clhit = (clinep != 0);

    if(clhit) {
        u64_t ts = cway->m_status[index].load();
        if(!cway->acquirets(ts, index)) 
            return false;
        
        if(clstate == UniqueDirty) {
            ccache_state_extension* c_ext = NULL;
            if(snooped_socket) {
                c_ext = new ccache_state_extension;
                c_ext->state = clstate;
            }
            
            trans_to_secondary(
                c_ext,
                WriteBack,
                m_geometry.linesize,
                line_addr,
                clinep,
                false,
                delay,
		rsize);
        }
            
        u64_t newts = cway->incts(ts, Invalid);
        assert(cway->releasets(index, ts, newts));
    }

    if(snooped_socket) {
        // Evict all lines
        ccache_state_extension* ncse = new ccache_state_extension;
        ncse->state = clstate;
        
        trans_to_secondary(
            ncse,
            Evict,
            m_geometry.linesize,
            line_addr,
            clinep,
            true,
            delay,
	    rsize);

        delete ncse;
    }

    // after we have done all evictions we can forward transaction
    // to secondary storage

    return true;
}


bool ccache::read(
    u64_t addr, 
    u64_t offset, 
    u64_t index, 
    u64_t tag, 
    u8_t* &destination,
    u32_t length,
    ccache_state_extension* &cse, 
    sc_time& delay,
    int rsize
#if TRACECOMM
    , int pid
#endif    
 ) {
    // 1. go through each way in a cache and try to
    // find if address is cached
    CTRC(addr, 
	   printf("CCACHE::read() -> for address 0x%lx; offset %ld, index %ld, tag 0x%lx\n",
		    addr, offset, index, tag));
    u8_t* clinep = 0;
    u64_t oldts;
    state_t clstate;
    way* cway;
    cway = lookup(addr, index, tag, &clinep, oldts, clstate, delay);
    
    u8_t sclinep[m_geometry.linesize];

    bool clhit = (clinep != 0);

    if(!clinep) {
        assert(cway == NULL && clstate == Invalid);
        // 2. we have a miss
        //    therefore we need to do a secondary lookup
        //    this secondary lookup can include snooping
        //    or just simply looking at the cache at the
        //    next level, where the request is sent
        //    depends on architectures
        clinep = &sclinep[0];
	secondary_lookup(addr,
			 clinep,
			 cse,
			 delay,
			 rsize
#if TRACECOMM
			 , pid
#endif			 
			 );
    
	CTRC(addr, 
	     printf("CCACHE::read() -> content of secondary lookup for address 0x%lx is: ", addr); \
	     for(int i = 0; i < m_geometry.linesize; i++)		\
	       printf("%02x ", clinep[i]);				\
	     printf("\n"));

	// 3. insert line to the cache
        state_t nstate = cse ? cse->state : UniqueClean;
        if(!insert(addr, index, tag, clinep, nstate, oldts, cway, delay, rsize)) {
            return false;
        }

    }
    else {
        if(cse)
            cse->state = UniqueClean;
    }

    assert(cway != NULL);
    if(!cway->acquirets(oldts, index)) {
        return false;
    }

    memcpy(destination, clinep + offset, length);

    u64_t newts = cway->incts(oldts);
    assert(cway->releasets(index, oldts, newts));

    if(clhit) {
        m_stats.read_hits.fetch_add(1);
#if TRACECOMM
	if(g_tracecomm && pid != -1) {
	  m_hit_reads[pid] += rsize;
	} 
#endif
    }
    else {
#ifdef CMISSTY
        u64_t line_addr = addr & ~(m_geometry.linesize - 1);
        if(m_stats.alladdrs.find(line_addr) == m_stats.alladdrs.end()) {
            m_stats.alladdrs.insert(line_addr);
            m_stats.compulsory.fetch_add(1);
        } else if(m_stats.caddrs.find(line_addr) != m_stats.caddrs.end()) {
            m_stats.conflict.fetch_add(1);
        } else {
            m_stats.capacity.fetch_add(1);
        }
#endif

        m_stats.read_miss.fetch_add(1);
    }

    return true;
}

void ccache::peq_cb(PW_TLM_PAYTYPE& trans, const tlm_phase& ph) {
    sc_time delay = SC_ZERO_TIME;

    ccache_state_extension* c_ext = 0;
    trans.get_extension(c_ext);
    assert(c_ext != 0);

    u64_t line_addr = trans.get_address();
    u64_t index = (line_addr >> m_geometry.boffsetb) & m_geometry.indexm;
    u64_t tag = (line_addr >> (m_geometry.boffsetb + m_geometry.indexb));

    CTRC(line_addr,
         printf("CCACHE::peq_cb() -> snoop request received from %d for line address 0x%lx; index %ld, tag 0x%lx\n -- transaction %d\n",
                c_ext->origin, line_addr, index, tag, c_ext->trans));
    
    switch(c_ext->trans) {
        case ReadShared: 
        case ReadUnique:
        case MakeUnique:
        case Evict:
        {
	    while(true) {
	        u8_t* clinep = 0;
		u64_t oldts;
		state_t clstate;
		way* cway = lookup(line_addr, index, tag, &clinep, oldts, clstate, delay);
		
		c_ext->state = clstate;

		if(clstate == Invalid) {
                    m_stats.snoop_miss.fetch_add(1);
		    break;
                }

		
		
		assert(cway != NULL);
		if(!cway->acquirets(oldts, index)) {
                    m_stats.snoop_contentions.fetch_add(1);
                    if(c_ext->trans == Evict) {
                        // this should never happen
                        // we should be able to lock the line
                        // if it happens let mp know how to reproduce it
                        assert(0);
                    }
                        
                    CTRC(line_addr, 
                         printf("CCACHE::peq_cb() -> snoop request CONTENTION for line address 0x%lx\n",
                                line_addr));

		    continue; // try again
                }

#if TRACECOMM
		if(g_tracecomm && cway->m_owner[index] == myid) {
		  m_comm_reads[(c_ext->origin/2)] +=
		    trans.get_streaming_width();
		}
#endif		
		
		memcpy(trans.get_data_ptr(), clinep, m_geometry.linesize);
	
		u64_t newts;
		if(c_ext->trans == ReadShared) {
                    m_stats.snoop_reads.fetch_add(1);
                    CTRC(line_addr, 
                         printf("CCACHE::peq_cb() -> snoop request changed state of line address 0x%lx to SharedClean\n",
                                line_addr));

                    newts = cway->incts(oldts, SharedClean);
                }
		else {
                    m_stats.snoop_evictions.fetch_add(1);
                    CTRC(line_addr, 
                         printf("CCACHE::peq_cb() -> snoop request changed state of line address 0x%lx to Invalid\n",
                                line_addr));
#if TRACECOMM
		    if(cway->m_owner[index] == myid)
		      cway->m_owner[index] = -1;
#endif		    
                    newts = cway->incts(oldts, Invalid);
                }
		assert(cway->releasets(index, oldts, newts));
                
                m_stats.snoop_hits.fetch_add(1);

		break;
	    }
	    break;
	}

        case MakeInvalid: {
	    way* cway = NULL;
	  
	    while(true) {
	        u8_t* clinep = 0;
		u64_t oldts;
		state_t clstate;
		cway = lookup(line_addr, index, tag, &clinep, oldts, clstate, delay);
		
		c_ext->state = clstate;
		if(clstate == Invalid) {
                    m_stats.snoop_miss.fetch_add(1);
		    break;
                }

                if(clstate != SharedClean) {
                    // between issuing MakeInvalid and this point
                    // the cache has changed this line from SharedClean
                    // (i.e it has written to it) and as result we
                    // cannot invlidate it. We are not going to do anything
                    // because write will fail thanks to our timestamp check
                    // as the cache that originated this transaction has been
                    // updated with new timestamp
                    m_stats.snoop_hits.fetch_add(1);
                    break;
                }

	        u64_t news = cway->incts(oldts, Invalid);
		if(!cway->updatets(index, oldts, news)) {
                    m_stats.snoop_contentions.fetch_add(1);
                    CTRC(line_addr, 
                         printf("CCACHE::peq_cb() -> snoop request CONTENTION for line address 0x%lx\n",
                                line_addr));

                    continue;
                }
                
                m_stats.snoop_hits.fetch_add(1);
                m_stats.snoop_evictions.fetch_add(1);
                CTRC(line_addr, 
                     printf("CCACHE::peq_cb() -> snoop request changed state of line address 0x%lx to Invalid\n",
                            line_addr));
                
		break;
	    }

#if TRACECOMM
	    if(cway != NULL && cway->m_owner[index] == myid)
	      cway->m_owner[index] == -1;
#endif	    

	    break;
	}
	  
    }
    tlm_phase nph = END_REQ;
    //cout << name() << ": delay after snooping is = " << delay << "\n";
    snooped_socket[0]->nb_transport_bw(trans, nph, delay);
}


tlm::tlm_sync_enum ccache::nb_transport_fw(
    int id,
    PW_TLM_PAYTYPE& trans,
    tlm_phase& ph,
    sc_time& delay) {

  //<<<<<<< HEAD
  sc_time new_delay = SC_ZERO_TIME;
  m_peq.notify(trans, ph, new_delay);

  // =======
  //    m_peq.notify(trans, ph);

  return tlm::TLM_ACCEPTED;
}

#if TRACECOMM
void ccache::comm_stat_report(FILE* fd) {
  if(fd != NULL) {
    fprintf(fd, "C%d,", myid);
    for(int i = 0; i < g_cores; ++i) {
      fprintf(fd, "%ld,", m_comm_reads[i]);
    }
    fprintf(fd, "\n");
  }
}

u64_t ccache::hit_reads(int pid) {
  return m_hit_reads[pid];
}
#endif



void ccache::stat_report(const char *msg, FILE *fd, bool reset) {
    if(fd) {
        fprintf(fd, "  Hits    Misses      Total       llsc    Contentions   READS %s\n", name());
        fprintf(fd, "-----------------------------------------------------\n");
        fprintf(fd, "%6ld%10d%11d%11d%15d\n",
                m_stats.read_hits.load(),
                m_stats.read_miss.load(),
                m_stats.reads.load(),
                m_stats.llsc_reads.load(),
                m_stats.read_contentions.load());

        fprintf(fd, "  Hits    Misses      Total       llsc    Contentions    Conflicts  WRITES %s \n", name());
        fprintf(fd, "------------------------------------------------------------------\n");
        fprintf(fd, "%6d%10d%11d%11d%15d%13d\n",
                m_stats.write_hits.load(),
                m_stats.write_miss.load(),
                m_stats.writes.load(),
                m_stats.llsc_writes.load(),
                m_stats.write_contentions.load(),
                m_stats.conflicts_evictions.load());

        fprintf(fd, "  Hits    Misses   Hit ratio   TOTALS  %s\n", name());
        fprintf(fd, "%6ld%10d%12.3f\n",
                m_stats.read_hits.load() + m_stats.write_hits.load(),
                m_stats.read_miss.load() + m_stats.write_miss.load(),
                (double)(m_stats.read_hits.load() + m_stats.write_hits.load()) / (double)(m_stats.reads.load()+m_stats.writes.load()));

#ifdef CMISSTY
        fprintf(fd, "\n");
        fprintf(fd, "  Compulsory    Capacity   Conflict   Total     %s\n", name());
        fprintf(fd, "%12d%12d%11d%8d\n",
                m_stats.compulsory.load(),
                m_stats.capacity.load(),
                m_stats.conflict.load(),
                (m_stats.read_miss.load() + m_stats.write_miss.load()));
#endif
        if(snooped_socket) {
            fprintf(fd, "  Hits    Misses      Reads       Evictions    Contentions   SNOOPS %s \n", name());
            fprintf(fd, "----------------------------------------------------------\n");
            fprintf(fd, "%6d%10d%11d%16d%15d\n",
                    m_stats.snoop_hits.load(),
                    m_stats.snoop_miss.load(),
                    m_stats.snoop_reads.load(),
                    m_stats.snoop_evictions.load(),
                    m_stats.snoop_contentions.load());
        }
    }

    if(reset) {
        m_stats.reset();
    }
}

PW_TLM_PAYTYPE* ccache_state_mm_t::allocate() {
  PW_TLM_PAYTYPE* ptr;

  lck.lock();
  if(free_list) {
    ptr = free_list->trans;
    empties = free_list;
    free_list = free_list->next;
  }
  else {   
    ptr = new (PW_TLM_PAYTYPE)(this);
  }
  lck.unlock();

  if(ptr->get_ref_count() < 0)
    while(ptr->get_ref_count() != 0)
      ptr->acquire();

  assert(ptr->get_ref_count() == 0);

  return ptr;
}


void ccache_state_mm_t::free(PW_TLM_PAYTYPE* trans) 
{
  // do not need to release extension as they are all allocated on stack
  // so clearing should be enough
  ccache_state_extension* cme = 0;
  trans->get_extension(cme);
  if(cme) {
    delete cme;
    trans->clear_extension<ccache_state_extension>(cme);
  }

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
