VHLS?=../vhls

.PHONY: all run clean
all: matMul matMul.dis

include $(VHLS)/images/Makefile.inc

CFLAGS+=-std=c99 #-mfloat-abi=hard

EXTRALIBS += dummy_isr.o 

# one core command line
CMD=$(SIM) $(DRAM_FLAGS) -self-starting-cores 1 -global-qk-ns 100000 -vcd-power-plot -tracelevel 0

matMul.o: matMul.c
	$(CC) $(CFLAGS) -I$(DDRIVERS)/uart64_cbg -c $^ -o $@

matMul: $(CRT1) matMul.o $(EXTRALIBS) $(LIBS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

matMul.dis: matMul
	$(OBJD) -d $< > $@

run: matMul
	$(CMD) -image ./matMul -name identity.test -- 0x01020304 0x01000001 1 | tee identity.test.output
	@echo "Simulation completed successfully"

clean:
	rm -f matMul *.o *.s *.test* *.txt *.dis *.vcd
