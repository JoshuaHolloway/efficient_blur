#include <matlab_engine.h>
#include "Timer.h"
#include "imp_4.h"
#include "imp_1.h"
// - - - - - - - - - - - - - - - -
float* mat2arr(const cv::Mat& mat)
{
	auto lin = [](size_t i, size_t j, size_t N) 
	{ return i * N + j; };

	const size_t M = mat.rows;
	const size_t N = mat.cols;

	float* x_1D = new float[M * N];
	cv::Mat mat_f(M, N, CV_32FC1);
	mat.convertTo(mat_f, CV_32FC1);
	for (int i = 0; i < M; ++i)
		for (int j = 0; j < N; ++j)
			x_1D[lin(i, j, N)] = mat_f.at<float>(i, j);
	return x_1D;
}
// - - - - - - - - - - - - - - - - 
auto main() -> int
{
	const cv::Mat x_mat = cv::imread("lena_512.png", CV_LOAD_IMAGE_GRAYSCALE);
	const size_t M = x_mat.rows, N = x_mat.cols;
	assert(M == N);
	const float* x_f = mat2arr(x_mat);

	/// Imp-1 - TODO: Write to buffer already zero-padded
                                              // prototype
	const float* z1 = naive::imp_1_general(x_f, N); // final

	/// Imp-4
	//tiled::imp_4(); // prototype
	const float* z4 = tiled::imp_4_general_tile6x6(x_f, N); // final
	
#ifdef MATLAB
	// Compare result against golden reference
	Matlab::Matlab matlab;
	matlab.pass_2D_into_matlab(z1, M, N, "z1_cpp");
	matlab.pass_2D_into_matlab(z4, M, N, "z4_cpp");
	matlab.command("box_blur");
#endif

	getchar();
	delete[] x_f, z1, z4;
	return 0;
}