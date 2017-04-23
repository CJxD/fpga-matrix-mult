LDLIBS=-lsystemc -ltlm-power3

.PHONY: all run clean-objs clean
all: matMul

CXXFLAGS+= -DTLM_POWER3

matMul.o: matMul.cpp
	$(CXX) -std=c++11 $(CXXFLAGS) -c $^ -o $@

matMul: matMul.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

run: matMul
	./matMul 0x01000001 1 | tee identity.test.txt
	./matMul 0x01000001 10000 | tee identity.10000.test.txt
	./matMul 0xBEEFBABE 1 | tee hard.test.txt
	./matMul 0xBEEFBABE 10000 | tee hard.10000.test.txt
	@echo "done"

clean-objs:
	rm -f *.o

clean: | clean-objs
	rm -f matMul *.txt



