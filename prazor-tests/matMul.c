#include <stdio.h>
#include <stdlib.h>

#define u32 unsigned int
#define u64 unsigned long long

#include "zynq_utils.h"

#define MEM_BASE 0xE0002000
#define MATA	0x00
#define MATB	0x04
#define RES_LO	0x08
#define RES_HI	0x0C

#define WRITE(offset, x) (*((u32*)(mem_base+offset)) = x)
#define READ(offset) (*((u32*)(mem_base+offset)))

#ifdef USE_ENERGYSHIM
#include "energyshim.h"
#define main(argc, argv) bm_main(argc, argv)
#define init(argc, argv) bm_init(argc, argv)
#define deinit() bm_deinit()
#endif

u32 matA, matB, iters;
u32 lRes, hRes;
void* mem_base;

int init(int argc, const char* argv[])
{
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
	
	mem_base = (void *) MEM_BASE;

	if (mem_base <= 0)
		return 1;

	printf("Waiting for last results to clear...\n");
	WRITE(MATA, 0);
	WRITE(MATB, 0);

	u32 res;
	do {
		res = READ(RES_LO) + READ(RES_HI);
	} while (res != 0);

#if !defined(USE_ENERGYSHIM) && !defined(__linux__)
	printf("Turning on L1 and L2 caches\n");
	zynq_enable_caches();
#endif

#if defined(__linux__) && defined(FUCK_WITH_INTERRUPTS)
	printf("Turning off interrupts\n");
	zynq_disable_interrupts();
#endif

	printf("\n");
	printf("input = 0x%08x 0x%08x\n", matA, matB);
	printf("matrix A:\n%d %d\n%d %d\n", matA & 0xFF, matA>>8 & 0xFF, matA>>16 & 0xFF, matA>>24 & 0xFF); 
	printf("matrix B:\n%d %d\n%d %d\n\n", matB & 0xFF, matB>>8 & 0xFF, matB>>16 & 0xFF, matB>>24 & 0xFF);

	printf("iterations: %u\n", iters);
}

int deinit()
{
	printf("res = 0x%08x 0x%08x\n", hRes, lRes);
	printf("res in matrix form:\n%d %d\n%d %d\n", lRes & 0xFFFF, lRes>>16 & 0xFFFF, hRes & 0xFFFF, hRes>>16 & 0xFFFF);
	printf("\n");

#if defined(__linux__) && defined(FUCK_WITH_INTERRUPTS)
	printf("Turning on interrupts\n");
	zynq_enable_interrupts();
#endif

	printf("\n");	
	return 0;
}

int main(int argc, const char* argv[])
{
#ifndef USE_ENERGYSHIM
	int rc = init(argc, argv);
	if (rc != 0)
		return rc;
#endif

	u32 i;
	for(i=0; i<iters; i++)
	{
		WRITE(MATA, matA);
		WRITE(MATB, matB);		
		lRes = READ(RES_LO);
		hRes = READ(RES_HI);
	}

#ifndef USE_ENERGYSHIM
	rc = deinit(argc, argv);
	return rc;
#endif
}
