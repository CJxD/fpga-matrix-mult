// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2012 M Puzovic and D J Greaves
// University of Cambridge, Computer Laboratory.
// $Id: $

// Header file for HyperTransport links

#ifndef HYPERTRANSPORT_H
#define HYPERTRANSPORT_H

#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/multi_passthrough_initiator_socket.h"
#include "tlm_utils/multi_passthrough_target_socket.h"

#include "prazor.h"
#include "tenos.h"


class hypertransport :
  public sc_module
#ifdef TLM_POWER3
  , public pw_module
#endif
{
 private:
  int m_socket_no;
  int m_link_no;
  // working frequency of the hypertransport link
  sc_time m_period;
  // size of the link in bits
  int m_link_size;
  // next time when information can be sent using the link
  sc_time m_next_time;
  // make sure that only one thread modifies next time
  sc_mutex* ht_lock;

  int m_dest_clk_cycles;
  int m_src_clk_cycles;

 public:
  tlm_utils::multi_passthrough_target_socket<hypertransport, 64, PW_TLM_TYPES> targ_socket;
  tlm_utils::multi_passthrough_initiator_socket<hypertransport, 64, PW_TLM_TYPES> init_socket;

  // Constructor
  hypertransport(sc_core::sc_module_name, 
		 int socket, 
		 int link_no,
		 sc_time period,
		 int link_size,
		 int,
		 int);

  // TLM-2 blocking transport method
  bool get_direct_mem_ptr(int n, PRAZOR_GP_T&, tlm::tlm_dmi& dmi_data);
  void b_transport(int id, PRAZOR_GP_T& trans, sc_time &delay);
  tlm::tlm_sync_enum nb_transport_fw(int n,
				     PRAZOR_GP_T& trans,
				     tlm::tlm_phase& phase,
				     sc_time& delay);

};

#endif
