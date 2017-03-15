//
//
//

#include <stdio.h>
#include <systemc.h>

#include "SpeedoDebug.h"

// Useful info here https://sourceware.org/gdb/onlinedocs/gdb/Connecting.html#Connecting
//  $ 
//  $ set remoteaddresssize 64
//  info registers
//  X addr          -- display memory
//  x/32x [ addr ]  -- display 32 words of memory in hex.
//  load /home/djg11/d320/prazor/trunk/chstone/dfsin/dfsin -- download an ELF binary

// info threads
// thread 2

/*
 set debug remote 1
 set debug target 1
 target remote localhost:9600
*/ 

SC_MODULE(World)
{

  //SpeedoDebugdebugroot
  GdbServerSC TheServer;

  SC_CTOR(World):
    TheServer("TheServer", 0, 0x10000, 9600)
  {
  };

};


int sc_main(int argc, char *argv[])
{
  printf("hello world\n");

  World *w = new World("TheWorld");
  sc_start(10000, SC_US);
  std::cout << "end\n";
  return 0;
}

// eof
