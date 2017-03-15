//
// Zynq-compatible UART.
//
// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2008-15 DJ Greaves.
// cbg TLM/ESL console uart model: in popup xterm or using stdin/out depending on the console mode
//
#ifndef UART64_CBG_H
#define UART64_CBG_H

#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "tenos.h"
#include "systemc.h"
#include "tlm.h"
//#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/multi_passthrough_target_socket.h"


// Programmer's model (Xilinx Zynq compatible).
// Register names
#define UART2_CR  0        // Control register
#define UART2_MR  4        // Mode register
#define UART2_IER 8        // Interrupt enable - write a one to enable an int condition in the mask.
#define UART2_IDR 0xC      // Interrupt disable  - write a one to disable. 
#define UART2_IMR 0x10     // Interrupt mask.
#define UART2_ISR 0x14     // (chnl_int_sts_reg0) Interrupt status unmasked (write to clear)  
#define UART2_BAUDRATE     0x18
#define UART2_RCVR_TIMEOUT 0x1c
#define UART2_RXWM    0x20 // RX fifo trigger level
#define UART2_MCR     0x24 // H/W flow control
#define UART2_MSE     0x28 // H/W flow control
#define UART2_SR      0x2c  // (chnl_sts_reg0) Main status register - readonly.
#define UART2_FIFO    0x30  // Data FIFO (send and receive)
#define UART2_BAUDRATE_DIVIDER    0x34  // 
#define UART2_TXWM    0x44  // TX fifo trigger level


// Interrupt bit lanes - now same as Zync
#define UART2_INT_TX_EMPTY        (0x08)
#define UART2_INT_RX_FULL         (0x04)
#define UART2_INT_RX_EMPTY        (0x02)
#define UART2_INT_RX_TRIGGER      (0x01)

// Status register bits
#define UART2_STATUS_TX_FULL  (0x10)  //
#define UART2_STATUS_TX_EMPTY (0x08)  //
#define UART2_STATUS_RX_EMPTY (0x02)  //



#ifdef TLM_POWER3
#include <tlm_power>
#define POWER3(X) X
using namespace sc_pwr;
#else
typedef tlm::tlm_base_protocol_types PW_TLM_TYPES;
typedef tlm::tlm_generic_payload PW_TLM_PAYTYPE;
#define POWER3(X)
#endif





class uart64_cbg : public sc_module
#ifdef TLM_POWER3
, public pw_module
#endif
{
  SC_HAS_PROCESS(uart64_cbg);
  POWER3(PW_TLM3(sc_pwr::tlm_bit_transition_tracker read_bus_tracker)); // Used if counting wire transitions. 
  bool use_stdin;
  int xterm_opened, threads_started, idle_count;
  FILE *output_f;
  int xterm_fd;
  bool xconsole;
  void open_xterm();
  volatile int logged, yielding;
  pthread_t input_thread0, input_thread1;
  sc_time latency;
  const char *canned_input;
  FILE *canned_fd;
  bool txready;

  struct stats_t
  {
    int reads, writes, ins, outs;
    stats_t() { reset(); }
    void reset()
    {
      reads = 0;
      writes = 0;
      ins = 0;
      outs = 0;
    }
  } stats;

  
  // blocking transport
  void b_access(int id, PW_TLM_PAYTYPE &trans, sc_time &delay);

  int testch();
  char rdch();
  char rdch_nonblock();
  void uart_wrch(char);
  void flush();

  void clear_interrupt(u32_t wtc);

  // Programmer's model registers
  u32_t interrupts_unmasked(); // Not an actual register - created dynamically on each read.
  u32_t status_register_read(); // Not an actual register - created dynamically on each read.
  u32_t tx_interrupt_level;
  u32_t rx_interrupt_level;
  u32_t mode_register;
  u32_t interrupt_mask_register;
  u32_t control_register;
  bool read_next_char;
  bool raised_interrupt;
  bool transmit_char;
  // Combinational interrupt output: manually call this whenever 
  // supporting fields (the support) are changed, instead of having 
  // a dedicated SystemC SC_THREAD.
  void update_interrupt();

  void end_of_simulation();
public:
  int open_log_file(const char *fn);  
  void *input_process(void *);// public for pthread callback (yuck).


public:
  traceregion *traceregions;

  // constructor
  uart64_cbg(sc_module_name name, bool use_x11, bool u);

  const char *kind();
  void stat_report(const char *msg, FILE *fd, bool resetf=0);

  //TLM-2 socket, defaults to 32-bits wide, power protocol
  //tlm_utils::simple_target_socket<uart64_cbg, 64, PW_TLM_TYPES> port0;
  tlm_utils::multi_passthrough_target_socket<uart64_cbg, 64, PW_TLM_TYPES> port0;

  // Note: Immediate string or file data for uart model input.
  // Note: the uart model will handle input starting with a dot or a slash as a file to read from
  void register_canned_input(const char *d);

  // Pass the address of this field to targets that want to be interrupted.
  sc_out<bool> IRQ;
};

#endif

// eof

