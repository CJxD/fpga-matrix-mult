# Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>

AM_CPPFLAGS =  -I../verilated -I../fastiss -I../memories -I../bus -I../dramsim2 -I../io -I../tenos -DSC_INCLUDE_DYNAMIC_PROCESSES -std=c++0x

lib_LTLIBRARIES = libarm.la
#libarm_la_CPPFLAGS = -I../verilated -I../fastiss -I../memories -I../bus -I../dramsim2 -I../io -I../tenos -DSC_INCLUDE_DYNAMIC_PROCESSES -std=c++0x -DSTATIC=1
libarm_la_SOURCES = bench1.h armdis.cpp armthumb.cpp armisa.cpp armcore_tlm.cpp armcore_tlm.h armisa.h armdis.h arm_mmu.h arm_mmu.cpp arm_cp15.h arm_cp15.cpp arm_ccache.h arm_ccache.cpp arm_scu.h arm_scu.cpp gic_arm_tlm.h gic_arm_tlm.cpp sclr_arm_tlm.h sclr_arm_tlm.cpp arm_cp14.h arm_cp14.cpp arm_timers.h arm_timers.cpp arm_L2Cpl310.h arm_L2Cpl310.cpp arm_abt.h arm_abt.cpp arm_cortex_a9.h arm_cortex_a9.cpp
libarm_la_LIBADD = $(top_builddir)/src/tenos/libtenos.la 
libarm_la_LDFLAGS = -export-dynamic

#fastiss_SOURCES = fastiss.cpp
#fastiss_LDADD = ../tenos/libtenos.a
#fastiss_CPPFLAGS = -I../memories -I../tenos -std=c++0x
#fastiss_LDFLAGS = -lm armisa.o armthumb.o armdis.o ../memories/memloaders.o
EXTRA_DIST = backdoor_reads.C backdoor_writes.C 

# eof
