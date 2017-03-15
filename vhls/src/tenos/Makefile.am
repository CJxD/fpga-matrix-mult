# Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>

AM_CPPFLAGS = -I.  -std=c++0x
SRCS = tenos.cpp tenos.h  COREISA_IF.h cpu_busaccess.cpp  cpu_busaccess.h MpHash.h MpHash.cpp argv_backdoor.cpp generic_branch_predictor.cpp generic_branch_predictor.h io_backdoor.cpp
if WITH_SPEEDO
SRCS += speedo_api.cpp speedo_api.h 
endif
lib_LTLIBRARIES = libtenos.la
libtenos_la_SOURCES = $(SRCS)
EXTRA_DIST = argv_backdoor.h llsc_extension.h prazor.h pw_confidence_switcher.h pw_debug.h socdam-bdoor-mapping.h io_backdoor.h
libtenos_la_LDFLAGS = -export-dynamic
