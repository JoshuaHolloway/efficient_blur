#include <math.h>
#include <matlab_engine.h>
#include "helper.h"
#include "Timer.h"
#include "tiled.h"
#include "Image.h"
#include "fast_blur.h"
// - - - - - - - - - - - - - - - - 
typedef __int32 int32;
// - - - - - - - - - - - - - - - -
int at(int i, int j, int N) { return i * N + j; }
// - - - - - - - - - - - - - - - -
void fast_blur(const Image &in, Image &blurred, int stride)
{
	// 2048 x 3072 (6-megapixels)

	// 2048 x 1536	= 264dpi	Apple iPad ("new" 2012)
	// 3072 = 1536*x

	__m128i one_third = _mm_set1_epi16(21846); 
	// (21,846 * 3) = 65,538 = 256^2 + 2

	const int N = stride;

#pragma omp parallel for
	for (int yTile = 0; yTile < in.height(); yTile += 32)
	{
		__m128i a, b, c, sum, avg;
		
		// Number of vectors 
		constexpr auto num_vecs = (256 / 8)*(32 + 2);

		// array of vectors
		__m128i tmp[num_vecs];

		// 
		for (int xTile = 0; xTile < in.width(); xTile += 256)
		{
			__m128i *tmpPtr = tmp;
			for (int y = -1; y < 32 + 1; y++)
			{
				//const uint16_t *inPtr = &(in.data[at(xTile, yTile + y, N)]);
				const float *inPtr = &(in.data[at(xTile, yTile + y, N)]);

				for (int x = 0; x < 256; x += 8)
				{
					a = _mm_loadu_si128((__m128i*)(inPtr - 1));
					b = _mm_loadu_si128((__m128i*)(inPtr + 1));
					c = _mm_load_si128((__m128i*)(inPtr));
					sum = _mm_add_epi16(_mm_add_epi16(a,b), c);
					avg = _mm_mulhi_epi16(sum, one_third);
					_mm_store_si128(tmpPtr++, avg);
					inPtr += 8;
				}
			}
			tmpPtr = tmp;
			for (int y = 0; y < 32; y++)
			{
				__m128i *outPtr = (__m128i *)(&blurred.data[at(xTile, yTile + y, N)]);
				for (int x = 0; x < 256; x += 8)
				{
					a = _mm_load_si128(tmpPtr + (2 * 256) / 8);
					b = _mm_load_si128(tmpPtr + 256 / 8);
					c = _mm_load_si128(tmpPtr++);
					sum = _mm_add_epi16(_mm_add_epi16(a, b), c);
					avg = _mm_mulhi_epi16(sum, one_third);
					_mm_store_si128(outPtr++, avg);
				}
			}
		}
	}
}
// - - - - - - - - - - - - - - - - 
auto main() -> int
{
	cv::Mat x_mat = cv::imread("lena_512.png", CV_LOAD_IMAGE_GRAYSCALE);
	//const size_t M = x_mat.rows, N = x_mat.cols;
	//Image image(x_mat, 3); // Create Image object from image data in cv::Mat object
	
	// Done testing: Tested on T=4,...,N for N=4,6,8
	// Parameters: // TODO: Test on T={4,...,N} for N={4,6,8,16,64,128,256,512,1024,2048,4096}
	//	(note: 4K resolution is 2160x3840 [ROWSxCOLS])
	constexpr size_t N = 8; // Input-image size
	constexpr size_t T = 8; // Tile size

	// Breaks at N=6, T=6

	size_t K = 3; // Kernel size
	size_t P = floor(K / 2); // Zero-padding on each size
	size_t N_zp = N + 2 * P; // Full zero-padded input size
	size_t O = ceil((float)K / 2.f); // Overlap between tiles
	size_t L = T - O; // Non-overlap of input between tiles
	size_t num_tiles = ceil((float)(N_zp - O) / (float)L);
	size_t N_f = L * (num_tiles - 1) + T; // Needed size of zero-padded input to evenly fit tiles
	size_t N_out = N_f - 2 * P; // Output size (before discarding right and bottom)

	float* x = new float[N * N];
	for (int i = 0; i < N * N; i++)
			x[i] = i + 1;


	Image x_image(x, N, N, K, N_f);
	x_image.print("x");
	delete[] x;


	// TODO: modify to fit output corresponding to 
	//       input zero-padded such that all tiles fit inside

	Image z_image(N_out, N_out);  // 
	//z_image.print(); 

	assert(T >= 4); // Algorithm is not designed for shift of 1 
	assert(T >= K); // Kernel must fit inside tile
	assert(T <= N);
	FastBlur::fast_blur_proto(x_image, z_image, N, T, K);
	z_image.print("z");

	std::cout << "\ntruncated:\n";

	Image z_image_trunc(N, N);
	z_image.truncate(N, N, z_image_trunc);
	z_image_trunc.print("Z");
	//z_image.view();



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
#ifdef TEST
	size_t exp = 9;
	size_t input_M = pow(2, exp), input_N = input_M;
	size_t kernel_M = 3, kernel_N = kernel_M;
	size_t tile_M = 3, tile_N = tile_M;
	assert(tile_M == tile_N);   // TODO: Rect
	assert(input_M == input_N); // TODO: Rect
	assert(tile_N >= kernel_N);
	assert(tile_N <= input_N+1);


#ifdef PROTOTYPE
	// general tiling prototype
	const float* test_matrix = Helper::genterate_test_matrix(input_M, input_N);
	LARGE_INTEGER PerfCountFrequencyResult;
	QueryPerformanceFrequency(&PerfCountFrequencyResult);
	const int64 PerfCountFrequency = PerfCountFrequencyResult.QuadPart;

	

	while (true)
	{
		//https://docs.microsoft.com/en-us/windows/desktop/SysInfo/acquiring-high-resolution-time-stamps

		int64 BeginCycleCount = __rdtsc();

		LARGE_INTEGER BeginCounter;
		QueryPerformanceCounter(&BeginCounter);

        const float* z_proto = Tiled::general(test_matrix, input_M, input_N, kernel_M, kernel_N, tile_M, tile_N);

		LARGE_INTEGER EndCounter;
		QueryPerformanceCounter(&EndCounter);

		int64 EndCycleCount = __rdtsc();

		int64 CounterElapsed = EndCounter.QuadPart - BeginCounter.QuadPart;
		int32 ms_per_frame = (1e3*CounterElapsed) / PerfCountFrequency; // counts / (counts/s) = s
		int32 us_per_frame = (1e6*CounterElapsed) / PerfCountFrequency; // counts / (counts/s) = s
		int32 ns_per_frame = (1e9*CounterElapsed) / PerfCountFrequency; // counts / (counts/s) = s
		
		int64 fps = (1 / ((double)ms_per_frame)*1e3);

		int64 CyclesElapsed = EndCycleCount - BeginCycleCount;

		using std::cout;
		cout << std::fixed << std::setprecision(2);
		cout << "ms/frame = " << ms_per_frame << "\t";
		cout << "Frames Per Second = " << fps << "\t";
		cout << "Cycles elapsed = " << CyclesElapsed / 1e6 << " MHz\t";
		cout << "Approximate clock-speed: " << fps * CyclesElapsed / 1e9 << " GHz\n";

		//  ( Frames / s. ) x ( Cycles / frame. ) = Clock Speed


		delete[] z_proto;
	}
	delete[] test_matrix;
#else
	const float* z4 = tiled::imp_4_general_tile6x6(x_f, N);
#endif

#endif // TEST


	// TODO: Imp-5: 4-tiles with each in a seperate thread
#ifdef MATLAB
	// Compare result against golden reference
	Matlab::Matlab matlab;
	#ifdef PROTOTYPE
		std::cout << "Sending data to matlab\n";
		matlab.pass_0D_into_matlab(N, "M");
		matlab.pass_0D_into_matlab(N, "N");
		matlab.pass_0D_into_matlab(K, "K");
		matlab.pass_0D_into_matlab(T, "T");
		matlab.pass_2D_into_matlab(z_image_trunc.data, N, N, "z_cpp");
		std::cout << "data has been sent to matlab\n";
		std::cout << "running test-bench script...\n";
		matlab.command("box_blur_prototype");
		std::cout << "running test-bench script has been executed\n";
		//delete[] z_proto;
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