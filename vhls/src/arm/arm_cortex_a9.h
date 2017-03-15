/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */


#ifndef __ARM_CORTEX_A9_H__
#define __ARM_CORTEX_A9_H__

#include "armcore_tlm.h"

class arm_cortex_a9 : public armcore_tlm {

 public:
  // Constructor
  arm_cortex_a9(sc_core::sc_module_name name, // core name
		u8_t idx, // core id
		bool harvardf); // harvard architecture
	       
};

#endif
