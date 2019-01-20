#include <iostream>
using std::cout;

// Run matlab script: matlab/blox_blur.m

int main()
{
	int x[6 * 6] = {
		 0, 0, 0, 0, 0, 0,
		 0, 1, 2, 3, 4, 0,
		 0, 5, 6, 7, 8, 0,
		 0, 9,10,11,12, 0,
		 0, 13,14,15,16,0,
		 0, 0, 0, 0, 0, 0};
	int y[6 * 6] = { 0 };

	auto lin = [](int i, int j, int N) -> int { return i * N + j; };

	int count = 0;
	for (int i = 1; i < 5; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			int sum = 0;
			for (int k = 0; k < 3; k++)
				sum += x[lin(i, j + k, 6)];

			y[count] = sum;
			cout << y[count] << "\t";
			count++;
		}
		cout << "\n";
	}
	getchar();

	return 0;
}