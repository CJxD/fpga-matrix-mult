# Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>

SUBDIRS = vcpp

AM_CPPFLAGS =  -Ivcpp -I$(top_builddir)/src/tenos -I.. -DSC_INCLUDE_DYNAMIC_PROCESSES -std=c++0x -DNO_BRANCH_DELAY -DM32
lib_LTLIBRARIES = libOR1200V.la
libOR1200V_la_SOURCES = OR1200V.cpp OR1200V.h OrpsocAccess.cpp OrpsocAccess.h 
libOR1200V_la_LIBADD = vcpp/libVor1200_top__ALL.la
libOR1200V_la_LDFLAGS = -export-dynamic

# eof
