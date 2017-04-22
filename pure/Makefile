.PHONY: all run clean-objs clean
all: matMul matMul.dis

OBJD?=objdump

CFLAGS+= -O0
LDLIBS= -lc

CMD=./matMul
CFLAGS+= -DNATIVE

ifeq ($(USE_ENERGYSHIM),1)

CFLAGS+= -DUSE_ENERGYSHIM
SHIMMETHOD=-DGET_ENERGY_FROM_PROBE

EXTRALIBS+= energyshim.o currentprobe-client.o
energyshim.o: energyshim.c
	$(CC) $(CFLAGS) $(SHIMMETHOD) -c $^ -o $@

currentprobe-client.o: currentprobe-client.c
	$(CC) $(CFLAGS) -c $^ -o $@

endif

matMul.o: matMul.c
	$(CC) -std=c99 $(CFLAGS) -c $^ -o $@

matMul: matMul.o $(EXTRALIBS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

matMul.dis: matMul
	$(OBJD) -d $< > $@

run: matMul
ifeq ($(USE_ENERGYSHIM),1)
	./matMul l=10000000 -- 0x01000001 | tee identity.test
	./matMul l=10000000 -- 0xBEEFBABE | tee hard.test
else
	./matMul 0x01000001 10000000 | tee identity.test
	./matMul 0xBEEFBABE 10000000 | tee hard.test
endif
	@echo "done"

clean-objs:
	rm -f *.o *.s *.dis

clean: | clean-objs
	rm -f matMul *.test*
