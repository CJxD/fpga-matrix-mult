// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2014 M Puzovic + D J Greaves

#ifndef __SCU_H__
#define __SCU_H__

#include "tenos.h"
#include "tlm.h"
#include "tlm_utils/multi_passthrough_initiator_socket.h"
#include "tlm_utils/multi_passthrough_target_socket.h"
#include "ccache.h"

#include <atomic>

#ifdef TLM_POWER3
#include "tlm_power.h"
using namespace sc_pwr;
#define POWER3(X) X
#else
typedef tlm::tlm_base_protocol_types PW_TLM_TYPES;
typedef tlm::tlm_generic_payload PW_TLM_PAYTYPE;
typedef int pw_customer_acct; /* use a simple int as a placeholder */
#define PW_TLM3(X)
#define POWER3(X)
#endif

#define SCUTRC(ADDR, X)\
{\
  if(traceregions && traceregions->check(ADDR, TENOS_TRACE_SNOOP)) {  \
    cout << sc_time_stamp() << " - ";\
    printf("tid=%d-%s (0x%lx): ", sc_get_current_process_b()->proc_id, name(), ADDR);\
    X;\
  }\
}


class scu : public sc_module
#ifdef TLM_POWER3
  , public pw_module
#endif
{
  public:
    void init();

    scu(sc_module_name name);

    tlm_utils::multi_passthrough_target_socket<scu, 64, PW_TLM_TYPES>  target_socket;
    tlm_utils::multi_passthrough_initiator_socket<scu, 64, PW_TLM_TYPES> *snoop_socket;
    tlm_utils::multi_passthrough_initiator_socket<scu, 64, PW_TLM_TYPES> *initiator_socket;

    void b_transport(
        int id, 
        PW_TLM_PAYTYPE &trans, 
        sc_time &delay);

    tlm::tlm_sync_enum nb_transport_bw(
        int id,
        PW_TLM_PAYTYPE& trans,
        tlm::tlm_phase& phase,
        sc_time& delay);

    typedef struct stats {
      public:
        std::atomic<u32_t> snoop_requests;
        std::atomic<u32_t> nonsnoop_requests;
        std::atomic<u32_t> total_requests;
        std::atomic<u32_t> secondary_read;
        std::atomic<u32_t> secondary_write;
        
        stats() { 
            reset(); 
        }

        void reset() {
            snoop_requests.store(0);
            nonsnoop_requests.store(0);
            total_requests.store(0);
            secondary_read.store(0);
            secondary_write.store(0);
        }
        
    } stats_t;

    void stat_report(const char *msg, FILE *fd, bool reset);

    traceregion *traceregions;

    void snooped_target(bool is_snooped) {
        snooped_targets.push_back(is_snooped);
    }        

  protected:
    POWER3(PW_TLM3(sc_pwr::tlm_bit_transition_tracker primary_bus_tracker)); // Used if counting wire transitions. 
    POWER3(PW_TLM3(sc_pwr::tlm_bit_transition_tracker secondary_bus_tracker)); // Used if counting wire transitions. 

  private:
    int snooped_id_from_target(int target) {
        int count = -1;
        for(int i = 0; i < snooped_targets.size(); i++) {
            if(snooped_targets.at(i)) {
                count++;
                
                if(i == target)
                    return count;
            }
        }
        // we should not reach htis
        assert(0);
    }


    void secondary_operation(
	int id,
        u64_t addr,
	PW_TLM_PAYTYPE& otrans,
	u8_t* data,
	bool read, 
	sc_time& delay);

    ccache_state_mm_t snoop_state_mm;
    sc_event** wait_events;
    std::atomic<u64_t>* served_requests;
    std::atomic<u32_t> read_locked_tid;

    stats_t* m_stats;

    std::vector<bool> snooped_targets;

    sc_time* snoop_delay;


};

#endif
