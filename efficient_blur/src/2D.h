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
				// This is wrong - the sum should be inside the next loop!
				// This is wrong - the sum should be inside the next loop!
				// See how I fixed this in the following conv_1() function
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
				

				for (size_t k1 = 0; k1 != K_; k1++)
				{
					cout << "Read From Input:   ";
					float sum = 0.f;
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
				float sum = 0;
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
	template <size_t rows, size_t cols>
	void conv_2(float(&x_zp)[rows][cols])
	{

	auto loop_print = [=](size_t i, size_t j, float val, string name) -> void
	{cout << name << "(" << i << "," << j << ") = " << val << "\t"; };



		float y[6][4] = { 0 };
		float z[4][4] = { 0 };

		for (size_t n1 = 0; n1 < N; n1++)
		{
			cout << "===========================\n";
			cout << "===========================\n";
			for (size_t n2 = 0; n2 < N; n2++)
			{
				for (size_t k1 = 0; k1 != K; k1++)
				{
					float sum = 0.f;
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
				float sum = 0;
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

	// - - - - - - - - - - - - - - - -
	template <size_t rows, size_t cols>
	void conv_4(float(&x_zp)[rows][cols])
	{
		// Imp-4: Tiled

		auto loop_print = [=](size_t i, size_t j, float val, string name) -> void
		{cout << name << "(" << i << "," << j << ") = " << val << "\t"; };

		float y[4][2] = { 0 };
		float z[4][4] = { 0 };

		int tile_num = 1;
		for (size_t n1 = 0; n1 < N; n1+=2)
		{
			for (size_t n2 = 0; n2 < N; n2+=2)
			{
				cout << "===========================\n";
				cout << "        Tile #" 
					<< tile_num++ << "\n";
				cout << "===========================\n";

				// Tile loop:
				for (size_t bi = 0; bi < 4; bi++) // tile-row
				{
					for (size_t bj = 0; bj < 2; bj++) // tile-col
					{
						// Do 1-D conv here				
						float sum = 0.f;
						cout << "Read From Input:   ";
						for (size_t k2 = 0; k2 != K; k2++)
						{
							int i = n1 + bi;
							int j = n2 + k2 + bj;
							sum += x_zp[i][j];
							loop_print(i, j, x_zp[i][j], "x");
						}
						size_t i = bi;
						size_t j = bj;

						y[i][j] = sum;

						cout << "\tWrite to Buffer:  ";
						loop_print(i, j, y[i][j], "y");
						getchar();
					} // bi
				} // bj

				for (size_t bj = 0; bj < 2; bj++) // tile-col
				{
					for (size_t bi = 0; bi < 2; bi++) // tile-row
					{
						cout << "Read from Buffer:  ";
						float sum = 0;
						for (size_t k1 = 0; k1 < 3; k1++)
						{
							int i = bi + k1;
							int j = bj;// oj + n2;
							

							// TODO: Change buffer to 1D-array
							
							
							// This indexing is off
							sum += y[i][j];



							loop_print(i, j, y[i][j], "y");
						}

						// This output index is  wrong
						int i = n1 + bi;
						int j = n2 + bj;

						// This index is off!

						z[i][j] = sum;
						cout << "\tWrite to Output:  ";
						loop_print(i, j, z[i][j], "z");
						getchar();
					}
				}


			} // n2
		} // n1
	}
	// - - - - - - - - - - - - - - - -
	template <size_t rows, size_t cols>
	void conv_4_input8x8_tile4x4(float(&x_zp)[rows][cols])
	{
		// Imp-4: 8x8 input and 6x6 tile

		auto loop_print = [=](size_t i, size_t j, float val, string name) -> void
		{cout << name << "(" << i << "," << j << ") = " << val << "\t"; };

		float y[4][2] = { 0 };
		float z[8][8] = { 0 };

		int tile_num = 1;
		for (size_t n1 = 0; n1 < 8; n1 += 2)
		{
			for (size_t n2 = 0; n2 < 8; n2 += 2)
			{
				cout << "===========================\n";
				cout << "        Tile #"
					<< tile_num++ << "\n";
				cout << "===========================\n";

				const int tile_height = 4;
				const int tile_width = 2;

				// Tile loop:
				for (size_t bi = 0; bi < tile_height; bi++) // tile-row
				{
					for (size_t bj = 0; bj < tile_width; bj++) // tile-col
					{
						// Do 1-D conv here				
						float sum = 0.f;
						cout << "Read From Input:   ";
						for (size_t k2 = 0; k2 != K; k2++)
						{
							int i = n1 + bi;
							int j = n2 + k2 + bj;
							sum += x_zp[i][j];
							loop_print(i, j, x_zp[i][j], "x");
						}
						size_t i = bi;
						size_t j = bj;

						y[i][j] = sum;

						cout << "\tWrite to Buffer:  ";
						loop_print(i, j, y[i][j], "y");
						getchar();
					} // bi
				} // bj

				for (size_t bj = 0; bj < 2; bj++) // tile-col
				{
					for (size_t bi = 0; bi < 2; bi++) // tile-row
					{
						cout << "Read from Buffer:  ";
						float sum = 0;
						for (size_t k1 = 0; k1 < 3; k1++)
						{
							int i = bi + k1;
							int j = bj;

							sum += y[i][j];
							loop_print(i, j, y[i][j], "y");
						}

						int i = n1 + bi;
						int j = n2 + bj;

						z[i][j] = sum;
						cout << "\tWrite to Output:  ";
						loop_print(i, j, z[i][j], "z");
						getchar();
					}
				}
			} // n2
		} // n1
	}
	// - - - - - - - - - - - - - - - -
	template <size_t rows, size_t cols>
	void conv_4_input8x8_tile6x6(float(&x_zp)[rows][cols])
	{
		// Imp-4: 8x8 input and 6x6 tile

		auto loop_print = [=](size_t i, size_t j, float val, string name) -> void
		{cout << name << "(" << i << "," << j << ") = " << val << "\t"; };
		
		constexpr size_t output_height = 8;
		constexpr size_t output_width = 8;

		constexpr size_t tile_height = 6;
		constexpr size_t tile_width = 6;

		constexpr size_t output_block_height = tile_height - 2 * P;
		constexpr size_t output_block_width = tile_width - 2 * P;

		constexpr size_t buffer_height = tile_height;
		constexpr size_t buffer_width = tile_width - 2*P;

		float y[buffer_height][buffer_width] = { 0 };
		float z[output_height][output_width] = { 0 };

		int tile_num = 1;
		for (size_t n1 = 0; n1 < output_height; n1 += output_block_height)
		{
			for (size_t n2 = 0; n2 < output_width; n2 += output_block_width)
			{
				cout << "===========================\n";
				cout << "        Tile #"
					<< tile_num++ << "\n";
				cout << "===========================\n";



				// Tile loop:
				for (size_t bi = 0; bi < buffer_height; bi++) // tile-row
				{
					for (size_t bj = 0; bj < buffer_width; bj++) // tile-col
					{
						// Do 1-D conv here				
						float sum = 0.f;
						cout << "Read From Input:   ";
						for (size_t k2 = 0; k2 != K; k2++)
						{
							int i = n1 + bi;
							int j = n2 + k2 + bj;
							sum += x_zp[i][j];
							loop_print(i, j, x_zp[i][j], "x");
						}
						size_t i = bi;
						size_t j = bj;

						y[i][j] = sum;

						cout << "\tWrite to Buffer:  ";
						loop_print(i, j, y[i][j], "y");
						getchar();
					} // bi
				} // bj

				for (size_t bj = 0; bj < output_block_height; bj++) // tile-col
				{
					for (size_t bi = 0; bi < output_block_width; bi++) // tile-row
					{
						cout << "Read from Buffer:  ";
						float sum = 0;
						for (size_t k1 = 0; k1 < 3; k1++)
						{
							int i = bi + k1;
							int j = bj;

							sum += y[i][j];
							loop_print(i, j, y[i][j], "y");
						}

						int i = n1 + bi;
						int j = n2 + bj;

						z[i][j] = sum;
						cout << "\tWrite to Output:  ";
						loop_print(i, j, z[i][j], "z");
						getchar();
					}
				}
			} // n2
		} // n1
	}
	// - - - - - - - - - - - - - - - -
	float* conv_4_input8x8_tile6x6_dynamic(float*x_zp)
	{
		// Imp-4: 8x8 input and 6x6 tile
		// with dynamic array as input

		auto loop_print = [=](size_t i, size_t j, float val, string name) -> void
		{cout << name << "(" << i << "," << j << ") = " << val << "\t"; };

		constexpr size_t output_height = 8;
		constexpr size_t output_width = 8;

		constexpr size_t tile_height = 6;
		constexpr size_t tile_width = 6;

		constexpr size_t output_block_height = tile_height - 2 * P;
		constexpr size_t output_block_width = tile_width - 2 * P;

		constexpr size_t buffer_height = tile_height;
		constexpr size_t buffer_width = tile_width - 2 * P;

		float y[buffer_height][buffer_width] = { 0 };
		float z[output_height][output_width] = { 0 };

		int tile_num = 1;
		for (size_t n1 = 0; n1 != output_height; n1 += output_block_height)
		{
			for (size_t n2 = 0; n2 != output_width; n2 += output_block_width)
			{
#ifdef PRINT
				cout << "===========================\n";
				cout << "        Tile #"
					<< tile_num++ << "\n";
				cout << "===========================\n";
#endif

				// Tile loop:
				for (size_t bi = 0; bi != buffer_height; ++bi) // tile-row
				{
					for (size_t bj = 0; bj != buffer_width; ++bj) // tile-col
					{
						// Do 1-D conv here				
						float sum = 0.f;
#ifdef PRINT
						cout << "Read From Input:   ";
#endif
						for (size_t k2 = 0; k2 != K; ++k2)
						{
							int i = n1 + bi;
							int j = n2 + k2 + bj;

							sum += x_zp[lin(i,j,10)];					
#ifdef PRINT
							loop_print(i, j, x_zp[lin(i,j,10)], "x");
#endif
						}
						size_t i = bi;
						size_t j = bj;

						y[i][j] = sum;

#ifdef PRINT
						cout << "\tWrite to Buffer:  ";
						loop_print(i, j, y[i][j], "y");
						getchar();
#endif
					} // bi
				} // bj

				for (size_t bj = 0; bj != output_block_height; ++bj) // tile-col
				{
					for (size_t bi = 0; bi != output_block_width; ++bi) // tile-row
					{
						//cout << "Read from Buffer:  ";
						float sum = 0;
						for (size_t k1 = 0; k1 != K; k1++)
						{
							int i = bi + k1;
							int j = bj;

							sum += y[i][j];
#ifdef PRINT
							loop_print(i, j, y[i][j], "y");
#endif
						}

						int i = n1 + bi;
						int j = n2 + bj;

						z[i][j] = sum;

#ifdef PRINT
						cout << "\tWrite to Output:  ";
						loop_print(i, j, z[i][j], "z");
						getchar();
#endif
					}
				}
			} // n2
		} // n1

#ifdef PRINT
		print("z", z);
#endif
		float* z_f = new float[output_height * output_width];
		for (int i = 0; i != output_height; ++i)
			for (int j = 0; j != output_width; ++j)
				z_f[i * output_width + j] = z[i][j];
		return z_f;
	}
	// - - - - - - - - - - - - - - - -
	float* conv_4_tile6x6(float*x_zp, const size_t M_, const size_t N_)
	{
		// Imp-4: 8x8 input and 6x6 tile
		// with dynamic array as input

		auto loop_print = [=](size_t i, size_t j, float val, string name) -> void
		{cout << name << "(" << i << "," << j << ") = " << val << "\t"; };

		const size_t output_height = M_;
		const size_t output_width = N_;

		// Zero padded size
		const size_t input_height = output_height + 2 * P;
		const size_t input_width = output_width + 2 * P;

		// Input tile-size
		const size_t tile_height = 6;
		const size_t tile_width = 6;

		// Output tile-size
		const size_t output_block_height = tile_height - 2 * P;
		const size_t output_block_width = tile_width - 2 * P;

		// Intermediate buffer-size
		const size_t buffer_height = tile_height;
		const size_t buffer_width = tile_width - 2 * P;

		// Allocate space for buffer and output
		float* y = new float[buffer_height * buffer_width];
		float* z = new float[output_height * output_width];

		int tile_num = 1;
		for (size_t n1 = 0; n1 != output_height; n1 += output_block_height)
		{
			for (size_t n2 = 0; n2 != output_width; n2 += output_block_width)
			{
#ifdef PRINT
				cout << "===========================\n";
				cout << "        Tile #"
					<< tile_num++ << "\n";
				cout << "===========================\n";
#endif
				// Tile loop:
				for (size_t bi = 0; bi != buffer_height; ++bi) // tile-row
				{
					for (size_t bj = 0; bj != buffer_width; ++bj) // tile-col
					{
						// Do 1-D conv here				
						float sum = 0.f;
#ifdef PRINT
						cout << "Read From Input:   ";
#endif
						for (size_t k2 = 0; k2 != K; ++k2)
						{
							int i = n1 + bi;
							int j = n2 + k2 + bj;
							sum += x_zp[lin(i, j, input_width)];
#ifdef PRINT
							loop_print(i, j, x_zp[lin(i, j, input_width)], "x");
#endif
						}
						size_t i = bi;
						size_t j = bj;
						y[lin(i, j, buffer_width)] = sum;
#ifdef PRINT
						cout << "\tWrite to Buffer:  ";
						loop_print(i, j, y[i][j], "y");
#endif
					} // bi
				} // bj

				for (size_t bj = 0; bj != output_block_height; ++bj) // tile-col
				{
					for (size_t bi = 0; bi != output_block_width; ++bi) // tile-row
					{
#ifdef PRINT
						cout << "Read from Buffer:  ";
#endif
						float sum = 0;
						for (size_t k1 = 0; k1 != K; k1++)
						{
							int i = bi + k1;
							int j = bj;
							sum += y[lin(i,j,buffer_width)];
#ifdef PRINT
							loop_print(i, j, y[i][j], "y");
#endif
						}
						int i = n1 + bi;
						int j = n2 + bj;
						z[lin(i,j,output_width)] = sum;

#ifdef PRINT
						cout << "\tWrite to Output:  ";
						loop_print(i, j, z[lin(i, j, output_width)], "z");
#endif
					}
				}
			} // n2
		} // n1

#ifdef PRINT
		print("z", z, output_height, output_width);
#endif

		return z;
	}

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
		// Imp-1: Using static 2D-arrays

		float x[N][N] = {
			{ 1,  2,  3, 4},
			{ 5,  6,  7, 8},
			{ 9, 10, 11, 12},
			{13, 14, 15, 16}
		};

		//// Repeat with 1D-array
		//float* x_1D = mat_2_arr(x);
		//print("x_arr", x_1D, N, N);
		//float* x_1D_zp = pad(x_1D, N, P, Q);
		//print("x_arr_zp", x_1D_zp, Q, Q);
		//float* y_1D = conv(x_1D_zp, N, K, P, Q);
		//print("y_1D", y_1D, N, N);

		//// put the thing down, flip it and reverse it:
		//float* y_arr_zp = pad(y_1D, N, P, Q);
		//print("y_arr_zp", y_arr_zp, Q, Q);
		//float* z_1D = conv(y_arr_zp, N, K, P, Q);
		//print("z_1D", z_1D, N, N);

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
	void imp_1()
	{
		// Imp-1: Using dynamic arrays

		// 4x4
		float x_44[N][N] = {
			{ 1,  2,  3, 4},
			{ 5,  6,  7, 8},
			{ 9, 10, 11, 12},
			{13, 14, 15, 16}
		};

		float x_88[8][8] = {};
		for (int i = 0; i < 8 ; ++i)
			for (int j = 0; j < 8; ++j)
				x_88[i][j] = i * 8 + j;

		float* x_44_1D = mat_2_arr(x_44);
		print("x_44_1D", x_44_1D, N, N);
		float* x_44_1D_zp = pad(x_44_1D, N, P, Q);
		print("x_44_1D_zp", x_44_1D_zp, Q, Q);
		float* y_44_1D = conv_1(x_44_1D_zp, N, K, P, Q);
		print("y_44_1D", y_44_1D, N, N);

		/// Step 2: Modify to work with 8x8 - not working yet
		//float* x_88_1D = mat_2_arr(x_88);
		//print("x_88_1D", x_88_1D, N, N);
		//float* x_88_1D_zp = pad(x_88_1D, N, P, Q);
		//print("x_88_1D_zp", x_88_1D_zp, Q, Q);
		//float* y_88_1D = conv_1(x_88_1D_zp, 8, K, P, 8 + 2 * P);
		//print("y_88_1D", y_88_1D, N, N);

		/// Step 3: Generalize tiles to arbitrary size

		ArrStruct_zp x_44_zp = pad(x_44);
		print("After zero-padding", x_44_zp.arr);
		conv_1(x_44_zp.arr);
		print("After zero-padding", x_44_zp.arr);
		getchar();
	}
	// - - - - - - - - - - - - - - - - 
	void imp_2()
	{
		// Imp-2: Maximum Locality

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
		conv_2(x_zp.arr);

		getchar();
	}
	// - - - - - - - - - - - - - - - - 
	void imp_3()
	{
		// Imp-3: Maximum Locality with Re-used Values

	}
	// - - - - - - - - - - - - - - - - 
	void imp_4()
	{
		// Imp-4: Tiled

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

		// Do conv
		conv_4(x_zp.arr);

		getchar();
	}
	// - - - - - - - - - - - - - - - - 
	void imp_4_input8x8_tile4x4()
	{
		// Generalize imp_4 up:
		/// -Step 1: Same as Imp-4 (Tiled) but with 8x8 input (4x4-tile)
		///		=>Done in this function
		// -Step 2: Imp-4 with 8x8 input and 6x6 tile
		// -Step 3: Scale up to image

		float x[8][8] = {
			{ 1, 2, 3, 4, 5, 6, 7, 8},
			{ 9,10,11,12,13,14,15,16},
			{17,18,19,20,21,22,23,24},
			{25,26,27,28,29,30,31,32},
			{33,34,35,36,37,38,39,40},
			{41,42,43,44,45,46,47,48},
			{49,50,51,52,53,54,55,56},
			{57,58,59,60,61,62,63,64}
		};

		// Zero-pad
		float x_[10][10] = {};
		for (size_t n1 = P; n1 != 10 - P; ++n1)
			for (size_t n2 = P; n2 != 10 - P; ++n2)
				x_[n1][n2] = x[n1 - P][n2 - P];

		// Display
		print("After zero-padding", x_);

		// Do conv
		conv_4_input8x8_tile4x4(x_);

		getchar();
	}
	// - - - - - - - - - - - - - - - - 
	void imp_4_input8x8_tile6x6()
	{
		// Generalize imp_4 up:
		// -Step 1: Same as Imp-4 (Tiled) but with 8x8 input (4x4-tile)
		/// -Step 2: Imp-4 with 8x8 input and 6x6 tile
		///		=>Done in this function
		// -Step 3: Scale up to image
		// -Step 4: Scale up to image
		// -Step 5: Use actual pixel data as input

		float x[8][8] = {
			{ 1, 2, 3, 4, 5, 6, 7, 8},
			{ 9,10,11,12,13,14,15,16},
			{17,18,19,20,21,22,23,24},
			{25,26,27,28,29,30,31,32},
			{33,34,35,36,37,38,39,40},
			{41,42,43,44,45,46,47,48},
			{49,50,51,52,53,54,55,56},
			{57,58,59,60,61,62,63,64}
		};

		// Zero-pad
		float x_[10][10] = {};
		for (size_t n1 = P; n1 != 10 - P; ++n1)
			for (size_t n2 = P; n2 != 10 - P; ++n2)
				x_[n1][n2] = x[n1 - P][n2 - P];

		// Display
		print("After zero-padding", x_);

		// Do conv
		conv_4_input8x8_tile6x6(x_);

		getchar();
	}
	// - - - - - - - - - - - - - - - - 
	float* imp_4_input8x8_tile6x6_dynamic()
	{
		// Generalize imp_4 up:
		// -Step 1: Same as Imp-4 (Tiled) but with 8x8 input (4x4-tile)
		// -Step 2: Imp-4 with 8x8 input and 6x6 tile
		/// -Step 3: Use dynamic array as input
		///		=>Done in this function
		// -Step 4: Scale up to image
		// -Step 5: Use actual pixel data as input

		// toy image size
		constexpr size_t img_size = 8;
		constexpr size_t img_zp_size = img_size + 2*P;

		float x[img_size][img_size] = {
			{ 1, 2, 3, 4, 5, 6, 7, 8},
			{ 9,10,11,12,13,14,15,16},
			{17,18,19,20,21,22,23,24},
			{25,26,27,28,29,30,31,32},
			{33,34,35,36,37,38,39,40},
			{41,42,43,44,45,46,47,48},
			{49,50,51,52,53,54,55,56},
			{57,58,59,60,61,62,63,64}
		};

		// Zero-pad
		float* x_ = new float[img_zp_size * img_zp_size];
		for (int i = 0; i < img_zp_size * img_zp_size; ++i)
			*(x_ + i) = 0;

		for (size_t n1 = P; n1 != img_zp_size - P; ++n1)
			for (size_t n2 = P; n2 != img_zp_size - P; ++n2)
			{
				//x_[n1][n2] = x[n1 - P][n2 - P];
				x_[lin(n1,n2, img_zp_size)] = x[n1 - P][n2 - P];
			}
		
		float* z = conv_4_input8x8_tile6x6_dynamic(x_);

#ifdef PRINT
		// Display
		print("After zero-padding", x_, img_zp_size, img_zp_size);
		print("\nAfter conv", z, 8, 8);
#endif
		return z;
	}
	// - - - - - - - - - - - - - - - - 
	float* imp_4_input16x16_tile6x6_dynamic()
	{
		// Generalize imp_4 up:
		// -Step 1: Same as Imp-4 (Tiled) but with 8x8 input (4x4-tile)
		// -Step 2: Imp-4 with 8x8 input and 6x6 tile
		// -Step 3: Use dynamic array as input
		/// -Step 4: Scale up to image
		///		=>Done in this function
		// -Step 5: Use actual pixel data as input

		// toy image size
		constexpr size_t img_size = 16;
		constexpr size_t img_zp_size = img_size + 2 * P;

		float x[img_size][img_size] = {};
		size_t k(1);
		for (size_t i = 0; i != img_size; ++i)
			for (size_t j = 0; j != img_size; ++j)
				x[i][j] = k++;
			
		// Zero-pad
		float* x_ = new float[img_zp_size * img_zp_size];
		for (int i = 0; i < img_zp_size * img_zp_size; ++i)
			*(x_ + i) = 0;

		for (size_t n1 = P; n1 != img_zp_size - P; ++n1)
			for (size_t n2 = P; n2 != img_zp_size - P; ++n2)
				x_[lin(n1, n2, img_zp_size)] = x[n1 - P][n2 - P];

		float* z = conv_4_tile6x6(x_, img_size, img_size);

#ifdef PRINT
		// Display
		print("After zero-padding", x_, img_zp_size, img_zp_size);
		print("\nAfter conv", z, 16, 16);
#endif
		return z;
	}
	// - - - - - - - - - - - - - - - - 
	float* imp_4_general_tile6x6(cv::Mat mat)
	{
		// -Step 1: Same as Imp-4 (Tiled) but with 8x8 input (4x4-tile)
		// -Step 2: Imp-4 with 8x8 input and 6x6 tile
		// -Step 3: Use dynamic array as input
		// -Step 4: Scale up to image
		// -Step 5: Use pixel data as input
		///		=>Done in this function

		// single float square
		//assert(mat.type() == CV_32FC1);
		assert(mat.rows == mat.cols);

		// toy image size
		const size_t img_size = mat.rows;
		const size_t img_zp_size = img_size + 2 * P;

		// Zero-pad
		float* x_ = new float[img_zp_size * img_zp_size];
		for (int i = 0; i < img_zp_size * img_zp_size; ++i)
			*(x_ + i) = 0;
		for (size_t n1 = P; n1 != img_zp_size - P; ++n1)
			for (size_t n2 = P; n2 != img_zp_size - P; ++n2)
				x_[lin(n1, n2, img_zp_size)] = (float)mat.data[lin(n1 - P, n2 - P, img_size)];

		float* z = conv_4_tile6x6(x_, img_size, img_size);

#ifdef PRINT
		// Display
		print("After zero-padding", x_, img_zp_size, img_zp_size);
		print("\nAfter conv", z, M_, N_);
#endif
		return z;
	}

} // namespace two_D