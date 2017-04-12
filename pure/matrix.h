#ifndef MATRIX_H
#define MATRIX_H

u32 _data[4];
u8* _matA = (u8*) &_data[0];
u8* _matB = (u8*) &_data[1];
u8* _lRes = (u8*) &_data[2];
u8* _hRes = (u8*) &_data[3];

inline matrix_multiply()
{
	u16 _res[4] = {0, 0, 0, 0};

	unsigned char i, j, k;
	for(i=0;i<2;i++)
		for(j=0;j<2;j++)
			for(k=0;k<2;k++)
				_res[i*2+j] += _matA[i*2+k] * _matB[k*2+j];

	*(u32*) _lRes = *(u32*) &_res[0];
	*(u32*) _hRes = *(u32*) &_res[2];
}

#endif
