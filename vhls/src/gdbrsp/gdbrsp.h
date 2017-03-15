#ifndef GDBRSP_H
#define GDBRSP_H
// $Id: gdbrsp.h,v 1.2 2010/09/24 00:07:03 djg11 Exp $
// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2009-10 D J Greaves
// University of Cambridge, Computer Laboratory.
// SoC D/M Classes - CBG Prazor  

#include "pthread.h"

#include "RspConnection.h"
#include "RspPacket.h"

class gdbrsp
{
  const char *name;
  pthread_t rsp_thread;
  RspConnection *connection;
 public:
  gdbrsp(const char *name, int pn);
  void process();
};

#endif


