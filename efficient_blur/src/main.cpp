#include <math.h>
#include <iomanip>
#include <fstream>
// - - - - - - - - - - - - - - - -
#include <matlab_engine.h>
#include "helper.h"
#include "Timer.h"
#include "tiled.h"
#include "Image.h"
#include "fast_blur.h"
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
	LARGE_INTEGER PerfCountFrequencyResult;
	QueryPerformanceFrequency(&PerfCountFrequencyResult);
	const int64 PerfCountFrequency = PerfCountFrequencyResult.QuadPart;

	// CSV file to write experimental data to
	std::ofstream outData;
	outData.open("runtimes.csv", std::ios::app);

	// Tested on T=4,...,N for N=4,6,8 (+2 zero-padded) and T=4,32,64,128,256,512,514 N=512 (512 zero-padded) [all with K=3]
	// Parameters:
	//	(note: 4K resolution is 2160x3840 [ROWSxCOLS])
	size_t N = pow(2, 9);    // Input-image size (2^12=4096)
	size_t K = 3;            // Kernel size
	size_t P = floor(K / 2); // Zero-padding on each size
	size_t N_zp = N + 2 * P; // Full zero-padded input size

	// JOSH: I think num_tiles = 1 if T = N_zp
	//       and num_tiles = 2 if T = N, 
	//       but what about between these?

	for (int T = N_zp; T > K; T-=1) // vary size of tile
	{
		size_t O = ceil((float)K / 2.f); // Overlap between tiles
		size_t L = T - O; // Non-overlap of input between tiles
		size_t num_tiles = ceil((float)(N_zp - O) / (float)L);
		size_t N_f = L * (num_tiles - 1) + T; // Needed size of zero-padded input to evenly fit tiles
		size_t N_out = N_f - 2 * P; // Output size (before discarding right and bottom)

		float* x = new float[N * N];
		for (int i = 0; i < N * N; i++)
			x[i] = i + 1; //rand();

		Image x_image(x, N, N, K, N_f);
		//x_image.print("x");
		delete[] x;

		Image z_image(N_out, N_out);  // 
		//z_image.print(); 

		assert(T >= 4); // Algorithm is not designed for shift of 1 
		assert(T >= K); // Kernel must fit inside tile
		assert(T <= N_zp); // Tile size == N_zp => one tile

		// Average of three metrics
		double micro_seconds_mean = {}; // Runtime
		double cycles_mean = {};        // Clock-cycles
		double fps_mean = {};           // Frames-per-second

		constexpr size_t num_runs = 1;
		double micro_seconds[num_runs] = {};
		double cycles[num_runs] = {};
		double fps[num_runs] = {};


		for (int run = 0; run < num_runs; ++run)
		{
			//https://docs.microsoft.com/en-us/windows/desktop/SysInfo/acquiring-high-resolution-time-stamps
			int64 BeginCycleCount = __rdtsc();
			LARGE_INTEGER BeginCounter;
			QueryPerformanceCounter(&BeginCounter);

			/// NOTE: Switch to fast_blur_proto() to see a printout of each tile (perhaps change to small tile size though for viewing ease)
			//FastBlur::fast_blur_proto(x_image, z_image, N, T, K);
			FastBlur::fast_blur(x_image, z_image, N, T, K);

			LARGE_INTEGER EndCounter;
			QueryPerformanceCounter(&EndCounter);

			int64 EndCycleCount = __rdtsc();

			int64 CounterElapsed = EndCounter.QuadPart - BeginCounter.QuadPart;
			__int32 ms_per_frame = (1e3*CounterElapsed) / PerfCountFrequency; // counts / (counts/s) = s
			__int32 us_per_frame = (1e6*CounterElapsed) / PerfCountFrequency; // counts / (counts/s) = s
			__int32 ns_per_frame = (1e9*CounterElapsed) / PerfCountFrequency; // counts / (counts/s) = s

			int64 FPS = (1 / ((double)ms_per_frame)*1e3);
			int64 CyclesElapsed = EndCycleCount - BeginCycleCount;

			using std::cout;
			cout << std::fixed << std::setprecision(2);
			cout << "ms/frame = " << ms_per_frame << "\t";
			cout << "Frames Per Second = " << FPS << "\t";
			cout << "Cycles elapsed = " << CyclesElapsed / 1.e6 << " MHz\t";
			cout << "Approximate clock-speed: " << FPS * CyclesElapsed / 1.e9 << " GHz\n";
			//  ( Frames / s. ) x ( Cycles / frame. ) = Clock Speed

			micro_seconds[run] = (double)us_per_frame;
			micro_seconds_mean += (double)us_per_frame;

			cycles[run] = (double)CyclesElapsed;
			cycles_mean += (double)CyclesElapsed;

			fps[run] = (double)FPS;
			fps_mean += (double)FPS;
		}
		micro_seconds_mean /= (double)num_runs;
		cycles_mean /= (double)num_runs;
		fps_mean /= (double)num_runs;


		// higher order statistics
		double us_std(0), fps_std(0), cycles_std(0);
		for (int run = 0; run < num_runs; ++run)
		{
			double us_temp = (micro_seconds[run] - micro_seconds_mean);
			us_std += us_temp * us_temp; // (us[i] - us_mean)^2
			//us_skw += us_std * us_temp;  // (us[i] - us_mean)^3
			//us_krt += us_skw * us_temp;  // (us[i] - us_mean)^4

			double fps_temp = (fps[run] - fps_mean);
			fps_std += fps_temp * fps_temp; // (fps[i] - fps_mean)^2
			//fps_skw += fps_std * fps_temp;  // (fps[i] - fps_mean)^3
			//fps_krt += fps_skw * fps_temp;  // (fps[i] - fps_mean)^4

			double cycles_temp = (cycles[run] - cycles_mean);
			cycles_std += cycles_temp * cycles_temp; // (cycles[i] - cycles_mean)^2
			//cycles_skw += cycles_std * cycles_temp;  // (cycles[i] - cycles_mean)^3
			//cycles_krt += cycles_skw * cycles_temp;  // (cycles[i] - cycles_mean)^4
		}
		double factor = (1. / (double)num_runs); // 1/(num_runs-1)
		factor = (1. / (double)num_runs);

		us_std = sqrt(factor * us_std);

		using std::cout;
		cout << std::fixed << std::setprecision(1);

		cout << "\n\n---------------------------------------------------------\n";
		cout << "Statistics summary over " << num_runs << "-runs\n\n";
		cout << "Image-size: " << N << "x" << N << "\t\tKernel-size: " << K << "x" << K << "\tTile-size: " << T << "x" << T;;
		cout << "\n---------------------------------------------------------\n";


		//// TODO: Fix this
		//double runtime_mean{}, runtime_std{};
		//if (1e3 <= micro_seconds_mean && micro_seconds_mean < 1e6)
		//{
		//	runtime_mean = micro_seconds_mean / 1.e3;
		//	runtime_std = us_std / 1.e3;
		//	cout << "\nruntime mean = " << runtime_mean << " ms./frame\t\t(standard-deviation = " << runtime_std << ")\n";
		//}
		//else
		//{
			cout << "\nruntime mean = " << micro_seconds_mean << " us./frame\t\t(standard-deviation = " << us_std << ")\n";
			//assert(1e3 <= micro_seconds_mean);
			//assert(micro_seconds_mean < 1e6);
		//}

		cout << "    fps mean = " << fps_mean << " frames/s." << /*"\tstd = " << fps_std << */"\n";
		cout << " cycles mean = " << cycles_mean / 1.e6 << " MCycles/frame" << /*" MHz.\tstd = " << cycles_std / 1.e6 <<*/ "\n\n\n";


		outData << micro_seconds_mean << "," << micro_seconds_mean << std::endl;
		//system("pause");

		//z_image.print("z");
		//std::cout << "\ntruncated:\n";

		Image z_image_trunc(N, N);
		z_image.truncate(N, N, z_image_trunc);
		//z_image_trunc.print("Z");
		//z_image.view();

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

	} // for T

	system("pause");
	return 0;
}