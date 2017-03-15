# Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>

AM_CPPFLAGS = -I$(top_builddir)/src -I$(top_builddir)/src/tenos -I$(top_builddir)/src/memories -std=c++0x -Wl,--whole-archive
lib_LTLIBRARIES = libzynq.la libzynq_s.la
libzynq_la_LIBADD = -lsystemc $(top_builddir)/src/memories/libprazormem.la $(top_builddir)/src/dramsim2/libdramsim2.la $(top_builddir)/src/arm/libarm.la $(top_builddir)/src/io/libsdio.la $(top_builddir)/src/bus/libbusmux64.la $(top_builddir)/src/io/libuart64.la $(top_builddir)/src/gdbrsp/libgdbrsp.la
libzynq_la_SOURCES = zynq.cpp zynq.h
libzynq_la_LDFLAGS = -export-all-symbols -module -export-dynamic -version-info 0:0:0

libzynq_s_la_LIBADD = -lsystemc $(top_builddir)/src/memories/libprazormem.la $(top_builddir)/src/dramsim2/libdramsim2.la $(top_builddir)/src/arm/libarm.la $(top_builddir)/src/io/libsdio.la $(top_builddir)/src/bus/libbusmux64.la $(top_builddir)/src/io/libuart64.la
libzynq_s_la_SOURCES = zynq.cpp zynq.h
libzynq_s_la_LDFLAGS = -export-all-symbols -module -export-dynamic -version-info 0:0:0


# This part is used to build a statically linked
# version of the driver program
vhls_arm_s_SOURCES = ../../../../vhls.cpp
vhls_arm_s_CPPFLAGS = -w -I$(top_builddir)/src -DVHLS_STATIC -DSC_INCLUDE_DYNAMIC_PROCESSES -std=c++0x
vhls_arm_s_LDADD = libzynq_s.la $(top_builddir)/src/tenos/libtenos.la @POWER_LIBRARY@ -lsystemc $(top_builddir)/src/memories/libprazormem.la $(top_builddir)/src/gdbrsp/libgdbrsp_empty.la
vhls_arm_s_LDFLAGS = -w -all-static 

bin_PROGRAMS = vhls-arm.s
