// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>


#include "arm_timers.h"
#include <iostream>

#define ARM_TIMERS(X)

#define ARM_PLL_CTRL 0xF8000100
#define PLL_FDIV_MASK 0x7F
#define PLL_FDIV_SHIFT 12

#define ARM_CLK_CTRL 0xF8000120
#define DIVISOR_MASK 0x3F
#define DIVISOR_SHIFT 8

using namespace std;
using namespace tlm;

base_timer::base_timer(sc_module_name name)
    : sc_module(name) {}

sc_time base_timer::calculate_period(int factor) {
    // get from SCLR what is the value of PLL_FDIV
    u32_t data;
    PRAZOR_GP_T trans;
    trans.set_byte_enable_length(0);
    trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    trans.set_read();
    trans.set_address(ARM_PLL_CTRL);
    trans.set_data_length(4);
    trans.set_data_ptr((unsigned char*) &data);

    sc_time delay = SC_ZERO_TIME;
    slcr_initiator->b_transport(trans, delay);
    assert(!trans.is_response_error());

    u7_t pll_fdiv = (data >> PLL_FDIV_SHIFT) & PLL_FDIV_MASK;
    //printf("%s: Value of PLL_FDIV is %d\n", name(), pll_fdiv);

    // now obtain value of DIVISOR
    trans.set_address(ARM_CLK_CTRL);
    trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    slcr_initiator->b_transport(trans, delay);
    assert(!trans.is_response_error());

    u6_t divisor = (data >> DIVISOR_SHIFT) & DIVISOR_MASK;
    //printf("%s: Value of DIVISOR is %d\n", name(), divisor);

    double period = ((double)PS_CLKS)/pll_fdiv*divisor*factor;
    sc_time period_ns = sc_time(period, SC_NS);

    //cout << name() << ": Private timer period is: " << period_ns << endl;

    return period_ns;
}


arm_twd::arm_twd(sc_module_name name) 
    : base_timer(name),
      twd_period(SC_ZERO_TIME) {

  // All reset values are 0x0
  load.store(0x0);
  counter.store(0x0);
  timer_enabled.store(false);
  auto_reload.store(false);
  irq_enabled.store(false);
  prescaler.store(0x0);
  is_event_flag.store(false);
  
  interrupt_state = false;

  // Watchdog
  wdg_load.store(0x0);
  wdg_counter.store(0x0);
  wdg_enabled.store(false);
  wdg_auto_reload.store(false);
  wdg_it_enabled.store(false);
  wdg_mode.store(false);
  wdg_prescaler = 0x0;
  wdg_interrupt_status.store(false);
  wdg_reset_status.store(false);
  wdg_disable_register = 0x0;

  wdg_started = SC_ZERO_TIME;

  twd_target0.register_b_transport(this, &arm_twd::b_access);

  SC_THREAD(run);
  SC_THREAD(run_watchdog);
}

