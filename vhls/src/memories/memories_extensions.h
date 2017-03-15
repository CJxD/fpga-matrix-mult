/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

#ifndef _MEMORIES_EXTENSIONS_H
#define _MEMORIES_EXTENSIONS_H


#ifndef PW_TLM_PAYLOAD
#define PW_TLM_PAYLOAD 3 // for now - need to make this work without.
#endif  // PW_TLM_PAYLOAD


#ifdef TLM_POWER3
#include "pw_tlm_payload.h"
#else
#include "tlm.h"
typedef tlm::tlm_base_protocol_types PW_TLM_TYPES;
typedef tlm::tlm_generic_payload PW_TLM_PAYTYPE;
#endif
#include <map>
#include <systemc.h>

enum CACHE_TYPE {
  CT_INSTRUCTION,
  CT_DATA,
  CT_UNIFORM
};

enum RESPONSE_TYPE {
  CT_MISS_SHARED,
  CT_MISS_EXCLUSIVE, // satisfied by DRAM controler
  CT_MISS_DIRTY,
  CT_MISS_OWNED,
  CT_UNKNOWN
};

typedef enum CACHE_TYPE cache_ty;
typedef enum RESPONSE_TYPE response_ty;

struct cache_miss_extension : tlm::tlm_extension<cache_miss_extension> {
  std::map<int, std::map<cache_ty, bool> > miss;
  response_ty resp;

  cache_miss_extension() {
    resp = CT_UNKNOWN;
  }

  tlm_extension_base* clone() const {
    cache_miss_extension* ext = new cache_miss_extension;
    for(std::map<int, std::map<cache_ty, bool> >::const_iterator i = miss.begin();
	i != miss.end();
	i++) 
      for(std::map<cache_ty, bool>::const_iterator j = i->second.begin();
	  j != i->second.end();
	  j++)
	ext->miss[i->first][j->first] = j->second;

    ext->resp = this->resp;

    return ext;
  }

  void copy_from(tlm_extension_base const &ext) {
    miss = static_cast<cache_miss_extension const&>(ext).miss;
    resp = static_cast<cache_miss_extension const&>(ext).resp;
  }
};


class cache_miss_mm_t : public tlm::tlm_mm_interface
{

 public:
  // constructor
  cache_miss_mm_t()
    : free_list(0), empties(0), lck("cache_mm") {}

  PW_TLM_PAYTYPE* allocate();
  void free(PW_TLM_PAYTYPE* r);

#ifdef TLM_POWER3
#if PW_TLM_PAYLOAD > 0 
  void free(tlm::tlm_generic_payload* trans) 
  {
    free((PW_TLM_PAYTYPE*)trans);
  }
#endif
#endif

 private:
  struct access {
    PW_TLM_PAYTYPE* trans;
    access* next;
    access* prev;
  };

  access* free_list;
  access* empties;

  sc_mutex lck;
};

#endif
