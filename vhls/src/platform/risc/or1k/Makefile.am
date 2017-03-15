# Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>

AM_CPPFLAGS = -DUSE_FASTISS=1 -I$(top_builddir)/src/or1k/fastiss -I$(top_builddir)/src -I$(top_builddir)/src/tenos -I$(top_builddir)/src/memories -DSC_INCLUDE_DYNAMIC_PROCESSES -std=c++0x
lib_LTLIBRARIES = libor1k.la
libor1k_la_LIBADD = -lsystemc $(top_builddir)/src/memories/libprazormem.la $(top_builddir)/src/dramsim2/libdramsim2.la $(top_builddir)/src/or1k/fastiss/libOR1200F.la $(top_builddir)/src/or1k/verilated/libOR1200V.la $(top_builddir)/src/io/libsdio.la $(top_builddir)/src/bus/libbusmux64.la $(top_builddir)/src/io/libuart64.la
libor1k_la_SOURCES = or1k.cpp or1k.h
libor1k_la_LDFLAGS = -module -export-dynamic -version-info 0:0:0