void arm_twd::b_access(int idx, PRAZOR_GP_T &trans, sc_time &delay) {
  tlm_command cmd = trans.get_command(); 
  u32_t adr = (u32_t)trans.get_address();
  u8_t* ptr = trans.get_data_ptr();

  switch(cmd) {
    case TLM_READ_COMMAND: {
      u32_t offset = adr - TWD_BASE_ADDR;
      switch(offset) {
        case 0x0: {
	  u32_t data = load.load();
	  memcpy(ptr, (u8_t*)(&data), 4);
	  break;
	}
	  
        case 0x4: {
	  u32_t data = counter.load();
	  memcpy(ptr, (u8_t*)(&counter), 4);
	  break;
	}

        case 0x8: {
	  // for the control register we need first
	  // to combine all variables we are using for it
	  u32_t data = 0;
	  data |= 
	    (prescaler.load() << 8) | 
	    ((u1_t)irq_enabled.load() << 2) |
	    ((u1_t)auto_reload.load() << 1) |
	    ((u1_t)timer_enabled.load());
	  
	  memcpy(ptr, (u8_t*)(&data), 4);
	  
	  break;
	}

        case 0xC: {
	  u32_t data = (u32_t)is_event_flag.load();
	  memcpy(ptr, (u8_t*)(&data), 4);
	  
	  break;
	}

        case 0x20: {
	  u32_t data = wdg_load.load();
	  memcpy(ptr, (u8_t*)(&data), 4);

	  break;
        }

        case 0x24: {
	  u32_t data;
	  if(wdg_enabled.load())
	    data = calculate_wdg_counter();
	  else
	    data = wdg_counter.load();

	  memcpy(ptr, (u8_t*)(&data), 4);
	  
	  break;
        }

        case 0x28: {
	  u32_t data = 0;
	  data |= (wdg_prescaler << 8)
	    | (wdg_mode.load() << 3)
	    | (wdg_it_enabled.load() << 2)
	    | (wdg_auto_reload.load() << 1)
	    | (wdg_enabled.load());

	  memcpy(ptr, (u8_t*)(&data), 4);

	  break;
        }

        case 0x2C: {
	  u32_t data = (u32_t)wdg_interrupt_status.load();
	  
	  memcpy(ptr, (u8_t*)(&data), 4);
	  
	  break;
        }

        case 0x30: {
	  u32_t data = (u32_t)wdg_reset_status.load();
	  
	  memcpy(ptr, (u8_t*)(&data), 4);

	  break;
        }

        case 0x34: {
	  // we should not be reading this value
	  // only writing to it
	  assert(0);
        }
      }

      ARM_TIMERS(printf("%s: read addr 0x%08x, offset 0x%02x, data 0x%08x, length %d\n",
                       name(), adr, offset, *((u32_t*)ptr), trans.get_data_length()));
      break;
    }

    case TLM_WRITE_COMMAND: {
      u32_t data = ((u64_t*)ptr)[0];
      u8_t* lanes = trans.get_byte_enable_ptr();
      assert(trans.get_data_length() == 8);

      if(lanes && lanes[4] == 0xFF) {
	// we need to increment address by 4
	// because data length is 8
	adr += 4;
	// since we have changed the address
	// we also need to recalculate the offset
      }

      u32_t offset = adr - TWD_BASE_ADDR;

      // in order to enable timer mode writes must be consecutive
      if(wdg_disable_register == 0x12345678 && offset != 0x34) 
	wdg_disable_register = 0x0;

      switch(offset) {
        case 0x0: {
	  // Register at offset 0x0 is Private Timer Load register
	  // and if we are writing to this register we also need to
	  // write to register at offset 0x4 (Private Timer Counter Register)
	  load.store(data);
	  counter.store(data);
	  break;
	}
        case 0x4: {
          // TODO: Need to check if timer is active, if timer is active
          // then we should reset it to count down from value just written
          counter.store(data);
	  break;
	}
        case 0x8: {
	  // Timer Control Register
	   
	  timer_enabled.store((bool)(data & 0x1));
	  if(timer_enabled.load())
	    timer_enabled_event.notify();
	  auto_reload.store((bool)(data & 0x2));
	  irq_enabled.store((bool)(data & 0x4));
	  prescaler.store((u8_t)((data >> 8) & 0xFF));
	  
	  break;
	}

        case 0xC: {
	  if((data & 0x1)) {
	    is_event_flag.store(false);
            interrupt_reset_event.notify();
	  }
	    
	  break;
	}

        case 0x20: {
	  // we should only write to this register
	  // when watchdog is disabled
	  assert(!wdg_enabled.load());

	  wdg_load.store(data);
	  wdg_counter.store(data);

	  break;
        }

        case 0x24: {
	  // we should only write to this register
	  // when watchdog is disabled
	  assert(!wdg_enabled.load());
	  if(!wdg_mode.load()) {
	    wdg_counter.store(data);
	  }

	  break;
        }

        case 0x28: {
	  bool enabled = wdg_enabled.load();
	  wdg_prescaler = (u8_t)((data >> 8) & 0xFFFF);
	  bool mode = !!(data & 0x8);
	  if(mode)
	    wdg_mode.store(mode);

	  wdg_it_enabled.store(!!(data & 0x4));
	  wdg_auto_reload.store(!!(data & 0x2));
	  wdg_enabled.store(!!(data & 0x1));

	  if(enabled != wdg_enabled.load())
	    wdg_enabled_event.notify();

	  break;
        }

        case 0x2C: {
	  if(data == 1) {
	    bool old_value = wdg_interrupt_status.load();
	    assert(wdg_interrupt_status.compare_exchange_strong(old_value, false));
	  }
	  
	  break;
        }

        case 0x30: {
	  if(data == 1) {
	    bool old_value = wdg_reset_status.load();
	    assert(wdg_reset_status.compare_exchange_strong(old_value, false));
	  }
	  
	  break;
        }
	  
        case 0x34: {
	  if(wdg_disable_register == 0x12345678 && data == 0x87654321)
	    wdg_mode.store(false);
	  wdg_disable_register = 0x0;

	  break;
        }
	
      }

      ARM_TIMERS(printf("%s: write addr 0x%08x, offset 0x%08x, data 0x%08x(0x%08x), length %d\n",
                       name(), adr, offset, *((u32_t*)ptr), data, trans.get_data_length()));
      break;
    }

    default:
      assert(0); // we should never reach this
  }

  trans.set_response_status(TLM_OK_RESPONSE);
}

sc_time arm_twd::calculate_timer_interval(u32_t counter) {
  assert(twd_period != SC_ZERO_TIME);
  double interval = (prescaler.load() + 1)*(counter + 1)*(twd_period.to_seconds()*1e9);
  
  //ARM_TIMERS(printf("%s: PERIPHCLK = %.4f ns, %d, %d\n", name(), periphclk, prescaler.load(), load.load()));
  //ARM_TIMERS(printf("%s: calculated timer interval is %.4f ns\n", name(), interval));

  return sc_time(interval, SC_NS);
}

u32_t arm_twd::calculate_wdg_counter() {
  uint64_t current_time = static_cast<uint64_t>(sc_time_stamp().to_seconds()*1e12);
  uint64_t wdg_start_time = static_cast<uint64_t>(wdg_started.to_seconds()*1e12);
  uint64_t period = static_cast<uint64_t>(twd_period.to_seconds()*1e12);

  u32_t count = ((u32_t)(floor((current_time-wdg_start_time)/period))) % wdg_counter.load();

  return count;
}

