# Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>

AM_CPPFLAGS = -g -DTARGET_BIG_ENDIAN -std=c++0x  -I$(top_builddir)/src/tenos 
lib_LTLIBRARIES = libgdbrsp.la libgdbrsp_empty.la

libgdbrsp_la_SOURCES = RspPacket.cpp RspPacket.h RspConnection.cpp RspConnection.h Utils.cpp Utils.h gdbrsp.cpp gdbrsp.h GdbServerSC.cpp GdbServerSC.h  SpeedoDebug.h SpeedoDebugBase.h vhls_soc_debug.h vhls_soc_debug_spr.h vhls_soc_debug.cpp
libgdbrsp_la_LDFLAGS = -export-dynamic

libgdbrsp_empty_la_SOURCES = vhls_soc_debug.h vhls_soc_debug_empty.cpp
libgdbrsp_empty_la_LDFLAGS = -export-dynamic

