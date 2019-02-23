#include <matlab_engine.h>
#include "helper.h"
#include "Timer.h"
#include "imp_4.h"
#include "imp_1.h"
#include "tiled.h"
// - - - - - - - - - - - - - - - - 
typedef __int32 int32;
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
	const size_t power = 7;
	const size_t input_M = pow(2,power), input_N = input_M;
	const size_t kernel_M = 3, kernel_N = kernel_M;
	
	
	const size_t tile_M = 4, tile_N = tile_M;
	assert(tile_M == tile_N); // TODO: Change

	assert(input_M == input_N); // TODO: Change
	assert(tile_N >= kernel_N && tile_M >= kernel_M); // Tile must be at least the size of the kernel
	assert(tile_N <= input_N  && tile_M <= input_M);   // Tile must be at least the size of the non-zero-padded image
	
#ifdef PROTOTYPE

	// general tiling prototype
	const float* test_matrix = Helper::genterate_test_matrix(input_M, input_N);

	LARGE_INTEGER PerfCountFrequencyResult;
	QueryPerformanceFrequency(&PerfCountFrequencyResult);
	//BOOL WINAPI QueryPerformanceFrequency(
	//	_Out_ LARGE_INTEGER *lpFrequency
	//);
	const int64 PerfCountFrequency = PerfCountFrequencyResult.QuadPart;

	while (true)
	{
		//https://docs.microsoft.com/en-us/windows/desktop/SysInfo/acquiring-high-resolution-time-stamps
		LARGE_INTEGER BeginCounter;
		//typedef struct _LARGE_INTEGER {
		//	LONGLONG QuadPart;
		//} LARGE_INTEGER;

		QueryPerformanceCounter(&BeginCounter);
		//BOOL WINAPI QueryPerformanceCounter(
		//	_Out_ LARGE_INTEGER *lpPerformanceCount
		//);

		const float* z_proto = Tiled::general(test_matrix, input_M, input_N, kernel_M, kernel_N, tile_M, tile_N);

		LARGE_INTEGER EndCounter;
		QueryPerformanceCounter(&EndCounter);

		int32 CounterElapsed = EndCounter.QuadPart - BeginCounter.QuadPart;
		int32 ms_per_frame = (1e3*CounterElapsed) / PerfCountFrequency; // counts / (counts/s) = s
		int32 us_per_frame = (1e6*CounterElapsed) / PerfCountFrequency; // counts / (counts/s) = s
		int32 ns_per_frame = (1e9*CounterElapsed) / PerfCountFrequency; // counts / (counts/s) = s

		char buffer[256]; // 256Bytes
		wsprintf(buffer, "ms/frame: %dms\n", ms_per_frame);
		OutputDebugStringA(buffer);
		std::cout << "ms/frame = " << ms_per_frame << "\t";
		std::cout << "Frames Per Second = " << (1/((double)ms_per_frame)*1e3)<< "\n";


		delete[] z_proto;
	}
	delete[] test_matrix;
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
		matlab.pass_0D_into_matlab(tile_N, "T");
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