#ifdef CLK_EDGE_TRIGGER 
void arm_twd::run() {
  while(true) {
    if(timer_enabled.load()) {
      u32_t value = counter.fetch_sub(1) - 1;
      if(value == 0) {
	// check to see if auto reload is enabled
	// if it is enabled then reload value 
	if(auto_reload.load()) {
	  while(!counter.compare_exchange_strong(value, load.load())) {
	    // We haven't managed to change the value because
	    // some other thread had jumped in, so we will wait
	    // for the same amount of time as if counter needs to
	    // decrement before trying to change the value again
	    assert(0);
	    wait(timer_interval);
	  }
	 
	}
	else {
	  // this is one shoot so what we are going to do
	  // is to disable timer
	  bool enabled = true;
	  assert(timer_enabled.compare_exchange_strong(enabled, false));
	}
		
	// check to see if IRQ is enabled, if it is enabled
	// then we need to raise the interrupt
	if(irq_enabled.load()) {
	  // write to timer interrupt status register 
	  is_event_flag.store(true);
          interrupt_state_event.notify();
	  ARM_TIMERS(printf("%s: sent interrupt @ ", name()); cout << sc_time_stamp() << endl);
	}
      }
    }
    wait(timer_interval);
  }
}
#else
void arm_twd::run() {
  while(true) {
    // we are only going to start timer
    // once it was enabled
    if(!timer_enabled.load())
      wait(timer_enabled_event);
    
    // calculate what is period of timer
    // TODO: at the moment we only recalculate period
    // when timer is reenabled, if we come up with user case
    // where clocks can be changed during the simulation
    // then we need to change that such that we recalculate 
    // period whenever clocks change
    twd_period = calculate_period(2); // CPU_3x2x

    // check to see if current time is a multiple of PERIPHCLK 
    // if it is not then wait until we get there
    uint64_t current_time = static_cast<uint64_t>(sc_time_stamp().to_seconds()*1e12);
    uint64_t periphclk = static_cast<uint64_t>(twd_period.to_seconds()*1e12);
    uint64_t remainder = current_time % periphclk;

    wait(sc_time(remainder, SC_PS));

    ARM_TIMERS(printf("%s: timer enabled @ ", name()); cout << sc_time_stamp() << endl);

    // the timer is enabled so we are entering
    // second 'infinite' loop that waits for
    // a specified interval
    while(true) {
      sc_time start = sc_time_stamp();
      sc_time interval = calculate_timer_interval(load.load());
      wait(interval, interrupt_reset_event);
      
      sc_time time_passed = sc_time_stamp() - start;
      if(interrupt_state && (interval > time_passed)) {
          interrupt_state = false;
          IRQ = interrupt_state;
          wait((interval - time_passed));
      } else if(interrupt_state && (time_passed >= interval)) {
          continue;
      }

      // we need to generate event now
      if(irq_enabled.load()) {
	// write to timer interrupt status register
	is_event_flag.store(true);
        interrupt_state = true;
        IRQ = interrupt_state;
	ARM_TIMERS(printf("%s: sent interrupt (%ld, %ld) @ ", name(), prescaler.load() + 1, counter + 1); cout << sc_time_stamp() << endl);
      }

      if(!auto_reload.load()
	 || !timer_enabled.load()) {
	// it is either one shot or timer was disabled as it
	// disabled then we will break out from this loop
	break;
      }
    }
  }
}

#endif

sc_time arm_twd::calculate_wdg_remainder() {
  uint64_t current_time = static_cast<uint64_t>(sc_time_stamp().to_seconds()*1e12);
  uint64_t periphclk = static_cast<uint64_t>(twd_period.to_seconds()*1e12);
  uint64_t remainder = current_time % periphclk;

  return sc_time(remainder, SC_PS);
}


void arm_twd::run_watchdog() {
  while(true) {
    wait(wdg_enabled_event);

    sc_time remainder = calculate_wdg_remainder();
    sc_time wait_period = twd_period - remainder;

    wait(wait_period);

    ARM_TIMERS(printf("%s: watchdog started @ ", name()); cout << sc_time_stamp() << endl);
    
    wdg_started = sc_time_stamp();

    while(true) {
      sc_time interval = calculate_timer_interval(wdg_counter.load());
      wait(interval);

      if(!wdg_mode.load() && wdg_it_enabled.load()) {
	wdg_interrupt_status.store(true);
	wdg_IRQ = true;
	ARM_TIMERS(printf("%s: watchdog sent interrupt @ ", name()); cout << sc_time_stamp() << endl);
      }
      
      if(!wdg_enabled.load() || wdg_mode.load())
	break;

      if(!wdg_mode.load() && wdg_auto_reload.load())
	continue;
    }
  }
}

arm_ttc::arm_ttc(sc_module_name name) 
  : base_timer(name) {
  ttc_target0.register_b_transport(this, &arm_ttc::b_access);
  
  char lname[64];
  snprintf(lname, 64, "TTC_Registers");

  for(int i = 0; i < 3; i++) {
    clock_control[i] = 0x0;
    counter_control[i] = 0x21;
    counter_value[i].store(0x0);
    interval_counter[i] = 0x0;
    for(int j = 0; j < 3; j++)
      match_counter[i][j] = 0x0;
    interrupt_register[i] = 0x0;
    interrupt_enable[i] = 0x0;
    event_control_timer[i] = 0x0;
    event_register[i] = 0x0;
    timer_interval[i] = SC_ZERO_TIME;
    timer_started[i] = SC_ZERO_TIME;
    timer_counter[i] = 1;
    interrupt_state[i] = false;
  }


  SC_THREAD(run_timer1);
  SC_THREAD(run_timer2);
  SC_THREAD(run_timer3);

}

