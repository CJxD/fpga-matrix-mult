# Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>

AM_CPPFLAGS = -fPIC -std=c++0x -I$(top_builddir)/src/tenos -DSC_INCLUDE_DYNAMIC_PROCESSES 
lib_LTLIBRARIES = libprazormem.la
libprazormem_la_SOURCES = memloaders.cpp memloaders.h sram64_cbg.cpp sram64_cbg.h dram64_cbg.cpp dram64_cbg.h  secondary_cache_with_directory.cpp secondary_cache_with_directory.h generic_tlm_mem.cpp generic_tlm_mem.h memories_extensions.h base_mmu.h base_mmu_tlm.h base_mmu_tlm.cpp ccache.h smallram.h ccache.cpp scu.h scu.cpp
# old caches: hammer_cache64.h hammer_cache64.cpp consistent_cache64.cpp consistent_cache64.h 
# djg experiments: variway_cache64.h variway_cache64.cpp
libprazormem_la_LDFLAGS = -export-dynamic
