/* C code to run on Prazor after plugging in the device */

#include <stdio.h>

#define WRITE_BASE 0xE0002000
#define READ_BASE 0xE0002010
#define WRITE(offset, x) (((u32*)WRITE_BASE)[offset] = x)
#define READ(offset) (((u32*)READ_BASE)[offset])

#define u64 unsigned long long
#define u32 unsigned int

int main(int argc, char* argv[])
{
	u32 iters = 1;
	u32 matA, matB;

	if (argc < 2 || argc > 3)
	{
		fprintf(stderr, "Usage: matMul <matrixA> <matrixB> [iterations]\n");
		return 1;
	}

	if (argc == 3)
	{
		iters = argv[3];
	}

	sscanf(argv[1], "%x", &matA);
	sscanf(argv[2], "%x", &matB);

	u32 hRes, lRes;
	u32 i;

	printf("matrix A:\n%d %d\n%d %d\n", matA & 0xFF, matA>>8 & 0xFF, matA>>16 & 0xFF, matA>>24 & 0xFF); 
	printf("matrix B:\n%d %d\n%d %d\n", matB & 0xFF, matB>>8 & 0xFF, matB>>16 & 0xFF, matB>>24 & 0xFF); 

	for(i=0; i<iters; i++)
	{
		WRITE(0, matA);
		WRITE(2, matB);		
		lRes = READ(0);
		hRes = READ(2);
	}

	printf("res = 0x%08x 0x%08x\n", hRes, lRes);
	printf("res in matrix form:\n%d %d\n%d %d\n", lRes & 0xFFFF, lRes>>16 & 0xFFFF, hRes & 0xFFFF, hRes>>16 & 0xFFFF); 
	printf("\n\n");
}

