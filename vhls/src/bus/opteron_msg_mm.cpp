// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2012 M Puzovic and D J Greaves
// University of Cambridge, Computer Laboratory.
// $Id: $

// AMD Opteron-like system messages implementation file
#include "opteron_messages.h"

opteron_msg_mm::opteron_msg_mm()
  : free_list(0), empties(0), lck("opteron_msg_mm")
#ifdef OPTERON_DEBUG
  , count(0)
#endif
{
}

PRAZOR_GP_T* opteron_msg_mm::allocate() {
#ifdef OPTERON_DEBUG
  lck.lock();
  fprintf(stdout, "---------- Called allocate(), #trans = %d\n", ++count);
  lck.unlock();
#endif

  PRAZOR_GP_T* ptr;
  lck.lock();
  if(free_list) {
    ptr = free_list->trans;
    empties = free_list;
    free_list = free_list->next;
  }
  else
    {
      assert(0);
      // need help with this line to get it to compile : ptr = new (PRAZOR_GP_T)(this);
      ptr = 0;
    }
  lck.unlock();

  // MP: I thought that I do some extra release that decrements the ref count
  // to negative value but couldn't find any of them, maybe bug in TLM
  if(ptr->get_ref_count() < 0) {
    while(ptr->get_ref_count() != 0)
      ptr->acquire();
  }
  
  assert(ptr->get_ref_count() == 0);

  return ptr;
}

void opteron_msg_mm::free(PRAZOR_GP_T* trans) 
{
#ifdef OPTERON_DEBUG
  lck.lock();
  fprintf(stdout, "tid=%d: ---------- Called free(0x%lx), #trans = %d\n", 
	  sc_get_current_process_b()->proc_id, trans, --count);
  lck.unlock();
#endif
  sri_msg* sri_mm = 0;
  req_msg* req_mm = 0;
  probe_msg* probe_mm = 0;
  ack_msg* ack_mm = 0; 
  unblock_msg* unblock_mm = 0;

  trans->get_extension(req_mm);
  trans->get_extension(sri_mm);
  trans->get_extension(probe_mm);
  trans->get_extension(ack_mm);
  trans->get_extension(unblock_mm);
  
  if(probe_mm || ack_mm) {
    u8_t* lanes = trans->get_byte_enable_ptr();
    if(lanes != 0)
      delete lanes;
    
    u8_t* data = trans->get_data_ptr();
    if(data != 0)
      delete data;
  }

  if(sri_mm) 
    trans->release_extension<sri_msg>(sri_mm);
  if(req_mm)
    trans->release_extension<req_msg>(req_mm);
  if(probe_mm) 
    trans->release_extension<probe_msg>(probe_mm);
  if(ack_mm)
    trans->release_extension<ack_msg>(ack_mm);
  if(unblock_mm) 
    trans->release_extension<unblock_msg>(unblock_mm);

  // clears the extension pointers
  trans->reset();

  lck.lock();
  if(!empties) {
    empties = new access;
    empties->next = free_list;
    empties->prev = 0;
    if(free_list)
      free_list->prev = empties;
  }

  free_list = empties;
  free_list->trans = trans;
  empties = free_list->prev;
  lck.unlock();
}
