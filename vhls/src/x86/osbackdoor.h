/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

#ifndef OSBACKDOOR_H
#define OSBACKDOOR_H


// This header file exists purely since there is a nameclash with importing SystemC into ptlsim
// Having this header allows the parts that need it to import it in isolation.

class osbackdoor
{
  void *_x86core; 
 public:
  int os_cmd(const char cmd, char *data, long long int addr, int len);

  void os_svc_exit(int arg);

  // Constructor
 osbackdoor(void *a): _x86core(a) { };
};

#endif
