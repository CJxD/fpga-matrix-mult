.PHONY: all clean

all: matMul

matMul.o: matMul_cpu.c	
	$(CC) $(CFLAGS) -c $< -o $@

matMul: matMul.o
	$(CC) $(LDFLAGS) $< -o $@

clean:
	-rm *.o matMul