void arm_ttc::run_timer1() {
  run_timer(0);
}

void arm_ttc::run_timer2() {
  run_timer(1);
}
void arm_ttc::run_timer3() {
  run_timer(2);
}

// we always want to start on clock edge
sc_time arm_ttc::calculate_reminder(int id) {
  assert(timer_interval[id] != SC_ZERO_TIME);
  uint64_t current_time = static_cast<uint64_t>(sc_time_stamp().to_seconds()*1e12);
  uint64_t periphclk = static_cast<uint64_t>(timer_interval[id].to_seconds()*1e12);
  uint64_t remainder = current_time % periphclk;

  return sc_time(remainder, SC_PS);
}

void arm_ttc::run_timer(int id) {
  sc_event_or_list start;
  start |= timer_enabled[id] | reset_event[id];

  while(true) {
    wait(start);

    // check to see if we need to restart counter value
    // 4th bit in counter control register
    if(counter_control[id] & 0x10) {
      counter_control[id] &= ~0x10;
    }

    bool match = !!(counter_control[id] & 0x8);
    bool decrement = !!(counter_control[id] & 0x4);
    bool interval = !!(counter_control[id] & 0x2);
    if(interval) 
      timer_counter[id] = interval_counter[id] + 1;
    else {
      timer_counter[id] = 0xFFFF + 1;
    }
    
    bool interval_interrupt = !!(interrupt_enable[id] & 0x1);
    bool match1_interrupt = !!(interrupt_enable[id] & 0x2);
    bool match2_interrupt = !!(interrupt_enable[id] & 0x4);
    bool match3_interrupt = !!(interrupt_enable[id] & 0x8);
    bool overflow_interrupt = !!(interrupt_enable[id] & 0x10);
    
    assert(timer_interval[id] != SC_ZERO_TIME);
    // check to see if current time is a multiple of PERIPHCLK 
    // if it is not then wait until we get there
    sc_time remainder = calculate_reminder(id);
    ARM_TIMERS(printf("%s: about to enable ttc %d after ", name(), id); cout << remainder << endl);

    wait(remainder);

    ARM_TIMERS(printf("%s: ttc %d enabled @ ", name(), id); cout << sc_time_stamp() << endl);
    ARM_TIMERS(printf("%s: ttc %d -> modes: match(%d), decrement(%d), interval(%d)\n", name(), id, match, decrement, interval));
    ARM_TIMERS(printf("%s: ttc %d -> interrupts: interval(%d), match(%d,%d,%d), overflow(%d)\n", 
                     name(), id, interval_interrupt, match1_interrupt, match2_interrupt, match3_interrupt, overflow_interrupt));

    timer_started[id] = sc_time_stamp();

    while(true) {
      // We are going to recalculate timer interval here
      // If no interrupts are enabled we are going to wait
      // for either reset or disable signal before waking
      // up this thread again, while if there is interrupt enabled
      // then we are going to wake it up when we are supposed 
      // to send the interrupt
      
      // TODO: MP to implement once we hit this use case
      // should be just simple arithmetics to decide when to wake up
      assert(!match1_interrupt && !match2_interrupt && !match3_interrupt);

      sc_time wait_interval = SC_ZERO_TIME;
      if(!interval_interrupt && !overflow_interrupt) {
	// here we are waitng for event that we either 
	// need to restart or disable, don't need to send
	// any interrupts
	wait(start);
      } else {
        assert(timer_interval[id] != SC_ZERO_TIME);
	if(interval_interrupt && !overflow_interrupt) {
	  // we are going to wake up every time we count to value in interval counter register
	  wait_interval = timer_interval[id]*timer_counter[id];
	} 
	else if(overflow_interrupt && !interval_interrupt) {
	  wait_interval = timer_interval[id]*timer_counter[id];
	}
	else {
	  // TODO: MP not aware of use cases where both interrupts
	  // can be enabled, if we hit this case let him know
	  assert(0);
	}
	
	wait(wait_interval, start);
      }

      // check to see if we have received reset or disable
      // disable
      if(counter_control[id] & 0x1) {
        ARM_TIMERS(printf("%s: ttc %d disabled @ ", name(), id); cout << sc_time_stamp() << endl);
	break; // we break out of loop and wait to be restarted or enabled again
      }
      else if(counter_control[id] & 0x10) {
	// reset
	sc_time remainder = calculate_reminder(id);
	wait(remainder);

	timer_started[id] = sc_time_stamp();
	continue;
      }

      bool send_interrupt = false;
      if(interval_interrupt) {
	interrupt_register[id] |= 0x1; // 1st bit
	ARM_TIMERS(printf("%s: ttc %d will send interval interrupt @ ", name(), id); cout << sc_time_stamp() << endl);
	send_interrupt = true;
      }
      else if(overflow_interrupt) {
        ARM_TIMERS(printf("%s: ttc %d will send overflow interrupt @ ", name(), id); cout << sc_time_stamp() << endl);
	interrupt_register[id] |= 0x10; // 5th bit
	send_interrupt = true;
      }

      if(send_interrupt) {
	interrupt_state[id] = true;
	IRQ[id] = interrupt_state[id];
      }

    }

  }
}

