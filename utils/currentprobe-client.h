/* Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com> */

#ifndef currentprobe_client_h
#define currentprobe_client_h

// A small RPC client that askes for the running energy total from a power probe.

#include <netdb.h>

#define PORT        2002

#define HOST        "localhost"
#define MESSAGESIZE     8192

#define N_ENERGIES  1

extern int currentprobe_open(struct hostent *hp, const char *hostname);
extern int currentprobe_operate(int *energies, const char *hostname);

#endif

