# Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>

AM_CPPFLAGS = -fPIC -I$(top_builddir)/src/tenos -DSC_INCLUDE_DYNAMIC_PROCESSES -std=c++0x
lib_LTLIBRARIES = libuart64.la libsdio.la
libuart64_la_SOURCES = uart64_cbg.cpp uart64_cbg.h
libsdio_la_SOURCES = sdio_cbg.cpp sdio_cbg.h
libuart64_la_LDFLAGS = -export-dynamic
libsdio_la_LDFLAGS = -export-dynamic