void arm_ttc::b_access(int idx, PRAZOR_GP_T &trans, sc_time &delay) {
  tlm_command cmd = trans.get_command(); 
  u32_t adr = (u32_t)trans.get_address();
  u8_t* ptr = trans.get_data_ptr();

  u32_t offset = adr - TTC0_BASE_ADDR;

  switch(cmd) {
    case TLM_READ_COMMAND: {
      assert(trans.get_data_length() == 4);

      switch(offset) {
        case 0x0:
        case 0x4:
        case 0x8: {
	  u8_t index = (u8_t)(offset/4);
	  u32_t data = (u32_t)(clock_control[index]);
	  memcpy(ptr, (u8_t*)(&data), 4);
	  break;
	}
        case 0xC:
        case 0x10:
        case 0x14: {
	  u8_t index = (u8_t)((offset-0xC)/4);
	  u32_t data = (u32_t)(counter_control[index]);
	  memcpy(ptr, (u8_t*)(&data), 4);
	  break;
	}
        case 0x18:
        case 0x1C:
        case 0x20: {
	  u8_t index = (u8_t)((offset-0x18)/4);
	  // we calculate here what value of the counter value should be
	  // as in order to speed up simulation we are not updating it
	  // on every positive clock edge
	  uint64_t current_time = static_cast<uint64_t>(sc_time_stamp().to_seconds()*1e12);
	  uint64_t timer_start_time = static_cast<uint64_t>(timer_started[index].to_seconds()*1e12);
	  uint64_t interval = static_cast<uint64_t>(timer_interval[index].to_seconds()*1e12);

	  u32_t time = floor((current_time-timer_start_time)/interval);
	  u32_t value = time % timer_counter[index];

	  memcpy(ptr, (u8_t*)(&value), 4);
	  break;
	}
        case 0x24:
        case 0x28:
        case 0x2C: {
	  u8_t index = (u8_t)((offset-0x20)/4);
	  u32_t data = (u32_t)(interval_counter[index]);
	  memcpy(ptr, (u8_t*)(&data), 4);
	  break;
	}
        case 0x30:
        case 0x34:
        case 0x38:
        case 0x3C:
        case 0x40:
        case 0x44:
        case 0x48:
        case 0x4C:
        case 0x50: {
	  u8_t index = (u8_t)((offset - 0x30)/4);
	  u8_t i = (u8_t)floor(index/3);
	  u8_t j = index % 3;
	  u32_t data = match_counter[i][j];
	  
	  memcpy(ptr, (u8_t*)(&data), 4);
	  
	  break;
	}
        case 0x54:
        case 0x58:
        case 0x5C: {
	  u8_t index = (u8_t)((offset - 0x54)/4);
	  u32_t data = interrupt_register[index];

	  memcpy(ptr, (u8_t*)(&data), 4);

	  // clear interrupt registers after reading
	  interrupt_register[index] = 0x0;
	  // notify that interrupt was acknowledged
	  interrupt_reset_event[index].notify();

	  break;
	}

        case 0x60:
        case 0x64:
        case 0x68: {
	  u8_t index = (u8_t)((offset - 0x60)/ 4);
	  u32_t data = interrupt_enable[index];
	  
	  memcpy(ptr, (u8_t*)(&data), 4);
	  
	  break;
	}

        case 0x6C:
        case 0x70:
        case 0x74: {
	  u8_t index = (u8_t)((offset - 0x6C)/4);
	  u32_t data = event_control_timer[index];
	  
	  memcpy(ptr, (u8_t*)(&data), 4);

	  break;
	}

        case 0x78:
        case 0x7C:
        case 0x80: {
	  u8_t index = (u8_t)((offset - 0x78)/4);
	  u32_t data = event_register[index];
	  
	  memcpy(ptr, (u8_t*)(&data), 4);

	  break;
	}
      }
      ARM_TIMERS(printf("%s: read addr 0x%08x, offset 0x%08x, data 0x%08x, length %d\n",
                       name(), adr, offset, *((u32_t*)ptr), trans.get_data_length()));

      break;
    }
    case TLM_WRITE_COMMAND: {
      u32_t data = ((u64_t*)ptr)[0];
      u8_t* lanes = trans.get_byte_enable_ptr();
      assert(trans.get_data_length() == 8);
      if(lanes && lanes[4] == 0xFF) {
	// we need to increment address by 4
	// because data length is 8
	adr += 4;
	// since we have changed the address
	// we also need to recalculate the offset
	offset = adr - TTC0_BASE_ADDR;
      }

      ARM_TIMERS(printf("%s: write addr 0x%08x, offset 0x%08x, data 0x%08x, length %d, lanes: ", \
			name(), adr, offset, *((u32_t*)ptr), trans.get_data_length()); \
		 if(lanes) {						\
		   for(int i = 0; i < trans.get_data_length(); i++)	\
		     printf("%02x", lanes[i]);				\
		 } else {						\
		   printf("0x0");					\
		 }							\
		 printf("\n"));

      switch(offset) {
        case 0x0:
        case 0x4:
        case 0x8: {
	  u8_t index = (u8_t)(offset / 4);
	  // TODO: we only have implementation when clock
	  // input is internal (i.e. CPU_1x), need to add
	  // logic for external clock input
	  // 6th and 5th bit should be 0, if they are 1 we are using external clock
	  assert(!(data & 0x40) && !(data & 0x20));
	  u64_t prescaler = pow(2, ((data & 0x1E) >> 1) + 1);
	  if(data & 0x1) {
	    // we need to prescale the clock source
            assert(timer_interval[index] == SC_ZERO_TIME);
            timer_interval[index] = calculate_period(6); // CPU_1x
            timer_interval[index] *= prescaler;
	  }
	  clock_control[index] = (u7_t)data;
	  break;
	}

        case 0xC:
        case 0x10:
        case 0x14: {
	  u8_t index = (u8_t)((offset - 0xC)/4);
	  u7_t old_control = counter_control[index];
	  counter_control[index] = (u7_t)data;
	  bool enable_changed = (counter_control[index] & 0x1) != (old_control & 0x1);
	  ARM_TIMERS(printf("%s: enable chaged %d for index %d --> %d vs %d, %08x\n", 
			    name(), enable_changed, index, (counter_control[index] & 0x1), (old_control & 0x1), data));
	  if(enable_changed && (counter_control[index] & 0x10)) {
	    // if they were both set only send one notification
	    timer_enabled[index].notify();
	  }
	  else if(enable_changed) {
	    timer_enabled[index].notify();
	  }
	  else if(counter_control[index] & 0x10) {
	    reset_event[index].notify();
	  }
	  break;
	}
	  
        case 0x18:
        case 0x1C:
        case 0x20: {
	  // we should never programatically write to counter value register
	  assert(0);
	  break;
	}

        case 0x24:
        case 0x28:
        case 0x2C: {
	  u8_t index = (u8_t)((offset-0x24)/4);
	  interval_counter[index] = (u16_t)data;
	  break;
	}

        case 0x30:
        case 0x34:
        case 0x38:
        case 0x3C:
        case 0x40:
        case 0x44:
        case 0x48:
        case 0x4C:
        case 0x50: {
	  u8_t index = (u8_t)((offset - 0x30)/4);
	  u8_t i = (u8_t)floor(index/3);
	  u8_t j = index % 3;
	  match_counter[i][j] = (u16_t)data;
	  
	  break;
	}

        case 0x54:
        case 0x58:
        case 0x5C: {
	  // we should never programmaticaly write to interrupt registers
	  assert(0); 
	  
	  break;
	}

        case 0x60:
        case 0x64:
        case 0x68: {
	  u8_t index = (u8_t)((offset - 0x60)/ 4);
	  interrupt_enable[index] = (u6_t)(data);

	  break;
	}
        case 0x6C:
        case 0x70:
        case 0x74: 
        case 0x78:
        case 0x7C:
        case 0x80: {
	  // TODO: event timer yet to be implemented
	  
	  assert(0);
	  break;
	}

        default: {
	  assert(0); // unknown offset
	}

      }

      break;
    }

    default:
      assert(0); // should never hit
  }

  trans.set_response_status(TLM_OK_RESPONSE);
}

