#pragma once
#include <assert.h>
#include <string>
#include <iostream>
#include <iomanip>

namespace Tiled
{
	// - - - - - - - - - - - - - - - - 
	inline size_t index(size_t i, size_t j, size_t cols)
	{
		return i * cols + j;
	}
	// - - - - - - - - - - - - - - - - 
	void print_matrix(std::string str, const float* x, 
		const size_t rows, const size_t cols)
	{
		using std::cout;
		auto loop_print = [=](size_t i, size_t j, float val, std::string name) -> void
		{cout << name << "(" << i << "," << j << ") = " << val << "\t"; };

		cout << __func__ << std::endl;
		cout << str.c_str() << "\n";
		for (size_t i = 0; i < rows; ++i)
		{
			cout << i << ": ";
			for (size_t j = 0; j < cols; ++j)
			{
				auto temp = x[index(i, j, cols)];
				//cout << temp << '\t';
				loop_print(i, j, temp, "x");
			}
			cout << std::endl;
		}
		cout << "\n";
	}
	// - - - - - - - - - - - - - - - - 
	void print_tile(std::string str, const float* x, 
		const size_t rows, const size_t cols,     // Dimensions of matrix
		const size_t tile_M, const size_t tile_N, // Dimensions of tile
		const size_t tile_m, const size_t tile_n) // 2D-tile index
	{
		using std::cout;
		auto loop_print = [=](size_t i, size_t j, float val, std::string name) -> void
		{cout << name << "(" << i << "," << j << ") = " << val << "\t"; };

		cout << str.c_str() << "\n";


		for (size_t i = tile_m; i < tile_m + tile_M; ++i)
		{
			cout << i << ": ";
			for (size_t j = tile_n; j < tile_n + tile_N; ++j)
			{
				auto temp = x[index(i, j, cols)];
				//cout << temp << '\t';
				loop_print(i, j, temp, "x");
			}
			cout << std::endl;
		}
		cout << "\n";
	}
	// - - - - - - - - - - - - - - - -
	float* conv_tiled(const float*x_zp,
		const size_t M_, const size_t N_, 
		const size_t tile_M, const size_t tile_N,
		const size_t kernel_size)
	{
		auto loop_print = [=](size_t i, size_t j, float val, std::string name) -> void
		{
			std::cout << std::fixed;
			std::cout << std::setprecision(2);
			std::cout << name << "(" << i << "," << j << ") = " << val << "\t"; 
		};

		const size_t output_height = M_;
		const size_t output_width = N_;

		const size_t P = kernel_size / 2;

		// Zero padded size
		const size_t input_height = output_height + 2 * P;
		const size_t input_width = output_width + 2 * P;

		// Input tile-size
		const size_t tile_height = tile_M;
		const size_t tile_width = tile_N;

		// Output tile-size - this is not general
		const size_t output_block_height = tile_height - 2 * P;
		const size_t output_block_width = tile_width - 2 * P;

		// Intermediate buffer-size
		const size_t buffer_height = tile_height;
		const size_t buffer_width = tile_width - 2 * P;

		// Allocate space for buffer and output
		float* y = new float[buffer_height * buffer_width];
		float* z = new float[output_height * output_width];

		float box_amplitude = 1 / float(kernel_size);

		const size_t shift = 2;

		int tile_num = 1;

		const size_t input_M = input_height;
		const size_t num_tiles_M = (input_M - tile_M) / 2 + 1;

		const size_t input_N = input_width;
		const size_t num_tiles_N = (input_N - tile_N) / 2 + 1;

		const size_t stride = 1;
		const size_t output_elems_per_tile_M = (buffer_height - kernel_size) / stride + 1;
		const size_t output_elems_per_tile_N = buffer_width;


		for (size_t n1 = 0; n1 < num_tiles_M * shift; n1 += shift)
		{
			for (size_t n2 = 0; n2 < num_tiles_N * shift; n2 += shift)
			{
#ifdef PRINT
				std::cout << "===========================\n";
				std::cout << "        Tile #"
					<< tile_num++ << "\n";
				std::cout << "===========================\n";
				print_tile("tile:", x_zp,
					input_height, input_width, // Dimensions of matrix
					tile_M, tile_N,            // Dimensions of tile
					n1, n2);                   // 2D-tile index
#endif
				for (size_t b1 = 0; b1 < buffer_height; ++b1)
				{
					for (size_t b2 = 0; b2 < buffer_width; ++b2)
					{
						// Do 1-D conv here				
						float sum = 0.f;
#ifdef PRINT
						std::cout << "Read From Input:   ";
#endif
						for (size_t k2 = 0; k2 < kernel_size; ++k2)
						{
							int i = n1 + b1;
							int j = n2 + k2 + b2;
							sum += x_zp[index(i, j, input_width)];
#ifdef PRINT
							loop_print(i, j, x_zp[index(i, j, input_width)], "x");
#endif
						}
						size_t i = b1;
						size_t j = b2;
						y[index(i, j, buffer_width)] = sum * box_amplitude;
#ifdef PRINT
						std::cout << "\tWrite to Buffer:  ";
						loop_print(i, j, y[index(i, j, buffer_width)], "y");
						//getchar();
						std::cout << "\n";
#endif
					} // b1
				} // b2

				for (size_t b2 = 0; b2 < output_elems_per_tile_M; ++b2)
				{
					for (size_t b1 = 0; b1 < output_elems_per_tile_N; ++b1)
					{
#ifdef PRINT
						std::cout << "Read from Buffer:  ";
#endif
						float sum = 0;
						for (size_t k1 = 0; k1 < kernel_size; k1++)
						{
							int i = b1 + k1;
							int j = b2;
							sum += y[index(i, j, buffer_width)];
#ifdef PRINT
							loop_print(i, j, y[index(i, j, buffer_width)], "y");
#endif
						}
						int i = n1 + b1;
						int j = n2 + b2;
						z[index(i, j, output_width)] = sum * box_amplitude;

#ifdef PRINT
						std::cout << "\tWrite to Output:  ";
						loop_print(i, j, z[index(i, j, output_width)], "z");
						//getchar();
						std::cout << "\n";
#endif					
					}
				}
			} // n2
		} // n1

#ifdef PRINT
		print_matrix("z", z, output_height, output_width);
#endif

		// TODO: Free memory
		//delete[] y;

		return z;
	}
	// - - - - - - - - - - - - - - - - 
	float* general(
		const size_t input_M, const size_t input_N,
		const size_t kernel_M, const size_t kernel_N, 
		const size_t tile_M, const size_t tile_N)
	{
		// TODO: completely generalize the tiling
		// -Should be a function of: Input Size, Tile Size, Kernel Size
		// -Overlap (shift between tiles) is function of kernel size that causes
		//	output partition to have no overlap
		// -Buffer size is function of kernel size and tile-size
		// -Definitions:
		//	1. Input size: The matrix size in one dimension before zero padding
		//  2. Zero-Padding size: Defines the kernel size (2*k+1) and number 
		//     of elements padding each size of input matrix.
		//     Could do this the oposite way by defining kernel size and 
		//     then zero-padding is floor(k/2).
		//  3. Tile size



		// TODO: CHANGE
		// square
		assert(input_M == input_N);
		assert(kernel_M == kernel_N);

		// toy image size
		const size_t P = kernel_M / 2;
		const size_t img_size = input_N;
		const size_t img_zp_size = img_size + 2 * P;


		//float x[img_size][img_size] = {};
		float* x = new float[img_size * img_size];
		size_t k(1);
		for (size_t i = 0; i != img_size; ++i)
			for (size_t j = 0; j != img_size; ++j)
				x[index(i,j,img_size)] = k++;
				//x[i][j] = k++;

		// Zero-pad
		float* x_ = new float[img_zp_size * img_zp_size];
		for (int i = 0; i < img_zp_size * img_zp_size; ++i)
			*(x_ + i) = 0;

		for (size_t n1 = P; n1 != img_zp_size - P; ++n1)
			for (size_t n2 = P; n2 != img_zp_size - P; ++n2)
				x_[index(n1, n2, img_zp_size)] = x[index(n1 - P,n2 - P,img_size)];
				//x_[index(n1, n2, img_zp_size)] = x[n1 - P][n2 - P];

#ifdef PRINT
		print_matrix("After zero-padding", x_, img_zp_size, img_zp_size);
#endif

		float* z = conv_tiled(x_, img_size, img_size, tile_M, tile_N, kernel_N);

#ifdef PRINT
		print_matrix("\nAfter conv", z, tile_N, tile_N);
#endif
		// TODO: Free memory
		//delete[] x_;

		return z;
	}
}