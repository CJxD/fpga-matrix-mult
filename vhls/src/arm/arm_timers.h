// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2014 M Puzovic + D J Greaves

#ifndef __ARM_TIMERS__
#define __ARM_TIMERS__

#include "prazor.h"
#include "tenos.h"
#include "systemc.h"
#include "tlm.h"
#include "../memories/smallram.h"
#include "tlm_utils/multi_passthrough_initiator_socket.h"
#include "tlm_utils/multi_passthrough_target_socket.h"
#include "armcore_tlm.h"
#include "gic_arm_tlm.h"

#include <atomic>

// if this directive is defined then timer is trigged
// at each clock cycle which will result in VERY SLOW 
// simulation performance.
//#define CLK_EDGE_TRIGGER 1

// TODO: This value is defined in device tree under node
// slcr with name ps-clk-frequency, for now it is
// hardcoded but we might look into parsing
// device tree file so that we get updated value always
#define PS_CLKS 30 // nanoseconds

#define GLBT_BASE_ADDR PERIPHBASE+0x200
#define GLBT_LENGTH 0xFF
#define TTC0_BASE_ADDR 0xf8001000
#define TWD_BASE_ADDR 0xf8f00600
#define TWD_LENGTH 0xFF
#define WDG_BASE_ADDR 0x20
#define SWDT_BASE_ADDR 0xF8005000
#define SWDT_LENGTH 0x100

// Base timer
// All timer should extend this class
class base_timer : public sc_module
#ifdef TLM_POWER3
  , public pw_module
#endif
{
  public:
    base_timer(sc_module_name name);

    // output port to SLCR
    tlm_utils::simple_initiator_socket<arm_mmu, 64, PW_TLM_TYPES> slcr_initiator;

  protected:
    sc_time calculate_period(int factor);
};

// Private timer and watchdog
class arm_twd : public base_timer
{
 public:
  arm_twd(sc_module_name name);

  // connecting port
  tlm_utils::multi_passthrough_target_socket<arm_twd, 64, PW_TLM_TYPES> twd_target0;
  
  void b_access(int idx, PRAZOR_GP_T &trans, sc_time &delay);

  void run();
  void run_watchdog();

  // Pass the address of this field to targets that want to be interrupted.
  sc_out<bool> IRQ;
  sc_out<bool> wdg_IRQ;

  // registers
 private:
  sc_time calculate_timer_interval(u32_t);

  // Private Timer

  // load register - 0x0
  std::atomic<u32_t> load;
  // counter register - 0x4
  std::atomic<u32_t> counter;

  // control register - 0x8
  // 31                  16 15        8 7        32 1 0
  // +---------------------+-----------+---------+-+-+-+ 
  // |       UNK/SZB       | Prescaler | UNK/SZB |I|R|E|
  // +---------------------+-----------+---------+-+-+-+
  std::atomic<bool> timer_enabled; // bit 0 -> E
  sc_event timer_enabled_event;
  std::atomic<bool> auto_reload; // bit 1 -> R
  std::atomic<bool> irq_enabled; // bit 2 -> I
  std::atomic<u8_t> prescaler;
  
  // interrupt status register - 0xC
  std::atomic<bool> is_event_flag;

  sc_event interrupt_reset_event;
  bool interrupt_state;

  sc_time twd_period;

  // Watchdog

  // load register - 0x20
  std::atomic<u32_t> wdg_load;
  // counter register - 0x24
  std::atomic<u32_t> wdg_counter;
  // control register - 0x28
  // 31                  16 15        8 7       43 2 1 0
  // +---------------------+-----------+--------+-+-+-+-+ 
  // |       UNK/SZB       | Prescaler | UNK/SZB|W|I|R|E|
  // +---------------------+-----------+--------+-+-+-+-+
  std::atomic<bool> wdg_enabled; // bit 0 -> E
  sc_event wdg_enabled_event;
  sc_time wdg_started;
  std::atomic<bool> wdg_auto_reload; // bit 1 -> R
  std::atomic<bool> wdg_it_enabled; // bit 2 -> I
  std::atomic<bool> wdg_mode; // bit 3 -> W
  u8_t wdg_prescaler;

