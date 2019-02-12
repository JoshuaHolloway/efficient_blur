#pragma once
#include <iostream>
using std::cout;

namespace two_D
{
	// Run matlab script: matlab/blox_blur.m
	constexpr size_t N = 4;         // Size of input image in each dim
	constexpr size_t K = 3;         // Size of kernel in each dim
	constexpr size_t P = K / 2;		// Zero-padding on each side
	constexpr size_t Q = N + 2 * P; // Size of zero-padded in each dim
	// - - - - - - - - - - - - - - - - 
	
	// L1 cache-line is 64-Bytes
	__declspec(align(64))
		struct ArrStruct_zp { int arr[Q][Q]; };

	__declspec(align(64))
		struct ArrStruct { int arr[N][N]; };
	
	template <size_t rows, size_t cols>
	void print(std::string str, int(&array)[rows][cols])
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
	template <size_t rows, size_t cols, typename T>
	ArrStruct_zp pad(T(&x)[rows][cols])
	{
		ArrStruct_zp s = {};
		for (size_t n1 = P; n1 != Q - P; ++n1)
			for (size_t n2 = P; n2 != Q - P; ++n2)
				s.arr[n1][n2] = x[n1 - P][n2 - P];
		return s;
	}
	// - - - - - - - - - - - - - - - -
	template <size_t rows, size_t cols, typename T>
	ArrStruct conv(T(&x_zp)[rows][cols])
	{
		// (Output is non-zero padded)
		// 1D-conv of (1xK) box-filter with (1xN_) signal
		// Write output in transposed form
		// MATLAB: 
		//   for i = 1:N_
		//      y(:, i) = conv(x(i, :), h, 'same')';
		//   end
		
		// Do Convolution
		ArrStruct y = {};

		// 1D-conv (row-vector)
		for (size_t n1 = P; n1 < Q-P; n1++)
		{
			cout << "===========================\n";
			cout << "n1 = " << n1 << "\n";
			cout << "===========================\n";
			for (size_t n2 = 0; n2 < 4; n2++)
			{
				int sum = 0;
				for (size_t k2 = 0; k2 != K; k2++)
				{
					sum += x_zp[n1][n2 + k2];
					cout << "n2=" << n2 << " k2=" << k2 << " [n2+k2]=" << n2 + k2;
					cout << "  (n1,n2):(" << n1 << "," << n2 << ")=" << x_zp[n1][n2 + k2] << "\n";
				}
				y.arr[n2][n1 - P] = sum;
				cout << "- - - - - - - - - - - - - \n";
				getchar();
			}
			int debug = 0;
		}

		return y;
	}
	// - - - - - - - - - - - - - - - - 
	template <size_t rows, size_t cols, typename T>
	ArrStruct conv_fused_1(T(&x_zp)[rows][cols])
	{
		// 2D-conv with maximum locality

		// Do Convolution
		ArrStruct y = {};

		// TODO: Change to full intermediate buffer
		int y_temp[K] = {};

		// 1D-conv (row-vector)
		for (size_t n1 = P; n1 < Q - P; ++n1)
		{
			cout << "===========================\n";
			cout << "n1 = " << n1 << "\n";
			cout << "===========================\n";
			for (size_t n2 = 0; n2 < 4; ++n2)
			{
				for (size_t k1 = 0; k1 != K; ++k1)
				{
					int sum = 0;
					for (size_t k2 = 0; k2 != K; ++k2)
					{
						sum += x_zp[n1 + k1][n2 + k2];
						cout << "x_zp[n1 + k1][n2 + k2] = x[" << n1 + k1 << "][" << n2 + k2 << "] =" << x_zp[n1 + k1][n2 + k2] << "\n";
					}
					cout << "Store buffer\n";;
					y_temp[k1] = sum;
				}

				int sum = 0;
				for (size_t k1 = 0; k1 != K; ++k1)
				{
					cout << "Add redundant\n";;
					sum += y_temp[k1];
				}

				y.arr[n2][n1 - P] = sum;
				cout << "- - - - - - - - - - - - - \n";
				getchar();
			}
			int debug = 0;
		}

		return y;
	}

	// TODO: Create new conv-stage-1() func where
	//       the output is written zero padded in cols
	//
	// TODO: Create new conv-stage-2() func that takes as input
	//       the col-zero padded matrix and writes output
	//       not zero-padded

	// - - - - - - - - - - - - - - - - 
	void two_D()
	{
		int x[N][N] = {
			{ 1,  2,  3, 4},
			{ 5,  6,  7, 8},
			{ 9, 10, 11, 12},
			{13, 14, 15, 16}
		};

		// Zero-pad
		ArrStruct_zp x_zp = pad(x);

		// Display
		print("After zero-padding", x_zp.arr);

		//// Do conv - extreme 1
		//// TODO: Remove the output written as transpose to 
		//// more easily compare different extremes
		//ArrStruct y = conv(x_zp.arr);
		//print("After conv (stage-1)", y.arr);
		//ArrStruct_zp y_zp = pad(y.arr);
		//ArrStruct z = conv(y_zp.arr);
		//print("After conv (stage-2)", z.arr);

		// Do 
		cout << "Entering fused conv\n";
		conv_fused_1(x_zp.arr);

		getchar();
	}
} // namespace two_D