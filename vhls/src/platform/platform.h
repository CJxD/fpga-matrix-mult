/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

#include <stdio.h>
#include <stdlib.h>
#include <systemc>

class platform {
 public:
  virtual void hup_handler(int p_arg) {
    /* Does nothing */
  }

  virtual void reset(const char*,
		     const char*,
		     const char*,
		     const char*) {
    /* Does nothing */
  }
  
  virtual void stat_report(const char*,
			   FILE*,
			   bool resetf = 0) {
    /* Does nothing */
  }

  virtual sc_core::sc_module* get_Core(int i) {
    /* Does nothing */
    return NULL;
  }
  
  // Constructor -- Does nothing
  platform() {
  }

  // Destructor -- Does nothing
  ~platform() {
  }
};
