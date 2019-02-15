#pragma once
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgcodecs/imgcodecs.hpp"
#include <iostream>
#include <string>
using std::cout;
using std::string;

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
		struct ArrStruct_zp { float arr[Q][Q]; };

	__declspec(align(64))
		struct ArrStruct { float arr[N][N]; };

	// - - - - - - - - - - - - - - - - 
	inline size_t lin(size_t i, size_t j, size_t cols)
	{
		return i * cols + j;
	}

	template <size_t rows, size_t cols, typename T>
	void print(std::string str, T(&array)[rows][cols])
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
	void print(std::string str, const float* x, size_t rows, size_t cols)
	{

		auto loop_print = [=](size_t i, size_t j, float val, string name) -> void
		{cout << name << "(" << i << "," << j << ") = " << val << "\t"; };

		using std::cout;
		cout << __func__ << std::endl;
		cout << str.c_str() << "\n";
		for (size_t i = 0; i < rows; ++i)
		{
			cout << i << ": ";
			for (size_t j = 0; j < cols; ++j)
			{
				auto temp = x[lin(i, j, cols)];
				//cout << temp << '\t';
				loop_print(i, j, temp, "x");
			}
			cout << std::endl;
		}
		cout << "\n";
	}
	// - - - - - - - - - - - - - - - - 
	template <size_t rows, size_t cols>
	inline float* mat_2_arr(float(&x)[rows][cols])
	{
		float* x_arr = new float[rows * cols];
		for (int i = 0; i != rows; i++)
			for (int j = 0; j != cols; j++)
				x_arr[lin(i, j, cols)] = x[i][j];
		return x_arr;
	}
	// - - - - - - - - - - - - - - - - 
	template <size_t rows, size_t cols>
	ArrStruct_zp pad(float(&x)[rows][cols])
	{
		ArrStruct_zp s = {};
		for (size_t n1 = P; n1 != Q - P; ++n1)
			for (size_t n2 = P; n2 != Q - P; ++n2)
				s.arr[n1][n2] = x[n1 - P][n2 - P];
		return s;
	}
	float* pad(const float* x, size_t N_, size_t P_, size_t Q_)
	{
		auto loop_print = [=](size_t i, size_t j, float val, string name) -> void
		{cout << name << "(" << i << "," << j << ") = " << val << "\t"; };

		float* x_zp = new float[Q_ * Q_];
		for (int i = 0; i < Q_*Q_; ++i)
			x_zp[i] = 0;

		float* local_arr = new float[Q_ * Q_];
		for (size_t n1 = P; n1 != Q_ - P_; ++n1)
		{
			for (size_t n2 = P; n2 != Q_ - P_; ++n2)
			{
				//s.arr[n1][n2] = x[n1 - P][n2 - P];
				x_zp[lin(n1, n2, Q_)] = x[lin(n1 - P_, n2 - P_, N_)];
			}
		}

		return x_zp;
	}

	// - - - - - - - - - - - - - - - -
	template <size_t rows, size_t cols>
	ArrStruct conv(float(&x_zp)[rows][cols])
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
				float sum = 0.f;
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
	float* conv(float* x_zp, size_t N_, size_t K_, size_t P_, size_t Q_)
	{
		// Same as previous conv() just with 1D-array
		// Also, added the non-unity filter taps
		//  to avoid the DC-offset introduced during filtering

		// Coefficients in the 2D-FIR filter
		float box_amplitude = 1 / float(K_);

		float* y = new float[N_ * N_];
		for (size_t n1 = P_; n1 < Q_-P_; n1++)
			for (size_t n2 = 0; n2 < N_; n2++)
			{
				float sum = 0.f;
				for (size_t k2 = 0; k2 != K_; k2++)
				{
					int i = n1;
					int j = n2 + k2;
					sum += x_zp[lin(i, j, Q_)];
				}
				y[lin(n2, n1 - P_, N_)] = sum * box_amplitude;
			}

		return y;
	}
	// - - - - - - - - - - - - - - - -
	template <size_t rows, size_t cols>
	void conv_1(float(&x_zp)[rows][cols])
	{
		// Pattern matching the dependency graph 
		// (far left in prototype diagram)
			   
		auto loop_print = [=](size_t i, size_t j, float val, string name) -> void 
		{cout << name << "(" << i << "," << j << ") = " << val << "\t"; };
		
		float y[6][4] = { 0 };
		float z[4][4] = { 0 };

		// 1D-conv (row-vector)
		for (size_t n2 = 0; n2 < 4; n2++)
		{
			cout << "===========================\n";
			cout << "===========================\n";	
			for (size_t n1 = 0; n1 < N; n1++)
			{
				float sum = 0.f;

				for (size_t k1 = 0; k1 != K; k1++)
				{
					cout << "Read From Input:   ";
					for (size_t k2 = 0; k2 != K; k2++)
					{
						int i = n1 + k1;
						int j = n2 + k2;
						sum += x_zp[i][j];
						loop_print(i, j, x_zp[i][j], "x");
					}
					int i = k1 + n1;
					int j = n2;
					y[i][j] = sum;
					cout << "\tWrite to Buffer:  ";
					loop_print(i, j, y[i][j], "y");
					getchar();
					cout << "\n";
				}

				cout << "Read from Buffer:  ";
				sum = 0;
				for (size_t k1 = 0; k1 < 3; k1++)
				{
					int i = k1 + n1;
					int j = n2;
					sum += y[i][n2];
					loop_print(i, j, y[i][j], "y");
				}
				int i = n1;
				int j = n2;
				z[i][j] = sum;
				cout << "\tWrite to Output:  ";
				loop_print(i, j, z[i][j], "z");
				cout << "\n- - - - - - - - - - - - - \n";
				getchar();

			} // n1
		} // n2
	}
	// - - - - - - - - - - - - - - - -
	float* conv_1(float* x_zp, size_t N_, size_t K_, size_t P_, size_t Q_)
	{
		// Pattern matching the dependency graph 
		// (far left in prototype diagram)

		auto loop_print = [=](size_t i, size_t j, float val, string name) -> void
		{cout << name << "(" << i << "," << j << ") = " << val << "\t"; };
		
		float* y = new float[Q_ * N_]; //float y[6][4] = { 0 };
		float* z = new float[N_ * N_]; //float z[4][4] = { 0 };

		// 1D-conv (row-vector)
		for (size_t n2 = 0; n2 < N_; n2++)
		{
			cout << "===========================\n";
			cout << "===========================\n";
			for (size_t n1 = 0; n1 < N_; n1++)
			{
				float sum = 0.f;

				for (size_t k1 = 0; k1 != K_; k1++)
				{
					cout << "Read From Input:   ";
					for (size_t k2 = 0; k2 != K_; k2++)
					{
						int i = n1 + k1;
						int j = n2 + k2;
						auto temp = x_zp[lin(i, j, Q_)];
						sum += temp; //sum += x_zp[i][j];

						//loop_print(i, j, x_zp[i][j], "x");
						loop_print(i, j, temp, "x");
					}
					int i = k1 + n1;
					int j = n2;
					//y[i][j] = sum;
					y[lin(i, j, N_)] = sum;
					cout << "\tWrite to Buffer:  ";
					loop_print(i, j, y[lin(i, j, N_)], "y");
					getchar();
					cout << "\n";
				}

				cout << "Read from Buffer:  ";
				sum = 0;
				for (size_t k1 = 0; k1 < K_; k1++)
				{
					int i = k1 + n1;
					int j = n2;
					//sum += y[i][n2];
					sum += y[lin(i,j,N_)];

					//loop_print(i, j, y[i][j], "y");
					loop_print(i, j, y[lin(i, j, N_)], "y");
				}
				int i = n1;
				int j = n2;
				//z[i][j] = sum;
				z[lin(i,j,N_)] = sum;
				
				cout << "\tWrite to Output:  ";
				//loop_print(i, j, z[i][j], "z");
				loop_print(i, j, z[lin(i,j,N_)], "z");
				cout << "\n- - - - - - - - - - - - - \n";
				getchar();

			} // n1
		} // n2
		return z;
	}
	// - - - - - - - - - - - - - - - -
