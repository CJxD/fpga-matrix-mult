#include <stdio.h>
#include <stdlib.h>

#include <chrono>

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

void multiplyMat(
	int* MO, int* MA, int* MB,
	int heightA, int widthA,
	int heightB, int widthB)
{
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
}

int main()
{
	using namespace std::chrono;
	
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

	auto start = high_resolution_clock::now();
	multiplyMat(MO, MA, MB, heightA, widthA, heightB, widthB);
	auto end = high_resolution_clock::now();

	printf("out:\n");
	//cout << "result:\n";
	printMat(MO, heightA, widthB);
	
	nanoseconds duration = duration_cast<nanoseconds>(end - start);
	printf("time: %llins\n", duration.count());
	
	return 0;
}
