# Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>

AM_CPPFLAGS = -fPIC -I$(top_builddir)/src/tenos -std=c++0x -DSC_INCLUDE_DYNAMIC_PROCESSES
lib_LTLIBRARIES = libbusmux64.la libbusopteron.la
libbusmux64_la_SOURCES = busmux64.cpp busmux64.h
libbusopteron_la_SOURCES = crossbar.cpp crossbar.h hypertransport.cpp hyperttransport.h opteron_messages.h opteron_msg_mm.cpp sri.cpp sri.h
EXTRA_DIST = vmunit64.cpp vmunit64.h
libbusmux64_la_LDFLAGS = -export-dynamic
libbusopteron_la_LDFLAGS = -export-dynamic 
