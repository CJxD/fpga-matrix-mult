// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
//
// $Id: $
//
//
#include <assert.h>
#include <stdlib.h>
#include "tenos.h"


#include "vhls_soc_debug.h"

#define TRC(X) 

vhls_debug_port::vhls_debug_port(const sc_module_name &mname)  // constructor
  //  vhls_debug_fabric_base(mname, "debug_port")
{
  current_target = 0;
  n_targets_registered = 0;
}


vhls_debug_comms *vhls_debug_port::select_target()
{
  if (current_target >= n_targets_registered)
    {
      printf("Debug targer %i >= number registered (%i)\n", current_target, n_targets_registered);
      assert(0); // TODO use SC_ERROR
    }
  return Targets[current_target];
}


// 
int vhls_debug_port::setThread(int letter, int tid)
{
  printf("Change target letter=%c to %i/%i\n", letter, tid, n_targets_registered);
  if (tid == -1) tid = 0; // Choose any (active?) thread if tid < 0.
  tid = tid % 32; // for now
  if (tid >=0 && tid <n_targets_registered) current_target = tid;
  else printf("Change target non-existant target %i/%i\n", tid, n_targets_registered);
  return tid;
}


void vhls_debug_comms::set_cmd(vhls_debug_comms::cmd_t cmd)
{
  pthread_mutex_lock(&comms_mutex);
  debug_cmd.cmd = cmd;
  pthread_mutex_unlock(&comms_mutex); // stupid condition var like coding! TODO.
}

void vhls_debug_comms::wait_idle()
{
  pthread_mutex_lock(&comms_mutex);
  pthread_mutex_unlock(&comms_mutex); // stupid condition var like coding! TODO.
  
  while(debug_cmd.cmd != Idle) wait();
}

u64_t vhls_debug_comms::wait_ack(int *r2_ptr)
{
  pthread_mutex_lock(&comms_mutex); // stupid condition var like coding! TODO.
  pthread_mutex_unlock(&comms_mutex); // stupid condition var like coding! TODO.
  sc_time dally (1,SC_US);
  while(debug_cmd.cmd != Ack) wait(dally);
  pthread_mutex_lock(&comms_mutex); // stupid condition var like coding! TODO.
  
  if (r2_ptr) *r2_ptr = debug_cmd.addr; // A second answer comes back in this field for some cmds.
  u64_t ans = debug_cmd.data;
  debug_cmd.cmd = Idle;
  pthread_mutex_unlock(&comms_mutex); // stupid condition var like coding! TODO.
  return ans;
}


  // Reset function for the debug unit                                                          
void vhls_debug_port::resetDebugUnit()
{ 
}


// Functions to control and report on the CPU        
void vhls_debug_port::control_cmd(core_control_cmd_t cmd, int except, u64_t addr)
{ 
  vhls_debug_comms *target = select_target();
  target->wait_idle();
  TRC(printf("vhls_debug_port: Target %p  core_control_cmd %i\n", target, cmd)); 
  target->debug_cmd.addr = addr;
  // oh ... where to put except!!! need a further field.
  target->debug_cmd.data = cmd;
  target->set_cmd(vhls_debug_comms::cmd_t::Control);
  target->wait_ack();
  TRC(printf("vhls_debug_port: Target %p stall done\n", target)); 
}

void vhls_debug_port::step(bool f)
{
  control_cmd(SingleStepC);
}


bool vhls_debug_port::isStalled(int *rsp_sigval)
{ 
  vhls_debug_comms *target = select_target();
  target->wait_idle();
  TRC(printf("vhls_debug_port: Target %p status\n", target)); 
  target->set_cmd(vhls_debug_comms::cmd_t::Status);
  target->debug_cmd.data = RunC;
  u64_t ans = target->wait_ack(rsp_sigval);
  TRC(printf("vhls_debug_port: Target %p status done\n", target)); 
  return (ans != 0);
}

int vhls_debug_port::max_gprs()
{
  vhls_debug_comms *target = select_target();
  target->wait_idle();
  TRC(printf("vhls_debug_port: Target %p status\n", target)); 
  target->set_cmd(vhls_debug_comms::cmd_t::ReadMeta);
  target->debug_cmd.addr = 0; // Meta reg 0: Read magic no
  u64_t magic_no = target->wait_ack();
  TRC(printf("vhls_debug_port: Target %p meta magic %x\n", target, magic_no)); 
  int r = -1;
  switch ((magic_no >> 8) & 0xFF)
    {
    case 1: // ARM
      r = 16;
      break;
    case 2: // OpenRISC
      r = 32;
      break;
    default:
      assert(0);
    }
  
  return r;
}



u64_t vhls_debug_port::readSpr(uint16_t sprNum)
{ 
  vhls_debug_comms *target = select_target();
  target->wait_idle();
  TRC(printf("vhls_debug_port: Target %p readSpr $%x\n", target, sprNum)); 
  target->debug_cmd.addr = sprNum;
  target->set_cmd(vhls_debug_comms::cmd_t::ReadSpr);
  u64_t ans = target->wait_ack();
  TRC(printf("vhls_debug_port: Target %p readSpr done. " PFX64 "\n", target, ans)); 
  return ans;
}


