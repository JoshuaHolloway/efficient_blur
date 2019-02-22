#include <matlab_engine.h>
#include "helper.h"
#include "Timer.h"
#include "imp_4.h"
#include "imp_1.h"
#include "tiled.h"
// - - - - - - - - - - - - - - - - 
auto main() -> int
{
	/// Imp-1
#ifdef PROTOTYPE
    // prototype
#else
	const cv::Mat x_mat = cv::imread("lena_512.png", CV_LOAD_IMAGE_GRAYSCALE);
	const size_t M = x_mat.rows, N = x_mat.cols;
	assert(M == N);
	const float* x_f = Helper::mat2arr(x_mat);
	const float* z1 = naive::imp_1_general(x_f, N); 
#endif

	// Tiled
	const size_t power = 5;
	const size_t input_M = pow(2,power), input_N = input_M;
	const size_t kernel_M = 3, kernel_N = kernel_M;
	
	
	const size_t tile_M = 8, tile_N = 8;
	assert(tile_M == tile_N); // TODO: Change

	assert(input_M == input_N); // TODO: Change
	assert(tile_N >= kernel_N && tile_M >= kernel_M); // Tile must be at least the size of the kernel
	assert(tile_N <= input_N  && tile_M <= input_M);   // Tile must be at least the size of the non-zero-padded image
	
#ifdef PROTOTYPE
	// general tiling prototype
	const float* z_proto = Tiled::general(input_M, input_N, kernel_M, kernel_N, tile_M, tile_N);
	//tiled::imp_4(); // 4x4 (6x6 zero-padded) with 4x4 tile prototype
	//tiled::imp_4_input8x8_tile8x8_dynamic(); // 8x8 (10x10 zero-padded) with 8x8 tile prototype
#else
	const float* z4 = tiled::imp_4_general_tile6x6(x_f, N);
#endif

	// TODO: Imp-5: 4-tiles with each in a seperate thread

#ifdef MATLAB
	// Compare result against golden reference
	Matlab::Matlab matlab;
	#ifdef PROTOTYPE
		std::cout << "Sending data to matlab\n";
		matlab.pass_0D_into_matlab(input_M, "M");
		matlab.pass_0D_into_matlab(input_N, "N");
		matlab.pass_0D_into_matlab(kernel_N, "K");
		matlab.pass_2D_into_matlab(z_proto, input_M, input_N, "z_cpp");
		std::cout << "data has been sent to matlab\n";
		std::cout << "running test-bench script...\n";
		matlab.command("box_blur_prototype");
		std::cout << "running test-bench script has been executed\n";
	#else
		matlab.pass_2D_into_matlab(z1, M, N, "z1_cpp");
		matlab.pass_2D_into_matlab(z4, M, N, "z4_cpp");
		matlab.command("box_blur");
		delete[] x_f, z1, z4;
	#endif
#endif

	getchar();
	return 0;
}