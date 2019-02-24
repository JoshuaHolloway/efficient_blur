#pragma once
#include "Image.h"
namespace FastBlur
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	inline size_t index(size_t i, size_t j, size_t N)
	{ return i * N + j; };
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	void read_frame_method_1(uint8_t* data, cv::Mat& mat, int stride)
	{
		// This function reads the frame
		// NOTE: This uses one of the slowest methods of indexing into the cv::Mat object

		int rows = mat.rows;
		int cols = mat.cols;
		for (int row = 0; row != rows; ++row)
		{
			uint8_t * row_ptr = data + row * stride;
			for (int col = 0; col != cols; ++col)
				mat.at<unsigned char>(row, col) = *(row_ptr++);
		}
	}
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	void read_frame_method_2(uint8_t* data, cv::Mat& mat, int stride)
	{
		// This function reads the frame
		// NOTE: This function is faster than method 1

		int rows = mat.rows;
		int cols = mat.cols;
		for (int row = 0; row != rows; ++row)
		{
			uint8_t * row_ptr = data + row * stride;
			unsigned char* rowPtr_ = mat.ptr<unsigned char>(row);
			for (int col = 0; col != cols; ++col)
				rowPtr_[col] = *(row_ptr++);
		}
	}
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	void fast_blur(const Image &in, Image &out,
		size_t tile_M, size_t tile_N,
		size_t kernel_size)
	{
		// Input image is zero padded
		size_t P = kernel_size / 2;
		size_t out_M = in.height() - 2 * P;
		size_t out_N = in.width() - 2 * P;

		// Tiles of size YTile x XTile
		size_t Y_Tile = tile_M;
		size_t X_Tile = tile_N;

		// Intermediate buffer-size
		size_t buffer_M = tile_M;
		size_t buffer_N = tile_N - 2 * (kernel_size / 2);

		// Allocate space for buffer and output
		float* buffer = new float[buffer_M * buffer_N];
		float box_amplitude = 1 / float(kernel_size);

		// Work inside tile
		size_t tile_num = 1;   // Input Tile
		size_t buffer_num = 1; // Intermdiate Buffer
		size_t overlap = 2;

		const size_t stride = 1;
		const size_t output_elems_per_tile_M = (buffer_M - kernel_size) / stride + 1;
		const size_t output_elems_per_tile_N = buffer_N;


		for (int y_tile = 0; y_tile < in.height() - overlap; y_tile += Y_Tile - overlap)
		{
			for (int x_tile = 0; x_tile < in.width() - overlap; x_tile += X_Tile - overlap)
			{
				// Write to buffer
				for (size_t y = 0; y < buffer_M; y++)
				{
					float * row_ptr = in.data + (y_tile + y) * in.stride();
					for (size_t x = 0; x < buffer_N; x++)
					{
						int j = x_tile + x;

						// Do 1-D conv [Unrolled Loop]
						float sum = 0.f;
						sum += *(row_ptr + j++);
						sum += *(row_ptr + j++);
						sum += *(row_ptr + j++);
						buffer[index(y, x, buffer_N)] = sum * box_amplitude;
					}
				}

				// Read from buffer
				for (size_t y = 0; y < output_elems_per_tile_M; y++)
				{
					float * row_ptr_ = out.data + (y_tile + y) * out.stride();
					for (size_t x = 0; x < output_elems_per_tile_N; x++)
					{
						// Do 1-D conv [Unrolled Loop]
						float sum = 0.f;
						int i = y;
						sum += *(buffer + index(i++, x, buffer_N));
						sum += *(buffer + index(i++, x, buffer_N));
						sum += *(buffer + index(i++, x, buffer_N));
						int j = x_tile + x;
						*(row_ptr_ + j) = sum * box_amplitude;
					}
				}
			}
		}
	}
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	void fast_blur_proto(const Image &in, Image &blurred,
		size_t tile_M, size_t tile_N,
		size_t kernel_size)
	{
		// Input image is zero padded
		size_t P = kernel_size / 2;
		size_t out_M = in.height() - 2 * P;
		size_t out_N = in.width() - 2 * P;

		// Tiles of size YTile x XTile
		size_t Y_Tile = tile_M;
		size_t X_Tile = tile_N;

		// Intermediate buffer-size
		size_t buffer_M = tile_M;
		size_t buffer_N = tile_N - 2 * (kernel_size / 2);

		// Allocate space for buffer and output
		float* buffer = new float[buffer_M * buffer_N];
		//float* z = new float[out_M * out_N];

		float box_amplitude = 1 / float(kernel_size);

		auto print_tile = [](std::string str, const float* x,
			const size_t rows, const size_t cols,     // Dimensions of matrix
			const size_t tile_M, const size_t tile_N, // Dimensions of tile
			const size_t tile_m, const size_t tile_n) -> void // 2D-tile index
		{
			using std::cout;
			auto loop_print = [=](size_t i, size_t j, float val, std::string name) -> void
			{
				cout << std::fixed << std::setprecision(2);
				cout << name << "(" << i << "," << j << ") = " << val << "\t"; 
			};

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
		};

		// Work inside tile
		size_t tile_num = 1;   // Input Tile
		size_t buffer_num = 1; // Intermdiate Buffer
		size_t overlap = 2;
		for (int y_tile = 0; y_tile < in.height() - overlap; y_tile += Y_Tile - overlap)
		{
			for (int x_tile = 0; x_tile < in.width() - overlap; x_tile += X_Tile - overlap)
			{
				std::cout << "===========================\n";
				std::cout << "        Tile #"
					<< tile_num++ << "\n";
				std::cout << "===========================\n";
				print_tile("tile:", in.data,
					in.height(), in.width(), // Dimensions of matrix
					tile_M, tile_N,            // Dimensions of tile
					y_tile, x_tile);                   // 2D-tile index

				int kernel_size = 3;
				int n1 = y_tile;
				int n2 = x_tile;

				auto loop_print = [](size_t i, size_t j, float val, std::string name) -> void
				{
					std::cout << std::fixed;
					std::cout << std::setprecision(2);
					std::cout << name << "(" << i << "," << j << ") = " << val << "\t";
				};

				// Write to buffer
				for (size_t y = 0; y < buffer_M; y++)
				{
					float * row_ptr = in.data + (y_tile +y) * in.stride();
					for (size_t x = 0; x < buffer_N; x++)
					{
						std::cout << "Read From Input:   ";

						// Do 1-D conv here
						float sum = 0.f;
						for (size_t k2 = 0; k2 < kernel_size; ++k2)
						{
							int i = n1 + y;
							int j = n2 + x + k2;
							auto temp = in.data[index(i, j, in.stride())];
							sum += temp;
							loop_print(i, j, temp, "x");
						}
						size_t i = y;
						size_t j = x;
						std::cout << "\tWrite to Buffer:  ";
						buffer[index(i, j, buffer_N)] = sum * box_amplitude;
						auto temp = buffer[index(i, j, buffer_N)];
						loop_print(i, j, temp, "y");
						std::cout << "\n";
					}
				}


				std::cout << "===========================\n";
				std::cout << "        Buffer #"
					<< buffer_num++ << "\n";
				std::cout << "===========================\n";
				print_tile("tile:", buffer,
					buffer_M, buffer_N, // Dimensions of matrix
					buffer_M, buffer_N, // Dimensions of tile
					y_tile, x_tile);    // 2D-tile index

				const size_t stride = 1;
				const size_t output_elems_per_tile_M = (buffer_M - kernel_size) / stride + 1;
				const size_t output_elems_per_tile_N = buffer_N;

				// Read from buffer
				for (size_t x = 0; x < output_elems_per_tile_N; x++)
				{
					for (size_t y = 0; y < output_elems_per_tile_M; y++)
					{
						std::cout << "Read from Buffer:  ";

						// Do 1-D conv here
						float sum = 0.f;
						for (size_t k1 = 0; k1 < kernel_size; ++k1)
						{
							int i = y + k1;
							int j = x;
							auto temp = buffer[index(i, j, buffer_N)];
							sum += temp;
							loop_print(i, j, temp, "y");
						}
						int i = y_tile + y;
						int j = x_tile + x;

						// NOTE: blurred should not be zero-padded
						blurred.data[index(i, j, out_N)] = sum * box_amplitude;

						std::cout << "\tWrite to Output:  ";
						auto temp = blurred.data[index(i, j, out_N)];
						loop_print(i, j, temp, "z");
						std::cout << "\n";
					}
				}
			}
		}
	}
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}