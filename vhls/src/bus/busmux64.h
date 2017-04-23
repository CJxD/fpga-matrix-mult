// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2009-11 D J Greaves
// University of Cambridge, Computer Laboratory.
// ACS P35 SoC D/M Classes - $Id: busmux64.h,v 1.6 2011/08/22 13:30:19 djg11 Exp $
// Generic payload blocking transport router for modelling contention.

#ifndef BUSMUX64_H
#define BUSMUX64_H

;

#include "systemc.h"


#include "tlm.h"
#include "tlm_utils/multi_passthrough_initiator_socket.h"
#include "tlm_utils/multi_passthrough_target_socket.h"
#include "prazor.h"
#include "tenos.h"

#include <queue>


#include <boost/icl/discrete_interval.hpp>
#include <boost/icl/interval_map.hpp>

#define MAX_TARGETS 64

#define BUSMUX64_BIND(BUS, TARGET, START, END)	\
  BUS.inita_socket.bind(TARGET); \
  BUS.bind((START),END)

class busmux64: 
  public sc_module
#ifdef TLM_POWER3
, public pw_module
#endif
{
  sc_mutex ml;
  sc_event/*_queue*/ wake_event[MAX_TARGETS];
  std::queue<int> requests[MAX_TARGETS];
  struct stats_t
  {
    int operations;
    int contended_operations;
    sc_time service_time;
    sc_time response_time;
    stats_t() { reset(); }
    void reset()
    {
      operations = 0;
      contended_operations = 0;
      response_time = SC_ZERO_TIME;
      service_time = SC_ZERO_TIME;
    }
    void stat_report(busmux64 *p, const char *msg, FILE *fd, bool resetf=false);
    void log(sc_time start_q, sc_time start_op, sc_time end_op)
    {
      response_time += (end_op - start_q);
      service_time += (end_op - start_op);
    }
  } stats;


  int route(int id, PRAZOR_GP_T &trans);
  
 public:
  traceregion *traceregions;

#ifdef CYCLE_ACCURATE
  volatile bool busy_flags[MAX_TARGETS];
#endif

  // Use tagged sockets to be able to distinguish incoming backward path calls
  tlm_utils::multi_passthrough_target_socket<busmux64, 64, PW_TLM_TYPES>    targ_socket;
  // use public bind() method from this class with three arguments to bind target socket
  tlm_utils::multi_passthrough_initiator_socket<busmux64, 64, PW_TLM_TYPES> inita_socket;


  // forwarding type
  enum fwd_type {
    THRESHOLD, // use threshold variable
    ONE_TO_ONE, // use one to one mapping 
    MANY_TO_ONE // uses many to one mapping
  } fwd;

  // Constructor
  busmux64(sc_core::sc_module_name name, 
	   fwd_type fwd = THRESHOLD);

  // FORWARD PATH
  // TLM-2 blocking transport method
  bool get_direct_mem_ptr(int n, PRAZOR_GP_T&, tlm::tlm_dmi& dmi_data);
  void b_transport(int id, PRAZOR_GP_T &trans, sc_time &delay);
  void stat_report(const char *msg, FILE *fd, bool resetf=false);

  bool cycle_accurate() { 
#ifdef CYCLE_ACCURATE
    return true;
#else
    return false;
#endif
  }

  void bind(
    u64_t start,
    u64_t size);

 private:
#ifdef TLM_POWER3
  pw_energy std_energy_op;
#endif

  typedef boost::icl::discrete_interval<u64_t> addr_range;
  // thresholds
  std::vector<addr_range> threshs;
};

#endif
// eof
