VHLS?=/home/hc475/P35/sim

all: matMul


include $(VHLS)/images/Makefile.inc

CFLAGS += -std=c99 #-mfloat-abi=hard

EXTRALIBS += dummy_isr.o 

# one core command line
#CMD=LD_LIBRARY_PATH=/usr/local/lib  $(SIM) $(DRAM_FLAGS) -cores 1 -tracelevel 0 -global-qk-ns 1 -no-caches 
CMD=$(SIM) $(DRAM_FLAGS) -self-starting-cores 1 -global-qk-ns 100000 -vcd-power-plot -tracelevel 1 -watch b8 +20 -tracelevel 0

matMul.o: matMul.c
	$(CC)  $(CFLAGS) -I$(DDRIVERS)/uart64_cbg -c matMul.c

matMul-clr: matMul run

matMul: $(LIBS) matMul.o $(CRT1) $(EXTRALIBS) 
	$(LD)  $(LDFLAGS) -o matMul $(CRT1) matMul.o $(EXTRALIBS)  $(LIBS) $(LDLIBS) 
	$(OBJD) -d matMul > matMul.dis

run:
	make
	ulimit -t 100000 ; $(CMD) -image ./matMul -name t
	echo done

clean:
	rm -f matMul *.o *.s *.txt *.dis
	


# eof