arm_glbt::arm_glbt(sc_module_name name, int cores) 
  : base_timer(name),
    n_cores(cores) {

  timer_counter = 0;
  timer_enabled = false;

  for(int i = 0; i < GIC_MAX_N_CPUS; i++) {
    comp_enabled[i] = false;
    irq_enabled[i] = false;
    auto_increment[i] = false;
    
    interrupt_status[i] = false;

    comparator[i] = 0;

    auto_increment_value[i] = 0;
  }

  timer_started = SC_ZERO_TIME;
  timer_interval = SC_ZERO_TIME; 
  timer_interval_core = -1;

  glb_target.register_b_transport(this, &arm_glbt::b_access);

  SC_THREAD(run);
}    

void arm_glbt::b_access(int idx, PRAZOR_GP_T &trans, sc_time &delay) {
  tlm_command cmd = trans.get_command();
  u32_t adr = (u32_t)trans.get_address();
  u8_t* ptr = trans.get_data_ptr();

  switch(cmd) {
    case TLM_READ_COMMAND: {
      assert(trans.get_data_length() == 4);
      u32_t offset = adr - GLBT_BASE_ADDR;
      switch(offset) {
        case 0x0:
        case 0x4: {
	  u64_t data;
	  if(timer_enabled) {
	    // need to calculate value
	    data = calculate_timer_counter();
	  } else {
	    // read from the counter register
	    data = timer_counter;
	  }

	  u32_t toread;
	  if(offset == 0x0) {
	    // we are reading lower bits
	    toread = (u32_t)((data << 32) >> 32);
	  } else {
	    // we are reading upper bits
	    toread = (u32_t)(data >> 32);
	  }
	  
	  memcpy(ptr, (u8_t*)(&toread), 4);

	  break;
	}

        case 0x8: {
	  u32_t data = 0;
	  data |= (prescaler << 8) 
	    | auto_increment[idx] << 3 
	    | irq_enabled[idx] << 2
	    | comp_enabled[idx] << 1
	    | timer_enabled;

	  memcpy(ptr, (u32_t*)(&data), 4);

	  break;
        }

        case 0xC: {
	  u32_t data = interrupt_status[idx];
	  
	  memcpy(ptr, (u32_t*)(&data), 4);

	  break;
        }

        case 0x10: {
	  u32_t data = (u32_t)((comparator[idx] << 32) >> 32);
	  
	  memcpy(ptr, (u32_t*)(&data), 4);

	  break;
	  
        }
        case 0x14: {
	  u32_t data = (u32_t)((comparator[idx] << 32) >> 32);
	  
	  memcpy(ptr, (u32_t*)(&data), 4);
	  

	  break;
        }

        case 0x18: {
	  memcpy(ptr, (u32_t*)(&(auto_increment_value[idx])), 4);

	  break;
        }

      }
      break;
    }
    case TLM_WRITE_COMMAND: {
      u32_t data = ((u64_t*)ptr)[0];
      u8_t* lanes = trans.get_byte_enable_ptr();
      assert(trans.get_data_length() == 8);
      if(lanes && lanes[4] == 0xFF) {
	// we need to increment address by 4
	// because data length is 8
	adr += 4;
      }
	// since we have changed the address
	// we also need to recalculate the offset
      u32_t offset = adr - GLBT_BASE_ADDR;

      switch(offset) {
        case 0x0:
        case 0x4: {
	  if(timer_enabled) {
	    sc_time remainder = calculate_reminder();
	    timer_started = sc_time_stamp() - remainder;
	  }
	  
	  if(offset == 0x0) {
	    timer_counter = ((timer_counter >> 32) << 32) | data;
	  }
	  else if(offset == 0x4) {
	    timer_counter = ((u64_t)data << 32) | ((timer_counter << 32) >> 32);
	  }

	  break;
        }

        case 0x8: {
	  prescaler = (u8_t)((data >> 8) & 0xFF);
	  auto_increment[idx] = !!(data & 0x8);
	  irq_enabled[idx] = !!(data & 0x4);
	  comp_enabled[idx] = !!(data & 0x2);

	  bool enable_flag = !!(data & 0x1);
	  if(enable_flag != timer_enabled) 
	    timer_enabled_event.notify();
	  timer_enabled = enable_flag;

	  break;
        }

        case 0xC: {
	  assert(data == 1);
	  interrupt_status[idx] = false;
	  break;
        }

        case 0x10: {
	  comparator[idx] = ((comparator[idx] >> 32) << 32) | data;
	  break;
        }

        case 0x14: {
	  comparator[idx] = ((u64_t)data << 32) | ((comparator[idx] << 32) >> 32);
	  break;
        }

        case 0x18: {
	  auto_increment_value[idx] = data;
	  break;
        }
      }

      break;
    }
    default:
      assert(0); // unknown command, should never reach this
  }
}


