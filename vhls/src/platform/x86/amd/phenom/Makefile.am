# Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>

AM_CPPFLAGS = -I$(top_builddir)/src -I$(top_builddir)/src/tenos -I$(top_builddir)/src/memories -I$(top_builddir)/src/x86 -std=c++0x
lib_LTLIBRARIES = libphenom.la
libphenom_la_LIBADD = -lsystemc $(top_builddir)/src/memories/libprazormem.la $(top_builddir)/src/dramsim2/libdramsim2.la $(top_builddir)/src/x86/libx86.la $(top_builddir)/src/bus/libbusmux64.la $(top_builddir)/src/io/libuart64.la
libphenom_la_SOURCES = phemon.h phenom.cpp
libphenom_la_LDFLAGS = -module -export-dynamic -version-info 0:0:0
