/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */


#ifndef ARMCORE_TLM_H
#define ARMCORE_TLM_H

#ifdef BSYSTEM
#include <pthread.h> 
#endif

#include "COREISA_IF.h"
#include "armisa.h"
#include "tlm_utils/tlm_quantumkeeper.h"
#include "speedo_reg_mapping.h"
#include "socdam_bdoor_mapping.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "arm_abt.h"

#define PERIPHBASE 0xf8f00000

struct cp_request_extension : tlm::tlm_extension<cp_request_extension> {
  public:
    u4_t cpr;
    u4_t crm;
    u3_t op1;
    u3_t op2;

    cp_request_extension() {
        cpr = -1;
        crm = -1;
        op1 = -1;
        op2 = -1;
    }

    cp_request_extension(u4_t _cpr, u4_t _crm, u4_t _op1, u4_t _op2) {
        cpr = _cpr;
        crm = _crm;
        op1 = _op1;
        op2 = _op2;
    }


    tlm_extension_base* clone() const {
        cp_request_extension* ext = new cp_request_extension;
        ext->cpr = this->cpr;
        ext->crm = this->crm;
        ext->op1 = this->op1;
        ext->op2 = this->op2;

        return ext;
    }

    void copy_from(tlm_extension_base const &ext) {
        cpr = static_cast<cp_request_extension const&>(ext).cpr;
        crm = static_cast<cp_request_extension const&>(ext).crm;
        op1 = static_cast<cp_request_extension const&>(ext).op1;
        op2 = static_cast<cp_request_extension const&>(ext).op2;
    }
};


class armcore_tlm: public COREISA_IF, public armisa
{
  sc_time lt_i_delay;
  sc_time lt_d_delay;
  bool over;
  bool halted;
  tlm_utils::tlm_quantumkeeper m_qk; // Quantum keeper for temporal decoupling 
  POWER3(pw_energy m_instruction_energy);
  POWER3(pw_energy m_mispredict_energy_penalty);


#ifdef BSYSTEM
  pthread_t pth; // BSYSTEM

  void run_proxy()
  {
    pthread_join(pth, 0);
  }
#endif


  struct ins_fetcher_t
  { // Instruction mini-cache for other half of a 64 bit word.
    COREISA_IF *parent;
    u64_t cached_adr, cached_ins;

  ins_fetcher_t(armcore_tlm *parent) : parent(parent) { cached_adr = 1; // an invalid address
    };
      bool fetch(u32_t adr, u32_t &i, sc_time &lt_busdelay);
  } ins_fetcher;

  void lt_all_sync();
  u1_t wbuf_logged();
  bool eval_insn(u32_t memaddr, int *bp, u32_t& data, u8_t size_code);
  void run();
  void sim_done(const char *msg);

  void /*override*/iss_yield(int cycles);  // Pause CPU for this time interval: this makes it take less static and dynamic power for this interval.

  int simulator_cmd(int cmd, int arg0, int arg1, u32_t *rp);
  int arm_sim_command(int cmd, int arg0, int arg1, u32_t *rp);
  void serve_debugger();
  bool gdb_warning_issued;
 public:
  u8_t armisa_write1(u32_t, u64_t, u2_t, u32_t *llsc_scp=0); // Return 0 on ok operation
  bool armisa_read1(u32_t, u1_t, u1_t,  bool linkedf=false, bool mmu=false);
  uint32_t copro_read(u4_t, u4_t, u4_t, u3_t op1=0, u3_t op2=0);
  void copro_write(u4_t, u4_t, u4_t, u32_t, u3_t op1=0, u3_t op2=0);
  void reset(bool selfstart);
  void stat_report(const char*, FILE*, bool);
  armcore_tlm (sc_core::sc_module_name name, u8_t idx, bool harvardf);
  void end_of_simulation(void); // An overload, called by SystemC on exit.
  u32_t check_tenos_trace(u32_t mask, u32_t pc);

  void set_inter_core_event(std::atomic<u32_t>* pevent) {
      inter_core_event = pevent;
  }

  // port for cp15
  tlm_utils::simple_initiator_socket<cpu_busaccess, 64, PW_TLM_TYPES> cp14_port;
  tlm_utils::simple_initiator_socket<cpu_busaccess, 64, PW_TLM_TYPES> cp15_port;

  void recompute_pvt_parameters(); // This callback invoked by POWER3 when supply voltage changed.
  sc_in<bool> IRQ; // Interrupt request.


  alternate_banch_target_buffer *add_abt(const char *name, bool with_snoop_socket)
  {
    m_abtb = new alternate_banch_target_buffer(name,  with_snoop_socket);
    return m_abtb;
  }

  void /*override*/diagnostic_report(FILE *fd, int severity, const char *msg);

}

;


#endif
