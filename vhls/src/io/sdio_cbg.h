// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2015 M Puzovic + D J Greaves

#ifndef __SDIO_CBG__
#define __SDIO_CBG__

#include "prazor.h"
#include "tenos.h"
#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/multi_passthrough_initiator_socket.h"
#include "tlm_utils/multi_passthrough_target_socket.h"


#define SDIOSTATS(X) X

class sdio_cbg : public sc_module 
#ifdef TLM_POWER3
  , public pw_module
#endif
{

 public:
  enum card_type {
    SDIO,
    SD
  };
  typedef enum card_type card_type_t;

  enum operating_state {
      invalid,
      stby, // stand-by state
      tran, // transfer state
      data, // sending data state 
      rcv, // receive data state
      prg, // programming state
      dis // disconnect state
  };
  typedef enum operating_state op_state_t;

  // Constructor
  sdio_cbg(sc_module_name name, u32_t base, card_type_t card);

  // connecting port
  tlm_utils::multi_passthrough_target_socket<sdio_cbg, 64, PW_TLM_TYPES> port0;
  // DMA
  tlm_utils::multi_passthrough_initiator_socket<sdio_cbg, 64, PW_TLM_TYPES> dma;


  void b_access(int idx, PRAZOR_GP_T &trans, sc_time &delay);

  sc_out<bool> IRQ;

  void run();
  void do_adma();

  void load(const char* vdd);

  void stat_report(const char *msg, FILE *fd, bool reset);  

 private:
  typedef struct stats {	   
  public:
    u32_t cmds_issued; 
    u32_t acmds_issued;
    u32_t ctrl_reads; // controller reads (in bits)
    u32_t ctrl_writes; // controller writes (in bits)
    u32_t card_regs_reads; // Card registers reads
    u32_t card_regs_writes; // Card registers writes
    u32_t io_reads; // I/O reads (in bytes)
    u32_t io_writes; // I/O writes (in bytes)

    bool collecting;

    stats() {
      collecting = false;
      reset();
    }
    
    void reset() {
      cmds_issued = 0;
      acmds_issued = 0;
      ctrl_reads = 0;
      ctrl_writes = 0;
      card_regs_reads = 0;
      card_regs_writes = 0;
      io_reads = 0;
      io_writes = 0;
    }
    
  } stats_t;
  

  stats_t m_stats;

  bool parse_and_exec_cmd(u32_t cmd, u32_t arg);
  void parse_block_size_and_count();

  card_type_t m_card;
  op_state_t m_op_state;

  // ADMA2 methods
  void do_adma2_tfr(u32_t addr, u16_t length, bool end);
  void do_adma2_cadr(u32_t addr, u16_t length, u2_t act, bool end);
  void do_adma2_fds();

  sc_event m_interrupt_change_event;
  bool m_interrupt_flag;

  u32_t m_base;

  // Available registers
  u32_t m_system_address;
  u32_t m_block_size_and_count;
  u32_t m_argument;
  u32_t m_transfer_mode_command;
  u32_t m_response[4];
  u32_t m_buffer_data_port;
  u25_t m_present_state;
  u32_t m_control;
  u27_t m_control_reset;
  u30_t m_interrupt_status;
  u30_t m_interrupt_status_enable;
  u30_t m_interrupt_signal_enable;
  u8_t m_error_status;
  u31_t m_capabilities;
  u24_t m_maximum_current_capabilities;
  u32_t m_force_event;
  u3_t m_ADMA_error_status;
  u32_t m_ADMA_system_address;
  u32_t m_boot_timeout_control;
  u1_t m_debug_selection;
  u8_t m_interrupt_support;
  u32_t m_slot_host;

  // Card Common Control Registers(CCCR)
  u8_t m_cccr[256];
  // Operations Conditions Register
  u32_t m_ocr;
  // Relative card address (RCA) register
  u16_t m_rca;
  // CIS
  u8_t* m_cis;
  // SD CARD Configuration register (SCR)
  u64_t m_scr;
  // SD Status register
  u8_t m_sd[64];
  
  bool m_next_acmd;

  u11_t m_block_size;
  u16_t m_block_count; 
  u32_t* m_RX_FIFO;
  u32_t m_RX_FIFO_idx;

  u32_t m_block_offset;
  
  // Data from VDD
  char* m_data_vdd;

  SC_HAS_PROCESS(sdio_cbg);
};

#endif