// we always want to start on clock edge
sc_time arm_glbt::calculate_reminder() {
  uint64_t current_time = static_cast<uint64_t>(sc_time_stamp().to_seconds()*1e12);
  uint64_t periphclk = static_cast<uint64_t>(timer_interval.to_seconds()*1e12);
  uint64_t remainder = current_time % periphclk;

  return sc_time(remainder, SC_PS);
}

void arm_glbt::run() {
  while(true) {
    wait(timer_enabled_event);

    assert(timer_interval == SC_ZERO_TIME);
    timer_interval = calculate_period(2); // CPU_3x2x
    sc_time remainder = calculate_reminder();
    sc_time wait_time = timer_interval - remainder;

    wait(wait_time);

    ARM_TIMERS(printf("%s: global timer started @ ", name()); cout << sc_time_stamp() << endl);
    
    timer_started = sc_time_stamp();

    while(true) {
      sc_time interval = calculate_timer_interval();
      
      if(interval > SC_ZERO_TIME) {
	wait(interval);

	assert(timer_interval_core != -1);
	if(irq_enabled[timer_interval_core]) {
	  interrupt_status[timer_interval_core] = true;
	  IRQ[timer_interval_core] = true;
	  ARM_TIMERS(printf("%s: sent interrupt by core %d", name(), timer_interval_core); cout << sc_time_stamp() << endl);
	}
	
	if(auto_increment[timer_interval_core])
	  comparator[timer_interval_core] += auto_increment_value[timer_interval_core];

	timer_interval_core = -1;
      }
      else
	wait(timer_enabled_event);
      
      if(!timer_enabled) {
	timer_counter = calculate_timer_counter();
	break;
      }

    }
  }
}

sc_time arm_glbt::calculate_timer_interval() {
  u64_t minimum_comparator = pow(2,63) - 1;
  int i = 0;
  u64_t calculated_timer_counter = calculate_timer_counter();
  for(; i < n_cores; i++)
    if(comp_enabled[i] && comparator[i] < minimum_comparator)
      minimum_comparator = comparator[i] - calculated_timer_counter;

  if(minimum_comparator <= 0)
    return SC_ZERO_TIME;

  double interval = ((prescaler + 1) * (minimum_comparator + 1))*(timer_interval.to_seconds()*1e9);

  timer_interval_core = i;
  //ARM_TIMERS(printf("%s: calculated timer interval is %.4f ns\n", name(), interval));
  return sc_time(interval, SC_NS);
}

