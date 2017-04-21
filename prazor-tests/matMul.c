#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t
#define ptr_t uintptr_t

#include "zynq_utils.h"

// Memory pointers
#define MEM_BASE 0xE0002000
#define MAT	0x00
#define RES	0x0C

// Disable page mapping
#define map_page(addr) addr
#define unmap_page(addr)

// Define write and read commands
#define WRITE(offset, x) (*((u32*)(mem_base+offset)) = x)
#define READ(offset) (*((u32*)(mem_base+offset)))

// Energyshim entry points
#ifdef USE_ENERGYSHIM
#include "energyshim.h"
#define main(argc, argv) bm_main(argc, argv)
#define init(argc, argv) bm_init(argc, argv)
#define deinit() bm_deinit()
#endif

u32 mat, iters;
u32 res;
ptr_t mem_base;

int init(int argc, const char* argv[])
{
	if (argc < 2 || argc > 4)
	{
		fprintf(stderr, "Usage: matMul <matrix> [iterations]\n\n\n");
		return 1;
	}

	if (argc == 3)
	{
		iters = strtoul(argv[2], NULL, 10);
	}
	else
	{
		iters = 1;
	}

	mat = strtoul(argv[1], NULL, 16);

	printf("Mapping memory\n");
	mem_base = (ptr_t) map_page(MEM_BASE);

	if (mem_base <= 0)
		return 1;

#if !defined(USE_ENERGYSHIM) && !defined(__linux__)
	printf("Turning on L1 and L2 caches\n");
	zynq_enable_caches();
#endif

#if defined(__linux__) && defined(FUCK_WITH_INTERRUPTS)
	printf("Turning off interrupts\n");
	zynq_disable_interrupts();
#endif

	printf("\n");
	printf("input = 0x%08x\n", mat);
	printf("matrix:\n%d %d\n%d %d\n", mat & 0xFF, mat>>8 & 0xFF, mat>>16 & 0xFF, mat>>24 & 0xFF); 

	printf("iterations: %u\n", iters);
	return 0;
}

int deinit()
{
	printf("res = 0x%08x\n", res);
	printf("res in matrix form:\n%d %d\n%d %d\n", res & 0xFF, res>>8 & 0xFF, res>>16 & 0xFF, res>>24 & 0xFF); 
	printf("\n");

#if defined(__linux__) && defined(FUCK_WITH_INTERRUPTS)
	printf("Turning on interrupts\n");
	zynq_enable_interrupts();
#endif

	printf("Unmapping memory\n");
	unmap_page((void*) mem_base);

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
		WRITE(MAT, mat);	
		res = READ(RES);
	}

#ifndef USE_ENERGYSHIM
	rc = deinit(argc, argv);
	return rc;
#endif
}

