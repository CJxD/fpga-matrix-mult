// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>


#include "arm_cortex_a9.h"

// Constructor
arm_cortex_a9::arm_cortex_a9(
  sc_core::sc_module_name name,
  u8_t idx,
  bool harvardf) :
  // Call constructor of inherited class
  armcore_tlm(name, idx, harvardf) {
  // Set to only measure IPC for 16bit Thumb instructions
  set_thumb16_only_ipc(true);
}