  // interrupt status register - 0x2C
  std::atomic<bool> wdg_interrupt_status;
  // reset status register - 0x30
  std::atomic<bool> wdg_reset_status;
  // disable register - 0x34
  u32_t wdg_disable_register;

  u32_t calculate_wdg_counter();
  sc_time calculate_wdg_remainder();


  SC_HAS_PROCESS(arm_twd);
};

// Triple timer counter
class arm_ttc : public base_timer
{
 private:
  u7_t clock_control[3];
  u7_t counter_control[3];
  std::atomic<u16_t> counter_value[3];
  u16_t interval_counter[3];
  u16_t match_counter[3][3];
  u6_t interrupt_register[3];
  u6_t interrupt_enable[3];
  u3_t event_control_timer[3];
  u16_t event_register[3];

  sc_time timer_interval[3];
  sc_time timer_started[3];
  u64_t timer_counter[3];

  sc_event timer_enabled[3];
  sc_event reset_event[3];

  sc_time calculate_reminder(int id);
  void run_timer(int);

  bool interrupt_state[3];
  sc_event interrupt_reset_event[3];

 public:
  // Constructor
  arm_ttc(sc_module_name);

  // connecting port
  tlm_utils::multi_passthrough_target_socket<arm_ttc, 64, PW_TLM_TYPES> ttc_target0;
  
  void b_access(int idx, PRAZOR_GP_T &trans, sc_time &delay);

  void run_timer1();
  void run_timer2();
  void run_timer3();


  // Pass the address of this field to targets that want to be interrupted.
  sc_out<bool> IRQ[3];
  
  SC_HAS_PROCESS(arm_ttc);
};

// Global timer
class arm_glbt : public base_timer
{
 private:
  // Global Timer counter registers
  u64_t timer_counter;

  // Global Timer Control Register
  bool timer_enabled;
  bool comp_enabled[GIC_MAX_N_CPUS];
  bool irq_enabled[GIC_MAX_N_CPUS];
  bool auto_increment[GIC_MAX_N_CPUS];
  u8_t prescaler;

  // Global Timer Interrupt Status Registers
  bool interrupt_status[GIC_MAX_N_CPUS];

  // Global Timer Comparator Value Registers
  u64_t comparator[GIC_MAX_N_CPUS];
  
  u32_t auto_increment_value[GIC_MAX_N_CPUS];

  // Helper variables
  sc_time timer_started;
  sc_time timer_interval;
  sc_event timer_enabled_event;
  int timer_interval_core;
  int n_cores;

  sc_time calculate_reminder();
  sc_time calculate_timer_interval();
  u64_t calculate_timer_counter();
 
 public:
  // Constructor
  arm_glbt(sc_module_name, int);

  sc_out<bool> IRQ[GIC_MAX_N_CPUS];

  tlm_utils::multi_passthrough_target_socket<arm_glbt, 64, PW_TLM_TYPES> glb_target;
  void b_access(int idx, PRAZOR_GP_T &trans, sc_time &delay);

  void run();

  SC_HAS_PROCESS(arm_glbt);
};

// System watchdog timer
class arm_swdt : public base_timer
{
 private:
  // offset 0x0 - register MODE
  u12_t zkey;
  u2_t irqln;
  bool irqen;
  bool rsten;
  bool wden;

  // offset 0x4 - register CONTROL
  u12_t ckey;
  u12_t crv;
  u2_t clksel;

  // offset 0x8 - register RESTART
  u16_t rstkey;
  sc_event reset_event;

  // offset 0xC
  bool wdz;

  sc_time calculate_reminder();
  sc_time calculate_timer_interval();

  sc_time swdt_period;
    
 public:
  arm_swdt(sc_module_name);

  tlm_utils::multi_passthrough_target_socket<arm_swdt, 64, PW_TLM_TYPES> swdt_target;
  void b_access(int idx, PRAZOR_GP_T &trans, sc_time &delay);

  void run();

  sc_out<bool> IRQ;

  SC_HAS_PROCESS(arm_swdt);
};

#endif