u64_t arm_glbt::calculate_timer_counter() {
  uint64_t current_time = static_cast<uint64_t>(sc_time_stamp().to_seconds()*1e12);
  uint64_t timer_start_time = static_cast<uint64_t>(timer_started.to_seconds()*1e12);
  uint64_t interval = static_cast<uint64_t>(timer_interval.to_seconds()*1e12);
  
  u64_t data = floor((current_time-timer_start_time)/interval) + timer_counter;

  return data;
}

arm_swdt::arm_swdt(sc_module_name name) 
  : base_timer(name) {
  // register MODE
  zkey = 0x0;
  irqln = 0x3;
  irqen = false;
  rsten = false;
  wden = false;

  // register CONTROL
  ckey = 0x0;
  crv = 0xFFF;
  clksel = 0x0;

  // register RESTART
  rstkey = 0x0;

  // register STATUS
  wdz = false;

  swdt_period = SC_ZERO_TIME;
  
  swdt_target.register_b_transport(this, &arm_swdt::b_access);

  SC_THREAD(run);
}

void arm_swdt::b_access(int idx, PRAZOR_GP_T &trans, sc_time &delay) {
  tlm_command cmd = trans.get_command(); 
  u32_t adr = (u32_t)trans.get_address();
  u8_t* ptr = trans.get_data_ptr();

  switch(cmd) {
    case TLM_READ_COMMAND: {
      assert(trans.get_data_length() == 4);
      u32_t offset = adr - SWDT_BASE_ADDR;

      switch(offset) {
        case 0x0: {
	  u32_t data = 0;
	  data |= ((u32_t)zkey << 12) 
	    | (irqln << 7)
	    | (0x4 << 4)
	    | (irqen << 2)
	    | (rsten << 1)
	    | (wden);

	  memcpy(ptr, (u8_t*)(&data), 4);

	  break;
        }

        case 0x4: {
	  u32_t data = 0;
	  data |= ((u32_t)ckey << 14)
	    | (crv << 2)
	    | clksel;
	  
	  memcpy(ptr, (u8_t*)(&data), 4);

	  break;
        }

        case 0x8: {
	  u32_t data = rstkey;

	  memcpy(ptr, (u8_t*)(&data), 4);

	  break;
        }
	
        case 0xC: {
	  u32_t data = wdz;

	  memcpy(ptr, (u8_t*)(&data), 4);

	  break;
        }
      }

      break;
    }

    case TLM_WRITE_COMMAND: {
      u32_t data = ((u64_t*)ptr)[0];
      u8_t* lanes = trans.get_byte_enable_ptr();
      assert(trans.get_data_length() == 8);
      if(lanes && lanes[4] == 0xFF) {
	// we need to increment address by 4
	// because data length is 8
	adr += 4;
      }
      // since we have changed the address
      // we also need to recalculate the offset
      u32_t offset = adr - SWDT_BASE_ADDR;

      switch(offset) {
        case 0x0: {
	  zkey = (data >> 12) & 0xFFF;
	  if(zkey == 0xABC) {
	    irqln = (data >> 7) & 0x3;
	    irqen = data & 0x4;
	    rsten = data & 0x2;
	    wden = data & 0x1;
	  }

	  break;
        }

        case 0x4: {
	  ckey = (data >> 14) & 0xFFF;
	  if(ckey == 0x248) {
	    crv = (data >> 2) & 0xFFF;
	    clksel = data & 0x3;
	  }

	  break;
        }

        case 0x8: {
	  rstkey = (u16_t)data;
	  if(rstkey == 0x1999)
	    reset_event.notify();
	  
	  break;
        }

        case 0xC: {
	  // we do not write to this register it
	  // is set when the watchdog reaches zero count
	  assert(0);
        }
      }
      break;
   }
  }

  // TODO there is no AUGMENT_LT_DELAY() call here!
}

void arm_swdt::run() {
  while(true) {
    assert(swdt_period == SC_ZERO_TIME);
    swdt_period = calculate_period(6); // CPU_1x
    sc_time timer_interval = calculate_timer_interval();
    wait(timer_interval);

    wdz = true;

    if(wden && irqen) {
      IRQ = true;
      // this signal is asserted for irqln pclk cycles
      // for now we just wait that long
      wait(swdt_period * (u32_t)pow(2, (irqln + 2)));
    }

    wait(reset_event);
  }
}

// we always want to start on clock edge
sc_time arm_swdt::calculate_reminder() {
  uint64_t current_time = static_cast<uint64_t>(sc_time_stamp().to_seconds()*1e12);
  assert(swdt_period != SC_ZERO_TIME);
  uint64_t periphclk = static_cast<uint64_t>(swdt_period.to_seconds()*1e12);
  uint64_t remainder = current_time % periphclk;

  return sc_time(remainder, SC_PS);
}

sc_time arm_swdt::calculate_timer_interval() {
  assert(swdt_period != SC_ZERO_TIME);
  double periphclk = swdt_period.to_seconds()*1e9;
  double counter = (crv << 12) | 0xFFF;
  double division;
  double interval = (counter + 1)/(periphclk/(pow(8, (clksel+1))));
  
  return sc_time(interval, SC_NS);
  
}

// eof
