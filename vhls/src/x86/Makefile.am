# Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>

AM_CPPFLAGS = -I../memories -I../bus -I../dramsim2 -I../io -I../tenos -DSC_INCLUDE_DYNAMIC_PROCESSES -std=c++0x -Wno-narrowing -Wno-int-to-pointer-cast

lib_LTLIBRARIES = libx86.la
libx86_la_SOURCES = x86core.cpp x86core.h superstl.cpp superstl.h globals.h klibc.h klibc.cpp syscalls.cpp syscalls.h ptlsim.h ptlsim.cpp config.h config.cpp kernel.h kernel.cpp ptlhwdef.h ptlhwdef.cpp datastore.h datastore.cpp mathlib.h mathlib.cpp mm.h mm-private.h mm.cpp decode.h decode-core.cpp decode-x87.cpp decode-complex.cpp uopimpl.cpp decode-sse.cpp decode-fast.cpp logic.h stats.h branchpred.h dcache-amd-k8.h dcache.h loader.h ptlcalls.h ooocore.h ptlxen.h xen-types.h seqcore.h
libx86_la_LIBADD = $(top_builddir)/src/tenos/libtenos.la
libx86_la_LDFLAGS = -export-dynamic

EXTRA_DIST = backdoor_nops.C backdoor_reads.C backdoor_writes.C seqcore.cpp
