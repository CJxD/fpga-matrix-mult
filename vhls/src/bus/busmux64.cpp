// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2009-10 D J Greaves
// University of Cambridge, Computer Laboratory.
// ACS P35 SoC D/M Classes - $Id: busmux64.cpp,v 1.10 2011/08/22 13:30:19 djg11 Exp $
// Generic payload blocking transport router


#include "busmux64.h"
#include <stdint.h>

#define BMON(X) 

//#undef CYCLE_ACCURATE


using namespace std;
using namespace tlm_utils;
using namespace boost::icl;

// Constructor
busmux64::busmux64(sc_core::sc_module_name name, 
		   fwd_type fwd):
  sc_module(name), 
  ml("ml"),
  //  wake_event("wake_event"),
  targ_socket("targ_socket"), 
  inita_socket("init_socket"), 
  fwd(fwd)
#ifdef TLM_POWER3
  ,  pw_module() // "power_config_busmux64.txt"
#endif


{
  traceregions = 0;
#ifdef TLM_POWER3
  // based on: just made up!
  POWER3(set_excess_area(pw_length(150, PW_um), pw_length(150,  PW_um)));

  std_energy_op = pw_energy((double) (0.1 * 64), pw_energy_unit::PW_pJ);   
#endif

  // Register callbacks for incoming interface method calls
  targ_socket.register_b_transport(this, &busmux64::b_transport);
  targ_socket.register_get_direct_mem_ptr(this, &busmux64::get_direct_mem_ptr);

#ifdef CYCLE_ACCURATE
  for (int i =0; i<MAX_TARGETS; i++)  busy_flags[i] = 0;
#endif

}

// Route to appropriate output socket.
int busmux64::route(int id, PW_TLM_PAYTYPE &trans)
{
  u64_t adr = (u64_t) trans.get_address();
  int idx;
  switch(fwd) {
    case THRESHOLD: {
      idx = inita_socket.size();
      int i = 0;
      for(vector<addr_range>::iterator it = threshs.begin();
	  it != threshs.end();
	  ++it, ++i) {
	if(contains(*it, adr)) {
	  idx = i;
	  break;
	}
      }
      break;
    }
    case ONE_TO_ONE: {
      assert(id < inita_socket.size());
      idx = id;
      break;
    }
    case MANY_TO_ONE: {
      assert(inita_socket.size() == 1);
      idx = 0;
      break;
    }
    default:
      assert(0);
  }
  int n_targets = inita_socket.size();
  
  //  printf("%s:%s: op " PFX64 " >= " PFX64 " n=%i/%i, tid=%i\n", name(), kind(), adr, thres0, idx, n_targets, sc_get_current_process_b()->proc_id);
  if (idx >= n_targets)
    {
      printf("%s:%s: op " PFX64 " n=%i/%i, tid=%i\n", name(), kind(), adr, idx, n_targets, sc_get_current_process_b()->proc_id);
      //SC_REPORT_ERROR("busmux64", "idx is out of range: insufficent targets connected");
      printf("%s: busmux64: %s", name(), "idx is out of range: insufficent targets connected.\n");
      printf("The ranges are:\n");
      printf("---------------\n");
      for(vector<addr_range>::iterator it = threshs.begin();
	  it != threshs.end();
	  ++it)
	cout << hex << *it << endl;
      return -1;
    }
  if (traceregions && traceregions->check(adr, TENOS_TRACE_BUS_OP))
    printf("%s:%s: op " PFX64 " n=%i/%i, tid=%i\n", name(), kind(), adr, idx, n_targets, sc_get_current_process_b()->proc_id);

  assert(idx>=0 && idx<n_targets);
  return idx;
}

