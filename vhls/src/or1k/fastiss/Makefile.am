# Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>

#
# Note that iss.C is included by orsim.c.  This file is supposed to be generated from execgen but not is not?

AM_CFLAGS = -O3 -g
AM_CXXFLAGS = -DM32 -I$(top_builddir)/src/tenos -std=c++0x
AM_CPPFLAGS = -DM32 -I$(top_builddir)/src/tenos
lib_LTLIBRARIES = libOR1200F.la
libOR1200F_la_SOURCES = OR1200F.cpp OR1200F.h or32.c or32.h orsim.cpp orsim.h arch.h branch-predict.h config.h except.h port.h spr_defs.h sprs.h ../remotecontrol_or1k.cpp ../remotecontrol_or1k.h
libOR1200F_la_LDFLAGS = -export-dynamic
EXTRA_DIST = execgen.c iss.C main.cpp 
