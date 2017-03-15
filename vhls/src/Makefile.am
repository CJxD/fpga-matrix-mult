# Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>

AUTOMAKE_OPTIONS = foreign
MYDEFS = @DEFS@
SUBDIRS = tenos io bus dramsim2 gdbrsp memories tenos arm or1k x86 mips64 platform 

vhls_SOURCES = vhls.cpp
vhls_CPPFLAGS = -I. -DSC_INCLUDE_DYNAMIC_PROCESSES -std=c++0x
vhls_LDADD = tenos/libtenos.la @POWER_LIBRARY@ 
#gdbrsp/libgdbrsp.la io/libuart64.la io/libsdio.la dramsim2/libdramsim2.la bus/libbusmux64.la bus/libbusopteron.la memories/libprazormem.la  arm/libarm.la @POWER_LIBRARY@
vhls_LDFLAGS = -export-dynamic -lsystemc -lpthread -lm -ldl 

bin_PROGRAMS = vhls
