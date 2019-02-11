#include <iostream>
using std::cout;
// Run matlab script: matlab/blox_blur.m
constexpr size_t N = 4;         // Size of input image in each dim
constexpr size_t K = 3;         // Size of kernel in each dim
constexpr size_t P = K / 2;		// Zero-padding on each side
constexpr size_t Q = N + 2 * P; // Size of zero-padded in each dim
// - - - - - - - - - - - - - - - - 
struct ArrStruct
{
	int arr[Q][Q];
};
// - - - - - - - - - - - - - - - - 
template <size_t rows, size_t cols>
void print(std::string str,int(&array)[rows][cols])
{
	using std::cout;
	cout << __func__ << std::endl;
	cout << str.c_str() << "\n";
	for (size_t i = 0; i < rows; ++i)
	{
		cout << i << ": ";
		for (size_t j = 0; j < cols; ++j)
			cout << array[i][j] << '\t';
		cout << std::endl;
	}
	cout << "\n";
}
// - - - - - - - - - - - - - - - - 
template <size_t cols>
void print(std::string str, int(&array)[cols])
{
	using std::cout;
	cout << __func__ << std::endl;
	cout << str.c_str() << "\n";

	for (size_t j = 0; j < cols; ++j)
		cout << array[j] << '\t';
	cout << std::endl;

}
// - - - - - - - - - - - - - - - - 
template <size_t rows, size_t cols>
ArrStruct pad(int(&x2)[rows][cols])
{
	ArrStruct s = {};
	for (size_t y = P; y != Q - P; y++)
		for (size_t x = P; x != Q - P; x++)
			s.arr[y][x] = x2[y - P][x - P];
	return s;
}
// - - - - - - - - - - - - - - - - 
int main()
{
	int x1[N] = { 1, 2, 3, 4 };
	int x2[N][N] = {
		{ 1,  2,  3, 4},
		{ 5,  6,  7, 8},
		{ 9, 10, 11, 12},
		{13, 14, 15, 16}
	};

	// Display
	ArrStruct x2_zp = pad(x2);
	print("Before processing", x2_zp.arr);
	//print("x1", x1);
	

	auto lambda = [](int n1, int n2, int val) -> void
	{
		cout << "(n1,n2):(" << n1 << "," << n2 << ")=" << val << "\n";
	};


	// Do Convolution
	int y2_zp[Q][Q] = {};
	for (size_t n1 = 0; n1 != N; n1++)
	{
		cout << "--------------\n";
		for (size_t n2 = 0; n2 != N; n2++)
		{
			// 1D-conv (row-vectors)
			int sum = 0;
			for (size_t k2 = 0; k2 != K; k2++)
			{
				//sum += x2_zp[n1][n2 + k2];
				//lambda(n1, n2, x2_zp[n1][n2 + k2]);
			}
			cout << "sum = " << sum << "\n";
			y2_zp[n1][n2] = sum;
		}
	}
	print("After processing", y2_zp);

	// Extract - can remove by just writing the output like this
	int y2[N][N] = {};
	for (size_t n1 = 0; n1 != N; n1++)
		for (size_t n2 = 0; n2 != N; n2++)
		{
			y2[n1][n2] = y2_zp[n1+1][n2+1];
		}
	print("After processing", y2);



	getchar();
	return 0;
}