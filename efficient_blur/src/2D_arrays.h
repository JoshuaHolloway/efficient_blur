#pragma once
#include <iostream>
using std::cout;

// TODO: Modify the current version of conv to work on arbitrary size image
//			-Returning the 2D-array via a struct prohibits the variable size.
//			-This should be fixed by using a dynamic 2D array.

// Returning 2D-Arrays from functionss
// https://stackoverflow.com/questions/8767166/passing-a-2d-array-to-a-c-function

// Passing 2D-Arrays into functions
// https://stackoverflow.com/questions/8617683/return-a-2d-array-from-a-function

// Method 1: Dynamic
void printArray(int** arr, int N)
{
	for (int i = 0; i < N; ++i)
	{
		for (int j = 0; j < N; ++j)
			cout << arr[i][j] << " ";
		cout << "\n";
	}
}
int** getArray(int N)
{
	int** arr = new int*[N];
	for (int i = 0; i < N; ++i)
	{
		arr[i] = new int[N];
		for (int j = 0; j < N; ++j)
			arr[i][j] = 1;
	}
	return arr;
}
// Method 2: Static
// Method 3: Struct

// - - - - - - - - - - - - - - - - 
// Static 2D-arrays
template <size_t rows, size_t cols>
void pass(float(&x)[rows][cols])
{
	for (size_t n1 = 0; n1 != rows; ++n1)
		for (size_t n2 = 0; n2 != cols; ++n2)
			auto temp = x[n1][n2];
}