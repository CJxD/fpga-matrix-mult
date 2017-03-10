# (C) 2009-17 DJG: University of Cambridge, SoCDAM.
# $Id: $  class1 makefile
#

include Makefile.def

# tlm_core is no longer separately needed since it is folded into the main SystemC release.

CXXFLAGS=-Wno-deprecated -I$(SYSCDIR)/include -I$(SYSCDIR)/include/tlm_core/tlm_2 -std=c++11 -O3



all:
	g++ $(CXXFLAGS) ex3.cpp $(LDFLAGS)
	./a.out



