/* Copyright (C) 2016 XPARCH, Ltd. <info@xparch.com> */

#ifndef _PROC_ID__EXTENSION_H_
#define _PROC_ID__EXTENSION_H_

#include "tlm.h"

struct proc_id_extension : tlm::tlm_extension<proc_id_extension> {
 public:
  u32_t proc_id;
  u32_t length;

  proc_id_extension() {
    proc_id = -1;
    length = -1;
  }

  tlm_extension_base* clone() const {
    proc_id_extension* ext = new proc_id_extension;
    ext->proc_id = this->proc_id;
    ext->length = this->length;

    return ext;
  }

  void copy_from(tlm_extension_base const &ext) {
    proc_id = static_cast<proc_id_extension const&>(ext).proc_id;
    length = static_cast<proc_id_extension const&>(ext).length;

  }
};

#endif
