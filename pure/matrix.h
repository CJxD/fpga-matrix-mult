#ifndef MATRIX_H
#define MATRIX_H

typedef int mat_t;

typedef struct
{
	mat_t* M;
	int width, height;

	void fill()
	{
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				M[i*width + j] = i*width + j;
			}
		}
	}
	
	void print()
	{
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				printf("%d ", M[i*width + j]);
			}
			printf("\n");
		}
		printf("\n");
	}
} Matrix;

inline Matrix matrix_create(int height, int width)
{
	Matrix m;
	m.height = height;
	m.width = width;
	m.M = (mat_t*) malloc(sizeof(mat_t) * height * width);
	return m;
}

inline void matrix_destroy(Matrix &M)
{
	free(M.M);
}

inline Matrix matrix_multiply(const Matrix& MA, const Matrix& MB)
{
	Matrix MO = matrix_create(MA.height, MB.width);
	
	for (int i = 0; i < MA.height; i++)
	{
		for (int j = 0; j < MB.width; j++)
		{
			int res = 0;
			for (int k = 0; k < MA.width; k++)
			{
				res += MA.M[i*MA.width + k] * MB.M[k*MB.width + j];
			}
			MO.M[i*MB.width + j] = res;
		}
	}
	
	return MO;
}

#endif