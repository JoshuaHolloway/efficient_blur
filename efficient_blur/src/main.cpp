#include <iostream>
using std::cout;

// Run matlab script: matlab/blox_blur.m

constexpr size_t N = 4;
constexpr size_t K = 3;

int main()
{
	//int x[6 * 6] = {
	//	 0, 0, 0, 0, 0, 0,
	//	 0, 1, 2, 3, 4, 0,
	//	 0, 5, 6, 7, 8, 0,
	//	 0, 9,10,11,12, 0,
	//	 0, 13,14,15,16,0,
	//	 0, 0, 0, 0, 0, 0};
	int x[N * N] = {
		 1, 2, 3, 4, 
		 5, 6, 7, 8, 
		 9,10,11,12, 
		 13,14,15,16,};
	int y[N * N] = { 0 };

	auto lin = [](int i, int j, int N) -> int { return i * N + j; };

	int count = 0;
	for (int i = 0; i < N; ++i)
	{
		for (int j = 0; j < N; ++j)
		{
			int sum = 0;
			for (int k = 0; k < K; ++k)
			{
				int kdx = j + k;
				if (kdx >= 0 && kdx < N)
					sum += x[lin(i, kdx, N)];
			}
			y[lin(i,j,N)] = sum;
			//cout << y[count] << "\t";
			//count++;
		}
		cout << "\n";
	}
	cout << "\n";
	
	// Look at entire intermediate matrix
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
			cout << y[lin(i,j,N)] << "\t";
		cout << "\n";
	}
	cout << "\n";

	getchar();
	return 0;
}