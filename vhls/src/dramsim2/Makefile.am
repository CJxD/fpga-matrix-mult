# Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>

SUBDIRS = dist

AM_CPPFLAGS = -fPIC -DDRAMSIM2 -Idist -I$(top_builddir)/src/tenos -DSC_INCLUDE_DYNAMIC_PROCESSES -std=c++0x -I$(top_builddir)/src/memories -rdynamic
lib_LTLIBRARIES = libdramsim2.la
libdramsim2_la_SOURCES = dramsim_sc_wrapper.cpp dramsim_sc_wrapper.h
libdramsim2_la_LIBADD = dist/libdramsim.la
libdramsim2_la_LDFLAGS = -export-dynamic