//	template <size_t rows, size_t cols>
//	void conv_2(float(&x_zp)[rows][cols])
//	{
//		// Maxim
//e
//
//		// Desired reads ordering:                          
//		//	Step-#	   Read-From               Write-to:	
//		//   0     x[00], x[01], x[02]   ->      y[00]		
//		//   1     x[10], x[11], x[12]   ->      y[10]      
//		//   2     x[20], x[21], x[22]   ->      y[20]      
//		//   3     y[00], y[10], y[20]   ->      z[00]                                      
//
//		//   4     x[01], x[02], x[03]   ->      y[01]
//		//   5     x[11], x[12], x[13]   ->      y[11]
//		//   6     x[21], x[22], x[23]   ->      y[21]
//		//   7     y[01], y[11], y[21]   ->      z[01]
//
//		//   8     x[02], x[03], x[04]   ->      y[02]
//		//   9     x[12], x[13], x[14]   ->      y[12]
//		//   10    x[22], x[23], x[24]   ->      y[22]
//		//   11    y[02], y[12], y[22]   ->      z[02]
//
//		//   10    x[03], x[04], x[05]   ->      y[03]
//		//   11    x[13], x[14], x[15]   ->      y[13]
//		//   12    x[23], x[24], x[25]   ->      y[23]
//		//   13    y[03], y[13], y[23]   ->      z[03]
//
//		// From cache: 
//
//
//		//   14    x[03], x[04], x[05]   ->      y[03]
//		//   15    x[13], x[14], x[15]   ->      y[13]
//		//   16    x[23], x[24], x[25]   ->      y[23]
//		//   17    y[03], y[14], y[25]   ->      z[03]
//
//	auto loop_print = [=](size_t i, size_t j, float val, string name) -> void
//	{cout << name << "(" << i << "," << j << ") = " << val << "\t"; };
//
//
//
//		float y[6][4] = { 0 };
//		float z[4][4] = { 0 };
//
//		// 1D-conv (row-vector)
//		for (size_t n1 = 0; n1 < N; n1++)
//		{
//			cout << "===========================\n";
//			cout << "===========================\n";
//			for (size_t n2 = 0; n2 < 4; n2++)
//			{
//				float sum = 0.f;
//
//				for (size_t k1 = 0; k1 != K; k1++)
//				{
//					cout << "Read From Input:   ";
//					for (size_t k2 = 0; k2 != K; k2++)
//					{
//						int i = n1 + k1;
//						int j = n2 + k2;
//						sum += x_zp[i][j];
//						loop_print(i, j, x_zp[i][j], "x");
//					}
//					int i = k1 + n1;
//					int j = n2;
//					y[i][j] = sum;
//					cout << "\tWrite to Buffer:  ";
//					loop_print(i, j, y[i][j], "y");
//					getchar();
//					cout << "\n";
//				}
//
//				cout << "Read from Buffer:  ";
//				sum = 0;
//				for (size_t k1 = 0; k1 < 3; k1++)
//				{
//					int i = k1 + n1;
//					int j = n2;
//					sum += y[i][n2];
//					loop_print(i, j, y[i][j], "y");
//				}
//				int i = n1;
//				int j = n2;
//				z[i][j] = sum;
//				cout << "\tWrite to Output:  ";
//				loop_print(i, j, z[i][j], "z");
//				cout << "\n- - - - - - - - - - - - - \n";
//				getchar();
//
//			} // n1
//		} // n2
//	}
	// - - - - - - - - - - - - - - - - 
	template <size_t rows, size_t cols, typename T>
	ArrStruct conv_fused_1(T(&x_zp)[rows][cols])
	{
		// 2D-conv with maximum locality

		// Do Convolution
		ArrStruct y = {};

		// TODO: Change to full intermediate buffer
		float y_temp[K] = {};

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
					float sum = 0.f;
					for (size_t k2 = 0; k2 != K; ++k2)
					{
						sum += x_zp[n1 + k1][n2 + k2];
						cout << "x_zp[n1 + k1][n2 + k2] = x[" << n1 + k1 << "][" << n2 + k2 << "] =" << x_zp[n1 + k1][n2 + k2] << "\n";
					}
					cout << "Store buffer\n";;
					y_temp[k1] = sum;
				}

				float sum = 0.f;
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
		// Current working prototype

		float x[N][N] = {
			{ 1,  2,  3, 4},
			{ 5,  6,  7, 8},
			{ 9, 10, 11, 12},
			{13, 14, 15, 16}
		};


		// Repeat with 1D-array
		float* x_1D = mat_2_arr(x);
		print("x_arr", x_1D, N, N);
		float* x_1D_zp = pad(x_1D, N, P, Q);
		print("x_arr_zp", x_1D_zp, Q, Q);
		float* y_1D = conv(x_1D_zp, N, K, P, Q);
		print("y_1D", y_1D, N, N);

		// put the thing down, flip it and reverse it:
		float* y_arr_zp = pad(y_1D, N, P, Q);
		print("y_arr_zp", y_arr_zp, Q, Q);
		float* z_1D = conv(y_arr_zp, N, K, P, Q);
		print("z_1D", z_1D, N, N);

		// Zero-pad
		ArrStruct_zp x_zp = pad(x);

		// Display
		print("After zero-padding", x_zp.arr);

		// Do conv - extreme 1
		// TODO: Remove the output written as transpose to 
		// more easily compare different extremes
		ArrStruct y = conv(x_zp.arr);
		print("After conv (stage-1)", y.arr);
		ArrStruct_zp y_zp = pad(y.arr);
		ArrStruct z = conv(y_zp.arr);
		print("After conv (stage-2)", z.arr);

		getchar();
	}
	// - - - - - - - - - - - - - - - - 
	cv::Mat two_D_image(cv::Mat& x)
	{
		const size_t rows = x.rows;
		const size_t cols = x.cols;

		float* x_1D = new float[rows * cols];
		cv::Mat x_f(rows, cols, CV_32FC1);
		x.convertTo(x_f, CV_32FC1);
		for (int i = 0; i < rows; ++i)
			for (int j = 0; j < cols; ++j)
				x_1D[lin(i, j, cols)] = x_f.at<float>(i, j);


		const size_t N_ = cols;
		const size_t P_ = K / 2;		// Zero-padding on each side
		const size_t Q_ = N_ + 2 * P_; // Size of zero-padded in each dim

		// pad
		//float* x_1D = mat_2_arr(x);
		//print("x_arr", x_1D, N, N);
		float* x_1D_zp = pad(x_1D, N_, P_, Q_);
		float* y_1D = conv(x_1D_zp, N_, K, P_, Q_);
		float* y_1D_zp = pad(y_1D, N_, P_, Q_);
		float* z_1D = conv(y_1D_zp, N_, K, P_, Q_);

		// convert back to mat

		cv::Mat z(rows, cols, CV_8UC1);
		for (int i = 0; i < rows*cols ; ++i)
			z.data[i] = (unsigned char)z_1D[i];

		cv::imshow("debug_mat", z);
		cv::waitKey(0);
	

		return z;
	}
	// - - - - - - - - - - - - - - - - 
	void extreme_1()
	{
		// Pattern matching the dependency graph 
		// (far left in prototype diagram)

		// 4x4
		float x_44[N][N] = {
			{ 1,  2,  3, 4},
			{ 5,  6,  7, 8},
			{ 9, 10, 11, 12},
			{13, 14, 15, 16}
		};

		// Step 1: Modify to return dynamic array
		float* x_44_1D = mat_2_arr(x_44);
		print("x_arr", x_44_1D, N, N);
		float* x_44_1D_zp = pad(x_44_1D, N, P, Q);
		print("x_arr_zp", x_44_1D_zp, Q, Q);
		float* y_44_1D = conv_1(x_44_1D_zp, N, K, P, Q);
		print("y_44_1D", y_44_1D, N, N);


		// Step 2: Modify to work with 8x8
		// Step 3: Generalize tiles to arbitrary size

		ArrStruct_zp x_44_zp = pad(x_44);
		print("After zero-padding", x_44_zp.arr);
		conv_1(x_44_zp.arr);
		print("After zero-padding", x_44_zp.arr);
		getchar();
	}
	// - - - - - - - - - - - - - - - - 
	void extreme_2()
	{
		// Matching the prototype indexing

		float x[N][N] = {
			{ 1,  2,  3, 4},
			{ 5,  6,  7, 8},
			{ 9, 10, 11, 12},
			{13, 14, 15, 16}
		};

		// Zero-pad
		ArrStruct_zp x_zp = pad(x);

		// Display
		print("After zero-padding", x_zp.arr);

		// Do conv - extreme 1

		//conv_2(x_zp.arr);

		getchar();
	}

} // namespace two_D