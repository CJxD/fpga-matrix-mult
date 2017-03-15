// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>

#ifndef VHLS_SOC_DEBUG_H
#define VHLS_SOC_DEBUG_H

#include "pthread.h"
#include "systemc.h"
#include "vhls_soc_debug_spr.h"


enum core_control_cmd_t { ResetC=0, PauseC=1, RunC=2, SingleStepC=3 } ;

class VHLSDebugIF // abstract interface
{
public:
  // Reset function for the debug unit                                                          
  virtual void resetDebugUnit() = 0;

  // Functions to control and report on the CPU        
  virtual void control_cmd(core_control_cmd_t cmd, int except = -1, u64_t addr = 0) = 0;

  virtual bool isStalled(int *rsp_sigval_ptr) = 0;

  virtual int setThread(int letter, int tid) = 0;

  virtual u64_t readSpr(uint16_t sprNum) = 0;

  virtual int writeSpr(uint16_t sprNum, u64_t value) = 0;

  virtual void andSpr(uint16_t sprNum, u64_t value) = 0;

  virtual void orSpr(uint16_t sprNum, u64_t value) = 0;

   
  // Functions to access memory                     
  virtual u64_t readMem32(u64_t addr) = 0;
  virtual bool writeMem32(u64_t addr, u64_t value) = 0;
  virtual uint8_t readMem8(u64_t addr) = 0;
  virtual bool writeMem8(u64_t addr, uint8_t value) = 0;
};



class vhls_debug_comms
{
  pthread_mutex_t comms_mutex;
 public:
  typedef enum { Idle, Ack, WriteReg, ReadReg, WriteMem, ReadMem, Control, Status, ReadSpr, WriteSpr, AndSpr, OrSpr, InsertMatchpoint, ReadMeta } cmd_t;



  struct {
    u64_t addr;
    u64_t data;
    u64_t lanes;
    volatile cmd_t cmd;
  } debug_cmd;

  void set_cmd(cmd_t cmd);
  void wait_idle();
  u64_t wait_ack(int *r2_ptr=0);

  vhls_debug_comms(sc_core::sc_module_name&); // constructor


  bool debug_op_requested();
};

// Instantiated on the cpu_busaccess to read the debug_comms_channel
class vhls_debug_target
{
 public:

  const char *kind() const { return "vhls_debug_target"; };

  vhls_debug_target(const sc_module_name &mname); // constructor


};


//
// A port that can be externally connected to by a gdb/rsp connection.
// It contains a table of targets so that the external debugger can choose what to connect to. 
// Only one external debugger connection is supported - this port could perhaps be instantiated more than once for parallel debugging?
//

class vhls_debug_port :/*public vhls_debug_fabric_base,*/ public VHLSDebugIF
{
#define MAX_DEBUG_TARGETS 4096
  //std::vector<vhls_debug_target> debug_children;
  vhls_debug_comms *Targets[MAX_DEBUG_TARGETS];
  int current_target;
  int n_targets_registered;
 public:


  class GdbServerSC *TheRspServer;
  const char *kind() const { return "vhls_debug_port"; };
  vhls_debug_port(const sc_module_name &mname); // constructor

  void connect_ip_block(const char *name, vhls_debug_comms *target);

  vhls_debug_comms *select_target(); // get handle on forward link to debug destination.

  // We implement this I/F directly so that our RSP handler can directly use this parent as its target.  Two objects in one really.
  // Functions to control and report on the CPU        
  void resetDebugUnit();

  void control_cmd(core_control_cmd_t cmd, int except= -1, u64_t addr=0);
  void step(bool continuef);

  bool isStalled(int *rsp_sigval_ptr);
  u64_t readSpr(uint16_t sprNum);
  int writeSpr(uint16_t sprNum, u64_t value);
  void andSpr(uint16_t sprNum, u64_t value);
  void orSpr(uint16_t sprNum, u64_t value);

  // Operations on break and watch points.
  const char *MatchpointControl(const char *cmd, int type, u64_t addr);


  // Functions to access memory                     
  u64_t readMem32(u64_t addr);
  bool writeMem32(u64_t addr, u64_t value);
  uint8_t readMem8(u64_t addr);
  bool writeMem8(u64_t addr, uint8_t value);
  int setThread(int letter, int tid);


  // Metainfo
  int max_gprs();
};

#endif