// TLM-2 blocking transport method
void busmux64::b_transport(int id, PW_TLM_PAYTYPE &trans, sc_time &delay)
{
  POWER3(PW_TLM3(pw_agent_record l_agent = trans.pw_log_hop(this))); // First call
  stats.operations += 1;
  sc_time start_queue_time = sc_time_stamp() + delay;

  int idx = route(id, trans);
  if (idx < 0)
    {
      trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
      return;
    }
#ifdef CYCLE_ACCURATE
  assert(idx>=0 && idx<MAX_TARGETS);
  int tid =  sc_get_current_process_b()->proc_id;
  //bool did_queue = false;
  ml.lock();
  requests[idx].push(tid);
  bool isTop = requests[idx].front() == tid;
  if (busy_flags[idx] || !isTop)
    {
      if(!isTop && !busy_flags[idx])
	// resend the wake event as there is going to be a thread
	// waiting for it that registered itself after wake event
	// has been generated by previous thread that it finished 
	// with bus
	wake_event[idx].notify();
      
      BMON(printf("tid=%i: Start wait " PFX64 " idx=%i\n", tid, adr, idx));
      //did_queue = true;
      stats.contended_operations += 1;
      wait(delay); delay = SC_ZERO_TIME;
      // added timeout since events being lost? : despite using sc_event_queue...

      //do { ml.unlock(); ; ml.lock(); } while (busy_flags[idx]);
      do { BMON(printf ("tid=%i: spin for %i\n", tid, idx)); 
	//printf("tid=%i: Flag0 is %i\n", tid, busy_flags[0]);
	ml.unlock();
	wait(wake_event[idx]);  
	ml.lock(); 
	isTop = requests[idx].front() == tid;
      } while (busy_flags[idx] || !isTop);
      BMON(printf("tid=%i: Finished wait " PFX64 " idx=%i\n", tid, adr, idx));
    }
  BMON(printf("tid=%i: claimed lock for " PFX64 " idx=%i\n", tid, adr, idx));
  busy_flags[idx] = true;
  ml.unlock();
#endif
  sc_time start_op_time = sc_time_stamp() + delay;


  inita_socket[idx]->b_transport(trans, delay); // HERE WE DO THE ACTUAL PASSTHRU.

#ifdef CYCLE_ACCURATE 
  if (true/* ||did_queue*/)
    {
      wait(delay); // do not really need this quantum sync even if we did queue.?
      delay = SC_ZERO_TIME;
    }
  
  ml.lock();
  busy_flags[idx] = false;
  int top = requests[idx].front();
  assert(top == tid);
  requests[idx].pop();
  BMON(printf("tid=%i: Notify " PFX64 " idx=%i\n", tid, adr, idx));
  //  printf("tid=%i: Flag0 is %i\n", tid, busy_flags[0]);
  //wake_event.notify(delay);
  wake_event[idx].notify();
  ml.unlock();
#else
  
  // TODO: add this.
  //  delay += contention estimate when loosely timed ... 

#endif
  sc_time end_op_time = sc_time_stamp() + delay;
  stats.log(start_queue_time, start_op_time, end_op_time);

#if PW_TLM_PAYLOAD > 0
  POWER3(l_agent.record_energy_use(std_energy_op, &trans));
  POWER3(trans.pw_log_hop(this)); //Second call, after return.
#else
  POWER3(record_energy_use(std_energy_op, &trans));
#endif

}



void busmux64::stat_report(const char *msg, FILE *fd, bool resetf) 
{
  stats.stat_report(this, msg, fd, resetf);
}

void busmux64::stats_t::stat_report(busmux64 *p, const char *msg, FILE *fd, bool resetf)  
{
  printf("%s: %s: operations=%i contended=%i\n", p->name(), msg, operations, contended_operations);
  if (operations)
    {
      sc_time rt = response_time / operations;
      sc_time st = service_time / operations;
      cout << p->name() << ": ave response time=" << rt << ", ave service_time=" << st << "\n";
    }
  if (resetf) reset();
}


bool busmux64::get_direct_mem_ptr(int id, PW_TLM_PAYTYPE &trans, tlm::tlm_dmi &dmi_data)
{
  int idx = route(id, trans);
  inita_socket[idx]->get_direct_mem_ptr(trans, dmi_data);
}

const char *busmux64() { return "busmux64"; }

void busmux64::bind(
  u64_t start,
  u64_t size) {
  threshs.push_back(construct<discrete_interval<u64_t> >(start, start+size, interval_bounds::right_open()));
}

// eof