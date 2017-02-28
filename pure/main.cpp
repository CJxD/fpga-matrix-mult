#include <stdio.h>
#include <stdlib.h>

#include <chrono>

#include "matrix.h"

int main()
{
	using namespace std::chrono;
	
	Matrix MA = matrix_create(3, 2);
	Matrix MB = matrix_create(2, 5);
	
	MA.fill();
	MB.fill();
	printf("in:\n");

	MA.print();
	MB.print();

	Matrix MO;
	auto start = high_resolution_clock::now();
	MO = matrix_multiply(MA, MB);
	auto end = high_resolution_clock::now();

	printf("out:\n");
	MO.print();
	
	nanoseconds duration = duration_cast<nanoseconds>(end - start);
	printf("time: %llins\n", duration.count());
	
	matrix_destroy(MA);
	matrix_destroy(MB);
	matrix_destroy(MO);
	
	return 0;
}
