/* C code to run on Prazor after plugging in the device */

#include <stdio.h>
#define WRITE_BASE 0xE0002000
#define READ_BASE 0xE0002010
#define u64 unsigned long long
#define u32 unsigned int
#define debug 
int main()
{
	
  u32* write = WRITE_BASE;
  u32* read = READ_BASE;
	u32 hRes, lRes;
	u32 matA = 0x01020304;
	u32 matB = 0x01000001;

	for(int i=0;i<1;i++)
	{
		write[0] = matA;
		write[2] = matB;		
		lRes = read[0];
		hRes = read[2];
		
		#ifdef debug
			printf("\n\nstart\n");
			printf("matrix A:\n%d %d\n%d %d\n", matA & 0xFF, matA>>8 & 0xFF, matA>>16 & 0xFF, matA>>24 & 0xFF); 
			printf("matrix B:\n%d %d\n%d %d\n", matB & 0xFF, matB>>8 & 0xFF, matB>>16 & 0xFF, matB>>24 & 0xFF); 
			printf("res = 0x%08x 0x%08x\n", hRes, lRes);
			printf("res in matrix form:\n%d %d\n%d %d\n", lRes & 0xFFFF, lRes>>16 & 0xFFFF, hRes & 0xFFFF, hRes>>16 & 0xFFFF); 
			printf("\n\n");
		#endif
	}
}

