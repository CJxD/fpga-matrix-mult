#ifndef SPEEDO_DEBUG_BASE_H
#define SPEEDO_DEBUG_BASE_H


// (C) 2013: DJ Greaves, University of Cambridge Computer Laboratory.
// Created as part of Project spEEDO SpeedoDebug based on the 2008 Version (C) Embecosm Limited.

#include "prazor.h"

#include "vhls_soc_debug.h"


class SpeedoDebugNode: public VHLSDebugIF
{
 public:

  // Reset function for the debug unit                                                          
  void resetDebugUnit()
  {
    printf("Reset Debug Unit\n");
  }

  // Functions to control and report on the CPU        
  void reset()
  { printf("Reset\n");}
  void stall()
  { printf("Stall\n");}
  void unstall()
  { printf("Unstall\n");}
  bool isStalled()
  { printf("Reset\n"); return 0; }

   u64_t readSpr(uint16_t sprNum)
   { printf("Read spr %i\n", sprNum); return 1024+sprNum; }

   int writeSpr(uint16_t sprNum, u64_t value) 
   { printf("Did not write spr %i %li\n", sprNum, value); 
     return -1;
   }

   void andSpr(uint16_t sprNum, u64_t value)
   { printf("Write AND spr %i %li\n", sprNum, value); }
   void orSpr(uint16_t sprNum, u64_t value)
   { printf("Write OR spr %i %li\n", sprNum, value); }
   
   // Functions to access memory                                                                                            
   u64_t readMem32(u64_t addr)
   { printf("Read mem %li \n", addr); return 22 + ((addr >>4)&1); }

   bool writeMem32(u64_t addr, u64_t value)
   { printf("Write mem %li %li\n", addr, value); }

   uint8_t readMem8(u64_t addr)
   { printf("Read mem8 %li \n", addr); return 22; }

   bool writeMem8(u64_t addr, uint8_t value)
   { printf("Write mem8 %li %i\n", addr, value); }
};



#endif


