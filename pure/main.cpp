#include <stdio.h>
#include <stdlib.h>
//using namespace std;
void printMat(int* M, int height, int width)
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			printf("%d ",M[i*width + j]);
			//cout << M[i*width + j] << " ";
		}
		printf("\n");
		//cout << "\n";
	}
	printf("\n");
	//cout << endl;
}

void fillMat(int* M, int height, int width)
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			M[i*width + j] = i*width + j;
		}
	}
}


int main()
{
	int heightA = 3;
	int widthA = 2;
	int heightB = 2;
	int widthB = 5;
	int* MA = (int*)malloc(4 * heightA*widthA);
	int* MB = (int*)malloc(4 * heightB*widthB);
	int* MO = (int*)malloc(4 * heightA*widthB);

	
	fillMat(MA, heightA, widthA);
	fillMat(MB, heightB, widthB);
	printf("in:\n");
	//cout << "input:\n";
	printMat(MA, heightA, widthA);
	printMat(MB, heightB, widthB);

	
	for (int i = 0; i < heightA; i++)
	{
		for (int j = 0; j < widthB; j++)
		{
			int res = 0;
			for (int k = 0; k < widthA; k++)
			{
				res += MA[i*widthA + k] * MB[k*widthB + j];
			}
			MO[i*widthB + j] = res;
		}
	}

	printf("out:\n");
	//cout << "result:\n";
	printMat(MO, heightA, widthB);
	return 0;
}
