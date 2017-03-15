/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

#ifndef LLSC_EXTENSION_H
#define LLSC_EXTENSION_H

// $Id: llsc_extension.h,v 1.2 2011/06/08 14:09:50 djg11 Exp $
// Load-locked, store conditional extension to generic payload.
// (Based loosely on the one in the Dulous training materials).

#include "tlm.h"

 
struct llsc_locks_t // Place an instance of these locks on any target that needs to support load linked.
{
  const char *name;
  struct llsc_lock
  {
    u64_t a;
    void *id;
    llsc_lock() // constructor
    {
      a = -1;
      id = 0;
    };

  } llsc0, llsc1;

llsc_locks_t(const char *name): name(name) // constructor
  {

  }

};


struct llsc_extension : tlm::tlm_extension<llsc_extension>
{
  enum llsc_cmd_t { llsc } cmd;
  void *id; // Locker's identity (if not anonymous).
  tlm::tlm_generic_payload *next_on_freelist;
  llsc_extension() // constructor
   {
     cmd = llsc;
   }


  // Must implement clone and copy_from in all extensions.
  tlm_extension_base* clone() const
  {
    llsc_extension* ext = new llsc_extension;
    ext->id   = this->id;
    ext->cmd  = this->cmd;
    return ext;
  }

  void copy_from(tlm_extension_base const &ext)
  {
    id   = static_cast<llsc_extension const &>(ext).id;
    cmd  = static_cast<llsc_extension const &>(ext).cmd;
  }


  // This implementation of load-linked invalidates an old lock if someone else reads in the meantime,
  // but alternatively we could fail the subsequent lock attempts while the lock is outstanding.
  // If all lock locations are in use, we randomly discard one as well.
  bool loadlocked_fail(llsc_locks_t *lks, unsigned int adr) 
  {
   if (lks->llsc0.a == adr)
     {
       lks->llsc0.id = id;
       //printf("LLSC: Assigning to lock 0 at 0x%lx id 0x%lx\n", adr, id);
     }
   else if (lks->llsc1.a == adr) 
     {
       lks->llsc1.id = id;
       //printf("LLSC: Assigning to lock 1 at 0x%lx id 0x%lx\n", adr, id);

     }
   else if (lks->llsc0.a == (unsigned int)-1) 
     {
       lks->llsc0.a = adr;
       lks->llsc0.id = id;
       //printf("LLSC: Assigning to lock 0 at 0x%lx id 0x%lx\n", adr, id);
     }	
    else 
      {
	lks->llsc1.a = adr;
	lks->llsc1.id = id;
	//printf("LLSC %s: Assigning to lock 1 at 0x%lx id 0x%lx\n", adr, id);

      }
    // Load linked ALWAYS SUCCEEDS.
    return false;
  }

  // A write fails if the owner no longer has a pending lock on the location or region.
  bool storeconditional_fail(llsc_locks_t *lks, u64_t adr)
  {
    bool failed = true;
    //printf("LLSC: %s Compare region lock addrs = %lx cf %lx;   ids = %p cf %p;\n", lks->name, lks->llsc0.a, adr, lks->llsc0.id, id);
    //printf("LLSC: %s Compare region lock addrs = %lx cf %lx;   ids = %p cf %p;\n", lks->name, lks->llsc1.a, adr, lks->llsc1.id, id);
    if (lks->llsc0.a == adr && lks->llsc0.id == id)
      {
	lks->llsc0.a = -1;
	failed = false;
      }
    if (failed && lks->llsc1.a == adr && lks->llsc1.id == id)
      {
	lks->llsc1.a = -1;
	failed = false;
      }
    //printf("failed=%i\n", failed);
    return failed;
  }

  
};



// Memory manager: not thread safe.
class llsc_mm_t: public tlm::tlm_mm_interface
{
 private:
  tlm::tlm_generic_payload *freelist;

 public:
  //  void set_mm(gp_mm* arg) { m_mm = arg; }
  //  virtual void  free(tlm::tlm_generic_payload* trans);
  
  llsc_mm_t () // constructor
    {
      freelist = 0;
      //printf("Made an llsc mem manager %p %p\n", this, &freelist);
    }


  tlm::tlm_generic_payload* allocate()
    {
      tlm::tlm_generic_payload *r;
      if (freelist)
	{
	  r = freelist;
	  llsc_extension *ext = 0;
	  r->get_extension(ext);
	  freelist = (ext) ?  ext->next_on_freelist: 0; // Our freelist contains complete payloads, but, confusingly, we link the list using a field in the extension. 
	  //printf("found %p on freelist\n", r);
	  //      llsc_extension *ext = 0;
	}
      else 
	{
	  // We store generic payloads with their llsc_extension installed.
	  r = new (tlm::tlm_generic_payload)(this);
	  // constructor does this: r->set_mm(this);
	  //printf("%p %p New needed %p\n", this, &freelist, r);
	}
      llsc_extension *ext = 0;
      r->get_extension(ext); // Should be already installed if came from free list.
      if (!ext)
	{

	  ext = new llsc_extension;
	  r->set_extension(ext);
	}
      return r;
    };

  void free(tlm::tlm_generic_payload* r)
  {
    //printf("%p %p freelist=%p Freed  %p\n", this, &freelist, freelist, r);
    llsc_extension *ext = 0;
    r->get_extension(ext);
    assert(ext);
    ext->next_on_freelist = freelist;
    freelist = r;
  }
};



#endif
