#include <matlab_engine.h>
#include "helper.h"
#include "Timer.h"
#include "tiled.h"
// - - - - - - - - - - - - - - - - 
typedef __int32 int32;
// - - - - - - - - - - - - - - - -
class Image
{
public:
	int height() const { return M; }
	int width() const { return N; }
	
public:
	int M{ 0 };
	int N{ 0 };
	uint16_t *data;
};
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
				const uint16_t *inPtr = &(in.data[at(xTile, yTile + y, N)]);

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
		
		char buffer[256]; // 256Bytes
		wsprintf(buffer, "ms/frame: %dms\n", ms_per_frame);
		OutputDebugStringA(buffer);
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
		delete[] z_proto;
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