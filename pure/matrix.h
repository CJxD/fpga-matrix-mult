#ifndef MATRIX_H
#define MATRIX_H

#define LAYER 4

u32 _data[2];
u8* _mat = (u8*) &_data[0];
u8* _res = (u8*) &_data[1];
u8 B[4] = {1,0,0,1};
u8 C[4] = {1,1,1,1};

inline void matMul(u8* MA, u8* MB, u8* MO)
{
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
		{
			u8 res = 0;
			for (u8 k = 0; k < 2; k++)
				res += MA[i*2 + k] * MB[k*2 + j];
			MO[i*2 + j] = res;
		}
}

inline void matAdd(u8* MA, u8* MB, u8* MO)
{
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			MO[i*2 + j] = MA[i*2 + j] + MB[i*2 + j];
}

inline void matrix_multiply()
{
	u8 tmp1[4];
	u8 tmp2[4];
	matMul(_mat,B,tmp1);
	matAdd(tmp1,C,tmp2);

	for(int i=0;i<LAYER-2;i++)
	{
		matMul(tmp2,B,tmp1);
		matAdd(tmp1,C,tmp2);
	}	

	matMul(tmp2,B,tmp1);
	matAdd(tmp1,C,_res);
}
#endif
