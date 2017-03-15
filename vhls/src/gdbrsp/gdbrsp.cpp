// $Id: gdbrsp.cpp,v 1.2 2010/09/24 00:07:03 djg11 Exp $
// Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
// (C) 2009-10 D J Greaves
// University of Cambridge, Computer Laboratory.
// SoC D/M Classes - CBG Prazor  

#include "gdbrsp.h"
#include <stdio.h>
#include <unistd.h>

void * gdbrsp_caller(void *arg) // suitable for pthreads call.
{
  gdbrsp *it = (gdbrsp *)(arg);
  it->process();
}


void gdbrsp::process()
{
  RspPacket pkt(1000);
  connection->rspConnect();
  while(1)
    {
      printf("Connected %i\n", connection->isConnected());
      sleep(1);
      connection->getPkt(&pkt);
      printf("Gto %i\n", pkt.getLen());
    }
}



gdbrsp::gdbrsp(const char *name, int n) : name(name)
{
  printf("RSP listening %s port number %i\n", name, n);
  connection = new RspConnection(n);


}


// eof
