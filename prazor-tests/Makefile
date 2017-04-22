VHLS?=../vhls

.PHONY: all run clean-objs clean
all: matMul matMul.dis

include $(VHLS)/images/Makefile.inc

CFLAGS+= -O0
EXTRALIBS+= dummy_isr.o

# one core command line
CMD=$(SIM) $(DRAM_FLAGS) -self-starting-cores 1 -global-qk-ns 100000 -vcd-power-plot -tracelevel 0

ifeq ($(USE_ENERGYSHIM),1)

CFLAGS+= -DUSE_ENERGYSHIM
EXTRALIBS+= energyshim.o

ifeq ($(NATIVE),1)
CFLAGS+= -DNATIVE
SHIMMETHOD=-DGET_ENERGY_FROM_PROBE
EXTRALIBS+= currentprobe-client.o
else
SHIMMETHOD=-DUSE_SPEEDO -DUSE_VHLS_BACKDOORS
endif

energyshim.o: energyshim.c
	$(CC) $(CFLAGS) $(SHIMMETHOD) -c $^ -o $@

currentprobe-client.o: currentprobe-client.c
	$(CC) $(CFLAGS) -c $^ -o $@
	
endif

matMul.o: matMul.c
	$(CC) -std=c99 $(CFLAGS) -I$(DDRIVERS)/uart64_cbg -c $^ -o $@

matMul: $(CRT1) matMul.o $(EXTRALIBS) $(LIBS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

matMul.dis: matMul
	$(OBJD) -d $< > $@

run: matMul
ifeq ($(USE_ENERGYSHIM),1)
	$(CMD) -image ./matMul -name identity.test	-- l=10000 -- 0x01020304 0x01000001
	$(CMD) -image ./matMul -name double.test	-- l=10000 -- 0x01020304 0x02000002
	$(CMD) -image ./matMul -name hard.test		-- l=10000 -- 0xBA5EBA11 0xBEEFBABE
else
	$(CMD) -image ./matMul -name identity.test	-- 0x01020304 0x01000001 10000
	$(CMD) -image ./matMul -name double.test	-- 0x01020304 0x02000002 10000
	$(CMD) -image ./matMul -name hard.test		-- 0xBA5EBA11 0xBEEFBABE 10000
endif
	@echo "done"

clean-objs:
	rm -f *.o *.s *.dis

clean: | clean-objs
	rm -f matMul *.test* *.txt *.vcd

