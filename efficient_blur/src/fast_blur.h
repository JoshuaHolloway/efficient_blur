#pragma once
#include "Image.h"
namespace FastBlur
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	inline size_t index(size_t i, size_t j, size_t N)
	{ return i * N + j; };
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	void print(const float* data,
		size_t M, size_t N,
		std::string str)
	{
		using std::cout;
		auto loop_print = [=](size_t i, size_t j, float val, std::string name) -> void
		{
			cout << name << "(" << i << "," << j << ") = " << val << "\t"; };

		auto index = [](size_t i, size_t j, size_t N) -> int
		{ return i * N + j; };

		for (size_t i = 0; i < M; ++i)
		{
			for (size_t j = 0; j < N; ++j)
			{
				auto temp = data[index(i, j, N)];
				loop_print(i, j, temp, str.c_str());
			}
			cout << std::endl;
		}
		cout << "\n";
	}
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
		size_t N, size_t T, size_t K)
	{
		// Same as fast_blur_proto() with printing removed

		// TODO: Remove function calls for index
		// TODO: Change buffer to static array
		// TODO: Unroll loops
		// TODO: Pointer arithmetic
		// TODO: Change to row-major writes

		// Zero-padded to fit all tiles in
		size_t in_M = in.height();
		size_t in_N = in.width();

		// Output will temporarilly be size corresponding to input
		// and then discard the right and bottom
		size_t P = K / 2;

		size_t out_M = in_M - 2 * P;
		size_t out_N = in_N - 2 * P;

		// Tiles of size YTile x XTile
		size_t Y_Tile = T;
		size_t X_Tile = T;

		// Intermediate buffer-size
		size_t buffer_M = T;
		size_t buffer_N = T - 2 * (K / 2);

		// Allocate space for buffer and output
		float* buffer = new float[buffer_M * buffer_N];

		float box_amplitude = 1 / float(K);

		size_t output_elems_per_tile_M = T - 2 * P;
		size_t output_elems_per_tile_N = T - 2 * P;


		// Work inside tile
		size_t tile_num = 1;   // Input Tile
		size_t buffer_num = 1; // Intermdiate Buffer
		size_t overlap = 2;
		for (int y_tile = 0;
			y_tile < in.height() - overlap;
			y_tile += Y_Tile - overlap)
		{
			for (int x_tile = 0;
				x_tile < in.width() - overlap;
				x_tile += X_Tile - overlap)
			{
				int n1 = y_tile;
				int n2 = x_tile;

				// Write to buffer
				for (size_t y = 0; y < buffer_M; y++)
				{
					float * row_ptr = in.data + (y_tile + y) * in.stride();
					for (size_t x = 0; x < buffer_N; x++)
					{
						// Do 1-D conv here
						float sum = 0.f;
						for (size_t k2 = 0; k2 < K; ++k2)
						{
							int i = n1 + y;
							int j = n2 + x + k2;
							sum += in.data[index(i, j, in.stride())];
						}
						size_t i = y;
						size_t j = x;
						buffer[index(i, j, buffer_N)] = sum * box_amplitude;
					}
				}

				// TODO: Change to row-major writes

				// Read from buffer
				for (size_t x = 0; x < output_elems_per_tile_N; x++)
				{
					for (size_t y = 0; y < output_elems_per_tile_M; y++)
					{
						// Do 1-D conv
						float sum = 0.f;

						// TODO: Unroll loop
						for (size_t k1 = 0; k1 < K; ++k1)
						{
							int i = y + k1;
							int j = x;
							sum += buffer[index(i, j, buffer_N)];
						}
						int i = y_tile + y;
						int j = x_tile + x;

						auto temp = sum * box_amplitude;
						out.data[index(i, j, out_N)] = temp;
					} // y
				} // x
			} // x_tile
		} // y_tile
		delete[] buffer;
	}
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	void fast_blur_proto(const Image &in, Image &out,
		size_t N, size_t T, size_t K)
	{
		// prototype for fast_blur() with printing values and no pointer arithmetic

		// Zero-padded to fit all tiles in
		size_t in_M = in.height();
		size_t in_N = in.width();

		// Output will temporarilly be size corresponding to input
		// and then discard the right and bottom
		size_t P = K / 2;
		//size_t out_M = N;
		//size_t out_N = N;
		size_t out_M = in_M - 2 * P;
		size_t out_N = in_N - 2 * P;

		// Tiles of size YTile x XTile
		size_t Y_Tile = T;
		size_t X_Tile = T;

		// Intermediate buffer-size
		size_t buffer_M = T;
		size_t buffer_N = T - 2 * (K / 2);

		// Allocate space for buffer and output
		float* buffer = new float[buffer_M * buffer_N];
		//float* z = new float[out_M * out_N];

		float box_amplitude = 1 / float(K);

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

			for (size_t i = tile_m; i < tile_m + tile_M; ++i)
			{
				for (size_t j = tile_n; j < tile_n + tile_N; ++j)
				{
					auto temp = x[index(i, j, cols)];
					//cout << temp << '\t';
					loop_print(i, j, temp, str.c_str());
				}
				cout << std::endl;
			}
			cout << "\n";
		};


		using std::cout;
		cout << "=============================\n";
		cout << "Complete Input Matrix " << in.height() << " x " << in.width() << "\n";
		cout << "=============================\n";
		in.print("x");
		cout << "\n";



		// Work inside tile
		size_t tile_num = 1;   // Input Tile
		size_t buffer_num = 1; // Intermdiate Buffer
		size_t overlap = 2;
		for (int y_tile = 0; 
			y_tile < in.height() - overlap; 
			y_tile += Y_Tile - overlap)
		{
			for (int x_tile = 0; 
				x_tile < in.width() - overlap; 
				x_tile += X_Tile - overlap)
			{
				cout << "===================================\n";
				cout << "        Tile #"
					<< tile_num++ << " (" << Y_Tile << " x " << X_Tile << ")\n";
				cout << "===================================\n";
				print_tile("x:", in.data,
					in.height(), in.width(), // Dimensions of matrix
					T, T,            // Dimensions of tile
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
						cout << "\tWrite to Buffer:  ";
						buffer[index(i, j, buffer_N)] = sum * box_amplitude;
						auto temp = buffer[index(i, j, buffer_N)];
						loop_print(i, j, temp, "y");
						cout << "\n";
					}
					int debug = 0;
				}


				cout << "===============================\n";
				cout << "        Buffer #"
					<< buffer_num++ << "\n";
				cout << "===============================\n";
				print(buffer, buffer_M, buffer_N, "y");

				size_t stride = 1;
				//size_t output_elems_per_tile_M = (buffer_M - kernel_size) / stride + 1;
				//size_t output_elems_per_tile_N = buffer_N;

				size_t output_elems_per_tile_M = T - 2*P;
				size_t output_elems_per_tile_N = T - 2*P;

				// Read from buffer
				for (size_t x = 0; x < output_elems_per_tile_N; x++)
				{
					for (size_t y = 0; y < output_elems_per_tile_M; y++)
					{
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

						// NOTE: out should not be zero-padded
						auto temp = sum * box_amplitude;
						out.data[index(i, j, out_N)] = temp;

						cout << "\tWrite to Output:  ";

						loop_print(i, j, temp, "z");
						cout << "\n";

						/// Look at full output
						//cout << "\n";
						//print(out.data, N, N, "z");
						//cout << "\n";
					}
				}
			}
		}

		cout << "\nDone with Conv\n";
		delete[] buffer;
	}
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		// - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	void fast_blur_simd(const Image &in, Image &out,
		size_t tile_M, size_t tile_N,
		size_t kernel_size)
	{
		// modified fast_blur_simd() to perform each inner loop step in simd registers

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
}