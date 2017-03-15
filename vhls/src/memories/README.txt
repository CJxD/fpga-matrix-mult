Copyright (C) 2015 XPARCH, Ltd. <info@xparch.com>


generic_tlm_mem.h    An abstract blocking TLM model used as the core of a number of models with
                     different parameterisations.

sram64_cbg           A model of a general on-chip synchronous SRAM with arbitrary size and word width
                     whose delay and energy logging are based on interpolation from serval CACTI runs.

consistent_cache64   A parameterisable set-associative cache with two write strategies and a line buffer
                     to accumulate partially dirty words. Snooping on other caches within a consistent
                     group is enabled by passing each cache an instance of a consisent\_group object.
                     Each way consists of a pair of instances of sram64 to contain data and tags.

dram64_cbg.h         A macroscopic model of arbitrary DRAMs that is simply a specific instance of
                     generic\_tlm\_mem with amortised average delay and energy logging for every
                     operation, regardless of whether in the same bank or row as the previous operation.


hammer_cache64       Another parameterisable cache that uses the AMD hammer protocol for consistency.


cache_with_directory  A model that can be used for directory based memory systems (but not used in the AMD
                      system experiments).

variway_cache        A research prototype for an energy efficient cache.


memories.pdf	     Hand-written notes about design of files in this directory. Milos to organise and type them in.
