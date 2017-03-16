/* C code to run on Prazor after plugging in the device */

#include <stdio.h>
#include <stdlib.h>

#define WRITE_BASE 0xE0002000
#define READ_BASE 0xE0002010
#define WRITE(offset, x) (((u32*)WRITE_BASE)[offset] = x)
#define READ(offset) (((u32*)READ_BASE)[offset])

#ifdef USE_ENERGYSHIM
#include "energyshim.h"
#define main(argc, argv) bm_main(argc, argv)
#define init(argc, argv) bm_init(argc, argv)
#endif

#define u64 unsigned long long
#define u32 unsigned int

u32 matA, matB, iters;

int init(int argc, const char* argv[])
{
#ifndef USE_ENERGYSHIM
	printf("Turning on L1 and L2 caches\n");
	__asm__("mov r0,#0x1000"); // Turn on L1 Cache (see Zynq TRM for further details.)
	__asm__("orr r0,r0,#4");
	__asm__("mcr  p15, 0, r0, c1, c0, 0"); //  (r0 = 0x1004)
	((int*)0xF8F02100)[0] = 1; // Zynq: turn on L2 cache
#endif

	printf("\n\n");
	
	if (argc < 3 || argc > 5)
	{
		fprintf(stderr, "Usage: matMul <matrixA> <matrixB> [iterations]\n\n\n");
		return 1;
	}

	if (argc == 4)
	{
		iters = strtoul(argv[3], NULL, 10);
	}
	else
	{
		iters = 1;
	}

	matA = strtoul(argv[1], NULL, 16);
	matB = strtoul(argv[2], NULL, 16);
	
	printf("input = 0x%08x 0x%08x\n", matA, matB);
	printf("matrix A:\n%d %d\n%d %d\n", matA & 0xFF, matA>>8 & 0xFF, matA>>16 & 0xFF, matA>>24 & 0xFF); 
	printf("matrix B:\n%d %d\n%d %d\n\n", matB & 0xFF, matB>>8 & 0xFF, matB>>16 & 0xFF, matB>>24 & 0xFF);

	printf("iterations: %lu\n", iters); 
}

int main(int argc, const char* argv[])
{
#ifndef USE_ENERGYSHIM
	init(argc, argv);
#endif

	u32 i;
	u32 hRes, lRes;

	for(i=0; i<iters; i++)
	{
		WRITE(0, matA);
		WRITE(2, matB);		
		lRes = READ(0);
		hRes = READ(2);
	}

#ifndef USE_ENERGYSHIM
	printf("res = 0x%08x 0x%08x\n", hRes, lRes);
	printf("res in matrix form:\n%d %d\n%d %d\n", lRes & 0xFFFF, lRes>>16 & 0xFFFF, hRes & 0xFFFF, hRes>>16 & 0xFFFF); 
	printf("\n\n");
#endif
}

