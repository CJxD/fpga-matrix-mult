#ifndef MATRIX_H
#define MATRIX_H

u32 _data[2];
u8* _mat = (u8*) &_data[0];
u8* _res = (u8*) &_data[1];

inline void matrix_multiply()
{
	unsigned char i, j, k;
	for(i=0;i<2;i++)
		for(j=0;j<2;j++)
			for(k=0;k<2;k++)
				_res[i*2+j] += _mat[i*2+k] * _mat[k*2+j];
}

#endif