int vhls_debug_port::writeSpr(uint16_t sprNum, u64_t value)
{ 
  vhls_debug_comms *target = select_target();
  target->wait_idle();
  TRC(printf("vhls_debug_port: Target %p WriteSpr\n", target)); 
  target->debug_cmd.addr = sprNum;
  target->debug_cmd.data = value;
  target->set_cmd(vhls_debug_comms::cmd_t::WriteSpr);
  target->wait_ack();
  TRC(printf("vhls_debug_port: Target %p writeSpr done\n", target)); 
  return 1;
}



void vhls_debug_port::andSpr(uint16_t sprNum, u64_t value)
{ 
  vhls_debug_comms *target = select_target();
  target->wait_idle();
  TRC(printf("vhls_debug_port: Target %p andreg\n", target)); 
  target->debug_cmd.addr = sprNum;
  target->debug_cmd.data = value;
  target->set_cmd(vhls_debug_comms::cmd_t::AndSpr);
  target->wait_ack();
  TRC(printf("vhls_debug_port: Target %p andreg done\n", target)); 
}



void vhls_debug_port::orSpr(uint16_t sprNum, u64_t value)
{ 
  vhls_debug_comms *target = select_target();
  target->wait_idle();
  TRC(printf("vhls_debug_port: Target %p orSpr\n", target)); 
  target->debug_cmd.addr = sprNum;
  target->debug_cmd.data = value;
  target->set_cmd(vhls_debug_comms::cmd_t::OrSpr);
  target->wait_ack();
  TRC(printf("vhls_debug_port: Target %p orSpr done\n", target)); 
}



   
// Functions to access memory                     
u64_t vhls_debug_port::readMem32(u64_t addr)
{ 
  vhls_debug_comms *target = select_target();
  target->wait_idle();
  TRC(printf("vhls_debug_port: Target %p readmem32\n", target)); 
  target->debug_cmd.lanes = 4;
  target->debug_cmd.addr = addr;
  target->set_cmd(vhls_debug_comms::cmd_t::ReadMem);
  u64_t ans = target->wait_ack();
  TRC(printf("vhls_debug_port: Target %p readmem32 done\n", target)); 
  return ans;
}


bool vhls_debug_port::writeMem32(u64_t addr, u64_t value)
{ 
  vhls_debug_comms *target = select_target();
  target->wait_idle();
  TRC(printf("vhls_debug_port: Target %p writeMem32\n", target)); 
  target->debug_cmd.addr = addr;
  target->debug_cmd.data = value;
  target->debug_cmd.lanes = 4;
  target->set_cmd(vhls_debug_comms::cmd_t::WriteMem);
  target->wait_ack();
  TRC(printf("vhls_debug_port: Target %p writeMem32 done\n", target)); 
  return 1;
}


uint8_t vhls_debug_port::readMem8(u64_t addr)
{ 
  vhls_debug_comms *target = select_target();
  target->wait_idle();
  TRC(printf("vhls_debug_port: Target %p readMem8\n", target)); 
  target->debug_cmd.addr = addr;
  target->debug_cmd.lanes = 1;
  target->set_cmd(vhls_debug_comms::cmd_t::ReadMem);
  u64_t ans = target->wait_ack();
  TRC(printf("vhls_debug_port: Target %p readMem8 done\n", target)); 
  return ans;
}


bool vhls_debug_port::writeMem8(u64_t addr, uint8_t value)
{ 
  vhls_debug_comms *target = select_target();
  target->wait_idle();
  TRC(printf("vhls_debug_port: Target %p writeMem8", target)); 
  target->debug_cmd.addr = addr;
  target->debug_cmd.data = value;
  target->debug_cmd.lanes = 1;
  target->set_cmd(vhls_debug_comms::cmd_t::WriteMem);
  target->wait_ack();
  TRC(printf("vhls_debug_port: Target %p writeMem8 done\n", target)); 
  return 1; // 1 for ok
}

const char *vhls_debug_port::MatchpointControl(char const *cmd, int type, u64_t addr)
{
  vhls_debug_comms *target = select_target();
  target->wait_idle();
  TRC(printf("vhls_debug_port: Target %p MatchpointControl cmd=%s type=%i addr=" PFX64 "\n", target, cmd, type, addr)); 
  target->debug_cmd.addr = addr;
  target->debug_cmd.data = type;
  target->debug_cmd.lanes = cmd[0];
  target->set_cmd(vhls_debug_comms::cmd_t::InsertMatchpoint);
  target->wait_ack();
  TRC(printf("vhls_debug_port: Target %p Matchpoint cmd done\n", target)); 
  return (target->debug_cmd.data == 0) ? "OK": "E01";
}


void vhls_debug_port::connect_ip_block(char const *name_, vhls_debug_comms *ip_block)
{
  assert(n_targets_registered < MAX_DEBUG_TARGETS);
  Targets[n_targets_registered++] = ip_block;
}
 
vhls_debug_comms::vhls_debug_comms(sc_core::sc_module_name&) // constructor
{
  pthread_mutex_init(&comms_mutex, 0);
}

bool vhls_debug_comms::debug_op_requested()
{
  //printf("Check op_req %i\n", debug_cmd.cmd);
  return debug_cmd.cmd != Idle && debug_cmd.cmd != Ack;
}

vhls_debug_target:: vhls_debug_target(const sc_module_name &mname) 
{


}
// eof
