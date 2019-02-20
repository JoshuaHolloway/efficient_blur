#pragma once
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgcodecs/imgcodecs.hpp"
#include <iostream>
#include <string>
using std::cout;
using std::string;

namespace tiled
{
	// TODO: Remove:
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
	// - - - - - - - - - - - - - - - - 
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
	void conv_4(float(&x_zp)[rows][cols])
	{
		// Imp-4: Tiled

		auto loop_print = [=](size_t i, size_t j, float val, string name) -> void
		{cout << name << "(" << i << "," << j << ") = " << val << "\t"; };

		float y[4][2] = { 0 };
		float z[4][4] = { 0 };

		int tile_num = 1;
		for (size_t n1 = 0; n1 < N; n1 += 2)
		{
			for (size_t n2 = 0; n2 < N; n2 += 2)
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
		constexpr size_t buffer_width = tile_width - 2 * P;

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

							sum += x_zp[lin(i, j, 10)];
#ifdef PRINT
							loop_print(i, j, x_zp[lin(i, j, 10)], "x");
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
	float* conv_4_tile6x6(const float*x_zp, const size_t M_, const size_t N_)
	{
		// Imp-4: General input and 6x6 tile
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
							sum += y[lin(i, j, buffer_width)];
#ifdef PRINT
							loop_print(i, j, y[i][j], "y");
#endif
						}
						int i = n1 + bi;
						int j = n2 + bj;
						z[lin(i, j, output_width)] = sum;

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
	// ===============================
	// ===============================
	// ===============================
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
		constexpr size_t img_zp_size = img_size + 2 * P;

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
				x_[lin(n1, n2, img_zp_size)] = x[n1 - P][n2 - P];
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
}