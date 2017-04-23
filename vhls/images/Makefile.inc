# Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>
#
# Arch-neutral defines and dispatcher for four builds: native, OR1K, ARM32 and x86_64

# EXTRALIBS is to be defined in the image for each Makefile for per-application binaries to be linked in
# BASELIBS is to be defined in arch-specific Makefile.inc
# We want each app to compile with prlibc and ucLibc for each architecture and for native.

#

SELF_DIR := $(dir $(lastword $(MAKEFILE_LIST)))


ifeq ($(strip $(VHLS)),)
ifeq ($(strip $(PRAZOR)),)
$(warning neither VHLS OR PRAZOR env variable is set)
PRAZOR  ?= $(HOME)/d320/prazor/trunk
endif
VHLS    ?= $(PRAZOR)/vhls
endif

# PRAZOR  ?= $(SELF_DIR)

THREADLIB ?= $(VHLS)/src/crt/djgthreads
TENOS   ?= $(VHLS)/src/tenos
SUPPORT ?=  $(VHLS)/src/tenos

GET_ENERGY_FROM_PROBE ?= 0

# DDR3_micron_8M_8B_x16_sg15: # NUM_BANKS=8 NUM_ROWS=8192 NUM_COLS=1024 DEVICE_WIDTH=16 = 8*8192*1024/2 Bytes = 64 MByte.
# 
DRAM_FLAGS ?= -dram-system-ini $(VHLS)/boards/parallella/dramsim.ini \
              -dram-device     $(VHLS)/boards/parallella/DDR3_micron_32M_8B_x32_parallella.ini





VHLS     ?= $(PRAZOR)/vhls
DDRIVERS ?=  $(VHLS)/src/devicedrivers
CRT      ?=  $(VHLS)/src/crt
PRLIBC   ?=  $(VHLS)/src/crt/prlibc
IMAGES   ?=  $(VHLS)/images



SUPPORT  ?=  $(VHLS)/src/tenos

CFLAGS +=-g -DHEAPBASE=$(HEAPBASE) -DHEAPEND=$(HEAPEND) -I$(VHLS)/src/tenos -I$(VHLS)/src/devicedrivers/uart64_cbg -I$(CRT)/prlibc -I$(CRT)/prlibc/$(TARCH) -D$(TARCH) -Wno-implicit-function-declaration -fno-builtin
LDFLAGS=-Ttext 0x0 -Tdata 0x8000 -Tbss 0x20000 


djgthread.o:$(THREADLIB)/djgthread.c
	$(CC) -c  $(CFLAGS) $<


dummy_isr.o: $(CRT)/prlibc/dummy_isr.c
	$(CC) -fno-builtin $(CFLAGS) -c $<

energyshim.o: $(IMAGES)/energyshim.c
	$(CC) -fno-builtin $(CFLAGS) -c $<


uclibc_backdoor.o: $(CRT)/uClibc/uclibc_backdoor.c
	$(CC) -fno-builtin $(CFLAGS) -c $<



ifeq ($(TARCH),x86_64)
include $(VHLS)/images/Makefile.inc.x86_64
else
ifeq ($(TARCH),OR1K)
include $(SELF_DIR)/Makefile.inc.or1k
else
ifeq ($(TARCH),ARM32)
include $(SELF_DIR)/Makefile.inc.arm
else
ifeq ($(TARCH),NATIVE)
include $(SELF_DIR)/Makefile.inc.native
else
$(error Please Set TARCH to x86_64, ARM32 or OR1K or NATIVE (a binary to run on this workstation outside the VHLS framework))
endif
endif
endif
endif





# eof
